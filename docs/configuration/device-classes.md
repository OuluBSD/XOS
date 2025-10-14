# Device Classes in XOS

## Overview
XOS implements a comprehensive device management system organized into device classes. Each device class represents a category of devices with similar characteristics and behaviors. This classification system provides a consistent interface for applications while allowing specialized functionality for different device types.

## Device Class Architecture

### Class Hierarchy
Device classes form a hierarchical structure:
- **Abstract Classes**: Define common interfaces
- **Concrete Classes**: Implement specific device types
- **Specialized Classes**: Provide enhanced functionality for specific use cases

### Class Characteristics
Each device class defines:
- Common characteristics shared by all devices in the class
- Default values for characteristics
- Valid ranges and constraints
- Behavioral specifications
- Interface contracts

## Standard Device Classes

### SYSTEM Class
Represents system-level resources and global characteristics:
- System memory management
- Processor resources
- Global system parameters
- Overall system state

Key Characteristics:
- **ALMLIMIT** - Alarm limit for system alerts
- **AVAILMEM** - Available memory in system
- **COUNTRY** - Country code for localization
- **DEBUG** - Debug mode flag
- **FPUENB** - Floating Point Unit enabled flag
- **INITIAL** - Initial system startup flag
- **LOGIN** - Login status
- **NUMFLPY** - Number of floppy drives
- **NUMHARD** - Number of hard drives
- **NUMPAR** - Number of parallel ports
- **NUMSER** - Number of serial ports
- **SYSNAME** - System name
- **XOSVER** - XOS version number

### PROCESS Class
Manages process-related information and resources:
- Process scheduling parameters
- Memory allocation limits
- Process privileges
- Resource usage tracking

Key Characteristics:
- **CONTRM** - Console termination status
- **COUNTRY** - Process country code
- **FPUENB** - Floating Point Unit enablement
- **LABLKS** - Large block size
- **LAINUSE** - Large memory in use
- **LALARGE** - Large memory size
- **NAME** - Process name
- **NUM** - Process number
- **PRIV** - Process privileges
- **PRIVAVL** - Privileges available
- **REALBASE** - Real mode base address
- **REALSIZE** - Real mode memory size
- **RMALLOW** - Real mode memory allowance
- **RMINUSE** - Real mode memory in use
- **RMLIMIT** - Real mode memory limit
- **WSALLOW** - Workspace memory allowance
- **WSINUSE** - Workspace memory in use
- **WSLIMIT** - Workspace memory limit

### DISK Class
Represents mass storage devices:
- Hard disks
- Floppy disks
- Optical drives
- Removable media devices

Key Characteristics:
- **AVAIL** - Available storage space
- **CBLKSZ** - Cluster block size
- **CCYLNS** - Cylinder count
- **CHEADS** - Head count
- **CLSSZ** - Cluster size
- **CLUS TERS** - Cluster count
- **CSECTS** - Sector count
- **DBLKSZ** - Data block size
- **DCYLNS** - Data cylinder count
- **DHEADS** - Data head count
- **DOSNAME** - DOS name for compatibility
- **DSECTS** - Data sector count
- **FSTYPE** - File system type
- **MODEL** - Disk model
- **PARTN** - Partition number
- **PROTECT** - Protection status
- **REMOVE** - Removable media flag
- **SERIALNO** - Serial number
- **VOLLABEL** - Volume label
- **VOLNAME** - Volume name

### SPL Class (Spooling)
Handles spooling operations for printers and similar devices:
- Print job queuing
- Output formatting
- Device scheduling
- Status reporting

Key Characteristics:
- **CLSMSG** - Close message
- **CLSNAME** - Close name
- **CLSTIME** - Close time
- **SEQNUM** - Sequence number
- **SPLSPEC** - Spooling specification

### TRM Class (Terminal)
Manages terminal devices and console I/O:
- Character display and input
- Cursor positioning
- Screen formatting
- Terminal characteristics

Key Characteristics:
- **BELLFREQ** - Bell frequency
- **BELLLEN** - Bell duration length
- **CHARIN** - Character input processing
- **CHAROUT** - Character output processing
- **CLASS** - Terminal class
- **CURFIX** - Cursor fix mode
- **INLBS** - Input line buffer size
- **INRBHELD** - Input bytes held count
- **INRBLOST** - Input bytes lost count
- **INRBPL** - Input raw buffer power loss handling
- **INRBS** - Input raw buffer size
- **INRBSL** - Input raw buffer size limit
- **IOUTFLOW** - Input output flow control
- **KBCHAR** - Keyboard character translation
- **KBTCHAR** - Keyboard termination character
- **OUT FLOW** - Output flow control
- **PASS WORD** - Access password
- **PRO GRAM** - Associated program
- **SCSVTIME** - Screen save time
- **SCSVTYPE** - Screen save type
- **SES SION** - Session information
- **TYPE** - Terminal type

### PCN Class (Pseudo Console)
Implements pseudo-terminal devices:
- Virtual terminal emulation
- Session management
- Remote terminal support
- Multiplexed terminal access

Key Characteristics:
- **CLASS** - Device class
- **INLBS** - Input line buffer size
- **INRBS** - Input raw buffer size
- **PASS WORD** - Access password
- **PRO GRAM** - Associated program
- **SES SION** - Session information
- **TYPE** - Device type

### IPM Class (Interprocess Message)
Facilitates interprocess messaging:
- Message queues
- Message passing
- Process synchronization
- Data exchange mechanisms

Key Characteristics:
- **LIMIT** - Message queue limit
- **MAXIMUM** - Maximum message size
- **NUMBER** - Queue number

### NULL Class
Provides null device functionality:
- Discards all output
- Returns no input
- Useful for testing and redirection

Key Characteristics:
- **CLASS** - Device class designation

### PPR Class (Paper)
Represents paper-based output devices:
- Printers
- Plotters
- Fax machines
- Other hardcopy devices

Key Characteristics:
- **CLASS** - Device class
- **INT** - Interrupt number
- **TIMEOUT** - Operation timeout value

### NET Class (Network)
Manages network interfaces and connections:
- Ethernet adapters
- Token ring adapters
- Wireless network interfaces
- Network protocol stacks

Key Characteristics:
- **BADPNT** - Bad packet count
- **BCPKTIN** - Broadcast packets received
- **BYTEIN** - Bytes received
- **BYTEOUT** - Bytes transmitted
- **ICRC** - CRC error count
- **IFRAME** - Frame error count
- **ILOST** - Lost packet count
- **INT** - Interrupt count
- **IOVRRN** - Overrun count
- **NETADDR** - Network address
- **NOBFR** - No buffer count
- **NODST** - No destination count
- **OCOL** - Collision count
- **OCSEN** - Carrier sense error count
- **OHTBT** - Heartbeat error count
- **OHUNG** - Hung transmission count
- **OOWC** - Out of window count
- **OUNDRN** - Underrun count
- **OXCOL** - Excessive collision count
- **PKTIN** - Packets received
- **PKTOUT** - Packets transmitted

### SNAP Class (Snapshot)
Provides snapshot and checkpoint functionality:
- System state capture
- Memory dumps
- Process snapshots
- Recovery mechanisms

Key Characteristics:
- **LIMIT** - Snapshot limit
- **MAXIMUM** - Maximum snapshot size
- **NUMBER** - Snapshot number

### ARP Class (Address Resolution Protocol)
Manages ARP table and address resolution:
- MAC address resolution
- IP address mapping
- ARP cache management
- Network topology discovery

Key Characteristics:
- **LIMIT** - ARP table limit
- **MAXIMUM** - Maximum entries
- **NUMBER** - Current entries

### IPS Class (Internet Packet Switching)
Handles IP packet switching operations:
- Packet routing
- Fragmentation and reassembly
- TTL management
- Header processing

Key Characteristics:
- **LIMIT** - Packet processing limit
- **MAXIMUM** - Maximum packet size
- **NUMBER** - Active packets

### UDP Class (User Datagram Protocol)
Implements UDP protocol services:
- Connectionless datagram service
- Port management
- Checksum processing
- Broadcast support

Key Characteristics:
- **BADHDR** - Bad header count
- **BYTEIN** - Bytes received
- **BYTEOUT** - Bytes transmitted
- **CHKSUM** - Checksum error count
- **HOSTDOWN** - Host down count
- **IBLXCD** - Input buffer exceeded count
- **IPPROT** - IP protocol error count
- **IPSDEV** - IP service device count
- **NAMESRVR** - Name server error count
- **NODST** - No destination count
- **PKTIN** - Packets received
- **PKTOUT** - Packets transmitted
- **PSLTDL** - Packet size too large for destination
- **PSLTMN** - Packet size too small minimum
- **RTREMOVE** - Route remove count

### TCP Class (Transmission Control Protocol)
Implements TCP protocol services:
- Connection-oriented byte stream service
- Flow control
- Error detection and correction
- Congestion control

Key Characteristics:
- **BADHDR** - Bad header count
- **BYTEIN** - Bytes received
- **BYTEOUT** - Bytes transmitted
- **CHKSUM** - Checksum error count
- **CLOST** - Connection lost count
- **FLOWOVR** - Flow control overrun count
- **MERGED** - Merged packet count
- **NOACK** - No acknowledgment count
- **NODST** - No destination count
- **OOSMAX** - Out of sequence maximum count
- **OOSMRGD** - Out of sequence merged count
- **OOSNUM** - Out of sequence number count
- **OUTSEQ** - Out of sequence count
- **OUTWIN** - Out of window count
- **PKTIN** - Packets received
- **PKTOUT** - Packets transmitted
- **PSLTHL** - Packet size too large for host
- **PSLTMN** - Packet size too small minimum
- **RETRY1** - First retry count
- **RETRY2** - Second retry count
- **REXMIT** - Retransmit count
- **RSTRCVD** - Reset received count
- **RSTSENT** - Reset sent count
- **UNXFIN** - Unexpected finish count

### TLN Class (Telnet)
Provides telnet protocol services:
- Remote terminal access
- Protocol negotiation
- Terminal type handling
- Character set conversion

Key Characteristics:
- **LIMIT** - Telnet session limit
- **MAXIMUM** - Maximum session count
- **NUMBER** - Active session count

### XFP Class (XOS File Protocol)
Implements XOS-specific file transfer protocol:
- Native file operations over network
- Authentication and security
- File attribute preservation
- Efficient transfer mechanisms

Key Characteristics:
- **LIMIT** - Connection limit
- **MAXIMUM** - Maximum connections
- **NUMBER** - Active connections

## Device Class Relationships

### Inheritance Hierarchy
Some device classes inherit characteristics from others:
- Terminal classes share common characteristics
- Network classes have common base features
- Storage classes share file system characteristics
- Communication classes inherit data transfer properties

### Composition Patterns
Complex devices may combine multiple class characteristics:
- Network terminals combine NET and TRM classes
- Spooled printers combine SPL and PPR classes
- Pseudo-terminals combine PCN and TRM classes

## Device Class Management

### Class Registration
Device classes are registered with the system:
- Class identifiers
- Characteristic definitions
- Default implementations
- Interface specifications

### Runtime Management
Classes are managed at runtime:
- Instance creation and destruction
- Resource allocation and deallocation
- Characteristic modification
- Status monitoring and reporting

## Device Characteristic System

### Characteristic Types
Characteristics can be:
- **Numeric**: Integer or floating-point values
- **String**: Text values with length limits
- **Boolean**: True/false or on/off values
- **Enumerated**: Predefined set of values
- **Binary**: Raw data or bit fields

### Characteristic Access
Characteristics are accessed through:
- **svcIoDevParm** - Get or set device parameters
- **Direct inquiry** - Read-only access methods
- **Configuration utilities** - Administrative tools
- **Runtime modification** - Dynamic adjustment

### Characteristic Validation
Validation mechanisms ensure:
- Value ranges are respected
- Data types are correct
- Dependencies are maintained
- Security constraints are enforced

## Class-Specific Behaviors

### Method Dispatching
Each class implements specific methods:
- Open/close operations
- Read/write operations
- Control functions
- Status reporting
- Error handling

### Event Handling
Classes handle specific events:
- Device insertion/removal
- Media change events
- Network connection/disconnection
- Timeout conditions
- Error conditions

## Extensibility Mechanisms

### Custom Device Classes
Developers can create custom classes:
- Define new characteristic sets
- Implement specialized behaviors
- Register with the system
- Integrate with existing infrastructure

### Class Enhancement
Existing classes can be enhanced:
- Add new characteristics
- Modify default behaviors
- Extend interface contracts
- Improve performance characteristics

## Performance Considerations

### Class Overhead
Efficiency considerations:
- Minimize characteristic lookup overhead
- Cache frequently accessed characteristics
- Optimize method dispatch mechanisms
- Reduce memory footprint for class metadata

### Scalability Factors
Scaling considerations:
- Number of class instances
- Characteristic access frequency
- Method invocation rates
- Resource allocation patterns

## Security Model

### Access Control
Classes implement access control:
- Privilege checking for sensitive operations
- Authentication for protected characteristics
- Authorization for administrative functions
- Audit logging for security-relevant events

### Data Protection
Classes protect sensitive data:
- Encrypt stored characteristics when appropriate
- Validate input to prevent injection attacks
- Limit exposure of internal state
- Sanitize output to prevent information leakage

## Error Handling and Recovery

### Class-Specific Errors
Each class defines relevant errors:
- Characteristic-specific error codes
- Operation-specific failure conditions
- Recovery procedures for transient errors
- Fallback behaviors for permanent failures

### Fault Tolerance
Classes implement fault tolerance:
- Graceful degradation under stress
- Automatic recovery from transient failures
- Isolation of faulty components
- Notification of system management

## Monitoring and Diagnostics

### Performance Metrics
Classes provide metrics for:
- Resource utilization
- Operation throughput
- Error rates and patterns
- Response times

### Diagnostic Capabilities
Diagnostic features include:
- Detailed status reporting
- Configuration verification
- Performance analysis tools
- Troubleshooting utilities

## Best Practices

### Class Design
Recommendations for class design:
- Follow established patterns and conventions
- Provide comprehensive characteristic sets
- Implement robust error handling
- Document interfaces and behaviors clearly
- Ensure backward compatibility

### Implementation Guidelines
Implementation recommendations:
- Use efficient data structures
- Minimize system call overhead
- Handle concurrency appropriately
- Provide adequate error recovery
- Optimize for common usage patterns

### Testing Strategies
Testing approaches:
- Unit test individual characteristics
- Integration test class interactions
- Stress test scalability limits
- Validate security constraints
- Verify error handling paths