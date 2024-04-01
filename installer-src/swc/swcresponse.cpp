#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"


swcResponse::swcResponse(
    swcDisplayBase *parent,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
    int    optionsarg,
    char  *text,
    long   fontnum,
    long   txtcolorarg,
    long   bgcolorarg,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3,
	long   arg2)
	: swcBase(xpos, ypos, width, height, parent)
///	: swcBase("EDIT", text, xpos, ypos, width, height, WS_CHILD|///WS_BORDER|
///			ES_LEFT|((options & SWCRESPONSE_HIDETEXT) ? ES_PASSWORD :
///			ES_MULTILINE), WS_EX_STATICEDGE, parent)

// Note: We would like to be able to set margins in the single line edit
//       window created here. Unfortunately, windows does not allow margins
//       to be set unless we have a multi-line edit window. Also, a
//       multi-line edit window does not support hiding characters.  Thus
//       we compromise somewhat. If text is not hidden, we actually create
//       a multi-line edit window but only use one line and set margins.
//       If text is hidden, we create a single line edit window, hide the
//       characters, and don't set margins!

{
    RECT rect;
	int  fsz;
	int  bits;

extern HINSTANCE hinst;

	options = optionsarg;
	createwindow("STATIC", text, 0, WS_CHILD | WS_CLIPCHILDREN |
			WS_CLIPSIBLINGS | WS_BORDER /* | SS_SUNKEN */, WS_EX_CLIENTEDGE);
	frame = hwin;
	charcheck = NULL;
	ShowWindow(frame, SW_SHOWNORMAL);
	bits = (options & SWC_RESPONSE_HIDETEXT) ?
			(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|ES_PASSWORD) :
			(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|ES_MULTILINE);
	if (options & SWC_RESPONSE_READONLY)
		bits |= ES_READONLY;
	if (options & (SWC_SCROLL_HORIZ_A|SWC_SCROLL_HORIZ_C))
		bits |= ES_AUTOHSCROLL;
	hwin = CreateWindowEx(0, "EDIT", text, bits, 0, 0,
			(width + sfr) / sfd - 6, (height + sfr) / sfd - 6, frame, 0, hinst, 0);
	type = SWC_WT_RESPONSE;
    action = actionfunc;
    SetWindowLong(hwin, GWL_USERDATA, (LONG)this);
    txtcolor = txtcolorarg;
    bgcolor = bgcolorarg;
	arg2data = arg2;
	arg3data = arg3;
    orgproc = (WNDPROC)SetWindowLong(hwin, GWL_WNDPROC, (long)responseproc);
	SendMessage(hwin, WM_SETFONT, (long)swcgetscnfont(fontnum), FALSE);
	if ((options & SWC_RESPONSE_HIDETEXT) == 0)
	{
		fsz = ((fontnum & 0xFFFF) + sfr) / sfd;
		rect.left = fsz / 3;
		rect.right = (width + sfr) / sfd - fsz / 3 - 4;
		rect.top = ((height + sfr) / sfd - fsz - 4) / 2;
		rect.bottom = ((height + sfr) / sfd + fsz - 4) / 2;
		SendMessage(hwin, EM_SETRECT, 0, (long)&rect);
	}
    ShowWindow(hwin, SW_SHOWNORMAL);
}

swcResponse::~swcResponse()

{
	HFONT font;

	font = GetWindowFont(hwin);
	SetWindowFont(hwin, GetStockFont(SYSTEM_FONT), FALSE);
	DeleteObject(font);
	DestroyWindow(hwin);
	hwin = 0;
	DestroyWindow(frame);
}

long APIENTRY swcResponse::responseproc(
    HWND hwnd,
    UINT msg,
    UINT wparam,
    LONG lparam)

{
	swcResponse *pwin;

    if ((pwin = (swcResponse *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
    {
		switch (msg)
		{
		 case WM_CHAR:
			if (pwin->travcheck(wparam))
				return (0);
			if (wparam == 0x0D)
			{
				if (pwin->charcheck != NULL && (pwin->charcheck)(pwin, 0x0D))
					return (0);
				if (pwin->action != NULL)
					(pwin->action)(pwin, 0x0D, pwin->arg2data, pwin->arg3data);
				return (0);
			}
			else if (wparam == 0x09)
			{
				if (pwin->action != NULL)
					(pwin->action)(pwin, (GetKeyState(VK_SHIFT) & 0x80) ?
							0x89 : 0x09, pwin->arg2data, pwin->arg3data);
				return (0);
			}
			if (pwin->charcheck != NULL && (pwin->charcheck)(pwin, wparam))
				return (0);
			break;

		 case WM_LBUTTONDOWN:
			if (pwin->mousefocus)
				pwin->ForceFocus();
			break;

		 case WM_SETFOCUS:
			if (pwin->checkfocus(HWND(wparam)) && (pwin->options &
					SWC_RESPONSE_SELALL))
				Edit_SetSel(pwin->hwin, 0, 0x7FFF);
			break;
		}
		return (CallWindowProc((pwin->orgproc),
				hwnd, msg, wparam, lparam));
    }
    return (DefWindowProc(hwnd, msg, wparam, lparam));
}

void swcResponse::SetText(
	char *text)

{
	Edit_SetText(hwin, text);
}

void swcResponse::SelectAll()

{
	Edit_SetSel(hwin, 0, 0x7FFF);
}
