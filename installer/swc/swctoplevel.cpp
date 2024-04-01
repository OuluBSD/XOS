#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"


swcTopLevel::swcTopLevel()

{
}


swcTopLevel::swcTopLevel(
    swcDisplayBase
	      *parent,
	char  *caption,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
    long   txtcolorarg,
    long   bgcolorarg,
	long   bits)
	: swcDisplayBase("swcDISPLAY", caption, parent, xpos, ypos, width, height,
			txtcolorarg, bgcolorarg, bits, WS_POPUP | WS_CLIPCHILDREN |
			WS_CLIPSIBLINGS)

{
	ffwindow = NULL;
}


swcTopLevel::swcTopLevel(
    swcDisplayBase
	      *parent,
	char  *caption,
	char  *classname,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
    long   txtcolorarg,
    long   bgcolorarg,
	long   bits)
	: swcDisplayBase(classname, caption, parent, xpos, ypos, width, height,
			txtcolorarg, bgcolorarg, bits, WS_OVERLAPPED | WS_CLIPCHILDREN |
			WS_CLIPSIBLINGS)

{
	ffwindow = NULL;
}



void swcTopLevel::SetTitle(
	char *text)

{
	SetWindowText(hwin, text);
}
