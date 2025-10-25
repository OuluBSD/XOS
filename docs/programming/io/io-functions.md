# I/O Parameters and System Calls in XOS

## Overview
XOS provides comprehensive Input/Output services with various system calls and parameter structures. The I/O system handles communication with devices, files, and interprocess communication channels.

## I/O System Calls

### svcIoOpen - Open Device or File
- Opens a device or file for I/O operations
- Validates access permissions
- Creates I/O handle for subsequent operations
- Supports various open modes (read, write, read-write)

### svcIoClose - Close Device
- Closes an open I/O handle
- Releases associated resources
- Flushes pending writes
- Updates device status

### svcIoInBlock - Input Block
- Reads a block of data from device
- Blocks until data is available or error occurs
- Supports various buffer sizes
- Handles device-specific formatting

### svcIoOutBlock - Output Block
- Writes a block of data to device
- Blocks until data is written or error occurs
- Supports various buffer sizes
- Handles device-specific formatting

### svcIoInSingle - Input Byte
- Reads a single byte from device
- Immediate or blocking operation
- Used for character-by-character input
- Handles device buffering

### svcIoOutSingle - Output Byte
- Writes a single byte to device
- Immediate or blocking operation
- Used for character-by-character output
- Handles device buffering

### svcIoInBlockP - Input Block/Parameter List
- Reads block data with additional parameters
- Supports device-specific options
- Combined operation for efficiency
- Parameter passing in list format

### svcIoOutBlockP - Output Block/Parameter List
- Writes block data with additional parameters
- Supports device-specific options
- Combined operation for efficiency
- Parameter passing in list format

### svcIoInSingleP - Input Byte/Parameter List
- Reads single byte with additional parameters
- Supports device-specific options
- Parameter passing in list format

### svcIoOutSingleP - Output Byte/Parameter List
- Writes single byte with additional parameters
- Supports device-specific options
- Parameter passing in list format

### svcIoOutString - Output String
- Writes a string to device
- Handles string formatting and encoding
- Null-terminated string output
- Supports device-specific string handling

### svcIoOutStringP - Output String/Parameter List
- Writes string with additional parameters
- Combined string and parameter operation
- Supports device-specific string options

### svcIoControl - I/O Request Control
- Performs device-specific control operations
- Handles device configuration and status
- Supports various control functions
- Generic interface for device control

### svcIoWait - Wait Until I/O is Complete
- Waits for pending I/O operation to complete
- Returns status of completed operation
- Supports timeout values
- Can wait for multiple I/O handles

### svcIoCancel - Cancel I/O Request
- Cancels a pending I/O request
- Cleans up I/O resources
- Returns appropriate error code
- Handles cleanup properly

### svcIoPath - Set Default Path
- Sets default path for file operations
- Used for relative file specifications
- Supports multiple path contexts
- Handles path validation

### svcIoDevParm - Get or Set Device Parameters
- Gets or sets device-specific parameters
- Handles parameter validation
- Supports different parameter types
- Maintains device state

### svcIoDstName - Build Destination Name
- Constructs destination file/device name
- Handles wildcard expansion
- Supports path resolution
- Validates destination format

### svcIoDuplicateHandle - Duplicate Device Handle
- Creates duplicate of a device handle
- Allows multiple references to same resource
- Handles reference counting
- Used for handle sharing between processes

### svcIoFindLog - Find Logical Name
- Resolves logical name to actual device/file
- Handles logical name translation
- Supports nested logical names
- Manages logical name hierarchy

### svcIoDefLog - Define Logical Name
- Defines a logical name for device/file
- Creates logical name mapping
- Handles name validation and conflicts
- Supports logical name hierarchy

### svcIoDelete - Delete File
- Removes a file from the system
- Handles file protection and permissions
- Updates directory structure
- Handles file locking checks

### svcIoRename - Rename File
- Renames an existing file
- Validates new name format
- Updates directory structure
- Preserves file content and attributes

### svcIoSetPos - Set I/O Position
- Sets current position in file/device
- Supports absolute and relative positioning
- Validates position within file bounds
- Updates file/device position pointer

### svcIoRun - Run or Load Program
- Executes a program file
- Loads and starts new process
- Handles parameter passing
- Supports various execution modes

### svcIoPorts - Control Access to I/O Ports
- Controls hardware I/O port access
- Manages port permission levels
- Prevents unauthorized port access
- Used in protected mode operations

### svcIoQueue - Queued I/O Functions
- Handles complex queued I/O operations
- Supports various queue functions through QFNC codes
- Uses QAB (Queued Argument Block) structure
- Provides advanced I/O capabilities

## Common I/O Parameters

### I/O Operation Options (IOPAR)
- **IOPAR_FILOPTN** - File operation options
- **IOPAR_FILSPEC** - File specification
- **IOPAR_UNITSTS** - Unit status information
- **IOPAR_GLBID** - Global identifier
- **IOPAR_DEVSTS** - Device status

### Common I/O Options
- **O$CREATE** - Create file if doesn't exist
- **O$TRUNCA** - Truncate file on open
- **O$FAILEX** - Fail if file exists
- **O$REQFILE** - Require existing file
- **O$NOMOUNT** - Don't perform mount operation

### File Operation Options
- **O$APPEND** - Append to end of file
- **O$FAPPEND** - Force append mode
- **O$CONTIG** - Request contiguous allocation
- **O$CRIT** - Critical I/O operation
- **O$NODFWR** - No default wildcard replacement
- **O$NORDAH** - No read-ahead for performance
- **O$NOWCL** - No wildcard lookup
- **O$FHANDLE** - File handle operation
- **O$FNR** - Force new record
- **O$NOINH** - No inheritance to child processes
- **O$PARTIAL** - Partial I/O allowed
- **O$PHYS** - Physical I/O operation
- **O$RAW** - Raw I/O without formatting
- **O$XREAD** - Exclusive read access
- **O$XWRITE** - Exclusive write access

## Queued I/O System (QAB)

### QAB Structure Fields
- **qab_vector** - Address of completion vector
- **qab_error** - Error code field
- **qab_amount** - Amount of data processed
- **qab_count** - Counter field
- **qab_buffer1** - Primary buffer address
- **qab_buffer2** - Secondary buffer address
- **qab_handle** - I/O handle
- **qab_option** - Operation options
- **qab_parmlist** - Parameter list pointer

### QFNC Codes for svcIoQueue
- **QFNC_OPEN** - Open device or file
- **QFNC_DEVPARM** - Device parameters
- **QFNC_DEVCHAR** - Device characteristics
- **QFNC_DELETE** - Delete file
- **QFNC_RENAME** - Rename file
- **QFNC_PATH** - Path functions
- **QFNC_CLASSFUNC** - Class functions
- **QFNC_INBLOCK** - Input block
- **QFNC_OUTBLOCK** - Output block
- **QFNC_OUTSTRING** - Output string
- **QFNC_SPECIAL** - Special device functions
- **QFNC_LABEL** - Volume label operations
- **QFNC_COMMIT** - Commit data to media
- **QFNC_CLOSE** - Close file/device
- **QFNC$WAIT** - Wait function
- **QFNC$CHILDTERM** - Wait for child termination
- **QFNC$SAMEPROC** - Same process operation
- **QFNC$DIO** - Direct I/O operation

## I/O Parameter Categories

### Common Parameters
- Device names and file specifications
- Access modes and permissions
- I/O buffer specifications
- Error handling options

### Mass Storage Parameters
- Block sizes and allocation units
- File system parameters
- Storage device characteristics
- Volume management options

### Terminal Parameters
- Terminal type and capabilities
- Input/output modes
- Flow control settings
- Character translation options

### Disk Parameters
- Disk geometry information
- Sector and track parameters
- Partition information
- Disk access parameters

### Network Parameters
- Connection parameters
- Protocol options
- Buffer sizes
- Network addressing

### Device Class Parameters
- Generic device parameters
- Class-specific options
- Device identification
- Resource allocation

## Best Practices
- Always check return codes for error conditions
- Properly close all opened I/O handles
- Use appropriate buffering for performance
- Handle I/O timeouts appropriately
- Validate device names and file specifications
- Use logical names for portability
- Follow proper error handling procedures
- Consider security implications of I/O operations