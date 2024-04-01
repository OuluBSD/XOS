#include <stdio.h>
#include <xostrm.h>
#include <xostime.h>
#include <xosxxws.h>
#include <xosgdp.h>


XWSCNTRPAR xws_WinParContainerDefault =
{	{	sizeof(XWSCNTRPAR),	// blklen
		100,				// zorder      - Z-order value
		0,					// travlist    - Traversal list number
		0,					// travinx     - Traversal index
		0x808080,			// bgcolorn    - Background color
		0x0000FF,			// bdcolor     - Border color
		0xC0C0C0,			// hlcolorn    - Highlight color
		0x606060,			// shcolorn    - Shadow color
		1500,				// bdwidtho    - Outer border width
		0,					// bdwidthc    - Center border width
		0,					// bdwidthi    - Inner border width
		0,					// bits1       - Window bits 1
		WB2_MOVEABLE|WB2_LINEO|WB2_RECESSI,
							// bits2       - Window bits 2
		0,					// xradius     - X radius
		0,					// yradius     - Y radius
		0,					// xsfd        - X scale factor divisor
		0,					// ysfd        - Y scale factor divisor
		0,					// orient      - Orientation
		0,					// format      - Format
		0,					// cursor      - CUR for cursor for window
		0,					// bgbitmapn   - Background bitmap
		0,					// brdbitmap   - Border bitmap
		NULL,				// draw        - Draw function
		0,					// evarg1      - First event argument
		0					// evarg2      - Second event argument
	},
	{	6000,				// sbsize      - Size
		400,				// sbbdwid     - Border width
		0xC0C0C0,			// sbhlcoln    - Normal highlight color for scroll
							//                 bar border
		0x808080,			// sbshcoln    - Normal shadow color for scroll bar
							//                 border
		0xA0A0A0,			// sbbarcol    - Bar color
		100,				// sbbtnsize   - Button size
		600,				// sbbtnbdwid  - Button border width
		0xA0A0A0,			// sbbtnbgcoln - Button normal background color
		0x909090,			// sbbtnbgcola - Button alternate background color
		0x000000,			// sbbtntxcoln - Button normal text color
		0x000000,			// sbbtntxcola - Button alternate text color
		0xA0A0A0,			// sbbtnhlcoln - Normal button and thumb highlight
							//                 color
		0x808080,			// sbbtnhlcola - Alternate button and thumb
							//                 highlight color
		0x808080,			// sbbtnshcoln - Normal button and thumb shadow
							//                 color
		0xA0A0A0,			// sbbtnshcola - Alternate button and thumb shadow
							//                 color

		1000,				// sbbtnshift  - Button pressed offset
		{0xE2, 0x96, 0xB2},	// sbbtntchr   - Character for top button
		{0xE2, 0x96, 0xBC},	// sbbtnbchr   - Character for bottom button
		{0xE2, 0x97, 0x80},	// sbbtnlchr   - Character for left button
		{0xE2, 0x96, 0xB6},	// sbbtnrchr   - Character for right button
		600,				// sbtmbbdwid  - Thumb border width
		0xA0A0A0,			// sbtmbbgcoln - Thumb normal background color
		0x808080			// sbtmbbgcola - Thumb alternate background color
	},
	0,						// titlesize   - Title bar size
	0,						// titlebits   - Title bar bits
	0						// titletext   - Title bar text
};


XWSCNTRPAR xws_WinParContainerPlain =
{	{	sizeof(XWSCNTRPAR),
							// blklen
		100,				// zorder      - Z-order value
		0,					// travlist    - Traversal list number
		0,					// travinx     - Traversal index
		0x808080,			// bgcolorn    - Background color
		0x0000FF,			// brdcolor    - Border color
		0xC0C0C0,			// hilcolorn   - Highlight color
		0x606060,			// shdcolorn   - Shadow color
		0,					// brdwidtho   - Outer border width
		0,					// brdwidthc   - Center border width
		0,					// brdwidthi   - Inner border width
		0,					// bits1       - Window bits 1
		0,					// bits2       - Window bits 2
		0,					// xradius     - X radius
		0,					// yradius     - Y radius
		0,					// xsfd        - X scale factor divisor
		0,					// ysfd        - Y scale factor divisor
		0,					// orient      - Orientation
		0,					// format      - Format
		0,					// cursor      - CUR for cursor for window
		0,					// bgbitmapn   - Background bitmap
		0,					// brdbitmap   - Border bitmap
		NULL,				// draw        - Draw function
		0,					// evarg1      - First event argument
		0					// evarg2      - Second event argument
	},
	{	6000,				// sbsize      - Size
		1000,				// sbbdwid     - Border width
		0xC0C0C0,			// sbhlcoln    - Normal highlight color for
							//                 scroll bar border
		0x808080,			// sbshcoln    - Normal shadow color for
							//                 scroll bar border
		0xA0A0A0,			// sbbarcol    - Bar color
		100,				// sbbtnsize   - Button size
		600,				// sbbtnbdwid  - Button border width
		0xA0A0A0,			// sbbtnbgcoln - Button normal background
							//                 color
		0x909090,			// sbbtnbgcola - Button alternate background
							//                 color
		0x000000,			// sbbtntxcoln - Button normal text color
		0x000000,			// sbbtntxcola - Button alternate text color
		0xA0A0A0,			// sbbtnhlcoln - Normal button and thumb
							//                 highlight color
		0x808080,			// sbbtnhlcola - Alternate button and thumb
							//                 highlight color
		0x808080,			// sbbtnshcoln - Normal button and thumb
							//                 shadow color
		0xA0A0A0,			// sbbtnshcola - Alternate button and thumb
							//                 shadow color
		1000,				// sbbtnshift  - Button pressed offset
		{0xE2, 0x96, 0xB2},	// sbbtntchr   - Character for top button
		{0xE2, 0x96, 0xBC},	// sbbtnbchr   - Character for bottom button
		{0xE2, 0x97, 0x80},	// sbbtnlchr   - Character for left button
		{0xE2, 0x96, 0xB6},	// sbbtnrchr   - Character for right button
		600,				// sbtmbbdwid  - Thumb border width
		0xA0A0A0,			// sbtmbbgcoln - Thumb normal background
							//                 color
		0x808080			// sbtmbbgcola - Thumb alternate background
	},						//                 color
	0,						// titlesize   - Title bar size
	0,						// titlebits   - Title bar bits
	0						// titletext   - Title bar text
};


XWSLISTPAR xws_WinParListDefault =
{	{	sizeof(XWSLISTPAR),	// blklen
		100,				// zorder      - Z-order value
		0,					// travlist    - Traversal list number
		0,					// travinx     - Traversal index
		0xF0F0F0,			// bgcolorn    - Background color
		0x0000FF,			// brdcolor    - Border color
		0xA0A0A0,			// hilcolorn   - Highlight color
		0x606060,			// shdcolorn   - Shadow color
		1500,				// brdwidtho   - Outer border width
		0,					// brdwidthc   - Center border width
		0,					// brdwidthi   - Inner border width
		0,					// bits1       - Window bits 1
		WB2_MOVEABLE|WB2_LINEO|WB2_RECESSI,
							// bits2       - Window bits 2
		0,					// xradius     - X radius
		0,					// yradius     - Y radius
		0,					// xsfd        - X scale factor divisor
		0,					// ysfd        - Y scale factor divisor
		0,					// orient      - Orientation
		0,					// format      - Format
		0,					// cursor      - CUR for cursor for window
		0,					// bgbitmapn   - Background bitmap
		0,					// brdbitmap   - Border bitmap
		NULL,				// draw        - Draw function
		0,					// evarg1      - First event argument
		0					// evarg2      - Second event argument
	},
	{	6000,				// sbsize      - Size
		300,				// sbbdwid     - Border width
		0xD0D0D0,			// sbhlcoln    - Normal highlight color for
							//                 scroll bar border
		0x606060,			// sbshcoln    - Normal shadow color for scroll bar
							//                 border
		0xC0C0C0,			// sbbarcol    - Bar color
		100,				// sbbtnsize   - Button size
		600,				// sbbtnbdwid  - Button border width
		0xA0A0A0,			// sbbtnbgcoln - Button normal background color
		0x909090,			// sbbtnbgcola - Button alternate background color
		0x000000,			// sbbtntxcoln - Button normal text color
		0x000000,			// sbbtntxcola - Button alternate text color
		0xD0D0D0,			// sbbtnhlcoln - Normal button and thumb highlight
							//                 color
		0x606060,			// sbbtnhlcola - Alternate button and thumb
							//                 highlight color
		0x606060,			// sbbtnshcoln - Normal button and thumb shadow
							//                 color
		0xD0D0D0,			// sbbtnshcola - Alternate button and thumb shadow
							//                 color
		1000,				// sbbtnshift  - Button pressed offset
		{0xE2, 0x96, 0xB2},	// sbbtntchr   - Character for top button
		{0xE2, 0x96, 0xBC},	// sbbtnbchr   - Character for bottom button
		{0xE2, 0x97, 0x80},	// sbbtnlchr   - Character for left button
		{0xE2, 0x96, 0xB6},	// sbbtnrchr   - Character for right button
		600,				// sbtmbbdwid  - Thumb border width
		0xA0A0A0,
							// sbtmbbgcoln - Thumb normal background color
		0x808080			// sbtmbbgcola - Thumb alternate background color
	},
};


XWSEDITPAR xws_WinParEditDefault =
{	{	sizeof(XWSEDITPAR),
							// blklen
		100,				// zorder      - Z-order value
		0,					// travlist    - Traversal list number
		0,					// travinx     - Traversal index
		0xE0E0E0,			// bgcolorn    - Background color
		0x0000FF,			// bdcolor     - Border color
		0xC0C0C0,			// hlcolorn    - Highlight color
		0x606060,			// shcolorn    - Shadow color
		1500,				// bdwidtho    - Outer border width
		0,					// bdwidthc    - Center border width
		0,					// bdwidthi    - Inner border width
		0,					// bits1       - Window bits 1
		WB2_MOVEABLE|WB2_LINEO|WB2_RECESSO,
							// bits2       - Window bits 2
		0,					// xradius     - X radius
		0,					// yradius     - Y radius
		0,					// xsfd        - X scale factor divisor
		0,					// ysfd        - Y scale factor divisor
		0,					// orient      - Orientation
		0,					// format      - Format
		0,					// cursor      - CUR for cursor for window
		0,					// bgbitmapn   - Background bitmap
		0,					// brdbitmap   - Border bitmap
		NULL,				// draw        - Draw function
		0,					// evarg1      - First event argument
		0					// evarg2      - Second event argument
	},
	{	6000,				// sbsize      - Size
		1000,				// sbbdwid     - Border width
		0xC0C0C0,			// sbhlcoln    - Normal highlight color for
							//                 scroll bar border
		0x808080,			// sbshcoln    - Normal shadow color for
							//                 scroll bar border
		0xA0A0A0,			// sbbarcol    - Bar color
		100,				// sbbtnsize   - Button size
		3000,				// sbbtnbdwid  - Button border width
		0xA0A0A0,			// sbbtnbgcoln - Button normal background color
		0x909090,			// sbbtnbgcola - Button alternate background color
		0x000000,			// sbbtntxcoln - Button normal text color
		0x000000,			// sbbtntxcola - Button alternate text color
		0xA0A0A0,			// sbbtnhlcoln - Normal button and thumb highlight
							//                 color
		0x808080,			// sbbtnhlcola - Alternate button and thumb
							//                 highlight color
		0x808080,			// sbbtnshcoln - Normal button and thumb shadow
							//                 color
		0xA0A0A0,			// sbbtnshcola - Alternate button and thumb shadow
							//                 color
		1000,				// sbbtnshift  - Button pressed offset
		{0xE2, 0x96, 0xB2},	// sbbtntchr   - Character for top button
		{0xE2, 0x96, 0xBC},	// sbbtnbchr   - Character for bottom button
		{0xE2, 0x97, 0x80},	// sbbtnlchr   - Character for left button
		{0xE2, 0x96, 0xB6},	// sbbtnrchr   - Character for right button
		600,				// sbtmbbdwid  - Thumb border width
		0xA0A0A0,			// sbtmbbgcoln - Thumb normal background color
		0x808080			// sbtmbbgcola - Thumb alternate background color
	},
	0,						// titlesize   - Title bar size
	0,						// titlebits   - Title bar bits
	0						// titletext   - Title bar text
};


XWSBUTNPAR xws_WinParButtonDefault =
{	{	sizeof(XWSBUTNPAR),
							// blklen
		100,				// zorder    - Z-order value
		0,					// travlist    - Traversal list number
		0,					// travinx     - Traversal index
		0xA0A0A0,			// bgcoln  - Background color
		0x000000,			// brdcor  - Border color
		0xD0D0D0,			// hlcoln  - Highlight color
		0x606060,			// shcoln  - Shadow color
		1500,				// brdwido - Outer border width
		0,					// brdwidc - Center border width
		0,					// brdwidi - Inner border width
		0,					// bits1   - Window bits 1
		WB2_LINEO|WB2_RECESSI,
							// bits2       - Window bits 2
		0,					// xradius - X radius
		0,					// yradius - Y radius
		0,					// xsfd    - X scale factor divisor
		0,					// ysfd    - Y scale factor divisor
		0,					// orient  - Orientation
		0,					// format  - Format
		0,					// cursor  - CUR for cursor for window
		0,					// bgbmn   - Background bitmap
		0,					// brdbm   - Border bitmap
		NULL,				// draw    - Draw function
		0,					// evarg1  - First event argument
		0					// evarg2  - Second event argument
	},
	0x909090,				// bgcola  - Alternate background color
	0x606060,				// hilcola - Alternate highlight color
	0xA0A0A0,				// shdcola - Alternate shadow color
	2,						// shift   - Pressed button offset
	0						// bgbma   - Alternate background bitmap
};


XWSSBARPAR xws_WinParScrollBarDefault =
{	{	sizeof(XWSSBARPAR),
							// blklen
		255,				// zorder    - Z-order value
		0,					// travlist    - Traversal list number
		0,					// travinx     - Traversal index
		0xA0A0A0,			// bgcolorn  - Background color (Normal button
							//               background color)
		0xC0C0C0,			// brdcolor  - Border color (Bar color)
		0xD0D0D0,			// hlcoln    - Highlight color (Overall highlight
							//               color)
		0x606060,			// shcoln    - Shadow color (Overall shadow color)
		900,				// brdwido   - Outer border width (Overall border
							//               width)
		1200,				// brdwidc   - Center border width (Button border
							//               width)
		600,				// brdwidi   - Inner border width (Thumb border
							//               width)
		0,					// bits1     - Window bits 1
		WB2_NOSHWFCS|WB2_NOFOCUS|WB2_RECESSO,
							// bits2       - Window bits 2
		0,					// xradius   - X radius
		0,					// yradius   - Y radius
		0,					// xsfd      - X scale factor divisor
		0,					// ysfd      - Y scale factor divisor
		0,					// orient    - Orientation
		0,					// format    - Format
		0,					// cursor    - CUR for cursor for window
		0,					// bgbmn     - Background bitmap
		0,					// bdbm      - Border bitmap
		NULL,				// draw      - Draw function
		0,					// evarg1    - First event argument
		0					// evarg2    - Second event argument
	},
	500 * ST_MILLISEC,		// delayi    - Initial delay before repeating
	100 * ST_MILLISEC,		// delayr    - Repeat delay
	1000,					// tmbrange  - Thumb range
	1000,					// tmbsize   - Thumb size
	0,						// tmbpos    - Thumb position
	600,					// tmbbdwid  = Thumb border width
	0xA0A0A0,				// tmbbgcoln - Normal thumb background color
	0x909090,				// tmbbgcola - Alternate thumb background color
	100,					// btnsize   - Button size
	600,					// btnbdwid  - Button border width
	0xA0A0A0,				// btnbgcoln - Normal button background color
	0x909090,				// btnbgcola - Alternate button background color
	0x000000,				// btntxcoln - Normal button text color
	0x000000,				// btntxcola - Alternate button text color
	0xD0D0D0,				// btnhlcoln - Normal button highlight color
	0x606060,				// btnhlcola - Alternate button highlight color
	0x606060,				// btnshcoln - Normal button shadow color
	0xD0D0D0,				// btnshcola - Alternate button shadow color
	1000,					// sbbtnshift  - Button pressed offset
	{0xE2, 0x96, 0xB2},		// sbbtn1chr   - Character for button 1
	{0xE2, 0x96, 0xBC}		// sbbtn2chr   - Character for button 2
};


XWSTXTPAR xws_TxtParContainerDefault =
{	sizeof(XWSTXTPAR),		// blklen
	0,						// right       - Right edge (returned)
	0,						// bottom      - Bottom edge (returned)
	8000,					// fheight     - Font height
	0,						// fwidth      - Font width
	3,						// fontnum     - Font number
	0,						// fattr       - Font Attributes
	100,					// fweight     - Font weight
	{0},
	0,						// tbattr      - Text box Attributes
	TLF_LEFT,				// lfmt        - Initial line format
	{0},
	XWS_COLOR_TRANS,		// bgdcolor    - Background color
	0x000000,				// txtcolor    - Normal text color
	0x8080FF,				// bgdcolorsel - Selected background color
	0xFFFFFF,				// txtcolorsel - Selected Text color
	0x0000B0,				// bgdcolorfcs - Selected background color/focus
	0xFFFFFF,				// txtcolorfcs - Selected Text color/focus
	0x000000,				// caretcolor  - Caret color
	0,						// leftmargin  - Left margin
	0,						// rightmargin - Right margin
	0,						// topmargin   - Top margin
	0,						// btmmargin   - Bottom margin
	0,						// leading     - Extra space above line
	0						// lspace      - Extra space below line
};


XWSTXTPAR xws_TxtParButtonDefault =
{	sizeof(XWSTXTPAR),		// blklen
	0,						// right       - Right edge (returned)
	0,						// bottom      - Bottom edge (returned)
	7500,					// fheight     - Font height
	0,						// fwidth      - Font width
	3,						// fnum        - Font number
	0,						// fattr       - Font Attributes
	100,					// fweight     - Font weight
	{0},
	TBA_VERCEN,				// tbattr      - Text box Attributes
	TLF_BCENTER,			// lfmt        - Line format
	{0},
	XWS_COLOR_TRANS,		// bgdcolor    - Background color
	0x000000,				// txtcolor    - Normal text color
	0x0000B0,				// bgdcolorsel - Selected background color
	0xFFFFFF,				// txtcolorsel - Selected Text color
	0x0000B0,				// bgdcolorfcs - Selected background color/focus
	0xFFFFFF,				// txtcolorfcs - Selected Text color/focus
	0x000000,				// caretcolor  - Caret color
	0,						// leftmargin  - Left margin
	0,						// rightmargin - Right margin
	0,						// topmargin   - Top margin
	0,						// btmmargin   - Bottom margin
	0,						// leading     - Extra space above line
	0						// lspace      - Extra space below line
};
