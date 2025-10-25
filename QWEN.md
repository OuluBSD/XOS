# XOS Documentation Project

Date: October 14, 2025

## Project Summary

Completed a comprehensive restructuring of the XOS documentation to improve accessibility and usability. The original documentation files were reorganized into a hierarchical structure with topic-specific markdown files extracted from the source material.

## Work Completed

1. **Documentation Restructuring**
   - Moved all original documentation files to `docs/orig/` for preservation
   - Created a hierarchical directory structure organized by topic areas
   - Generated `Index.md` with complete documentation outline
   - Created README.md navigation files in each directory

2. **Content Extraction**
   - Created 42 detailed markdown files covering key XOS topics:
     - System calls reference with detailed function descriptions
     - Device classes and characteristics
     - Process management
     - Memory management
     - I/O operations
     - Terminal functions
     - Screen symbiont operations
     - Network protocols
     - Error codes
     - Symbionts
     - Batch file commands
     - User operations
     - Programming interfaces
     - System architecture

3. **Git Integration**
   - Added all new documentation files to version control
   - Created commit with descriptive message

## Future Development

When planning or implementing new features for XOS, this newly organized documentation should be consulted first as it contains comprehensive information about:
- System call interfaces and parameters
- Device characteristics and management
- Process and memory management mechanisms
- I/O and terminal system operations
- Network protocols and implementation
- Error handling and recovery procedures

The documentation provides valuable context for understanding how different components of the system interact and can inform design decisions for new features.

## Task Management

We maintain a TASKS.md file that tracks both completed and pending tasks for the XOS project. When a task is completed, it is moved from the TODO section to the DONE section. This helps us track our progress and maintain focus on the most important objectives.

## Current Status: Building XOS for QEMU

### Understanding the Challenge

XOS uses a self-hosting build system that presents a bootstrapping challenge:

- **Native toolchain**: XOS is built using XMAC assembler, OWXC (Open Watcom C compiler port), and XLINK linker
- **Self-hosting**: These tools run on XOS itself, not on Linux
- **Dependency cycle**: To build XOS, you need XOS tools; to have XOS tools, you need to build them on XOS

### Available Assets

The source code contains evidence of GCC support:
- `xossvc.gcc` file provides GCC inline assembly definitions for XOS system calls
- Assembly code file `gcc.s` in the xmac directory
- This suggests work has been done toward GCC compilation support

### Pre-built Binaries Solution

We found and successfully utilized pre-built XOS binaries from the XOS project:
- Downloaded `xos-4.5.1-memstick.7z` (8GB bootable disk image)
- Extracted `xos_memstick.img`, which is a DOS/MBR boot sector image with FAT32 partition
- Updated QEMU startup scripts to use this image

### Work Completed Toward QEMU

1. **QEMU Environment Setup**
   - Created a startup script `start_xos.sh`
   - Created a 100MB raw disk image with FAT32 partition structure
   - Configured appropriate parameters for x86 32-bit, 32MB RAM

2. **Boot Process Understanding**
   - XOS uses a two-level boot process:
     - Level 1: Loads at 0x7C00 (boot sector)
     - Level 2: Loads from reserved blocks
   - Requires `BOOT.RUN` file with specific segment structure

3. **Serial Console Configuration**
   - Added TCP serial support to QEMU startup script
   - XOS can now be accessed via serial connection on port 4444
   - Connect using: `telnet localhost 4444`

4. **Network Support**
   - XOS includes network drivers for multiple Ethernet cards including Realtek 8139
   - Created `start_xos_network.sh` script with RTL8139 network card emulation
   - Network configuration uses QEMU's user networking mode

### Next Steps Required

To further enhance XOS in QEMU:

1. **Create ISO image** - Convert the disk image to ISO format for CD-ROM booting
2. **Test ISO boot** - Verify XOS can boot from ISO in QEMU
3. **Test network functionality** - Verify XOS network drivers work with QEMU's emulated hardware

## Development Loop for AI

- Edit files by mounting memstick image in userspace: bins/xos_memstick.img. Edit it directly. Copy file to/from sandbox for editing. Unmount before running.
- Run QEMU in one process/thread and OCR in another process/thread (separate terminals)
- Use timeout 120 for QEMU and timeout 60 for OCR, or wait longer if needed (for compilation)
- You must wait in steps in qwen-coder... like max 20 seconds... you must check for output files or something, and then keep going
- OCR process might take a long time, like 2 minutes. You must wait until OCR is finished, and check the results in qemu_ocr_output.txt
- Run by @./start_xos_vga.sh  and give it 1 minute. Run @./qemu_screenshot_ocr.sh to get the output
- If running the development loop script, make sure to remove @qemu_ocr_output.txt beforehand
- Kill the qemu and use the output
- Get your command to run by writing it to the end of xosusr/default/shellini.bat
- Use these commands to mount/unmount (no sudo required):
  - udisksctl loop-setup -f bins/xos_memstick.img
  - udisksctl mount -b /dev/loop5
  - udisksctl unmount -b /dev/loop5
  - udisksctl loop-delete -b /dev/loop5
- IMPORTANT: Do not use sudo with these commands, as udisksctl manages permissions automatically
- If you don't see the QEMU window, run commands with DISPLAY=:0
- You MUST use 2 threads/terminals: one for qemu and one for ocr
- You MUST use 2 threads: one for qemu and one for ocr. Run qemu in background and then run ocr from another thread.