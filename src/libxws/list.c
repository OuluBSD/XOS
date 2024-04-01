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

static int XOSFNC puttbinview(XWSWIN *win, TB *tb);


//*************************************************************//
// Function: xwsWinCreateList - Create a list window           //
// Returned: 0 if normal or a negative XOS error code if error //
//*************************************************************//

long XOSFNC xwsWinCreateList(
	XWSWIN    *parent,		// Parent window
	long       xpos,		// X position
	long       ypos,		// Y position
	long       xsize,		// X size (width)
	long       ysize,		// Y size (height)
	XWSWINPAR *parms,		// Parameter structure
	XWSEVENT  *event,		// Pointer to caller's event function (may be null)
	long       evmask,		// Event mask bits
	XWSEDB    *edb,			// Pointer to caller's environment data block
	XWSWIN   **pwin)
{
	XWSWIN *win;
	long    rtn;

	if (parent != NULL)
	{
		xpos = scalex(parent, xpos);
		ypos = scaley(parent, ypos);
		xsize = scalex(parent, xsize);
		ysize = scaley(parent, ysize);
	}
	if ((rtn = xwscreatewindow(parent, XWS_WINTYPE_LIST, sizeof(XWSLIST),
			xpos, ypos, xsize, ysize, parms, event, evmask, edb, parent->drw,
			&win)) < 0)
		return (rtn);
	win->config |= WCB_LIST;
	win->selitem = 0;
	win->topitem = 0;
///	win->alphamin = parms->minvalue;
///	win->listalphamax = parms->maxvalue;

///	win->drw->funcdsp->shwcursor();

	if (win->bits & WB2_HORSB)			// Want a horizontal scroll bar?
		xwsaddhorscrollbar(win, parms, xwslistvsbevent, edb);
	if (win->bits & WB2_VERSB)			// Want a vertical scroll bar?
		xwsaddverscrollbar(win, parms, xwslisthsbevent, edb);
	if (pwin != NULL)
		*pwin = win;
	return (0);
}


//*****************************************************************//
// Function: xwseventlist - System event function for list windows //
// Returned: Event return code                                     //
//*****************************************************************//

long XOSFNC xwseventlist(
	XWSEDBLK *edblk)		// Address of the event data block
{
	XWSWIN       *win;
	TB           *tb;
	XWSEDBLK_LIST edblklst;

	edblklst.subtype = 0;				// Assume no list event will be
										//   generated
	win = edblk->orgwin;
	switch (edblk->type)
	{
	 case XWS_EVENT_MOUSEMV:			// Mouse movement

		if ((win->bits & WB2_MENULST) == 0)
		{
			if ((win->status & WSB_PRESSED) == 0)
				break;
			edblklst.subtype = XWS_EVENT_WIN_LIST_ENT;
		}
		break;

	 case XWS_EVENT_MOUSEBT:			// Mouse button change
		switch (edblk->subtype)
		{
		 case XWS_EVENT_MBTN_LDN:		// Left mouse button pressed
			win->status |= WSB_PRESSED;
			xwscursorgrabwin = win;
			edblklst.subtype = XWS_EVENT_WIN_LIST_SEL;
			if ((tb = xwssearchtextboxlist(win, SCALE(win->ysfr, win->ysfd,
					edblk->ypos))) != NULL)
			{
				xwslistsetseltb(win, tb);
				if (win->selitem != 0 && (win->bits & WB2_NOSCLLST) == 0)
					puttbinview(win, win->selitem);
			}
			break;

		 case XWS_EVENT_MBTN_LUP:		// Left mouse button released?
			win->status &= ~WSB_PRESSED;
			if (win == xwscursorgrabwin) // Have we grabbed the cursor?
			{
				xwscursorgrabwin = NULL; // Yes - but not any more
				xwsupdatecursor();
				edblklst.subtype = XWS_EVENT_WIN_LIST_REL;
			}
			break;
		}
		break;

	 case XWS_EVENT_KEYRELS:
		break;

	 case XWS_EVENT_KEYMAKE:			// Keyboard key make?
		switch (edblk->charcd)
		{
		 case 0:
			switch (edblk->scancd)
			{
			 case ISC_UPARW:				// Up-arrow key
				if ((tb = win->selitem) != NULL && tb->prev != NULL)
					xwslistsetseltb(win, tb->prev);
				break;

			 case ISC_DNARW:				// Down-arrow key
				if ((tb = win->selitem) != NULL && tb->next != NULL)
					xwslistsetseltb(win, tb->next);
				break;
			}
			break;

		 case '\t':
			if ((win->bits & WB2_RHTTRAV) &&
					(edblk->kbsts & (XWS_EVENT_KBS_LFTSHF|
					XWS_EVENT_KBS_RHTSHF)))
				xwstravback(win);
			else if (win->bits & WB2_HTTRAV)
				xwstravahead(win);
			break;

		 case '\r':
			if (win->bits & WB2_CRTRAV)
				xwstravahead(win);
			break;
		}
		break;
	}
	if (edblklst.subtype != 0)			// Want to generate a list event?
	{
		edblklst.label = 'EDB#';
		edblklst.length = sizeof(XWSEDBLK_LIST);
		edblklst.type = XWS_EVENT_WINDOW;
		edblklst.orgwin = win;
		edblklst.index = (win->selitem != NULL) ? win->selitem->inx : 0;
		xwsdoevent((XWSEDBLK *)&edblklst);
	}
	return (XWS_EVRTN_DONE);
}


//********************************************************
// Function: xwsWinListSetSelItem - Set the specified item
//				as the selected item
// Returned: 0 if normal or a negative error code if error
//********************************************************

long XOSFNC xwsWinListSetSelItem(
	XWSWIN *win,
	long    inx)
{
	TB *tb;

	if ((tb = xwsfindtextboxbyinx((XWSWIN *)win, inx, NULL)) == NULL)
		return (ER_NTDEF);
	xwslistsetseltb(win, tb);
	return (0);
}


//***************************************************************
// Function: xwsWinListGetSelItem - Get the current selected item
// Returned: Item index (positive) if normal or a negative error
//				code if error
//***************************************************************

long XOSFNC xwsWinListGetSelItem(
	XWSWIN *win)
{
	TB *tpnt;

	return (((tpnt = win->selitem) == NULL) ? ER_NTSEL : tpnt->inx);
}


//********************************************************
// Function: xwsWinListPutItemAtTop - Put the specified at
//				the top of the widow
// Returned: 0 if normal or a negative error code if error
//********************************************************

long XOSFNC xwsWinListPutItemAtTop(
	XWSWIN *win,
	long    inx)
{
	TB *tpnt;

	if ((tpnt = xwsfindtextboxbyinx((XWSWIN *)win, inx, NULL)) == NULL)
		return (ER_NTDEF);

	xwslistputtbattop(win, tpnt, TRUE);
	return (0);
}


//************************************************************
// Function: xwsWinListPutItemInView - Put the specified item
//				in the displayed window with minimum scrolling
// Returned: 0 if normal or a negative error code if error
//************************************************************

long XOSFNC xwsWinListPutItemInView(
	XWSWIN *win,
	long    inx)
{
	TB *tpnt;

	if ((tpnt = xwsfindtextboxbyinx((XWSWIN *)win, inx, NULL)) == NULL)
		return (ER_NTDEF);
	puttbinview(win, tpnt);
	return (0);
}


//*************************************************
// Function: xwsWinListScroll - Scroll a list widow
// Returned: Nothing
//*************************************************

void XOSFNC xwsWinListScroll(
	XWSWIN *win,
	long    amnt)		// Number of item to scroll. Plus is down, minus is up.
{
	TB *tpnt;

	if ((tpnt = win->topitem) != NULL && amnt != 0)
	{									// Have something to do here?
		if (amnt > 0)					// Yes - moving down?
		{
			do
			{
				if (tpnt->next == NULL)
					break;
				tpnt = tpnt->next;
			} while (--amnt > 0);
		}
		else							// If moving up
		{
			do
			{
				if (tpnt->prev == NULL)
					break;
				tpnt = tpnt->prev;
			} while (++amnt < 0);
		}
		xwslistputtbattop(win, tpnt, TRUE);
	}
}


//********************************************************
// Function: xwslistfindbyypos - Find a text box in a list
//				window given a y position
// Returned: Address of the TB or NULL if not found
//********************************************************

TB * XOSFNC xwslistfindbyypos(
	XWSWIN *win,
	long    ypos)
{
	TB *tpnt;

	tpnt = win->tbhead;
	while (tpnt != NULL)
	{
		if (ypos < (tpnt->ypos + tpnt->ysize))
			return ((ypos >= tpnt->ypos) ? tpnt : NULL);
		tpnt = tpnt->next;
	}
	return (NULL);
}


//******************************************************
// Function: xwslistsetseltb - Set the selected text box
// Returned: Nothing
//******************************************************

void XOSFNC xwslistsetseltb(
	XWSWIN *win,
	TB     *newtb)
{
	XWSEDBLK_LIST edblk;
	TB           *curtb;

	if ((newtb->tbattr & TBA_NOSELECT) == 0 && newtb != win->selitem)
	{
		if ((curtb = win->selitem) != NULL) // Have an item selected now?
		{
			xwswininvalidaterectns(win, 0, curtb->ypos, win->clntwidth,
					curtb->ysize);
			win->selitem = NULL;
		}
		win->selitem = newtb;
		if ((win->bits & WB2_NOSCLLST) == 0)
			puttbinview(win, newtb);
		xwswininvalidaterectns(win, 0, newtb->ypos, win->clntwidth,
				newtb->ysize);
		edblk.label = 'EDB#';
		edblk.length = sizeof(XWSEDBLK_LIST);
		edblk.type = XWS_EVENT_WINDOW;
		edblk.subtype = XWS_EVENT_WIN_LIST_CHG;
		edblk.orgwin = (XWSWIN *)win;
		edblk.arg1 = win->evarg1;
		edblk.arg2 = win->evarg2;
		edblk.index = newtb->inx;
		xwsdoevent((XWSEDBLK *)&edblk);
	}
}


//********************************************************//
// Function: puttbinview - Put the specified item in view //
// Returned: TRUE if window scrolled, FALSE otherwise     //
//********************************************************//

static int XOSFNC puttbinview(
	XWSWIN *win,
	TB     *tb)
{
	TB  *tpnt;
	TB  *chkd;
	long toppos;

	chkd = tpnt = tb;
	if ((tb->ypos + win->yorg) >= win->tbheight) // Is the item above the
	{											 //   window?
		if ((tb->ypos + win->yorg + tb->ysize) <= win->clntheight)
										// is it at least partly below the
										//   window?
			return (FALSE);				// No - no scrolling needed

		// Here if the specifeid item is at least partly below the window.
		//   Scroll down to put the new selected item at the bottom of the
		//   window - This is somewhat difficult since we must first
		//   determine which item needs to be put at the top since we always
		//   scroll such that the top item starts at the top of the window.
		//   There may be a partial item at the bottom.

		toppos = tb->ypos + tb->ysize - win->clntheight;
		while ((tpnt = tpnt->prev) != NULL)
		{
			if (tpnt->ypos < toppos)
				break;
			chkd = tpnt;
		}
		if (tpnt == NULL)
			tpnt = tb;					// If get here the new selected item
										//   must be the last one and must be
										//   bigger than the window. Just put
										//   it at the top.
	}
	xwslistputtbattop(win, chkd, TRUE);
	return (TRUE);
}


//*****************************************************
// Function: xwslistputtbattop - Put the specified text
//				box at the top of the window
// Returned: TRUE if window scrolled, FALSE otherwise
//*****************************************************

int XOSFNC xwslistputtbattop(
	XWSWIN *win,
	TB     *tb,
	int     check)
{
	long toppos;

	if (check)
	{
		toppos = win->tbtail->ypos + win->tbtail->ysize - win->clntheight;
		while (tb->prev != NULL && (tb->ypos - tb->prev->ysize) > toppos)
			tb = tb->prev;
	}
	if (tb != win->topitem)
	{
		win->topitem = tb;
		win->yorg = win->tbheight - tb->ypos;
		xwsWinInvalidateAll((XWSWIN *)win);
		if (win->versb != NULL)
			xwslistscrollbarset(win);
		return (TRUE);
	}
	return (FALSE);
}


//**********************************************************//
// Function: xwslistinvalidatesel - Invalidate the selected //
//              text box in a list window                   //
// Returned: Nothing                                        //
//**********************************************************//

// This function is called when a list window looses or gains focus. If the
//   text or backgound colors are different for the focus case the selected
//   text box is invalidated to force it to be redrawn with the correct
//   colors for the new focus state.

void XOSFNC xwslistinvalidatesel(
	XWSWIN *win)
{
	TB *tb;

	if ((tb = win->selitem) != NULL && (tb->bgdcolorsel != tb->bgdcolorfcs ||
			tb->txtcolorsel != tb->txtcolorfcs))
		xwswininvalidaterectns(win, 0, tb->ypos, win->clntwidth, tb->ysize);
}



void XOSFNC xwslistscrollbarset(
	XWSWIN *win)
{
	XWSWIN *sbwin;
	long    bottom;
	long    tsize;
	long    tpos;

	if (win->label != 'WIN#')
		xwsFail(0, "xwslistscrollbarset: Bad WIN");
	if (win->versb->label != 'WIN#')
		xwsFail(0, "xwslistscrollbarset: Bad VSB WIN");

	sbwin = win->versb;
	bottom = win->tbtail->ypos + win->tbtail->ysize;
	if (bottom > win->clntheight)
	{
		tsize = (win->clntheight * sbwin->tmbrange) / bottom;
		tpos = -(win->yorg * sbwin->tmbrange) / bottom;
	}
	else
	{
		tsize = sbwin->tmbrange;
		tpos = 0;
	}
	xwsWinScrollBarSet(sbwin, -1, tpos, tsize);
}


//**********************************************************
// Function: xwslisthsbevent - Event function for scroll bar
//				events for enbedded horizontal scroll bars
// Returned: XWS_EVRTN_SYS
//**********************************************************

long XOSFNC xwslisthsbevent(
	XWSEDBLK *edblk)
{
	edblk = edblk;

	return(XWS_EVRTN_SYS);
}


//**********************************************************
// Function: xwslistvsbevent - Event function for scroll bar
//				events for enbedded vertical scroll bars
// Returned: XWS_EVRTN_SYS
//**********************************************************

long XOSFNC xwslistvsbevent(
	XWSEDBLK *edblk)
{
	XWSSBAR *sbw;
	TB      *tpnt;

	if (edblk->type == XWS_EVENT_WINDOW)
	{
///		BREAK();

		sbw = (XWSSBAR *)(edblk->orgwin);
		switch (edblk->subtype)
		{
		 case XWS_EVENT_WIN_SB_THUMB:

			if ((tpnt = xwslistfindbyypos(sbw->parent, 
					((XWSEDBLK_SCROLLBAR *)edblk)->tmbpos)) &&
					tpnt != sbw->parent->topitem)
				xwslistputtbattop(sbw->parent, tpnt, TRUE);
			break;

		 case XWS_EVENT_WIN_SB_BTN1:
			xwsWinListScroll(sbw->parent, -1);
			break;

		 case XWS_EVENT_WIN_SB_BAR1:
			break;

		 case XWS_EVENT_WIN_SB_BAR2:
			break;

		 case XWS_EVENT_WIN_SB_BTN2:
			xwsWinListScroll(sbw->parent, 1);
			break;
		}
	}
	return(XWS_EVRTN_SYS);
}
