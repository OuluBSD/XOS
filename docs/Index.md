# XOS Documentation Index

This index provides an organized overview of the XOS documentation set, which includes both the Programmer's Guide and User's Guide.

## Documentation Hierarchy

### 1. Introduction and Overview
- [XOS Programmer's Guide - Chapter 1: Introduction](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS User's Guide - Chapter 1: Introduction](orig/XOS User's guide v3.2-outline.html#outline)
- [XOS User's Guide - Chapter 2: Overview of XOS](orig/XOS User's guide v3.2-outline.html#outline)

### 2. System Architecture
- [XOS Programmer's Guide - Chapter 2: Structure of XOS](orig/XOS Programmer's guide v3.2-outline.html#outline)
  - User Processes
  - Scheduling
  - Memory Management and Allocation
  - Environment Strings
  - Devices
  - File Specifications
  - Wildcard File Specifications
  - Destination Wildcard File Specifications
  - Logical Names
  - Input/Output Operations
- [XOS Programmer's Guide - Chapter 3: The Programming Environment](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS Programmer's Guide - Chapter 4: Process Privileges](orig/XOS Programmer's guide v3.2-outline.html#outline)

### 3. System Programming
- [XOS Programmer's Guide - Chapter 5: The Signal System](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS Programmer's Guide - Chapter 6: Interprocess Communication](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS Programmer's Guide - Chapter 7: System Calls Overview](orig/XOS Programmer's guide v3.2-outline.html#outline)

#### 3.1 Utility Functions
- [XOS Programmer's Guide - Chapter 8: Utility Function System Calls](orig/XOS Programmer's guide v3.2-outline.html#outline)
  - svcSysCmos - CMOS Memory Function
  - svcSysDateTime - Date and Time Functions
  - svcSysDefEnv - Define Environment String
  - svcSysErrMsg - Get Error Message
  - svcSysFindEnv - Find Environment String
  - svcSysGetEnv - Get All Environment Strings
  - svcSysLoadLke - Load LKE
  - svcSysLog - Place Entry in System Log File

#### 3.2 Scheduler Functions
- [XOS Programmer's Guide - Chapter 9: Scheduler System Calls](orig/XOS Programmer's guide v3.2-outline.html#outline)
  - svcSchAlarm - Alarm Functions
  - svcSchClrEvent - Clear Event(s)
  - svcSchCtlCDone - Report ctl-C Processing Done
  - svcSchDismiss - Dismiss Signal
  - svcSchExit - Terminate Process
  - svcSchGetVector - Get Signal Vector
  - svcSchIntrProc - Interrupt Child Process
  - svcSchIRet - Return From Interrupt
  - svcSchKill - Terminate Any Process
  - svcSchMakEvent - Make Event Cluster
  - svcSchRelEvent - Release Event
  - svcSchResEvent - Reserve Event
  - svcSchSetEvent - Set Event(s)
  - svcSchSetLevel - Set Signal Level
  - svcSchSetVector - Set Signal Vector
  - svcSchSpawn - Create Child Process
  - svcSchSuspend - Suspend Process
  - svcSchWaitProc - Wait for Process to Terminate
  - svcSchWaitMEvent - Wait for Multiple Events
  - svcSchWaitSEvent - Wait for Single Event

#### 3.3 Memory Management
- [XOS Programmer's Guide - Chapter 10: Memory System Calls](orig/XOS Programmer's guide v3.2-outline.html#outline)
  - svcMemBlkAlloc - Allocate Linear Memory Block
  - svcMemBlkChange - Change Size of Linear Memory Block
  - svcMemBlkFree - Give up All Linear Memory Blocks
  - svcMemChange - Change Memory Allocation
  - svcMemConvShr - Convert to Shared Section
  - svcMemCopy2PM - Copy Data to Protected Mode Memory
  - svcMemCreate - Create New Segment
  - svcMemDebug - Memory Debug Functions
  - svcMemDescAlloc - Allocate Segment Descriptor
  - svcMemDescFind - Find Segment Descriptor
  - svcMemDescFree - Give Up Segment Descriptor
  - svcMemDescRead - Read Segment Descriptor
  - svcMemDescSet - Set Value in Segment Descriptor
  - svcMemDescWrite - Write Segment Descriptor
  - svcMemDosSetup - Set Up DOS Memory
  - svcMemLink - Link Segment Selectors
  - svcMemLinkShr - Link to Shared Section
  - svcMemMap - Map Physical Section
  - svcMemMove - Move Memory Section
  - svcMemNull - Map Null Memory
  - svcMemPageType - Change Memory Page Type
  - svcMemRemove - Remove Segment
  - svcMemRmvMult - Remove Multiple Segments
  - svcMemSegType - Change Segment Type
  - svcMemWPFunc - Watchpoint Functions
  - svcMemWPSet - Set Watchpoint

#### 3.4 I/O System
- [XOS Programmer's Guide - Chapter 11: I/O Parameters](orig/XOS Programmer's guide v3.2-outline.html#outline)
  - Common Parameters
  - Mass Storage Parameters
  - Terminal Parameters
  - Disk Parameters
  - Tape Parameters
  - Network Parameters
  - Interprocess Message Parameters
  - Datagram Parameters
  - svcIoRun Parameters
  - Device Class Parameter

- [XOS Programmer's Guide - Chapter 15: svcIoQueue System Call](orig/XOS Programmer's guide v3.2-outline.html#outline)
  - QAB Format
  - Summary of svcIoQueue Functions
  - QFNC_OPEN - Open Device or File
  - QFNC_DEVPARM - Device Parameters
  - QFNC_DEVCHAR - Device Characteristics Functions
  - QFNC_DELETE - Delete File
  - QFNC_RENAME - Rename File
  - QFNC_PATH - Path Functions
  - QFNC_CLASSFUNC - Class Functions
  - QFNC_INBLOCK - Input Block
  - QFNC_OUTBLOCK - Output Block
  - QFNC_OUTSTRING - Output String
  - QFNC_SPECIAL - Special Device Functions
  - QFNC_LABEL - Read or Write Volume Label
  - QFNC_COMMIT - Commit Data to Media
  - QFNC_CLOSE - Close File

- [XOS Programmer's Guide - Chapter 16: Input/Output System Calls](orig/XOS Programmer's guide v3.2-outline.html#outline)
  - svcIoCancel - Cancel I/O Request
  - svcIoClose - Close Device
  - svcIoControl - I/O Request Control
  - svcIoDefLog - Define Logical Name
  - svcIoDelete - Delete File
  - svcIoDevParm - Get or Set Device Parameters
  - svcIoDstName - Build Destination Name
  - svcIoDupHandle - Duplicate Device Handle
  - svcIoFindLog - Find Logical Name
  - svcIoInBlock - Input Block
  - svcIoInBlockP - Input Block/Parameter List
  - svcIoInSingle - Input Byte
  - svcIoInSingleP - Input Byte/Parameter List
  - svcIoOpen - Open Device or file
  - svcIoOutBlock - Output Block
  - svcIoOutBlockP - Output Block/Parameter List
  - svcIoOutSingle - Output Byte
  - svcIoOutSingleP - Output Byte/Parameter List
  - svcIoOutString - Output String
  - svcIoOutStringP - Output String/Parameter List
  - svcIoPath - Set Default Path
  - svcIoPorts - Control Access to I/O Ports
  - svcIoRename - Rename File
  - svcIoRun - Run or Load Program
  - svcIoSetPos - Set I/O Position
  - svcIoWait - Wait Until I/O is Complete

#### 3.5 Terminal System
- [XOS Programmer's Guide - Chapter 17: Terminal System Calls](orig/XOS Programmer's guide v3.2-outline.html#outline)
  - svcTrmAttrib - Get or Set Display Attributes
  - svcTrmCurPos - Get or Set Cursor Position
  - svcTrmCurType - Get or Set Cursor Type
  - svcTrmDspPage - Get or Set Current Display Page
  - svcTrmFunction - General Terminal Functions
  - svcTrmGetAtChr - Get Attribute and Character
  - svcTrmGCurCol - Set Graphic Cursor Colors
  - svcTrmGCurPat - Set Graphic Cursor Pattern
  - svcTrmGCurPos - Set Graphic Cursor Position
  - svcTrmLdStdFont - Load Standard Font
  - svcTrmLdCusFont - Load Custom Font
  - svcTrmMapScrn - Map Screen Buffer
  - svcTrmSelFont - Select Font
  - svcTrmSetAtChr - Set Attribute and Character
  - svcTrmSetChr - Set Character
  - svcTrmScroll - Scroll Window
  - svcTrmWrtInB - Write to Input Buffer

#### 3.6 Screen Symbiont System
- [XOS Programmer's Guide - Chapter 18: Screen Symbiont System Calls](orig/XOS Programmer's guide v3.2-outline.html#outline)
  - svcScnMapBufr - Map Physical Screen Buffer
  - svcScnMaskWrt - Masked Write to Screen Buffer
  - svcScnTrans - Transfter Data for Screen Symbiont
  - svcScnUtil - Screen Symbiont Utility Functions

#### 3.7 Device Characteristics
- [XOS Programmer's Guide - Chapter 12: Class Characteristics](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS Programmer's Guide - Chapter 13: Device Characteristics](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS Programmer's Guide - Chapter 14: Add-Unit Characteristics](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS Programmer's Guide - Chapter 19: Device Dependent I/O Functions](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS Programmer's Guide - Chapter 20: svcLoadLKE System Call](orig/XOS Programmer's guide v3.2-outline.html#outline)

### 4. User Operations
- [XOS User's Guide - Chapter 3: Command Reference](orig/XOS User's guide v3.2-outline.html#outline)
  - Common Options
  - ADDUNIT
  - ALIAS
  - ALIB
  - ALINK
  - AMAC
  - AMAKE
  - ATTRIB
  - BINCOM
  - CHDIR
  - CHKDSK
  - CLS
  - CLSCHAR
  - COLOR
  - CONFIG
  - COPY
  - COUNT
  - CRSHSAVE
  - DATE
  - DAYTIME
  - DEFAULT
  - DELETE
  - DEVCHAR
  - DIR
  - DISMOUNT
  - DISPLAY
  - DOSCOM
  - DOSDRIVE
  - DOSLPT
  - DUMP
  - DUMPLOG
  - ECHO
  - ERASE
  - EXE2RUN
  - FIND
  - GECKO
  - GENSYM
  - GETDSPTP
  - HISTORY
  - KILLPROC
  - LABEL
  - LKELOAD
  - LOGICAL
  - LPRT
  - MKBOOT
  - MKDIR
  - MODE
  - MORE
  - MOVE
  - NETLINK
  - NETMODEM
  - NETSHOW
  - OBJDMP
  - PATH
  - PING
  - PROMPT
  - RENAME
  - RMBOOT
  - RMDIR
  - RUN2EXE
  - RUNDMP
  - SET
  - SETENV
  - SHELL
  - SHOW
  - SYMBIONT
  - SYSCHAR
  - SYSDIS
  - TELNET
  - TIME
  - TOUCH
  - TYPE
  - VER
  - VOL

### 5. Batch Processing
- [XOS User's Guide - Chapter 4: Batch File Commands](orig/XOS User's guide v3.2-outline.html#outline)
  - : (label)
  - @
  - BATOPT
  - CALL
  - ECHO
  - FOR
  - GOTO
  - IF
  - PAUSE
  - REM
  - RETURN
  - SHIFT

### 6. System Configuration
- [XOS User's Guide - Chapter 5: Class Characteristics](orig/XOS User's guide v3.2-outline.html#outline)
- [XOS User's Guide - Chapter 6: Device Characteristics](orig/XOS User's guide v3.2-outline.html#outline)

### 7. System Components
- [XOS User's Guide - Chapter 7: Symbionts](orig/XOS User's guide v3.2-outline.html#outline)
  - BOOTSRV Symbiont
  - FTPSRV Symbiont
  - IPSSRV Symbiont
  - SCREEN Symbiont
  - TLNSRV Symbiont
  - UNSPOOL Symbiont

### 8. System Reference
- [XOS Programmer's Guide - Appendix A: List of System Calls](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS Programmer's Guide - Appendix B: System Error Codes](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS User's Guide - Chapter 8: System Error Messages](orig/XOS User's guide v3.2-outline.html#outline)
- [XOS User's Guide - Appendix A: Command Comparison](orig/XOS User's guide v3.2-outline.html#outline)
  - DOS to XOS Command Comparison
  - UNIX to XOS Command Comparison
  - VMS to XOS Command Comparison
- [XOS User's Guide - Appendix B: Technical Support](orig/XOS User's guide v3.2-outline.html#outline)

### 9. Index
- [XOS Programmer's Guide - Index](orig/XOS Programmer's guide v3.2-outline.html#outline)
- [XOS User's Guide - Index](orig/XOS User's guide v3.2-outline.html#outline)

## File Types in Documentation Set

### Original Documentation Files
- `XOS Programmer's guide v3.2-outline.html` - Outline of the programmer's guide
- `XOS Programmer's guide v3.2-html.html` - HTML version of the programmer's guide
- `XOS Programmer's guide v3.2.txt` - Text version of the programmer's guide
- `XOS Programmer's guide v3.2.pdf` - PDF version of the programmer's guide
- `XOS Programmer's guide v3.2[001-513].png` - Individual pages/images of the programmer's guide

- `XOS User's guide v3.2-outline.html` - Outline of the user's guide
- `XOS User's guide v3.2-html.html` - HTML version of the user's guide
- `XOS User's guide v3.2.txt` - Text version of the user's guide
- `XOS User's guide v3.2.pdf` - PDF version of the user's guide
- `XOS User's guide v3.2[001-305].png` - Individual pages/images of the user's guide

### Documentation Metadata
- `CONVERSION_SUMMARY.md` - Summary of the conversion process
- `converted_docs_metadata.json` - Metadata about the converted documents

## Navigation Notes

This documentation provides comprehensive coverage of the XOS system from both a user and programmer perspective. The Programmer's Guide focuses on system internals, APIs, and programming interfaces, while the User's Guide focuses on operational commands and usage.

For system administrators and power users, both guides are valuable. Programmers developing for XOS should focus on the Programmer's Guide, while general users will find the User's Guide more appropriate for daily operations.