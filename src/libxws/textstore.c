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

// The following formatting functions are modified here:
//   TF_FGCOLX: Changed to TF_FGCOL function with 1-byte index replaced with
//                4-byte color value.
//   TF_BGCOLX: Changed to TF_BGCOL function with 1-byte index replaced with
//                4-byte color value followed by 12 added bytes.
//   TF_BGCOL:  12 bytes are added to the value.
// All other function are stored as-is in the working TL (wtl) text buffer

// There are a number of values in each TL that define the vertical position
//   of a line: (all distances are in pixels)
//     ypos    - The distance  from the top of the text box to the top of the
//                 the line's bounding box.
//     ascent  - The distance from the top of the line's bounding box to the
//                 text base line.
//     descent - The distance from the text base line to the bottom of the
//                 bottom of the font area.
//     lspace  - The distance from the bottom of the font area to the bottom
//                 of the lines's bounding box.
// Thus the height of the line's bounding box is ascent + descent + lspace.
//   ascent and descent are the maximum values for all fonts on the line. When
//   a non-transparent background color is specified it extends above and
//   below the text base line by the maximum ascent and descent values for
//   all fonts in the area covered. These may be less than the ascent and
//   descent values for the TL.
// The ypos value for the first TL is always equal to the tmargin value for
//   the TB.

// A TB always contains at least one TL. The final TL in a TB will always
//   be unterminated (TFB_EOL and TFB_EOP are both 0). When adding text
//   to the end of a TB and the final line is terminated, an additional
//   unterminated line is added. This ensures that there is always a place
//   to put the carret at the end of the text box.

// We go to quite a bit of trouble to minimize the amount that is invalidated
//   when creating and modifying a text box.
// When creating a text box with a non-transparent background the entire
//   text box is invalidated. When creating a text box with a transparent
//   background, only the bounding rectangle for the text on each line is
//   invalidated.
// When modifying a text box the first line modified is invalidated from the
//   position of the first character changed to the end of the changed area.
//   For any following lines modified, the union of the original and new text
//   bounding boxes are invalidated. If the vertical position of any line
//   changes, any following area that did or will contain text is invalidated.

	long clxpos;
	long clypos;
	long clbtm;
	long clright;
	long clclxpos;


//***************************************************//
// Function: xwstextstr - Store text into a text box //
// Returned: Number of new characters stored         //
//***************************************************//

// Text is inserted at the caret position. If there is a highlighted area
//   before or after the caret, the highlighted area is first deleted.

long xwstextstr(
	XWSWIN *win,		// WIN
	TB     *tb,			// TB
	char   *text,		// Text to add
	long    length)		// Length of the text (Byte count including all
						//   command bytes and all multibyte Unicode bytes.)
{
	FD      fd;					// Formatting data
	STA     temp;
	TB     *tb2;
	TL     *tl;
	TL     *tl2;
	TL     *next;
	TL     *prev;
	TL     *ptl;
	TL     *ctl;
	TL     *atl;
	TL     *tl3;
	TL     *firsttl;
	TL     *lasttl;
	long    num;
	long    ypos;
	long    height;
	long    bottom;
	long    right;
	long    ysize;
	long    oldtop;
	long    oldbtm;
	long    newtop;
	long    newbtm;
	long    changed;
	long    lasttotal;
	char    stopped;

	if (win->config & WCB_CARET)
		xwstextinvalidatecaret(win);

	// If the caret is at the end of the lighlighted area, move it to the
	//   beginning so we don't have to worry about its position when we are
	//   done. This lets us use the caret values as the "first selected"
	//   values and the anchor values as the "last selected" values.

	if (win->caret.lnum > win->anchor.lnum ||
			(win->caret.lnum == win->anchor.lnum &&
			win->caret.xpos > win->anchor.xpos))
	{
		temp = win->caret;
		win->caret = win->anchor;
		win->anchor = temp;
	}
	else
		temp.tl = (TL *)0xFFFFFFFF;		// We use this as a flag that we don't
										//   need to invalidate the caret when
										//   nothing was inserted (If we swapped
										//   caret and anchor and the caret is
										//   not moved we must invalidate it.)
	ctl = win->caret.tl;
	atl = win->anchor.tl;
	tb = ctl->tb;

	// Remember the X and Y positions, length, height and caret X position
	//   for the line containg the caret. We need this to see if we need to
	//   invalidate some more after we are finished. This is necessary since
	//   we treat invalidating this line specially so as to avoid invalidating
	//   the area before the selected area if we can. This is the usual case
	//   but there are a few rare cases (such as changing the maximum font
	//   height for the line) that require invalidating the entire line. It's
	//   easier to wait until we are almost finished and check for this then.

	clypos = ctl->ypos;
	clxpos = ctl->xpos;
	clright = ctl->xpos + ctl->fmt.f.length;
	clbtm = ctl->ypos + ctl->fmt.f.ysize;
	clclxpos = win->caret.xpos;

	// Get the change in the total number of bytes in the TB. This is used
	//   to determine when we can stop formatting.

	changed = length - (atl->ttlcnt + win->anchor.cnum - (ctl->ttlcnt +
			win->caret.cnum));

	sysLibMemSetLong((long *)&fd.f, 0, (sizeof(FDF) + sizeof(FDI))/4);
	fd.save.wpnt = fd.wrap.wpnt = NULL;
	fd.f.spwidth = 0;
	fd.f.wpnt = fd.text;
///	fd.f.curbgc = &fd.i.fmti.bgnbgc;
	fd.f.curbgc = NULL;

	fd.newptl = fd.newstl = NULL;

///	if (fd->i.fmti.bgnbgc.length != 0)
///	{
///		fd->f.curbgc = &fd->i.fmti.bgnbgc;
///		fd->i.fmti.bgnbgc.xpos = 0;
///	}
///	else



	// Normally, only the selected text and following text are modifed when
	//   inserting or deleting. However, if inserting a space or deleting
	//   anything before a space in a line and the preceeding line wraped,
	//   the PRECEEDING line may change because the first word in the selected
	//   area may now fit on the preceeding line! If quite a bit of work to
	//   determine if this condition is met, so if we have a preceeding
	//   wrapped line, we always format it first. If it changes, we use the
	//   changed version. If not, we discard it and proceed normally. This

///NO NO NO - BC COLORS!!!!

	//   sounds worse than it is. We do not really need to reformat it at
	//   all since we know the format state of the end of the line. We
	//   simply set the formatting state and continue. We do not bother
	//   copying the data from the TL to our buffer. That will be done by
	//   xwsfinishtext if it is necessary to store a modified version of
	//   this line.

	// NOTE: xwsfinishtext checks fd.prev when storing a TL and does the
	//       right thing if it is storeing that TL. Therefore no special
	//       checks for this are required here even if that TL is not
	//       emitted well into this routine (which is possible).

	if ((fd.prev = ctl->prev) != NULL &&
			fd.prev->text[fd.prev->count - 1] != '\n')
	{
		// Set the formatting state for the end of the previous line (which
		//   is almost the same as state of the start of the caret line.

		fd.f.fmt = fd.prev->fmt;
		fd.i.fmti = fd.prev->fmt.i;
		fd.i.ptl = ptl = fd.prev->prev;
		fd.f.reqfnt = fd.i.fmti.font;
		fd.f.fmt.f.length = 0;

		// Format the previous line

		fd.tpnt = fd.prev->text;
		fd.tpnt[fd.prev->count] = ' ';
		fd.tcnt = fd.prev->count + 1;
		do
		{
			if (xwsformattext(win, tb, &fd) || fd.tcnt > 0)
				xwsfinishtext(win, tb, &fd);
		} while (fd.tcnt > 0);
	}
	else
	{
		// Here if don't need to reformat the previous line

		fd.prev = NULL;
		fd.f.fmt = ctl->fmt;
		fd.i.fmti = fd.f.fmt.i;
		fd.i.ptl = ptl = ctl->prev;
		fd.f.reqfnt = fd.i.fmti.font;
		fd.f.fmt.f.length = 0;
	}

	fd.f.fmt.f.ysize = fd.f.fmt.f.leading + fd.f.fmt.f.ascent +
			fd.f.fmt.f.descent + fd.f.fmt.f.lspace;


	// Unlink all TLs which contain highlighted text from the TB. These will
	//   all be replaced. This leaves tl pointing to the first line after
	//   the selected lines.

	if (ctl->prev == NULL)
		tb->tlhead = atl->next;
	else
		ctl->prev->next = atl->next;
	if ((tl = atl->next) == NULL)
		tb->tltail = ctl->prev;
	else
		tl->prev = ctl->prev;

	// Any unselected text in the first selected TL is formatted by calling
	//   xwsformattext. If we don't reformat the previous line this will
	//   never emit a TL but it will probably do so if we reformat the
	//   previous line.

	fd.invbgn = win->caret.xpos - ctl->xpos;
	if (win->caret.cinx > 0)
	{
		fd.tcnt = win->caret.cinx;
		fd.tpnt = ctl->text;
		do
		{
			if (xwsformattext(win, tb, &fd) || fd.tcnt > 0)
				xwsfinishtext(win, tb, &fd);
		} while (fd.tcnt > 0);
	}

	win->caret.tl = win->anchor.tl = NULL; // No selection now (leave everything
										   //   except TL since we use it)

	// Invalidate the area in the first line that we changed. If the previous
	//   line wasThis always
	//   extends from the start of the selected area to the end of the line.
	//   If there is unselected text after the selected area it is extremely
	//   likely that it will have shifted. We don't bother to check for this
	//   but simply invalidate it.

	if (ctl != atl)						// Single line selected area?
	{
		// Give up all but the last selected TL - These have already been
		//   unlinked from the TB so all we  have to do is give them up.

		tl2 = ctl;
		do
		{
			if (tl2 == NULL)
				xwsFail(0, "Bad selected text area");
			if (tl2 != ctl)
				xwswininvalidaterectns(win, tb->xpos + tl2->xpos, tb->ypos +
					tl2->ypos,  tl2->fmt.f.length, tl2->fmt.f.ysize);
			next = tl2->next;
			xwsFree(tl2);

		} while ((tl2 = next) != atl);
	}

	// Now add the new text. This may or may not create one or more new TLs.

	fd.total = 0;
	lasttotal = 0;
	fd.f.caret.lnum = fd.f.anchor.lnum = 0;
	if ((fd.tcnt = length) > 0)
	{
		fd.tpnt = text;
		do
		{
			if (xwsformattext(win, tb, &fd))
			{
				xwsfinishtext(win, tb, &fd);

				// If we are checking for overflow and the text has overflowed
				//   simply return. If we are checking, we know that this is
				//   not a list window, the text is not vertically centered,
				//   and there is nothing following, so we can just return
				//   here!

				if (tb->tbattr & TBA_NOOVER)
				{
///					INT3;

					if ((fd.i.ptl->ypos + fd.i.ptl->fmt.f.ysize) > tb->ysize)
					{
						// Here if the text has overflowed the text box and
						//   we care about this. We must delete the last line
						//   and return the number of characters added before
						//   this. We know that this was the last line in the
						//   text box.

						if ((prev = fd.i.ptl->prev) == NULL)
							tb->tlhead = NULL;
						else
							prev->next = NULL;
						tb->tltail = prev;
						xwsFree(fd.i.ptl);
						fd.i.ptl = prev;

						// NOTE: This may leave the TB with the last line
						//       complete. Since we are assuming that a TB
						//       that overflows will never be modified this
						//       will not cause any problems. This probably
						//       should be fixed sometime, probably when we
						//       implimente linked TBs.

						return (lasttotal);
					}
				}
				lasttotal = fd.total;
			}
		} while (fd.tcnt > 0);
	}

	fd.tpnt = "\x10\x04";
	fd.tcnt = 2;
	if (xwsformattext(win, tb, &fd))
		xwsfinishtext(win, tb, &fd);

	// Format text until we have consumed the unselected part of the last
	//   selected TL (This will normally generate one or no TLs but could
	//   generate more than one TL if the margins have been significantly
	//   reduced.)

	if ((fd.tcnt = atl->count - win->anchor.cinx) > 0)
	{
		fd.tpnt = atl->text + win->anchor.cinx;
		if (atl->next != NULL && fd.tpnt[fd.tcnt - 1] != '\n')
		{
			fd.tpnt[fd.tcnt] = ' ';
			fd.tcnt++;
		}
///		fd.f.reqfnt = atl->fmt.i.font;
		do
		{
			if (xwsformattext(win, tb, &fd))
				xwsfinishtext(win, tb, &fd);
		} while (fd.tcnt > 0);
		if (ctl != atl)					// Single line selected area?
										// No - invalidate the entire last
										//   selected line
			xwswininvalidaterectns(win, tb->xpos + atl->xpos, tb->ypos +
					atl->ypos,  atl->fmt.f.length, atl->fmt.f.ysize);
	}
	else if (tl != NULL)
	{
		// Here if there is no tralling unselected part - This means we must
		//   have inserted at the end of a line. If it was not the last line
		//   and the insersion caused the line to wrap, we must insert a space
		//   at this point.

		fd.tcnt = 1;
		fd.tpnt = " ";
		do
		{
			if (xwsformattext(win, tb, &fd))
				xwsfinishtext(win, tb, &fd);
		} while (fd.tcnt > 0);
	}

	xwsFree(atl);						// Give up the final selected line

	// Format any following lines that change. We only want to reformat and
	//   invalidate as much as we need to. In general determining when we
	//   can stop is somewhat complex. If we find a line that we just
	//   constructed that has the same total character count value (ttlcnt)
	//   and the same formatting state (fmt) we can stop. Unfortunately,
	//   we may need to generate fewer or more lines than we consume before
	//   this occurs. We handle this by keeping all of the TL beyond this
	//   point (including ones we have already used) and compare each one
	//   to each line we generate. When (if) we find a match, we stop
	//   formatting, discard all old lines before the one that match and
	//   keep the match line and all the lines following (we did not change).
	//   This does NOT check for a change in vertical position since that
	//   does not effect formatting, just display. This is check for below
	//   and the necessary areas are invalidated there.

	stopped = FALSE;					// Assume will not stop formatting
										//   before the end of the TB
	if (tl != NULL)
	{
		// Disconnect the following lines from the previous ones

		firsttl = tl;
		lasttl = tb->tltail;
		if ((prev = firsttl->prev) == NULL)
			tb->tlhead = NULL;
		else
			prev->next = NULL;
		tb->tltail = prev;
		while (tl != NULL)
		{
			// Format the text

			fd.tpnt = tl->text;
			fd.tcnt = tl->count;
			next = tl->next;
			if (next != NULL && fd.tpnt[fd.tcnt - 1] != '\n')
			{
				fd.tpnt[fd.tcnt] = ' ';
				fd.tcnt++;
			}
			do
			{
				if (xwsformattext(win, tb, &fd))
				{
					// Here with a complete line - See if it matches an
					//   old line.

					tl3 = firsttl;
					num = ((fd.i.ptl != NULL) ? (fd.i.ptl->ttlcnt +
							fd.i.ptl->count + 1) : 0) - changed;
					while (tl3 != NULL && num > tl3->ttlcnt)
						tl3 = tl3->next;
					if (tl3 != NULL && num == tl3->ttlcnt &&
							sysLibMemCmpLong((long *)&tl3->fmt,
							(long *)&fd.f.fmt, sizeof(FMT)/4) == 0)
					{
						// Here with a matching line - There is no need
						//   to reformat (or invalidate) beyond this point
						//   but we need to link the unchnaged lines back
						//   in and update their ttlcnt values.

						if (tl3 != firsttl)
							tl3->prev->next = NULL;
						else
							firsttl = NULL;

						if (fd.i.ptl == NULL)
							tb->tlhead = tl3;
						else
							fd.i.ptl->next = tl3;
						tl3->prev = fd.i.ptl;
						tb->tltail = lasttl;

						// Scan through the unchanged TLs and update the
						//   ttlcnt values

						tl = tl3;
						num += changed;
						while (tl3 != NULL)
						{
							tl3->ttlcnt = num;
							num += (tl3->count + 1);
							tl3 = tl3->next;
						}
						stopped = TRUE;
						break;
					}
					else				// If don't have a match - Emit the
										//   new TL and continue.
						xwsfinishtext(win, tb, &fd);
				}
			} while (fd.tcnt > 0);
			if (stopped)
				break;
			tl = next;
		}

		// Invalidate and give up all of the old TLs that we replaced

		while (firsttl != NULL)
		{
			xwswininvalidaterectns(win, tb->xpos + firsttl->xpos, tb->ypos +
					firsttl->ypos, firsttl->fmt.f.length, firsttl->fmt.f.ysize);
			xwsFree(firsttl);
			firsttl = firsttl->next;
		}
	}

	// Handle a possible incomplete last line. This will only happen when
	//   we have formated the last TL in the text box and it did not end
	//   with a LF.

	if (!stopped)
		xwsfinishtext(win, tb, &fd);

	// The following is not needed if the caret line was previously 0 length

	if (clright > clxpos)
	{

		// Invalidate the original coverage of the line that contained the caret.
		//   We must defer this to here since we are not sure what needs to be
		//   invalidated until we are finished. There are a few strange cases
		//   where the caret line is not complete until we get here. There are
		//   several cases we must consider:
		//   1. The caret line moved or changed height or we reformatted the
		//      previous line: We reformat the entire caret line.
		//   2. Caret line now ends before the original caret position: We
		//      invalidate from the new caret position to the end of the line.
		//   3. The previous line was not reformatted and the caret line was not
		//      shortened to before the caret (normal case): We invalidate from
		//      the caret position (which is unchanged) to the end of the line
		//      (the greater of the old and new ends).

		right = fd.newstl->xpos + fd.newstl->fmt.f.length;
		bottom = fd.newstl->ypos + fd.newstl->fmt.f.ysize;
		if (clright < right)
			clright = right;

		// First see if we formatted the previous line or the caret line has
		//   moved or changed height - if so, invalidate the entire caret
		//   line.

		if (fd.newptl != NULL || clxpos != fd.newstl->xpos || clypos !=
					fd.newstl->ypos || clbtm != bottom)
		{
			if (clxpos > fd.newstl->xpos)
				clxpos = fd.newstl->xpos;
			if (clypos > fd.newstl->ypos)
				clypos = fd.newstl->ypos;

			if (clbtm < bottom)
				clbtm = bottom;

			xwswininvalidaterectns(win, tb->xpos + clxpos, tb->ypos + clypos,
					clright - clxpos , clbtm - clypos);
		}

		// See if we shortened the line to before the original caret position.
		//   If so we invalidate from the end of the line to the new cared
		//   position

		else if ((fd.newstl->xpos + fd.newstl->fmt.f.length) < clclxpos)
			xwswininvalidaterectns(win, tb->xpos + fd.newstl->fmt.f.leftmargin +
					fd.newstl->fmt.f.length, tb->ypos + clypos, clclxpos -
					(fd.newstl->fmt.f.leftmargin + fd.newstl->fmt.f.length),
					clbtm - clypos);

		// Otherwise we have the normal case and invalidate from the caret position
		//   to the end of the line.

		else
			xwswininvalidaterectns(win, tb->xpos + clclxpos, tb->ypos + clypos,
					clright - clclxpos, clbtm - clypos);
	}
	if ((tb->tbattr & TBA_VERCEN) && (tb->tbattr & TBA_HEIGHT))
	{
		// Here if the TB is vertically centered. We recalculate the ypos
		//   value for the first line. This is done first without changing
		//   any ypos values so we can minimize the number of unnecessary
		//   invalidations that we do. (NOTE: Only fixed height text boxes
		//   may contain vertically centered text.)

		tl = tb->tlhead;
		height = 0;
		do
		{
			height += tl->fmt.f.ysize;
		} while ((tl = tl->next) != NULL);
		ypos = (tb->ysize - (tb->topmargin + height + tb->btmmargin)) >> 1;

		// We how have the new Y position of the first line. We now scan
		//   through all of the lines. If a line's Y position has changed,
		//   we invalidate the line at its original position, update the
		//   position and invalidate the line at its new positon. Any
		//   lines that have been modified have already been invalidated
		//   so all we need to invalidate here are lines that have moved.
		//   This may negate the need for some of the invalidations already
		//   done but this should be uncommon. We do not expect to have
		//   many insertions or deletions done on  vertically centered text.
		//   Vertically centered text is most often used for button text
		//   and sometimes for list window text. Both are usually not
		//   modified at the character level.

		//   ???? (All invalidations are suppressed when initially
		//   creating a text box with a background.)

		tl = tb->tlhead;
		do
		{
			if (tl->ypos != ypos)
			{
				xwswininvalidaterectns(win, tb->xpos + tl->xpos, tb->ypos +
						tl->ypos,  tl->fmt.f.length, tl->fmt.f.ysize);
				tl->ypos = ypos;
				xwswininvalidaterectns(win, tb->xpos + tl->xpos, tb->ypos +
						tl->ypos,  tl->fmt.f.length, tl->fmt.f.ysize);
			}
			ypos += tl->fmt.f.ysize;
		} while ((tl = tl->next) != NULL);
	}
	else
	{
		// Here if the TB is not vertically centered. In this case we know
		//   that any lines above the first line modified have not moved and
		//   do not need to be invalidated. We also know that any modified/
		//   removed/added lines have already been invalidated and that any
		//   following line that has not moved does not need to be invalidated.
		//   Once we find an unmodified line that has not moved we know that
		//   none of the following lines have moved.

		// tl points to the first unmodified TL.

		if (tl != NULL)
		{
			if ((tl2 = tl->prev) == NULL)
				xwsFail(0, "Text error: No modified line");
			ypos = tl2->ypos + tl2->fmt.f.ysize;
			while (ypos != tl->ypos)
			{
				xwswininvalidaterectns(win, tb->xpos + tl->xpos, tb->ypos +
						tl->ypos,  tl->fmt.f.length, tl->fmt.f.ysize);
				tl->ypos = ypos;
				xwswininvalidaterectns(win, tb->xpos + tl->xpos, tb->ypos +
						tl->ypos,  tl->fmt.f.length, tl->fmt.f.ysize);
				ypos += tl->fmt.f.ysize;
				if ((tl = tl->next) == NULL)
					break;
			}
		}
	}

	// Here with all ypos and lnum values updated.

	tl2 = tb->tltail;

	if ((tb->tbattr & TBA_HEIGHT) == 0)	// Is the height fixed?
	{									// No - get the current height
		ysize = tl2->ypos + tl2->fmt.f.ysize + tb->btmmargin;
		if (ysize != tb->ysize)			// Has the height changed?
		{
			if (tb->bgdcolor != XWS_COLOR_TRANS)
			{
				if (ysize > tb->ysize)	// Yes - has it increated?
										// Yes - invalidate the increase
					xwswininvalidaterectns(win, tb->xpos, tb->ypos + tb->ysize,
							tb->xsize, ysize - tb->ysize);
				else					// No - invalidate the descrease
					xwswininvalidaterectns(win, tb->xpos, tb->ypos + ysize,
							tb->xsize, tb->ysize - ysize);
			}
			tb->ysize = ysize;			// Store the new height
		}
	}

	// Do special fix-up for list windows

	if (win->config & WCB_LIST)
	{
		// Here if the TB is in a list type window. In this case if the height
		//   of the TB changes we must adjust the positions of all following
		//   TBs and invalidate them. When we get here we have already
		//   invalidated as much of the current TB as is necessary. While
		//   we really should do before and after invalidations of each
		//   following TB that is a lot of everhead and in most cases will
		//   not do much to reduce flicker. Thus we simplely invalidate a
		//   single rectangle starting with the minimum old and new top
		//   and ending with the maximum old and new bottom or the bottom
		//   of the list window.

		ypos = newtop = tb->ypos + tb->ysize;
		if ((tb2 = tb->next) != NULL && tb2->ypos != ypos)
		{
			oldtop = tb2->ypos;			// Remember the current top and bottom
			oldbtm = win->tbtail->ypos + win->tbtail->ysize;

			// Fix up the ypos of each TB

			while (tb2->next != NULL)
			{
				tb2->ypos = ypos;
				ypos += (tb2->ysize + tb2->yinc);
				tb2 = tb2->next;
			}
			oldbtm = tb2->ypos + tb2->ysize;
			tb2->ypos = ypos;
			newbtm = ypos + tb2->ysize;
			if (newtop > oldtop)
				newtop = oldtop;
			if (newbtm < oldbtm)
				newbtm = oldbtm;
			xwswininvalidaterectns(win, 0, newtop, win->clntwidth,
					newbtm - newtop);
		}
		if (fd.f.selitem)				// Want to select this item?
			xwslistsetseltb(win, tb);	// Yes
		if (win->versb != NULL)			// Do we have a vertical scroll bar?
			xwslistscrollbarset(win);	// Yes - adjust it if necessary
	}
	else if (win->type == XWS_WINTYPE_EDIT && win->versb != NULL)
		xwseditdoscroll(win);
	return (fd.total);
}
