#ifndef PACKET_INSPECTION_IOCTL_H
#define PACKET_INSPECTION_IOCTL_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/ioctl.h>
#else
#include <stdint.h>
#include <sys/ioctl.h>
#endif

#define PACKET_INSPECTION_IOC_MAGIC 'P'

struct packet_inspection_rule {
    uint32_t id;
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
    uint8_t action;
};

struct packet_inspection_stats {
    uint64_t packets_seen;
    uint64_t packets_passed;
    uint64_t packets_dropped;
};

#define PACKET_INSPECTION_ADD_RULE    _IOW(PACKET_INSPECTION_IOC_MAGIC, 1, struct packet_inspection_rule)
#define PACKET_INSPECTION_DEL_RULE    _IOW(PACKET_INSPECTION_IOC_MAGIC, 2, uint32_t)
#define PACKET_INSPECTION_GET_STATS   _IOR(PACKET_INSPECTION_IOC_MAGIC, 3, struct packet_inspection_stats)
#define PACKET_INSPECTION_LIST_RULES  _IOR(PACKET_INSPECTION_IOC_MAGIC, 4, struct packet_inspection_rule)

#endif
