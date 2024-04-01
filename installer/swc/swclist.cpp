#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

swcList::swcList(
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
	long   bits,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3,
	long   arg2)
	: swcBase(xpos, ypos, width, height, parent)

{
	listconstruct(0, fontnum, txtcolorarg, bgcolorarg, contents, select, bits,
			actionfunc, arg3, arg2);
}

swcList::swcList(
	swcDisplayBase
	      *parent,
	long   xpos,
	long   ypos,
	long   width,
	long   height,
	long   iheight,
	long   fontnum,
	long   txtcolorarg,
	long   bgcolorarg,
	char **contents,
	int    select,
	long   bits,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void   *arg3,
	long    arg2)
	: swcBase(xpos, ypos, width, height, parent)

{
	bgcolorsel1 = bgcolorsel2 = 0x7F0000;
	listconstruct(iheight, fontnum, txtcolorarg, bgcolorarg, contents, select,
			bits, actionfunc, arg3, arg2);
}


void swcList::listconstruct(
	long   iheightarg,
	long   fontnumarg,
	long   txtcolorarg,
	long   bgcolorarg,
	char **contents,
	int    select,
	long   bits,
	void (*actionfunc)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3,
	long   arg2)

{
	char *pnt;
	long  xbits;
	int   index;

	iheight = iheightarg & 0xFFFF;
	ihbits = iheightarg;
	anchor = (uchar)((bits >> 20) & 0x0F);
	bits = swcconvertbits(bits, &xbits);
	createwindow("LISTBOX", "", (long)this, bits | WS_CHILD | LBS_NOTIFY |
			LBS_NOINTEGRALHEIGHT |  ((ihbits & SWC_LIST_VARIABLE) ?
			LBS_OWNERDRAWVARIABLE : LBS_OWNERDRAWFIXED), xbits);
	type = SWC_WT_LIST;
	action = actionfunc;
	xlinkfunc = NULL;
	SetWindowLong(hwin, GWL_USERDATA, (LONG)this);
	txtcolor = txtcolorarg;
	bgcolor = bgcolorarg;
	arg2data = arg2;
	arg3data = arg3;
	fontnum = fontnumarg;
///	SendMessage(hwin, WM_SETFONT, (long)swcgetscnfont(fontnum), FALSE);
	orgproc = (WNDPROC)SetWindowLong(hwin, GWL_WNDPROC, (long)listproc);
	if (contents != NULL)
	{
		index = 0;
		while ((pnt = *contents++) != NULL)
			PutItem(index++, 0, pnt, strlen(pnt));
		ListBox_SetCurSel(hwin, select);
	}
	ShowWindow(hwin, SW_SHOWNORMAL);
}


swcList::~swcList()

{
/*
	listitem *ipnt;
	txtblk   *tpnt;
	txtblk   *tnxt;
	int       cnt;

	cnt = ListBox_GetCount(hwin);
	while (--cnt >= 0)
	{
		ipnt = (listitem *)ListBox_GetItemData(hwin, cnt);
		tpnt = ipnt->txthead;
		while (tpnt != NULL)
		{
			tnxt = tpnt->next;
			givememory(tpnt);
			tpnt = tnxt;
		}
	}
*/
}


int swcList::GetSelection()

{
	return (ListBox_GetCurSel(hwin));
}

void swcList::SetSelection(
	int sel)

{
	ListBox_SetCurSel(hwin, sel);
}

int swcList::SetTopItem(
	int sel)

{
	ListBox_SetTopIndex(hwin, sel);
	return (sel == ListBox_GetTopIndex(hwin));
}

int swcList::ClearContent()

{
	return (ListBox_ResetContent(hwin));
}

uchar *swcList::UpdateItem(
	int   index,
	int   extra,
	char *text,
	int   len,
	int   height,
	int   topmargin)

{
	return (UpdateItem(index, extra, 0xFFFFFF, text, len, height, topmargin));
}

uchar *swcList::UpdateItem(
	int   index,
	int   extra,
	long  bgcolor,
	char *text,
	int   len,
	int   height,
	int   topmargin)

{
	uchar *item;
	int    sel;

	sel = ListBox_GetCurSel(hwin);
	if (!RemoveItem(index))
		return (NULL);
	item = PutItem(index, extra, bgcolor, text, len, height, topmargin);
	ListBox_SetCurSel(hwin, sel);
	return (item);
}


uchar *swcList::PutItem(
	int   index,
	int   extra,
	char *text,
	int   len,
	int   height,
	int   topmargin)

{
	return (PutItem(index, extra, 0xFFFFFF, text, len, height, topmargin));
}


uchar *swcList::PutItem(
	int   index,
	int   extra,
	long  bgcolor,
	char *text,
	int   len,
	int   height,
	int   topmargin)

{
	listitem *pnt;

	if (len == 0)
		len = strlen(text);
	pnt = (listitem *)(new char[len + extra + offsetof(listitem, data) + 1]);
	pnt->extra = (ushort)extra;
	pnt->bgcolor = bgcolor;
	pnt->height = (ushort)height;
	pnt->topmargin = (ushort)topmargin;
	pnt->txthead = NULL;
	pnt->txttail = NULL;
	memcpy(pnt->data + extra, text, len + 1);
	ListBox_InsertItemData(hwin, index, (LPARAM)pnt);
	return (pnt->data);
}


int swcList::RemoveItem(
	int index)

{
	listitem *pnt;

	if ((pnt = (listitem *)ListBox_GetItemData(hwin, index)) == (listitem *)-1)
		return (false);
	ListBox_DeleteString(hwin, index);
///	givememory(pnt);
	return (true);
}


uchar *swcList::GetItem(
	int index)

{
	listitem *pnt;

	if ((pnt = (listitem *)ListBox_GetItemData(hwin, index)) ==
			(listitem *)(-1))
		return (NULL);
	else
		return (pnt->data);
}


void swcList::PutText(
	int   index,
	long  xp,
	long  yp,
	long  width,
	long  height,
	long  txtcolor,
	int   font,
	int   mode,
	char *text)

{
///	RECT      rect;
	listitem *ipnt;
    txtblk   *tpnt;
	txtblk   *last;
	txtblk  **prev;
    int       len;
	int       sxpos;
	int       sypos;

	if ((ipnt = (listitem *)ListBox_GetItemData(hwin, index)) ==
			(listitem *)(-1))
		return;
	sxpos = xp;
	sypos = yp;

	// First, delete this text block if it's already defined

	prev = &ipnt->txthead;
	last = NULL;
	while ((tpnt = *prev) != NULL)
	{
		if (tpnt->left == sxpos && tpnt->top == sypos)
		{
			if ((*prev = tpnt->next) == NULL)
				ipnt->txttail = last;
///			rect.left = tpnt->left + xpos;
///			rect.right = tpnt->right + xpos;
///			rect.top = tpnt->top + ypos;
///			rect.bottom = tpnt->bottom + ypos;
///			InvalidateRect(hwin, &rect, TRUE);
			delete [] (char *)tpnt;
			break;
		}
		prev = &(tpnt->next);
		last = tpnt;
	}
	if (text != NULL && text[0] != 0)
	{
		len = strlen(text);
		if ((tpnt = (txtblk *)(new char[sizeof(txtblk) + len])) == NULL)
			return;
		tpnt->length = len;
		tpnt->left = sxpos;
		tpnt->right = sxpos + width;
		tpnt->top = sypos;
		tpnt->bottom = sypos + ((height != 0) ? height : swcgetfontact(fontnum));
		tpnt->fontnum = fontnum;
		tpnt->mode = mode;
		tpnt->txtcolor = txtcolor;
		strcpy(tpnt->text, text);
		tpnt->next = NULL;
		if(ipnt->txttail != NULL)
			ipnt->txttail->next = tpnt;
		else
			ipnt->txthead = tpnt;
		ipnt->txttail = tpnt;
	}
	ipnt->height = 0;
	tpnt = ipnt->txthead;
	while (tpnt != NULL)
	{
		if (ipnt->height < tpnt->bottom)
			ipnt->height = (ushort)tpnt->bottom;
		tpnt = tpnt->next;
	}
	ListBox_SetItemHeight(hwin, index, ipnt->height);
	InvalidateRect(hwin, NULL, true);
}


void swcList::HorizScrollRange(
	int range)

{
	ListBox_SetHorizontalExtent(hwin, (range + sfr) / sfd);
}


void swcList::setxlink(
	void            (*func)(void *arg, int xpos),
	swcDisplayLinked *arg)

{
	xlinkfunc = func;
	xlinkarg = arg;
}


void swcList::clrxlink(
	swcDisplayLinked *arg)

{
	if (xlinkarg == arg)
	{
		xlinkarg = NULL;
		xlinkfunc = NULL;
	}
}


long APIENTRY swcList::listproc(
    HWND hwnd,
    UINT msg,
    UINT wparam,
    LONG lparam)

{
	swcList *pwin;
	RECT     rect;
	HBRUSH   hbrush;

    if ((pwin = (swcList *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
    {
		switch (msg)
		{
		 case WM_ERASEBKGND:
			if ((hbrush = CreateSolidBrush(pwin->bgcolor)) != NULL)
			{
				rect.top = 0;
				rect.bottom = pwin->height;
				rect.left = 0;
				rect.right = pwin->width;
				FillRect((HDC)wparam, &rect, hbrush);
				DeleteObject(hbrush);
			}
			return (1);

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


void swcList::measureitem(
	MEASUREITEMSTRUCT *mis)

{
	if (ihbits & SWC_LIST_VARIABLE)
		mis->itemHeight = (((listitem *)(mis->itemData))->height + sfr) / sfd;
	else if (iheight != 0)
		mis->itemHeight = (iheight + sfr) / sfd;
}



void swcList::yyydeleteitem(
	listitem *data)

{
	txtblk   *tpnt;
	txtblk   *tnxt;

	tpnt = data->txthead;
	while (tpnt != NULL)
	{
		tnxt = tpnt->next;
		delete [] (char *)tpnt;
		tpnt = tnxt;
	}
	delete [] (char *)data;
}


void swcList::drawitem(
	DRAWITEMSTRUCT *dis)

{
	RECT      lrect;
	RECT      trect;
	RECT      brect;
	POINT     point;
	HBRUSH    brush;
	HFONT     savedfont;
	listitem *ipnt;
	txtblk   *tpnt;
	char     *pnt;
	char     *bgn;
	long      tcolor;
	long      bcolor;
	int       hpos;
	int       hsize;
	int       hsizev;
	int       lheight;
	int       tmp;
	int       curfontnum;
	int       newfontnum;
	char      chr;

    static int modetbl[4] = {0, DT_LEFT, DT_CENTER, DT_RIGHT};

	savedfont = 0;						// To keep VS happy!!!
	if (xlinkfunc != NULL)
	{
		GetWindowOrgEx(dis->hDC, &point);
		if (xlinkfunc != NULL && point.x != xpos)
		{
			xpos = point.x;
			xlinkfunc(xlinkarg, xpos);
		}
	}
	lrect = dis->rcItem;
	ipnt = (listitem *)(dis->itemData);
	if (dis->itemState & ODS_SELECTED)
	{
		bcolor = ((ihbits & SWC_LIST_SHOWSELECT) == 0 || GetFocus() == hwin) ?
				bgcolorsel1 : bgcolorsel2;
		tcolor = 0xFFFFFF;
	}
	else
	{
		bcolor = (ipnt != NULL) ? ipnt->bgcolor : bgcolor;
		tcolor = 0x000000;
	}
	brush = CreateSolidBrush(bcolor);
	SetBkColor(dis->hDC, bcolor);
	SetTextColor(dis->hDC, tcolor);
	FillRect(dis->hDC, &lrect, brush);
	DeleteObject(brush);
	if (ihbits & SWC_LIST_LINE)
	{
		brect = lrect;
		brect.top = brect.bottom - 2;
		FillRect(dis->hDC, &brect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	lheight = (iheight + sfr) / sfd;
	if (ipnt != NULL)
	{
		pnt = ((char *)ipnt->data) + ipnt->extra;
		trect = lrect;
		trect.top += (ipnt->topmargin + sfr) / sfd;
		if (ihbits & SWC_LIST_VARIABLE)
			trect.bottom = trect.top + lheight;
		curfontnum = -1;
		newfontnum = fontnum;
		hpos = hsize = 0;
		while (true)
		{
			bgn = pnt;
			while ((chr = *pnt++) != 0 && chr != 0x11 && chr != '\n')
				;
			trect.left = lrect.left + (hpos + sfr) / sfd;
			if ((hsizev = hsize) < 0)
				hsizev = -hsize;
			trect.right = (hsizev != 0) ? (trect.left + hsizev) : lrect.right;
			if ((tmp = pnt - bgn - 1) > 0)
			{
				if (curfontnum != newfontnum)
				{
					if (curfontnum == -1)
						savedfont = (HFONT)SelectObject(dis->hDC,
							swcgetscnfont(newfontnum));
					else
						DeleteObject(SelectObject(dis->hDC,
								swcgetscnfont(newfontnum)));
					curfontnum = newfontnum;
				}
				DrawText(dis->hDC, bgn, tmp, &trect, (hsize <= 0) ?
						(DT_SINGLELINE | DT_VCENTER | DT_LEFT |
						DT_NOPREFIX) : (DT_SINGLELINE | DT_VCENTER |
						DT_RIGHT | DT_NOPREFIX));
				hsize = 0;
			}
			if (chr == 0)
				break;
			if (chr == '\n')
			{
				trect.left = lrect.left;
				trect.top += lheight;
				trect.bottom += lheight;
				hpos = 0;
			}
			else
			{
				// Here if we have a format prefix (0x11)

				switch (*pnt++)
				{
				 case 0x04:				// 1-byte horizontal position
					hpos = (uchar)*pnt++;
					break;

				 case 0x05:				// 2-byte horizontal position
					hpos = (uchar)(pnt[0]) + ((int)((uchar)(pnt[1])) << 8);
					pnt += 2;
					break;

				 case 0x06:				// 1-byte horizontal size
					hsize = (uchar)*pnt++;
					goto sizecom;


				 case 0x07:				// 2-byte horizontal size
					hsize = (short)((uchar)(pnt[0]) +
							((int)((uchar)(pnt[1])) << 8));
					pnt += 2;
				 sizecom:
					hsize = (hsize + sfr) / sfd;
					break;

				 case 0x08:				// Font number (always 1 byte)
					if (((uint)(tmp = *pnt++)) <= FNTMAX)
						newfontnum = tmp;
					break;

				 case 0x09:				// Font size (always 1 byte)

					break;

				 case 0x0A:				// Font color (always 3 bytes,)
					tcolor = ((uchar)*pnt++) << 16;
					tcolor |= (((uchar)*pnt++) << 8);
					tcolor |= ((uchar)*pnt++);
					SetTextColor(dis->hDC, tcolor);
					break;

				 case 0x0B:				// Line height (always 2 bytes)
					trect.top += ((((uchar)*pnt++) + sfr) / sfd);
					lheight = (((uchar)*pnt++) + sfr) / sfd;
					trect.bottom = trect.top + lheight;
					break;

				 case 0x0C:				// Default font color (always 0 bytes)
					SetTextColor(dis->hDC, (dis->itemState & 0x01) ? 0xFFFFFF :
							0x000000);
				}
			}
		}
		if (curfontnum != -1)
			DeleteObject(SelectObject(dis->hDC, savedfont));

		SetBkMode(dis->hDC, TRANSPARENT);
///		SetTextColor(dis->hDC, pwin->txtcolor);
		if ((tpnt = ipnt->txthead) != NULL)
		{
			fontnum = -1;
			do
			{
				SetTextColor(dis->hDC, tpnt->txtcolor);
				if (tpnt->fontnum != fontnum)
				{
					DeleteObject(SelectObject(dis->hDC,
							swcgetscnfont(tpnt->fontnum)));
					fontnum = tpnt->fontnum;
				}
				trect.left = tpnt->left + lrect.left;
				trect.right = tpnt->right + lrect.left;
				trect.top = tpnt->top + lrect.top;
				trect.bottom = tpnt->bottom + lrect.top;

///				brush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
///				FillRect(dis->hDC, &trect, brush);

				DrawText(dis->hDC, tpnt->text, tpnt->length, &trect,
						DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX |
						modetbl[tpnt->mode]);
			} while ((tpnt = tpnt->next) != NULL);
			DeleteObject(SelectObject(dis->hDC, GetStockFont(SYSTEM_FONT)));
		}
	}
	if (dis->itemState & 0x01)
	{
		SetBkColor(dis->hDC, 0xFFFFFF);
		SetTextColor(dis->hDC, 0x000000);
	}
}
