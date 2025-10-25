#!/bin/bash

# XOS QEMU startup script with TCP serial console support

# Define disk image - using the pre-built XOS image
DISK_IMG="/home/sblo/XOS/bins/xos_memstick.img"

echo "Starting XOS in QEMU with pre-built image..."
echo "Using $DISK_IMG"
echo "Serial console is available via TCP port 4444"
echo "Connect with: telnet localhost 4444"
echo "Press Ctrl+A, then X to exit QEMU"

qemu-system-i386 \
    -m 32M \
    -boot c \
    -drive file="$DISK_IMG",index=0,media=disk,format=raw \
    -rtc base=localtime \
    -serial tcp:127.0.0.1:4444,server,nowait \
    -nographic \
    -no-shutdown