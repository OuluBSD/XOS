#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

swcTabDisp::swcTabDisp()
{
}

swcTabDisp::swcTabDisp(
    swcDisplayBase
	      *parent,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
	long   xpad,
	long   ypad,
	long   fontnum,
    long   txtcolorarg,
    long   bgcolorarg,
	long   bits,
	void (*actionarg)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3arg,
	int    arg2arg)
	: swcDisplayBase(WC_TABCONTROL,
			"", parent, xpos, ypos, width, height, txtcolorarg,
			bgcolorarg, bits, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			actionarg, arg3arg, arg2arg)
{
	type = SWC_WT_TABDISP;
	havetab = false;
    orgproc = (WNDPROC)SetWindowLong(hwin, GWL_WNDPROC, (long)tabproc);
	TabCtrl_SetPadding(hwin, (xpad + sfr) / sfd, (ypad + sfr) / sfd);
	SendMessage(hwin, WM_SETFONT, (long)swcgetscnfont(fontnum), FALSE);
}


int swcTabDisp::AddTab(
	int  index,
	char *text,
	int  image)
{
	TCITEM item;
	RECT   rect;
	int    rtn;

	item.mask = TCIF_TEXT | TCIF_IMAGE; 
	item.pszText = text;
	item.iImage = image;
	item.lParam = 0;
	if ((rtn = TabCtrl_InsertItem(hwin, index, &item)) >= 0 && !havetab)
	{
		::GetWindowRect(hwin, &rect);
		rect.right -= rect.left;
		rect.left = 0;
		rect.bottom -= rect.top;
		rect.top = 0;

///		GetClientRect(&rect);

		TabCtrl_AdjustRect(hwin, false, &rect);
		xoffset = -rect.left * sfd;
		yoffset = -rect.top * sfd;
		havetab = true;
	}
	return (rtn);
}


int swcTabDisp::SetSelection(
	int inx)
{
	return (TabCtrl_SetCurSel(hwin, inx));
}


int swcTabDisp::GetClientRect(
	RECT *rect)
{
	::GetWindowRect(hwin, rect);
	rect->right -= rect->left;
	rect->left = 0;
	rect->bottom -= rect->top;
	rect->top = 0;
	TabCtrl_AdjustRect(hwin, false, rect);
	rect->top = rect->top * sfd + yoffset;
	rect->bottom = rect->bottom * sfd + yoffset;
	rect->left = rect->left * sfd + xoffset;
	rect->right = rect->right * sfd + xoffset;;
	return (true);
}



long APIENTRY swcTabDisp::tabproc(
    HWND hwnd,
    UINT msg,
    UINT wparam,
    LONG lparam)

{
	swcButton *pwin;
	HBRUSH     hbrush;
	RECT       rect;

    if ((pwin = (swcButton *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
    {
		switch (msg)
		{
		 case WM_ERASEBKGND:
		    if ((hbrush = CreateSolidBrush(pwin->bgcolor)) != 0)
			{
				GetClipBox((HDC)wparam, &rect);
				FillRect((HDC)wparam, &rect, hbrush);
				DeleteObject(hbrush);
			}
			return (TRUE);
		}
		return (CallWindowProc((pwin->orgproc), hwnd,
				msg, wparam, lparam));
    }
    return (DefWindowProc(hwnd, msg, wparam, lparam));
}



void swcTabDisp::selchanged(
	int index)
{
	if (action != NULL)
		action(this, index, arg2data, arg3data);
}
