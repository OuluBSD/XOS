# Scheduler System Calls in XOS

## Overview
The XOS scheduler system calls provide comprehensive process and thread management capabilities. These functions control process execution, timing, and resource allocation.

## Process Control Functions

### svcSchExit - Terminate Process
- Terminates the current process
- Handles cleanup of process resources
- Returns exit code to parent process
- Synchronous operation - does not return

### svcSchKill - Terminate Any Process
- Terminates a specified process by ID
- Requires appropriate privileges
- Forces termination without process cooperation
- Can specify termination code

### svcSchSpawn - Create Child Process
- Creates a new child process
- Supports various process attributes
- Inherits parent environment and privileges
- Returns process ID of new process

### svcSchSuspend - Suspend Process
- Temporarily stops a process execution
- Process can be resumed later
- Does not release system resources
- Can be self-suspend or target another process

### svcSchWaitProc - Wait for Process to Terminate
- Wait for a child process to terminate
- Blocks until child terminates or timeout
- Returns exit code of terminated process
- Supports waiting for specific or any child

## Signal and Event Functions

### svcSchSetVector - Set Signal Vector
- Sets the signal handler for a specific signal type
- Defines how the process responds to specific signals
- Supports multiple signal types
- Can set default handlers

### svcSchGetVector - Get Signal Vector
- Retrieves the current signal handler
- Used for signal chaining or inspection
- Allows examining current signal disposition

### svcSchSetLevel - Set Signal Level
- Sets the current signal level for the process
- Controls which signals can interrupt the process
- Higher levels prevent lower-level signal delivery
- Used for critical section protection

### svcSchDismiss - Dismiss Signal
- Dismisses a signal without processing
- Used when a signal is received but not needed
- Allows selective signal handling

### svcSchCtlCDone - Report ctl-C Processing Done
- Reports completion of control-C processing
- Allows custom control-C handling
- Tells system control-C sequence is complete

## Event Management Functions

### svcSchSetEvent - Set Event(s)
- Sets one or more events to active state
- Wakes up processes waiting on these events
- Can set multiple events in a single call
- Asynchronous operation

### svcSchWaitSEvent - Wait for Single Event
- Wait for a single specific event
- Blocks until event is set or timeout
- Can specify multiple possible events and wait for any one
- Supports timeout values

### svcSchWaitMEvent - Wait for Multiple Events
- Wait for multiple events with complex conditions
- Can wait for all events, any event, or complex boolean combinations
- Supports timeout values
- More complex than single event wait

### svcSchClrEvent - Clear Event(s)
- Clears one or more events to inactive state
- Opposite of SetEvent
- Can clear multiple events in single call

### svcSchMakEvent - Make Event Cluster
- Creates a cluster of related events
- Groups events for coordinated operations
- Allows collective operations on event groups

### svcSchResEvent - Reserve Event
- Reserves an event for specific use
- Prevents other processes from using the same event
- Ensures event availability for specific purposes

### svcSchRelEvent - Release Event
- Releases a reserved event
- Makes event available for other processes
- Complement to event reservation

## Timing and Alarm Functions

### svcSchAlarm - Alarm Functions
- Sets alarms for future events
- Supports various alarm types and timing models
- Can set one-time or periodic alarms
- Can cancel alarms before they fire

## Interrupt Functions

### svcSchIRet - Return From Interrupt
- Returns from an interrupt service routine
- Restores processor state after interrupt handling
- Properly restores all register contexts
- Handles nested interrupt scenarios

### svcSchIntrProc - Interrupt Child Process
- Sends interrupt signal to a child process
- Similar to Unix SIGINT
- Allows parent to interrupt child execution
- Requires appropriate privileges

## Process State Management
The scheduler maintains various process states:
- **Ready**: Process is ready to run but waiting for CPU
- **Running**: Process is currently executing
- **Waiting**: Process is blocked waiting for an event
- **Suspended**: Process suspended by system or process
- **Terminated**: Process has finished execution

## Priority and Scheduling
- Each process has a priority level that affects scheduling
- Higher priority processes get more CPU time
- Priority can be adjusted dynamically based on system conditions
- Real-time processes can be given priority over normal processes