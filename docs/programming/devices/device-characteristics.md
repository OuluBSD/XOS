# Device Characteristics in XOS

## Overview
XOS implements a sophisticated device management system with detailed device characteristics that define the behavior and capabilities of various device types. Devices are organized into classes with specific characteristics that control their operation.

## Device Organization

### Device Classes
Devices in XOS are organized into the following classes:
- **SYSTEM** - System-level devices and resources
- **PROCESS** - Process management devices
- **DISK** - Mass storage devices
- **SPL** - Spooling devices
- **TRM** - Terminal devices
- **PCN** - Pseudo-console devices
- **IPM** - Interprocess message devices
- **NULL** - Null devices
- **PPR** - Paper devices (printers)
- **NET** - Network devices
- **SNAP** - Snapshot devices
- **ARP** - Address Resolution Protocol devices
- **IPS** - Internet Packet Switching devices
- **UDP** - User Datagram Protocol devices
- **TCP** - Transmission Control Protocol devices
- **TLN** - Telnet devices
- **XFP** - XOS File Protocol devices

## Common Device Characteristics

### Basic Characteristics
Most device classes support these fundamental characteristics:
- **CLASS** - Device class designation
- **TYPE** - Specific device type within the class
- **INDEX** - Device index number
- **IOREG** - I/O register base address
- **UNITSTS** - Unit status information

## SYSTEM Class Characteristics

Key characteristics for system-level devices:
- **ALMLIMIT** - Alarm limit for system alerts
- **AVAILMEM** - Available memory in system
- **COUNTRY** - Country code for localization
- **DEBUG** - Debug mode flag
- **DOSVER** - DOS version compatibility
- **FPUENB** - Floating Point Unit enabled flag
- **FPUTYPE** - Floating Point Unit type identifier
- **HIGHDMA** - High DMA availability
- **INITIAL** - Initial system startup flag
- **KBRESET** - Keyboard reset status
- **LOADDATE** - System load date
- **LOADTIME** - System load time
- **LOGIN** - Login status
- **NUMFLPY** - Number of floppy drives
- **NUMHARD** - Number of hard drives
- **NUMPAR** - Number of parallel ports
- **NUMSER** - Number of serial ports
- **OMLIMIT** - Outer memory limit
- **PMLIMIT** - Protected mode memory limit
- **PROINUSE** - Number of processes in use
- **PROLIMIT** - Process limit
- **REALBASE** - Real mode base address
- **REALSIZE** - Real mode memory size
- **RMLIMIT** - Real mode memory limit
- **SELINUSE** - Selector count in use
- **SELNUM** - Total selector count
- **SERNUM** - System serial number
- **SPEED** - System speed indicator
- **STATE** - System state information
- **SYSNAME** - System name
- **TMLIMIT** - Time limit for processes
- **TOTALMEM** - Total memory in system
- **USERMEM** - User memory available
- **WSLIMIT** - Workspace memory limit
- **XFFINUSE** - Extended file function in use count
- **XFFLIMIT** - Extended file function limit
- **XFFMAX** - Maximum extended file functions
- **XFFNUM** - Extended file function number
- **XMBAMAX** - Extended memory block allocation maximum
- **XMBAVAIL** - Extended memory block available
- **XMBINUSE** - Extended memory block in use
- **XMBMAX** - Extended memory block maximum
- **XMBRESRV** - Extended memory block reserved
- **XOSVER** - XOS version number

## PROCESS Class Characteristics

Characteristics for managing processes:
- **CONTRM** - Console termination status
- **COUNTRY** - Process country code
- **FPUENB** - Floating Point Unit enablement
- **LABLKS** - Large block size
- **LAINUSE** - Large memory in use
- **LALARGE** - Large memory size
- **NAME** - Process name
- **NUM** - Process number
- **OMALLOW** - Outer memory allowance
- **OMINUSE** - Outer memory in use
- **OMLIMIT** - Outer memory limit
- **PMALLOW** - Protected mode memory allowance
- **PMINUSE** - Protected mode memory in use
- **PMLIMIT** - Protected mode memory limit
- **PRIV** - Process privileges
- **PRIVAVL** - Privileges available
- **REALBASE** - Real mode base address
- **REALSIZE** - Real mode memory size
- **RMALLOW** - Real mode memory allowance
- **RMINUSE** - Real mode memory in use
- **RMLIMIT** - Real mode memory limit
- **SEQ** - Process sequence number
- **SHRDELAY** - Shared memory delay
- **SHRRETRY** - Shared memory retry count
- **TMALLOW** - Time allowance
- **TMINUSE** - Time in use
- **TMLIMIT** - Time limit
- **WSALLOW** - Workspace memory allowance
- **WSINUSE** - Workspace memory in use
- **WSLIMIT** - Workspace memory limit

## DISK Class Characteristics

Storage device characteristics:
- **AVAIL** - Available storage space
- **CBLKSZ** - Cluster block size
- **CCYLNS** - Cylinder count
- **CHEADS** - Head count
- **CLASS** - Disk class
- **CLSSZ** - Cluster size
- **CLUS TERS** - Cluster count
- **CSECTS** - Sector count
- **DBLKSZ** - Data block size
- **DCYLNS** - Data cylinder count
- **DHEADS** - Data head count
- **DOSNAME** - DOS name for compatibility
- **DSECTS** - Data sector count
- **FATMODE** - FAT mode
- **FSTYPE** - File system type (DOS12, DOS16, DOSEXT, DOSHP, DSS12, DSS12L, DSS16, DSS16L, XOS)
- **HDATAERR** - Hard data error count
- **HDEVERR** - Hard device error count
- **HIDFERR** - Hard ID field error count
- **HOVRNERR** - Hard overrun error count
- **HRNFERR** - Hard record not found error count
- **HSEEKERR** - Hard seek error count
- **HUNGERR** - Hung error count
- **IBLKSZ** - Input block size
- **IBLOCKS** - Input block count
- **ICYLNS** - Input cylinder count
- **IHEADS** - Input head count
- **ISECTS** - Input sector count
- **MCYLNS** - Media cylinder count
- **MHEADS** - Media head count
- **MSECTS** - Media sector count
- **MODEL** - Disk model
- **PARTN** - Partition number
- **PARTOFF** - Partition offset
- **PROTECT** - Protection status
- **RAMAX** - Recommended address maximum
- **REMOVE** - Removable media flag
- **RE VI SION** - Revision number
- **ROOTBLK** - Root block number
- **ROOTPROT** - Root protection status
- **ROOTSIZE** - Root size
- **SCSIDEV** - SCSI device number
- **SCSILUN** - SCSI logical unit number
- **SCSITAR** - SCSI target number
- **SECPINT** - Sectors per interrupt count
- **SERIALNO** - Serial number
- **SHRDELAY** - Shared delay value
- **SHRFAIL** - Shared failure count
- **SHRRETRY** - Shared retry count
- **TDATAERR** - Temporary data error count
- **TDEVERR** - Temporary device error count
- **TIDFERR** - Temporary ID field error count
- **TOVRNERR** - Temporary overrun error count
- **TRNFERR** - Temporary transfer error count
- **TSEEKERR** - Temporary seek error count
- **UNITTYPE** - Unit type identifier
- **VOLCDT** - Volume creation date
- **VOLEDT** - Volume expiration date
- **VOLLABEL** - Volume label
- **VOLMDT** - Volume modification date
- **VOLNAME** - Volume name
- **VOLXDT** - Volume access date
- **WTMAX** - Write maximum count

## TRM (Terminal) Class Characteristics

Terminal device characteristics:
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

## NET (Network) Class Characteristics

Network device characteristics:
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

## UDP (User Datagram Protocol) Class Characteristics

UDP protocol characteristics:
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

## TCP (Transmission Control Protocol) Class Characteristics

TCP protocol characteristics:
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

## Device Characteristic Management

### Setting Device Characteristics
Device characteristics can be set through:
- **svcIoDevParm** - Get or set device parameters
- **Device configuration files** - Static configuration
- **System initialization** - Automatic setup
- **Runtime adjustment** - Dynamic reconfiguration

### Reading Device Characteristics
Device characteristics can be read through:
- **svcIoDevParm** - Get device parameters
- **System calls** - Specialized characteristic retrieval
- **Device inquiry** - Direct device interrogation
- **Configuration utilities** - Administrative tools

## Device Class Relationships
Device classes form a hierarchy:
- Parent-child relationships between device types
- Inheritance of characteristics from parent classes
- Override of specific characteristics in child classes
- Composition of multiple characteristics for complex devices

## Error Handling
Common device characteristic errors:
- **DEVER** - Device error
- **DEVFL** - Device failed
- **DFDEV** - Device function error
- **DLOCK** - Device locked
- **NOERR** - No error (operation successful)