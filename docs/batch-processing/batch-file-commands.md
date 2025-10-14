# Batch File Commands in XOS

## Overview
Batch files in XOS are scripts containing sequences of commands that are executed automatically. They provide a way to automate repetitive tasks, perform complex operations, and create reusable command sequences. Batch files use a `.bat` or `.cmd` extension and can include both built-in batch commands and external programs.

## Batch File Structure

### Basic Syntax
Batch files consist of:
- Commands, one per line
- Comments prefixed with `REM` or `::`
- Labels marked with `:labelname`
- Control structures for flow management
- Variable expansions using `%variable%` syntax

### Execution
Batch files are executed by:
- Typing the filename at the command prompt
- Using the `CALL` command to execute from within another batch file
- Double-clicking in graphical environments
- Being invoked by other programs or system events

## Batch File Commands

### : - Label Definition
Defines a label for use with `GOTO`:
```
:labelname
```
Labels are used as targets for conditional jumps and loops.

### @ - Command Echo Suppression
Suppresses echoing of the current command:
```
@command
```
When placed at the beginning of a batch file, suppresses echoing of all commands:
```
@ECHO OFF
```

### CALL - Execute Batch File
Executes another batch file and returns to the current batch file:
```
CALL batchfile [parameters]
```
Without `CALL`, control would not return to the calling batch file.

### ECHO - Display Messages and Control Echoing
Controls command echoing and displays messages:
```
ECHO [ON|OFF]
ECHO message
```
Options:
- `ON` - Enable command echoing
- `OFF` - Disable command echoing
- `message` - Display specified message

### FOR - Loop Processing
Processes files or items in a loop:
```
FOR %%variable IN (set) DO command
```
Variants:
- `FOR %%variable IN (fileset) DO command` - Process files
- `FOR /L %%variable IN (start,step,end) DO command` - Numeric loop
- `FOR /F %%variable IN (options) string DO command` - Parse strings

### GOTO - Jump to Label
Jumps to a labeled line in the batch file:
```
GOTO label
```
Use with caution as it can create unstructured code.

### IF - Conditional Execution
Performs conditional execution:
```
IF [NOT] condition command
```
Conditions:
- `ERRORLEVEL number` - Check exit code
- `EXIST filename` - Check if file exists
- `string1==string2` - String comparison
- `DEFINED variable` - Check if variable is defined

Extended syntax:
```
IF condition (
    command1
    command2
) ELSE (
    command3
    command4
)
```

### PAUSE - Wait for User Input
Suspends batch file execution until user presses a key:
```
PAUSE
```
Often used to view output before continuing.

### REM - Comments
Adds comments to batch files:
```
REM comment text
```
Alternative using double colons:
```
:: comment text
```

### RETURN - Exit Subroutine
Returns from a subroutine called with `CALL`:
```
RETURN
```
Note: In standard XOS batch files, `GOTO :EOF` is often used instead.

### SHIFT - Shift Command Line Parameters
Shifts command line parameters to the left:
```
SHIFT
```
Used to process variable numbers of parameters in loops.

### BATOPT - Batch File Options
Sets batch file processing options:
```
BATOPT option
```
Options control how the batch processor handles various situations.

## Control Structures

### Conditional Execution
Simple condition:
```
IF EXIST myfile.txt ECHO File exists
```

Complex condition with blocks:
```
IF "%1"=="install" (
    ECHO Installing...
    CALL install.bat
) ELSE (
    ECHO Usage: %0 install
)
```

### Loops
File processing loop:
```
FOR %%f IN (*.txt) DO (
    ECHO Processing %%f
    TYPE %%f
)
```

Numeric loop:
```
FOR /L %%i IN (1,1,10) DO (
    ECHO Number %%i
)
```

### Subroutines
Call subroutine:
```
CALL :mysubroutine param1 param2
GOTO :after_sub

:mysubroutine
ECHO Parameters: %1 %2
SHIFT
SHIFT
GOTO :EOF

:after_sub
```

## Variables and Parameter Expansion

### Command Line Parameters
- `%0` - Batch file name
- `%1` to `%9` - First nine parameters
- `%*` - All parameters

### Environment Variables
Access environment variables:
```
ECHO Current directory is %CD%
ECHO Path is %PATH%
```

### Variable Manipulation
Substring extraction:
```
%variable:~start,length%
```

Replace substring:
```
%variable:str=repl%
```

### Delayed Variable Expansion
Enable with:
```
SETLOCAL ENABLEDELAYEDEXPANSION
```
Then use:
```
!variable!
```
instead of `%variable%` for variables modified within loops.

## Built-in Variables

### Automatic Variables
- `%CD%` - Current directory
- `%DATE%` - Current date
- `%TIME%` - Current time
- `%RANDOM%` - Random number
- `%ERRORLEVEL%` - Last error code

### Path Variables
- `%PATH%` - Search path
- `%PATHEXT%` - Executable file extensions
- `%PROMPT%` - Command prompt

## File Operations in Batch Files

### Testing File Conditions
```
IF EXIST filename (
    ECHO File exists
) ELSE (
    ECHO File not found
)
```

### Directory Operations
```
IF NOT EXIST dirname\NUL MKDIR dirname
```
(Note: The `\NUL` tests for directory existence)

### File Comparisons
```
FC file1.txt file2.txt >NUL
IF ERRORLEVEL 1 (
    ECHO Files differ
) ELSE (
    ECHO Files are identical
)
```

## Error Handling

### Checking Error Levels
```
command
IF ERRORLEVEL 1 ECHO Command failed
```

### Specific Error Level Testing
```
command
IF ERRORLEVEL 255 ECHO Serious error occurred
IF ERRORLEVEL 1 IF NOT ERRORLEVEL 2 ECHO Minor error occurred
```

### Error Handling Best Practices
```
ECHO Performing critical operation...
critical_command.exe
IF ERRORLEVEL 1 (
    ECHO Critical operation failed!
    PAUSE
    EXIT /B 1
)
ECHO Critical operation succeeded.
```

## Advanced Features

### Input Redirection in Batch Files
Reading from file:
```
FOR /F "tokens=*" %%i IN (input.txt) DO (
    ECHO Line: %%i
)
```

### Output Redirection
Writing to file:
```
DIR > filelist.txt
ECHO Completed. >> logfile.txt
```

### Pipes in Batch Files
```
DIR | SORT > sorted_files.txt
TYPE *.txt | FIND "keyword"
```

### Combining Commands
Sequential execution:
```
command1 && command2
```
(command2 executes only if command1 succeeds)

Conditional execution:
```
command1 || command2
```
(command2 executes only if command1 fails)

Both commands:
```
command1 & command2
```
(both commands execute regardless of success/failure)

## Batch File Programming Techniques

### Modular Design
Break complex operations into subroutines:
```
:main
CALL :initialize
CALL :process_data
CALL :cleanup
GOTO :EOF

:initialize
REM Initialization code here
GOTO :EOF

:process_data
REM Data processing code here
GOTO :EOF

:cleanup
REM Cleanup code here
GOTO :EOF
```

### Parameter Validation
```
:check_parameters
IF "%1"=="" (
    ECHO Usage: %0 parameter
    EXIT /B 1
)
GOTO :EOF
```

### Logging
```
SET LOGFILE=operation.log
ECHO %DATE% %TIME% - Starting operation >> %LOGFILE%
```

## Best Practices

### Code Organization
1. Use meaningful labels and variable names
2. Comment complex sections
3. Structure code with consistent indentation
4. Handle errors appropriately
5. Document usage and parameters

### Performance Considerations
1. Minimize external command calls
2. Use built-in commands when possible
3. Batch file operations where appropriate
4. Avoid unnecessary directory changes
5. Use efficient loops and conditions

### Security Considerations
1. Validate all input parameters
2. Sanitize file paths to prevent directory traversal
3. Use appropriate permissions for batch files
4. Avoid hardcoding sensitive information
5. Consider using SETLOCAL to limit variable scope

### Debugging Techniques
1. Use `ECHO ON` for debugging output
2. Insert `PAUSE` statements at key points
3. Log intermediate results
4. Use `ECHO` to display variable values
5. Test with sample data before production use

## Common Batch File Patterns

### Menu System
```
:menu
CLS
ECHO 1. Option One
ECHO 2. Option Two
ECHO 3. Exit
CHOICE /C:123
IF ERRORLEVEL 3 GOTO :eof
IF ERRORLEVEL 2 GOTO :option_two
IF ERRORLEVEL 1 GOTO :option_one
```

### Backup Script Template
```
@ECHO OFF
SET BACKUP_DEST=D:\Backup
SET DATESTAMP=%DATE:~-4%-%DATE:~4,2%-%DATE:~7,2%

ECHO Creating backup...
XCOPY C:\Data\*.* %BACKUP_DEST%\%DATESTAMP%\ /S /Y
IF ERRORLEVEL 1 (
    ECHO Backup failed!
    EXIT /B 1
)
ECHO Backup completed successfully.
```

### Installation Script Template
```
@ECHO OFF
IF "%1"=="/silent" GOTO :silent_install

ECHO Installing Application...
:silent_install
COPY files\*.* C:\Program\
IF ERRORLEVEL 1 (
    ECHO Installation failed!
    EXIT /B 1
)
ECHO Installation completed.
```

## Compatibility Considerations

### Cross-Version Compatibility
- Test batch files on target XOS versions
- Use standard commands when possible
- Document any version-specific features used
- Consider using feature detection for optional functionality

### Integration with External Tools
- Handle return codes from external programs
- Account for differences in command-line syntax
- Validate tool availability before use
- Provide fallback options when tools are unavailable