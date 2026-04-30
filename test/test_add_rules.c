#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include "../include/packet_inspection_ioctl.h"

int main() {
    int fd = open("/dev/packet_inspection", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }
    
    struct packet_inspection_rule rule;
    
    // Rule 1: Block SSH from 192.168.1.100
    memset(&rule, 0, sizeof(rule));
    rule.src_ip = inet_addr("192.168.1.100");
    rule.protocol = 6;  // TCP
    rule.dst_port = htons(22);  // SSH
    rule.action = 0;  // DROP
    
    if (ioctl(fd, PACKET_INSPECTION_ADD_RULE, &rule) == 0) {
        printf("Added rule ID %u: Block SSH from 192.168.1.100\n", rule.id);
    }
    
    // Rule 2: Allow HTTP to 10.0.0.1
    memset(&rule, 0, sizeof(rule));
    rule.dst_ip = inet_addr("10.0.0.1");
    rule.protocol = 6;  // TCP
    rule.dst_port = htons(80);  // HTTP
    rule.action = 1;  // ACCEPT
    
    if (ioctl(fd, PACKET_INSPECTION_ADD_RULE, &rule) == 0) {
        printf("Added rule ID %u: Allow HTTP to 10.0.0.1\n", rule.id);
    }
    
    // Rule 3: Drop all ICMP
    memset(&rule, 0, sizeof(rule));
    rule.protocol = 1;  // ICMP
    rule.action = 0;  // DROP
    
    if (ioctl(fd, PACKET_INSPECTION_ADD_RULE, &rule) == 0) {
        printf("Added rule ID %u: Drop all ICMP\n", rule.id);
    }
    
    close(fd);
    return 0;
}
