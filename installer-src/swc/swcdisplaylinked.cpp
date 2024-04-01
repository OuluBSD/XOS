#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

static void doxlink(void *arg, int xpos);

swcDisplayLinked::swcDisplayLinked(
    swcDisplayBase
	        *parent,
	swcList *link,
    long     xpos,
    long     ypos,
    long     width,
    long     height,
    long     txtcolorarg,
    long     bgcolorarg,
	long     bits)
	: swcDisplay(parent, xpos, ypos, width, height, txtcolorarg, bgcolorarg,
			bits)

{
	listwin = link;
	link->setxlink(doxlink, this);
}


swcDisplayLinked::~swcDisplayLinked()

{
	if (listwin != NULL)
		listwin->clrxlink(this);
}


void swcDisplayLinked::unlink(
	swcList *listarg)

{
	if (listwin == listarg)
		listwin = NULL;
}


static void doxlink(
	void *arg,
	int   xpos)

{
	((swcDisplayLinked *)arg)->xoffset = xpos;
	((swcDisplayLinked *)arg)->adjustchilds();
	InvalidateRect(((swcDisplayLinked *)arg)->hwin, NULL, TRUE);
}

