#!/bin/bash

# Script to loop through PNG files and convert them to TXT using tesseract if the text file doesn't exist yet
# Author: Qwen Code
# Date: 2025-10-25

set -e  # Exit on any error

# Enable command tracing
#set -x

# Check if required tools are available
for tool in tesseract convert; do
    if ! command -v "$tool" &> /dev/null; then
        echo "Error: $tool is not installed or not in PATH"
        exit 1
    fi
done

# Function to show usage
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -d, --dir DIR           Directory to search for PNG files (default: current directory)"
    echo "  -s, --start NUM         Starting number for the loop (default: 1)"
    echo "  -e, --end NUM           Ending number for the loop (default: 100000)"
    echo "  -l, --lang LANG         OCR language (default: eng)"
    echo "  -p, --prefix PREFIX     Filename prefix (default: screenshot_)"
    echo "  -d, --dry-run           Show what would be done without executing"
    echo "  -h, --help              Show this help"
    exit 1
}

# Default values
SEARCH_DIR="."
START_NUM=1
END_NUM=100000
OCR_LANG="eng"
FILE_PREFIX="screenshot_"
DRY_RUN=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--dir)
            SEARCH_DIR="$2"
            shift 2
            ;;
        -s|--start)
            START_NUM="$2"
            shift 2
            ;;
        -e|--end)
            END_NUM="$2"
            shift 2
            ;;
        -l|--lang)
            OCR_LANG="$2"
            shift 2
            ;;
        -p|--prefix)
            FILE_PREFIX="$2"
            shift 2
            ;;
        -n|--dry-run)
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
    echo "DRY RUN: Would process PNG files from $SEARCH_DIR with prefix '$FILE_PREFIX' and numbers from $START_NUM to $END_NUM"
    echo "DRY RUN: Would use language $OCR_LANG for OCR"
    echo "DRY RUN: Would create .txt file for each corresponding .png if the .txt file doesn't exist"
    echo "DRY RUN: Completed successfully"
    exit 0
fi

echo "Processing PNG files in directory: $SEARCH_DIR"
echo "File prefix: $FILE_PREFIX"
echo "Processing numbers from $START_NUM to $END_NUM"

# Loop through numbers from START_NUM to END_NUM
for ((i = START_NUM; i <= END_NUM; i++)); do
    png_file="$SEARCH_DIR/${FILE_PREFIX}${i}.png"
    txt_file="$SEARCH_DIR/${FILE_PREFIX}${i}.txt"
    
    # Check if PNG file exists
    if [ -f "$png_file" ]; then
        # Check if TXT file doesn't exist
        if [ ! -f "$txt_file" ]; then
            echo "Processing: $png_file"
            
            # Preprocess image for better OCR (optimizing for bitmap font)
            temp_preprocessed="$SEARCH_DIR/${FILE_PREFIX}${i}_preprocessed.png"
            echo "  Preprocessing image for better OCR..."
            convert "$png_file" -colorspace Gray -normalize -level 10%,90% "$temp_preprocessed"
            
            # Perform OCR on preprocessed image
            echo "  Performing OCR..."
            tesseract -l "$OCR_LANG" --psm 6 "$temp_preprocessed" "$SEARCH_DIR/${FILE_PREFIX}${i}" txt
            
            # Clean up temporary preprocessed file
            rm -f "$temp_preprocessed"
            
            # Check if OCR result file was created successfully
            if [ -f "$txt_file" ]; then
                echo "  Successfully created: $txt_file"
            else
                echo "  Warning: OCR failed for $png_file, no text file created"
            fi
        else
            echo "Skipping: $png_file (TXT file already exists)"
        fi
    else
        # PNG file doesn't exist, skip
        continue
    fi
done

echo "Processing completed successfully!"