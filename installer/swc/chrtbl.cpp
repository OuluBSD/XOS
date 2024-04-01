#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
///#include <xosstuff.h>
#include "swcwindows.h"

// This table translates from the internal scan code to an ASCII code when
//   neither the shift or control keys are pressed

static uchar keytblnorm[] =
{	0,			// 00
	0,			// 01 - F1 key
	0,			// 02 - F2 key
	0,			// 03 - F3 key
	0,			// 04 - F4 key
	0,			// 05 - F5 key
	0,			// 06 - F6 key
	0,			// 07 - F7 key
	0,			// 08 - F8 key
	0,			// 09 - F9 key
	0,			// 0A - F10 key
	0,			// 0B - F11 key
	0,			// 0C - F12 key
	0,			// 0D
	0,			// 0E
	0,			// 0F
	0x1B,		// 10 - Escape
	0,			// 11 - Insert
	0,			// 12 - Home
	0,			// 13 - Page up
	0,			// 14 - Delete
	0,			// 15 - End
	0,			// 16 - Page down
	0,			// 17 - Up arrow
	0,			// 18 - Left arrow
	0,			// 19 - Down arrow
	0,			// 1A - Right arrow
	0,			// 1B
	0,			// 1C
	0,			// 1D
	0,			// 1E
	0,			// 1F
	'0',		// 20 - Keypad 0
	'1',		// 21 - Keypad 1
	'2',		// 22 - Keypad 2
	'3',		// 23 - Keypad 3
	'4',		// 24 - Keypad 4
	'5',		// 25 - Keypad 5
	'6',		// 26 - Keypad 6
	'7',		// 27 - Keypad 7
	'8',		// 28 - Keypad 8
	'9',		// 29 - Keypad 9
	'.',		// 2A - Keypad .
	'/',		// 2B - Keypad /
	'*',		// 2C - Keypad *
	'-',		// 2D - Keypad - (!!!)
	'+',		// 2E - Keypad +
	0x0D,		// 2F - Keypad enter
	0,			// 30
	0,			// 31
	0,			// 32
	0,			// 33
	0,			// 34
	0,			// 35
	0,			// 36
	0,			// 37
	0x0D,		// 38 - Enter
	0x0A,		// 39 - Line feed
	0x08,		// 3A - Backspace
	0x09,		// 3B - Tab
	0x7F,		// 3C - DEL
	0,			// 3D
	0,			// 3E
	0,			// 3F
	'`',		// 40 - Grave or ~
	'1',		// 41 - 1 or !
	'2',		// 42 - 2 or @
	'3',		// 43 - 3 or #
	'4',		// 44 - 4 or $
	'5',		// 45 - 5 or %
	'6',		// 46 - 6 or ^
	'7',		// 47 - 7 or &
	'8',		// 48 - 8 or *
	'9',		// 49 - 9 or (
	'0',		// 4A - 0 or )
	'-',		// 4B - - or _
	'=',		// 4C - = or +
	'q',		// 4D - Q
	'w',		// 4E - W
	'e',		// 4F - E
	'r',		// 50 - R
	't',		// 51 - T
	'y',		// 52 - Y
	'u',		// 53 - U
	'i',		// 54 - I
	'o',		// 55 - O
	'p',		// 56 - P
	'[',		// 57 - [ or {
	']',		// 58 - ] or }
	'\\',		// 59 - \ or |
	'a',		// 5A - A
	's',		// 5B - S
	'd',		// 5C - D
	'f',		// 5D - F
	'g',		// 5E - G
	'h',		// 5F - H
	'j',		// 60 - J
	'k',		// 61 - K
	'l',		// 62 - L
	';',		// 63 - ;
	0x27,		// 64 - ' or "
	'z',		// 65 - Z
	'x',		// 66 - X
	'c',		// 67 - C
	'v',		// 68 - V
	'b',		// 69 - B
	'n',		// 6A - N
	'm',		// 6B - M
	',',		// 6C - , or <
	'.',		// 6D - . or >
	'/',		// 6E - / or ?
	' ',		// 6F - Space
	0,			// 70
	0,			// 71
	0,			// 72
	0,			// 73 - Scroll lock
	0,			// 74 - Num lock
	0,			// 75 - Caps lock
	0,			// 76 - Left shift
	0,			// 77 - Right shift
	0,			// 78 - Left Alt
	0,			// 79 - Right Alt
	0,			// 7A - Left control
	0,			// 7B - Right control
	0,			// 7C - Print screen
	0,			// 7D - Break
	0,			// 7E - Pause
	0			// 7F - System shift
};

// This table translates from the internal scan code to an ASCII code when
//   the shift key(s) only is pressed

static uchar keytblshift[] =
{	0,			// 00
	0,			// 01 - F1 key
	0,			// 02 - F2 key
	0,			// 03 - F3 key
	0,			// 04 - F4 key
	0,			// 05 - F5 key
	0,			// 06 - F6 key
	0,			// 07 - F7 key
	0,			// 08 - F8 key
	0,			// 09 - F9 key
	0,			// 0A - F10 key
	0,			// 0B - F11 key
	0,			// 0C - F12 key
	0,			// 0D
	0,			// 0E
	0,			// 0F
	0x1B,		// 10 - ESC
	0,			// 11 - Insert
	0,			// 12 - Home
	0,			// 13 - Page up
	0,			// 14 - Delete
	0,			// 15 - End
	0,			// 16 - Page down
	0,			// 17 - Up arrow
	0,			// 18 - Left arrow
	0,			// 19 - Down arrow
	0,			// 1A - Right arrow
	0,			// 1B
	0,			// 1C
	0,			// 1D
	0,			// 1E
	0,			// 1F
	'0',		// 20 - Keypad 0
	'1',		// 21 - Keypad 1
	'2',		// 22 - Keypad 2
	'3',		// 23 - Keypad 3
	'4',		// 24 - Keypad 4
	'5',		// 25 - Keypad 5
	'6',		// 26 - Keypad 6
	'7',		// 27 - Keypad 7
	'8',		// 28 - Keypad 8
	'9',		// 29 - Keypad 9
	'.',		// 2A - Keypad .
	'/',		// 2B - Keypad /
	'*',		// 2C - Keypad *
	'-',		// 2D - Keypad - (!!!)
	'+',		// 2E - Keypad +
	0x0D,		// 2F - Keypad enter
	0,			// 30
	0,			// 31
	0,			// 32
	0,			// 33
	0,			// 34
	0,			// 35
	0,			// 36
	0,			// 37
	0x0D,		// 38 - Enter
	0x0A,		// 39 - Line feed
	0x08,		// 3A - Backspace
	0x09,		// 3B - Tab
	0x7F,		// 3C - DEL
	0,			// 3D
	0,			// 3E
	0,			// 3F
	'~',		// 40 - Grave or ~
	'!',		// 41 - 1 or !
	'@',		// 42 - 2 or @
	'#',		// 43 - 3 or #
	'$',		// 44 - 4 or $
	'%',		// 45 - 5 or %
	'^',		// 46 - 6 or ^
	'&',		// 47 - 7 or &
	'*',		// 48 - 8 or *
	'(',		// 49 - 9 or (
	')',		// 4A - 0 or )
	'_',		// 4B - - or _
	'+',		// 4C - = or +
	'Q',		// 4D - Q
	'W',		// 4E - W
	'E',		// 4F - E
	'R',		// 50 - R
	'T',		// 51 - T
	'Y',		// 52 - Y
	'U',		// 53 - U
	'I',		// 54 - I
	'O',		// 55 - O
	'P',		// 56 - P
	'{',		// 57 - [ or {
	'}',		// 58 - ] or }
	'|',		// 59 - \ or |
	'A',		// 5A - A
	'S',		// 5B - S
	'D',		// 5C - D
	'F',		// 5D - F
	'G',		// 5E - G
	'H',		// 5F - H
	'J',		// 60 - J
	'K',		// 61 - K
	'L',		// 62 - L
	':',		// 63 - ;
	'"',		// 64 - ' or "
	'Z',		// 65 - Z
	'X',		// 66 - X
	'C',		// 67 - C
	'V',		// 68 - V
	'B',		// 69 - B
	'N',		// 6A - N
	'M',		// 6B - M
	'<',		// 6C - , or <
	'>',		// 6D - . or >
	'?',		// 6E - / or ?
	' ',		// 6F - Space
	0,			// 70
	0,			// 71
	0,			// 72
	0,			// 73 - Scroll lock
	0,			// 74 - Num lock
	0,			// 75 - Caps lock
	0,			// 76 - Left shift
	0,			// 77 - Right shift
	0,			// 78 - Left Alt
	0,			// 79 - Right Alt
	0,			// 7A - Left control
	0,			// 7B - Right control
	0,			// 7C - Print screen
	0,			// 7D - Break
	0,			// 7E - Pause
	0			// 7F - System shift
};

// This table translates from the internal scan code to an ASCII code
//   when the control key only is pressed

static uchar keytblctrl[] =
{	0,			// 00
	0,			// 01 - F1 key
	0,			// 02 - F2 key
	0,			// 03 - F3 key
	0,			// 04 - F4 key
	0,			// 05 - F5 key
	0,			// 06 - F6 key
	0,			// 07 - F7 key
	0,			// 08 - F8 key
	0,			// 09 - F9 key
	0,			// 0A - F10 key
	0,			// 0B - F11 key
	0,			// 0C - F12 key
	0,			// 0D
	0,			// 0E
	0,			// 0F
	0x1B,		// 10 - ESC
	0,			// 11 - Insert
	0,			// 12 - Home
	0,			// 13 - Page up
	0,			// 14 - Delete
	0,			// 15 - End
	0,			// 16 - Page down
	0,			// 17 - Up arrow
	0,			// 18 - Left arrow
	0,			// 19 - Down arrow
	0,			// 1A - Right arrow
	0,			// 1B
	0,			// 1C
	0,			// 1D
	0,			// 1E
	0,			// 1F
	'0',		// 20 - Keypad 0
	'1',		// 21 - Keypad 1
	'2',		// 22 - Keypad 2
	'3',		// 23 - Keypad 3
	'4',		// 24 - Keypad 4
	'5',		// 25 - Keypad 5
	'6',		// 26 - Keypad 6
	'7',		// 27 - Keypad 7
	'8',		// 28 - Keypad 8
	'9',		// 29 - Keypad 9
	'.',		// 2A - Keypad .
	'/',		// 2B - Keypad /
	'*',		// 2C - Keypad *
	'-',		// 2D - Keypad - (!!!)
	'+',		// 2E - Keypad +
	0x0D,		// 2F - Keypad enter
	0,			// 30
	0,			// 31
	0,			// 32
	0,			// 33
	0,			// 34
	0,			// 35
	0,			// 36
	0,			// 37
	0x0D,		// 38 - Enter
	0x0A,		// 39 - Line feed
	0x7F,		// 3A - Backspace
	0x11,		// 3B - Tab
	0x7F,		// 3C - DEL
	0,			// 3D
	0,			// 3E
	0,			// 3F
	'`',		// 40 - Grave or ~
	'1',		// 41 - 1 or !
	'2',		// 42 - 2 or @
	'3',		// 43 - 3 or #
	'4',		// 44 - 4 or $
	'5',		// 45 - 5 or %
	'6',		// 46 - 6 or ^
	'7',		// 47 - 7 or &
	'8',		// 48 - 8 or *
	'9',		// 49 - 9 or (
	'0',		// 4A - 0 or )
	'-',		// 4B - - or _
	'=',		// 4C - = or +
	'Q'-0x40,	// 4D - Q
	'W'-0x40,	// 4E - W
	'E'-0x40,	// 4F - E
	'R'-0x40,	// 50 - R
	'T'-0x40,	// 51 - T
	'Y'-0x40,	// 52 - Y
	'U'-0x40,	// 53 - U
	'I'-0x40,	// 54 - I
	'O'-0x40,	// 55 - O
	'P'-0x40,	// 56 - P
	'['-0x40,	// 57 - [ or {
	']'-0x40,	// 58 - ] or }
	'\\'-0x40,	// 59 - \ or |
	'A'-0x40,	// 5A - A
	'S'-0x40,	// 5B - S
	'D'-0x40,	// 5C - D
	'F'-0x40,	// 5D - F
	'G'-0x40,	// 5E - G
	'H'-0x40,	// 5F - H
	'J'-0x40,	// 60 - J
	'K'-0x40,	// 61 - K
	'L'-0x40,	// 62 - L
	';',		// 63 - ;
	0x27,		// 64 - ' or "
	'Z'-0x40,	// 65 - Z
	'X'-0x40,	// 66 - X
	'C'-0x40,	// 67 - C
	'V'-0x40,	// 68 - V
	'B'-0x40,	// 69 - B
	'N'-0x40,	// 6A - N
	'M'-0x40,	// 6B - M
	',',		// 7C - , or <
	'.',		// 7D - . or >
	'/',		// 7E - / or ?
	' ',		// 7F - Space
	0,			// 70
	0,			// 71
	0,			// 72
	0,			// 73 - Scroll lock
	0,			// 74 - Num lock
	0,			// 75 - Caps lock
	0,			// 76 - Left shift
	0,			// 77 - Right shift
	0,			// 78 - Left Alt
	0,			// 79 - Right Alt
	0,			// 7A - Left control
	0,			// 7B - Right control
	0,			// 7C - Print screen
	0,			// 7D - Break
	0,			// 7E - Pause
	0			// 7F - System shift
};

// This table translates from the internal scan code to an ASCII code
//   when the control key and shift keys are pressed

static uchar keytblctrlshift[] =
{	0,			// 00
	0,			// 01 - F1 key
	0,			// 02 - F2 key
	0,			// 03 - F3 key
	0,			// 04 - F4 key
	0,			// 05 - F5 key
	0,			// 06 - F6 key
	0,			// 07 - F7 key
	0,			// 08 - F8 key
	0,			// 09 - F9 key
	0,			// 0A - F10 key
	0,			// 0B - F11 key
	0,			// 0C - F12 key
	0,			// 0D
	0,			// 0E
	0,			// 0F
	0x1B,		// 10 - ESC
	0,			// 11 - Insert
	0,			// 12 - Home
	0,			// 13 - Page up
	0,			// 14 - Delete
	0,			// 15 - End
	0,			// 16 - Page down
	0,			// 17 - Up arrow
	0,			// 18 - Left arrow
	0,			// 19 - Down arrow
	0,			// 1A - Right arrow
	0,			// 1B
	0,			// 1C
	0,			// 1D
	0,			// 1E
	0,			// 1F
	'0',		// 20 - Keypad 0
	'1',		// 21 - Keypad 1
	'2',		// 22 - Keypad 2
	'3',		// 23 - Keypad 3
	'4',		// 24 - Keypad 4
	'5',		// 25 - Keypad 5
	'6',		// 26 - Keypad 6
	'7',		// 27 - Keypad 7
	'8',		// 28 - Keypad 8
	'9',		// 29 - Keypad 9
	'.',		// 2A - Keypad .
	'/',		// 2B - Keypad /
	'*',		// 2C - Keypad *
	'-',		// 2D - Keypad - (!!!)
	'+',		// 2E - Keypad +
	0x0D,		// 2F - Keypad enter
	0,			// 30
	0,			// 31
	0,			// 32
	0,			// 33
	0,			// 34
	0,			// 35
	0,			// 36
	0,			// 37
	0x0D,		// 38 - Enter
	0x0A,		// 39 - Line feed
	0x7F,		// 3A - Backspace
	0x09,			// 3B - Tab
	0x7F,		// 3C - DEL
	0,			// 3D
	0,			// 3E
	0,			// 3F
	'`',		// 40 - Grave or ~
	'1',		// 41 - 1 or !
	'@',		// 42 - 2 or @
	'3',		// 43 - 3 or #
	'4',		// 44 - 4 or $
	'5',		// 45 - 5 or %
	'^'-0x40,	// 46 - 6 or ^
	'7',		// 47 - 7 or &
	'8',		// 48 - 8 or *
	'9',		// 49 - 9 or (
	'0',		// 4A - 0 or )
	'_'-0x40,	// 4B - - or _
	'=',		// 4C - = or +
	'Q'-0x40,	// 4D - Q
	'W'-0x40,	// 4E - W
	'E'-0x40,	// 4F - E
	'R'-0x40,	// 50 - R
	'T'-0x40,	// 51 - T
	'Y'-0x40,	// 52 - Y
	'U'-0x40,	// 53 - U
	'I'-0x40,	// 54 - I
	'O'-0x40,	// 55 - O
	'P'-0x40,	// 56 - P
	'['-0x40,	// 57 - [ or {
	']'-0x40,	// 58 - ] or }
	'\\'-0x40,	// 59 - \ or |
	'A'-0x40,	// 5A - A
	'S'-0x40,	// 5B - S
	'D'-0x40,	// 5C - D
	'F'-0x40,	// 5D - F
	'G'-0x40,	// 5E - G
	'H'-0x40,	// 5F - H
	'J'-0x40,	// 60 - J
	'K'-0x40,	// 61 - K
	'L'-0x40,	// 62 - L
	';',		// 63 - ;
	0x27,		// 64 - ' or "
	'Z'-0x40,	// 65 - Z
	'X'-0x40,	// 66 - X
	'C'-0x40,	// 67 - C
	'V'-0x40,	// 68 - V
	'B'-0x40,	// 69 - B
	'N'-0x40,	// 6A - N
	'M'-0x40,	// 6B - M
	',',		// 6C - , or <
	'.',		// 6D - . or >
	'/',		// 6E - / or ?
	' ',		// 6F - Space
	0,			// 70
	0,			// 71
	0,			// 72
	0,			// 73 - Scroll lock
	0,			// 74 - Num lock
	0,			// 75 - Caps lock
	0,			// 76 - Left shift
	0,			// 77 - Right shift
	0,			// 78 - Left Alt
	0,			// 79 - Right Alt
	0,			// 7A - Left control
	0,			// 7B - Right control
	0,			// 7C - Print screen
	0,			// 7D - Break
	0,			// 7E - Pause
	0			// 7F - System shift
};


uchar *swckeytbl[] =
{	keytblnorm,
	keytblshift,
	keytblctrl,
	keytblctrlshift
};
