#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

long     editcolor = -1;
HBRUSH   heditbrush;


static long drawtextblk(char *pnt, RECT *lrect, long tcolor, long newfontnum,
        long curfontnum, HDC hdc);


swcDisplayBase::swcDisplayBase()

{
}


swcDisplayBase::swcDisplayBase(
	char  *name,
	char  *label,
    swcDisplayBase
	      *parent,
    long   xpos,
    long   ypos,
    long   width,
    long   height,
    long   txtcolorarg,
    long   bgcolorarg,
	long   bits,
	long   wbits,
	void (*actionarg)(swcBase *win, long arg1, long arg2, void *arg3),
	void  *arg3arg,
	int    arg2arg)
	: swcBase(xpos, ypos, width, height, parent)

{
	RECT rect;
	long xbits;

	hmf = NULL;
	PaintFunc = NULL;
	mmoveaction = ((bits & SWC_MMOVE_ACTION) != 0);
	anchor = (uchar)((bits >> 20) & 0x0F);
	bits = swcconvertbits(bits | SWC_STATE_ENABLE, &xbits) | wbits;
	createwindow(name, label, 0, bits, xbits);
	type = SWC_WT_DISPLAY;
	txthead = txttail = NULL;
	imghead = imgtail = NULL;
	action = actionarg;
	arg2data = arg2arg;
	arg3data = arg3arg;
	SetWindowLong(hwin, GWL_USERDATA, (LONG)this);
	txtcolor = txtcolorarg;
	bgcolor = bgcolorarg;
	xoffset = yoffset = 0;
	fchild = NULL;
	::GetClientRect(hwin, &rect);
	cwidth = rect.right * sfd;
	cheight = rect.bottom * sfd;
	ShowWindow(hwin, SW_SHOWNORMAL);
}

swcDisplayBase::~swcDisplayBase()

{
	Clear();
}


void swcDisplayBase::Clear()

{
	txtblk *tpnt;
	txtblk *tnxt;
	imgblk *ipnt;
	imgblk *inxt;

	while (fchild != NULL)				// Destroy all child windows
		delete fchild;
	if ((tpnt = txthead) != NULL)		// Give up all text blocks
	{
		txthead = txttail = NULL;
		do
		{
			tnxt = tpnt->next;
			delete [] (char *)tpnt;
		} while ((tpnt = tnxt) != NULL);
	}
	if ((ipnt = imghead) != NULL)		// Give up all image blocks
	{
		imghead = imgtail = NULL;
		do
		{
			inxt = ipnt->next;
			if (ipnt->img != NULL)
			{
///				ipnt->img->Release();	// qqqqqqqqqqqqqqqqqqqqqqq
				ipnt->img = NULL;
			}
			delete ipnt;
		} while ((ipnt = inxt) != NULL);
	}
	ForceRedraw();
}


void swcDisplayBase::BringToTop()

{
	BringWindowToTop(hwin);
}



void swcDisplayBase::DrawLine(
	long type,
	long xpos,
	long ypos,
	long length,
	long width)
{
	CreateWindow( "STATIC", "", SS_SUNKEN|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|
			WS_CLIPSIBLINGS, (xpos + sfr) / sfd, (ypos + sfr) /sfd,
			(length + sfr) / sfd, width, hwin, NULL, hinst, NULL);
}


void swcDisplayBase::PutText(
    long    xpos,
    long    ypos,
    long    width,
    long    height,
    long    txtcolor,
    int     attrb,
    char   *text)

{
	PutText(xpos, ypos, width, height, txtcolor, attrb, attrb, text);
}


void swcDisplayBase::PutText(
    long    xpos,
    long    ypos,
    long    width,
    long    height,
    long    txtcolor,
    int     fontnum,
    int     mode,
    char   *text)

{
	RECT     rect;
    txtblk  *pnt;
	txtblk  *last;
	txtblk **prev;
    int      len;
	int      sxpos;
	int      sypos;

	sxpos = xpos;
	sypos = ypos;

	// First, delete this text block if it's already defined

	prev = &txthead;
	last = NULL;
	while ((pnt = *prev) != NULL)
	{
		if (pnt->left == sxpos && pnt->top == sypos)
		{
			if ((*prev = pnt->next) == NULL)
				txttail = last;
			rect.left = (pnt->left + sfr) / sfd;
			rect.right = (pnt->right + sfr) / sfd;
			rect.top = (pnt->top + sfr) / sfd;
			rect.bottom = (pnt->bottom + sfr) / sfd;
			if (this == &swc_BaseWin)
			{
				rect.top += swcbaseyoffset;
				rect.bottom += swcbaseyoffset;
				rect.left += swcbasexoffset;
				rect.right += swcbasexoffset;
			}
			InvalidateRect(hwin, &rect, TRUE);
			delete [] (char *)pnt;
			break;
		}
		prev = &(pnt->next);
		last = pnt;
	}
	if (text != NULL && text[0] != 0)
	{
		len = strlen(text);

///		if ((pnt = (txtblk *)malloc(sizeof(txtblk) + len)) == NULL)

		if ((pnt = (txtblk *)(new char[sizeof(txtblk) + len])) == NULL)
			return;
		pnt->length = len;
		pnt->left = rect.left = sxpos;
		pnt->right = rect.right = (xpos + width);
		pnt->top = rect.top = sypos;
		pnt->bottom = rect.bottom = sypos + ((height != 0) ? height :
				(swcgetfontact(fontnum) * sfd));
		pnt->fontnum = fontnum;
////qqqqqqqqqqqqqqq
		pnt->mode = (mode >> 16) & 0x0F;
		pnt->txtcolor = txtcolor;
		strcpy(pnt->text, text);
		pnt->next = NULL;
		if(txttail != NULL)
			txttail->next = pnt;
		else
			txthead = pnt;
		txttail = pnt;
		rect.left = (pnt->left + sfr) / sfd;
		rect.right = (pnt->right + sfr) / sfd;
		rect.top = (pnt->top + sfr) / sfd;
		rect.bottom = (pnt->bottom + sfr) / sfd;
		if (this == &swc_BaseWin)
		{
			rect.top += swcbaseyoffset;
			rect.bottom += swcbaseyoffset;
			rect.left += swcbasexoffset;
			rect.right += swcbasexoffset;
		}
		InvalidateRect(hwin, &rect, true);
	}
}


int swcDisplayBase::GetTextExtent(
	int   fontnum,
	char *text,
	int   len,
	long *size)

{
	SIZE sz;
	HDC  hdc;
	int  rtn;

	if ((hdc = GetDC(hwin)) == NULL)
		return (false);
	DeleteObject(SelectObject(hdc, swcgetscnfont(fontnum)));
	rtn = GetTextExtentPoint32(hdc, text, len, &sz);
	DeleteObject(SelectObject(hdc, GetStockFont(SYSTEM_FONT)));
	ReleaseDC(hwin, hdc);
	size[0] = sz.cx * sfd;
	size[1] = sz.cy * sfd;
	return (rtn);
}


void swcDisplayBase::HorizScrollRange(
	long rangearg,
	long incrementarg)

{
	SCROLLINFO sinfo;

	xincrement = incrementarg;
	sinfo.cbSize = sizeof(sinfo);
	sinfo.fMask = SIF_ALL;
	sinfo.nMin = 0;
	sinfo.nMax = xrange = (rangearg + sfr) / sfd;
	sinfo.nPage = cwidth;
	sinfo.nPos =	0;
	SetScrollInfo(hwin, SB_HORZ, &sinfo, TRUE);
}

void swcDisplayBase::HorizScrollPosition(
	long pos)

{
	SCROLLINFO  sinfo;

	if (xrange > cwidth)
	{
		xoffset = pos;
		if (xoffset < 0)
			xoffset = 0;
		else if (xoffset > (xrange - cwidth))
			xoffset = xrange - cwidth;
		sinfo.cbSize = sizeof(sinfo);
		sinfo.fMask = SIF_POS;
		sinfo.nPos = xoffset;
		SetScrollInfo(hwin, SB_HORZ, &sinfo, TRUE);
		adjustchilds();
		InvalidateRect(hwin, NULL, TRUE);
	}
}


void swcDisplayBase::VertScrollRange(
	long rangearg,
	long incrementarg)

{
	SCROLLINFO sinfo;

	yincrement = (incrementarg + sfr) / sfd;
	sinfo.cbSize = sizeof(sinfo);
	sinfo.fMask = SIF_ALL;
	sinfo.nMin = 0;
	sinfo.nMax = ((yrange = rangearg) < cheight) ? cheight : yrange;
	sinfo.nPage = cheight;;
	sinfo.nPos =	yoffset * sfd;
	SetScrollInfo(hwin, SB_VERT, &sinfo, TRUE);
}

void swcDisplayBase::VertScrollPosition(
	long pos)

{
	SCROLLINFO  sinfo;

	if (yrange > cheight)
	{
		yoffset = (pos + sfr) / sfd;
		if (yoffset < 0)
			yoffset = 0;
		else if ((yoffset * sfd) > (yrange - cheight))
			yoffset = (yrange - cheight + sfr) / sfd;
		sinfo.cbSize = sizeof(sinfo);
		sinfo.fMask = SIF_POS;
		sinfo.nPos = yoffset * sfd;
		SetScrollInfo(hwin, SB_VERT, &sinfo, TRUE);
		adjustchilds();
		InvalidateRect(hwin, NULL, TRUE);
	}
}


void swcDisplayBase::PutImage(
    long      xpos,
    long      ypos,
    swcImage *img)

{
	PutImage(xpos, ypos, 0, 0, img);
}


void swcDisplayBase::PutImage(
    long      xpos,
    long      ypos,
	long      width,
	long      height,
    swcImage *img)

{
	imgblk  *pnt;
	imgblk **prev;
	imgblk  *last;
	RECT     irect;
	long     sxpos;
	long     sypos;

	sxpos = (xpos + sfr) / sfd;
	sypos = (ypos + sfr) / sfd;
	if (img == NULL)
	{
		prev = &imghead;
		last = NULL;

		while ((pnt = *prev) != NULL)
		{
			if (pnt->xpos == sxpos && pnt->ypos == sypos)
			{
				if ((*prev = pnt->next) == NULL)
					imgtail = last;
///				pnt->img->Release();		/// qqqqqqqqqqqqqqqqqqqqq
				irect.left = pnt->xpos;
				irect.right = pnt->xpos + pnt->width;
				irect.top = pnt->ypos;
				irect.bottom = pnt->ypos + pnt->height;
				if (this == &swc_BaseWin)
				{
					irect.top += swcbaseyoffset;
					irect.bottom += swcbaseyoffset;
					irect.left += swcbasexoffset;
					irect.right += swcbasexoffset;
				}
				InvalidateRect(hwin, &irect, TRUE);
				delete [] (char *)pnt;
				break;
			}
			prev = &(pnt->next);
			last = pnt;
		}
		return;
	}
///	img->Use();				/// qqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
	pnt = new imgblk();
    pnt->xpos = irect.left = sxpos;
    pnt->ypos = irect.top = sypos;
	pnt->width = (width + sfr) / sfd;
	pnt->height = (height + sfr) / sfd;
	pnt->img = img;
	if (this == &swc_BaseWin)
	{
		irect.left += swcbasexoffset;
		irect.top += swcbaseyoffset;
	}
	irect.right = irect.left + pnt->width;
	irect.bottom = irect.top + pnt->height;
    pnt->next = NULL;
    if(imgtail != NULL)
		imgtail->next = pnt;
    else
		imghead = pnt;
    imgtail = pnt;
	InvalidateRect(hwin, &irect, TRUE);
}

/*

imgblk *swcDisplayBase::putbmpimage(
	long   size,
	uchar *data)

{
	imgblk *pnt;
	long    dsize;

	if ((dsize = size - ((BITMAPFILEHEADER *)data)->bfOffBits) <= 10 ||
			dsize > size || dsize > 4000000)
		return (NULL);
	pnt = new imgblk();
	memcpy(&pnt->bmih, data + sizeof(BITMAPFILEHEADER),
			sizeof(BITMAPINFOHEADER));
	pnt->data = new uchar[dsize];
	memcpy(pnt->data, data + ((BITMAPFILEHEADER *)data)->bfOffBits, dsize);

	return (pnt);
}


extern char *direndpnt;
extern char dirspec[];

imgblk *swcDisplayBase::putjpegimage(
	long   size,
	uchar *data)

{
	imgblk *pnt;
	uchar  *dpnt;
	uchar **spnt;
    JSAMPARRAY scanlines;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	long   width;
	int    cnt;

	cinfo.err = jpeg_std_error(&jerr);	// Set up for jpeg decompression
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, data, size);
	jpeg_read_header(&cinfo, TRUE);
	pnt = new imgblk();
	width = ((cinfo.image_width * 3) + 3) & 0xFFFFFFFC;
	pnt->data = dpnt = new uchar[width * cinfo.image_height];
	scanlines = (JSAMPARRAY)new long[cinfo.image_height];
	spnt = (uchar **)scanlines + (cinfo.image_height - 1);
	cnt = cinfo.image_height;
	while (--cnt >= 0)
	{
		*spnt-- = (uchar *)dpnt;
		dpnt += width;
	}
	jpeg_start_decompress(&cinfo);		// Decompress the image
	while (cinfo.output_scanline < cinfo.output_height)
		jpeg_read_scanlines(&cinfo, scanlines + cinfo.output_scanline, 10);
	jpeg_finish_decompress(&cinfo);
	pnt->bmih.biSize = sizeof(BITMAPINFOHEADER); // Fill in the bmp header
	pnt->bmih.biWidth = cinfo.image_width;		//    for windows
	pnt->bmih.biHeight = cinfo.image_height;
	pnt->bmih.biPlanes = 1;
	pnt->bmih.biBitCount = 24;
	pnt->bmih.biCompression = BI_RGB;
	pnt->bmih.biSizeImage = 0;
	pnt->bmih.biXPelsPerMeter = 0;
	pnt->bmih.biYPelsPerMeter = 0;
	pnt->bmih.biClrUsed = 0 ;
	pnt->bmih.biClrImportant = 0;
	jpeg_destroy_decompress(&cinfo);	// Give up the resources used for
										//  decompression
	return (pnt);
}
*/


/*
void swcDisplayBase::PutImage(
    long  xpos,
    long  ypos,
    char *name)

{
	PutImage(xpos, ypos, 0, 0, name);
}


void swcDisplayBase::PutImage(
    long  xpos,
    long  ypos,
	long  width,
	long  height,
    char *name)

{
    BITMAPFILEHEADER
	         header;
    OFSTRUCT ofstr;
    imgblk  *pnt;
	imgblk  *last;
	imgblk **prev;
	RECT     irect;
    long     file;
	ulong    amount;
	int      sxpos;
	int      sypos;

	// First, delete this image if it's already defined

	sxpos = xpos * sf;
	sypos = ypos * sf;
	prev = &imghead;
	last = NULL;
	while ((pnt = *prev) != NULL)
	{
		if (pnt->xpos == sxpos && pnt->ypos == sypos)
		{
			if ((*prev = pnt->next) == NULL)
				imgtail = last;
			irect.left = pnt->xpos;
			irect.right = pnt->xpos + pnt->width;
			irect.top = pnt->ypos;
			irect.bottom = pnt->ypos + pnt->height;
			if (this == &swc_BaseWin)
			{
				irect.top += swcbaseyoffset;
				irect.bottom += swcbaseyoffset;
				irect.left += swcbasexoffset;
				irect.right += swcbasexoffset;
			}
			InvalidateRect(hwin, &irect, TRUE);
			givememory(pnt);
			break;
		}
		prev = &(pnt->next);
		last = pnt;
	}
    if ((file = OpenFile(name, &ofstr, OF_READ)) < 0)
		return;
    if (ReadFile((HANDLE)file, (char *)&header, sizeof(header), &amount,
			NULL) == 0)
	{
		CloseHandle((HANDLE)file);
		return;
	}

	// CHECK FOR BMP FILE HERE!!!!!

	pnt = new imgblk();
    if (ReadFile((HANDLE)file, (char *)&pnt->bmih, sizeof(pnt->bmih), &amount,
			NULL) == 0)
	{
		delete pnt;
		CloseHandle((HANDLE)file);
		return;
	}

	if (SetFilePointer((HANDLE)file, header.bfOffBits, NULL, FILE_BEGIN) < 0)
	{
		delete pnt;
		CloseHandle((HANDLE)file);
		return;
	}
	pnt->data = new uchar[header.bfSize - header.bfOffBits];
	if (ReadFile((HANDLE)file, (char *)(pnt->data), header.bfSize -
			header.bfOffBits, &amount, NULL) == 0)
    {
		delete [] pnt->data;
		delete pnt;
		CloseHandle((HANDLE)file);
		return;
    }
	CloseHandle((HANDLE)file);
    pnt->type = IMG_BMP;
    pnt->xpos = irect.left = xpos * sf;
    pnt->ypos = irect.top = ypos * sf;
	pnt->width = width * sf;
	pnt->height = height * sf;
	if (this == &swc_BaseWin)
	{
		irect.left += swcbasexoffset;
		irect.top += swcbaseyoffset;
	}
	irect.right = irect.left + pnt->width;
	irect.bottom = irect.top + pnt->height;
    pnt->next = NULL;
    if(imgtail != NULL)
		imgtail->next = pnt;
    else
		imghead = pnt;
    imgtail = pnt;
	InvalidateRect(hwin, &irect, TRUE);
}

*/


void swcDisplayBase::sizefixup()
{
	SCROLLINFO sinfo;

	if (yoffset > 0)
	{
		if ((yoffset * sfd) > (yrange - cheight))
		{
			if ((yoffset = (yrange - cheight + sfr) / sfd) < 0)
				yoffset = 0;
///			ForceRedraw();
		}
	}

	sinfo.cbSize = sizeof(sinfo);
	sinfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
    sinfo.nMin = 0xF0F0F0F;
	GetScrollInfo(hwin, SB_VERT, &sinfo);
    if (sinfo.nMin != 0xF0F0F0F)
    {
	    sinfo.cbSize = sizeof(sinfo);
	    sinfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	    sinfo.nPos = ((swcDisplayBase *)this)->yoffset * sfd;
	    sinfo.nMin = 0;
	    sinfo.nMax = (((swcDisplayBase *)this)->yrange <
			    ((swcDisplayBase *)this)->cheight) ?
			    ((swcDisplayBase *)this)->cheight :
			    ((swcDisplayBase *)this)->yrange;
	    sinfo.nPage = ((swcDisplayBase *)this)->cheight;
    	SetScrollInfo(hwin, SB_VERT, &sinfo, true);
    }
}


long APIENTRY swcDisplayBase::displayproc(
    HWND hwnd,
    UINT message,
    UINT wparam,
    LONG lparam)

{
    swcDisplayBase *pwin;
    swcBase     *cwin;
    txtblk      *tpnt;
    imgblk      *ipnt;
	swcImage    *img;
	NMHDR       *pnmhdr;
    HDC          hdc;
    RECT         rect;
	POINT        point;
    PAINTSTRUCT  ps;
    HBRUSH       hbrush;
	HPEN         hpen;
	SCROLLINFO   sinfo;
	swcMouseMove mmove;
	long         temp;
	long         top;
	long         bottom;
	long         pxpos;
	long         pypos;
	long         pwidth;
	long         pheight;
	int          fontnum;
	int          abits;

    static int modetbl[4] = {0, DT_LEFT, DT_CENTER, DT_RIGHT};

    switch (message)
    {
	 case WM_DEVICECHANGE:
		if (swc_DeviceNotify != NULL)
			swc_DeviceNotify(message, wparam, lparam);
		break;

	 case WM_ERASEBKGND:
		if (!IsIconic(hwnd) && (pwin = (swcDisplayBase *)GetWindowLong(hwnd,
				GWL_USERDATA)) != NULL)
		{
		    if ((hbrush = CreateSolidBrush(pwin->bgcolor)) != 0)
			{
				GetClipBox((HDC)wparam, &rect);
				FillRect((HDC)wparam, &rect, hbrush);
				DeleteObject(hbrush);
			}
			return (TRUE);
		}
		break;

     case WM_PAINT:
		if (!IsIconic(hwnd) && (pwin = (swcDisplayBase *)GetWindowLong(hwnd,
				GWL_USERDATA)) != NULL)
		{
		    hdc = BeginPaint(hwnd, &ps);

			if (pwin->PaintFunc != NULL)
				pwin->PaintFunc(pwin, hdc);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, pwin->txtcolor);
			if ((tpnt = pwin->txthead) != NULL)
			{
				fontnum = -1;
				do
				{
					top = (tpnt->top + sfr) / sfd;
					bottom = (tpnt->bottom + sfr) / sfd;
					if (bottom > pwin->yoffset &&
							top < (pwin->yoffset + pwin->cheight))
					{
						SetTextColor(hdc, tpnt->txtcolor);
						if (tpnt->fontnum != fontnum)
						{
							DeleteObject(SelectObject(hdc,
									swcgetscnfont(tpnt->fontnum)));
							fontnum = tpnt->fontnum;
						}
						rect.left = (tpnt->left + sfr) / sfd -
								pwin->xoffset;
						rect.right = (tpnt->right + sfr) /sfd -
								pwin->xoffset;
						rect.top = top - pwin->yoffset;
						rect.bottom = bottom - pwin->yoffset;

						if (modetbl[tpnt->mode] & (DT_CENTER|DT_RIGHT))
                            DrawText(hdc, tpnt->text, tpnt->length, &rect,
                                    DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX |
                                    modetbl[tpnt->mode]);
						else
							drawtextblk(tpnt->text, &rect, tpnt->txtcolor,
                                    tpnt->fontnum, -1, hdc);
					}
				} while ((tpnt = tpnt->next) != NULL);
				DeleteObject(SelectObject(hdc, GetStockFont(SYSTEM_FONT)));
			}
			ipnt = pwin->imghead;
			while (ipnt != NULL)
			{
				img = ipnt->img;
				if (ipnt->width <= 0)
					SetDIBitsToDevice(hdc, ipnt->xpos - pwin->xoffset,
							ipnt->ypos - pwin->yoffset, img->bmih.biWidth,
							img->bmih.biHeight, 0, 0, 0, img->bmih.biHeight,
							img->data, (BITMAPINFO *)&img->bmih,
							DIB_RGB_COLORS);
				else
					StretchDIBits(hdc, ipnt->xpos - pwin->xoffset,
							ipnt->ypos - pwin->yoffset, ipnt->width,
							ipnt->height, 0, 0, img->bmih.biWidth,
							img->bmih.biHeight, img->data,
							(BITMAPINFO *)&img->bmih, DIB_RGB_COLORS,
							SRCCOPY);
				ipnt = ipnt->next;
			}
			if (pwin->hmf != NULL)
			{
				SelectBrush(hdc, GetStockBrush(BLACK_BRUSH));
				Rectangle(hdc, 0, 0, 310, 310);

				rect.top = 0;
				rect.bottom = 600;
				rect.left = 0;
				rect.right = 600;
				PlayEnhMetaFile(hdc, pwin->hmf, &rect);
				hpen = CreatePen(PS_SOLID, 30, RGB(0,0,0xFF));
				SelectObject(hdc, hpen);
				MoveToEx(hdc, 30, 100, NULL);
				LineTo(hdc, 500, 100);
				SelectObject(hdc, GetStockObject(BLACK_PEN));
				DeleteObject(hpen);
			}
			EndPaint(hwnd, &ps);
			return (0);
		}
		break;

     case WM_CTLCOLORMSGBOX:
     case WM_CTLCOLOREDIT:
     case WM_CTLCOLORLISTBOX:
///	 case WM_CTLCOLORBTN:
     case WM_CTLCOLORDLG:
     case WM_CTLCOLORSCROLLBAR:
     case WM_CTLCOLORSTATIC:
		if ((cwin = (swcBase *)GetWindowLong((HWND)lparam,
				GWL_USERDATA)) != NULL &&
				(hbrush = (cwin->ctlbkgcolor((HDC)wparam))) != NULL)
			return ((long)hbrush);
		break;

	 case WM_MEASUREITEM:
		if ((cwin = (swcBase *)((MEASUREITEMSTRUCT *)lparam)->CtlID) != NULL)
			cwin->measureitem((MEASUREITEMSTRUCT *)lparam);
		break;

	 case WM_DELETEITEM:
		if ((cwin = (swcBase *)((DELETEITEMSTRUCT *)lparam)->CtlID) != NULL)
		{
			if (cwin->type == SWC_WT_LIST)
				((swcList *)cwin)->yyydeleteitem(
						(swcList::listitem *)(((DELETEITEMSTRUCT *)lparam)->itemData));
		}
		break;

	 case WM_NOTIFY:
		pnmhdr = (NMHDR *)lparam;
		if (pnmhdr->code == TCN_SELCHANGE)
		{
			pwin = (swcDisplayBase *)GetWindowLong(pnmhdr->hwndFrom, GWL_USERDATA);
			if (pwin->type == SWC_WT_TABDISP)
				((swcTabDisp *)pwin)->selchanged(TabCtrl_GetCurSel(
						pnmhdr->hwndFrom));
		}
		break;

	 case WM_COMMAND:
		if ((pwin = (swcDisplayBase *)GetWindowLong((HWND)lparam, GWL_USERDATA))
				!= NULL)
		{
			if (pwin->action != NULL &&
					((pwin->type == SWC_WT_BUTTON &&
						!((swcButton *)pwin)->ignore) ||
					(pwin->type == SWC_WT_LIST &&
						HIWORD(wparam) == LBN_DBLCLK) ||
					((pwin->type == SWC_WT_DROPDOWN || pwin->type == SWC_WT_LIST) &&
						HIWORD(wparam) == LBN_SELCHANGE)))
				(pwin->action)(pwin, -1, pwin->arg2data, pwin->arg3data);
			else if (pwin->type == SWC_WT_DROPDOWN &&
					((swcDropDown *)pwin)->processcmdmsg(wparam))
				return (0);
		}
		break;
/*
	 case WM_CTLCOLORSTATIC:
	 case WM_CTLCOLORBTN:
		if ((cwin = (swcBase *)GetWindowLong((HWND)lparam, GWL_USERDATA))
				!= NULL)
			return (cwin->ctlcolor((HDC)wparam));
		break;
*/
	 case WM_DRAWITEM:
		if ((cwin = (swcBase *)GetWindowLong(
				((DRAWITEMSTRUCT *)lparam)->hwndItem, GWL_USERDATA)) != NULL)
			cwin->drawitem((DRAWITEMSTRUCT *)lparam);
		break;

	 case WM_VSCROLL:
		if (!IsIconic(hwnd) && (pwin = (swcDisplay *)GetWindowLong(hwnd,
				GWL_USERDATA)) != NULL)
		{
			switch(LOWORD(wparam))
			{
			 case SB_TOP:
				pwin->yoffset = 0;
				goto setvsb2;

			 case SB_BOTTOM:
				pwin->yoffset = (pwin->yrange - pwin->height + sfr) / sfd;
				goto setvsb2;

			 case SB_PAGEUP:
				pwin->yoffset -= (pwin->cheight + sfr) / sfd;
				goto setvsb;

			 case SB_PAGEDOWN:
				pwin->yoffset += (pwin->cheight + sfr) / sfd;
				goto setvsb;

			 case SB_LINELEFT:
                if (pwin->yoffset == 0)
                    break;
                pwin->yoffset -= pwin->yincrement;
				goto setvsb;

			 case SB_LINERIGHT:
                if (pwin->yoffset >= ((pwin->yrange - pwin->cheight + sfr) / sfd))
                    break;
				pwin->yoffset += pwin->yincrement;
				goto setvsb;

			 case SB_THUMBPOSITION:
			 case SB_THUMBTRACK:
				sinfo.cbSize = sizeof(sinfo);
				sinfo.fMask = SIF_ALL;
				GetScrollInfo(hwnd, SB_VERT, &sinfo);
				pwin->yoffset = (sinfo.nTrackPos + sfr) / sfd;
			 setvsb:
				if (pwin->yoffset < 0)
					pwin->yoffset = 0;
				else if ((pwin->yoffset * sfd) > (pwin->yrange - pwin->cheight))
					pwin->yoffset = (pwin->yrange - pwin->cheight + sfr) / sfd;
			 setvsb2:
				sinfo.cbSize = sizeof(sinfo);
				sinfo.fMask = SIF_POS;
				sinfo.nPos = pwin->yoffset * sfd;
				SetScrollInfo(hwnd, SB_VERT, &sinfo, TRUE);
				pwin->adjustchilds();
				InvalidateRect(hwnd, NULL, TRUE);
				break;
			}
		}
		break;

	 case WM_HSCROLL:
		if (!IsIconic(hwnd) && (pwin = (swcDisplay *)GetWindowLong(hwnd,
				GWL_USERDATA)) != NULL)
		{
			switch(LOWORD(wparam))
			{
			 case SB_LEFT:
				pwin->xoffset = 0;
				goto sethsb2;

			 case SB_RIGHT:
				pwin->xoffset = pwin->xrange - pwin->width;
				goto sethsb2;

			 case SB_PAGELEFT:
				pwin->xoffset -= pwin->cwidth;
				goto sethsb;

			 case SB_PAGERIGHT:
				pwin->xoffset += pwin->cwidth;
				goto sethsb;

			 case SB_LINELEFT:
                if (pwin->xoffset == 0)
                    break;
                pwin->xoffset -= pwin->xincrement;
				goto sethsb;

			 case SB_LINERIGHT:
                if (pwin->xoffset >= ((pwin->xrange - pwin->cwidth + sfr) / sfd))
                    break;
				pwin->xoffset += pwin->xincrement;
				goto sethsb;

			 case SB_THUMBPOSITION:
			 case SB_THUMBTRACK:
				sinfo.cbSize = sizeof(sinfo);
				sinfo.fMask = SIF_ALL;
				GetScrollInfo(hwnd, SB_HORZ, &sinfo);
				pwin->xoffset = sinfo.nTrackPos;
			 sethsb:
				if (pwin->xoffset < 0)
					pwin->xoffset = 0;
				else if (pwin->xoffset > (pwin->xrange - pwin->cwidth))
					pwin->xoffset = pwin->xrange - pwin->cwidth;
			 sethsb2:
				sinfo.cbSize = sizeof(sinfo);
				sinfo.fMask = SIF_POS;
				sinfo.nPos = pwin->xoffset;
				SetScrollInfo(hwnd, SB_HORZ, &sinfo, TRUE);
				pwin->adjustchilds();
				InvalidateRect(hwnd, NULL, TRUE);
				break;
			}
		}
		break;

	 case WM_SETFOCUS:
		if ((cwin = (swcBase *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
			cwin->checkfocus(HWND(wparam));
		break;

	 case WM_MOUSEMOVE:
		if ((pwin = (swcDisplay *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL &&
				 pwin->action != NULL && pwin->mmoveaction)
		{
			temp = GetMessagePos();
			point.x = (short)temp;
			point.y = temp >> 16;
			ScreenToClient(hwnd, &point);
			mmove.xpos = point.x * sfd;
			mmove.ypos = point.y * sfd;
			mmove.zdelta = 0;
			mmove.time = GetMessageTime();
			mmove.arg = pwin->arg2data;
			(pwin->action)(pwin, 4, (long)&mmove, pwin->arg3data);
		}
		break;

	 case WM_LBUTTONDOWN:
		if ((pwin = (swcDisplay *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL &&
				 pwin->action != NULL)
		{
			temp = GetMessagePos();
			point.x = (short)temp;
			point.y = temp >> 16;
			ScreenToClient(hwnd, &point);
			mmove.xpos = point.x * sfd;
			mmove.ypos = point.y * sfd;
			mmove.zdelta = 0;
			mmove.time = GetMessageTime();
			mmove.arg = pwin->arg2data;
			(pwin->action)(pwin, 1, (long)&mmove, pwin->arg3data);
		}
		break;

	 case WM_LBUTTONDBLCLK:
		if ((pwin = (swcDisplay *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL &&
				 pwin->action != NULL)
			(pwin->action)(pwin, 2, pwin->arg2data, pwin->arg3data);
		break;

	 case WM_RBUTTONDOWN:
		if ((pwin = (swcDisplay *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL &&
				 pwin->action != NULL)
			(pwin->action)(pwin, 3, pwin->arg2data, pwin->arg3data);
		break;

	 case WM_MOUSEWHEEL:
		if (swcwheelwin != NULL)
		{

			temp = (short)HIWORD(wparam);

			temp = (long)(temp/120.0 * swcwheelwin->yincrement);

		    if (temp > 0)
            {
                if (swcwheelwin->yoffset <= 0)
                    break;
            }
            else
            {
                if (swcwheelwin->yoffset >= ((swcwheelwin->yrange -
                        swcwheelwin->cheight + sfr) / sfd))
                    break;
            }
          	swcwheelwin->yoffset -= temp;

			if (swcwheelwin->yoffset < 0)
				swcwheelwin->yoffset = 0;
			else if (swcwheelwin->yoffset > ((swcwheelwin->yrange -
					swcwheelwin->cheight + sfr) / sfd))
				swcwheelwin->yoffset = (swcwheelwin->yrange -
						swcwheelwin->cheight +sfr) / sfd;
			sinfo.cbSize = sizeof(sinfo);
			sinfo.fMask = SIF_POS;
			sinfo.nPos = swcwheelwin->yoffset * sfd;
			SetScrollInfo(swcwheelwin->hwin, SB_VERT, &sinfo, TRUE);
			swcwheelwin->adjustchilds();
			InvalidateRect(swcwheelwin->hwin, NULL, TRUE);
			return (0);



/*			temp = GetMessagePos();
			point.x = (short)temp;
			point.y = temp >> 16;
			ScreenToClient(hwnd, &point);
			mmove.xpos = point.x * sfd;
			mmove.ypos = point.y * sfd;
			mmove.zdelta = (short)(wparam >> 16);
			mmove.time = GetMessageTime();
			mmove.arg = pwin->arg2data;
			(pwin->action)(pwin, 5, (long)&mmove, pwin->arg3data);
*/
		}
		break;

	 case WM_SIZE:
		if ((pwin = (swcDisplay *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
		{
			pwin->GetClientRect(&rect);
			pwin->cwidth = rect.right;
			pwin->cheight = rect.bottom;
			cwin = pwin->fchild;
			while (cwin != NULL)
			{
				pxpos = cwin->xpos;
				pypos = cwin->ypos;
				pwidth = cwin->width;
				pheight = cwin->height;
			
				abits = cwin->anchor & ((SWC_ANCHOR_LEFT |
						SWC_ANCHOR_RIGHT) >> 20);
				if (abits == ((SWC_ANCHOR_LEFT | SWC_ANCHOR_RIGHT) >> 20))
					pwidth = pwin->cwidth - cwin->xpos - cwin->xanchor;
				else if (abits == (SWC_ANCHOR_RIGHT >> 20))
					pxpos = pwin->cwidth - cwin->width - cwin->xanchor;

				abits = cwin->anchor & ((SWC_ANCHOR_TOP |
						SWC_ANCHOR_BOTTOM) >> 20);
				if (abits == ((SWC_ANCHOR_TOP | SWC_ANCHOR_BOTTOM) >> 20))
					pheight = pwin->cheight - cwin->ypos - cwin->yanchor;
				else if (abits == (SWC_ANCHOR_BOTTOM >> 20))
					pypos = pwin->cheight - cwin->height - cwin->yanchor;

				if (pwidth != cwin->width || pheight != cwin->height)
					cwin->MoveResize(pxpos, pypos, pwidth, pheight, true);
				else if (pxpos != cwin->xpos || pypos != cwin->ypos)
					cwin->Move(pxpos, pypos, true);
				cwin = cwin->sibling;
			}
		}
		break;

	 case WM_CLOSE:
		if ((pwin = (swcDisplay *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
		{
			if (pwin == &swc_BaseWin)
			{
				if (swc_BaseClose != NULL && !swc_BaseClose())
					return (0);
			}
			else
			{
				if (!(pwin->haveclose()))
					return (0);
			}
		}
		break;

	 case WM_DESTROY:
		if ((pwin = (swcDisplay *)GetWindowLong(hwnd, GWL_USERDATA)) != NULL)
		{
			if (pwin == &swc_BaseWin)
			{
				swc_Ending = true;
				PostQuitMessage(0);
			}
			else if (!(pwin->dieing))
			{
				SetWindowLong(pwin->hwin, GWL_USERDATA, 0);
				pwin->hwin = NULL;
				delete pwin;
			}
		}
		break;
	}
	return (DefWindowProc(hwnd, message, wparam, lparam));
}


int swcDisplayBase::haveclose()

{
	return (true);
}


int swcDisplayBase::havefocus(
	swcBase *win)

{
	NOTUSED(win);

	return (false);
}


void swcDisplayBase::msg111(MSG *msg)

{

}

void swcDisplayBase::msg112(MSG *msg)

{

}


void swcDisplayBase::adjustchilds()

{
	swcBase *child;

	if ((child = fchild) != NULL)
	{
		HDWP hdwp = BeginDeferWindowPos(10);
		do
		{
			DeferWindowPos(hdwp, child->hwin, 0, child->xpos - xoffset,
					child->ypos - yoffset, 0, 0, SWP_NOOWNERZORDER|
					SWP_NOZORDER|SWP_NOSIZE|SWP_SHOWWINDOW);
		} while ((child = child->sibling) != NULL);
		EndDeferWindowPos(hdwp);
	}
}

long debugxxx;

long drawtextblk(
    char *text,
    RECT *lrect,
    long  tcolor,
    long  newfontnum,
    long  curfontnum,
    HDC   hdc)
{
    char *pnt;
    char *bgn;
    RECT  trect;
    SIZE  tsize;
    int   tmp;
    int   lheight;
    long  left;
    long  hpos;
    long  hsize;
    long  hsizev;
    char  chr;

    if (strncmp(text, "Error ", 6) == 0)
        debugxxx++;

    pnt = text;
	hpos = hsize = 0;
    left = lrect->left;
    lheight = 16;
	while (true)
	{
		bgn = pnt;
		while ((chr = *pnt++) != 0 && chr != 0x11 && chr != '\n')
			;
        trect = *lrect;
		trect.left = lrect->left + (hpos + sfr) / sfd;
		if ((hsizev = hsize) < 0)
			hsizev = -hsize;
		trect.right = (hsizev != 0) ? (trect.left + hsizev) : lrect->right;
		if ((tmp = pnt - bgn - 1) > 0)
		{
			if (curfontnum != newfontnum)
			{
			    DeleteObject(SelectObject(hdc, swcgetscnfont(newfontnum)));
				curfontnum = newfontnum;
			}
			DrawText(hdc, bgn, tmp, &trect, (hsize <= 0) ?
					(DT_SINGLELINE | /* DT_VCENTER | */ DT_LEFT | DT_WORDBREAK |
					DT_NOPREFIX) : (DT_SINGLELINE | DT_VCENTER |
					DT_RIGHT | DT_NOPREFIX));
			hsize = 0;
            GetTextExtentPoint32(hdc, bgn, tmp, &tsize);
            lrect->left += tsize.cx;
            lheight = tsize.cy;
		}
		if (chr == 0)
			break;
		if (chr == '\n')
		{
			lrect->left = left;
			lrect->top += lheight;
///			lrect->bottom += lheight;
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

			 case 0x08:				// Font number (always 2 bytes)
                newfontnum &= 0xFFFF0000;
				newfontnum |= (((uchar)*pnt++) << 8);
				newfontnum |= ((uchar)*pnt++);
				break;

			 case 0x09:				// Font size (always 2 bytes)
                newfontnum &= 0x0000FFFF;
				newfontnum |= ((uchar)*pnt++) << 24;
				newfontnum |= ((uchar)*pnt++) << 16;
				break;

			 case 0x0A:				// Font color (always 3 bytes,)
				tcolor = ((uchar)*pnt++) << 16;
				tcolor |= (((uchar)*pnt++) << 8);
				tcolor |= ((uchar)*pnt++);
				SetTextColor(hdc, tcolor);
				break;

			 case 0x0B:				// Line height (always 2 bytes)
				trect.top += ((((uchar)*pnt++) + sfr) / sfd);
				lheight = (((uchar)*pnt++) + sfr) / sfd;
				trect.bottom = trect.top + lheight;
				break;

			 case 0x0C:				// Default font color (always 0 bytes)
///				SetTextColor(hdc, (dis->itemState & 0x01) ? 0xFFFFFF :
///						0x000000);
                break;
			}
		}
	}
    return (curfontnum);
}