#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xosstuff.h"
#include "xcstring.h"
///#include "scancodes.h"
#include "swcwindows.h"

extern HINSTANCE hinst;


swcBase::swcBase()

{
	toplvl = (swcTopLevel *)this;
}


swcBase::swcBase(
	long            xposarg,
	long            yposarg,
	long            widtharg,
	long            heightarg,
	swcDisplayBase *parentarg)

{
	xpos = xposarg;
	ypos = yposarg;
	width = widtharg;
	height = heightarg;
	parent = parentarg;
	travgroup = travnumber = 0;
	wchild = 0;
	mousefocus = swc_MouseFocus;
	memset(travlist, 0, sizeof(travlist));
	travwin = NULL;
	hwin = NULL;
	dieing = false;
	if (parent != NULL)
	{
		sibling = parent->fchild;
		parent->fchild = this;
		toplvl = parent->toplvl;
	}
	else
	{
		sibling = NULL;
		toplvl = (swcTopLevel *)this;
	}
	travlist[0] = travlist[1] = travlist[2] = travlist[3] =
			travlist[4] = NULL;
	travwin = travnext = travprev = NULL;
	gkhook = NULL;
	gblfunc = NULL;
	gblargp = NULL;
}


swcBase::~swcBase()

{
	HFONT     font;
	swcBase **pnt;
	keyhook  *tmppnt;

	dieing = true;
	if (toplvl->ffwindow == this)
		toplvl->ffwindow = NULL;
	travremove();						// Remove from traverse list
	while (gkhook != NULL)
	{
		tmppnt = gkhook->next;
		delete gkhook;
		gkhook = tmppnt;
	}
	if (parent != NULL)
	{
		pnt = &(parent->fchild);		// First remove this window fr0m its
		do								//   parent's list for child windows
		{
			if (*pnt == this)
			{
				*pnt = sibling;
				break;
			}
		} while ((pnt = &((*pnt)->sibling)) != NULL);
	}
	if (swc_WindowGone != NULL)
		swc_WindowGone(this);
	if (hwin != NULL)
	{
		font = GetWindowFont(hwin);
		SetWindowFont(hwin, GetStockFont(SYSTEM_FONT), FALSE);
		DeleteObject(font);
		DestroyWindow(hwin);			// Finally, destroy the linked
	}									//   physical window
}


long winerr;

void swcBase::createwindow(
	char           *winclass,
	char           *wintext,
	long            ctlid,
	long            bits,
	long            exbits)

{
	if (parent == &swc_BaseWin)
	{
		xpos += swcbasexoffset;
		ypos += swcbaseyoffset;
	}
	xanchor = parent->cwidth - xpos - width;
	yanchor = parent->cheight - ypos - height;
    hwin = CreateWindowEx(exbits, winclass, wintext, bits | WS_CLIPCHILDREN |
			WS_CLIPSIBLINGS, (((parent != NULL) ? (xpos - parent->xoffset) :
			xpos) + sfr) / sfd, (((parent != NULL) ?
			(ypos - parent->yoffset) : ypos) + sfr) / sfd, (width + sfr)/sfd,
			(height + sfr) / sfd, (parent != NULL) ? parent->hwin : NULL,
			(HMENU)ctlid, hinst, 0);
	if (hwin == 0)
		winerr = GetLastError();
	disabled = ((bits & WS_DISABLED) != 0);
}

void swcBase::ForceRedraw()

{
	InvalidateRect(hwin, NULL, TRUE);
}

int swcBase::GetClientRect(
    RECT   *rect)

{
	int rtn;

    rtn = ::GetClientRect(hwin, rect);
	rect->top *= sfd;
	rect->bottom *= sfd;
	rect->left *= sfd;
	rect->right *= sfd;
	return (rtn);
}


int swcBase::GetUnscaledClientRect(
    RECT   *rect)

{
	return (::GetClientRect(hwin, rect));
}


int swcBase::GetWindowRect(
    RECT   *rect)

{
	int rtn;

    rtn = ::GetWindowRect(hwin, rect);
	rect->top *= sfd;
	rect->bottom *= sfd;
	rect->left *= sfd;
	rect->right *= sfd;
	return (rtn);
}


int swcBase::SetEnabled(
	int enablearg)

{
	disabled = (uchar)!enablearg;
	return (EnableWindow(hwin, enablearg));
}


int swcBase::Visible(
	int visible)

{
	return (::ShowWindow(hwin, (visible) ? SW_SHOWNORMAL : SW_HIDE));
}


void swcBase::BringToTop()

{
	BringWindowToTop(hwin);
}

void swcBase::SetFocus()

{
	toplvl->ffwindow = NULL;
    ::SetFocus(hwin);
}

/// extern char swc_rptfocus;

void swcBase::ForceFocus()

{
	toplvl->ffwindow = (swc_ForceFocusEnable) ? this : NULL;
	if (wchild == NULL || wchild != GetFocus())
	{
///		swc_rptfocus = false;
///		::SetFocus(toplvl->hwin);
///		swc_rptfocus = true;
///		::BringWindowToTop(hwin);
		::SetFocus(hwin);
	}
}

void swcBase::SetFont(
	int fontnum)

{
	HFONT font;

	font = GetWindowFont(hwin);
	SetWindowFont(hwin, swcgetscnfont(fontnum), FALSE);
	DeleteObject(font);
}


int swcBase::GetText(
	char *bufr,
	int   size)

{
	return (GetWindowText(hwin, bufr, size));
}


int swcBase::GetTextLength()

{
	return (GetWindowTextLength(hwin));
}


int swcBase::SetText(
	char *bufr)

{
	return (SetWindowText(hwin, bufr));
}


int swcBase::MoveResize(
	long xposarg,
	long yposarg,
	long widtharg,
	long heightarg,
	int  repaint)

{
///	RECT       rect;

   	xpos = xposarg;
   	ypos = yposarg;
    if (parent == &swc_BaseWin)
    {
    	xpos += swcbasexoffset;
    	ypos += swcbaseyoffset;
	}
	width = widtharg;
	height = heightarg;
	if (!MoveWindow(hwin, (xpos + sfr)/sfd, (ypos + sfr)/sfd,
			(width + sfr)/sfd, (height + sfr)/sfd, repaint))
		return (false);
///	::GetClientRect(hwin, &rect);
	this->sizefixup();
	return (true);
}


int swcBase::Move(
	long xposarg,
	long yposarg,
	int  repaint)

{
	repaint = repaint;

	xpos = xposarg;
	ypos = yposarg;
	return (SetWindowPos(hwin, NULL, (xpos + sfr)/sfd, (ypos + sfr)/sfd, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE));
}


void swcBase::SetColors(
	long txtcolorarg,
	long bkcolorarg)

{
	txtcolor = txtcolorarg;
	bgcolor = bkcolorarg;
	InvalidateRect(hwin, NULL, true);
}


void swcBase::SetMouseFocus(
	int value)

{
	mousefocus = (uchar)value;
}


int swcBase::GetTextHeight(
	int   fontnum,
	char *text,
	int   len,
	int   width)

{
	RECT rect;
	HDC  hdc;

	rect.left = rect.top = 0;
	rect.right = (width + sfr) / sfd;
	rect.bottom = 1; 
	hdc = GetDC(hwin);
	DeleteObject(SelectObject(hdc, swcgetscnfont(fontnum)));
	DrawText(hdc, text, len, &rect, DT_CALCRECT | DT_WORDBREAK | DT_LEFT |
			DT_NOPREFIX);
	DeleteObject(SelectObject(hdc, GetStockFont(SYSTEM_FONT)));
	ReleaseDC(hwin, hdc);
	return (rect.bottom * sfd);
}


int swcBase::SetTraverse(
	uint group,
	uint number,
	int  travrtnarg)

{
	return (SetTraverse(parent, group, number, travrtnarg));
}


int swcBase::SetTraverse(
	swcDisplayBase *win,
	uint            group,
	uint            number,
	int             travrtnarg)

{
	swcBase *tnwin;
	swcBase *tpwin;

	if (group > 4)
		return (-1);
	travremove();
	travrtn = (uchar)travrtnarg;
	if (group == 0)
		return (0);
	if (number > 255)
		number = 255;
	if (win->travlist[group] == NULL)
	{
		travnext = travprev = this;
		win->travlist[group] = this;
	}
	else
	{
		tnwin = win->travlist[group];
		tpwin = tnwin->travprev;
		do
		{
			if (number < tnwin->travnumber)
				break;
			tpwin = tnwin;
			tnwin = tnwin->travnext;
		} while (tnwin != win->travlist[group]);
		travnext = tnwin;
		travprev = tpwin;
		tnwin->travprev = this;
		tpwin->travnext = this;
		if (number < win->travlist[group]->travnumber)
			win->travlist[group] = this;
	}
	travwin = win;
	travgroup = (uchar)group;
	travnumber = (uchar)number;
	return (0);
}

void swcBase::travremove()

{
	if (travwin == NULL || travgroup == 0)
		return;
	if (travnext == this)
		travwin->travlist[travgroup] = NULL;
	else
	{
		travnext->travprev = travprev;
		travprev->travnext = travnext;
		if (travwin->travlist[travgroup] == this)
			travwin->travlist[travgroup] = travnext;
	}
	travwin = NULL;
	travgroup = 0;
	travnext = travprev = NULL;
}

int swcBase::travcheck(
	int chr)

{
	if (travgroup == 0)
		return (FALSE);
	if (travrtn && chr == 0x0D)
	{
		setfocustonext();
		return (TRUE);
	}
    if (chr == 0x09)
	{
		if (GetKeyState(VK_SHIFT) & 0x80)
			setfocustoprev();
		else
			setfocustonext();
		return (TRUE);
	}
	return (FALSE);
}

void swcBase::setfocustoprev(void)

{
	swcBase *win;

	win = travprev;
	while (win->disabled)
	{
		win = win->travprev;
		if (win == this)
			return;
	}
	if (toplvl->ffwindow == this)
		win->ForceFocus();
	else
		win->SetFocus();
}

void swcBase::setfocustonext(void)

{
	swcBase *win;

	win = travnext;
	while (win->disabled)
	{
		win = win->travnext;
		if (win == this)
			return;
	}
	if (toplvl->ffwindow == this)
		win->ForceFocus();
	else
		win->SetFocus();
}

void swcBase::SetGblKeyHook(
	int   code,
	int (*func)(swcBase *win, ulong data, long argi, void *argp),
	long  argi,
	void *argp)

{
	keyhook *hpnt;

	hpnt = gkhook;
	while (hpnt != NULL && hpnt->code != code)
		hpnt = hpnt->next;
	if (hpnt == NULL)
	{
		hpnt = new keyhook;
		hpnt->code = code;
		hpnt->next = gkhook;
		gkhook = hpnt;
	}
	hpnt->func = func;
	hpnt->argi = argi;
	hpnt->argp = argp;
}


int swcBase::RemoveGblKeyHook(
	int   code)

{
	keyhook **ppnt;
	keyhook  *hpnt;

	ppnt = &gkhook;
	while ((hpnt = *ppnt) != NULL)
	{
		if (hpnt->code == code)
		{
			*ppnt = hpnt->next;
			delete hpnt;
			return (true);
		}
		ppnt = &(hpnt->next);
	}
	return (false);
}


//*******************************************************
// Function: checkforkey - Check for globally hooked key
// Returned: TRUE if hooked (already processed), FALSE if
//				normal or if rejected
//*******************************************************

// This function is called in the window which initially received the
//   keyboard character message

int swcBase::checkforkey(
	long wval)

{
	swcBase *wpnt;
	keyhook *hpnt;
	long     code;

	if ((wval & 0xFFFFFF00) != 0 || (code = swcvirkey2xos[wval]) == 0 ||
			(code & 0x80) != 0)
		return (false);
	if (type == SWC_WT_DROPDOWN)
	{
		if (((swcDropDown *)this)->ddcheckforkey(code))
			return (false);
	}
	if (GetKeyState(VK_SHIFT) < 0)
		code |= 0x100;
	if (GetKeyState(VK_CONTROL) < 0)
		code |= 0x200;
///	if (GetKeyState(VK_ALT) < 0)
///		code |= 0x400;

	wpnt = this;						// First scan all parents looking for
	do									//   a global character function
	{
		if (wpnt->gblfunc != NULL)
		{
			if (wpnt->gblfunc(code, wpnt->gblargp))
				return (true);
		}
	} while ((wpnt = wpnt->parent) != NULL);
	wpnt = this;
	do
	{
		hpnt = wpnt->gkhook;
		while (hpnt != NULL)
		{
			if (code == hpnt->code)
			{
				if ((hpnt->func)(wpnt, hpnt->code, hpnt->argi, hpnt->argp))
					return (true);
				break;
			}
			hpnt = hpnt->next;
		}
	} while ((wpnt = wpnt->parent) != NULL);
	return (false);
}


bool swcBase::checkfocus(
	HWND fwin)

{
	HWND twin;

	if (toplvl != NULL)
	{
		if (mousefocus)
		{
			while ((twin = GetParent(fwin)) != NULL)
				fwin = twin;
			if (fwin != toplvl->hwin)
			{
				if (toplvl->ffwindow == NULL)
					toplvl->ffwindow = this;
				::PostMessage(NULL, WM_USER + 50, 0, (long)toplvl);
				return (false);
			}
		}
		if (toplvl->ffwindow != NULL /* && this != toplvl->ffwindow */)
		{
			::PostMessage(NULL, WM_USER + 50, 0, (long)toplvl);
			return (false);
		}
	}
	return (true);
}
