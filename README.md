# XOS
XOS is a multi-tasking operating system for PCs which implements the x86 32-bit architecture. It provides many features found in Unix and VMS. It has a relatively small footprint but still provides a fully protected multi-tasking environment. The basic system is command line oriented and the tools needed to implement a graphical user interface are provided.

![](Startup.webp)

It has been used in a number of embedded/dedicated applications and can also be used as a desktop system.

XOS itself is in the public domain. There are no restrictions on use, including commercial use. The XOS distribution includes several freeware packages that have some restrictions on redistribution, mainly requiring including copyright notices and crediting the original authors. To the best of our knowledge, all of these allow use for private or commercial purposes and free redistribution with minimal conditions. None of the software provided here is licensed under the GPL or LGPL.

The goal for XOS has been to provide maximum practical functionality while remaining understandable, simple, secure, and efficient. (Probably "secure" should be listed first, but we believe that a system cannot be secure if it is not understandable. To be understandable it must be reasonably simple.) 


## Overview

XOS is a general purpose protected mode operating system for the x86 32-bit PC architecture (also referred to as IA-32). XOS was originally written when the only available x86 32-bit PCs had a 16MHz 80386 CPU and, at most, 1MB of RAM, and was designed to be usable in that environment. To keep it small and fast, the kernel was written in assembler, although most of the associated user mode programs were written in C. The available execution environment has changed quite a bit since then and XOS has been expanded and we have added new features such as additional CPU power and memory have become available. It still has a strong focus on efficiency and has NOT grown uncontrollably as have most other systems.

The current version of XOS can be run in as little as 2MB (yes, that's MB) of RAM, although performance, especially with modern large disks, is much better with at least 10MB. XOS does use a disk cache and, with large disks, this should be on the order of 10 to 20MB. XOS will use up to 4GB (minus the hardware space). XOS does not support general virtual memory, mainly because it has never been needed for the applications where XOS has been used. It supports "virtual allocation", which is mainly used to allocate stack space automatically.

Most XOS programs are small. Even the ported OpenWatcom C compiler loads in about 750KB. Real graphics mode applications run in a few MB.

XOS provides a flexible method for loading extensions to the kernel which are known as LKEs (Loadable Kernel Extensions). This includes device drivers but is not limited to that. Most device drivers are implemented as LKEs. A mechanism is provided for loading LKEs into a temporary RAM-disk at boot time using the bootstrap before switching to protected mode. This allows even the drivers for the boot disk to be loaded as LKEs.

XOS provides a flexible system for allocating memory, allowing multiple discontiguous blocks of allocated memory.

XOS provides an advanced signal (as the term is used with Unix) capability which provides multiple priority levels. It is patterned after a multi-level hardware interrupt system and queues signals until all higher priority signals have been delivered and dismissed.

XOS supports multiple execution threads in a process, both "heavy weight" preemptively scheduled threads and "light weight" non-preemptively scheduled threads (sometimes called fibers).

XOS supports both blocking and non-blocking IO for most devices. The only exceptions are the console display (which is CPU bound) and the sound (HDA) device which uses a different continuous stream API.

Only the flat memory model is supported. 

A VGALib style graphics library is included, which can be used to create programs using a graphical user interface. This includes support for PCL3GUI "win-printers". The intent is eventually to use this library to implement a graphics server that will implement a GUI desktop.

USB 2.0 is supported. There are low level drivers for the EHCI, UHCI, and OHCI controllers. There are high level drivers for mass storage devices (bulk transfer only protocol), keyboards, and mice, and there is a general purpose bulk transfer/interrupt driver that has been used with a LabJack and other custom devices. XOS can be booted and run from a USB disk.

The HDA sound interface is supported.

Multiple pre-emptively scheduled execution threads per process are supported.

## Documentation

See docs folder for slightly outdated v3.2 documentation pdf.


## FAQ

#### What is XOS good for?

XOS is targeted at various embedded or dedicated applications. It provides a full featured multitasking environment which has a very small footprint. It is a relitively simple system which can reasonably be well enough understood to provide assurance that the system is really doing what it is intended to do and nothing else. The fact that it provides a non-standard execution environment is actually a major advantage in many dedicated applications. For example, if a PC is used as a POS (point of sale) terminal, you do not want games and other programs downloaded and run on that machine.

#### Do we really need yet another OS?

We think so. We believe that most available operating systems have increased in complexity to the point that they are not really understandable by any one person. We believe this level of understanding is necessary if a system is to be really secure. The goal of XOS development is to "Keep It Simple" while still providing reasonable functionality.

#### How is XOS different from Linux?

One word: simplicity. Over the past few years the standard Linux distributions have grown more and more complex, making it progressively more difficult to use Linux as the basis for a system that can be fully understood. We believe this unnecessary complexity makes it very difficult if not impossible to create a really secure system using Linux. While XOS lacks some of the more esoteric capabilities of Linux, we believe it is a better choice for those applications it can support.

#### What hardware does XOS support?

XOS should run on any "standard" x86 PC with a Pentinum class processor and at least 2 to 4MB (yes, that's MB) of RAM. Use of graphics mode programs requires that the console display support the version 3.0 real mode VESA BIOS functions. XOS's sound support requires HDA audio. We have tried XOS on numerous PCs with almost complete success. We have had problems with some different (not RTL888) HDA codecs and have found at least one several year old motherboard whose OHCI fails at startup. We have not found any Pentium class CPU chips that don't work with XOS. (As far as we know, it's never been tried on a Transmeta chip, but that's probably not very important now!)

#### Does XOS support "legacy free" PCs?

Yes. XOS has been run extensively on an Asus EeeBox, which has no legacy devices except for an emulated IDE controller (which XOS requires for hard disk access).

#### Does XOS support multiple CPUs?

No, XOS does not currently support multiple CPUs. We have felt that, given the intended uses for XOS, this is not worth the resources that would be required to implement. However, we may look at this again at some point. Fortunately, all of the current multi-core chips work fine as a single CPU chip and thus work fine with XOS in that mode.

#### Is there a 64-bit version of XOS?

No, there is no 64-bit version of XOS. We do see an eventual need for this and hope to provide it at some point. Unfortunately, our choice of OpenWatcom as the XOS C compilier creates problems here since OpenWatcom can not generate 64-bit code and so far OpenWatcom has not committed to adding this. We will have to: 1) Wait for OpenWatcom, 2) Upgrade the OpenWatcom compilier ourselves, or 3) Find another 64-bit C compiler (maybe gcc?).

#### Why is there so much assembler code in XOS?

XOS was originally written when the only 32-bit x86 CPU in existence was a 16MHz 80386 and a "large" system had 1MB of RAM! Given this, it was felt that efficiency was extremely important. Thus all kernel level code was written in 80386 assembler. The situation today is quite different and we are currently in the process of rewriting virtually all of the assmebler code in C. Providing an exec mode execution enviroment that would support code written in C was one of the main changes between versions 3 and 4. This project is on the order of 15 to 20% complete and is continuing.

#### Does XOS support a C++ compiler?

Not at present. We have not needed one for the projects we are currently working on so have not taken the time to port the OpenWatcom C++ compiler. Given that it is written in C and runs in the same environment as the C compiler, this should be a reasonably simple task.

#### Is XOS really free?

Yes. All XOS code and documentation is in the public domain and can be freely downloaded and used however you want as long as you agree not to hold the authors responsible for what you do with it. You should note that XOS is NOT intended for life-critical applications. Some features of XOS (mainly graphic mode support) use external freeware packages which have somewhat more restrictive terms. None of them, however, are under the GPL or any other license that makes any "contamination" claims. Please see the Legal page for details.

#### Is support available for commercial use of XOS?

There is currently no formal support program for XOS. However, if you are considering XOS for a commercial application and would be interested in some level of paid support, please contact us. 



## Bugs

This is a list of the known bugs in XOS version 4.5.0. In general this only includes outright bugs. It does not include features that are simply not implmented or not complete. With the major changes between version 3 and 4, there are quite a few such incomplete features due to modules that simply have not been fully updated from version 3.

 Please report any other bugs observed or any additional information about reproducing any of the bugs listed here by creating new issue in github.
 
#### Bootstrap problems:

If the file specified to the bootstrap is not found the bootstrap must be restarted (with ctl-alt-DEL) before it will be able to load a different file.

#### USB related problems:

Failures (IO errors and system crashes) have occasionally been observed associated with the OHCI USB controller. Any information on reproducing this would be appreciated.

The system may crash if a USB disk is accessed while it is being mounted.

#### Keyboard and command shell related problems:

Type-ahead of non-ASCII keys of more than one key press is lost.

Editing (inserting and/or deleting) characters before the end of a line occasionally inserts extra spaces at the end of the line. Any information on reproducing this would be appreciated.

The command shell (XSHELL) can get into a state where all input appears to be ignored. Entering a local command (such as CLS) usually restores normal operation. Any information on reproducing this would be appreciated.

#### XWS (GUI) problems:

Keyboard input to edit windows has numerous problems. This is currently being worked on.

Scroll bars on windows do not work. This is also being worked on.