#!/bin/bash

# XOS QEMU startup script with serial console support

# Define disk image - using the pre-built XOS image
DISK_IMG="/home/sblo/XOS/bins/xos_memstick.img"

echo "Starting XOS in QEMU with pre-built image..."
echo "Using $DISK_IMG"
echo "Serial console is available via stdio"
echo "Press Ctrl+A, then X to exit QEMU"

qemu-system-i386 \
    -m 32M \
    -boot c \
    -drive file="$DISK_IMG",index=0,media=disk,format=raw \
    -rtc base=localtime \
    -nographic \
    -no-shutdown \
    -monitor none \
    -serial mon:stdio
