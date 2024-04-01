#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

swcDropDown2::swcDropDown2(
	swcDisplayBase
	      *parent,
	long   xpos,
	long   ypos,
	long   width,
	long   height,
	long   fontnum,
	long   txtcolorarg,
	long   bgcolorarg,
	char **contents,
	int    select,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3,
	long   arg2)
	: swcBase(xpos, ypos, width, height, parent)

{
///	char *pnt;

	createwindow("STATIC", "", 0, WS_CHILD | WS_CLIPCHILDREN |
			WS_CLIPSIBLINGS | WS_BORDER /* | SS_SUNKEN */, WS_EX_CLIENTEDGE);
	frame = hwin;
	ShowWindow(frame, SW_SHOWNORMAL);
	hwin = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_CLIPCHILDREN |
			WS_CLIPSIBLINGS|ES_MULTILINE, 0, 0, (width + sfr)/sfd - 6,
			(height + sfr)/sfd - 6, frame, 0, hinst, 0);

	list = CreateWindowEx(0, "LISTBOX", "xxx", WS_CHILD | WS_CLIPCHILDREN |
			WS_CLIPSIBLINGS, (xpos + sfr)/sfd, (ypos - 20 + sfr) /sfd,
			(width + sfr)/sfd - 6, 200, parent->hwin, 0, hinst, 0);
	ShowWindow(list, SW_SHOWNORMAL);

/*
	createwindow("COMBOBOX", "", 0, WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
			WS_EX_CLIENTEDGE);
	type = SWC_WT_DROPDOWN2;
	action = actionfunc;
    SetWindowLong(hwin, GWL_USERDATA, (LONG)this);
    txtcolor = txtcolorarg;
    bgcolor = bgcolorarg;
	arg2data = arg2;
	arg3data = arg3;
	SendMessage(hwin, WM_SETFONT, (long)swcgetscnfont(fontnum), FALSE);
	if (contents != NULL)
	{
		while ((pnt = *contents++) != NULL)
			ComboBox_AddString(hwin, pnt);
		ComboBox_SetCurSel(hwin, select);
	}
*/
    orgproc = (WNDPROC)SetWindowLong(hwin, GWL_WNDPROC, (long)dropdown2proc);
    ShowWindow(hwin, SW_SHOWNORMAL);
}

long APIENTRY swcDropDown2::dropdown2proc(
    HWND hwnd,
    UINT msg,
    UINT wparam,
    LONG lparam)

{
	swcDropDown2 *pwin;

    if ((pwin = (swcDropDown2 *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
    {
		switch (msg)
		{
		 case WM_CHAR:
			if (pwin->travcheck(wparam))
				return (0);
			if (pwin->action != NULL)
			{
				(pwin->action)(pwin, wparam, pwin->arg2data,
						pwin->arg3data);
				return (0);
			}
			break;

		 case WM_LBUTTONDOWN:
			if (pwin->mousefocus)
				pwin->ForceFocus();
			break;

		 case WM_SETFOCUS:
			pwin->checkfocus(HWND(wparam));
			break;
		}
		return (CallWindowProc((pwin->orgproc), hwnd,
				msg, wparam, lparam));
    }
    return (DefWindowProc(hwnd, msg, wparam, lparam));
}

int swcDropDown2::GetSelection()

{
	return (ComboBox_GetCurSel(hwin));
}

int swcDropDown2::SetSelection(
	int sel)

{
	return (ComboBox_SetCurSel(hwin, sel));
}


int swcDropDown2::PutItem(
	char *str)

{
	return (ComboBox_AddString(hwin, str));
}
