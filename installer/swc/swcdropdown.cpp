#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "scancodes.h"
#include "swcwindows.h"

// arg3 values for the action function:
//   >= 0 - Character typed
//     -1 - Lost focus
//     -2 - ???
//     -3 - Selection changed
//     -4 - Lost focus
//     -5 - Got focus
//     -6 - List hidden

swcDropDown *swcDropDown::ddfirst;


swcDropDown::swcDropDown(
	swcDisplayBase
	      *parent,
	long   xpos,
	long   ypos,
	long   width,
	long   height,
	long   ddheight,
	long   fontnum,
	long   txtcolorarg,
	long   bgcolorarg,
	char **contents,
	int    select,
	long   bitsarg,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3,
	long   arg2)
	: swcBase(xpos, ypos, width, ddheight, parent)

{
	char *pnt;
///	char  text[200];
///	char  chr;
	long  wbits;

	wbits = WS_CHILD | WS_BORDER;
	if (bitsarg & SWC_SCROLL_VERT_A)	// Vertical scroll bar (always
										//   displayed)?
		wbits |= (WS_VSCROLL | LBS_DISABLENOSCROLL);
	else if (bitsarg & SWC_SCROLL_VERT_C) // Vertical scroll bar (conditionally
		wbits |= WS_VSCROLL;			  //   displayed)?
	wbits |= (bitsarg & SWC_DROPDOWN_EDIT) ? CBS_DROPDOWN : CBS_DROPDOWNLIST;

///	createwindow("COMBOBOX", "", 0, WS_VSCROLL | (bitsarg & SWC_DROPDOWN_EDIT) ?
///			(WS_CHILD | WS_BORDER | CBS_DROPDOWN) : (WS_CHILD | WS_BORDER |
///			CBS_DROPDOWNLIST), WS_EX_CLIENTEDGE);

	createwindow("COMBOBOX", "", 0, wbits, WS_EX_CLIENTEDGE);

	type = SWC_WT_DROPDOWN;
	bits = bitsarg;
	action = actionfunc;
    SetWindowLong(hwin, GWL_USERDATA, (LONG)this);
	wchild = GetWindow(hwin, GW_CHILD);
    txtcolor = txtcolorarg;
    bgcolor = bgcolorarg;
	arg2data = arg2;
	arg3data = arg3;
    changed = false;
	SendMessage(hwin, WM_SETFONT, (long)swcgetscnfont(fontnum), FALSE);
	numitems = 0;
	if (contents != NULL)
	{
		while ((pnt = *contents++) != NULL)
		{
///			while ((chr = *pnt) != NULL && isspace(*pnt))
///				pnt++;
///			if (chr == 0)
///				text[0] = 0;
///			else
///			{
///				text[0] = ' ';
///				strncpyn(text + 1, pnt, 198);
///			}
			ComboBox_AddString(hwin, pnt);
			numitems++;
		}
		cursel = select;
		ComboBox_SetCurSel(hwin, select);
	}
	ComboBox_SetItemHeight(hwin, -1, (height + sfr)/sfd - 6);
	ddnext = ddfirst;					// Set height of the actual window
	ddfirst = this;
    orgproc = (WNDPROC)SetWindowLong(hwin, GWL_WNDPROC, (long)dropdownproc);
	if (wchild != NULL)
	{
		SetWindowLong(wchild, GWL_USERDATA, (long)this);
		editproc = (WNDPROC)SetWindowLong(wchild, GWL_WNDPROC,
				(long)ddeditproc);
	}
    ShowWindow(hwin, SW_SHOWNORMAL);
}


swcDropDown::~swcDropDown()

{
	swcDropDown **ppwin;
	swcDropDown  *pwin;

	ppwin = &ddfirst;
	while ((pwin = *ppwin) != NULL)
	{
		if (pwin == this)
		{
			*ppwin = ddnext;
			return;
		}
		ppwin = &(pwin->ddnext);
	}
}


long APIENTRY swcDropDown::dropdownproc(
    HWND hwnd,
    UINT msg,
    UINT wparam,
    LONG lparam)

{
	swcDropDown *pwin;
///	char xxx[64];

    if ((pwin = (swcDropDown *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
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

		 case WM_KILLFOCUS:
///			GetWindowText(hwnd, xxx, 32);
			if (pwin->action != NULL)
				(pwin->action)(pwin, -4, pwin->arg2data, pwin->arg3data);
			break;

		 case WM_SETFOCUS:
			pwin->checkfocus(HWND(wparam));
			if (pwin->action != NULL)
				(pwin->action)(pwin, -5, pwin->arg2data, pwin->arg3data);
			break;
		}
		return (CallWindowProc((pwin->orgproc), hwnd,
				msg, wparam, lparam));
    }
    return (DefWindowProc(hwnd, msg, wparam, lparam));
}


long APIENTRY swcDropDown::ddeditproc(
    HWND hwnd,
    UINT msg,
    UINT wparam,
    LONG lparam)

{
	swcDropDown *pwin;

	pwin = ddfirst;
	while (pwin != NULL)
	{
		if (pwin->wchild == hwnd)
		{
			switch (msg)
			{
			 case WM_CHAR:
				if (pwin->travcheck(wparam))
					return (0);
				if (wparam == '\r' && pwin->action != NULL)
				{
					if (pwin->changed)
					{
						(pwin->action)(pwin, -2, pwin->arg2data,
								pwin->arg3data);
						pwin->changed = false;
					}
					(pwin->action)(pwin, wparam, pwin->arg2data,
							pwin->arg3data);
					return (0);
				}
				if (pwin->cursel != (pwin->numitems - 1))
					return (0);
				break;

			 case WM_KILLFOCUS:
				if (pwin->changed)
				{
					pwin->addtolist();
					if (pwin->action != NULL)
						(pwin->action)(pwin, -1, pwin->arg2data,
								pwin->arg3data);
				}
				break;
			}
			return (CallWindowProc((pwin->editproc), hwnd,
					msg, wparam, lparam));
		}
		pwin = pwin->ddnext;
	}
    return (DefWindowProc(hwnd, msg, wparam, lparam));
}


int swcDropDown::processcmdmsg(
    UINT wparam)

{
	int num;

	if (HIWORD(wparam) == CBN_EDITUPDATE)
	{
	}
	else if (HIWORD(wparam) == CBN_EDITCHANGE)
	{
		if (cursel >= 0)
		{
			if (cursel < numitems - 1)
			{
				numitems--;
				RemoveItem(cursel);
				::PostMessage(NULL, WM_USER + 51, cursel, (long)this);
			}
			else
				changed = true;
		}
	}
	else if (HIWORD(wparam) == CBN_CLOSEUP)
	{
				if (action != NULL)
				action(this, -6, arg2data, arg3data);
	}
	else if (HIWORD(wparam) == CBN_SELCHANGE)
	{
		if ((num = ComboBox_GetCurSel(hwin)) >= 0)
			cursel = num;
	}
	return (false);
}


int swcDropDown::ddcheckforkey(
	int code)

{
	if ((bits & SWC_DROPDOWN_EDIT) && changed && (code == ISC_UPARW ||
			code == ISC_DNARW))
		return (addtolist());
	return (false);
}


int swcDropDown::addtolist()

{
	char *pnt1;
	char *pnt2;
	int   inx;
	char  text1[200];
	char  text2[200];
	char  chr;

	GetText(text1 + 1, 198);
	if (bits & SWC_DROPDOWN_FRCUC)
		_strupr(text1 + 1);
	pnt1 = text1 + 1;
	while ((chr = *pnt1) != NULL && isspace(chr))
		pnt1++;
	if (chr != 0)
	{
		*(--pnt1) = ' ';
		inx = 0;
		do
		{
			ComboBox_GetLBText(hwin, inx, text2);
			pnt2 = text2;
			while ((chr = *pnt2) != NULL && isspace(*pnt2))
				pnt2++;
			if (pnt2[0] == 0 || _stricmp(pnt1 + 1, pnt2) < 0)
				break;
		} while (++inx < numitems);
		ComboBox_InsertString(hwin, inx, pnt1);
		ComboBox_ShowDropdown(hwin, false);
		ComboBox_ShowDropdown(hwin, true);
		cursel = inx;
		::PostMessage(NULL, WM_USER + 51, cursel, (long)this);
		numitems++;
		changed = false;
		return (true);
	}
	return (false);
}


int swcDropDown::GetSelection()

{
	return (ComboBox_GetCurSel(hwin));
}


int swcDropDown::SetSelection(
	int sel)

{
	cursel = sel;
	return (ComboBox_SetCurSel(hwin, sel));
}


int swcDropDown::GetItem(
	int   inx,
	char *bufr,
	int   len)

{
	if (ComboBox_GetLBTextLen(hwin, inx) >= len)
		return (false);
	return (ComboBox_GetLBText(hwin, inx, bufr));
}


int swcDropDown::PutItem(
	char *str)

{
	return (ComboBox_AddString(hwin, str));
}


int swcDropDown::InsertItem(
	int   inx,
	char *str)

{
	return (ComboBox_InsertString(hwin, inx, str));
}


int swcDropDown::RemoveItem(
	int index)

{
///	uchar *pnt;

///	if ((pnt = (uchar *)ComboBox_GetItemData(hwin, index)) == (uchar *)-1)
///		return (false);
	ComboBox_DeleteString(hwin, index);
	ComboBox_ShowDropdown(hwin, false);
	ComboBox_ShowDropdown(hwin, true);
	return (true);
}


int swcDropDown::FindString(
	int   start,
	char *str)

{
	return (ComboBox_FindString(hwin, start, str));
}

int swcDropDown::FindStringExact(
	int   start,
	char *str)

{
	return (ComboBox_FindStringExact(hwin, start, str));
}


int swcDropDown::ShowDropDown(
	int state)

{
	return (ComboBox_ShowDropdown(hwin, state));
}
