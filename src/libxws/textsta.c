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

//=================================================================//
// This file contains functions that manage the selected area of a //
//   text box.                                                     //
//=================================================================//


//*********************************************************//
// Function: xwsWinTextSetSel - Set the selected text area //
// Returned: 0 if normal or an XOS error code if error     //
//*********************************************************//

long XOSFNC xwsWinTextSetSel(
	XWSWIN *win,
	long    inx,
	long    bgnline,
	long    bgnchar,
	long    length)
{
	STA  cpd;
	TB  *tb;
	TL  *tl;

	if ((tb = xwsfindtextboxbyinx(win, inx, NULL)) == NULL)
		return (ER_NTDEF);
	tl = tb->tlhead;
	while (--bgnline >= 0 && tl != NULL)
		tl = tl->next;
	if (bgnline >= 0)
		return (0);
	xwstextgetcharpos(win, tl, bgnchar, 0x7FFFFFFFF, &cpd);
	xwstextsetcaret(win, &cpd, TRUE);
	xwstextmovecarethoriz(win, length, TRUE);
	return (0);
}


//***********************************************************//
// Function: xwstextmovecarethoriz - Move caret horizontally //
// Returned: TRUE if something changed, FALSE if no change   //
//***********************************************************//

int XOSFNC xwstextmovecarethoriz(
	XWSWIN *win,		// Address of WIN
	long    num,		// Number of characters to move (+ = right, - = left)
	long    keepsta)	// Non-zero if should retain selected area
{
	TL  *tl;
	STA  cpd;
	long pos;

	CHECKSTA(win);

	if ((tl = win->caret.tl) == NULL)
		return (FALSE);
	if ((pos = win->caret.cnum + num) < 0) // Moving past beginning of line?
	{
		if (tl->prev == NULL)			// Yes - move to end of previous line
			return (FALSE);				//   if there is a previous line
		tl = tl->prev;
		pos = 0x7FFF;
	}
	else if (pos > tl->numchar)			// Moving past end of line?
	{
		if (tl->next == NULL)			// Yes - move to beginning of next line
			pos = tl->numchar;			//   if there is a next line
		else
		{
			tl = tl->next;
			pos = 0;
		}
	}
	xwstextgetcharpos(win, tl, pos, 0x7FFFFFFF, &cpd);
	return ((keepsta) ? xwstextsetsta(win, &cpd, TRUE) :
			xwstextsetcaret(win, &cpd, TRUE));
}


//*********************************************************//
// Function: xwstextmovecaretvert - Move caret vertically  //
// Returned: TRUE if something changed, FALSE if no change //
//*********************************************************//

// When moving vertically we want to move to the character which is at the
//   horizontal target position for vertical movement. This is the horizontal
//   position at which something other than pure vertical movement was done.
//   Keeping track of this preserves the horizontal positon of the caret when
//   moving through shorter lines using the arrow keys.

int XOSFNC xwstextmovecaretvert(
	XWSWIN *win,		// Address of WIN
	long    num,		// Number of lines to move (+ = down, - = up)
	long    keepsta)	// Non-zero if should retain selected area
{
	TL  *tl;
	STA  cpd;

	CHECKSTA(win);

	if ((tl = win->caret.tl) == NULL)
		return (FALSE);

	if (num >= 0)						// Get the target line
		while (--num >= 0 && tl->next != NULL)
			tl = tl->next;
	else
		while (++num <= 0 && tl->prev != NULL)
			tl = tl->prev;
	xwstextgetcharpos(win, tl, 0x7FFFFFFF, tl->tb->xpos + win->cartarget, &cpd);
	return ((keepsta) ? xwstextsetsta(win, &cpd, FALSE) :
			xwstextsetcaret(win, &cpd, FALSE));
}


long drawypos;
long drawheight;


//************************************************//
// Function: xwstextsetcaret - Reset the selected //
//             area and set the caret position    //
// Returned: Nothing                              //
//************************************************//

int XOSFNC xwstextsetcaret(
	XWSWIN *win,		// Address of XWSWIN
	STA    *cpd,		// Address of STA
	int     settar)
{
	TB  *tb;
	TL  *tl;
	long width;
	long height;

	CHECKSTA(win);

	tl = cpd->tl;
	tb = tl->tb;

	// See if we are changing the caret position

	if (tl == win->caret.tl && tl == win->anchor.tl && cpd->xpos ==
			win->caret.xpos && cpd->xpos == win->anchor.xpos)
		return (FALSE);

	// Here if actually chaning the caret position

//// DO WE NEED THIS TEST????

	if (win->caret.tl != NULL)			// Do we have a selected area?
	{
		xwstextinvalidatecaret(win);
		xwstextinvalidatesta(win, &win->caret, &win->anchor);
	}

	// Now set up the new zero length selected area

	win->caret = win->anchor = *cpd;

	// Draw the caret directly - This seems a little better than invalidating
	//   the area and letting redraw do it but it probably does not help very
	//   much since we are almost certainly going to be redrawing the window
	//   for other reasons!

	height = tl->fmt.f.leading + tl->fmt.f.ascent + tl->fmt.f.descent;
	if ((width = height / 10) < 1)
		width = 1;
	xwsfillrectns(win, tb->xpos + cpd->xpos - width / 2, tb->ypos + tl->ypos,
			width, height, 0x000000);

	drawypos = tb->ypos + tl->ypos;
	drawheight = height;


	if (settar)
		win->cartarget = win->caret.xpos;

	return (TRUE);
}


//***********************************************************//
// Function: xwstextsetsta - Set up new selected text area   //
// Returned: TRUE if highlight area changed, FALSE otherwise //
//***********************************************************//

int XOSFNC xwstextsetsta(
	XWSWIN *win,		// Address of WIN
	STA    *cpd,		// Address of STA which specifies positon
	int     settar)
{
	// This appears to be complex but is really very simple. All we need to
	//   do here is invalidate the area been the old and new caret positions
	//   and store the new caret position. Everything else needed to change
	//   the lighlighted selected area is done in textdsp.

	CHECKSTA(win);

	if (win->caret.xpos == cpd->xpos && // Are we moving the caret?
			win->caret.tl == cpd->tl)
		return(FALSE);					// No - nothing needed here

	// Here if really moving the caret

	xwstextinvalidatecaret(win);		// Always invalidate the current caret
	xwstextinvalidatesta(win, cpd, &win->caret);
	win->caret = *cpd;					// Store the new caret values

	CHECKSTA(win);

	xwstextinvalidatecaret(win);		// Invalidate the new caret
	if (settar)
		win->cartarget = cpd->xpos;
	return (TRUE);
}

long invalypos;
long invalheight;


//*********************************************************//
// Function: xwstextinvalidatecaret - Invalidate the caret //
// Returned: Nothing                                       //
//*********************************************************//

void XOSFNC xwstextinvalidatecaret(
	XWSWIN *win)
{
	TL  *tl;
	TB  *tb;
	long height;
	long width;

	CHECKSTA(win);

	if ((tl = win->caret.tl) != NULL)
	{
		tb = tl->tb;
		height = tl->fmt.f.leading + tl->fmt.f.ascent + tl->fmt.f.descent;
		if ((width = height / 10) < 1)
			width = 1;
		xwswininvalidaterectns(win, tb->xpos + win->caret.xpos - width / 2,
			tb->ypos + tl->ypos, width, height);

		invalypos = tb->ypos + tl->ypos;
		invalheight = height;
	}
}


char *xxxdpnt;
long  xxxcinx;
long  xxxcnum;
long  xxxlength;


//********************************************************************//
// Function: xwstextgetcharpos - Determine horizontal pixel position, //
//             character number, and character index in a text line   //
//             given the character number or the horizontal position  //
// Returned: Nothing                                                  //
//********************************************************************//

// Both cnum and xpos can be specified and the first one that is found,
//   searching left to right is returned. This is normally not useful, in
//   virtually all cases one or the other will be very large so it will have
//   no affect on finding the other.

// All STA values are filled in by this function:
//   tl   = Address of the TL
//   cinx = Index of character at position specified - If position is between
//            characters, it is the index of the following character. If past
//            the end of line it is the index of the final null or LF.
//   cnum = Number of character at position specified - If position if between
//            characters, it is the number of the following character. If past
//            the end of line, it is equal to the number of characters in the
//            line.
//   xpos = X position (pixels) in TB of the character specified - This is
//            calculated even when a position is specified. It is always the
//            base (left side) position of the character unless the specified
//			  character/position is past the end of the line in which case it
//            is the position after the last character.
// There is no error return. It is assumed that the caller has determined that
//   the position is within the TL specified.

void XOSFNC xwstextgetcharpos(
	XWSWIN *win,		// Address of the WIN
	TL     *tl,			// Address of TL for line
	long    cnum,		// Character number (-1 if not specified)
	long    xpos,		// X position (-1 if not specified)
	STA    *cpd)		// Address of the STA
{
	FNTVAL reqf;
	char  *dpnt;
	long   value;
	long   length;
	int    size;
	int    dcnt;
	int    ccnt;
	int    func;
	int    chr;
	char   chkfont;

	CHECKSTA(win);

	xpos -= (tl->tb->xpos + tl->xpos);
	cpd->tl = tl;
	cpd->lnum = tl->lnum;

	// See if we are beyond the end of the line

	if (cnum >= tl->numchar && xpos >= tl->fmt.f.length)
	{
		cpd->cinx = (tl->text[tl->count - 1] == '\n') ? (tl->count - 1) :
				tl->count;
		cpd->cnum = tl->numchar;
		cpd->xpos = tl->fmt.f.length + tl->fmt.f.leftmargin;
		return;
	}

	// Not beyond end of line. It would be nice to check for the beginning 
	//   of the line here but we can't do that since we need to account for
	//   possible leading formating functions. We always want to return the
	//   index for a character, not a possible preceding function.

	reqf = tl->fmt.i.font;
	chkfont = TRUE;
	dcnt = tl->count;
	dpnt = tl->text;
	ccnt = 0;
	length = 0;
	while (dcnt > 0)
	{
		if ((chr = dpnt[0]) == TF_PREFIX)
		{
			// Here if have a function prefix

			if (dcnt < 2) // Have another character?
				xwsFail(0, "Incomplete format function in stored text");
			func = dpnt[1];
			if (func < TF_1BYTE)		// If have no added bytes (we don't
			{							//   case about any of these here
				dpnt += 2;
				dcnt -= 2;
			}
			else if (func < TF_2BYTE)	// If have one added byte
			{
				if (dcnt < 3)			// Have enought characters?
					xwsFail(0, "Incomplete format function in stored text");
				switch ((int)dpnt[2])
				{
				 case TF_FONT:			// Set character font
					if (value < 1 || value > win->edb->numfonts)
						value = 1;
					reqf.num = value;
					goto com1byte;

				 case TF_SETFATTR:		// Set font attributes
					reqf.attr |= value;
					goto com1byte;

				 case TF_CLRFATTR:		// Clear font attr
					reqf.attr &= ~value;
					goto com1byte;

				 case TF_FWEIGHT:		// Set font weight
					reqf.weight = value;
				 com1byte:
					chkfont = TRUE;
					break;
				}
				dpnt += 3;
				dcnt -= 3;
			}
			else if (func < TF_4BYTE)	// If have 2 added bytes
			{
				if (dcnt < 4)	 		// Have enought characters?
					xwsFail(0, "Incomplete format function in "
							"text");
				value = *(ushort *)(dpnt + 2);
				switch (func)
				{
			 	 case TF_FHEIGHTP:		// Set char height
					reqf.height = value;
					goto com4byte;

				 case TF_FWIDTHP:		// Set char width
					reqf.width = value;
				 com4byte:
					chkfont = TRUE;
					break;

				 case TF_HORPOSP:		// Set horizontal position
					length = value;
					break;
				}
				dpnt += 4;
				dcnt -= 4;
			}
			else if (func < TFX_XBYTE)	// If have 4 added bytes
			{
				if (dcnt < 6)			// Have enought characters?
					xwsFail(0, "Incomplete format function in text");
				dpnt += 6;				// We don't care about any of these
				dcnt -= 6;
			}
			else						// If have special internal function
			{
				if (dcnt < 14)			// Have enought characters?
					xwsFail(0, "Incomplete format function in text");
				dpnt += 14;				// We don't care about any of these
				dcnt -= 14;
			}

			// End of function processing
		}
		else if (chr == '\n')
			break;
		else							// If have a character to add in to
		{								//   the pixel width
			if (ccnt >= cnum)
				break;
			if (chkfont)				// Might the font have changed?
			{
				xwssetupfont(win, &reqf); // Yes - set it up if necessary
				chkfont = FALSE;
			}
			size = xwsfontgetchrwidthns(xwscurfnt, dpnt, dcnt, 0, &value);
			if ((length + value / 2) > xpos)
				break;
			length += value;
			ccnt += size;
			dpnt += size;
			dcnt -= size;
		}
	}

	// Finished - fill in his STA

	xxxdpnt = dpnt;
	xxxcinx = cpd->cinx = dpnt - tl->text;
	xxxcnum = cpd->cnum = ccnt;
	xxxlength = cpd->xpos = length + tl->fmt.f.leftmargin;
}


//********************************************************************//
// Function: xwstextinvalidatesta - Invalidate the selected text area //
// Returned: Nothing                                                  //
//********************************************************************//

// We go to quite a bit of trouble to only invalidate the absolute minimum
//   area necessary. This is needed to minimize flicker when updating the
//   caret or the selected area.

// Since this function may be used to invalidate only part of the current
//   selected area when it is being changed, it is necessary to explicitly
//   pass the beginning and end of the area being invalidated. This cannot
//   be taken from the XWSWIN.

// The sta1 and sta2 arguments may be in either order (sta1 is before sta2 or
//   sta1 is after sta2)

void XOSFNC xwstextinvalidatesta(
	XWSWIN *win,
	STA    *sta1,
	STA    *sta2)
{
	TB  *tb;
	TL  *tl;
	STA *bgn;
	STA *end;
	long xpos;

	CHECKSTA(win);

	// Determine which STA is first

	if (sta1->lnum < sta2->lnum ||
			(sta1->lnum == sta2->lnum && sta1->xpos <= sta2->xpos))
	{
		bgn = sta1;
		end = sta2;
	}
	else
	{
		bgn = sta2;
		end = sta1;
	}

	// Invalidate the selected part of the first line in the selected
	//   area

	tl = bgn->tl;
	tb = tl->tb;
	xpos = (tl != end->tl) ? (tl->fmt.f.leftmargin + tl->fmt.f.length) :
			end->xpos;
	xwswininvalidaterectns(win, tb->xpos + bgn->xpos, tb->ypos + tl->ypos,
			xpos - bgn->xpos, tl->fmt.f.leading + tl->fmt.f.ascent +
			tl->fmt.f.descent + tl->fmt.f.lspace);

	if (tl == end->tl)					// Finished if only one line
		return;

	// Invalidate the completely selected lines

	while ((tl = tl->next) != end->tl)
	{
		if (tl == NULL)
			xwsFail(0, "Bad selected text area");
		xwswininvalidaterectns(win, tb->xpos + tl->xpos, tb->ypos + tl->ypos,
				tl->fmt.f.length, tl->fmt.f.leading + tl->fmt.f.ascent +
				tl->fmt.f.descent + tl->fmt.f.lspace);
	}

	// Invalidate the selected part of the last line in the selected area

	xwswininvalidaterectns(win, tb->xpos + tl->xpos, tb->ypos + tl->ypos,
			end->xpos - tl->xpos, tl->fmt.f.leading + tl->fmt.f.ascent +
			tl->fmt.f.descent + tl->fmt.f.lspace);
}
