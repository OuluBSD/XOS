# The Signal System in XOS

## Overview
The XOS signal system is responsible for handling asynchronous events and interrupts in the system. Signals provide a mechanism for processes to receive notifications about various system events.

## Signal Concepts
In XOS, signals are used to:
- Notify processes of hardware interrupts
- Handle software exceptions
- Communicate between processes
- Handle system events (e.g., timer expiration)
- Manage process control (e.g., suspension, termination)

## Signal Levels
XOS uses signal levels to prioritize different types of signals:
- Higher numeric levels indicate higher priority
- The system maintains current and previous signal levels
- Processes can set their own signal level to control interrupt handling

## Stack Formats for DPMI Vectors
XOS uses specific stack formats when handling DPMI (DOS Protected Mode Interface) vectors:
- 32-bit stack frames for 32-bit processes
- 16-bit stack frames for 16-bit processes
- Proper handling of stack switching between different modes

## Signal Functions
The system provides functions to:
- Set signal vectors (svcSchSetVector)
- Get current signal vectors (svcSchGetVector)
- Adjust signal levels (svcSchSetLevel)
- Process interrupts (svcSchIRet)
- Dismiss signals (svcSchDismiss)
- Control C processing (svcSchCtlCDone)

## Mixed-Mode Stack Management
XOS handles mixed-mode programming environments (16-bit and 32-bit) by:
- Managing stack transitions between modes
- Preserving register contexts during mode switches
- Handling exceptions that cross mode boundaries
- Managing different calling conventions

## Signal Handling Process
1. Hardware or software event occurs
2. System determines appropriate signal level
3. Current process signal level checked
4. If signal level is higher, interrupt is processed
5. Signal vector is called
6. Signal handler executes
7. Control returns to interrupted process or new process

## Special Signal Cases
- Timer interrupts for process scheduling
- Hardware interrupt handling
- Exception processing (division by zero, invalid opcodes, etc.)
- Process termination signals
- Communication signals between processes