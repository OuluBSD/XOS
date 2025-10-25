# Terminal System Calls in XOS

## Overview
The XOS terminal system calls provide comprehensive control over terminal devices and display functionality. These functions manage console I/O, screen display, cursor positioning, and terminal characteristics.

## Display Attribute Functions

### svcTrmAttrib - Get or Set Display Attributes
- Gets or sets display attributes like colors, blinking, etc.
- Controls foreground and background colors
- Manages character attributes (bold, underline, reverse video)
- Supports various display modes
- Works with different terminal types

## Cursor Management Functions

### svcTrmCurPos - Get or Set Cursor Position
- Gets or sets the cursor position on the display
- Supports absolute positioning (row, column)
- Can retrieve current cursor position
- Validates position within screen boundaries
- Handles different screen sizes and modes

### svcTrmCurType - Get or Set Cursor Type
- Controls cursor appearance (block, underline, etc.)
- Sets cursor visibility (visible, hidden, blinking)
- Supports different cursor styles for various applications
- Manages cursor blink rate when applicable

## Display Page Functions

### svcTrmDspPage - Get or Set Current Display Page
- Manages multiple display pages in console
- Allows switching between display pages
- Gets information about current display page
- Supports systems with multiple virtual display pages

## Terminal Function Capabilities

### svcTrmFunction - General Terminal Functions
- Performs various general terminal operations
- Supports terminal-specific features
- Handles terminal initialization and configuration
- Manages terminal status and features
- Supports different terminal types and capabilities

## Character Read/Write Functions

### svcTrmGetAtChr - Get Attribute and Character
- Reads character and attributes at cursor position
- Gets both character code and display attributes
- Supports reading from any screen position
- Used for screen manipulation and capture
- Preserves original screen content

### svcTrmSetAtChr - Set Attribute and Character
- Sets character and attributes at cursor position
- Updates both character code and display attributes
- Supports updating any screen position
- Used for screen manipulation and updates

### svcTrmSetChr - Set Character
- Sets character at cursor position without changing attributes
- Updates only the character code
- Preserves existing display attributes
- More efficient than SetAtChr when only character changes

## Font Management Functions

### svcTrmLdStdFont - Load Standard Font
- Loads the standard system font
- Resets terminal to use standard character set
- Handles different font sizes and types
- Manages font resources

### svcTrmLdCusFont - Load Custom Font
- Loads a custom font for the terminal
- Supports user-defined character sets
- Handles font validation and installation
- Manages custom font resources

### svcTrmSelFont - Select Font
- Selects an already loaded font for use
- Switches between different loaded fonts
- Supports multiple font management
- Handles font switching operations

## Screen Management Functions

### svcTrmMapScrn - Map Screen Buffer
- Maps physical screen buffer for direct access
- Allows direct memory access to screen contents
- Supports screen capture and manipulation
- Manages memory mapping resources

### svcTrmScroll - Scroll Window
- Scrolls text within a specified window
- Supports up, down, left, right scrolling
- Defines scroll region with coordinates
- Handles text repositioning within boundaries

## Input Buffer Functions

### svcTrmWrtInB - Write to Input Buffer
- Writes characters to terminal input buffer
- Simulates keyboard input programmatically
- Supports special key sequences
- Allows process to inject input into the system

## Graphic Cursor Functions

### svcTrmGCurPos - Set Graphic Cursor Position
- Sets position for graphic cursor
- Used for systems with graphic cursors
- Supports precise positioning
- Handles coordinate system conversion

### svcTrmGCurCol - Set Graphic Cursor Colors
- Sets colors for graphic cursor
- Controls graphic cursor appearance
- Supports foreground and background colors
- Manages color attributes

### svcTrmGCurPat - Set Graphic Cursor Pattern
- Sets pattern for graphic cursor
- Defines custom cursor appearance
- Supports various cursor shapes
- Manages cursor pattern data

## Terminal Parameters

### Common Terminal Characteristics
- **CLASS** - Device class designation
- **TYPE** - Specific device type
- **INLBS** - Input line buffer size
- **INRBS** - Input raw buffer size  
- **OUTRBS** - Output raw buffer size
- **PASS WORD** - Password for access
- **PRO GRAM** - Associated program
- **SES SION** - Session information

### Serial Port Characteristics
- **DBITS** - Data bits (7 or 8)
- **SBITS** - Stop bits (1 or 2)
- **PAR ITY** - Parity (none, odd, even, mark, space)
- **RATE** - Baud rate
- **FLOWOVR** - Flow control override
- **CTS** - Clear to send control
- **DSR** - Data set ready control
- **XONXOFF** - Software flow control

### Console Terminal Characteristics
- **BELLFREQ** - Bell frequency
- **BELLLEN** - Bell duration
- **CHAROUT** - Character output processing
- **CURFIX** - Cursor fix mode
- **INRBHELD** - Input bytes held
- **INRBLOST** - Input bytes lost
- **INRBPL** - Input raw buffer power loss
- **KBTCHAR** - Keyboard translation character
- **OUT FLOW** - Output flow control
- **SCSVTIME** - Screen save time
- **SCSVTYPE** - Screen save type

## Virtual Screen Management
XOS supports multiple virtual screens per physical console:
- **Virtual Screen Switching**: Allows multiple applications to maintain separate screen contexts
- **Screen Context Preservation**: Each virtual screen maintains its own cursor position, content, and attributes
- **System Menu Integration**: Virtual screens accessed through system menu
- **Keyboard Control**: Dedicated keys for virtual screen switching

## Terminal Compatibility
The terminal system supports:
- **DOS Compatibility**: Compatible with standard DOS terminal operations
- **ANSI Sequences**: Support for ANSI escape sequences
- **Custom Control**: Vendor-specific terminal features
- **Device Independence**: Applications work with different terminal types

## Error Handling
Common error codes for terminal functions:
- `TRMNA` - Terminal not available
- `DEVFUL` - Device table full
- `ER_PRIV` - Insufficient privileges
- `PARMF` - Parameter format error
- `BUSY` - Terminal busy
- `NOERR` - Operation successful

## Best Practices
- Always check terminal capabilities before using advanced features
- Use standard functions instead of direct hardware access
- Handle terminal initialization properly
- Preserve terminal state when switching from/to other applications
- Use appropriate error handling for terminal operations
- Consider different terminal types in application design
- Use virtual screens appropriately for multitasking applications