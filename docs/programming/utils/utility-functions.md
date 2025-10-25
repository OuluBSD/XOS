# Utility Function System Calls in XOS

## Overview
XOS provides a variety of utility system calls that perform common system-level functions not directly related to memory management, I/O, or scheduling, but essential for system operation.

## CMOS Memory Functions

### svcSysCmos - CMOS Memory Function
- Access to CMOS (Complementary Metal-Oxide Semiconductor) memory
- Used for system configuration data and real-time clock
- Allows reading and writing to CMOS registers

## Date and Time Functions

### svcSysDateTime - Date and Time Functions
- Get and set system date and time
- Supports various date/time formats
- Time zone handling
- Access to real-time clock

## Environment String Functions

### svcSysDefEnv - Define Environment String
- Defines environment strings for processes
- Associates a name with a value in the environment
- Can be inherited by child processes

### svcSysFindEnv - Find Environment String
- Locates a specific environment string by name
- Returns the value associated with the name
- Can search through different environment scopes

### svcSysGetEnv - Get All Environment Strings
- Retrieves all environment strings for a process
- Returns complete environment data structure
- Used for process initialization and debugging

## Error Handling Functions

### svcSysErrMsg - Get Error Message
- Converts error codes to human-readable messages
- Supports multiple languages
- Provides detailed error information
- Used for error reporting and debugging

## System Logging Functions

### svcSysLog - Place Entry in System Log File
- Adds entries to the system log
- Supports different log levels (info, warning, error, debug)
- Timestamped entries
- Used for system monitoring and debugging

## Loadable Kernel Extensions

### svcSysLoadLke - Load LKE
- Dynamically load Loadable Kernel Extensions
- Extend kernel functionality without reboot
- Support for drivers and system services
- Unload functionality for system cleanup

## Implementation Notes
- All utility functions are synchronous unless otherwise specified
- Error codes are returned in standard format
- Most functions are available to processes with appropriate privileges
- Parameters are validated before function execution
- Functions may block if waiting for system resources

## Error Codes
Common error codes returned by utility functions:
- `ER_PRIV` - Insufficient privileges
- `ER_PARMV` - Invalid parameter value
- `ER_PARMF` - Invalid parameter format
- `NOERR` - Operation completed successfully
- `NOMEM` - Insufficient memory for operation
- `DEVFUL` - Device table full
- `FILNF` - File not found (for file-related utilities)