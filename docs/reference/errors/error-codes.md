# Error Codes in XOS

## Overview
XOS uses a standardized error code system to report the status of system operations. All system calls return error codes that indicate success, warnings, or failures. Understanding these error codes is essential for proper application development and system administration.

## Error Code Structure

### Return Value Conventions
- **0 (NOERR)** - Operation completed successfully
- **Positive values** - Warning conditions (operation completed with caveats)
- **Negative values** - Error conditions (operation failed)
- **Range-specific meanings** - Different ranges for different subsystems

### Error Code Categories
Error codes are organized into categories:
- System-level errors (-1 to -99)
- Device-level errors (-100 to -199)
- File system errors (-200 to -299)
- Memory management errors (-300 to -399)
- Process management errors (-400 to -499)
- Network errors (-500 to -599)
- I/O errors (-600 to -699)
- Terminal errors (-700 to -799)

## Standard Error Codes

### Success Codes
- `NOERR` (0) - No error (successful completion)

### General System Errors
- `ER_PARMF` (-1) - Parameter format error
- `ER_PARMI` (-2) - Parameter invalid
- `ER_PARMM` (-3) - Parameter missing
- `ER_PARMS` (-4) - Parameter syntax error
- `ER_PARMV` (-5) - Parameter value error
- `ER_PRIV` (-6) - Privilege violation
- `NOMEM` (-7) - Insufficient memory
- `DEVFUL` (-8) - Device table full
- `FILNF` (-9) - File not found
- `DIRNF` (-10) - Directory not found
- `BUSY` (-11) - Device or resource busy
- `CANCL` (-12) - Operation cancelled
- `FUNC` (-13) - Function not implemented
- `PARMF` (-14) - Parameter format error (alternative)
- `PARMI` (-15) - Parameter invalid (alternative)
- `PARMM` (-16) - Parameter missing (alternative)
- `PARMS` (-17) - Parameter syntax error (alternative)
- `PARMT` (-18) - Parameter type error
- `PARMV` (-19) - Parameter value error (alternative)
- `EOF` (-20) - End of file
- `ER_ROR` (-21) - General error
- `DEVFL` (-22) - Device failed
- `DFDEV` (-23) - Device function error
- `DEVER` (-24) - Device error
- `DIRFL` (-25) - Directory failed
- `DIRNE` (-26) - Directory not empty
- `DIRNF` (-27) - Directory not found (alternative)
- `DIRTD` (-28) - Directory too deep
- `DIVER` (-29) - Device driver error
- `DKCHG` (-30) - Disk changed
- `DKRMV` (-31) - Disk removed
- `DLOCK` (-32) - Device locked
- `DOSMC` (-33) - DOS memory conflict
- `DOSPB` (-34) - DOS parameter block error
- `DPMIC` (-35) - DPMI call failed
- `DQUOT` (-36) - Disk quota exceeded
- `DRFER` (-37) - Drive full error
- `DRRER` (-38) - Drive read error
- `DRWER` (-39) - Drive write error
- `DTINT` (-40) - Date/time invalid
- `DUADF` (-41) - Duplicate file
- `VALUE` (-42) - Value out of range
- `VECNS` (-43) - Vector not set
- `WLDNA` (-44) - Wildcard not allowed
- `WPRER` (-45) - Write protect error
- `WRTER` (-46) - Write error
- `XFRBK` (-47) - Transfer block error

### Memory Management Errors
- `ILLIN` (-300) - Illegal instruction
- `INCMO` (-301) - Incompatible mode
- `ININU` (-302) - Invalid number
- `INVST` (-303) - Invalid state
- `IOSAT` (-304) - I/O saturation
- `IPDIR` (-305) - Invalid process directory
- `ISDIR` (-306) - Is directory
- `LASNA` (-307) - Last available
- `LKEAL` (-308) - LKE already loaded
- `LOCK` (-309) - Lock violation
- `LSTER` (-310) - List error
- `MACFT` (-311) - Macro file type
- `MAERR` (-312) - Macro error
- `MATH` (-313) - Math error
- `MEMLX` (-314) - Memory limit exceeded
- `MPILK` (-315) - MPI link error
- `MSNPR` (-316) - Memory segment not present
- `NACT` (-317) - Not active
- `NCCLR` (-318) - No clear
- `NCLST` (-319) - No cluster
- `NCOMP` (-320) - Not compatible
- `NCONG` (-321) - No congestion
- `NCRFS` (-322) - No CRFS
- `NDOSD` (-323) - No DOS driver
- `NEMA` (-324) - No EMA
- `NHSTA` (-325) - No HSTA
- `NILAD` (-326) - Nil address
- `NILPC` (-327) - Nil PC
- `NILPR` (-328) - Nil process
- `NILRF` (-329) - Nil reference
- `NIYT` (-330) - Not in your time
- `NLKNA` (-331) - No lock available
- `NMBTS` (-332) - No more bytes
- `NNAVL` (-333) - Not available
- `NNOPC` (-334) - No operation
- `NNSER` (-335) - No such error
- `NNSNA` (-336) - No such name
- `NNSNC` (-337) - No such node
- `NNSRF` (-338) - No such reference
- `NNSRQ` (-339) - No such request
- `NNSRS` (-340) - No such resource
- `NOBUF` (-341) - No buffer
- `NODCB` (-342) - No DCB
- `NOERR` (-343) - No error (alternative)
- `NOIN` (-344) - No input
- `NOMEM` (-345) - No memory
- `NOOUT` (-346) - No output
- `NOPAP` (-347) - No paper
- `NORSP` (-348) - No response
- `NOSAD` (-349) - No such address
- `NOSTK` (-350) - No stack
- `NPCIU` (-351) - No PCI unit
- `NPERR` (-352) - No parity error
- `NPRIU` (-353) - No printer unit
- `NPRNO` (-354) - No process number
- `NRTER` (-355) - No router
- `NRTNA` (-356) - No route available
- `NSCLS` (-357) - No such class
- `NSDEV` (-358) - No such device
- `NSEGA` (-359) - No segment available
- `NSLP` (-360) - No sleep
- `NSNOD` (-361) - No such node
- `NSP` (-362) - No space
- `NSTYP` (-363) - No such type
- `NTDEF` (-364) - Not defined
- `NTDIR` (-365) - Not directory
- `NTDSK` (-366) - Not disk
- `NTFIL` (-367) - Not file
- `NTIMP` (-368) - Not implemented
- `NTLCL` (-369) - Not local
- `NTLNG` (-370) - Not long
- `NTRDY` (-371) - Not ready
- `NTTIM` (-372) - Not timed
- `NTTRM` (-373) - Not terminal

### Process Management Errors
- `PRIV` (-400) - Privilege error
- `RANGE` (-401) - Range error
- `RELTR` (-402) - Relative transfer
- `RNFER` (-403) - Reference error
- `SBFER` (-404) - Subscript format error
- `SBRER` (-405) - Subscript range error
- `SBWER` (-406) - Subscript warning error
- `SEKER` (-407) - Seek error
- `STKER` (-408) - Stack error
- `SVC` (-409) - Service error

### I/O Errors
- `FBFER` (-600) - File buffer format error
- `FBPER` (-601) - File buffer parameter error
- `FBRER` (-602) - File buffer range error
- `FBWER` (-603) - File buffer warning error
- `FILAD` (-604) - File address error
- `FILAF` (-605) - File attribute format error
- `FILCF` (-606) - File compare format error
- `FILEX` (-607) - File exists
- `FILNF` (-608) - File not found (alternative)
- `FILRF` (-609) - File reference error
- `FILXF` (-610) - File transfer format error
- `FSINC` (-611) - File system inconsistency
- `FTPER` (-612) - File transfer parameter error
- `FTRER` (-613) - File transfer range error
- `FTWER` (-614) - File transfer warning error
- `FUNC` (-615) - Function error

### Terminal Errors
- `TRMNA` (-700) - Terminal not available
- `UNXSI` (-701) - Unexpected signal

## Error Code Usage

### Checking for Errors
Applications should always check system call return codes:
```assembly
; Example of error checking
call svcIoOpen          ; Call system function
cmp ax, NOERR           ; Check for success
je  no_error            ; Branch if no error
; Handle error here
call svcSysErrMsg       ; Convert error to message
; Display or log error
```

### Converting Errors to Messages
Use `svcSysErrMsg` to convert error codes to readable messages:
```assembly
mov ax, error_code      ; Load error code
push ax                 ; Push error code
call svcSysErrMsg       ; Get error message
; AX now contains message pointer
```

### Error Recovery Strategies
Different error types require different recovery approaches:
- **Transient errors**: Retry operation after delay
- **Permanent errors**: Report to user and terminate operation
- **Resource errors**: Free resources and retry or fail gracefully
- **Parameter errors**: Fix parameters and retry
- **Privilege errors**: Request appropriate privileges or fail

## Error Reporting Best Practices

### Application-Level Error Handling
1. Log all errors with timestamp and context
2. Provide user-friendly error messages
3. Include technical details in logs for debugging
4. Attempt graceful degradation when possible
5. Fail securely to prevent data corruption

### System Administration
1. Monitor system logs for recurring errors
2. Correlate errors with system events
3. Track error patterns for troubleshooting
4. Use error information for capacity planning
5. Document common error causes and solutions

## Debugging with Error Codes

### Diagnostic Techniques
1. Enable verbose error reporting during development
2. Use debug versions of system calls when available
3. Monitor error logs for patterns
4. Correlate errors with system resource usage
5. Use profiling tools to identify error-prone code paths

### Development Tools
1. Error simulation for testing error handling
2. Memory debugging tools for memory-related errors
3. I/O tracing for device and file errors
4. Performance monitoring for resource errors
5. Privilege analysis tools for security errors

## Error Code Extensions

### Vendor-Specific Errors
Vendors may extend the error code system:
- Use positive values for vendor warnings
- Use negative values below -1000 for vendor errors
- Document extension ranges to avoid conflicts
- Follow standard error reporting conventions

### Future Expansion
The error code system is designed for expansion:
- Reserved ranges for future use
- Hierarchical organization by subsystem
- Consistent numbering schemes
- Backward compatibility maintenance

## Internationalization
Error messages can be localized:
- Use `svcSysErrMsg` with language parameters
- Support multiple character encodings
- Allow customization of error message format
- Provide translation databases for multilingual systems

## Performance Considerations

### Error Handling Overhead
Minimize performance impact of error handling:
- Optimize common success paths
- Use inline checking for critical operations
- Cache frequently used error messages
- Profile error handling code