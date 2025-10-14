# XOS (X Operating System) - Development Context

## Project Overview

XOS is a multi-tasking operating system for PCs implementing the x86 32-bit architecture. It's designed to provide maximum practical functionality while remaining understandable, simple, secure, and efficient. The system has been in development since the era of 16MHz 80386 processors and has been continuously evolved to maintain efficiency without growing uncontrollably like other systems.

### Key Features
- x86 32-bit (IA-32) protected mode operating system
- Small footprint (runs in as little as 2MB RAM)
- Multi-threading support (both preemptive and non-preemptive threads)
- Advanced signal handling with multiple priority levels
- Loadable Kernel Extensions (LKEs) for device drivers and kernel extensions
- USB 2.0 support (EHCI, UHCI, OHCI controllers)
- HDA sound interface support
- VGA-style graphics library
- VGALib style graphics library for GUI programs
- Multiple execution models

### Architecture
- Kernel written primarily in assembly (m86 files) for efficiency
- User programs written in C
- Currently 15-20% complete project to convert assembly code to C
- Uses OpenWatcom C compiler (hence the `.xc`, `.uc`, `.oc` file extensions)
- Custom build system using XMAC (assembler) and XLINK (linker)

## Build System

XOS uses a custom build system based on:
- `XMAC` - Custom assembler for .m86 files
- `OWXC` - OpenWatcom C compiler for .xc/.uc/.oc files
- `XLINK` - Custom linker for creating executable files
- `XMAKE` - Custom make utility (similar to standard make)

### File Extensions
- `.m86` - Assembly source files
- `.xc` - C source files compiled in user context (exec mode)
- `.uc` - C source files compiled in user mode
- `.oc` - C source files compiled in other modes
- `.obj` - Object files
- `.run` - Executable files for XOS

### Build Process
The build system is controlled by `.mak` files (e.g., `makefile.mak` in kernel directory) and uses the following tools:
- `xmac` - Assembles m86 files
- `owxc` - Compiles C files (xc/uc/oc extensions)
- `xlink` - Links object files into executables
- `xmake` - Handles dependency tracking and builds

### Directory Structure
- `src/boot` - Bootloader code and utilities
- `src/kernel` - Core kernel in assembly (m86 files)
- `src/lke` - Loadable Kernel Extensions (device drivers)
- `src/libc` - C library implementation
- `src/libx` - XOS-specific library functions
- `src/xlib` - Extended library functions
- `src/xmac` - Macro assembler
- `src/xlink` - Linker
- `src/xmake` - Build system
- `src/xshell` - Command shell
- `src/owxc` - OpenWatcom C compiler interface
- `src/usbctl` - USB control and drivers
- `src/dsp` - Display and graphics
- `src/sound` - Audio support
- `src/netlke` - Network Loadable Kernel Extensions
- `src/netutil` - Network utilities
- `src/ftp` - FTP client/server

## Building and Running

### Development Tools Required
- OpenWatcom C/C++ compiler (OWXC)
- XOS-specific tools (XMAC, XLINK, XMAKE)

### Build Commands (inferred from makefiles)
```bash
# For kernel compilation
cd src/kernel
xmake all  # or run the corresponding makefile.mak

# Key executables built:
# - xos.run: Standard kernel executable
# - xost.run: Kernel with additional components
# - xosx.run: Extended kernel variant
```

### Build Configuration
The system is configured with version numbers:
- Major Version (MV): 4
- Minor Version (NV): 5
- Edit Number (EN): 1
- Vendor: SYSTEM

## Development Conventions

### Programming Languages
- Assembly (M86) for kernel-level, performance-critical code
- C (XC/UC/OC) for user and higher-level kernel code
- Structured data definitions in C header files (like xosdef.h)

### Code Organization
- Data structures defined in .h files
- Kernel functions are system calls (SVC) with defined interfaces
- Thread management through TDA (Task Data Area) structure
- Process management through PDA (Process Data Area) structure
- Modular design using Loadable Kernel Extensions (LKEs)

### Naming Conventions
- Assembly files: `.m86`
- C files: `.xc`, `.uc`, `.oc` depending on execution context
- Object files: `.obj`
- Executables: `.run` (XOS executable format)

## Testing and Debugging
- Debugging support exists with debugdisp component
- Signal handling for debugging and error conditions
- Watchpoint support for debugging

## Key Data Structures
- PDA (Process Data Area) - Process control information
- TDA (Task Data Area) - Thread control information
- TMR (Timer Data Block) - Timer management
- SLB (Signal Link Block) - Signal management
- Various blocks for I/O, memory management, etc.

## Project Status and Goals
- Currently converting assembly code to C (15-20% complete)
- Targeting embedded/dedicated applications
- Focus on simplicity, security, and understandability
- Not supporting virtual memory but has virtual allocation
- No multi-CPU support (single CPU only)
- No 64-bit version yet (limited by OpenWatcom compiler)

## Security and Stability
- Designed for security through understandability
- Not suitable for life-critical applications
- Focus on preventing external code execution in embedded scenarios
- Small footprint reduces attack surface

This project represents a unique approach to OS design, prioritizing simplicity and understandability over feature completeness.