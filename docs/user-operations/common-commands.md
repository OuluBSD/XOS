# Common XOS User Commands

## Overview
XOS provides a rich set of user commands for system management, file operations, process control, and application execution. These commands form the foundation of user interaction with the XOS system.

## File Management Commands

### DIR - Directory Listing
Displays the contents of directories:
```
DIR [filespec] [/options]
```
Options:
- `/B` - Brief format
- `/W` - Wide format
- `/S` - Include subdirectories
- `/A` - Show all files including hidden

### COPY - File Copy
Copies files from one location to another:
```
COPY source destination [/options]
```
Options:
- `/V` - Verify copied files
- `/Y` - Suppress overwrite prompts
- `/A` - ASCII mode copy
- `/B` - Binary mode copy

### DELETE/ERASE - File Deletion
Removes files from the system:
```
DELETE filespec [/options]
ERASE filespec [/options]
```
Options:
- `/Q` - Quiet mode (no confirmation)
- `/P` - Prompt before each deletion

### RENAME - File Renaming
Changes the name of files:
```
RENAME oldname newname
```

### ATTRIB - File Attributes
Displays or modifies file attributes:
```
ATTRIB [+R|-R] [+H|-H] [+S|-S] filespec
```
Attributes:
- `R` - Read-only
- `H` - Hidden
- `S` - System

## Directory Management Commands

### MKDIR/MD - Create Directory
Creates new directories:
```
MKDIR dirname
MD dirname
```

### RMDIR/RD - Remove Directory
Removes empty directories:
```
RMDIR dirname
RD dirname
```

### CHDIR/CD - Change Directory
Changes the current working directory:
```
CHDIR [dirname]
CD [dirname]
```

## System Information Commands

### VER - Version Information
Displays the XOS version:
```
VER
```

### VOL - Volume Information
Displays volume label and serial number:
```
VOL [drive:]
```

### DATE - Set/Display Date
Displays or sets the system date:
```
DATE [MM-DD-YYYY]
```

### TIME - Set/Display Time
Displays or sets the system time:
```
TIME [HH:MM:SS]
```

## Process Management Commands

### KILLPROC - Terminate Process
Forcefully terminates a running process:
```
KILLPROC process_id
```

### HISTORY - Command History
Displays previously executed commands:
```
HISTORY
```

## Text Processing Commands

### TYPE - Display File Contents
Displays the contents of text files:
```
TYPE filename
```

### MORE - Paginated Display
Displays file contents one screen at a time:
```
MORE filename
```
Or in pipeline:
```
command | MORE
```

### FIND - Search in Files
Searches for text within files:
```
FIND "string" filespec
```

## Environment Commands

### SET - Set Environment Variables
Sets environment variables for the current session:
```
SET variable=value
```

### PATH - Set Search Path
Defines directories to search for executable files:
```
PATH [path1;path2;...]
```

### PROMPT - Customize Command Prompt
Changes the command prompt appearance:
```
PROMPT [text]
```
Special codes:
- `$D` - Current date
- `$T` - Current time
- `$P` - Current drive and path
- `$N` - Current drive
- `$_` - Carriage return and linefeed

## Batch File Commands

### CALL - Execute Batch File
Calls another batch file from within a batch file:
```
CALL batchfile [parameters]
```

### ECHO - Display Messages
Displays messages or controls command echoing:
```
ECHO [ON|OFF]
ECHO message
```

### REM - Comments
Adds comments to batch files:
```
REM comment text
```
Or using `::`:
```
:: comment text
```

### GOTO - Jump to Label
Jumps to a labeled line in a batch file:
```
GOTO label
```

### IF - Conditional Execution
Performs conditional execution:
```
IF [NOT] condition command
```
Conditions:
- `ERRORLEVEL number` - Check exit code
- `EXIST filename` - Check if file exists
- `string1==string2` - String comparison

### FOR - Loop Processing
Processes files or items in a loop:
```
FOR %%variable IN (set) DO command
```

## I/O Redirection

### Input Redirection
Redirects input from a file:
```
command < inputfile
```

### Output Redirection
Redirects output to a file:
```
command > outputfile
command >> outputfile  (append)
```

### Pipeline
Connects output of one command to input of another:
```
command1 | command2
```

## Network Commands

### PING - Network Connectivity Test
Tests network connectivity to a host:
```
PING hostname
```

### NETLINK - Network Connection
Establishes network connections:
```
NETLINK parameters
```

## Device Management Commands

### MODE - Configure Devices
Configures system devices:
```
MODE device options
```

### DEVCHAR - Device Characteristics
Displays or sets device characteristics:
```
DEVCHAR device [characteristic=value]
```

## Disk Management Commands

### CHKDSK - Check Disk
Checks disk for errors and displays status:
```
CHKDSK [drive:] [/options]
```

### LABEL - Set Volume Label
Sets the volume label of a disk:
```
LABEL [drive:] [label]
```

### DISMOUNT - Unmount Disk
Unmounts removable media:
```
DISMOUNT drive:
```

## Utility Commands

### CLS - Clear Screen
Clears the terminal screen:
```
CLS
```

### ECHO - Display Messages
Displays messages or toggles command echoing:
```
ECHO [ON|OFF|message]
```

### ALIAS - Create Command Aliases
Creates shortcuts for commands:
```
ALIAS name=command
```

## Command Options and Syntax

### Common Options
Many XOS commands support these standard options:
- `/?` - Display help information
- `/H` - Display extended help
- `/Q` - Quiet mode (suppress output)
- `/V` - Verbose mode (detailed output)

### File Specifications
XOS supports flexible file specifications:
- Drive letters: `C:`, `D:`, etc.
- Directory paths: `\path\to\file`
- Wildcards: `*` (multiple characters), `?` (single character)
- Extensions: `.txt`, `.exe`, etc.

## Best Practices

### Command Usage
- Use tab completion for command and filename completion
- Use command history (up/down arrows) to recall previous commands
- Use `/?" option to get help for any command
- Combine commands with pipes and redirection for powerful operations

### File Operations
- Always verify file operations with `/V` option when available
- Use wildcards carefully to avoid unintended operations
- Backup important files before bulk operations
- Use relative paths when appropriate for portability

### System Maintenance
- Regularly check disk integrity with CHKDSK
- Keep system clean with periodic file cleanup
- Monitor system resources with appropriate commands
- Use batch files for repetitive operations

## Error Handling
Commands return error levels:
- `0` - Success
- Non-zero - Various error conditions
- Use `IF ERRORLEVEL` in batch files to check results