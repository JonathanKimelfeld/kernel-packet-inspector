#!/bin/bash

echo "=== Packet Inspection System Demo ==="
echo

# Load kernel module
echo "1. Loading kernel module..."
cd ../kernel
sudo insmod packet_inspection.ko
MAJOR=$(dmesg | grep "packet_inspection: Registered" | tail -1 | grep -oP 'major number \K\d+')
echo "   Major number: $MAJOR"

# Create device node
echo "2. Creating device node..."
sudo mknod /dev/packet_inspection c $MAJOR 0 2>/dev/null || echo "   Device already exists"
sudo chmod 666 /dev/packet_inspection

# Add rules
echo "3. Adding filtering rules..."
cd ../test
gcc test_add_rules.c -o test_add_rules
./test_add_rules

# Show rules
echo
echo "4. Active rules:"
cat /dev/packet_inspection

# Show stats
echo
echo "5. Statistics:"
cat /proc/packet_inspection_stats

# Cleanup
echo
echo "6. Cleanup..."
sudo rmmod packet_inspection
sudo rm /dev/packet_inspection

echo
echo "=== Demo Complete ==="
