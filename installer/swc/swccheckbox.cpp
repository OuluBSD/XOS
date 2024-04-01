#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

///static long APIENTRY buttonproc(HWND hwnd, UINT msg, UINT wparam, LONG lparam);

swcCheckBox::swcCheckBox(
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
	int    state,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3,
	long   arg2)
	: swcBase(xpos, ypos, width, height, parent)

{
	long bits;

	anchor = (uchar)((state >> 20) & 0x0F);
	bits = WS_CHILD | BS_AUTOCHECKBOX;
	if ((state & SWC_STATE_ENABLE) == 0)
		bits |= WS_DISABLED;
	if (state & SWC_TEXT_LEFT)
		bits |= BS_LEFTTEXT;
	createwindow("BUTTON", text, 0, bits, 0);
    type = SWC_WT_CHECKBOX;
    action = actionfunc;
    SetWindowLong(hwin, GWL_USERDATA, (LONG)this);
    txtcolor = txtcolorarg;
    bgcolor = bgcolorarg;
	arg2data = arg2;
	arg3data = arg3;
    orgproc = (WNDPROC)SetWindowLong(hwin, GWL_WNDPROC, (long)checkboxproc);
	SendMessage(hwin, WM_SETFONT, (long)swcgetscnfont(fontnum), FALSE);
	Button_SetCheck(hwin, (state & SWC_STATE_SET) ? BST_CHECKED :
			BST_UNCHECKED);
    ShowWindow(hwin, SW_SHOWNORMAL);
}


int swcCheckBox::GetState()

{
	return ((Button_GetCheck(hwin) == BST_CHECKED) ? SWC_STATE_SET :
			SWC_STATE_CLEAR);
}


void swcCheckBox::SetState(
    int state)

{
	Button_SetCheck(hwin, (state) ? BST_CHECKED : BST_UNCHECKED);
}


long APIENTRY swcCheckBox::checkboxproc(
    HWND hwnd,
    UINT msg,
    UINT wparam,
    LONG lparam)

{
	swcCheckBox *pwin;

    if ((pwin = (swcCheckBox *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
    {
		switch (msg)
		{
		 case WM_CHAR:
			if (pwin->travcheck(wparam))
				return (0);
			if (wparam == '\r' && pwin->action != NULL)
			{
				(pwin->action)(pwin, '\r', pwin->arg2data, pwin->arg3data);
				return (0);
			}
			break;

		 case WM_LBUTTONDOWN:
			if (pwin->mousefocus)
				pwin->ForceFocus();
			break;

		 case WM_CTLCOLORBTN:
			return ((long)CreateSolidBrush(0x7F0000));

		 case WM_SETFOCUS:
			pwin->checkfocus(HWND(wparam));
			break;
		}
		return (CallWindowProc((pwin->orgproc), hwnd,
				msg, wparam, lparam));
    }
    return (DefWindowProc(hwnd, msg, wparam, lparam));
}


HBRUSH swcCheckBox::ctlbkgcolor(
	HDC hdc)

{
	SetBkMode(hdc, TRANSPARENT);
	return ((HBRUSH)CreateSolidBrush(bgcolor));
}
