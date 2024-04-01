#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"
#include "scancodes.h"


static void scdflt(int func, long arg2, void *obj);
static int  getcode(int letter);


swcButton::swcButton(
    swcDisplayBase
	      *parent,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
    char  *text,
    long   fontnum,
    long   txtcolorarg,
    long   bgcolorarg,
	int    bitsarg,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3,
	long   arg2,
	void (*scfuncarg)(int func, long arg2, void *obj),
	void  *scobjarg)
	: swcBase(xpos, ypos, width, height, parent)

{
	char  bufr[256];

	long  bits;
	long  xbits;

	letter = 0;
	ignore = false;
	pressed = false;
	gettext(bufr, text);

	anchor = (uchar)((bitsarg >> 20) & 0x0F);
	bits = swcconvertbits(bitsarg, &xbits);
	createwindow("BUTTON", bufr, 0, bits | WS_CHILD | WS_BORDER, xbits);

    type = SWC_WT_BUTTON;
    action = actionfunc;
    SetWindowLong(hwin, GWL_USERDATA, (LONG)this);
    txtcolor = txtcolorarg;
    bgcolor = bgcolorarg;
	arg2data = arg2;
	arg3data = arg3;
    orgproc = (WNDPROC)SetWindowLong(hwin, GWL_WNDPROC, (long)buttonproc);
	SendMessage(hwin, WM_SETFONT, (long)swcgetscnfont(fontnum), FALSE);

	scfunc = (scfuncarg == NULL) ? scdflt : scfuncarg;
	scobj = (scobjarg == NULL) ? this : scobjarg;
	scactive = false;
	if (scfunc != NULL)
	{
		scfunc(0, 0, scobj);
		if (letter != 0)
			scfunc(1, arg2, scobj);
	}
    ShowWindow(hwin, SW_SHOWNORMAL);
}


swcButton::~swcButton()

{
	if (scactive)
		scfunc(2, 0, scobj);
}


void swcButton::SetText(
	char *text)

{
	long saveletter;
	long tmp;
	char bufr[256];

	saveletter = letter;
	letter = 0;
	gettext(bufr, text);
	SetWindowText(hwin, bufr);
	if (saveletter != letter)
	{
		if (saveletter != 0)
		{
			tmp = letter;
			letter = saveletter;
			scfunc(2, arg2data, scobj);
			letter = tmp;
		}
		if (letter != 0)
			scfunc(1, arg2data, scobj);
	}
}


void swcButton::SetState(
	char *text,
	int   state)

{
	pressed = (uchar)state;
	Button_SetState(hwin, state);
	SetText(text);
}


void swcButton::SetShortCut(
	char *text)

{
	long saveletter;
	long tmp;

	saveletter = letter;
	letter = 0;
	if (text != NULL && text[1] != 0)
	{
		if (text[0] == '&')
			letter = getcode(text[1]);
		else if (text[0] == '~')
			letter = getcode(text[1]) | 0x200;
	}
	if (saveletter != letter)
	{
		if (saveletter != 0)
		{
			tmp = letter;
			letter = saveletter;
			scfunc(2, arg2data, scobj);
			letter = tmp;
		}
		if (letter != 0)
			scfunc(1, arg2data, scobj);
	}
}



void swcButton::gettext(
	char *bufr,
	char *text)

{
	char  chr;

	if (text != NULL)
	{
		while ((chr = *text++) != 0)
		{
			if (chr == '&')
			{
				if (*text == '&')
				{
					text++;
					*bufr++ = '&';
				}
				else
					if (letter == 0)
						letter = getcode(*text);
			}
			else if (chr == '~')
			{
				if (*text == '~')
					text++;
				else
				{
					chr = '&';
					if (letter == 0)
						letter = getcode(*text) | 0x200;
				}
			}
			*bufr++ = chr;
		}
	}
	*bufr = 0;
}


static void scdflt(
	int   func,
	long  arg2,
	void *obj)

{
	if (func == 1)
		((swcButton *)obj)->setshortcut(arg2);
	else if (func == 2)
		((swcButton *)obj)->clearshortcut();
}


void swcButton::setshortcut(
	long arg2)

{
	swc_BaseWin.SetGblKeyHook(letter, havekey, arg2, this);
	if ((letter & 0x200) == 0)
		swc_BaseWin.SetGblKeyHook(letter | 0x200, havekey, arg2, this);
	scactive = true;
}


void swcButton::clearshortcut()

{
	swc_BaseWin.RemoveGblKeyHook(letter);
	if ((letter & 0x200) == 0)
		swc_BaseWin.RemoveGblKeyHook(letter | 0x200);
	scactive = false;
}


static int xxxxx;

long APIENTRY swcButton::buttonproc(
    HWND hwnd,
    UINT msg,
    UINT wparam,
    LONG lparam)

{
	swcButton *pwin;

    if ((pwin = (swcButton *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
    {
		switch (msg)
		{
		 case BM_SETSTATE:
			if (pwin->pressed)
				wparam = true;
			break;

		 case WM_CHAR:
			if (pwin->travcheck(wparam))
				return (0);
			if (wparam == 0x0D)
			{
				if (pwin->action != NULL)
					(pwin->action)(pwin, 0x0D, pwin->arg2data, pwin->arg3data);
				return (0);
			}
			break;

		 case WM_LBUTTONDOWN:
			pwin->ignore = false;
			if (pwin->mousefocus)
				pwin->ForceFocus();
			break;

		 case WM_SETFOCUS:
			pwin->checkfocus((HWND)wparam);
			break;

		 case WM_KILLFOCUS:
			if (pwin->pressed)
				Button_SetState(pwin->hwin, true);
			pwin->ignore = true;
			break;
		}
		return (CallWindowProc((pwin->orgproc), hwnd,
				msg, wparam, lparam));
    }
    return (DefWindowProc(hwnd, msg, wparam, lparam));
}


int swcButton::havekey(
	swcBase *win,
	ulong    code,
	long     argi,
	void    *argp)

{
	NOTUSED(win);
	NOTUSED(code);
	NOTUSED(argi);

	((swcButton *)argp)->havekey2();
	return (true);
}


void swcButton::havekey2()

{
	if (!disabled)
		action(this, -1, arg2data, arg3data); 
}


static int getcode(
	int letter)

{
	static uchar isctbl[] =
	{	ISC_A, ISC_B, ISC_C, ISC_D, ISC_E, ISC_F, ISC_G, ISC_H, ISC_I, ISC_J,
		ISC_K, ISC_L, ISC_M, ISC_N, ISC_O, ISC_P, ISC_Q, ISC_R, ISC_S, ISC_T,
		ISC_U, ISC_V, ISC_W, ISC_X, ISC_Y, ISC_Z
	};

	letter = toupper(letter);
	return ((letter < 'A' || letter > 'Z') ? 0 : isctbl[letter - 'A']);
}
