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

// Text is managed in container, button, edit, response, and list windows
//   using text boxes. A text box contains one or more lines of text. It is
//   identified by an index value which is unique withing the window. This
//   value also specifies the order in which test is drawn with lower numbered
//   text boxes drawn first. In display windows and buttons, text boxes can
//   be located anywhere in the window and can overlap if desired. Edit windows
//   are created containing a single text box which covers the entire window.
//   This can be replaced if desired. In list windowes, text boxes are
//   "stacked" vertically and cannot overlap. The  Y position specifies the
//   distance below the previous text box. Text boxes are ordered vertically
//   based on the index value. Text is clipped to the boundry of the text box.
//   If the height is 0 or negative, the height values specifies the distance
//   between the bottom of the text box and the bottom of the window. If the
//   height is 1 the height used is the value required to include all lines.

// A text line is said to be complete if it is terminated with a LF character.
//   If word-wrap is specified lines can also be incomplete which means that
//   the are terminated when a word would extend past the right margin.
//   If word-wrap is not specified only the last line in a text box can be
//   incomplete.

// There is always an incomplete line at the end of a text box. This is a line
//   that is not terminated with a LF. If the last line is complete, an empty
//   incomplete line is added. This is done so we can always add text to a
//   line without needing to reformat any previous lines. If the last line
//   was complete, we would have to reformat it to obtain the starting format
//   values for a line that is added. Note that when adding to an incomplete
//   last line, that line is reformated from the beginning of the line. Since
//   a text box is created containing an emtpy incomplete line, the actual
//   implementation of this is that a check is made for a line being added
//   being incomplete and the next line is deleted if it is zero length and
//   incomplete and the last line. Only in the case where a LF or VT is added
//   at the end of the last line is it necessary to actually add an extra empty
//   line.

// There are a number of colors associated with a text box as follows:

// Background color (bgcolor)
//   Used to fill the entire text box before anything is drawn.

// Normal text color (nrmcolor)
//   Used to draw text characters when the window is in the normal state and
//   no color is specified in the text string.

// Alternate text color (altcolor)
//   Used to draw text characters when the window is in the alternate or
//   pressed state and no color is specified in the text string.

// Highlight color (hlcolor)
//   Used to draw highlighted text. Also used to draw text in the selected item
//   for list-type windows. Note that colors specified in the text are ignored
//   for selected text.

// Background highlight color (bghlcolor)
//   Used to draw the background for highlighted text and as the backgound
//   color for the selected item in list-type windows.

// Caret color (caretcolor)
//   All windoes: Used to draw the caret.

// Note that there is no alternate background color. Since this color is
//   usually set to "transparent" (especially for buttons), it would hardly
//   ever be  useful.

// These colors apply to all lines in a text box. Local changes in the normal
//   text and background colors can be made using the text formatting commands.
//   Highlight and alternate colors cannot be changed locally.

TB *xxxtb;

//***********************************************
// Function: xwsWinTextRemove - Remove a text box
// Returned: Nothing
//***********************************************

void XOSFNC xwsWinTextRemove(
	XWSWIN *win,
	long    inx)
{
	TB *tb;

	if (inx == -1)						// Want to give up all text boxes?
	{
		while ((tb = win->tbhead) != NULL)
		{
			win->tbhead = tb->next;
			xwstextgivetb(win, tb);
		}
		win->tbtail = NULL;
	}
	else
		xwsWinTextNew(win, inx, 0, 0, 0, 0, 0, 0, -1);

/// NEED TO DO FIXUP ON LIST WINDOWS HERE!!!!!

}


//************************************************************//
// Function: xwsWinTextNew - Store a text string in a window, //
//             starting a new text block                      //
// Returned: The height of the text box unless TBA_NOOVER is  //
//             set, then number of characters accepted        //
//************************************************************//

long XOSFNC xwsWinTextNew(
	XWSWIN *win,
	long    inx,
	long    xpos,
	long    ypos,
	long    xsize,		// X size
						//    >0  - Width
						//    <=0 - Distance from right edge of containing
						//            window to right edge of text box
	long    ysize,		// Y size
						//   If not list type window:
						//     >1  - Height
						//     =1  - Variable as needed to hold the text
						//     <=0 - Distance from bottom edge of containing
						//             window to bottom of text box
						//   If list type window:
						//     Distance from preceeding TB
	XWSTXTPAR *parm,	// Text parameters
	char      *text,	// Text
	long       count)	// Length of the text (including all formating bytes)
{
	TB     *prevtb;
	TB     *nexttb;
	TB     *tb;
	TL     *tl;
	XWSDRW *drw;
	XWSGETCOLOR
	       *getcolor;
	long    rtn;

	if ((tb = xwsfindtextboxbyinx(win, inx, &prevtb)) != NULL)
	{									// Find this text box if it exists
		// Found it - unlink this block

		nexttb = tb->next;
		if (prevtb == NULL)
			win->tbhead = nexttb;
		else
			prevtb->next = nexttb;
		if (nexttb == NULL)
			win->tbtail = prevtb;
		else
			nexttb->prev = prevtb;

/// NEED TO DO FIXUP ON LIST WINDOWS HERE!!!!!

		xwstextgivetb(win, tb);			// Give up the TB (This also invalidates
	}									//   the area covered by the text box.)		
	if (count < 0)						// Have anything to store?
		return (0);						// No - finished here



	if (text == NULL)					// Null string?
	{
		count = 0;
		text = "";
	}
	else if (count == 0)				// Did he specify a length
		count = strlen(text);			// No - he wants us to calculate the
										//   length

	// Here with something to store (which may be 0 length)

	if ((tb = (TB *)xwsCMalloc(sizeof(TB))) == NULL)
		return (-errno);

	xxxtb = tb;

	tb->label = 'TB##';					// Initialize the TB

	if ((tl = (TL *)xwsCMalloc(sizeof(TL))) == NULL) // Get an empty TL
	{
		xwsFree(tb);
		return (-errno);
	}
	tl->label = 'TL##';					// Initialize the TL
	tb->inx = inx;
	tb->tlhead = tb->tltail = tl;
	tl->tb = tb;
	tl->next = tl->prev = NULL;
	tb->tbattr = parm->tbattr & ~TBA_HEIGHT;

	if (ysize == 1)
		ysize = 0;
	else
		tb->tbattr |= TBA_HEIGHT;
	if (!xwsnoscale)
	{
		xpos = SCALE(win->xsfr, win->xsfd, xpos);
		ypos = SCALE(win->ysfr, win->ysfd, ypos);
		xsize = SCALE(win->xsfr, win->xsfd, xsize);
		ysize = SCALE(win->ysfr, win->ysfd, ysize);
	}
	if (xpos < 0)
		xpos += win->clntwidth;
	if (ypos < 0)
		ypos += win->clntheight;
	if (xsize<= 0)
		xsize = xsize + win->clntwidth - xpos;
	if (ysize <= 0)
		ysize = ysize + win->clntheight - ypos;
	tb->xpos = xpos;
	tb->yinc = ypos;
	tb->ypos = ((win->config & WCB_LIST) == 0 || prevtb == NULL) ? ypos :
			(ypos + prevtb->ypos + prevtb->ysize);
	tb->xsize = xsize;
	drw = win->drw;

	tb->topmargin = SCALE(win->ysfr, win->ysfd, parm->topmargin);
	tb->btmmargin = SCALE(win->ysfr, win->ysfd, parm->btmmargin);

	getcolor = drw->funcdsp->getcolor;
	tb->bgdcolor = getcolor(parm->bgdcolor);
	tb->bgdcolorsel = getcolor(parm->bgdcolorsel);
	tb->txtcolorsel = getcolor(parm->txtcolorsel);
	tb->bgdcolorfcs = getcolor(parm->bgdcolorfcs);
	tb->txtcolorfcs = getcolor(parm->txtcolorfcs);
	tb->caretcol = getcolor(parm->caretcolor);

	tl->fmt.f.leftmargin = SCALE(win->xsfr, win->xsfd, parm->leftmargin);
	if ((tl->fmt.f.rightmargin = xsize - SCALE(win->xsfr, win->xsfd,
			parm->rightmargin)) <= 0)
		tl->fmt.f.rightmargin += xsize;

	tl->fmt.i.font.num = parm->fnum;
	tl->fmt.i.font.height = SCALE(win->ysfr, win->ysfd, parm->fheight);
	tl->fmt.i.font.width = SCALE(win->xsfr, win->ysfd, parm->fwidth);;
	tl->fmt.i.font.attr = parm->fattr;
	tl->fmt.i.font.weight = parm->fweight;

	if (xwssetupfont(win, &tl->fmt.i.font))
		tl->fmt.i.font = xwsfntval;

	tl->fmt.f.leading = 0;
	tl->fmt.f.ascent = xwscurfnt->ascent + xwscurfnt->leading;
	tl->fmt.f.descent = xwscurfnt->descent;
	tl->fmt.f.lspace = SCALE(win->xsfr, win->xsfd, parm->lspace);
	if ((tl->fmt.f.lfmt = parm->lfmt) > 3)
		tl->fmt.f.lfmt = 0;
	tl->fmt.i.txtcolor = getcolor(parm->txtcolor);
	tl->fmt.i.bgnbgc.length = 0;
	tl->fmt.i.bgnbgc.xpos = 0;
	tl->fmt.i.bgnbgc.color = XWS_COLOR_TRANS;	
	tl->xpos = tl->fmt.f.leftmargin;
	tl->ypos = tb->topmargin;
	tl->fmt.f.ysize = tl->fmt.f.leading + tl->fmt.f.ascent + tl->fmt.f.descent +
			tl->fmt.f.lspace;
	tl->fmt.f.length = 0;
	tl->lnum = 1;
	tl->count = 0;

	// Put the caret at the beginning of the only line

	win->caret.tl = win->anchor.tl = tl;
	win->caret.lnum = win->anchor.lnum = 1;
	win->caret.cinx = win->anchor.cinx = 0;
	win->caret.cnum = win->anchor.cnum = 0;
	win->caret.xpos = win->anchor.xpos = tl->fmt.f.leftmargin;
///	if (win->config & WCB_CARET)
///		xwstextinvalidatecaret(win);

	// Set height

	tb->ysize = (tb->tbattr & TBA_HEIGHT) ? ysize : (tb->topmargin +
				tl->fmt.f.leading + tl->fmt.f.ascent + tl->fmt.f.descent +
				tl->fmt.f.lspace + tb->btmmargin);

	// Invalidate the TB's area if the background is not transparent

	if (tb->bgdcolor != XWS_COLOR_TRANS)
		xwswininvalidaterectns(win, tb->xpos, tb->ypos, tb->xsize, tb->ysize);

	if ((tb->prev = prevtb) == NULL)	// Link the new TB into the list for
	{									//   the window
		nexttb = win->tbhead;
		win->tbhead = tb;
	}
	else
	{
		nexttb = prevtb->next;
		prevtb->next = tb;
	}
	if ((tb->next = nexttb) == NULL)
		win->tbtail = tb;
	else
		nexttb->prev = tb;

	// Clear TBA_NOOVER if it is not valid

	if (((tb->tbattr ^ TBA_HEIGHT) & (TBA_HEIGHT|TBA_VERCEN)) ||
			win->type == XWS_WINTYPE_LIST)
		tb->tbattr &= ~TBA_NOOVER;

	// Store the text

	if (count > 0)
		rtn = xwstextstr(win, tb, text, count);
	if ((tb->tbattr & TBA_NOOVER) == 0)
		rtn = (xwsnoscale) ? tb->ysize : (tb->ysize * win->ysfd);
	else
		tb->tbattr &= ~TBA_NOOVER;			// TBA_NOOVER cannot stay set in
											//   case more text is added!
	if (win->type == XWS_WINTYPE_EDIT)		// Edit window?
		xwseditdoscroll(win);				// Yes - scroll if necessary
	parm->right = (tb->xpos + tb->xsize - 1) * win->xsfd;
	parm->bottom = (tb->ypos + tb->ysize - 1) * win->ysfd;
	return (rtn);
}


//**************************************************************
// Function: xwsWinTextAdd - Display text in a window, adding to
//                           the end of an existing text block.
// Returned: ???
//**************************************************************

long XOSFNC xwsWinTextAdd(
	XWSWIN *win,
	long    inx,
	char   *text,
	long    count)
{
	return (ER_NIYT);

#if 0
	PUSHL	EDI
	PUSHL	ESI
	PUSHL	EBP
	MOVL	EBP, ESP
	PUSHL	sta_win[EBP]
	PUSHL	sta_inx[EBP]
	PUSHL	#0
	CALL	xwsfindtextboxbyinx
	TESTL	EAX, EAX
	JE	4$			;Fail if not found

;;;;	MOVL	ft_tl+tl_tb[EBP], EAX


	PUSHL	sta_win[EBP]
	PUSHL	EAX
	MOVL	EAX, tl_tb[EAX]
	PUSHL	tb_width[EAX]
	PUSHL	tb_height[EAX]
	PUSHL	#9999
	PUSHL	sta_text[EBP]
	PUSHL	sta_count[EBP]
	CALL	xwstextstr#
	TESTL	EAX, EAX
	JS	2$
	CLRL	EAX
2$:	MOVL	ESP, EBP
	POPL	EBP
	POPL	ESI
	POPL	EDI
	RET	16t

;Here if did not find the specified text block

4$:	MOVL	EAX, #ER_NTDEF
	JMP	2$
#endif
}


//*******************************************************//
// Function: xwsWinTextIns - Display text in a text box, //
//             replacing the current  selected text area //
// Returned: ???                                         //
//*******************************************************//

long XOSFNC xwsWinTextInsert(
	XWSWIN *win,
	char   *text,
	long    count)
{
	if (win->caret.tl != NULL)
		xwstextstr(win, win->caret.tl->tb, text, count);
	return (0);
}


//**************************************************************
// Function: xwsWinTextGet - Get the contents of a text box
// Returned: Value returned is the length of the string returned
//**************************************************************

// All lines in the text box are returned in a single buffer with LF inserted
//   as needed after complete lines.

// If the value returned is equal to or larger than the length argument, the
//   string has been truncated to the length of the buffer. If it is less
//   than the buffer length the entire string has been stored with a final
//   0.

long xwsWinTextGet(
	XWSWIN  *win,		// Address of the WIN
	long     inx,		// Text box index (0 means to use lowest
						//   defined index)
	long     mode,		// Mode:
						//   Bit 0: Set = Remove all formatting.
						//   Bit 1: Set = Indicate word wrap breaks by
						//                inserting ^A.
						//          Clr = Insert space for word wrap breaks
						//   Bit 2: Set = Insert LF for word wrap breaks
						//          Clr = See bit 1
	char    *bufr,		// Address of buffer to receive text. Can be NULL
						//   in which case only the length is returned.
	long     len)		// Length of the buffer. 0 indicates no buffer.
{
	TB   *tb;
	TL   *tl;
	char *pnt;
	long  total;
	long  cnt;
	int   func;
	char  chr;

	if ((tb = (inx == 0) ? win->tbhead : xwsfindtextboxbyinx(win, inx, NULL))
			== NULL)
		return (ER_NTDEF);
	total = 0;
	if (bufr == NULL)
		len = 0;
	tl = tb->tlhead;
	do
	{
		pnt = tl->text;
		cnt = tl->count;
		while (--cnt >= 0)
		{
			chr = *pnt++;
			if (chr == TF_PREFIX && (mode & 0x01))
			{
				if (--cnt < 0)
					break;
				func = *pnt++;
				if (func < TF_1BYTE)
				{
				}
				else if (func < TF_2BYTE)
				{
					if (--cnt < 0)
						break;
				}
				else if (func < TF_4BYTE)
				{
					if ((cnt -= 4) < 0)
						break;
				}
				else
				{
					if ((cnt -= 14) < 0)
						break;
				}
				continue;
			}
			total++;
			if (len > 0)
			{
				*bufr++ = chr;
				len--;
			}
		}
		if (pnt > tl->text && pnt[-1] != '\n' && tl->next != NULL)
		{
			total++;
			if (len > 0)
			{
				*bufr++ = ((mode & 0x06) == 0) ? ' ' : (mode & 0x02) ?
						0x01 : '\n';
				len--;
			}
		}
	} while ((tl = tl->next) != NULL);
	if (len > 0)
		*bufr = 0;
	return (total);
}


//********************************************************************//
// Function: xwsWinTextGetSel - Get the selected text from a text box //
// Returned: Value returned is the length of the string returned      //
//********************************************************************//

// All selected text in the text box are returned in a single buffer with LF
//   inserted as needed after complete lines.

// If the value returned is equal to or larger than the length argument, the
//   string has been truncated to the length of the buffer. If it is less
//   than the buffer length the entire string has been stored with a final
//   0.

long XOSFNC xwsWinTextGetSel(
	XWSWIN  *win,		// Address of the WIN
	long     mode,		// Mode:
						//   Bit 0: Set = Remove all formatting.
						//   Bit 1: Set = Indicate word wrap breaks by
						//                inserting ^A.
						//          Clr = Insert space for word wrap breaks
						//   Bit 2: Set = Insert LF for word wrap breaks
						//          Clr = See bit 1
	char    *bufr,		// Address of buffer to receive text. Can be NULL
						//   in which case only the length is returned.
	long     len)		// Length of the buffer. 0 indicates no buffer.
{
	TL   *tl;
	STA  *stabgn;
	STA  *staend;
	char *pnt;
	long  total;
	long  cnt;
	int   func;
	char  chr;

	if (win->caret.tl == NULL)
		return (ER_NTDEF);
	total = 0;
	if (bufr == NULL)
		len = 0;

	// If nothing selected, return 0 length text

	if (win->caret.tl == win->anchor.tl && win->caret.cinx == win->anchor.cinx)
	{
		if (len > 0)
			*bufr = 0;
		return (0);
	}

	// Get the beginning and end of the selected area

	if (win->caret.lnum > win->anchor.lnum ||
			(win->caret.lnum == win->anchor.lnum &&
			win->caret.cinx > win->anchor.cinx))
	{
		stabgn = &win->anchor;
		staend = &win->caret;
	}
	else
	{
		stabgn = &win->caret;
		staend = &win->anchor;
	}
	tl = stabgn->tl;
	cnt = ((tl == staend->tl) ? staend->cinx : tl->count) - stabgn->cinx;
	pnt = tl->text + stabgn->cinx;
	while (TRUE)
	{
		while (--cnt >= 0)
		{
			chr = *pnt++;
			if (chr == TF_PREFIX && (mode & 0x01))
			{
				if (--cnt < 0)
					break;
				func = *pnt++;
				if (func < TF_1BYTE)
				{
				}
				else if (func < TF_2BYTE)
				{
					if (--cnt < 0)
						break;
				}
				else if (func < TF_4BYTE)
				{
					if ((cnt -= 4) < 0)
						break;
				}
				else
				{
					if ((cnt -= 14) < 0)
						break;
				}
				continue;
			}
			total++;
			if (len > 0)
			{
				*bufr++ = chr;
				len--;
			}
		}

		if (tl == staend->tl)
			break;
		if (pnt > tl->text && pnt[-1] != '\n' && tl->next != NULL)
		{
			total++;
			if (len > 0)
			{
				*bufr++ = ((mode & 0x06) == 0) ? ' ' : (mode & 0x02) ?
						0x01 : '\n';
				len--;
			}
		}
		if ((tl = tl->next) == staend->tl && (cnt = staend->cinx) <= 0)
			break;
		pnt = tl->text;
	}
	if (len > 0)
		*pnt = 0;
	return (total);
}


//**************************************************************
// Function: xwsWinTextGetBBox - Get bounding box for a text box
// Returned: 0 if normal or a negative XOS error code if error
//**************************************************************

// Values are returned in bbox.

long XOSFNC xwsWinTextGetBBox(
	XWSWIN  *win,
	long     inx,
	XWSRECT *bbox)
{
	TB *tb;

	if ((tb = xwsfindtextboxbyinx(win, inx, 0)) == NULL)
		return (ER_NTDEF);
	bbox->xpos = tb->xpos * win->xsfd;
	bbox->ypos = tb->ypos * win->ysfd;
	bbox->xsize = tb->xsize * win->xsfd;
	bbox->ysize = tb->ysize * win->ysfd;
	return (0);
}


//*************************************************
// Function: Find a text box given the index value
// Returned: Address of the TB or NULL if not found
//*************************************************

// ptb receives the address of the previous TB if the requested TB was found.
//   If the requested TB is the first, this is NULL. If the requested TB is
//   not found it is the address of the TB before where the requested TB would
//   be inserted or NULL if it would be the first TB.

TB* XOSFNC xwsfindtextboxbyinx(
	XWSWIN *win,
	long    inx,
	TB    **ptb)
{
	TB *tb;
	TB *prev;

	tb = win->tbhead;					// Get first TB
	prev = NULL;
	while (tb != NULL && inx >= tb->inx)
	{
		if (inx == tb->inx)				// This one?
		{
			if (ptb != NULL)
				*ptb = prev;
			return (tb);
		}
		prev = tb;
		tb = tb->next;
	}
	if (ptb != NULL)
		*ptb = prev;
	return (0);
}


//******************************************************************
// Function: xwssearchtextboxlist - Find a text box given Y position
//                                  in a list type window
// Returned: Address of the TB or NULL if not found
//******************************************************************

// A hit is returned if the Y position specified is anywhere within the
//   text box. We take advantage of the fact that text boxes in list windows
//   are ordered by their y position to spped this up somewhat. Also, we
//   search backwards since the only way we can not get a hit is if the
//   positon is below the last text box.

TB* XOSFNC xwssearchtextboxlist(
	XWSWIN *win,
	long    ypos)
{
	TB *tb;

/// NEED TO SCALE HERE???

	tb = win->tbtail;					// Get last text box
	while (tb != NULL && ypos < (tb->ypos + tb->ysize))
	{
		if (ypos >= tb->ypos)
			return (tb);
		tb = tb->prev;
	}
	return (NULL);
}


//*************************************************************
// Function: xwssearchtextline - Find a text line given X and Y
//                               positions on screen
// Returned: Address of the TB if found or NULL if not found
//*************************************************************

// The TB list is searched in reverse order so if two TLs overlap on the
//   display, the one that is visible will be returned. When checking to see
//   if a position is in a line, the line is assumed to extend to both sides
//   of the text box. The first line is assumed to begin at the top edge of
//   the text box and the last line is assumed to extend to the bottom of the
//   text box. Thus if a position is within the bounding rectangle of a text
//   box, it will always be reported as being "in" a line in that text box.
//   This can produce unexpected results if fixed height text boxes overlap.

TL* XOSFNC xwssearchtextline(
	XWSWIN *win,		// Address of WIN
	long    xpos,		// X position in window
	long    ypos)		// Y position in window
{
	TB *tb;
	TL *tl;
	TL *nxttl;

	if ((tb = win->tbtail) == NULL)
		return (NULL);
	do
	{
		if (xpos < tb->xpos || ypos < tb->ypos || xpos > (tb->xpos + tb->xsize)
				|| ypos > (tb->ypos + tb->ysize)) // Within this TB?
			continue;					// No

		ypos -= tb->ypos;

		tl = tb->tlhead;
		while (TRUE)
		{
			if ((nxttl = tl->next) == NULL || ypos < nxttl->ypos)
				return (tl);

			tl = nxttl;
		}
		break;
	} while (tb = tb->prev);
	return (NULL);
}


//***********************************************************
// Function: xwstextgivetb - Give up a text block (TB) object
//                           and all of its text lines (TLs)
// Returned: Address of next TB in the window or NULL if this
//           was the last
//***********************************************************

// Caller must have already unlinked the TB from the XWSWIN

TB *xwstextgivetb(
	XWSWIN *win,
	TB     *tb)
{
	TL *tl;
	TL *nexttl;
	TB *nexttb;

	tl = tb->tlhead;
	while (tl != NULL)
	{
		nexttl = tl->next;
		xwsFree(tl);
		tl = nexttl;
	}

	// Here with no more TLs to give up - give up the TB

	if (win->status & WSB_DRAWN)		// Has the window been drawn?
										// Yes - invalidate the text box's
										//   rectangle
		xwswininvalidaterectns(win, tb->xpos, tb->ypos, tb->xsize, tb->ysize);
	nexttb = tb->next;
	xwsFree(tb);
	return  (nexttb);
}

