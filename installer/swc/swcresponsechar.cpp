#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

swcResponseChar::swcResponseChar(
    swcDisplayBase
	      *parent,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
    char  *text,
    long   font,
    long   txtcolorarg,
    long   bgcolorarg,
	int  (*actionfunc)(swcResponseChar *win, long arg1, void *arg2),
	void  *arg2)
	: swcResponse(parent, xpos, ypos, width, height, FALSE, text, font,
			txtcolorarg, bgcolorarg, NULL, NULL, 0)

{
	charaction = actionfunc;
	charactarg = arg2;
	charcheck = (int (*)(swcResponse *win, int chr))charcheckfunc;
}

int swcResponseChar::charcheckfunc(
	swcResponseChar *win,
	int              chr)

{
	return ((win->charaction)(win, chr, win->charactarg));
}
