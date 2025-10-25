#!/bin/bash

# Script to take screenshot of QEMU window and perform OCR
# Author: Qwen Code
# Date: 2025-10-15

set -e  # Exit on any error

# Enable command tracing
#set -x

# Check if we're in an X11 environment
if [ -z "$DISPLAY" ]; then
    echo "Error: No X11 display found. Please run this script in a graphical environment."
    echo "If running over SSH, use 'ssh -X' or 'ssh -Y' to forward X11."
    exit 1
fi

# Check if required tools are available
for tool in xwininfo magick tesseract convert; do
    if ! command -v "$tool" &> /dev/null; then
        echo "Error: $tool is not installed or not in PATH"
        exit 1
    fi
done

# Function to show usage
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -w, --window-name PATTERN   Window name pattern to match (default: qemu)"
    echo "  -o, --output FILE         Output image file (default: qemu_screenshot.png)"
    echo "  -l, --lang LANG           OCR language (default: eng)"
    echo "  -d, --dry-run             Show what would be done without executing"
    echo "  -h, --help                Show this help"
    exit 1
}

# Default values
WINDOW_NAME="qemu"
OUTPUT_FILE="qemu_screenshot.png"
OCR_LANG="eng"
OCR_OUTPUT_FILE=""
DRY_RUN=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -w|--window-name)
            WINDOW_NAME="$2"
            shift 2
            ;;
        -o|--output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        -l|--lang)
            OCR_LANG="$2"
            shift 2
            ;;
        -d|--dry-run)
            DRY_RUN=true
            shift
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

# If --output was specified, the OCR output will be written to stdout
# If no --output was specified (default), we'll use a temporary file for OCR and remove it
if [[ "$OUTPUT_FILE" == "qemu_screenshot.png" ]]; then
    OCR_OUTPUT_FILE="/tmp/qemu_ocr_output_$$"
else
    # Extract base name without extension for OCR output
    BASE_NAME="${OUTPUT_FILE%.*}"
    OCR_OUTPUT_FILE="$BASE_NAME"
fi

echo "Looking for visible QEMU window..."

# Find the QEMU window ID, filtering for visible windows (not unmapped)
WINDOW_ID=$(xwininfo -root -tree | grep -E '"qemu"|QEMU' -i | grep -v '1x1+-1+-1' | grep -v '10x10' | head -n1 | grep -o '0x[0-9a-f]*')

if [ -z "$WINDOW_ID" ]; then
    echo "Error: No visible QEMU window found matching pattern"
    
    # Show available windows as hint
    echo ""
    echo "All windows containing 'qemu' (case insensitive):"
    xwininfo -root -tree | grep -i qemu
    
    exit 1
fi

echo "Found QEMU window ID: $WINDOW_ID"

if [ "$DRY_RUN" = true ]; then
    echo "DRY RUN: Would capture screenshot of window $WINDOW_ID to $OUTPUT_FILE"
    echo "DRY RUN: Would perform OCR with language $OCR_LANG"
    echo "DRY RUN: Completed successfully"
    exit 0
fi

# Take screenshot using magick import
echo "Capturing screenshot..."
magick import -window "$WINDOW_ID" "$OUTPUT_FILE"

echo "Screenshot saved to: $OUTPUT_FILE"

# Crop out the top 24 pixels to remove QEMU menubar before OCR
echo "Cropping menubar from screenshot..."
convert "$OUTPUT_FILE" -crop +0+24 "$OUTPUT_FILE.cropped.png"
mv "$OUTPUT_FILE.cropped.png" "$OUTPUT_FILE"

# Perform OCR using tesseract with image preprocessing optimized for bitmap fonts
echo "Preprocessing image for better OCR (optimizing for bitmap font)..."
# Convert to grayscale, make gray areas white with appropriate threshold, resize for bitmap fonts
convert "$OUTPUT_FILE" -colorspace Gray -normalize -level 10%,90% -resize 100%x150% -negate "$OUTPUT_FILE.preprocessed.png"

# Use the preprocessed image for OCR with PSM mode optimized for single uniform block of text
echo "Performing OCR with debug logging..."
tesseract --loglevel DEBUG -l "$OCR_LANG" --psm 6 "$OUTPUT_FILE.preprocessed.png" "$OCR_OUTPUT_FILE" txt

# Output the OCR result
if [ -f "$OCR_OUTPUT_FILE.txt" ]; then
    # Copy OCR result to qemu_ocr_output.txt
    cp "$OCR_OUTPUT_FILE.txt" "qemu_ocr_output.txt"
    
    echo ""
    echo "OCR Result:"
    echo "-----------"
    cat "$OCR_OUTPUT_FILE.txt"
    echo ""
    
    # Clean up temporary files if we created them
    rm -f "$OUTPUT_FILE.preprocessed.png"
    if [[ "$OUTPUT_FILE" == "qemu_screenshot.png" ]]; then
        rm -f "$OCR_OUTPUT_FILE.txt"
    fi
else
    # Fallback to original image if preprocessing failed
    echo "Preprocessing failed, trying with original cropped image..."
    tesseract --loglevel DEBUG -l "$OCR_LANG" --psm 6 "$OUTPUT_FILE" "$OCR_OUTPUT_FILE" txt
    
    if [ -f "$OCR_OUTPUT_FILE.txt" ]; then
        # Copy OCR result to qemu_ocr_output.txt
        cp "$OCR_OUTPUT_FILE.txt" "qemu_ocr_output.txt"
        
        echo ""
        echo "OCR Result:"
        echo "-----------"
        cat "$OCR_OUTPUT_FILE.txt"
        echo ""
        
        # Clean up temporary OCR output file if we created one
        if [[ "$OUTPUT_FILE" == "qemu_screenshot.png" ]]; then
            rm -f "$OCR_OUTPUT_FILE.txt"
        fi
    else
        echo "Error: OCR output file was not created"
        exit 1
    fi
fi

echo "OCR completed successfully!"
