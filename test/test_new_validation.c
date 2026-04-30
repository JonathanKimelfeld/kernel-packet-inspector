#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h>
#include "../include/packet_inspection_ioctl.h"

int main() {
    int fd = open("/dev/packet_inspection", O_RDWR);
    struct packet_inspection_rule rule;
    
    // Test invalid protocol (should fail)
    memset(&rule, 0, sizeof(rule));
    rule.protocol = 99;  // Invalid
    rule.action = 1;
    
    printf("Testing invalid protocol...\n");
    if (ioctl(fd, PACKET_INSPECTION_ADD_RULE, &rule) < 0) {
        printf("✓ Correctly rejected invalid protocol\n");
    }
    
    // Test invalid action (should fail)
    rule.protocol = 6;
    rule.action = 5;  // Invalid
    
    printf("Testing invalid action...\n");
    if (ioctl(fd, PACKET_INSPECTION_ADD_RULE, &rule) < 0) {
        printf("✓ Correctly rejected invalid action\n");
    }
    
    close(fd);
    return 0;
}
