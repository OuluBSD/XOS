#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"


swcEdit::swcEdit(
    swcDisplayBase
	      *parent,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
    int    options,
    long   fontnum,
    long   txtcolorarg,
    long   bgcolorarg,
    char  *text,
	long   bits,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3,
	long   arg2)
	: swcBase(xpos, ypos, width, height, parent)

{
    RECT rect;
	long xbits;

	anchor = (uchar)((bits >> 20) & 0x0F);
	bits = swcconvertbits(bits, &xbits);
	if (options & SWC_EDIT_READONLY)
		bits |= ES_READONLY;
	createwindow("EDIT", text, 0, bits | WS_CHILD | ES_LEFT | ES_MULTILINE |
			ES_WANTRETURN, xbits);
	type = SWC_WT_EDIT;
    action = actionfunc;
    SetWindowLong(hwin, GWL_USERDATA, (LONG)this);
    txtcolor = txtcolorarg;
    bgcolor = bgcolorarg;
	arg2data = arg2;
	arg3data = arg3;
    orgproc = (WNDPROC)SetWindowLong(hwin, GWL_WNDPROC, (long)editproc);
	SendMessage(hwin, WM_SETFONT, (long)swcgetscnfont(fontnum), false);
	rect.left = 4;
	rect.right = (width + sfr)/sfd - 4;
	rect.top = 4;
	rect.bottom = (height + sfr)/sfd - 4;
	SendMessage(hwin, EM_SETRECT, 0, (long)&rect);
    ShowWindow(hwin, SW_SHOWNORMAL);
}


void swcEdit::SetText(
	char *text)

{
	Edit_SetText(hwin, text);

///	Edit_GetText(hwin, text, 123);
}


long APIENTRY swcEdit::editproc(
    HWND hwnd,
    UINT msg,
    UINT wparam,
    LONG lparam)

{
	swcEdit *pwin;

    if ((pwin = (swcEdit *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
    {
		switch (msg)
		{
		 case WM_CHAR:
			if (pwin->travcheck(wparam))
				return (0);
			break;

		 case WM_LBUTTONDOWN:
			if (pwin->mousefocus)
				pwin->ForceFocus();
			break;

		 case WM_SETFOCUS:
			pwin->checkfocus(HWND(wparam));
			break;
		}
		return (CallWindowProc((pwin->orgproc),
				hwnd, msg, wparam, lparam));
    }
    return (DefWindowProc(hwnd, msg, wparam, lparam));
}
