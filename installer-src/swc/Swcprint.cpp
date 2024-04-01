#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

int prtangletbl[] =
{	0,							// TXT_000 = 0x0000
	900,						// TXT_090 = 0x0010
	1800,						// TXT_180 = 0x0020
	2700,						// TXT_270 = 0x0030
};

static uchar abortedflg;

///static long APIENTRY buttonproc(HWND hwnd, UINT msg, UINT wparam, LONG lparam);

static BOOL CALLBACK abortfunc(HDC hdc, int ncode);


swcPrint::swcPrint(
	swcDisplayBase *ownerarg)

{
	owner = ownerarg;
	fontnum = (ulong)-1;
	clprgn = ::CreateRectRgn(0, 0, 0, 0);
}


swcPrint::~swcPrint()

{
	DeleteRgn(clprgn);
	if (basergn != 0)
		DeleteRgn(basergn);
}


int swcPrint::BeginPrint(
	int    landscape,
	char  *name,				// Printer name (NULL = use default, "" = ask
								//   for printer selection)
	int    xresarg,
	int    yresarg,
	void (*notifyarg)(nfunc func, char *text, void *arg),
	void  *notargarg)

{
	PRINTDLG pd;
	DEVMODE *dm;
	int      size;

	notify = notifyarg;
	notarg = notargarg;
	notifydone = false;
	abortedflg = false;
	memset(&pd, 0, sizeof(PRINTDLG));
	if (name == NULL || name[0] == 0)	// Did he specify a printer?
	{									// No
		// Here if want to use the default printer or if want to ask the
		//   user to select a printer with the standard printer selection
		//   dialog. Note that we do not use PrintDlg to obtain the DC for
		//   the printer.  It is done seperately to allow us to modify some
		//   values.

		pd.lStructSize = sizeof(PRINTDLG);
		pd.hwndOwner = owner->hwin;
		pd.Flags = (name == NULL) ? (PD_RETURNDEFAULT | PD_NOWARNING) :
				PD_NOWARNING;
		if (!PrintDlg(&pd))
		{
			errornotify("Error initializing printing");
			return (false);
		}
		dm = (DEVMODE *)GlobalLock(pd.hDevMode);
	}
	else
	{
		// Here if the caller specified a printer directly

		if ((size = DocumentProperties(0, 0, name, NULL, NULL, 0)) < 0)
		{
			errornotify("Printer is not defined");
			return (false);
		}
		if ((dm = (DEVMODE *)new char[size]) == NULL ||
				DocumentProperties(0, 0, name, dm, NULL, DM_OUT_BUFFER) != IDOK)
		{
			errornotify("Error initializing printing");
			return (false);
		}
	}

	// When get here, we have obtained a DEVMODE structure for the desired
	//   printer

	dm->dmFields |= DM_ORIENTATION;
	dm->dmOrientation = (short)((landscape) ? DMORIENT_LANDSCAPE :
			DMORIENT_PORTRAIT);
	if ((hdc = CreateDC(NULL, (char *)(dm->dmDeviceName), NULL, dm)) == NULL)
	{
		errornotify("Error initializing printing");
		return (false);
	}

	// We are now finished with the DEVMODE structure. We give it up
	//   differently depending on how it was created

	if (name == NULL || name[0] == 0)	// Did we use PrintDlg?
	{
		GlobalUnlock(pd.hDevMode);		// Yes
		GlobalFree(pd.hDevMode);
		if (pd.hDevNames != NULL)
			GlobalFree(pd.hDevNames);
	}
	else								// No - we used DocumentProperties
		delete dm;
	xres = (xresarg > 0) ? xresarg : GetDeviceCaps(hdc, LOGPIXELSX);
	if (xres <= 0)
		xres =  300;
	yres = (yresarg > 0) ? yresarg : GetDeviceCaps(hdc, LOGPIXELSY);
	if (yres <= 0)
		yres = 300;

///	static int xxx;

	pagenum = 0;
	SetAbortProc(hdc, abortfunc);

	if (basergn != 0)
		DeleteRgn(basergn);
	basergn = GetClipRgn();
///	xxx = CopyRgn(clprgn, basergn);

///	RECT rect;

///	GetRgnBox(clprgn, &rect);
///	GetRgnBox(basergn, &rect);

	if (!notifydone)
		notify(begin, "Initialling printing...", notarg);

	memset(&docinfo, 0, sizeof(docinfo));
	docinfo.cbSize = sizeof(docinfo);
	docinfo.lpszDocName = "MMS";
	if (StartDoc(hdc, &docinfo) <= 0)
	{
		errornotify("Error initializing printing");
		return (false);
	}
	return (true);
}


int swcPrint::BeginPage()

{
	char text[64];

	if (abortedflg)
		return (false);
	if (!notifydone)
	{
		sprintf(text, "Printing page %d ...", ++pagenum);
		notify(status, text, notarg);
	}
	if (StartPage(hdc) <= 0)
	{
		errornotify("Error starting next page");
		return (false);
	}	
	SetBkMode(hdc, TRANSPARENT);
	return (true);
}

swcRgn *swcPrint::GetClipRgn()

{
	RECT    rect;

	if (GetClipBox(hdc, &rect) < 0)
		return (NULL);
	return (::CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom));
}


swcRgn *swcPrint::CreateRectRgn(
	long hpos,
	long vpos,
	long width,
	long height)

{
	long a1;
	long a2;
	long a3;
	long a4;

	a1 = (hpos * xres + 300)/600;
	a2 = (vpos * yres + 300)/600;
	a3 = ((hpos + width) * xres + 300)/600 - 1;
	a4 = ((vpos + height) * yres + 300)/600 - 1;
	return (::CreateRectRgn(a1, a2, a3, a4));
}


int swcPrint::SetClipRgn(
	swcRgn *rgn)

{
	CopyRgn(clprgn, rgn);
	return (SelectClipRgn(hdc, (rgn != NULL) ? rgn : basergn));
}


void swcPrint::PutText(
    long  xpos,
    long  ypos,
	long  width,
	long  height,
    long  txtcolor,
    ulong fontnumarg,
    int   mode,
    char *text)

{
///	SIZE    size;
///	swcRgn *txtrgn;
///	long    xrgn;
///	long    yrgn;
///	long    temp;
///	int     len;
	RECT rect;
	HFONT   hfont;

	if (xres == 0)
		return;
	if (fontnum != fontnumarg || weight != (mode >> 4))
	{
		hfont = getprtfont(fontnumarg, (mode >> 8) & 0x0F, (mode >> 4) & 0x0F,
				(mode & TXT_ITALIC) != 0);
		DeleteObject(SelectObject(hdc, hfont));
		fontnum = fontnumarg;
		weight = mode >> 4;
	}
	SetTextColor(hdc, txtcolor);
/*
	mode &= 0x00FF;
	len = strlen(text);
	GetTextExtentPoint32(hdc, text, len, &size);
	size.cx = (size.cx * 600 + xres/2)/xres;
	size.cy = (size.cy * 600 + yres/2)/yres;
	xrgn = xpos;
	yrgn = ypos;
	switch (mode & 0xF0)
	{
	 case TXT_000:
		if (height == 0)
			height = size.cy;
		if (width > size.cx && (mode & 0x0F) != TXT_LEFT)
			xpos += ((mode == TXT_RIGHT) ? (width - size.cx) :
					(width - size.cx)/2);
		if (height > size.cy)
			ypos += ((height - size.cy)/2);
		break;

	 case TXT_090:
		if (height == 0)
			height = size.cx;
		yrgn -= width;
		if (width > size.cy && (mode & 0x0F) != TXT_LEFT)
			ypos -= ((mode == TXT_RIGHT) ? (width - size.cy) :
					(width - size.cy)/2);
		if (height > size.cx)
			xpos += ((height - size.cx)/2);
		goto swapdim;

	 case TXT_180:
		if (height == 0)
			height = size.cy;
		xrgn -= width;
		yrgn -= height;
		if (width > size.cx && (mode & 0x0F) != TXT_LEFT)
			xpos -= ((mode == TXT_RIGHT) ? (width - size.cx) :
					(width - size.cx)/2);
		if (height > size.cy)
			ypos -= ((height - size.cy)/2);
		break;

	 case TXT_270:
		if (height == 0)
			height = size.cx;
		xrgn -= height;
		if (width > size.cy && (mode & 0x0F) != TXT_LEFT)
			ypos += ((mode == TXT_RIGHT) ? (width - size.cy) :
					(width - size.cy)/2);
		if (height > size.cx)
			xpos -= ((height - size.cx)/2);
	 swapdim:
		temp = width;
		width = height;
		height = temp;
		break;
	}
*/

///	txtrgn = CreateRectRgn(xrgn, yrgn, width, height);
///	ExtSelectClipRgn(hdc, txtrgn, RGN_AND);
///	swcDeleteRgn(txtrgn);

	SIZE tsize;
	int  len;
	char *dpnt;

	if ((fontnum & 0xF0000000) != FONT_CONTROL)
	{
		rect.left = (xpos * xres + 300)/600;
		rect.top = (ypos * yres + 300)/600;
		rect.right = ((xpos + width) * xres + 300)/600;
		rect.bottom = ((ypos + height) * yres + 300)/600;
		len = strlen(text);
		mode &= 0x0F;
		if (mode == TXT_DECIMAL)
		{
			mode = TXT_RIGHT;

			// Find the decimal point

			if ((dpnt = strchr(text, '.')) != NULL)
			{
				GetTextExtentPoint32(hdc, dpnt, len - (dpnt - text), &tsize);
				rect.right += tsize.cx;
			}
		}

///		txtrgn = CreateRectRgnIndirect(&rect);
///		ExtSelectClipRgn(hdc, txtrgn, RGN_AND);
///		swcDeleteRgn(txtrgn);
		DrawText(hdc, text, len, &rect, DT_WORDBREAK | DT_NOPREFIX |
				DT_TOP | ((mode == TXT_RIGHT) ? DT_RIGHT :
				(mode == TXT_CENTER) ? DT_CENTER : DT_LEFT));
///		SelectClipRgn(hdc, basergn);

///		::SelectObject(hdc, ::CreatePen(PS_SOLID, 5, RGB(0, 0, 0)));
///		Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	}
	else
		TextOut(hdc, (xpos * xres + 300)/600, (ypos * yres + 300)/600, text,
				strlen(text));
}


void swcPrint::PutImage(
	long      xpos,
	long      ypos,
	long      width,
	long      height,
	swcImage *img)

{
	struct ihead
	{	BITMAPINFOHEADER bmih;
		long             colors[2];
	};
	BITMAPINFO *xbmi;
	ihead ih;
	char *xdata;
	char *spnt;
	char *dpnt;
	uchar *bpnt;
	uchar value;
	int   cnt1;
	int   cnt2;
	int   cnt3;
	int   wid;

	if (width <= 0)
	{
		width = img->bmih.biWidth;
		height = img->bmih.biHeight;
	}

	// Several windows printer drivers have bugs that cause the incorrect
	//   display of 24/32 bit bit-maps.  To work around this, we convert
	//   to a monochrome bit-map!

	wid = (img->bmih.biWidth + 31)/32;
	xdata = new char[wid * img->bmih.biHeight * 4 + 4];
	spnt = (char *)(img->data) + 1;
	dpnt = xdata;
	cnt1 = img->bmih.biHeight;
	while (--cnt1 >= 0)
	{
		bpnt = (uchar *)dpnt;
		cnt2 = img->bmih.biWidth;
		while (cnt2 > 0)
		{
			cnt3 = 8;
			value = 0xFF;
			do
			{
				value <<= 1;
				if (*spnt)
					value |= 0x01;
				spnt += 4;
				if (--cnt2 <= 0)
				{	cnt3 = 0;
					break;
				}
			} while (--cnt3 > 0);
			*bpnt++ = value;
		}
		dpnt += (wid * 4);
	}
	ih.bmih = *&img->bmih;
	ih.colors[0] = RGB(0, 0, 0);
	ih.colors[1] = RGB(0xFF, 0xFF, 0xFF);
	ih.bmih.biBitCount = 1;
	xbmi = (BITMAPINFO *)&ih;
	StretchDIBits(hdc, (xpos * xres + 300)/600, (ypos * yres + 300)/600,
			(width * xres + 300)/600, (height * yres)/600, 0, 0,
			img->bmih.biWidth, img->bmih.biHeight, xdata, xbmi, DIB_RGB_COLORS,
			SRCCOPY);
}


void swcPrint::DrawRect(
	long xpos,
	long ypos,
	long width,
	long height,
	long fcolor,
	long brdrwid,
	long bcolor)

{
	DrawRect(xpos, ypos, width, height, bcolor);
	DrawRect(xpos + brdrwid, ypos + brdrwid, width - 2 * brdrwid,
			height - 2 * brdrwid, fcolor);
}


void swcPrint::DrawRect(
	long xpos,
	long ypos,
	long width,
	long height,
	long color)

{
	HBRUSH brush;
	RECT   rect;

	rect.left = (xpos * xres + 300)/600;
	rect.right = ((xpos + width) * xres + 300)/600 - 1;
	rect.top = (ypos * yres + 300)/600;
	rect.bottom = ((ypos + height) * yres + 300)/600 - 1;
	if ((brush = CreateSolidBrush(color)) != NULL)
	{
		FillRect(hdc, &rect, brush);
		DeleteObject(brush);
	}
}


void swcPrint::DrawLine(
	long xbgn,
	long ybgn,
	long xend,
	long yend,
	long width,
	long color)

{
	HPEN     pen;
	HPEN     oldpen;
	LOGBRUSH logbrush;
	
	logbrush.lbStyle = BS_SOLID;
	logbrush.lbColor = color;
	logbrush.lbHatch = 0;

	if ((pen = ExtCreatePen(PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_SQUARE|
		PS_JOIN_BEVEL, width, &logbrush, 0, NULL)) != NULL)
	{
		oldpen = SelectPen(hdc, pen);
		MoveToEx(hdc, xbgn, ybgn, NULL);
		LineTo(hdc, xend, yend);
		SelectPen(hdc, oldpen);
		DeletePen(pen);
	}
}
	

int swcPrint::EndPage()

{
	swcDeleteRgn(basergn);
	fontnum = (ulong)-1;
	if (abortedflg)
		return (false);
	if (::EndPage(hdc) <= 0)
	{
		errornotify("Error printing page");
		return (false);
	}
	return (!abortedflg);
}

int swcPrint::EndPrint()

{
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	if (abortedflg)
		AbortDoc(hdc);
	else if (EndDoc(hdc) <= 0)
		errornotify("Error printing document");
	else if (!notifydone)
	{
		DeleteDC(hdc);
		hdc = 0;
		notifydone = true;
		notify(finish, "Printing is complete", notarg);
		return (true);
	}
	DeleteDC(hdc);
	hdc = 0;
	return (true);
}


void swcPrint::CancelPrint()

{
	if (!notifydone)
		notify(status, "Canceling printing ...", notarg);
	abortedflg = true;
}


void swcPrint::errornotify(
	char *text)

{
	if (!notifydone)
	{
		notifydone = true;
		notify(error, text, notarg);
	}
}


static BOOL CALLBACK abortfunc(
	HDC hdc,
	int ncode)

{
	MSG msg;

	NOTUSED(hdc);
	NOTUSED(ncode);

    while (!abortedflg && PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		swchandlemessage(&msg);
	return (!abortedflg);
}


HFONT swcPrint::getprtfont(
	ulong fontnum,
	int   weight,
	int   angle,
	int   italic)

{
    LOGFONT logfnt;

    memset(&logfnt, 0, sizeof(logfnt));
	logfnt.lfHeight = (((ushort)fontnum) * xres + 300)/600;
	logfnt.lfWidth = (((fontnum >> 16) & 0x0FFF) * xres + 300)/600;
	logfnt.lfEscapement = prtangletbl[angle];
///	logfnt.lfWeight = prtweighttbl[weight];
	logfnt.lfItalic = (uchar)italic;
///	strcpy(logfnt.lfFaceName, prtfonttbl[(fontnum >> 28) & 0x0F]);
	return (CreateFontIndirect(&logfnt));
}
