#!/bin/bash

# XOS QEMU startup script with network support

# Define disk image - using the pre-built XOS image
DISK_IMG="/home/sblo/XOS/bins/xos_memstick.img"

echo "Starting XOS in QEMU with network support..."
echo "Using $DISK_IMG"
echo "Network device: rtl8139 (Realtek 8139)"
echo "Serial console is available via TCP port 4445"
echo "Connect with: telnet localhost 4445"
echo "Press Ctrl+A, then X to exit QEMU"

qemu-system-i386 \
    -m 32M \
    -boot c \
    -drive file="$DISK_IMG",index=0,media=disk,format=raw \
    -rtc base=localtime \
    -vga std \
    -no-shutdown \
    -monitor none \
    -serial tcp:127.0.0.1:4445,server,nowait \
    -netdev user,id=net0 \
    -device rtl8139,netdev=net0