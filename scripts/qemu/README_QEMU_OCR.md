# QEMU Screenshot OCR Script

This script captures a screenshot of a QEMU window and performs OCR to extract the text.

## Dependencies

You'll need the following tools installed:

- `xwininfo` - for finding the QEMU window
- `import` (from ImageMagick) - for taking screenshots
- `tesseract` - for OCR functionality

### Installing Dependencies

**For Gentoo:**
```bash
emerge -av imagemagick tesseract tesseract-data
```

**For Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install imagemagick tesseract-ocr tesseract-ocr-eng
```

**For Fedora:**
```bash
sudo dnf install ImageMagick tesseract tesseract-devel tesseract-langpack-eng
```

**For Arch:**
```bash
sudo pacman -S imagemagick tesseract tesseract-data-eng
```

## Usage

The script must be run in a graphical environment with X11 access.

```bash
./qemu_screenshot_ocr.sh [options]
```

### Options

- `-w, --window-name PATTERN` - Window name pattern to match (default: qemu)
- `-o, --output FILE` - Output image file (default: qemu_screenshot.png)
- `-l, --lang LANG` - OCR language (default: eng)
- `-d, --dry-run` - Show what would be done without executing
- `-h, --help` - Show help

### Examples

```bash
# Capture default QEMU window and perform OCR
./qemu_screenshot_ocr.sh

# Capture window with a specific name pattern
./qemu_screenshot_ocr.sh -w "My QEMU VM"

# Save screenshot to a specific file and perform OCR
./qemu_screenshot_ocr.sh -o my_screenshot.png

# Use a different OCR language (e.g., for German text)
./qemu_screenshot_ocr.sh -l deu
```

## Notes

1. The script looks for windows containing "qemu" in the title by default
2. Make sure your QEMU window is visible when running the script
3. If running over SSH, use `ssh -X` or `ssh -Y` to enable X11 forwarding
4. Supported OCR languages depend on installed tesseract language packs