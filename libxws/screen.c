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

// This module contains routines to support console screen switching.  The
//   screen symbiont can not display the system menu on a graphics screen and
//   does not support saving and restoring graphics screens when switching
//   between screens.  Thus we must do all of this. When the console is in
//   a graphic mode the SYSSHF character is sent directly to the console's
//   input stream, not to the screen symbiont. The terminal is never put in
//   the system shift state. When a switch to this screen is complete a
//   SSCOMP character is sent to the console's input stream. When this
//   character is seen, the entire screen is invalidated, causing all
//   windows to be redrawn. A switch from this screen is iniated by sending
//   an IPM message to the screen symbiont.

#define WIDTH  100000

static long numscreens;
static long lheight = 0;

static long    xpos = 0x80000000;
static long    ypos = 2000;
static XWSWIN *winfrm;
static XWSWIN *winlst;

static char chrname[] = "PROCESS:";
static char namebfr[40];

static struct
{   BYTE4CHAR  num;
    uchar      end;
} trmlist =
{   {PAR_GET|REP_DECV, 4, "NUMSCN"},
};

static struct
{   TEXT8CHAR  trm;
    LNGSTRCHAR prg;
    uchar      end;
} chrlist =
{   {PAR_SET|REP_TEXT, 8, "CONTRM"},
    {PAR_GET|REP_STR , 0, "NAME", namebfr, sizeof(namebfr), sizeof(namebfr)}
};

static QAB chrqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// func    = 0  - Function
    0,							// status  = 2  - Returned status
    0,							// error   = 4  - Error code
    0,							// amount  = 8  - Amount transfered
    0,							// handle  = 12 - Device handle
    0,							// vector  = 16 - Vector for interrupt
    {0},
    CF_VALUES,					// option  = 20 - Option bits
    0,							// count   = 24 - Amount to transfer
    chrname,					// buffer1 = 28 - Pointer to data buffer
	(char *)&chrlist,			// buffer2 = 32 - Pointer to characteristic list
    NULL						// parm    = 36 - Pointer to parameter list
};

static XWSEVENT eventmenu;
static void     sendswitch(long value);


//****************************************************************//
// Function: xwsInitScreen - Initialize screen switching routines //
// Returned: Nothing                                              //
//****************************************************************//

void xwsInitScreen(void)
{
	svcIoDevChar(xws_TrmHndl, (char *)&trmlist); // Get number of screens
	if ((numscreens = trmlist.num.value) > 35) // Make sure not more than 35
		numscreens = 35;					   //   (Since we use a single
											   //   character to select a
											   //   screen and we only want to
}											   //   use digits and leters.)


//*******************************************************************//
// Function: xwsdisplaysessionmenu - Display the session select menu //
// Returned: Nothing                                                 //
//*******************************************************************//

void xwsdisplaysessionmenu(void)
{
	static char title[] = "Session Select";

	XWSWINPAR winparms;
	XWSTXTPAR txtparms;
	XWSRECT   rect;
	int       num;
	int       len;
	int       lblchr;
	int       selfnc;
	char      menutext[100];

	if (xws_ScreenWIN == NULL)
		return;
	if (winfrm != NULL)
	{
		xwsWinDestroy((XWSWIN *)winfrm);
		winfrm = NULL;
		return;
	}
	if (xpos == 0x80000000)
	{
		xwsWinGetClient((XWSWIN *)xws_ScreenWIN, &rect);
		xpos = rect.xsize - WIDTH - 2000; // Set initial X position for menu
	}
	winparms.cntr = xws_WinParContainerDefault;
	winparms.cntr.zorder = 253;
	winparms.cntr.bgdcolor = 0x0000FF;
	winparms.cntr.bdrwidthc = 2000;
	winparms.cntr.bdrwidtho = 2000;
	winparms.cntr.bdrcolor = 0x0000FF;
	winparms.cntr.bits2 |= WB2_NOFOCUS;
	if (xwsWinCreateContainer(xws_ScreenWIN, xpos, ypos, WIDTH, lheight + 18000,
			&winparms, eventmenu, XWS_EVMASK_WINDOW|XWS_EVMASK_KEYMAKE, &xwsedb,
			&winfrm) < 0)
		return;
	txtparms = xws_TxtParContainerDefault;
	txtparms.fheight = 8000;
	txtparms.fnum = 1;
	txtparms.topmargin = 0;
	txtparms.btmmargin = 0;
	txtparms.leftmargin = 0;
	txtparms.rightmargin = 0;
	txtparms.lfmt = TLF_BCENTER;
	txtparms.txtcolor = 0xFFFFFF;
	xwsWinTextNew((XWSWIN *)winfrm, 1, 0, 0, 0, 1, &txtparms, title,
			sizeof(title) - 1);

	winparms.list = xws_WinParListDefault;
	winparms.list.cursor = xws_CurFinger;
	winparms.list.bdrwidtho = 2000;
	winparms.list.bits2 = (WB2_MENULST|WB2_MODAL|WB2_NOSHWFCS|WB2_RECESSO);
	winparms.list.bgdcolor = 0xE0E0D0;

///	winparms.list.bgcols = 0xA0C0FF;

	if (xwsWinCreateList(winfrm, 0, 10000, WIDTH - 8000, lheight,
			&winparms, eventmenu, XWS_EVMASK_WINDOW, &xwsedb, &winlst) < 0)
		return;


	txtparms = xws_TxtParContainerDefault;
	txtparms.fheight = 7000;
	txtparms.fnum = 1;
	txtparms.bgdcolor = 0xE0E0D0;
	txtparms.txtcolorsel = 0xFFFFFF;

///	txtparms.hlcolor = 0xC0D0FF;

	txtparms.lspace = 0;
	txtparms.leftmargin = 2000;
	num = 1;
	do
	{
		sprintf(chrlist.trm.value, "TRM%dS%d", xwstrmnum, num);
		if (svcIoQueue(&chrqab) < 0 || chrqab.error < 0)
			namebfr[0] = 0;

		// 10-01				Select list item
		// 10-D0-A0-A0-A0-00	Set FG color = 0xA0A0A0
		// 10-C2-10-27-00-00	Set horiz pos = 10000

		if (num == xwsscnnum)
		{
			lblchr = '>';
			selfnc = TF_SELITEM;
		}
		else
		{
			lblchr = ':';
			selfnc = TF_SELITEM | 0x80000000;
		}
		len = sprintf(menutext, "%v%c%c%v%s", selfnc,
				(num < 10) ? ('0' + num) : ('A' - 10 + num),
				lblchr, TF_HORPOS, 13000, namebfr);
		txtparms.txtcolor = (namebfr[0] == 0) ? 0xA0A0A0 : 0x000000;
		xwsWinTextNew((XWSWIN *)winlst, num, 0, 0, 0, 7000, &txtparms,
				menutext, len);
	} while (++num <= numscreens);

	if (lheight == 0)
	{
		xwsWinTextGetBBox((XWSWIN *)winlst, numscreens, &rect);
		lheight = rect.ypos + rect.ysize + 5000;
		xwsWinMove((XWSWIN *)winfrm, xpos, ypos, WIDTH, lheight + 18000);
		xwsWinMove((XWSWIN *)winlst, 0, 10000, WIDTH - 8000, lheight);
	}
}


//*****************************************************************
// Function: eventmenu - Event function for the session menu window
// Returend: TRUE if event handled, FALSE otherwise
//*****************************************************************

static long eventmenu(
	XWSEDBLK *edblk)
{
	int  item;
	char chr;

    if (edblk->type == XWS_EVENT_MOVED)
    {
		xpos = ((XWSEDBLK_MOUSE *)edblk)->xpos;
		ypos = ((XWSEDBLK_MOUSE *)edblk)->ypos;
		return (XWS_EVRTN_SYS);
    }
    if (edblk->type == XWS_EVENT_WINDOW)
    {
		if (edblk->subtype == XWS_EVENT_WIN_LIST_SEL)
			sendswitch(((XWSEDBLK_LIST *)edblk)->index);
		return (XWS_EVRTN_DONE);
    }
    if (edblk->type == XWS_EVENT_KEYMAKE)
	{
		chr = ((XWSEDBLK_KEYBRD *)edblk)->charcd;
		if (chr == ESC)
		{
			xwsWinDestroy((XWSWIN *)winfrm);
			winfrm = NULL;
			return (XWS_EVRTN_DONE);
		}
		if ((chr == '\r' || chr == ' ') &&
			(item = xwsWinListGetSelItem(winlst)) > 0)
		{
			sendswitch(item);
			return (XWS_EVRTN_DONE);
		}
		if (((XWSEDBLK_KEYBRD *)edblk)->scancd == ISC_SSCOMP)
		{
			xws_ScreenWIN->drw->disable = FALSE;
			xwsWinInvalidateAll((XWSWIN *)xws_ScreenWIN);
			return (XWS_EVRTN_DONE);
		}
		if (isdigit(chr))
			item = chr & 0x0F;
		else if (isalpha(chr))
			item = (chr & 0x1F) + 9;
		else
		    return (XWS_EVRTN_SYS);
		if (item >= 1 && item <= numscreens)
		{
			sendswitch(item);
			return (XWS_EVRTN_DONE);
		}
	}
    return (XWS_EVRTN_SYS);
}


//**************************************************************
// Function: sendswitch - Send report message to screen symbiont
// Returned: Nothing
//**************************************************************

static void sendswitch(
    long value)
{
    xwsWinDestroy((XWSWIN *)winfrm); // Remove the session menu window
    winfrm = NULL;
    if (value != xwsscnnum)				// Requesting switch to self?
	{
		xws_ScreenWIN->drw->disable = TRUE; // No - disable screen access
		svcTrmSwScreen(DH_STDTRM, value); // Switch screens
	}
}
