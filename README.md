
## Part 1: Kernel Module

A Linux kernel module, which provides packet filtering rule management & statistics.

### Build Instructions

```bash
cd kernel/
make
```

### Installation

```bash
# Load module
sudo insmod packet_inspection.ko

# Create device node & using major number
MAJOR=$(sudo dmesg | grep "packet_inspection: Registered" | tail -1 | grep -oP 'major number \K\d+')
sudo mknod /dev/packet_inspection c $MAJOR 0
sudo chmod 666 /dev/packet_inspection
```

### Using the rules and module

**Add new rules:**
```bash
cd test/
gcc test_add_rules.c -o test_add_rules
./test_add_rules
```

**List existing rules:**
```bash
cat /dev/packet_inspection
```

**View statistics: (in /proc)**
```bash
cat /proc/packet_inspection_stats
```

**Unload module:**
```bash
sudo rmmod packet_inspection
sudo rm /dev/packet_inspection
```

### Tests

Run the demo (sh script):
```bash
cd test/
chmod +x demo.sh
./demo.sh
```

### Design Overview

# Key design choices

1. **Linked list for storing the filtering rules:**
   - Choosing fixed array would require choosing a fixed size upfront
   - Dynamic memory: rule gets allocated only when it's added, no wasted space as using static
   - No hardcoded max on num of rules (added a configurable limit via module parameter for safety)
   - Insertion in O(1) when adding to tail, deletion doesn't require shifting rest of rules
   - The tradeoff: lookup is O(n) when searching through rules, but for a typical set of rules this is acceptable

2. **Spinlock instead of mutex for protecting the rules list:**
   - The critical sections are very short - just adding/removing from the list
   - Sleeping isn't allowed in kernel contexts
   - Lower overhead than a mutex
   - spinlock is safter here since operations are fast

3. **Auto-incrementing rule IDs:**
   - Kernel assigns IDs automatically - set up an atomic counter,
   - which prevents users from accidentally creating duplicate ids
   - allows for simpler API so that users don't have to worry which ids are in use

4. **Atomic counters in statistics:**
   - Packet statistics (seen/passed/dropped) use atomic variables
   - incrementing them doesn't require lock
   - doesnt require spinlocks; much better performance without them

5. **Input validation before allocating memory:**
   - Check protocol is valid before doing anything else
   - Check action is either drop or accept
   - If validation fails, return immediately skipping an invalid rule
   - clear error messages for user

6. **copy_from_user and copy_to_user**
   - Pointers from userspace could point anywhere, can't be trusted
   - copy_from_user & copt_to_user validate addresses
   - Prevents the kernel from crashing if user passes a bad pointer
   - critical necessety for saftey & fault tolerance

### Assumptions

- Rules checked in order based on first match found
- Any process with device access can modify rules because there's no auth
- Small amount of rules, which is typical in firewall usage, that's why O(n) lookup is still acceptable here

###Limitations:

- No rule priority beyond insertion order
- Rule matching done in user space
- No per-rule statistics- only global counts
- LinkedList limitation: rules can't be reordered without delete/re-add
- Important: no rule conflict detection or validation
- Statistics can't be reset without unload module
- listing rules via read() allows for 4kb output
- no persistence: rules are lost when module unload

### Development Environment

- Ubuntu 24.04 (ARM64)
- Linux kernel 7.0.0-14-generic
- GCC 15.2.0

### AI Usage

**Tools I used:** Claude 3.7 Sonnet

**What it helped me with:**
- Initial kernel module structure and boilerplate
- use of APIs for kernel (copy_from_user, spinlocks, proc_fs)
- ioctl commands and their definitions
- Debugging compilation errors
- setting up ssh from my mac to the vm
- Debating design decisions (mainly linkedList and spinlock) 

**What I verified/changed:**
- all code reviewed and tested personally, with notes for readability
- Modified error handling and paths
- Adjusted locking strategy
- tested concurrent access
- Verified memory cleanup on module exit

**What I understand:**
- Character device regist. and file_operations
- Kernel memory management - kernel kmalloc/kfree
- Synchronization - the spinlock vs mutex trade-off
- User-kernel data transfer (copy_from_user/copy_to_user)
- Kernel module lifecycle (init/exit)
