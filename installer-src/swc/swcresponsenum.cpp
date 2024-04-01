#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

swcResponseNum::swcResponseNum(
    swcDisplayBase
	      *parent,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
    int    hidetext,
    char  *text,
    long   font,
    long   txtcolorarg,
    long   bgcolorarg,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3,
	long   arg2)
	: swcResponse(parent, xpos, ypos, width, height, hidetext, text, font,
			txtcolorarg, bgcolorarg, actionfunc, arg3, arg2)

{
	charcheck = numcheck;
}

int swcResponseNum::GetValue(
	long *value)

{
	char text[64];

	if (GetWindowText(hwin, text, 64) <= 0)
		return (FALSE);
	*value = atol(text);
	return (TRUE);
}

int swcResponseNum::numcheck(
	swcResponse *win,
	int          chr)

{
	NOTUSED(win);

	return (chr != 0x0D && chr != 0x08 && chr != '-' && !isdigit(chr));
}
