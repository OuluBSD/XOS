//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <xosxxws.h>

// A scroll bar is a composite window which consists of a container window with
//   3 child windows, each of which is a button. One dimension of the container
//   window must be at least 2 times the other dimension.

// The main part of the window parameters for a scroll bar apply to the
//   container window. There are additional values added for scroll bars
//   that specify parameters for the contained buttons. The text parameters
//   specified apply to the two scroll buttons.

static void     buttondown(XWSSBAR *sbw, int subtype, int sbit);
static void     buttonup(XWSSBAR *sbw);
static XWSEVENT btn1event;
/* static */ XWSEVENT btn2event;
/* static */ XWSEVENT thumbevent;

static XWSTIMER sbtimer1;
static XWSTIMER sbtimer2;
static void     updatethumb(XWSSBAR *sbwin);

int valbgbmn = offsetof(XWSWIN, bgdbm);


//********************************************************
// Function: xwsWinCreateScrollBar - Creates a scrollbar
// Returned: 0 if normal or a negative error code if error
//********************************************************

// This function uses the Pascal calling sequence

long xwsWinCreateScrollBar(
	XWSWIN     *parent,		// Parent window
	long        xpos,		// X position
	long        ypos,		// Y position
	long        xsize,		// Width
	long        ysize,		// Height
	XWSWINPAR  *parms,		// Window parameters
	XWSEVENT   *event,		// Pointer to caller's event function (may be null)
	long        evmask,		// Event mask bits
	XWSEDB     *edb,		// Pointer to caller's environment data block
	XWSWIN    **pwin)		// Pointer to location to receive address of
							//   the WIN created
{
	XWSWIN    *sbwin;
///	XWSSBARPAR barpar;
	XWSBUTNPAR btnpar;
	XWSTXTPAR  txtpar;
	long       rtn;
	long       btnw;
	long       btnh;
	long       btn2x;
	long       btn2y;
	long       thumbx;
	long       thumby;
	long       thumbw;
	long       thumbh;

	// Create the underlying "simple" window

	if (parent != NULL)
	{
		xpos = scalex(parent, xpos);
		ypos = scaley(parent, ypos);
		xsize = scalex(parent, xsize);
		ysize = scaley(parent, ysize);
	}
	if ((rtn = xwscreatewindow(parent, XWS_WINTYPE_SCROLLBAR, sizeof(XWSSBAR),
			xpos, ypos, xsize, ysize, parms, event, evmask, edb, parent->drw,
			&sbwin)) < 0)
		return (rtn);
	sbwin->tmbrange = parms->tmbrange;
	sbwin->tmbsize = parms->tmbsize;
	sbwin->tmbpos = parms->tmbpos;
	sbwin->tmrhndl = 0;
	sbwin->delayi = parms->delayi;
	sbwin->delayr = parms->delayr;

	txtpar = xws_TxtParButtonDefault;

	txtpar.lspace = 600;

	// Determine the orientation of the scroll bar

	if (xsize > ysize)
	{
		// Here if have a horizontal scroll bar

		thumbh = btnh = 0;
		sbwin->tmbpxbegin = btnw = (parms->btnsize * sbwin->clntheight)/100;
		sbwin->tmbpxsize = thumbw = ((sbwin->clntwidth - 2 * btnw) *
				sbwin->tmbsize) / sbwin->tmbrange;
		btn2x = sbwin->clntwidth - btnw;
		btn2y = 0;
		sbwin->tmbpxtotal = sbwin->clntwidth - 2 * btnw;
		sbwin->tmbpxpos = (parms->tmbpos * sbwin->tmbpxtotal) / sbwin->tmbrange;
		thumbx = btnw + sbwin->tmbpxpos;
		thumby = 0;
		sbwin->state = 0;
	}
	else
	{
		// Here if have a vertical scroll bar

		thumbw = btnw = 0;
		sbwin->tmbpxbegin = btnh = (parms->btnsize * sbwin->clntwidth)/100;
		sbwin->tmbpxsize = thumbh = ((sbwin->clntheight - 2 * btnh) *
				sbwin->tmbsize) / sbwin->tmbrange;
		btn2x = 0;
		btn2y = sbwin->clntheight - btnh;
		sbwin->tmbpxtotal = sbwin->clntheight - 2 * btnh;
		thumbx = 0;
		sbwin->tmbpxpos = (parms->tmbpos * sbwin->tmbpxtotal) / sbwin->tmbrange;
		thumby = btnh + sbwin->tmbpxpos;
		sbwin->state = XWSSBS_VERT;

		txtpar.fheight = (btnh * sbwin->ysfd * 7) / 10;
	}

	// Create button 1

	btnpar = xws_WinParButtonDefault;

	btnpar.bits2 |= WB2_NOSHWFCS|WB2_NOFOCUS;

	btnpar.bdrwidtho = parms->btnbdwid;
	btnpar.bgdcolor = parms->btnbgcoln;

///	btnpar.bgdcoloralt = parms->btnbgdcoloralt;
	btnpar.hlcolor = parms->hlcolor;
	btnpar.hlcoloralt = parms->hlcoloralt;
	btnpar.shdcolor = parms->shdcolor;
	btnpar.shdcoloralt = parms->shdcoloralt;
	btnpar.shift = parms->shift;

	if ((rtn = xwsWinCreateButton(sbwin, 0, 0, btnw | 0x80000000,
			btnh | 0x80000000, (XWSWINPAR *)&btnpar, &txtpar, btn1event,
			XWS_EVMASK_MOUSEBT, edb, &sbwin->btn1, parms->btn1chr,
			3)) < 0)
	{
		xwsWinDestroy(sbwin);
		xwsnoscale = TRUE;
		return (rtn);
	}
	sbwin->btn1->config |= WCB_EMBEDDED;

	// Create button 2

	if ((rtn = xwsWinCreateButton(sbwin, btn2x | 0x80000000, btn2y | 0x80000000,
			btnw | 0x80000000, btnh | 0x80000000, (XWSWINPAR *)&btnpar,
			&txtpar, btn2event, XWS_EVMASK_MOUSEBT, edb, &sbwin->btn2,
			parms->btn2chr, 3)) < 0)
	{
		xwsWinDestroy((XWSWIN *)&sbwin->btn1);
		xwsWinDestroy(sbwin);
		xwsnoscale = TRUE;
		return (rtn);
	}
	sbwin->btn2->config |= WCB_EMBEDDED;

	// Create the thumb

	btnpar.bdrwidtho = parms->tmbbdwid;
	btnpar.bgdcolor = parms->tmbbgcoln;
	btnpar.bgdcoloralt = parms->tmbbgcola;
	if ((rtn = xwsWinCreateButton(sbwin, thumbx | 0x80000000,
			thumby | 0x80000000, thumbw | 0x80000000, thumbh | 0x80000000,
			(XWSWINPAR *)&btnpar, NULL, thumbevent, XWS_EVMASK_MOUSEBT|
			XWS_EVMASK_MOUSEMV|XWS_EVMASK_WINDOW, edb, &sbwin->thumb,
			NULL, 0)) < 0)
	{
		xwsWinDestroy((XWSWIN *)&sbwin->btn2);
		xwsWinDestroy((XWSWIN *)&sbwin->btn1);
		xwsWinDestroy(sbwin);
		xwsnoscale = TRUE;
		return (rtn);
	}
	sbwin->thumb->config |= WCB_EMBEDDED;

	xwsnoscale = FALSE;

	*pwin = sbwin;
	return (0);
}


//*******************************************************************//
// Function: xwsWinScrollBarSet - Set size and position of the thumb //
// Returned:                                                         //
//*******************************************************************//

long xwsWinScrollBarSet(
	XWSWIN *win,
	long    tmbrange,			// 0 means no change
	long    tmbpos,				// < 0 means no change
	long    tmbsize)			// < 0 means no change
{
	long xpos;
	long ypos;
	long width;
	long height;

	if (win->label != 'WIN#')
		xwsFail(0, "xwsWinScrollBarSet: Bad WIN");

	if (win->type != XWS_WINTYPE_SCROLLBAR)
		xwsFail(0, "xwsWinScrollBarSet: Not a scroll bar");

	if (win->thumb->label != 'WIN#')
		xwsFail(0, "xwsWinScrollBarSet: Thumb not a WIN");

	if (tmbrange > 0)
		win->tmbrange = tmbrange;

	if (tmbpos < 0)
		tmbpos = win->tmbpos;

	if (tmbsize < 0)
		tmbsize = win->tmbsize;

	win->tmbsize = (tmbsize <= win->tmbrange) ? tmbsize : win->tmbrange;
	win->tmbpxsize = (win->tmbsize * win->tmbpxtotal) / win->tmbrange;

	win->tmbpos = (tmbpos <= 0) ? 0 : (tmbpos <= (win->tmbrange -
			win->tmbsize)) ? tmbpos : (win->tmbrange - win->tmbsize);
	win->tmbpxpos = (win->tmbpos * win->tmbpxtotal) / win->tmbrange;
	if (win->state & XWSSBS_VERT)
	{
		xpos = 0;
		ypos = win->tmbpxpos + win->tmbpxbegin;
		width = 0;
		height = win->tmbpxsize;
	}
	else
	{
		xpos = win->tmbpxpos + win->tmbpxbegin;
		ypos = 0;
		width = win->tmbpxsize;
		height = 0;
	}
	xwswinmovens((XWSWIN *)win->thumb, xpos, ypos, width, height);
	return (0);
}


long xwsaddhorscrollbar(
	XWSWIN    *win,
	XWSWINPAR *parm,
	XWSEVENT  *hsbevent,
	XWSEDB    *edb)
{
	win = win;
	parm = parm;
	hsbevent = hsbevent;
	edb = edb;

	return (0);
}


///long sbxpos;
///long sbypos;
///long sbwidth;
///long sbheight;
///XWSWIN *sbwin;


long XOSFNC xwsaddverscrollbar(
	XWSWIN    *win,
	XWSWINPAR *parms,
	XWSEVENT  *vsbevent,
	XWSEDB    *edb)
{
	XWSWIN    *sbwin;
	XWSSBARPAR sbparms;
	long       rtn;

	sbparms = xws_WinParScrollBarDefault;

	// Set up the base parameter values for the scrollbar

///	sbparms.bgdcolor = parms->sbbtnbgcoln; // Background color (Normal button
										   //   background color)

	sbparms.bgdcolor = parms->sbbarcol;	// Background color (Bar color)

///	sbparms.bgdcolor = parms->sbbtnbgcoln; // Normal button background color

	sbparms.hlcolor = parms->sbhlcoln;	// Overall highlight color
	sbparms.shdcolor = parms->sbshcoln;	// Overall shadow color
	sbparms.bdrwidtho = parms->sbbdwid;	// Outer border width for base window
										//   width)
	sbparms.bdrwidthc = 0;				// Center border width for base window
	sbparms.bdrwidthi = 0;				// Inner border width for base window





	sbparms.tmbrange = sbparms.tmbsize = win->clntheight; // Thumb range & size
	sbparms.tmbbgcoln = parms->sbtmbbgcoln; // Normal Thumb background color
	sbparms.tmbbgcola = parms->sbtmbbgcola; // Alternate Thumb background color
	sbparms.tmbbdwid = parms->sbtmbbdwid; // Thumb border size
	sbparms.btnsize = parms->sbbtnsize;	// Button size
	sbparms.btnbdwid = parms->sbbtnbdwid; // Button border size

	*(long *)sbparms.btn1chr = *(long *)parms->sbbtntchr;
										// Character for top button
	*(long *)sbparms.btn2chr = *(long *)parms->sbbtnbchr;
										// Character for bottom button
	sbparms.btntxcoln = parms->sbbtntxcoln;
	sbparms.btntxcola = parms->sbbtntxcola;
	sbparms.btnbgcoln = parms->sbbtnbgcoln; // Normal button background color
	sbparms.btnbgcola = parms->sbbtnbgcola; // Alternate button background color
	sbparms.btnhlcoln = parms->sbbtnhlcoln; // Normal button highlight color
	sbparms.btnhlcola = parms->sbbtnhlcola; // Alternate button highlight color
	sbparms.btnshcoln = parms->sbbtnshcoln; // Normal button shadow color
	sbparms.btnshcola = parms->sbbtnshcola; // Alternate button shadow color
	sbparms.btnshift = parms->sbbtnshift; // Pressed button offset

///	sbxpos = win->clntwidth * win->xsfd - parm->sbsize;
///	sbypos = 0;
///	sbwidth = parm->sbsize;
///	sbheight = win->clntheight * win->ysfd;

	rtn = xwsWinCreateScrollBar(win, (win->clntwidth - scalex(win,
			parms->sbsize)) | 0x80000000, 0, parms->sbsize,
			win->clntheight | 0x80000000, (XWSWINPAR *)&sbparms, vsbevent,
			XWS_EVMASK_WINDOW, edb, &sbwin);
	sbwin->config |= WCB_EMBEDDED;
	win->versb = sbwin;
	win->rbwidth += sbwin->width;
	win->clntwidth -= sbwin->width;
	return (rtn);
}


long xwseventscrollbar(
	XWSEDBLK *edblk)
{
	XWSSBAR *sbw;

	if (edblk->subtype == XWS_EVENT_MBTN_LDN &&
			edblk->type == XWS_EVENT_MOUSEBT)
	{
		sbw = (XWSSBAR *)edblk->orgwin;
		buttondown(sbw, XWS_EVENT_WIN_SB_BAR1, 0);
	}
	return (XWS_EVRTN_DONE);
}


static long btn1event(
	XWSEDBLK *edblk)
{
	XWSSBAR *sbw;

	if (edblk->type == XWS_EVENT_MOUSEBT)
	{
		sbw = (XWSSBAR *)edblk->orgwin->parent;
		if (edblk->subtype == XWS_EVENT_MBTN_LDN)
			buttondown(sbw, XWS_EVENT_WIN_SB_BTN1, XWSSBS_BTN1);
		else if (edblk->subtype == XWS_EVENT_MBTN_LUP)
			buttonup(sbw);
	}
	return (XWS_EVRTN_SYS);
}


/* static */ long btn2event(
	XWSEDBLK *edblk)
{
	XWSSBAR *sbw;

	if (edblk->type == XWS_EVENT_MOUSEBT)
	{
		sbw = (XWSSBAR *)edblk->orgwin->parent;
		if (edblk->subtype == XWS_EVENT_MBTN_LDN)
			buttondown(sbw, XWS_EVENT_WIN_SB_BTN2, XWSSBS_BTN2);
		else if (edblk->subtype == XWS_EVENT_MBTN_LUP)
			buttonup(sbw);
	}
	return (XWS_EVRTN_SYS);
}


static void buttondown(
	XWSSBAR *sbw,
	int      subtype,
	int      sbit)
{
	XWSEDBLK_SCROLLBAR sbblk;

	sbblk.label = 'EDB#';
	sbblk.length = sizeof(XWSEDBLK_SCROLLBAR);
	sbblk.type = XWS_EVENT_WINDOW;
	sbblk.subtype = subtype;
	sbblk.orgwin = (XWSWIN *)sbw;
	sbblk.tmbpos = 0x80000000;
	xwsdoevent((XWSEDBLK *)&sbblk);
	if (sbit != 0 && (sbw->state & sbit) == 0)
	{
		if (sbw->tmrhndl != 0)
		{
			sbw->state &= ~(XWSSBS_BTN1|XWSSBS_BTN2|XWSSBS_BAR1|XWSSBS_BAR2);
			xwsTimerStop(sbw->tmrhndl);
		}
		sbw->state |= sbit;
		sbw->tmrhndl = xwsTimerStart(sbtimer1, (long)sbw, (1LL << 63) +
				sbw->delayi);
	}
}


static void buttonup(
	XWSSBAR *sbw)
{
	sbw->state &= ~(XWSSBS_BTN1|XWSSBS_BTN2|XWSSBS_BAR1|XWSSBS_BAR2);
	if (sbw->tmrhndl != 0)
	{
		xwsTimerStop(sbw->tmrhndl);
		sbw->tmrhndl = 0;
	}
}


// THIS IS WRONG - NEEDS TO HANDLE VERT AND HORIZ SEPARATELY!

/* static */ long thumbevent(
	XWSEDBLK *edblk)
{
	XWSSBAR           *sbw;
	XWSBUTN           *btw;
	XWSEDBLK_SCROLLBAR sbblk;
	long               pxpos;

	if (edblk->type == XWS_EVENT_MOUSEBT || edblk->type == XWS_EVENT_MOUSEMV)
	{
		if (((XWSEDBLK_MOUSE *)edblk)->buttons & XWS_EVENT_MBTN_L)
		{
			btw = (XWSBUTN *)edblk->orgwin;
			sbw = (XWSSBAR *)btw->parent;

			// Get pixels from the left end of the button's client area

			pxpos = (sbw->state & XWSSBS_VERT) ?
					(((XWSEDBLK_MOUSE *)edblk)->ypos /
						btw->ysfd + btw->tbheight) :
					(((XWSEDBLK_MOUSE *)edblk)->xpos /
						btw->xsfd + btw->lbwidth);

			if (edblk->subtype == XWS_EVENT_MBTN_LDN)
				sbw->tmbpxgrab = pxpos;
			else
			{
				// Get new desired pixel postion of the left edge of the button
				//   relative to the start of the bar area

				pxpos = sbw->tmbpxpos + (pxpos - sbw->tmbpxgrab);
				sbblk.tmbpos = (pxpos * sbw->tmbrange) / sbw->tmbpxtotal;
				if (sbblk.tmbpos != sbw->tmbpos)
				{
					sbblk.label = 'EDB#';
					sbblk.length = sizeof(XWSEDBLK_SCROLLBAR);
					sbblk.type = XWS_EVENT_WINDOW;
					sbblk.subtype = XWS_EVENT_WIN_SB_THUMB;
					sbblk.orgwin = (XWSWIN *)sbw;
					xwsdoevent((XWSEDBLK *)&sbblk);
				}
			}
		}
	}
	return (XWS_EVRTN_SYS);
}


void sbtimer1(
	long hndl,
	long data)
{
	sbtimer2(hndl, data);
	((XWSSBAR *)data)->tmrhndl = xwsTimerStart(sbtimer2, data, (1LL << 63) +
			((XWSSBAR *)data)->delayr);
}


void sbtimer2(
	long hndl,
	long data)
{
	(void)hndl;

	if (((XWSSBAR *)data)->state & XWSSBS_BTN1)
		buttondown((XWSSBAR *)data, XWS_EVENT_WIN_SB_BTN1, 0);
	else if (((XWSSBAR *)data)->state & XWSSBS_BTN2)
		buttondown((XWSSBAR *)data, XWS_EVENT_WIN_SB_BTN2, 0);
	else if (((XWSSBAR *)data)->state & XWSSBS_BAR1)
		buttondown((XWSSBAR *)data, XWS_EVENT_WIN_SB_BAR1, 0);
	else if (((XWSSBAR *)data)->state & XWSSBS_BAR2)
		buttondown((XWSSBAR *)data, XWS_EVENT_WIN_SB_BAR2, 0);
}
