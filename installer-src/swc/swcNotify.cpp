#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <time.h>
#include "xosstuff.h"
#include "xcstring.h"
#include "swcwindows.h"
#include "xosqdes.h"

extern HINSTANCE hinst;

static RECT rect;

swcNotify *swc_NotifyWin = NULL;

swcNotify::swcNotify(
	char  *label,
	int    flag,
	char  *text,
	int    widtharg,
	long   color,
	int    autormvarg,
	int  (*donefuncarg)(int code, void *arg),
	void  *argarg,
	char  *but1text,
	char  *but2text,
	char  *but3text)
	: swcTopLevel(&swc_BaseWin, label, (GetWindowRect(swc_BaseWin.hwin, &rect),
			((rect.right + rect.left)/sf - widtharg)/2),
			((rect.bottom + rect.top)/sf - getheight(flag, text, widtharg))/2,
			widtharg, getheight(flag, text, widtharg), 0x000000, color,
			WS_OVERLAPPED, WS_EX_TOPMOST)

{
	static int butwid[3] = {24, 24, 24};
	static int butpos[3][3] = {{-12, 0, 0}, {-28, 4, 0}, {-40, -12, 16}};

	swcButton *button;
	char      *buttext[3];
	int        numbuts;
	int        num;
	int        xpos;
	int        mode;
	char       bufr[4];

	if (swc_NotifyWin != NULL)
		delete swc_NotifyWin;
	autormv = autormvarg;
	donefunc = donefuncarg;
	arg = argarg;

	EnableWindow(swc_BaseWin.hwin, FALSE);
	if (flag == 0)
	{
		mode = TXT_CENTER;
		xpos = 6;
	}
	else
	{
		mode = TXT_LEFT;
		xpos = (flag > 0) ? xpos = 18 : 4;
	}
	if (flag > 0)
	{
		bufr[0] = flag;
		bufr[1] = 0;
		PutText(4, 1, 10, 0, (flag == '?') ? 0x0000C0 : 0xC00000,
				FONT_VLHEADING, TXT_CENTER, bufr);
	}
	PutText(xpos, 5, widtharg - xpos - 6, height, 0x000000, FONT_MDTEXT, mode,
			text);
	buttext[0] = but1text;
	buttext[1] = but2text;
	buttext[2] = but3text;
	if (but1text != NULL)
	{
		numbuts = (but2text == NULL) ? 1 : (but3text == NULL) ? 2 : 3;
		num = numbuts - 1;
		do
		{
			button = new swcButton(this, (widtharg - 1)/2 +
					butpos[numbuts - 1][num], height/sf - 15,
					butwid[numbuts - 1], 7, buttext[num], FONT_MDTEXT,
					0x000000, 0xD0D0D0, TRUE, havebutton, this, num);
			button->SetTraverse(4, num);
		} while (--num >= 0);
		button->SetFocus();
	}
	swc_NotifyWin = this;
}

swcNotify::~swcNotify()

{
	if (this == swc_NotifyWin)
		swc_NotifyWin = NULL;
	EnableWindow(swc_BaseWin.hwin, TRUE);
}


void swcNotify::Remove()

{
	if (swc_NotifyWin != NULL)
		delete swc_NotifyWin;
}


void swcNotify::ReplaceText(
	char *text)

{
	PutText(0, 5, width/sf - 1, height/sf, 0x000000, FONT_MDTEXT, TXT_CENTER,
			text);
}

int swcNotify::getheight(
	int   flag,
	char *text,
	int   width)

{
	RECT rect;
	HDC  hdc;
	int  rtn;

	static heightval = 0;

	if (heightval != 0)
	{
		rtn = heightval;
		heightval = 0;
		return (rtn);
	}
	rect.left = rect.top = 0;
	rect.right = (width - ((flag == 0) ? 12 : (flag > 0) ?  24 : 10)) * sf;
	rect.bottom = 1;
	hdc = GetDC(swc_BaseWin.hwin);
	DeleteObject(SelectObject(hdc, swcgetscnfont(FONT_MDTEXT)));
	DrawText(hdc, text, -1, &rect, DT_CALCRECT | DT_WORDBREAK | DT_LEFT |
			DT_NOPREFIX);
	DeleteObject(SelectObject(hdc, GetStockFont(SYSTEM_FONT)));
	ReleaseDC(swc_BaseWin.hwin, hdc);
	if ((heightval = rect.bottom/sf + 24) < 32)
		heightval = 32;
	return (heightval);
}


void swcNotify::havebutton(
    swcBase *win,
    long     arg1,
	long     arg2,
	void    *arg3)

{
	int (*func)(int code, void *arg);
	void *arg;

	func = ((swcNotify *)(arg3))->donefunc;
	arg = ((swcNotify *)(arg3))->arg;
	if (((swcNotify *)(arg3))->autormv)
		delete ((swcNotify *)arg3);
	if (func != NULL)
		func(arg2, arg);
}

void swcNotify::haveclose(
	swcTopLevel *win,
	void        *arg)

{
	if (((swcNotify *)arg)->donefunc != NULL)
		((swcNotify *)arg)->donefunc(-1, ((swcNotify *)arg)->arg);
}
