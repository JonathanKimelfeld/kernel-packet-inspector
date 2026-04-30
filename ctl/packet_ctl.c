#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include "../include/packet_inspection_ioctl.h"

#define DEVICE_PATH "/dev/packet_inspection"


// usage in print format

void print_usage(const char *prog) {
    printf("Usage: %s <command> [options]\n\n", prog);
    printf("Commands:\n");
    printf("  add <src_ip> <dst_ip> <protocol> <src_port> <dst_port> <action>\n");
    printf("      Add a new filtering rule\n");
    printf("      protocol: 0=any, 1=ICMP, 6=TCP, 17=UDP\n");
    printf("      action: 0=DROP, 1=ACCEPT\n");
    printf("      Use 0.0.0.0 for any IP, 0 for any port\n\n");
    printf("  del <rule_id>\n");
    printf("      Delete rule by ID\n\n");
    printf("  list\n");
    printf("      List all active rules\n\n");
    printf("  stats\n");
    printf("      Show packet statistics\n\n");
    printf("Examples:\n");
    printf("  %s add 192.168.1.100 0.0.0.0 6 0 22 0   # Block SSH from 192.168.1.100\n", prog);
    printf("  %s add 0.0.0.0 10.0.0.1 6 0 80 1        # Allow HTTP to 10.0.0.1\n", prog);
    printf("  %s del 5                                # Delete rule ID 5\n", prog);
}

// api call to add rule

int cmd_add_rule(int fd, int argc, char *argv[]) {
    struct packet_inspection_rule rule;
    
    if (argc < 8) {
        fprintf(stderr, "Error: Missing arguments for add command\n");
        return 1;
    }
    
    memset(&rule, 0, sizeof(rule));
    
    if (inet_pton(AF_INET, argv[2], &rule.src_ip) != 1) {
        fprintf(stderr, "Error: Invalid source IP: %s\n", argv[2]);
        return 1;
    }
    
    if (inet_pton(AF_INET, argv[3], &rule.dst_ip) != 1) {
        fprintf(stderr, "Error: Invalid destination IP: %s\n", argv[3]);
        return 1;
    }
    
    rule.protocol = atoi(argv[4]);
    rule.src_port = htons(atoi(argv[5]));
    rule.dst_port = htons(atoi(argv[6]));
    rule.action = atoi(argv[7]);
    
    if (ioctl(fd, PACKET_INSPECTION_ADD_RULE, &rule) < 0) {
        perror("Failed to add rule");
        return 1;
    }
    
    printf("Rule added successfully with ID: %u\n", rule.id);
    return 0;
}


// api call to delete rule

int cmd_del_rule(int fd, int argc, char *argv[]) {
    uint32_t rule_id;
    
    if (argc < 3) {
        fprintf(stderr, "Error: Missing rule ID\n");
        return 1;
    }
    
    rule_id = atoi(argv[2]);
    
    if (ioctl(fd, PACKET_INSPECTION_DEL_RULE, &rule_id) < 0) {
        perror("Failed to delete rule");
        return 1;
    }
    
    printf("Rule %u deleted successfully\n", rule_id);
    return 0;
}

int cmd_list_rules(int fd) {
    char buffer[4096];
    ssize_t bytes;
    
    bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        perror("Failed to list rules");
        return 1;
    }
    
    buffer[bytes] = '\0';
    printf("%s", buffer);
    return 0;
}


// get stats

int cmd_stats(int fd) {
    struct packet_inspection_stats stats;
    
    if (ioctl(fd, PACKET_INSPECTION_GET_STATS, &stats) < 0) {
        perror("Failed to get statistics");
        return 1;
    }
    
    printf("Packet Statistics:\n");
    printf("==================\n");
    printf("Packets seen:    %lu\n", stats.packets_seen);
    printf("Packets passed:  %lu\n", stats.packets_passed);
    printf("Packets dropped: %lu\n", stats.packets_dropped);
    
    return 0;
}

// main - ctl api

int main(int argc, char *argv[]) {
    int fd;
    int ret = 0;
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        fprintf(stderr, "Make sure the kernel module is loaded and device exists\n");
        return 1;
    }
    
    if (strcmp(argv[1], "add") == 0) {
        ret = cmd_add_rule(fd, argc, argv);
    } else if (strcmp(argv[1], "del") == 0) {
        ret = cmd_del_rule(fd, argc, argv);
    } else if (strcmp(argv[1], "list") == 0) {
        ret = cmd_list_rules(fd);
    } else if (strcmp(argv[1], "stats") == 0) {
        ret = cmd_stats(fd);
    } else {
        fprintf(stderr, "Unknown command: %s\n\n", argv[1]);
        print_usage(argv[0]);
        ret = 1;
    }
    
    close(fd);
    return ret;
}
