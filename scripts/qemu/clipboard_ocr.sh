#!/bin/bash

# Script to read screenshot from clipboard and perform OCR
# Author: Qwen Code
# Date: 2025-10-25

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
for tool in xclip magick tesseract convert; do
    if ! command -v "$tool" &> /dev/null; then
        echo "Error: $tool is not installed or not in PATH"
        exit 1
    fi
done

# Function to show usage
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -i, --input FILE        Input image file (default: clipboard)"
    echo "  -o, --output FILE       Output file for OCR result (default: clipboard_ocr_output.txt)"
    echo "  -l, --lang LANG         OCR language (default: eng)"
    echo "  -d, --dry-run           Show what would be done without executing"
    echo "  -h, --help              Show this help"
    exit 1
}

# Default values
INPUT_FILE=""
OUTPUT_FILE="clipboard_ocr_output.txt"
OCR_LANG="eng"
TEMP_INPUT_FILE="/tmp/clipboard_screenshot_$$.png"
DRY_RUN=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -i|--input)
            INPUT_FILE="$2"
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

if [ "$DRY_RUN" = true ]; then
    if [ -n "$INPUT_FILE" ]; then
        echo "DRY RUN: Would perform OCR on $INPUT_FILE with language $OCR_LANG"
    else
        echo "DRY RUN: Would extract image from clipboard to $TEMP_INPUT_FILE"
        echo "DRY RUN: Would perform OCR with language $OCR_LANG"
    fi
    echo "DRY RUN: Would save result to $OUTPUT_FILE"
    echo "DRY RUN: Completed successfully"
    exit 0
fi

echo "Reading screenshot..."

if [ -n "$INPUT_FILE" ]; then
    # Use provided input file
    if [ ! -f "$INPUT_FILE" ]; then
        echo "Error: Input file $INPUT_FILE does not exist"
        exit 1
    fi
    cp "$INPUT_FILE" "$TEMP_INPUT_FILE"
    echo "Using input file: $INPUT_FILE"
else
    # Read from clipboard
    if xclip -selection clipboard -o -t image/png > "$TEMP_INPUT_FILE" 2>/dev/null; then
        echo "Screenshot captured from clipboard (PNG format)"
    elif xclip -selection clipboard -o -t image/jpg > "$TEMP_INPUT_FILE" 2>/dev/null; then
        echo "Screenshot captured from clipboard (JPG format)"
    elif xclip -selection clipboard -o -t image/jpeg > "$TEMP_INPUT_FILE" 2>/dev/null; then
        echo "Screenshot captured from clipboard (JPEG format)"
    elif xclip -selection clipboard -o > "$TEMP_INPUT_FILE" 2>/dev/null; then
        echo "Screenshot captured from clipboard (unknown format, assuming image)"
    else
        echo "Error: No image found in clipboard"
        exit 1
    fi
fi

# Perform OCR using tesseract with image preprocessing optimized for bitmap fonts
echo "Preprocessing image for better OCR (optimizing for bitmap font)..."
# Convert to grayscale, make gray areas white with appropriate threshold, resize for bitmap fonts
convert "$TEMP_INPUT_FILE" -colorspace Gray -normalize -level 10%,90% -resize 100%x150% -negate "$TEMP_INPUT_FILE.preprocessed.png"

# Use the preprocessed image for OCR with PSM mode optimized for single uniform block of text
echo "Performing OCR with debug logging..."
tesseract --loglevel DEBUG -l "$OCR_LANG" --psm 6 "$TEMP_INPUT_FILE.preprocessed.png" "/tmp/clipboard_ocr_result_$$" txt

# Output the OCR result
if [ -f "/tmp/clipboard_ocr_result_$$.txt" ]; then
    # Copy OCR result to output file
    cp "/tmp/clipboard_ocr_result_$$.txt" "$OUTPUT_FILE"
    
    echo ""
    echo "OCR Result:"
    echo "-----------"
    cat "$OUTPUT_FILE"
    echo ""
    
    # Clean up temporary files
    rm -f "$TEMP_INPUT_FILE" "$TEMP_INPUT_FILE.preprocessed.png" "/tmp/clipboard_ocr_result_$$.txt"
else
    # Fallback to original image if preprocessing failed
    echo "Preprocessing failed, trying with original image..."
    tesseract --loglevel DEBUG -l "$OCR_LANG" --psm 6 "$TEMP_INPUT_FILE" "/tmp/clipboard_ocr_result_$$" txt
    
    if [ -f "/tmp/clipboard_ocr_result_$$.txt" ]; then
        # Copy OCR result to output file
        cp "/tmp/clipboard_ocr_result_$$.txt" "$OUTPUT_FILE"
        
        echo ""
        echo "OCR Result:"
        echo "-----------"
        cat "$OUTPUT_FILE"
        echo ""
        
        # Clean up temporary files
        rm -f "$TEMP_INPUT_FILE" "/tmp/clipboard_ocr_result_$$.txt"
    else
        echo "Error: OCR output file was not created"
        exit 1
    fi
fi

echo "OCR completed successfully!"