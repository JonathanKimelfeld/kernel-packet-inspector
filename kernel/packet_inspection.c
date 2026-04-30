#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "../include/packet_inspection_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jonathan");
MODULE_DESCRIPTION("Packet Inspection Device");

#define DEVICE_NAME "packet_inspection"


/*
* Packet Inspection Kernel Module
* 
* My design rationale:
* 1. LinkedList for storing the rules:
* - Dynamic memory allocation when needed for new rules
* - Easy insertion (O(1)) and deletion without moving the rest of the stored rules
* - Acceptable for small rule number, since it's still O(n) lookup - that's the tradeoff
* 2. Spinlock over mutex:
* - sleeping is not allowed in kernel (short critical sections in general),
* - lower overhead than mutex
* 3. Inrementing rule ids automatically:
* - preventing conflicts of duplicate ids
* - simpler api for user, doesn't require to manage id allocation
*/


static int major_number;

// Module parameter for maximum rules
static int max_rules = 100;
module_param(max_rules, int, 0444);
MODULE_PARM_DESC(max_rules, "Maximum number of filtering rules");

struct rule_entry {
    struct packet_inspection_rule rule;
    struct list_head list;
};

static LIST_HEAD(rules_list);
static DEFINE_SPINLOCK(rules_lock);

static atomic_t next_rule_id = ATOMIC_INIT(1);
static atomic_t rule_count = ATOMIC_INIT(0);

static atomic64_t packets_seen = ATOMIC64_INIT(0);
static atomic64_t packets_passed = ATOMIC64_INIT(0);
static atomic64_t packets_dropped = ATOMIC64_INIT(0);

static int proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Packet Inspection -  Statistics\n");
    seq_printf(m, "============================\n");
    seq_printf(m, "Packets seen:    %lld\n", atomic64_read(&packets_seen));
    seq_printf(m, "Packets passed:  %lld\n", atomic64_read(&packets_passed));
    seq_printf(m, "Packets dropped: %lld\n", atomic64_read(&packets_dropped));
    
    return 0;
}

static int proc_open(struct inode *inode, struct file *file)
{
    return (single_open(file, proc_show, NULL));
}

static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static struct proc_dir_entry *proc_entry;

static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "packet_inspection: Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "packet_inspection: Device closed\n");
    return 0;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    struct rule_entry *entry;
    char *output;
    int written = 0;
    int ret;
    
    if (*offset > 0)
        return 0;
    
    output = kmalloc(4096, GFP_KERNEL);
    if (!output)
        return -ENOMEM;
    
    written += sprintf(output + written, "Active Filtering Rules:\n");
    written += sprintf(output + written, "=======================\n");
    
    spin_lock(&rules_lock);
    list_for_each_entry(entry, &rules_list, list) {
        written += sprintf(output + written,
            "ID=%u src=%pI4 dst=%pI4 proto=%u sport=%u dport=%u action=%s\n",
            entry->rule.id,
            &entry->rule.src_ip,
            &entry->rule.dst_ip,
            entry->rule.protocol,
            ntohs(entry->rule.src_port),
            ntohs(entry->rule.dst_port),
            entry->rule.action ? "ACCEPT" : "DROP");
    }
    spin_unlock(&rules_lock);
    
    if (written == 0)
        written += sprintf(output + written, "(no rules)\n");
    
    ret = simple_read_from_buffer(buffer, len, offset, output, written);
    kfree(output);
    
    return ret;
}

// Device write

static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    printk(KERN_INFO "packet_inspection: Write operation not supported (use ioctl)\n");
    return -EINVAL;
}

// Device input output control (ioctl)

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct packet_inspection_rule user_rule;
    struct rule_entry *entry, *tmp;
    uint32_t rule_id;
    int ret = 0;
    
    switch (cmd) {
	
	// Add rule
        case PACKET_INSPECTION_ADD_RULE:

            if (copy_from_user(&user_rule, (void __user *)arg, sizeof(user_rule))) {
                return -EFAULT;
            }

	    // Validate protocol values
    	    if (user_rule.protocol != 1 && user_rule.protocol != 6 && 
		    user_rule.protocol != 17 && user_rule.protocol != 0) {
                printk(KERN_WARNING "packet_inspection: Invalid protocol %u\n", user_rule.protocol);
                return -EINVAL;
            }
    
            // Validate action
            if (user_rule.action > 1) {
                printk(KERN_WARNING "packet_inspection: Invalid action %u\n", user_rule.action);
                return -EINVAL;
            }

	    // Check max rules limit
	    if (atomic_read(&rule_count) >= max_rules) {
		printk(KERN_WARNING "packet_inspection: Maximum rules (%d) reached\n", max_rules);
    		return -ENOSPC;
	    }
            
	    // Kernel malloc
            entry = kmalloc(sizeof(*entry), GFP_KERNEL);
            if (!entry) {
                return -ENOMEM;
            }
            
            user_rule.id = atomic_inc_return(&next_rule_id);
            entry->rule = user_rule;
            
            spin_lock(&rules_lock);
            list_add_tail(&entry->list, &rules_list);
            atomic_inc(&rule_count);
	    spin_unlock(&rules_lock);
	    
            printk(KERN_INFO "packet_inspection: Added rule ID %u\n", user_rule.id);
            
            if (copy_to_user((void __user *)arg, &user_rule, sizeof(user_rule))) {
                return -EFAULT;
            }
            break;
        
	// Delete rule
        case PACKET_INSPECTION_DEL_RULE:

            if (copy_from_user(&rule_id, (void __user *)arg, sizeof(rule_id))) {
                return -EFAULT;
            }
            
            spin_lock(&rules_lock);
            list_for_each_entry_safe(entry, tmp, &rules_list, list) {
                if (entry->rule.id == rule_id) {
                    list_del(&entry->list);
                    atomic_dec(&rule_count);
		    spin_unlock(&rules_lock);
                    kfree(entry);
		    

                    printk(KERN_INFO "packet_inspection: Deleted rule ID %u\n", rule_id);

                    return 0;
                }
            }
            spin_unlock(&rules_lock);
            
            printk(KERN_WARNING "packet_inspection: Rule ID %u not found\n", rule_id);

            return -ENOENT;
            
	// Get stats rule
        case PACKET_INSPECTION_GET_STATS:
            {
                struct packet_inspection_stats stats;
                stats.packets_seen = atomic64_read(&packets_seen);
                stats.packets_passed = atomic64_read(&packets_passed);
                stats.packets_dropped = atomic64_read(&packets_dropped);
                
                if (copy_to_user((void __user *)arg, &stats, sizeof(stats))) {
                    return -EFAULT;
                }
                printk(KERN_INFO "packet_inspection: Sent stats to userspace\n");
            }
            break;
            
        case PACKET_INSPECTION_LIST_RULES:
            printk(KERN_INFO "packet_inspection: LIST_RULES not yet implemented\n");
            return -ENOSYS;
            
        default:
            return -EINVAL;
    }
    
    return (ret);
}

// File operations (fops)

static struct file_operations fops = {

    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,

};

// Init packet inspection

static int __init packet_inspection_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    
    if (major_number < 0) {
        printk(KERN_ALERT "packet_inspection: Failed to register device\n");
        return major_number;
    }
    
    proc_entry = proc_create("packet_inspection_stats", 0444, NULL, &proc_fops);
    if (!proc_entry) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "packet_inspection: Failed to create /proc entry\n");
        return -ENOMEM;
    }
    
    printk(KERN_INFO "packet_inspection: Registered with major number %d\n", major_number);
    
    return 0;
}

// Exit packet inspection

static void __exit packet_inspection_exit(void)
{
    struct rule_entry *entry, *tmp;
    
    proc_remove(proc_entry);
    
    spin_lock(&rules_lock);

    list_for_each_entry_safe(entry, tmp, &rules_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }

    spin_unlock(&rules_lock);
    
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "packet_inspection: Module unloaded, all rules freed\n");
}

// exit and init
module_init(packet_inspection_init);
module_exit(packet_inspection_exit);
