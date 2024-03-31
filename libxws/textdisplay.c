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

// The size of a font is described by a number of values (all values are
//   positive):
//        Font height: The height of the "EM" box if scalable font or the
//                       height of the bitmap if bitmapped font
//         Font width: The width of the "EM" box if scalable font or the width
//                       of the bitmap if bitmapped font
//             Ascent: Amount "normal" characters may extend above the baseline
//            Descent: Amount "normal" characters may extend below the baseline
//              Extra: Amount that may be used above "normal" characters for
//                       various diacritical marks (If 0, this space is included
//                       in the Ascent value.)
//          Line size: Standard spacing between baselines of two successive
//                       lines of text

// Text which is automatically displayed in a window is contained in a "text
//   box" which contains one or more "text lines". A text box is identified by
//   an index number. Each text box in a window must have a unique index
//   number. Text boxes are drawn in order by index number, so if there is
//   overlap, the text box with the higher index number will be "on top".
//   If a text box is created with the same index as an existing text box,
//   the existing text box is deleted.

// Text is clipped to the boundries of the text box. Initially the left margin
//   for text lines is the left edge of the text box and the right margin is
//   the right edge. The first line is positioned so that the top of its
//   bounding top is at the top of the text box.

// If the width of the text box is specified as 0, the width will be the minimum
//   width needed to include the longest line. If the height is specified as 0,
//   the heigth will be the minimum needed to include all lines. If the width
//   is 0, word wrap is ignored if specified. If the height is specified as 0,
//   specifying vertical centering has no effect.

// Three text colors can be specified, a normal, an alternate, and a highlight
//   color. The normal color is used as the initial text color when the text is
//   not highlighted and the window is not in the alternate state (this is
//   mainly used as the pressed state for buttons). The alternate color is used
//   as the the initial text color when the window is in the alternate state
//   and the text is not highlighted. The highlight color is used whenever text
//   is high-lighted. A text color specified with a formating function always
//   overrides the normal and alternate colors. It does not override the
//   highlight color.


typedef struct
{	char  *dpnt;
	long   dcnt;
	char  *bpnt;
	long   bgcol;		// Background color
	long   txtcol;		// Default text color
	long   bghlcol;
	long   stabgnxpos;	// X position of beginning of selected area
	long   staendxpos;	// X position of end of selected area (actually first
						//   position after selected area)
	long   stabgncinx;	// Character index for first character in selected area
	long   staendcinx;	// Character index for first character past the
						//   selected area
	long   left;
	long   length;
	long   xpos;		// X position of the text box
	long   ypos;		// Y position of the text box
	long   lxpos;
	FMT    fmt;
	FNTVAL reqf;
} DD;


static void XOSFNC dodisplay(XWSWIN *win, TL *tl, DD *dd);
static void XOSFNC fillcbg(XWSWIN *win, BGC *bgc, TL *tl, DD *dd);

long drawdheight;

//***************************************************************
// Function: xwsWinDispText - Display all text boxes for a window
// Returned: Nothing
//***************************************************************

// This function should normally be called from the window's redraw function

void XOSFNC xwsWinDispText(
	XWSWIN *win)
{
	TB     *tb;
	TL     *tl;
	XWSRGN *rgn;
	XWSRGN *clipsv;
	STA    *fsta;
	STA    *lsta;
	DD      dd;
	long    value;
	long    lineheight;
	long    lineypos;
	long    top;
	long    bottom;
	int     width;
	int     height;
	int     chr;
	int     func;
	int     sfnc;

	tb = win->tbhead;
	while (tb != NULL)
	{
		/// SHOULD WE SEE IF IN WINDOW HERE ????

		rgn = xwsrgncreaterectns(tb->xpos + win->xorg, tb->ypos + win->yorg,
				tb->xsize, tb->ysize); // Create a region containing only the
									   //   text box
		clipsv = xwsWinReduceClipRgn(win, rgn); // Reduce the clipping region
												//   to only contain the TB
		if (win->cliprgn->type != RGNTYPE_NULL) // Is the clipping region now
												//   null?
		{
			// Here if the clipping region for the TB is not NULL - We need
			//   to draw at least some of the TB. First fill in the area
			//   above the text if the background is not transparent

			dd.bgcol = ((win->config & WCB_LIST) &&
					tb == ((XWSLIST *)win)->selitem) ?
					((win == xwsfocuswin) ? tb->bgdcolorfcs : tb->bgdcolorsel) :
					tb->bgdcolor;
			top = tb->tlhead->ypos;
			if (top > 0 && tb->bgdcolor != XWS_COLOR_TRANS)
				xwsfillrectns(win, tb->xpos, tb->ypos, tb->xsize, top,
						dd.bgcol);

			// Draw the text lines

			tl = tb->tlhead;
			do
			{
				//// SHOULD WE SEE IF IN CLIPPING REGION HERE ????

				dd.fmt = tl->fmt;
				dd.reqf = tl->fmt.i.font;
				dd.ypos = tb->ypos;
				dd.xpos = tb->xpos;
				if (win->status & WSB_ALTCOL)
				{
					dd.ypos += win->shift;
					dd.xpos += win->shift;
				}		

				dd.lxpos = dd.xpos + tl->xpos;

///				dd.lxpos = dd.xpos + tl->fmt.f.lmargin;

				// Fill the background as necessary. If none of the line is
				//   highlighted, it is filled as a single area with the text
				//   box background color overlayed with any necessary text
				//   backgrounds.
				// If any or all of the line is highlighed there are 5
				//   possible fill areas in a line:
				//   1. Before the left margin - Always filled with the text
				//      box background color.
				//   2. Left margin to start of highlight - Filled with the
				//      text box background color overlayed with any text
				//      background areas.
				//   3. The highlight area - Filled with the highlight color.
				//   4. From the highlight area to the right margin - Filled
				//      the same as area #2 above.
				//   5. After the right margin - Filled with the text box
				//      background color.
				// Any (or all) of these areas may be 0 length. The text box
				//   background color may be transparent. None of the other
				//   fill colors may be transparent.

				lineheight = tl->fmt.f.ysize;
				lineypos = dd.ypos + tl->ypos;
				
				if (win->caret.lnum < win->anchor.lnum ||
						(win->caret.lnum == win->anchor.lnum &&
						win->caret.xpos <= win->anchor.xpos))
				{
					fsta = &win->caret;
					lsta = &win->anchor;
				}
				else
				{
					fsta = &win->anchor;
					lsta = &win->caret;
				}

				CHECKSTA(win);

				if ((tl->lnum < fsta->lnum || tl->lnum > lsta->lnum) ||
						(fsta->tl == lsta->tl && fsta->xpos == lsta->xpos))
				{
					// Here if there is no selection in this line either
					//   because the line is not in the selection area or
					//   the selection area is 0 width.

					dd.stabgncinx = 0x7FFFFFFF; // Indicate no selected area
					if (dd.bgcol != XWS_COLOR_TRANS) // Do we need to fill the
													 //   background?
						xwsfillrectns(win, dd.xpos, lineypos, tb->xsize,
								lineheight, dd.bgcol);
					if (tl->fmt.i.bgnbgc.length != 0)
										// Do we have an initial character
										//   background area?
						fillcbg(win, &tl->fmt.i.bgnbgc, tl, &dd); // Yes
				}
				else
				{
					// Here if at least part of the line is selected. First
					//   determine the start of the selection area.

					if (tl == fsta->tl)
					{
						dd.stabgnxpos = tb->xpos + fsta->xpos;
						dd.stabgncinx = fsta->cinx;
					}
					else
					{
						dd.stabgnxpos = tb->xpos + tl->xpos;
						dd.stabgncinx = 0;
					}

					// Determine the end of the selection area.

					if (tl == lsta->tl)
					{
						dd.staendxpos = tb->xpos + lsta->xpos;
						dd.staendcinx = lsta->cinx;
					}
					else
					{
						dd.staendxpos = tb->xpos + tl->xpos + tl->fmt.f.length;
						dd.staendcinx = tl->numchar;
					}

					// Fill the initial non-transparent non-selected area,
					//   if any

					if (dd.stabgnxpos > 0 && dd.bgcol != XWS_COLOR_TRANS)
					{
						xwsfillrectns(win, dd.xpos, lineypos, dd.stabgnxpos,
								lineheight, dd.bgcol);

						// If there is an initial character background fill in
						//   as much as is in this area

						if (tl->fmt.i.bgnbgc.length != 0)
							fillcbg(win, &tl->fmt.i.bgnbgc, tl, &dd); // Yes
					}

					// Fill in the highlight area

					xwsfillrectns(win, dd.stabgnxpos, lineypos, dd.staendxpos -
							dd.stabgnxpos, lineheight, tb->bgdcolorsel);

					// Fill in the final non-highlight area, if any

					if (dd.bgcol != XWS_COLOR_TRANS &&
							dd.staendxpos < tb->xsize)
						xwsfillrectns(win, /* dd.xpos + */ dd.staendxpos, lineypos,
								tb->xsize - dd.staendxpos, lineheight,
								dd.bgcol);
				}

				// Here with all of the line's background areas filled in if
				//   necessary - now draw the text.

				dd.dcnt = tl->count;
				dd.dpnt = dd.bpnt = tl->text; // Intialize pointers to the line
											  //   buffer
				while (dd.dcnt > 0)
				{
					if ((chr = dd.dpnt[0]) == TF_PREFIX)
					{
						// Here if have a function prefix

						if (dd.dcnt < 2) // Have another character?
							xwsFail(0, "Incomplete format function in stored "
									"text");
						dodisplay(win, tl, &dd); // Display text up to this
												 //   point
						func = dd.dpnt[1];
						sfnc = func & 0xF0;
						if (func < TF_1BYTE) // Have no added bytes?
						{
							value = func & 0x0F;
							if (func == TF_CLRCOFS) // Clear character offset?
								dd.fmt.i.offset = 0;

							dd.dpnt += 2;
							dd.dcnt -= 2;
						}
						else if (func < TF_2BYTE) // Have one added byte?
						{
							if (dd.dcnt < 3) // Have enought characters?
								xwsFail(0, "Incomplete format function in "
										"stored text");
							switch ((int)dd.dpnt[2])
							{
							 case TF_FONT: // Set character font?
								if (value < 1 || value > win->edb->numfonts)
									value = 1;
								dd.reqf.num = value;
								break;

							 case TF_SETFATTR: // Set font attributes
								dd.reqf.attr |= value;
								break;

							 case TF_CLRFATTR: // Clear font attr
								dd.reqf.attr &= ~value;
								break;

							 case TF_FWEIGHT: // Set font weight
								dd.reqf.weight = value;
							}
							dd.dpnt += 3;
							dd.dcnt -= 3;
						}
						else if (func < TF_4BYTE) // If have 2 added bytes
						{
							if (dd.dcnt < 4) // Have enought characters?
								xwsFail(0, "Incomplete format function in "
										"text");
							value = *(ushort *)(dd.dpnt + 2);
							switch (func)
							{
						 	 case TF_FHEIGHTP: // Set char height?
								dd.reqf.height = value;
								break;

							 case TF_FWIDTHP: // Set char width?
								dd.reqf.width = value;
								break;

							 case TF_HORPOSP: // Set horizontal position?
								dd.length = value;
								dd.lxpos = dd.xpos + value;
								break;

							 case TF_SETCOFSP: // Set char offset?
								dd.fmt.i.offset = value;
								break;
							}
							dd.dpnt += 4;
							dd.dcnt -= 4;
						}
						else if (func < TFX_XBYTE) // If have 4 added bytes
						{
							if (dd.dcnt < 6) // Have enought characters?
								xwsFail(0, "Incomplete format function in "
										"text");

							value = *(long *)(dd.dpnt + 2);
							switch (func)
							{
							 case TFX_FGCOL: // Set foreground color?
								dd.fmt.i.txtcolor = value;
								break;
							}
							dd.dpnt += 6;
							dd.dcnt -= 6;
						}
						else					// If have special internal
						{						//   function
							switch (func)
							{
							 case TFX_BGCOL:
								fillcbg(win, (BGC *)(dd.dpnt + 2), tl, &dd);
								break;
							}
							dd.dpnt += 14;
							dd.dcnt -= 14;
						}
						dd.bpnt = dd.dpnt; // Start drawing after this function

						// End of function processing
					}
					else if (chr == '\n')
					{
						dodisplay(win, tl, &dd);
						dd.bpnt = dd.dpnt;
						break;
					}
					else
					{
						dd.dpnt++;					// Consume the character
						dd.dcnt--;
					}
				}
				dodisplay(win, tl, &dd);
			} while ((tl = tl->next) != NULL);

			// Draw the area below the text if the TB background is not
			//   transparent

			tl = tb->tltail;
			if (dd.bgcol != XWS_COLOR_TRANS && ((bottom = tl->ypos +
					tl->fmt.f.ysize)) < tb->ysize)
				xwsfillrectns(win, tb->xpos, tb->ypos + bottom, tb->xsize,
						tb->ysize - bottom, dd.bgcol);

			// Draw the caret if necessary

			if (win == xwsfocuswin && (win->config & WCB_CARET) &&
					(tl = win->caret.tl) != NULL && tl->tb == tb)
			{							// Is the caret in this TB?
				height = tl->fmt.f.leading + tl->fmt.f.ascent +
						tl->fmt.f.descent;
				if ((width = height / 10) < 1)
					width = 1;
				xwsfillrectns(win, tb->xpos + win->caret.xpos - width / 2,
						tb->ypos + tl->ypos, width, height, 0x000000);

				drawdheight = height;
			}
		}
		xwsWinChangeClipRgn(win, clipsv); // Restore the previous clipping
										  //   region
		xwsRgnDestroy(rgn);				// Give up the region we created
		tb = tb->next;
	}
}


//*****************************************************
// Function: fillcbg - Fill a character background area
// Returned: Nothing
//*****************************************************

static void XOSFNC fillcbg(
	XWSWIN *win,
	BGC    *bgc,
	TL     *tl,
	DD     *dd)
{
	long top;
	long height;

	top = dd->ypos + tl->ypos + tl->fmt.f.leading + tl->fmt.f.ascent -
			bgc->ascent;
	height = bgc->ascent + bgc->descent;
	xwsfillrectns(win, dd->xpos + tl->xpos + bgc->xpos, top, bgc->length,
			height, bgc->color);
}


//*********************************************
// Function: dodisplay - Display a text segment
// REturned: ????
//*********************************************

// A text segment is always completely within or without a text background
//   area. It may, however, cross into or out of (or both) a selected area.

// NOTE: This assumes that a button or list window will never have selected
//       text.

long llll;

static void XOSFNC dodisplay(
	XWSWIN *win,
	TL     *tl,
	DD     *dd)
{
	TB  *tb;
	long ybase;
	long txtcolor;
	int  amnt;
	int  cnt;

	if ((cnt = dd->dpnt - dd->bpnt) <= 0)
		return;
	xwssetupfont(win, &dd->reqf);
	tb = tl->tb;

	ybase = dd->ypos+tl->ypos+tl->fmt.f.ascent + tl->fmt.f.leading -
			dd->fmt.i.offset;
	txtcolor = (((win->config & WCB_LIST) && tb == win->selitem) ||
			(win->status & WSB_ALTCOL)) ? tb->txtcolorsel : dd->fmt.i.txtcolor;

	llll = dd->lxpos;

	if (dd->stabgncinx < cnt)			// Is at least part of this segment
										//   selected?
	{									// Yes
		// Draw any text preceeding the selected text in the normal text color

		if (dd->stabgncinx > 0)
		{
			dd->lxpos = xwsDrawText(win, dd->bpnt, dd->stabgncinx |
						0x80000000, dd->lxpos, ybase, xwscurfnt, txtcolor);
			dd->bpnt += dd->stabgncinx;
			if ((cnt -= dd->stabgncinx) <= 0)
				return;
		}

		// Draw the selected area in the selection text color

		if ((amnt = dd->staendcinx - dd->stabgncinx) > cnt)
			amnt = cnt;
		dd->lxpos = xwsDrawText(win, dd->bpnt, amnt | 0x80000000, dd->lxpos,
				ybase, xwscurfnt, tb->txtcolorsel);
		dd->bpnt += amnt;
		if ((cnt -= amnt) <= 0)
			return;

		// Draw any text following the selected text in the normal text color

///		dd->lxpos = xwsDrawText(win, dd->bpnt, cnt | 0x80000000, dd->lxpos,
///				dd->ypos, xwscurfnt, dd->txtcol);
	}
	dd->lxpos = xwsDrawText(win, dd->bpnt, cnt | 0x80000000, dd->lxpos, ybase,
			xwscurfnt, txtcolor);
}
