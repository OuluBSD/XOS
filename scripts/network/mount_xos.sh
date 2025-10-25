#!/bin/bash

# XOS Image Mount Helper Script
# Provides convenient commands for mounting and unmounting the XOS disk image

set -e  # Exit on any error

XOS_IMAGE="/home/sblo/XOS/bins/xos_memstick.img"
LOOP_DEVICE="/dev/loop5"
MOUNT_POINT="/mnt/xos"

# Function to display usage
usage() {
    echo "Usage: $0 [command]"
    echo "Commands:"
    echo "  mount     Mount the XOS image to $MOUNT_POINT"
    echo "  unmount   Unmount the XOS image from $MOUNT_POINT"
    echo "  setup     Setup the loop device for the XOS image"
    echo "  cleanup   Remove the loop device"
    echo "  all       Setup loop device and mount image (default)"
    echo "  help      Show this help"
    exit 1
}

# Default command
COMMAND="all"

if [[ $# -gt 0 ]]; then
    COMMAND="$1"
    shift
fi

case "$COMMAND" in
    mount)
        echo "Mounting XOS image..."
        sudo mkdir -p "$MOUNT_POINT"
        sudo mount -o uid=$(id -u),gid=$(id -g) "$LOOP_DEVICE" "$MOUNT_POINT"
        echo "XOS image mounted to $MOUNT_POINT"
        ;;
    unmount)
        echo "Unmounting XOS image..."
        sudo umount "$MOUNT_POINT"
        echo "XOS image unmounted from $MOUNT_POINT"
        ;;
    setup)
        echo "Setting up loop device for XOS image..."
        sudo udisksctl loop-setup -f "$XOS_IMAGE"
        echo "Loop device created for $XOS_IMAGE"
        ;;
    cleanup)
        echo "Cleaning up loop device..."
        sudo udisksctl loop-delete -b "$LOOP_DEVICE"
        echo "Loop device $LOOP_DEVICE deleted"
        ;;
    all)
        echo "Setting up and mounting XOS image..."
        sudo udisksctl loop-setup -f "$XOS_IMAGE"
        sleep 2  # Give the system time to recognize the device
        sudo mkdir -p "$MOUNT_POINT"
        sudo mount -o uid=$(id -u),gid=$(id -g) "$LOOP_DEVICE" "$MOUNT_POINT"
        echo "XOS image set up and mounted to $MOUNT_POINT"
        ;;
    help|*)
        usage
        ;;
esac