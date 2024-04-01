#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

swcDisplay::swcDisplay()

{
}

swcDisplay::swcDisplay(
    swcDisplayBase
	      *parent,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
    long   txtcolorarg,
    long   bgcolorarg,
	long   bits,
	void (*actionarg)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3arg,
	int    arg2arg)
	: swcDisplayBase((bits & SWC_WIN_USEDBLCLK) ? "swcDISPLAYDC" : "swcDISPLAY",
			"WinName", parent, xpos, ypos, width, height, txtcolorarg,
			bgcolorarg, bits, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			actionarg, arg3arg, arg2arg)
{
}
