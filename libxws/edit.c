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


//*************************************************************//
// Function: xwsWinCreate Edit - Create an edit window         //
// Returned: 0 if normal or a negative XOS error code if error //
//*************************************************************//

long xwsWinCreateEdit(
	XWSWIN    *parent,		// Parent window
	long       xpos,		// X position
	long       ypos,		// Y position
	long       xsize,		// X size
							//    >0  - Width
							//    <=0 - Distance from right edge of
							//            containing window
	long       ysize,		// Y size
							//    >0  - Height
							//    <=0 - Distance from bottom edge of
							//            containing window
	XWSWINPAR *winparms,	// Window parameters
	XWSTXTPAR *txtparms,	// Text parameters
	XWSEVENT  *event,		// Pointer to caller's event function (may be null)

	long       evmask,		// Event mask bits
	XWSEDB    *edb,			// Pointer to caller's environment data block
	XWSWIN   **pwin,
	char      *text,		// Initial text to display
	long       length)		// Length of the initial text string
{
	XWSWIN *win;
	long    rtn;
	long    sbwidth;

	// Create the window

	if (parent != NULL)
	{
		xpos = scalex(parent, xpos);
		ypos = scaley(parent, ypos);
		xsize = scalex(parent, xsize);
		ysize = scaley(parent, ysize);
	}
	if ((rtn = xwscreatewindow(parent, XWS_WINTYPE_EDIT, sizeof(XWSCNTR),
			xpos, ypos, xsize, ysize, winparms, event, evmask, edb, parent->drw,
			&win)) < 0)
		return (rtn);

	if (win->bits & WB2_HORSB)			// Want a horizontal scroll bar?
		xwsaddhorscrollbar(win, winparms, xwslistvsbevent, edb);
	if (win->bits & WB2_VERSB)			// Want a vertical scroll bar?
	{
		xwsaddverscrollbar(win, winparms, xwslisthsbevent, edb);
		sbwidth = win->versb->width;
	}
	else
		sbwidth = 0;

	// Create a variable height text box at the top of the window

///	txtparms->bgdcolor = 0xC0FFC0;

	win->config |= WCB_CARET;
	xwsWinTextNew(win, 1, 1000, 1000, -1000, 1, txtparms, text, length);

///	win->drw->funcdsp->shwcursor();

	if (pwin != NULL)
		*pwin = win;
	return (0);
}


//*****************************************************************//
// Function: xwseventedit - System event function for edit windows //
// Returned: ????                                                  //
//*****************************************************************//

long XOSFNC xwseventedit(
	XWSEDBLK *edblk)
{
	XWSWIN *win;
	TL     *tl;
	STA     cpd;
	long    val;
	long    rtn;
	long    cnt;

	win = edblk->orgwin;
	switch (edblk->type)
	{
	 case XWS_EVENT_MOUSEMV:			// Mouse movement

		if ((edblk->buttons & XWS_EVENT_MBTN_L) &&
				(tl = xwssearchtextline(win, edblk->xposp,
				edblk->yposp)) != NULL)
		{
			xwstextgetcharpos(win, tl, 0x7FFFF, edblk->xposp, &cpd);
			xwstextsetsta(win, &cpd, TRUE);
			win->status |= WSB_PRESSED; // Grab the cursor
			xwscursorgrabwin = win;
		}
		break;

	 case XWS_EVENT_MOUSEBT:			// Mouse button change?
		switch (edblk->subtype)
		{
		 case XWS_EVENT_MBTN_LDN:		// Left mouse button pressed
			if ((tl = xwssearchtextline(win, edblk->xposp, edblk->yposp)) != NULL)
			{
				xwstextgetcharpos(win, tl, 0x7FFFF, edblk->xposp, &cpd);
				if (edblk->kbsts & (XWS_EVENT_KBS_LFTSHF|XWS_EVENT_KBS_RHTSHF))
					xwstextsetsta(win, &cpd, TRUE);
				else
					xwstextsetcaret(win, &cpd, TRUE);
				win->status |= WSB_PRESSED; // Grab the cursor
				xwscursorgrabwin = win;
			}
			break;

		 case XWS_EVENT_MBTN_LUP:		// Left mouse button released
			win->status &= ~WSB_PRESSED;
			if (win == xwscursorgrabwin) // Have we grabbed the cursor?
			{
				xwscursorgrabwin = NULL; // Yes - but not any more
				xwsupdatecursor();
			}
			break;
		}
		break;

	 case XWS_EVENT_KEYMAKE:			// Keyboard key pressed
		if (win->caret.tl == NULL)
			break;
		if (edblk->charcd != 0)			// ASCII key?
		{
			if (edblk->charcd == '\b') // Back-space?
			{							// Yes
				if (win->caret.tl == win->anchor.tl &&
						win->caret.xpos == win->anchor.xpos)
										// Select the previous character if
										//   nothing selected
					xwstextmovecarethoriz(win, -1, TRUE);
				xwstextstr(win, win->caret.tl->tb, "", 0);
			}
			else if (edblk->charcd == ('C' - 0x40) || // ^C or ^X?
					edblk->charcd == ('X' - 0x40))
			{
				// Here if ^C or ^X - Copy the selected text to the clipboard.

				if ((cnt = xwsWinTextGetSel(win, 0, NULL, 0)) > 0)
				{
					if (xwsclipboard != NULL)
						xwsFree(xwsclipboard);
					if ((xwsclipboard = xwsMalloc(sizeof(CLPBRD) + cnt + 1)) ==
							NULL)
						xwsFail(-errno, "Canot allocate memory for clipboard");
					xwsclipboard->size = cnt;
					if ((rtn = xwsWinTextGetSel(win, 0, xwsclipboard->text,
							cnt)) < 0)
						xwsFail(rtn, "Error copying text to clipboard");
				}
				if (edblk->charcd == ('X' - 0x40)) // ^X?
										//Yes - Delete the selected text
					xwstextstr(win, win->caret.tl->tb, "", 0);
			}
			else if (edblk->charcd == ('V' - 0x40)) // ^V?
			{
				if (xwsclipboard != NULL) // Yes - Insert clipboard text if
										  //   is any
					xwstextstr(win, win->caret.tl->tb, xwsclipboard->text,
							xwsclipboard->size);
			}
			else if ((edblk->charcd == '\t' && (win->bits & WB2_RHTTRAV)) &&
					(edblk->kbsts & (XWS_EVENT_KBS_LFTSHF|
					XWS_EVENT_KBS_RHTSHF)))
				xwstravback(win);
			else if ((edblk->charcd == '\r' && (win->bits & WB2_CRTRAV)) ||
					(edblk->charcd == '\t' && (win->bits & WB2_HTTRAV)))
				xwstravahead(win);
			else
			{
				if (edblk->charcd == '\r')
					edblk->charcd = '\n';
				xwstextstr(win, win->caret.tl->tb,
						(char *)&edblk->charcd, 1);
			}
			break;
		}
		switch (edblk->scancd)			// Not ASCII, dispatch on the scan code
		{
		 case ISC_LTARW:				// Left arrow
		 case ISC_KPLTARW:
			val = -1;
			goto horizcom;

		 case ISC_RTARW:				// Right arrow
		 case ISC_KPRTARW:
			val = 1;
		 horizcom:
			xwstextmovecarethoriz(win, val, edblk->kbsts &
					(XWS_EVENT_KBS_LFTSHF|XWS_EVENT_KBS_RHTSHF));
			break;

		 case ISC_UPARW:				// Up arrow
		 case ISC_KPUPARW:
			val = -1;
			goto vertcom;

		 case ISC_DNARW:				// Down arrow
		 case ISC_KPDNARW:
			val = 1;
		 vertcom:
			xwstextmovecaretvert(win, val, edblk->kbsts &
					(XWS_EVENT_KBS_LFTSHF|XWS_EVENT_KBS_RHTSHF));
		 chkscroll:
			xwseditdoscroll(win);
			break;

		 case ISC_HOME:					// Home
		 case ISC_KPHOME:
			val = 0;
			goto homend;

		 case ISC_END:					// End
		 case ISC_KPEND:
			val = 0x7FFF;
		 homend:
			tl = win->caret.tl;
			if (edblk->kbsts & (XWS_EVENT_KBS_LFTCTL|XWS_EVENT_KBS_RHTCTL))
				tl = (val) ? tl->tb->tltail : tl->tb->tlhead;
			xwstextgetcharpos(win, tl, val, val, &cpd);
			if (edblk->kbsts & (XWS_EVENT_KBS_LFTSHF|XWS_EVENT_KBS_RHTSHF))
				xwstextsetsta(win, &cpd, TRUE);
			else
				xwstextsetcaret(win, &cpd, TRUE);
			win->cartarget = val;
			goto chkscroll;

		 case ISC_DELETE:				// Delete
		 case ISC_KPDELETE:
			if (win->caret.tl == win->anchor.tl &&
					win->caret.xpos == win->anchor.xpos)
										// Select the next character if
										//   nothing selected
				xwstextmovecarethoriz(win, 1, TRUE);
			xwstextstr(win, win->caret.tl->tb, "", 0);
			break;
		}
		return (XWS_EVRTN_PASS);
	}
	return (XWS_EVRTN_DONE);
}


//***********************************************************//
// Function: xwseditdoscroll - Do scrolling for edit windows //
// Returned: Nothing                                         //
//***********************************************************//

// This function is called whenever an edit window is modified such that it
//   may need to be scrolled or the size of the scroll bar thumb may need to
//   be changed.

// NOTE: All of the scrolling stuff simply invaliates the entire window to
//       scroll it! This works, but causes massive flicker. It should actually
//       scroll (bitblt) the window whenever it can. This needs to be fixed
//       real soon.

void XOSFNC xwseditdoscroll(
	XWSWIN *win)
{
	XWSWIN *sbwin;
	TB     *tb;
	TL     *tl;
	long    top;
	long    btm;
	long    tpos;
	long    tsize;

	if (win->config & WCB_CARET)
	{
		tl = win->caret.tl;
		tb = tl->tb;
		top = tb->ypos + tl->ypos - win->tbheight;
		btm = top + tl->fmt.f.ysize;

///		if ((tb->ypos + tl->ypos) < (-win->yorg + tb->topmargin +
///				win->tbheight))			// Is caret above the visible part?

		if (win->yorg < -(tb->ypos + tl->ypos - tb->topmargin -
				win->tbheight))
		{

			// Here if the caret is above the visible part of the window.
			//   Put the line containing the caret at the top of the
			//   window. (This is easy!)

			if (win->yorg != -(tb->ypos + tl->ypos - tb->topmargin -
					win->tbheight))
			{
				win->yorg = -(tb->ypos + tl->ypos - tb->topmargin -
						win->tbheight);
				xwsWinInvalidateRgn(win, NULL);
			}
		}
		else if (btm > (-win->yorg + win->clntheight - tb->btmmargin))
		{
			// Here if the caret is below the visible part of the window.
			//   In this case we want to put the line containing the caret at
			//   the bottom of the screen. Since we always want to have a line
			//   start at exactly the top of the screen, we must determine
			//   which line to put at the top to place the caret line as the
			//   last fully visible line.

			// NOTE: This is intended to work with edit windows which have one
			//       text box at the top of the window (the standard case for
			//       edit windows. It will NOT, in general, work with text
			//       boxes not at the top of the window or with a window with
			//       multiple text boxes.

			if ((top = tb->ypos + tl->ypos + tl->fmt.f.ysize -
					(win->clntheight - tb->btmmargin)) < 0)
			{
				if (win->yorg != win->tbheight)
				{
					win->yorg = win->tbheight;
					xwsWinInvalidateRgn(win, NULL);
				}
			}
			else
			{
				// Here if we need to scroll down

				tl = tb->tlhead;
				while (tl->next != NULL && top >= (tb->ypos + tl->ypos -
						tb->topmargin - win->tbheight))
					tl = tl->next;
				if (win->yorg != -(tb->ypos + tl->ypos - tb->topmargin -
						win->tbheight))
				{
					win->yorg = -(tb->ypos + tl->ypos - tb->topmargin -
							win->tbheight);
					xwsWinInvalidateRgn(win, NULL);
				}
			}
		}
	}

	// Now determine the current size and position for the thumb. We must
	//   determine the bottom most bottom of a text box.

	btm = 0;
	tb = win->tbhead;
	while (tb != NULL)
	{
		if (btm < (tb->ypos + tb->ysize))
			btm = tb->ypos + tb->ysize;
		tb = tb->next;
	}
	if ((sbwin = win->versb) != NULL)
	{
		if (btm > win->clntheight)
		{
			tsize = (win->clntheight * sbwin->tmbrange) / btm;
			tpos = ((win->tbheight - win->yorg) * sbwin->tmbrange) / btm;
		}
		else
		{
			tsize = sbwin->tmbrange;
			tpos = 0;
		}
		xwsWinScrollBarSet(sbwin, -1, tpos, tsize);
	}
}
