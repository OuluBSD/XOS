# System Calls Reference in XOS

## Overview
XOS provides a comprehensive set of system calls that allow applications to interface with the operating system kernel. These system calls provide access to system services including process management, memory management, I/O operations, and interprocess communication.

## System Call Categories

### Utility Functions (svcSys*)
General system utility functions:
- `svcSysCmos` - CMOS memory function
- `svcSysDateTime` - Date and time functions
- `svcSysDefEnv` - Define environment string
- `svcSysErrMsg` - Get error message
- `svcSysFindEnv` - Find environment string
- `svcSysGetEnv` - Get all environment strings
- `svcSysLoadLke` - Load LKE (Loadable Kernel Extension)
- `svcSysLog` - Place entry in system log file

### Scheduler Functions (svcSch*)
Process and thread scheduling functions:
- `svcSchAlarm` - Alarm functions
- `svcSchClrEvent` - Clear event(s)
- `svcSchCtlCDone` - Report ctl-C processing done
- `svcSchDismiss` - Dismiss signal
- `svcSchExit` - Terminate process
- `svcSchGetVector` - Get signal vector
- `svcSchIntrProc` - Interrupt child process
- `svcSchIRet` - Return from interrupt
- `svcSchKill` - Terminate any process
- `svcSchMakEvent` - Make event cluster
- `svcSchRelEvent` - Release event
- `svcSchResEvent` - Reserve event
- `svcSchSetEvent` - Set event(s)
- `svcSchSetLevel` - Set signal level
- `svcSchSetVector` - Set signal vector
- `svcSchSpawn` - Create child process
- `svcSchSuspend` - Suspend process
- `svcSchWaitProc` - Wait for process to terminate
- `svcSchWaitMEvent` - Wait for multiple events
- `svcSchWaitSEvent` - Wait for single event

### Memory Management Functions (svcMem*)
Memory allocation and management functions:
- `svcMemBlkAlloc` - Allocate linear memory block
- `svcMemBlkChange` - Change size of linear memory block
- `svcMemBlkFree` - Give up all linear memory blocks
- `svcMemChange` - Change memory allocation
- `svcMemConvShr` - Convert to shared section
- `svcMemCopy2PM` - Copy data to protected mode memory
- `svcMemCreate` - Create new segment
- `svcMemDebug` - Memory debug functions
- `svcMemDescAlloc` - Allocate segment descriptor
- `svcMemDescFind` - Find segment descriptor
- `svcMemDescFree` - Give up segment descriptor
- `svcMemDescRead` - Read segment descriptor
- `svcMemDescSet` - Set value in segment descriptor
- `svcMemDescWrite` - Write segment descriptor
- `svcMemDosSetup` - Set up DOS memory
- `svcMemLink` - Link segment selectors
- `svcMemLinkShr` - Link to shared section
- `svcMemMap` - Map physical section
- `svcMemMove` - Move memory section
- `svcMemNull` - Map null memory
- `svcMemPageType` - Change memory page type
- `svcMemRemove` - Remove segment
- `svcMemRmvMult` - Remove multiple segments
- `svcMemSegType` - Change segment type
- `svcMemWPFunc` - Watchpoint functions
- `svcMemWPSet` - Set watchpoint

### I/O Functions (svcIo*)
Input/output and device management functions:
- `svcIoCancel` - Cancel I/O request
- `svcIoClose` - Close device
- `svcIoControl` - I/O request control
- `svcIoDefLog` - Define logical name
- `svcIoDelete` - Delete file
- `svcIoDevParm` - Get or set device parameters
- `svcIoDstName` - Build destination name
- `svcIoDupHandle` - Duplicate device handle
- `svcIoFindLog` - Find logical name
- `svcIoInBlock` - Input block
- `svcIoInBlockP` - Input block/parameter list
- `svcIoInSingle` - Input byte
- `svcIoInSingleP` - Input byte/parameter list
- `svcIoOpen` - Open device or file
- `svcIoOutBlock` - Output block
- `svcIoOutBlockP` - Output block/parameter list
- `svcIoOutSingle` - Output byte
- `svcIoOutSingleP` - Output byte/parameter list
- `svcIoOutString` - Output string
- `svcIoOutStringP` - Output string/parameter list
- `svcIoPath` - Set default path
- `svcIoPorts` - Control access to I/O ports
- `svcIoRename` - Rename file
- `svcIoRun` - Run or load program
- `svcIoSetPos` - Set I/O position
- `svcIoWait` - Wait until I/O is complete
- `svcIoQueue` - Queued I/O functions

### Terminal Functions (svcTrm*)
Terminal and display functions:
- `svcTrmAttrib` - Get or set display attributes
- `svcTrmCurPos` - Get or set cursor position
- `svcTrmCurType` - Get or set cursor type
- `svcTrmDspPage` - Get or set current display page
- `svcTrmFunction` - General terminal functions
- `svcTrmGetAtChr` - Get attribute and character
- `svcTrmGCurCol` - Set graphic cursor colors
- `svcTrmGCurPat` - Set graphic cursor pattern
- `svcTrmGCurPos` - Set graphic cursor position
- `svcTrmLdStdFont` - Load standard font
- `svcTrmLdCusFont` - Load custom font
- `svcTrmMapScrn` - Map screen buffer
- `svcTrmSelFont` - Select font
- `svcTrmSetAtChr` - Set attribute and character
- `svcTrmSetChr` - Set character
- `svcTrmScroll` - Scroll window
- `svcTrmWrtInB` - Write to input buffer

### Screen Symbiont Functions (svcScn*)
Screen symbiont functions:
- `svcScnMapBufr` - Map physical screen buffer
- `svcScnMaskWrt` - Masked write to screen buffer
- `svcScnTrans` - Transfer data for screen symbiont
- `svcScnUtil` - Screen symbiont utility functions

## System Call Invocation

### Calling Convention
System calls in XOS follow a standard calling convention:
1. Load parameters into appropriate registers
2. Load system call number into AX register
3. Execute INT instruction to invoke system call
4. Check return code in AX register
5. Retrieve results from registers or memory as specified

### Parameter Passing
Parameters can be passed in several ways:
- Register parameters for simple calls
- Memory-based parameter blocks for complex calls
- Parameter lists for variable argument functions
- Combination of registers and memory for optimal performance

### Return Values
All system calls return a standardized error code:
- `NOERR` (0) - Operation completed successfully
- Positive values - Warning conditions
- Negative values - Error conditions
- Special meaning codes for specific functions

## Error Handling

### Standard Error Codes
Common error codes returned by system calls:
- `NOERR` - No error (success)
- `ER_PARMF` - Parameter format error
- `ER_PARMI` - Parameter invalid
- `ER_PARMM` - Parameter missing
- `ER_PARMS` - Parameter syntax error
- `ER_PARMV` - Parameter value error
- `ER_PRIV` - Privilege violation
- `NOMEM` - Insufficient memory
- `DEVFUL` - Device table full
- `FILNF` - File not found
- `DIRNF` - Directory not found
- `BUSY` - Device or resource busy

### Error Reporting
Applications should:
1. Always check return codes from system calls
2. Use `svcSysErrMsg` to convert error codes to descriptive messages
3. Log errors appropriately for debugging
4. Handle recoverable errors gracefully
5. Terminate cleanly on unrecoverable errors

## Performance Considerations

### System Call Overhead
- System calls have inherent overhead due to context switching
- Minimize system calls in performance-critical code
- Batch operations when possible to reduce call frequency
- Use appropriate buffering to reduce I/O system calls

### Memory Management Efficiency
- Allocate memory in appropriately sized blocks
- Reuse memory blocks when possible
- Free memory promptly to avoid fragmentation
- Use shared memory for interprocess communication

### I/O Optimization
- Use buffered I/O for sequential operations
- Minimize disk seeks through proper file organization
- Use appropriate block sizes for device types
- Consider asynchronous I/O for improved performance

## Security Considerations

### Privilege Checking
- Many system calls require specific privileges
- Applications should check for sufficient privileges before calling
- Handle privilege violations gracefully
- Use least privilege principle when designing applications

### Resource Management
- Properly close files and devices
- Free allocated memory
- Clean up temporary resources
- Validate input parameters to prevent buffer overflows

## Debugging Support

### Memory Debugging
- Use `svcMemDebug` for memory leak detection
- Enable bounds checking during development
- Monitor memory usage statistics
- Use watchpoints for memory access monitoring

### System Logging
- Use `svcSysLog` for application logging
- Include appropriate detail levels
- Log significant events and errors
- Coordinate with system logging policies

## Best Practices

### System Call Usage
1. Always validate parameters before system calls
2. Check return codes and handle errors appropriately
3. Use system calls consistently with documented interfaces
4. Minimize system call frequency in performance-sensitive code
5. Follow established patterns for similar operations

### Resource Management
1. Acquire resources in a consistent order to prevent deadlocks
2. Release resources promptly in reverse acquisition order
3. Use RAII (Resource Acquisition Is Initialization) patterns
4. Handle resource exhaustion gracefully
5. Monitor resource usage for leaks

### Error Handling
1. Treat system call errors as exceptional conditions
2. Provide meaningful error messages to users
3. Log errors for diagnostic purposes
4. Recover from transient errors when possible
5. Fail securely on unrecoverable errors