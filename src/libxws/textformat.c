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

XWSFNT *xwscurfnt;
FNTVAL  xwsfntval = {0xFF};

// The FMT structure contains all information needed to specify the formating
//   state at a partictular point in a line. There is a copy of this structure
//   in each TL which specifies the formating state at the beginning of the
//   line. This allows display to start at any line withou having to process
//   any previous lines. There is also a TXTFMT copy in each TB which specifies
//   the formating state at the end of the last line in the TL. This allows
//   characters added to the end of the last line (which is the most common
//   case when doing input to an edit window) to be displayed without
//   reformating the entire line.

// The FD structure contains all state information used when formatting a
//   text line. This includes an FMT structure and all pointers and counts
//   used. A copy of this structure is saved at each word-wrap break point
//   so that the state can be easily restored when it is necessary to back
//   up to start a new line. It also provides a convenient way to pass the
//   state to xwsformattext.


static void XOSFNC dobgcolor(XWSWIN *win, FD *fd, long value);
static void XOSFNC setupfont(XWSWIN *win, FD *fd);


//******************************************************//
// Function: xwsformattext - Format text                //
// Returned: TRUE if need to emit line, FALSE otherwise //
//******************************************************//

// Text taken from the input buffer is copied to the text buffer in FD.
//   and formated. If a complete line if built (terminated by LF or word-
//   wrapped) TRUE is returned. We the input string is exausted without
//   building a complete line FALSE is returned.

//   On a TRUE return the caller will call xwsfinishtext to store the
//   new line into a TL (unless this is being called to verify a backwards
//   format requirement and there is no change).

//   After a TRUE return the tcnt value in FD should be checked. If this is
//   non-0, xwsformattext should be called again. This must be repeated
//   until either TRUE is returned or tcnt is 0.

//   It is possible for word-wrap residue may be left in the buffer after
//   processing the last line in a TB (wpnt does not point to text). In
//   this case xwsfinishtext can be called to store the final line even
//   though xwsformattext did not return TRUE.

//   Formating functions must be complete in a single input string. Any
//   partial formating functions encountered at the end of an input string
//   are discared.

// This function can be called repeatedly for the same data without harm.
//   (It is extra overhead!)

// Each TL contains two structures (FMTF and FMTI) which specify the initial
//   formatting state for the line. The values in FMTF apply to the entire
//   line and consist of vertical spacing and horizontal margin values. These
//   values are either the maximum of all values for line (vertical spacing
//   values) or are the last value specified for the line (horizontal margin
//   values). The values in FMTI apply to the start of the line and are
//   modified as formatting functions are processed.

// The FDF structure contains all values that must apply at the end of a line.
//   This includes the FMTF structure and all pointers and counts used to
//   fetch and store characters. A copy of this structure is saved at each
//   possible word-wrap point and is used to restore it when the line
//   overflows the available space and it is necessary to back up to the
//   previous word-wrap point. Since this structure contains the complete
//   formatting state, restoring it is all that is needed to back up.

// NOTE: A new right margin generally should be specified at or near the
//       beginning of a line. If the right margin value is being reduced and
//       the text position when it is specified is between the new and old
//       right margin value, the line will be wraped at that point, resulting
//       in an unexpected right margin value for that single line.

long XOSFNC xwsformattext(
	XWSWIN *win,
	TB     *tb,
	FD     *fd)
{
///    XWSGCB *gcb;
	long    value;
	long    bgnnum;
	long    chr;
	long    width;
	int     func;
	int     size;

	bgnnum = fd->tcnt;

///	if (fd->i.fmti.bgnbgc.length != 0)
///	{
///		fd->f.curbgc = &fd->i.fmti.bgnbgc;
///		fd->i.fmti.bgnbgc.xpos = 0;
///	}
///	else
///		fd->f.curbgc = NULL;

	while (fd->tcnt > 0)
	{
		// See if there is space left in the working buffer. We consider the
		//   buffer full if there are less than 18 bytes left so we will
		//   always have room to expand formating function when we need to.

		if (fd->f.wpnt > (fd->text + LINEMAX - 18))
		{
			// Here if the working buffer is full! This should not happen
			//   since the buffer is much larger than should be necessary.
			//   If it does happen, we simply terminate the line at this
			//   point by inserting a LF.

			*fd->f.wpnt++ = '\n';
			return (TRUE);
		}
		if ((chr = fd->tpnt[0]) == TF_PREFIX)
		{
			// Here if have a function prefix

			if (fd->tcnt < 2)			// Have another character?
			{
				fd->tcnt = 0;			// No - discard the partial function
				break;
			}
			fd->f.wpnt[0] = TF_PREFIX;
			fd->f.wpnt[1] = func = fd->tpnt[1];
			if (func < TF_1BYTE)		// Have no added bytes?
			{
				switch (func)
				{
				 case TF_SELITEM:		// Select list box item
					fd->f.selitem = TRUE;
					goto com0byte;

				 case TF_CLRCOFS:		// Clear character offset
					fd->f.fmt.i.offset = 0;
				 com0byte:
					fd->f.wpnt += 2;
					break;

				 case TF_CLRCBGC:		// Clear background color?
					fd->f.wpnt[1] = TFX_BGCOL;
					dobgcolor(win, fd, XWS_COLOR_TRANS);
					break;

				 case TF_CARET:			// Place caret here
					fd->f.caret.lnum = fd->f.anchor.lnum = 1;
					fd->f.caret.cinx = fd->f.anchor.cinx = (fd->f.wpnt -
							fd->text);
					fd->f.caret.cnum = fd->f.anchor.cnum = fd->f.numchar;
					fd->f.caret.xpos = fd->f.anchor.xpos =
							fd->f.fmt.f.leftmargin + fd->f.fmt.f.length;
					break;

				 case TF_BGNSELBC:		// Begin highlight area with beginning
										//   caret
					fd->f.caret.lnum = 1;
					fd->f.caret.cinx = fd->f.wpnt - fd->text;
					fd->f.caret.cnum = fd->f.numchar;
					fd->f.caret.xpos = fd->f.fmt.f.leftmargin +
							fd->f.fmt.f.length;
					break;

				 case TF_BGNSELEC:		// Begin highlight area with ending
										//   caret
					fd->f.anchor.lnum = 1;
					fd->f.anchor.cinx = fd->f.wpnt - fd->text;
					fd->f.anchor.cnum = fd->f.numchar;
					fd->f.anchor.xpos = fd->f.fmt.f.leftmargin +
							fd->f.fmt.f.length;
					break;

				 case TF_ENDSEL:		// End highlight area
					if (fd->f.anchor.lnum != 0)
					{
						fd->f.caret.lnum = 1;
						fd->f.caret.cinx = fd->f.wpnt - fd->text;
						fd->f.caret.cnum = fd->f.numchar;
						fd->f.caret.xpos = fd->f.fmt.f.leftmargin +
								fd->f.fmt.f.length;
					}
					else if (fd->f.caret.lnum != 0)
					{
						fd->f.anchor.lnum = 1;
						fd->f.anchor.cinx = fd->f.wpnt - fd->text;
						fd->f.anchor.cnum = fd->f.numchar;
						fd->f.anchor.xpos = fd->f.fmt.f.leftmargin +
								fd->f.fmt.f.length;
					}
					break;
				}
				fd->tpnt += 2;
				fd->tcnt -= 2;
				fd->total += 2;
			}
			else if (func < TF_4BYTE)	// Have one added byte?
			{
				if (fd->tcnt < 3)		// Have enought characters?
				{
					fd->tcnt = 0;		// No - discard the partial function
					break;
				}
				value = fd->tpnt[2];
				switch (func)
				{
				 case TF_BGCOLX:
					fd->f.wpnt[1] = TFX_BGCOL;
					dobgcolor(win, fd, win->drw->funcdsp->getcolor(
							(((ulong)value) < win->edb->numcols) ?
							win->edb->coltbl[value] : 0));
					break;

				 case TF_FGCOLX:
					fd->f.wpnt[1] = TFX_FGCOL;
					fd->f.fmt.i.txtcolor = *(long *)(fd->f.wpnt + 2) =
							win->drw->funcdsp->getcolor((((ulong)value) <
							win->edb->numcols) ? win->edb->coltbl[value] : 0);
					fd->f.wpnt += 6;
					break;

				 case TF_FONT:			// Set character font?
					if (value < 1 || value > win->edb->numfonts)
						value = 1;
					fd->f.reqfnt.num = value;
					goto com1byte;

				 case TF_SETFATTR:		// Set font attributes
					fd->f.reqfnt.attr |= value;
					goto com1byte;

				 case TF_CLRFATTR:		// Clear font attributes
					fd->f.reqfnt.attr &= ~value;
					goto com1byte;

				 case TF_FWEIGHT:		// Set font weight
					fd->f.reqfnt.weight = value;
					goto com1byte;

				 case TF_LINEFMT:		// 0x20 Set line format
										//   0x20 = Hard left
										//   0x21 = Hard right
										//   0x22 = Text box centered
										//   0x23 = Margin centered
										//   0x24 = Justified (Not currently
					if (value > 3)		//          implemented, treated as
						value = 0;		//          hard left.)
					fd->f.fmt.f.lfmt = (char)value;
				 com1byte:
					fd->f.wpnt[2] = value;
					fd->f.wpnt += 3;
					break;
				}
				fd->tpnt += 3;
				fd->tcnt -= 3;
				fd->total += 3;
			}
			else if (func < TF_4BYTE)	// If have 2 additional characters
			{
				if (fd->tcnt < 4)		// Have enought characters?

				{
					fd->tcnt = 0;		// No - discard the partial function
					break;
				}
				value = *(ushort *)(fd->tpnt + 2);
				switch (func)
				{
			 	 case TF_FHEIGHTP:		// Set character height
					fd->f.reqfnt.height = value;
					goto com2byte;

				 case TF_HORPOSP:		// Set horizontal position
					fd->f.fmt.f.length = value;
					goto com4byte;

				 case TF_FWIDTHP:		// Set character width
					fd->f.reqfnt.width = value;
					goto com2byte;

				 case TF_LINESPP:		// Set line spacing
					break;

				 case TF_LEFTMARP:		// Set left margin
					fd->f.fmt.f.leftmargin = value;
					goto com2byte;

				 case TF_RIGHTMARP:		// Set right margin
					fd->f.fmt.f.rightmargin = value;
					goto com2byte;

				 case TF_XSPACEP:		// Insert extra space?
					break;

				 case TF_SETCOFSP:		// Set character offset
					fd->f.fmt.i.offset = value;
				 com2byte:
					*(ushort *)(fd->f.wpnt + 2) = value;
					fd->f.wpnt += 4;
					break;
				}
				fd->tpnt += 4;
				fd->tcnt -= 4;
				fd->total += 4;
			}
			else if (func < TFX_XBYTE)	// If have 4 additional characters
			{
				if (fd->tcnt < 6)		// Have enought characters?
				{
					fd->tcnt = 0;		// No - discard the partial function
					break;
				}
				value = *(long *)(fd->tpnt + 2);
				switch (func)
				{
			 	 case TF_FHEIGHT:		// Set character height
					value = SCALE(win->ysfr, win->ysfd, value);
					fd->f.wpnt[1] = TF_FHEIGHTP;
					fd->f.reqfnt.height = value;
					goto com4byte2;

				 case TF_FWIDTH:		// Set character width?
					value = SCALE(win->xsfr, win->xsfd, value);
					fd->f.wpnt[1] = TF_FWIDTHP;
					fd->f.reqfnt.width = value;
					goto com4byte2;

				 case TF_HORPOS:		// Set horizontal position
					value = SCALE(win->xsfr, win->xsfd, value);
					fd->f.wpnt[1] = TF_HORPOSP;
					fd->f.fmt.f.length = value;
					goto com4byte2;

				 case TF_LEFTMAR:		// Set left margin?
					value = SCALE(win->xsfr, win->xsfd, value);
					fd->f.wpnt[1] = TF_LEFTMARP;
					fd->f.fmt.f.leftmargin = value;
					goto com4byte2;

				 case TF_RIGHTMAR:		// Set right margin?
					value = SCALE(win->xsfr, win->xsfd, value);
					fd->f.wpnt[1] = TF_RIGHTMARP;
					fd->f.fmt.f.rightmargin = value;
					goto com4byte2;

				 case TF_SETCOFS:		// Set character offset?
					value = SCALE(win->ysfr, win->ysfd, value);
					fd->f.wpnt[1] = TF_SETCOFSP;
					fd->f.fmt.i.offset = value;
				 com4byte2:
					*(ushort *)(fd->f.wpnt + 2) = value;
					fd->f.wpnt += 4;
					break;

				 case TF_LINESP:		// Set line spacing?
					break;

				 case TF_XSPACE:		// Insert extra space?
					break;

				 case TF_FGCOL:			// Set foreground color?
					fd->f.wpnt[1] = TFX_FGCOL;
					value =  win->drw->funcdsp->getcolor(value);
					fd->f.fmt.i.txtcolor = value;
					goto com4byte;

				 case TFX_FGCOL:		// Set foreground color?
					fd->f.fmt.i.txtcolor = value;
				 com4byte:
					*(long *)(fd->f.wpnt + 2) = value;
					fd->f.wpnt += 6;
					break;

				 case TF_BGCOL:
					fd->f.wpnt[1] = TFX_BGCOL;
					dobgcolor(win, fd, win->drw->funcdsp->getcolor(value));
					break;
				}
				fd->tpnt += 6;
				fd->tcnt -= 6;
				fd->total += 6;
			}
			else						// If extended internal function
			{
				switch (func)
				{
				 case TFX_BGCOL:		// (12 bytes) Expanded TF_BGCOL or
										//   TF_BGCOLX function
					dobgcolor(win, fd, *(long *)(fd->tpnt + 2));
					fd->tpnt += 14;
					fd->tcnt -= 14;
					fd->total += 14;
					break;
				}
			}

			// End of function processing
		}
		else if (chr == CR)
		{
			fd->tpnt++;
			fd->tcnt--;
			fd->total++;
		}
		else if (chr == '\n')
		{
			*fd->f.wpnt++ = chr;		// Store and consume the LF
			fd->tpnt++;
			fd->tcnt--;
			fd->total++;
			return (TRUE);
		}
		else
		{
			setupfont(win, fd);			// Make sure font is current
			size = xwsfontgetchrwidthns(xwscurfnt, fd->tpnt, fd->tcnt, 0,
					&width);			// Get width of the character
			if (tb->tbattr & TBA_WORDWRAP) // Doing word-wrap?
			{
				if (((char)chr) == ' ')	// Yes - space character?
				{
					fd->f.spwidth = width; // Save its width
					fd->save = fd->f;	// save the current state in
				}						//   case need to wrap
				else if ((fd->f.fmt.f.leftmargin + // Need to wrap here?
						fd->f.fmt.f.length + width) >=
						fd->f.fmt.f.rightmargin && fd->save.wpnt != NULL)
				{
					fd->wrap = fd->f;	// Yes - save the current state
					fd->f = fd->save;	// Restore the saved state (This is the
										//   state before the space character.)
					fd->save.wpnt = NULL;
					*fd->f.wpnt = 0;	// Add a "funny" space character but
										//   don't count it!
					return (TRUE);
				}
			}
			fd->f.fmt.f.length += width;
			fd->f.numchar++;
			switch (size)
			{
			 case 1:
				*fd->f.wpnt = chr;
				break;

			 case 3:
				fd->f.wpnt[2] = fd->tpnt[2];
			 case 2:
				*(ushort *)fd->f.wpnt  = *(ushort *)fd->tpnt;
				break;

			 default:
				*(long *)fd->f.wpnt  = *(long *)fd->tpnt;
				break;
			}
			fd->f.wpnt += size;
			fd->tpnt += size;			// Consume the character
			fd->tcnt -= size;
			fd->total += size;
		}
	}

	// Here if have consumed all of the input text without finding a LF or
	//   wrapping

	return (FALSE);
}


//*********************************************************
// Function: xwsfinishtext - Finishing storing text in a TL
// Returned: Nothing
//*********************************************************

void XOSFNC xwsfinishtext(
	XWSWIN *win,
	TB     *tb,
	FD     *fd)
{
	TL  *tl;
	TL  *next;
	TL  *prev;
	TL  *ptl;
	long lendiff;
	long chrdiff;
	long left;
	long width;
	long adj;
	long cnt;
	char dosave;

	setupfont(win, fd);					// Make sure font is current
	dosave = TRUE;
	if ((ptl = fd->prev) != NULL)		// Is this the "previous TL"?
	{
		// Here if this is the "previous TL". This is the TL before the
		//   first selection TL which has been formatted to make sure it
		//   does not change.

		if (ptl->count >= (fd->f.wpnt - fd->text)) // Did it change?
		{
			dosave = FALSE;				// No - don't bother saving it
			fd->i.ptl = ptl;
			ptl = NULL;
		}
		else
			fd->invbgn = ptl->fmt.f.length; // Yes - update the line
	}
	fd->prev = NULL;

	// Create a TL if we should

	if (dosave)
	{
		if (fd->f.curbgc != NULL)
		{
			fd->f.curbgc->length = fd->f.fmt.f.length - fd->f.curbgc->xpos;
			fd->f.curbgc->ascent = (fd->f.bgascent != 0) ? fd->f.bgascent :
					(xwscurfnt->ascent + xwscurfnt->leading);
			fd->f.curbgc->descent = (fd->f.bgdescent != 0) ? fd->f.bgdescent :
					xwscurfnt->descent;
		}

		// Allocate and set up a TL (Allocate 1 extra byte to allow appending a
		//   space character when reprocessing the text. The text is not 0
		//   terminated.)

		if ((tl = (TL *)xwsMalloc(offsetof(TL, text) + fd->f.wpnt -
				fd->text + 1)) == NULL)
			xwsFail(-errno, "Could not allocate memory for text");

		tl->label = 'TL##';
		tl->tb = tb;
		tl->xpos = fd->f.fmt.f.leftmargin;

		tl->fmt.f = fd->f.fmt.f;		// Copy the final state of the FMTF
										//   structure to the TL
		tl->fmt.i = fd->i.fmti;			// Copy the initial state of the FMTI
										//   structure to the TL
		tl->count = fd->f.wpnt - fd->text;
		tl->numchar = fd->f.numchar;
		if (tl->count > 0)
			sysLibMemCpyLong((long *)tl->text, (long *)fd->text, // Copy text to
					(tl->count + 3) >> 2);						 //   the TL
		if (fd->f.curbgc != NULL)
		{
			fd->f.fmt.i.bgnbgc = *fd->f.curbgc;
			fd->f.fmt.i.bgnbgc.xpos = 0;
			fd->f.curbgc = &fd->i.fmti.bgnbgc;
		}
		else
		{
			fd->i.fmti.bgnbgc.color = XWS_COLOR_TRANS;
			fd->i.fmti.bgnbgc.length = 0;
			fd->f.curbgc = 0;
		}

		if (tl->fmt.f.lfmt == TLF_MCENTER || tl->fmt.f.lfmt == TLF_BCENTER)
		{
			// Here if the line is centered horizontally - fix up the xpos value
			//   for the line

			if (tl->fmt.f.lfmt == TLF_MCENTER)
			{
				left = tl->fmt.f.leftmargin;
				width = tl->fmt.f.rightmargin - tl->fmt.f.leftmargin;
			}
			else
			{
				left = 0;
				width = tb->xsize;
			}
			adj = left + (width - fd->f.fmt.f.length) / 2 - tl->xpos;
			if (fd->f.caret.lnum > 0)
				fd->f.caret.xpos += adj;
			if (fd->f.anchor.lnum > 0)
				fd->f.anchor.xpos += adj;
			tl->xpos += adj;
		}
		else if (tl->fmt.f.lfmt == TLF_RIGHT)
		{
			// Here if the line is right justified - fixup the xpos value for
			//   the line

			adj = tl->fmt.f.rightmargin - tl->fmt.f.length;
			if (fd->f.caret.lnum > 0)
				fd->f.caret.xpos += adj;
			if (fd->f.anchor.lnum > 0)
				fd->f.anchor.xpos += adj;
			tl->xpos += adj;
		}

		// If this is the first TL emitted (excluding a possible "previous
		//   line") save its address.

		if (ptl != NULL)
			fd->newptl = tl;
		else if (fd->newstl == NULL)
			fd->newstl = tl;

		// Link the new TL into the TL list for the text box and set its
		//   vertical position

		if ((prev = fd->i.ptl) != NULL)
		{
			next = prev->next;
			tl->ypos = prev->ypos + prev->fmt.f.ysize;
			tl->lnum = prev->lnum + 1;
			tl->ttlcnt = prev->ttlcnt + prev->count + 1;
			prev->next = tl;
		}
		else
		{
			next = tb->tlhead;
			tl->ypos = tb->topmargin;
			tl->lnum = 1;
			tl->ttlcnt = 0;
			tb->tlhead = tl;
		}
		tl->prev = prev;
		if ((tl->next = next) != NULL)
			next->prev = tl;
		else
			tb->tltail = tl;
		fd->i.ptl = tl;

		if (fd->f.caret.lnum > 0)
		{
			win->caret = fd->f.caret;
			win->caret.tl = tl;
			win->caret.lnum = tl->lnum;
			win->cartarget = win->caret.xpos;
			xwstextinvalidatecaret(win);
			fd->f.caret.lnum = fd->wrap.caret.lnum = -1;
		}
		if (fd->f.anchor.lnum > 0)
		{
			win->anchor = fd->f.anchor;
			win->anchor.tl = tl;
			win->anchor.lnum = tl->lnum;
			fd->f.anchor.lnum = fd->wrap.anchor.lnum = -1;
		}
		if (fd->invbgn >= 0)
		{
			xwswininvalidaterectns(win, tb->xpos + tl->xpos + fd->invbgn,
					tb->ypos + tl->ypos, tl->fmt.f.length - fd->invbgn,
					tl->fmt.f.ysize);
			fd->invbgn = 0;
		}
	}

	// Set up for processing the next line

	fd->i.fmti = fd->f.fmt.i;			// The final FMTI structure becomes the
										//   initial FMTI structure for the
										//   next line.

	// If the line wrapped we must copy what did not fit to the beginning
	//   of the buffer and adjust the formatting state accordingly. When
	//   we get here fd->fmt contains the formatting state at the point
	//   where the previous line wrapped. fd->wrap contains the formatting
	//   state after the last character processed.

	if (fd->wrap.wpnt != NULL)
	{
		lendiff = fd->wrap.fmt.f.leftmargin + fd->wrap.fmt.f.length -
				(fd->f.fmt.f.leftmargin + fd->f.fmt.f.length) -
				fd->wrap.spwidth;
		chrdiff = fd->wrap.numchar - fd->f.numchar - 1;
		if ((cnt = fd->wrap.wpnt - fd->f.wpnt - 1) > 0)
			sysLibMemCpy(fd->text, fd->f.wpnt + 1, cnt);
		if (fd->wrap.caret.lnum > 0)
		{
			fd->wrap.caret.xpos -= (fd->f.fmt.f.length + fd->wrap.spwidth);
 			fd->wrap.caret.cinx -= (fd->f.wpnt - fd->text + 1);
			fd->wrap.caret.cnum -= (fd->f.numchar + 1);
		}
		if (fd->wrap.anchor.lnum > 0)
		{
			fd->wrap.anchor.xpos -= (fd->f.fmt.f.length + fd->wrap.spwidth);
 			fd->wrap.anchor.cinx -= (fd->f.wpnt - fd->text + 1);
 			fd->wrap.anchor.cnum -= (fd->f.numchar + 1);
		}
		fd->f = fd->wrap;
		fd->f.wpnt = fd->text + cnt;
		fd->f.fmt.f.length = lendiff;
		fd->f.numchar = chrdiff;
		fd->wrap.wpnt = NULL;
	}
	else
	{
		fd->f.wpnt = fd->text;
		fd->f.fmt.f.length = 0;
		fd->f.numchar = 0;
		fd->save.wpnt = NULL;
	}
	fd->f.spwidth = 0;

	// If we saved the "previous line" invalidate it and give up the original
	//   line.

	if (ptl != NULL)
	{
		if (ptl->fmt.f.ysize != fd->f.fmt.f.ysize) // Did the height change?
										// Yes - invalidate the whole line
			xwswininvalidaterectns(win, tb->xpos + tl->xpos + fd->invbgn,
					tb->ypos + tl->ypos, tl->fmt.f.length - fd->invbgn,
					(ptl->fmt.f.ysize > fd->f.fmt.f.ysize) ? tl->fmt.f.ysize :
					fd->f.fmt.f.ysize);
		ptl->prev->next = ptl->next;	// Unlink this TL
		if (ptl->next != NULL)
			ptl->next->prev = ptl->prev;
		else
			tb->tltail = ptl->prev;
		xwsFree(ptl);					// Give it up
	}
}


//************************************************//
// Function: dobgcolor - Process format functions //
//             that set a text background color   //
// Returned: Nothing                              //
//************************************************//

static void XOSFNC dobgcolor(
	XWSWIN *win,
	FD     *fd,
	long    color)
{
	BGC *bgc;

	// Finish up the current background area if there is one

	if ((bgc = fd->f.curbgc) != NULL)
	{
		setupfont(win, fd);
		bgc->length = fd->f.fmt.f.length - bgc->xpos;
		bgc->ascent = (fd->f.bgascent != 0) ? fd->f.bgascent :
				(xwscurfnt->ascent + xwscurfnt->leading);
		bgc->descent = (fd->f.bgdescent != 0) ? fd->f.bgdescent :
				xwscurfnt->descent;
	}

	// Set up the new background area if it is not transparent

	if (color != XWS_COLOR_TRANS)
	{
		fd->f.wpnt[1] = TFX_BGCOL;
		bgc = (BGC *)(fd->f.wpnt + 2);
		bgc->color = color;
		bgc->length = 0;
		bgc->xpos = fd->f.fmt.f.length;
		bgc->ascent = 0;
		bgc->descent = 0;
		fd->f.curbgc = bgc;
		fd->f.wpnt += 14;
	}
	else
	{
		fd->f.wpnt[1] = TF_CLRCBGC;
		fd->f.curbgc = NULL;
		fd->f.wpnt += 2;
	}
	fd->f.bgascent = fd->f.bgdescent = 0;
}


//**********************************************
// Function: setupfont - Set up the current font
// Returned: Nothing
//**********************************************

// If the requested font can not be set up for some reason, the default font
//   (from the first entry in the EDB font table) is set up. If this fails a
//   fatal error is given.

static void XOSFNC setupfont(
	XWSWIN *win,
	FD     *fd)
{
	long    val;

	if (xwssetupfont(win, &fd->f.reqfnt) || fd->f.fmt.f.ascent == 0)
	{
		fd->f.fmt.i.font = xwsfntval;

		// Update ascent and descent values if they are now larger

		val = (xwscurfnt->ascent + xwscurfnt->leading);
		if (fd->f.fmt.f.ascent < val)
			fd->f.fmt.f.ascent = val;
		if (fd->f.bgascent < val)
			fd->f.bgascent = val;

		if (fd->f.fmt.f.descent < xwscurfnt->descent)
			fd->f.fmt.f.descent = xwscurfnt->descent;
		if (fd->f.bgdescent < xwscurfnt->descent)
			fd->f.bgdescent = xwscurfnt->descent;
		fd->f.fmt.f.ysize = fd->f.fmt.f.leading + fd->f.fmt.f.ascent +
				fd->f.fmt.f.descent + fd->f.fmt.f.lspace;
	}
}


int XOSFNC xwssetupfont(
	XWSWIN *win,
	FNTVAL *reqf)
{
	XWSEDB *edb;

	if (xwsfntval.num == reqf->num && xwsfntval.height == reqf->height &&
			xwsfntval.attr == reqf->attr && xwsfntval.weight == reqf->weight &&
			xwsfntval.width == reqf->width) // Need to change the font?
		return (FALSE);					// No - nothing needed here
	if (xwscurfnt != NULL)				// Yes - do we have a font now?
	{
		if (xwscurfnt->label != 'FNT#') // Debug check
			xwsFail(0, "Invalid FNT structure");
		xwsFontUnload(xwscurfnt);		// Unload it
		xwscurfnt = NULL;
	}
	edb = win->edb;
	if (reqf->num < 1 || reqf->num >= edb->numfonts)
		reqf->num = 1;
	if ((xwscurfnt = xwsFontLoad(edb->fonttbl[reqf->num - 1], win, reqf->height,
			reqf->width, reqf->attr | 0x80000000)) == NULL)
	{
		// Here if could not load the requested font for some reason - Try
		//   the default font

		if ((xwscurfnt = xwsFontLoad(edb->fonttbl[0], win, reqf->height,
				reqf->width, reqf->attr)) == NULL)
			xwsFail(0, "Could not load default font ");
	}
	xwsfntval = *reqf;					// Store the new font parameters as
	return (TRUE);						//   the current parameters
}

