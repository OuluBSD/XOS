# XOS Tasks

## TODO

1. **Create .iso image for XOS (optional)** - Once the hard drive image works, create an ISO image that can also be used with QEMU.

2. **Run XOS from .iso in QEMU (optional)** - Test running XOS from the ISO image in QEMU.

## DONE

1. **Find pre-built XOS binaries** - Located and downloaded pre-built XOS binaries from https://github.com/OuluBSD/XOS/releases including xos-4.5.1-memstick.7z (8GB bootable image) and xos_4_5_1.xid

2. **Extract and examine XOS image** - Successfully extracted xos_memstick.img from the 7z archive. The image is a DOS/MBR boot sector image with a partition suitable for running XOS.

3. **Update QEMU startup script** - Modified start_xos.sh to use the pre-built xos_memstick.img instead of the empty disk image.

4. **Test XOS boot in QEMU** - Confirmed that XOS can be started in QEMU using the pre-built disk image.

5. **Set up serial connection** - Configured serial communication for XOS in QEMU via TCP port 4444, allowing connection with telnet.

6. **Examine documentation files in docs/ directory** - Completed review of markdown files in docs directory to understand XOS architecture and components. XOS is a multitasking OS for x86 32-bit architecture with Unix/VMS-like features. Key components include kernel, LKEs, memory management, signals, I/O system, scheduler, etc.

7. **Understand XOS project** - XOS is a multi-tasking operating system for PCs implementing x86 32-bit architecture. It provides Unix and VMS-like features with a small footprint. It has a protected mode kernel written in assembler with user programs in C. The system can run in 2MB of RAM and provides multitasking, LKEs (Loadable Kernel Extensions), memory management, signals, threading, I/O support, and graphics capabilities.

8. **Analyze build system requirements** - Identified that XOS requires XMAC assembler, OWXC (Open Watcom C compiler port), and xlink linker that run on XOS itself, making cross-compilation from Linux challenging. However, there is evidence of GCC support with xossvc.gcc file.

9. **Set up QEMU environment** - Configured QEMU with appropriate settings for x86 32-bit architecture. Created a startup script (start_xos.sh) and a 100MB raw disk image with FAT32 partition.

10. **Set up TASKS.md file** - Created this file with TODO and DONE sections to track the work needed to get XOS running in QEMU.