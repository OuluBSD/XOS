#!/bin/bash

# XOS Development Loop Script
# Implements the two-process approach for QEMU and OCR with proper timeouts

set -e  # Exit on any error

# Default values
QEMU_TIMEOUT=120  # 2 minutes for QEMU
OCR_TIMEOUT=60    # 1 minute for OCR
OCR_INTERVAL=20   # Check every 20 seconds
XOS_IMAGE="/home/sblo/XOS/bins/xos_memstick.img"

# Function to display usage
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -q, --qemu-timeout SEC    Timeout for QEMU process (default: 120)"
    echo "  -o, --ocr-timeout SEC     Timeout for OCR process (default: 60)"
    echo "  -i, --interval SEC        Interval to check for output (default: 20)"
    echo "  -h, --help                Show this help"
    exit 1
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -q|--qemu-timeout)
            QEMU_TIMEOUT="$2"
            shift 2
            ;;
        -o|--ocr-timeout)
            OCR_TIMEOUT="$2"
            shift 2
            ;;
        -i|--interval)
            OCR_INTERVAL="$2"
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "Unknown option: $1"
            usage
            ;;
    esac
done

echo "Starting XOS Development Loop"
echo "QEMU timeout: ${QEMU_TIMEOUT}s"
echo "OCR timeout: ${OCR_TIMEOUT}s"
echo "Check interval: ${OCR_INTERVAL}s"

# Check if QEMU and OCR scripts exist
if [[ ! -f "./start_xos_vga.sh" ]]; then
    echo "Error: start_xos_vga.sh not found"
    exit 1
fi

if [[ ! -f "./qemu_screenshot_ocr.sh" ]]; then
    echo "Error: qemu_screenshot_ocr.sh not found"
    exit 1
fi

if [[ ! -f "$XOS_IMAGE" ]]; then
    echo "Error: XOS image not found at $XOS_IMAGE"
    exit 1
fi

# Function to kill background processes
cleanup() {
    echo "Cleaning up background processes..."
    if [[ -n "$QEMU_PID" ]]; then
        kill $QEMU_PID 2>/dev/null || true
    fi
    if [[ -n "$OCR_PID" ]]; then
        kill $OCR_PID 2>/dev/null || true
    fi
}
trap cleanup EXIT

# Start QEMU in background
echo "Starting QEMU in background..."
timeout $QEMU_TIMEOUT ./start_xos_vga.sh &
QEMU_PID=$!

echo "QEMU started with PID $QEMU_PID"

# Give QEMU time to fully boot before starting OCR
echo "Waiting for XOS to boot completely before starting OCR..."
sleep 60

# Run OCR once after waiting for boot
echo "Waiting for XOS to execute commands..."
sleep $OCR_TIMEOUT

echo "Running OCR once to capture the output..."
./qemu_screenshot_ocr.sh || true

# Show the captured OCR results
if [[ -s "qemu_ocr_output.txt" ]]; then
    echo ""
    echo "=== OCR OUTPUT ==="
    cat "qemu_ocr_output.txt"
    echo ""
    echo "=== END OCR OUTPUT ==="
    echo ""
fi

echo "Development loop completed."
echo "Results are in $OCR_OUTPUT_FILE"