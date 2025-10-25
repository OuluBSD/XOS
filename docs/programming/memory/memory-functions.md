# Memory System Calls in XOS

## Overview
The XOS memory system calls provide comprehensive memory management capabilities including allocation, deallocation, sharing, and protection. These functions support both linear memory blocks and segmented memory models.

## Linear Memory Block Functions

### svcMemBlkAlloc - Allocate Linear Memory Block
- Allocates a contiguous block of linear memory
- Supports different memory types (conventional, extended, etc.)
- Returns handle to the allocated block
- Can specify initial size and allocation flags

### svcMemBlkChange - Change Size of Linear Memory Block
- Changes the size of an existing memory block
- Can grow or shrink the block
- May move the block to a different location
- Preserves content during resizing

### svcMemBlkFree - Give up All Linear Memory Blocks
- Frees all linear memory allocated by the current process
- Performs cleanup of all allocated blocks
- Used during process termination
- Can also free individual blocks

## Basic Memory Management Functions

### svcMemChange - Change Memory Allocation
- Modifies properties of allocated memory
- Can change access permissions, size, or location
- Supports memory protection changes
- Used for dynamic memory management

## Shared Memory Functions

### svcMemConvShr - Convert to Shared Section
- Converts private memory to shared memory
- Allows multiple processes to access same memory
- Handles reference counting
- Manages shared memory permissions

### svcMemLinkShr - Link to Shared Section
- Creates a link to an existing shared memory section
- Allows process to access shared memory created elsewhere
- Supports different access permissions
- Handles reference counting

## Memory Copy Functions

### svcMemCopy2PM - Copy Data to Protected Mode Memory
- Copies data to memory in protected mode
- Handles segment:offset addressing in 16-bit mode
- Performs bounds checking
- Manages different memory models

## Segment Management Functions

### svcMemCreate - Create New Segment
- Creates a new memory segment
- Defines segment properties and permissions
- Returns segment selector for access
- Supports various segment types (code, data, stack)

### svcMemRemove - Remove Segment
- Removes an existing memory segment
- Frees segment resources
- Updates process memory map
- Handles shared segment reference counting

### svcMemRmvMult - Remove Multiple Segments
- Removes multiple segments in one operation
- Efficient for process cleanup
- Handles complex segment relationships
- Maintains memory consistency

### svcMemMap - Map Physical Section
- Maps physical memory to virtual address space
- Used for device drivers and system services
- Handles physical-to-virtual address translation
- Manages memory protection

### svcMemNull - Map Null Memory
- Maps a null segment/pointer
- Used for error checking and debugging
- Provides safe way to test for null pointers
- Prevents accidental access to low memory areas

## Memory Protection and Type Functions

### svcMemSegType - Change Segment Type
- Changes the type of an existing memory segment
- Supports code, data, and stack segment types
- Updates segment access permissions
- Validates type change compatibility

### svcMemPageType - Change Memory Page Type
- Changes the type of memory pages
- Supports different page protection schemes
- Handles read/write/execute permissions
- Manages virtual memory attributes

### svcMemWPFunc - Watchpoint Functions
- Sets memory watchpoints for debugging
- Monitors read/write access to specific locations
- Supports execution breakpoints
- Used for debugging and analysis

### svcMemWPSet - Set Watchpoint
- Sets a specific watchpoint on memory location
- Defines access type to monitor (read, write, execute)
- Handles multiple simultaneous watchpoints
- Manages watchpoint resources

## Descriptor Management Functions

### svcMemDescAlloc - Allocate Segment Descriptor
- Allocates a segment descriptor
- Used for custom memory segment creation
- Handles descriptor table management
- Returns descriptor selector

### svcMemDescFind - Find Segment Descriptor
- Locates an existing segment descriptor
- Searches descriptor table by criteria
- Returns information about descriptor
- Used for descriptor validation

### svcMemDescFree - Give Up Segment Descriptor
- Frees an allocated segment descriptor
- Updates descriptor table
- Handles descriptor reference counting
- Used during cleanup

### svcMemDescRead - Read Segment Descriptor
- Reads contents of a segment descriptor
- Returns segment properties and attributes
- Used for debugging and verification
- Performs security checks

### svcMemDescWrite - Write Segment Descriptor
- Modifies contents of a segment descriptor
- Updates segment properties and attributes
- Performs access checks
- Manages descriptor security

### svcMemDescSet - Set Value in Segment Descriptor
- Sets a specific field in segment descriptor
- More efficient than full descriptor write
- Performs validation and access checks
- Supports different descriptor formats

## DOS Memory Functions

### svcMemDosSetup - Set Up DOS Memory
- Configures DOS memory for XOS use
- Handles DOS memory allocation
- Sets up DOS memory management
- Supports DOS memory sharing

## Memory Linking Functions

### svcMemLink - Link Segment Selectors
- Links different segment selectors together
- Creates relationships between segments
- Supports segment sharing
- Handles memory management

## Memory Movement Functions

### svcMemMove - Move Memory Section
- Moves memory from one location to another
- Handles overlapping memory regions
- Maintains data integrity during move
- Can move between different memory types

## Memory Debugging Functions

### svcMemDebug - Memory Debug Functions
- Provides memory debugging capabilities
- Memory leak detection
- Bounds checking
- Memory corruption detection
- Usage statistics

## Memory Management Notes
- All memory allocations should be properly freed
- Memory protection helps prevent process interference
- Shared memory requires synchronization between processes
- Memory allocation may fail under memory pressure
- Memory addresses are virtual and managed by the system
- Different memory types have different performance characteristics