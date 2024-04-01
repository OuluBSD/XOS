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

#include <xxws.h>


//***************************************************************//
// Function: xwsWinMove - Move and resize a window with scalling //
// Returned: Nothing                                             //
//***************************************************************//

void XOSFNC xwsWinMove(
	XWSWIN *win,
	long    xpos,
	long    ypos,
	long    width,
	long    height)
{
	xwswinmovens(win, SCALE(win->xsfr, win->xsfd, xpos),
			SCALE(win->ysfr, win->ysfd, ypos),
			SCALE(win->xsfr, win->xsfd, width),
			SCALE(win->ysfr, win->ysfd, height);
}


//********************************************************************//
// Function: xwswinmovens - Move and resize a window without scalling //
// Returned: Nothing                                                  //
//********************************************************************//

// The goal of this routine is to move a window and all of its child windows
//   on the screen with an absolute minimum of drawing and with as few
//   region operations as possible.  We copy (BITBLT) all of the window and
//   its children that is currently visible.  Unfortunately, this is somewhat
//   complex!  Normally, child windows are excluded from their parent's mask
//   region.  This means that if the window has children, we must generate a
//   new move source region from scratch, without excluding this window's child
//   windows.  We do that here and save it in the WIN for use by the remask
//   routine, which does the actual copy operation.  We will also need a move
//   destination region with is the mask for the window in its new location
//   without its children excluded.  This is generated by the remask routine.
//   This is relatively easy, since we can do this without any extra region
//   operations as we regenerate the window's final mask.  The whole thing is
//   futher complicated by the problem of having to move more than one window
//   at a time.  We handle this by checking for additional pending moves each
//   time we move a window in the remask routine and exclude any area we copied
//   to from the source areas for the additional windows to move.

// When considering what this function does, remember that no changes to the
//   displayed screen image are made here. All changes to the image are made
//   in the draw thread after this function completes. This function updates
//   all window parameters and regions. This includes creating special
//   source and destination move regions. This function can be called multiple
//   times (for the same or different windows) before the draw thread executes
//   without causing unnecessary invalidations.

void XOSFNC xwswinmovens(
	XWSWIN *win,
	long    xpos,
	long    ypos,
	long    width,
	long    height)
{
	XWSEDBLK edblk;
	XWSRGN  *oldmaskrgn;
	XWSRGN   rightrgn;
	XWSRGN   bottomrgn;
	long     oldxpos;
	long     oldypos;
	long     oldwidth;
	long     oldheight;
	char     resize;
	char     moving;

	rightrgn = bottomrgn = NULL;
	if (width <= 0)
		width += win->parent->clntwidth - win->xpos;
	if (height <= 0)
		height += win->parent->cintheight - win->ypos;

	// Make sure new size is big enough for the borders

	if (width < ((win->vbwidth + 1) << 1))
		width = (win->vbwidth + 1) << 1;
	if (height < ((win->hbheight + 1) << 1))
		height = (win->hbheight + 1) << 1;

	// Save the current window position and size

	oldxpos = win->scnxpos;
	oldypos = win->scnypos;
	oldwidth = win->width;
	oldheight = win->height;
	resize = (oldwidth != width || oldheight != height);
	moving = (oldxpos != xpos || oldypos != ypos);

	// If we are both moving and resizing the window we want to do this in an
	//   order than minimizes unnecessary invalidation. If the size is
	//   decreasing we want to resize first and then move. If the size is
	//   increasing we want to move and then resize. This is simple if both
	//   the width and height are increasing or decreasng but not if one is
	//   increasing and the other descreasing. In this case we do this in
	//   three steps: 1) Resize the dimension that is decreasing, 2) Move the
	//   window, 3) Resize the dimension that is increasing. This case is
	//   encountered often, especially when a window is being resized for the
	//   top or left side.


	MOVB	mw_resize[EBP], #0	;Assume not changing the size
	CMPL	EAX, mw_width[EBP]
	JNE	6$
	CMPL	EDX, mw_height[EBP]
	JE	8$
6$:	MOVB	mw_resize[EBP], #1
8$:	MOVL	ECX, win_basdrw[EDI]	
	MOVB	mw_moving[EBP], #0	;Assume not moving the window
	MOVL	EAX, mw_xpos[EBP]	;Really moving it?
	MOVL	EDX, mw_ypos[EBP]
	SUBL	EAX, win_basxpos[EDI]
	JNE	10$			;Yes
	CMPL	EDX, win_basypos[EDI]	;Maybe
	JNE	10$			;Yes


	CMPB	mw_resize[EBP], #0	;No - changing the window size?
	JNE	nomove			;Yes - go on
	JMP	movedone		;No - nothing to do here

	// Here if moving the window - first scan through the child windows and
	//   update their positions

10$:	SUBL	EDX, win_basypos[EDI]	;Get difference in position
	MOVB	mw_moving[EBP], #1
	MOVL	ESI, win_basfchild[EDI]
	TESTL	ESI, ESI
	JE	14$			;If no children
	CALL	updatechildpos		;Scan through the child windows and
					;  update their positions
14$:	MOVL	EAX, mw_xpos[EBP]	;Update the window position
	MOVL	win_basxpos[EDI], EAX
	MOVL	EDX, mw_ypos[EBP]
	MOVL	win_basypos[EDI], EDX
	MOVL	EBX, win_basparent[EDI]
	TESTL	EBX, EBX
	JE	16$
	ADDL	EAX, win_basscnxpos[EBX]
	ADDL	EDX, win_basscnypos[EBX]
16$:	MOVL	win_basscnxpos[EDI], EAX
	MOVL	win_basscnypos[EDI], EDX
	IMULL	EDX, drw_pixelspan[ECX] ;Update the window buffer offset
	ADDL	EDX, EAX
	IMULL	EDX, drw_pixelsize[ECX]
	ADDL	EDX, drw_buffer[ECX]
	MOVL	win_basoffset[EDI], EDX
					;Fall into code on next page
.PAGE
;Branch to here if not moving the window

nomove:	CMPB	mw_resize[EBP], #0	;Are we changing the window size?
	JE	nosize			;No
	MOVL	EAX, mw_width[EBP]	;Yes - update the window size
	MOVL	win_baswidth[EDI], EAX
	SUBL	EAX, win_basleftwidth[EDI]
	SUBL	EAX, win_basrightwidth[EDI]
	MOVL	win_basclntwidth[EDI], EAX
	MOVL	EAX, mw_height[EBP]
	MOVL	win_basheight[EDI], EAX
	SUBL	EAX, win_bastopheight[EDI]
	SUBL	EAX, win_basbtmheight[EDI]
	MOVL	win_basclntheight[EDI], EAX
	PUSHL	win_basbasergn[EDI]
	CALL	xwsRgnDestroy#
	PUSHL	#0
	PUSHL	#0
	PUSHL	win_baswidth[EDI]
	PUSHL	win_basheight[EDI]
	CALL	xwsrgncreaterectns#
	MOVL	win_basbasergn[EDI], EAX
	PUSHL	win_basclientrgn[EDI]
	CALL	xwsRgnDestroy#
	PUSHL	win_basleftwidth[EDI]	;Create the client region
	PUSHL	win_bastopheight[EDI]
	PUSHL	win_basclntwidth[EDI]
	PUSHL	win_basclntheight[EDI]
	CALL	xwsrgncreaterectns#
	MOVL	win_basclientrgn[EDI], EAX
	MOVL	EAX, mw_width[EBP]	;Changing the width?
	SUBL	EAX, mw_oldwidth[EBP]
	JE	6$			;No
	JG	2$
	CLRL	EAX
2$:	MOVL	EDX, win_baswidth[EDI]      ;Yes - generate a region for the
	SUBL	EDX, win_basrightwidth[EDI] ;  right border
	SUBL	EDX, EAX
	PUSHL	EDX
	PUSHL	#0
	ADDL	EAX, win_basrightwidth[EDI]
	PUSHL	EAX
	PUSHL	win_basheight[EDI]
	CALL	xwsrgncreaterectns#
	MOVL	EDX, win_basscnxpos[EDI]
	ADDL	EDX, win_baswidth[EDI]
	SUBL	EDX, mw_oldxpos[EBP]
	SUBL	EDX, mw_oldwidth[EBP]
	JNE	3$
	PUSHL	EAX
	PUSHL	EAX
	CALL	xwsRgnDuplicate#
	MOVL	mw_rightrgn[EBP], EAX
	POPL	EAX
3$:	CMPL	win_basdrawrgn[EDI], #0
	JNE	4$
	MOVL	win_basdrawrgn[EDI], EAX
	JMP	6$

4$:	PUSHL	EAX
	PUSHL	EAX
	PUSHL	win_basdrawrgn[EDI]
	CALL	xwsRgnUnion#
	PUSHL	EAX
	PUSHL	win_basdrawrgn[EDI]
	CALL	xwsRgnDestroy#
	POPL	win_basdrawrgn[EDI]
	CALL	xwsRgnDestroy#
6$:	MOVL	EAX, mw_height[EBP]	;Changing the height?
	SUBL	EAX, mw_oldheight[EBP]
	JE	nosize			;No
	JG	8$
	CLRL	EAX
8$:	PUSHL	#0
	MOVL	EDX, win_basheight[EDI]    ;Yes - generate a region for the
	SUBL	EDX, win_basbtmheight[EDI] ;  bottom border
	SUBL	EDX, EAX
	PUSHL	EDX
	PUSHL	win_baswidth[EDI]
	ADDL	EAX, win_basbtmheight[EDI]
	PUSHL	EAX
	CALL	xwsrgncreaterectns#
	MOVL	EDX, win_basscnypos[EDI]
	ADDL	EDX, win_basheight[EDI]
	SUBL	EDX, mw_oldypos[EBP]
	SUBL	EDX, mw_oldheight[EBP]
	JNE	9$
	PUSHL	EAX
	PUSHL	EAX
	CALL	xwsRgnDuplicate#
	MOVL	mw_bottomrgn[EBP], EAX
	POPL	EAX
9$:	CMPL	win_basdrawrgn[EDI], #0
	JNE	10$
	MOVL	win_basdrawrgn[EDI], EAX
	JMP	nosize

10$:	PUSHL	EAX
	PUSHL	EAX
	PUSHL	win_basdrawrgn[EDI]
	CALL	xwsRgnUnion#
	PUSHL	EAX
	PUSHL	win_basdrawrgn[EDI]
	CALL	xwsRgnDestroy#
	POPL	win_basdrawrgn[EDI]
	CALL	xwsRgnDestroy#
.PAGE
;Branch to here if not changing the window size

nosize:	MOVB	AL, #1
	CALL	remaskwindow		;Remask this window
	MOVL	mw_oldmaskrgn[EBP], EAX ;Save the original mask region for now
	CMPL	mw_rightrgn[EBP], #0
	JE	102$
	PUSHL	mw_oldmaskrgn[EBP]
	PUSHL	mw_rightrgn[EBP]
	CALL	xwsRgnDiff#
	PUSHL	EAX
	PUSHL	mw_oldmaskrgn[EBP]
	CALL	xwsRgnDestroy#
	POPL	mw_oldmaskrgn[EBP]
	PUSHL	mw_rightrgn[EBP]
	CALL	xwsRgnDestroy#

102$:	CMPL	mw_bottomrgn[EBP], #0
	JE	104$
	PUSHL	mw_oldmaskrgn[EBP]
	PUSHL	mw_bottomrgn[EBP]
	CALL	xwsRgnDiff#
	PUSHL	EAX
	PUSHL	mw_oldmaskrgn[EBP]
	CALL	xwsRgnDestroy#
	POPL	mw_oldmaskrgn[EBP]
	PUSHL	mw_bottomrgn[EBP]
	CALL	xwsRgnDestroy#

104$:	CMPB	mw_moving[EBP], #0	;Are we moving the window?
	JE	4$			;No
	MOVL	ESI, win_basfchild[EDI] ;Yes - get first child window
	TESTL	ESI, ESI
	JE	4$

;Here if moving the window and have at least one child window - scan through
;  the child windows and include their mask regions in the saved mask region

	CALL	includechilds

;Now remask all of this window's children

4$:	MOVL	ESI, EDI
	MOVL	EDI, win_basfchild[ESI]
	TESTL	EDI, EDI
	JE	8$
	CALL	xwsremaskallchilds
8$:	MOVL	EDI, win_basparent[ESI] ;Remask the parent window
	MOVB	AL, #1
	CALL	remaskwindow
	PUSHL	EAX			;Give up the old mask region
	CALL	xwsRgnDestroy#

;Now we find and remask all windows whose covering might be changed because
;  this window is modifed. This is all windows that were overlapped by this
;  window at its previous position plus all windows that will be overlapped
;  by this window at its new position
;	c{EDI} = Offset of WIN for parent window
;	c{ESI} = Offset of WIN for window being moved

	MOVL	EDI, win_basfchild[EDI]
	CMPL	EDI, ESI
	JE	10$
	CALL	remasksibling

;Here with the parent and all necessary sibling windows remasked. Now we
;  move this window if that is necessary.

10$:	MOVL	EDI, ESI
	CMPB	mw_moving[EBP], #0	;Are we moving the window?
	JE	20$			;No
	PUSHL	win_basmaskrgn[EDI]	;Yes
	CALL	xwsRgnDuplicate#
	MOVL	ESI, EAX
	CMPL	win_basfchild[EDI], #0 ;Have any children?
	JE	16$
	MOVL	EBX, EDI
	PUSHL	EDI
	MOVL	EDI, win_basfchild[EDI] ;Get first child
	CALL	mergechilds
	POPL	EDI
16$:	PUSHL	ESI		 	;Get intersection of the old and new
	PUSHL	mw_oldmaskrgn[EBP]	;  mask regions
	CALL	xwsRgnIntersect#

	CHECKRGN EAX

	PUSHL	EAX			;Stack this for call to xwsRgnDestroy
	MOVL	ECX, win_basdrw[EDI]
	PUSHL	ECX			;Source DRW
	PUSHL	ECX			;Destination DRW
	PUSHL	EAX			;RGN
	PUSHL	mw_oldxpos[EBP]		;Source X position
	PUSHL	mw_oldypos[EBP]		;Source Y position
	PUSHL	win_basscnxpos[EDI]	;Destination X position
	PUSHL	win_basscnypos[EDI]	;Destination Y position
	MOVL	EAX, drw_funcdsp[ECX]
	CALLI	gfx_rgncopysame[EAX]	;Copy the region
	CALL	xwsRgnDestroy#		;Give up the intersection we generated
	PUSHL	ESI			;  here
	CALL	xwsRgnDestroy#

	MOVL	EDX, win_basdrw[EDI]
	MOVL	EDX, drw_funcdsp[EDX]
	CALLI	gfx_waitidle[EDX]
20$:	CMPB	mw_resize[EBP], #0
	JE	22$
	MOVW	mw_edblk+edblk_length[EBP], #edblk_SIZE
	MOVB	mw_edblk+edblk_type[EBP], #XWS_EVENT_RESIZE
	MOVB	mw_edblk+edblk_subtype[EBP], #0
	MOVL	mw_edblk+edblk_orgwin[EBP], EDI
	LEAL	EAX, mw_edblk[EBP]
	PUSHL	EAX
	CALL	xwsdoevent#
22$:	CMPB	mw_moving[EBP], #0
	JE	24$
	MOVW	mw_edblk+edblk_length[EBP], #edblk_SIZE
	MOVB	mw_edblk+edblk_type[EBP], #XWS_EVENT_MOVE
	MOVB	mw_edblk+edblk_subtype[EBP], #0
	MOVL	mw_edblk+edblk_orgwin[EBP], EDI
	LEAL	EAX, mw_edblk[EBP]
	PUSHL	EAX
	CALL	xwsdoevent#
24$:	PUSHL	mw_oldmaskrgn[EBP]	;Give up the saved mask region
	CALL	xwsRgnDestroy#

	PUSHL	EDI
	CALL	xwsdodraw#		;Draw the window we just moved

	MOVL	EDI, win_basfchild[EDI]
	TESTL	EDI, EDI
	JE	movedone
26$:
	PUSHL	EDI
	CALL	xwsdodraw#

	MOVL	EDI, win_bassibling[EDI]
	TESTL	EDI, EDI
	JNE	26$
movedone:
	MOVL	EDI, mw_win[EBP]
	CMPL	EDI, xwscurwin#
	JE	28$
	MOVL	EDX, xws_ScreenDRW#+drw_funcdsp
	CALLI	gfx_shwcursor[EDX]
28$:	LEAVE
	POPL	EBX
	POPL	ESI
	POPL	EDI
	RET	20t
.PAGE
updatechildpos:
	ADDL	win_basscnxpos[ESI], EAX
	ADDL	win_basscnypos[ESI], EDX
	MOVL	EBX, win_basscnypos[ESI]
	IMULL	EBX, drw_pixelspan[ECX] ;Update the window buffer offset
	ADDL	EBX, win_basscnxpos[ESI]
	IMULL	EBX, drw_pixelsize[ECX]
	ADDL	EBX, drw_buffer[ECX]
	MOVL	win_basoffset[ESI], EBX
	CMPL	win_basfchild[ESI], #0
	JE	4$
	PUSHL	ESI
	MOVL	ESI, win_basfchild[ESI]
	CALL	updatechildpos
	POPL	ESI
4$:	MOVL	ESI, win_bassibling[ESI]
	TESTL	ESI, ESI
	JNE	updatechildpos
	RET
.PAGE
xwsremaskallchilds::
	MOVB	AL, #1
	CALL	remaskwindow
	PUSHL	EAX
	CALL	xwsRgnDestroy#
	CMPL	win_basfchild[EDI], #0
	JE	4$
	PUSHL	EDI
	MOVL	EDI, win_basfchild[EDI]
	CALL	xwsremaskallchilds
	POPL	EDI
4$:	MOVL	EDI, win_bassibling[EDI]
	TESTL	EDI, EDI
	JNE	xwsremaskallchilds
	RET
.PAGE
includechilds:
	MOVL	EAX, win_basscnxpos[ESI]
	SUBL	EAX, win_basscnxpos[EDI]
	MOVL	EDX, win_basscnypos[ESI]
	SUBL	EDX, win_basscnypos[EDI]
	PUSHL	win_basmaskrgn[ESI]
	PUSHL	EAX
	PUSHL	EDX
	CALL	xwsRgnDupOffset#
	PUSHL	EAX
	PUSHL	EAX
	PUSHL	mw_oldmaskrgn[EBP]
	CALL	xwsRgnUnion#
	PUSHL	EAX
	PUSHL	mw_oldmaskrgn[EBP]
	CALL	xwsRgnDestroy#
	POPL	mw_oldmaskrgn[EBP]
	CALL	xwsRgnDestroy#
	CMPL	win_basfchild[ESI], #0
	JE	4$
	PUSHL	ESI
	MOVL	ESI, win_basfchild[ESI]
	CALL	includechilds
	POPL	ESI
4$:	MOVL	ESI, win_bassibling[ESI]
	TESTL	ESI, ESI
	JNE	includechilds
	RET
.PAGE
remasksibling:
	MOVL	EAX, win_basscnxpos[EDI] ;Get left edge of sibling window
	MOVL	EDX, EAX		;Get right edge of sibling window
	ADDL	EDX, win_baswidth[EDI]
	MOVL	ECX, win_basscnxpos[ESI]
	CMPL	EDX, ECX		;Is sibling window now left of this
					;  window?
	JLE	8$			;Yes - no overlap here
	ADDL	ECX, win_baswidth[ESI]
	CMPL	ECX, EAX		;Is sibling window now right of this
					;  window?
	JLE	8$			;Yes - no overlap here
	MOVL	EAX, win_basscnypos[EDI] ;Get top edge of sibling window
	MOVL	EDX, EAX		;Get bottom edge of sibling window
	ADDL	EDX, win_basheight[EDI]
	MOVL	ECX, win_basscnypos[ESI]
	CMPL	EDX, ECX		;Is sibling window now above this
					;  window?
	JLE	6$			;Yes - no overlap
	ADDL	ECX, win_basheight[ESI] ;Is sibling window now below this
	CMPL	ECX, EAX		;  window?
	JG	10$			;No - have overlap

;Here if the original position does not overlap the sibling window - now see
;  if it overlaps the new position

6$:	MOVL	EAX, win_basscnxpos[EDI] ;Get left edge of sibling window
	MOVL	EDX, EAX		;Get right edge of sibling window
	ADDL	EDX, win_baswidth[EDI]
8$:	MOVL	ECX, mw_oldxpos[EBP]
	CMPL	EDX, ECX		;Is sibling window now left of this
					;  window?
	JLE	12$			;Yes - no overlap
	ADDL	ECX, mw_oldwidth[EBP]
	CMPL	ECX, EAX		;Is sibling window now right of this
					;  window?
	JLE	12$			;Yes - no overlap
	MOVL	EAX, win_basscnypos[EDI] ;Get top edge of sibling window
	MOVL	EDX, EAX		;Get bottom edge of sibling window
	ADDL	EDX, win_basheight[EDI]
	MOVL	ECX, mw_oldypos[EBP]	;Is sibling window now above this
	CMPL	EDX, ECX		;  window?
	JLE	12$			;Yes - no overlap
	ADDL	ECX, mw_oldheight[EBP]	;Is sibling window now below this
	CMPL	ECX, EAX		;  window?
	JLE	12$
10$:	MOVB	AL, #1
	CALL	remaskwindow		;Have overlap - remask the sibling
	PUSHL	EAX			;  window
	CALL	xwsRgnDestroy#
	CMPL	win_basfchild[EDI], #0
	JE	12$
	PUSHL	EDI
	MOVL	EDI, win_basfchild[EDI]
	CALL	remasksibling
	POPL	EDI
12$:	MOVL	EDI, win_bassibling[EDI]
	CMPL	EDI, ESI
	JE	14$
	TESTL	EDI, EDI
	JNE	remasksibling
14$:	RET
.PAGE
;	c{EBX} = Offset of WIN for window being moved
;	c{EDI} = Offset of WIN for first child window
;	CALL	mergechilds

mergechilds:
	MOVL	EAX, win_basscnxpos[EDI]
	SUBL	EAX, win_basscnxpos[EBX]
	MOVL	EDX, win_basscnypos[EDI]
	SUBL	EDX, win_basscnypos[EBX]
	PUSHL	EBX
	PUSHL	win_basmaskrgn[EDI]
	PUSHL	EAX
	PUSHL	EDX
	CALL	xwsRgnDupOffset#
	PUSHL	EAX
	PUSHL	EAX
	PUSHL	ESI
	CALL	xwsRgnUnion#
	PUSHL	ESI
	MOVL	ESI, EAX
	CALL	xwsRgnDestroy#
	CALL	xwsRgnDestroy#
	POPL	EBX
	CMPL	win_basfchild[EDI], #0
	JE	4$
	PUSHL	EDI
	MOVL	EDI, win_basfchild[EDI]
	CALL	mergechilds
	POPL	EDI
4$:	MOVL	EDI, win_bassibling[EDI]
	TESTL	EDI, EDI
	JNE	mergechilds
	RET
.PAGE
;Suboutine to remask a window
;	c{EDI} = Offset of WIN for window
;	C(AL)  = 1 if should queue draw request, 0 if not
;	CALL	remaskwindow
;	c{EAX} = Offset of RGN for original mask region

$$$=!0
FRM rmw_draw      , 4t
FRM rmw_oldmaskrgn, 4t
FRM rmw_saveebx   , 4t
rmw_SIZE=!$$$

remaskwindow:
	PUSHL	ESI
	ENTER	rmw_SIZE, 0

	CHECKRGN win_basmaskrgn[EDI]
	CHECKRGN win_basbasergn[EDI]

	MOVL	rmw_draw[EBP], EAX	;Save draw argument
	MOVL	EAX, win_basmaskrgn[EDI] ;Save the current mask region
	MOVL	rmw_oldmaskrgn[EBP], EAX

;First we create an initial mask region which is the intersection of the
;  base region for this window and the client regions (relative to this
;  window) of the ancestor windows of this window, up to and including the
;  base window for the display context.

	PUSHL	win_basbasergn[EDI]	;Start out with a copy of the window's
	CALL	xwsRgnDuplicate#	;  base region
	MOVL	ESI, EAX
	MOVL	EBX, EDI
4$:	MOVL	EBX, win_basparent[EBX] ;Have another ancestor window?
	TESTL	EBX, EBX
	JE	6$			;No - finished with this part
	PUSHL	EBX
	PUSHL	win_basclientrgn[EBX]
	MOVL	EAX, win_basscnxpos[EBX]   ;Yes - create a region for its
	SUBL	EAX, win_basscnxpos[EDI]   ;  window
	PUSHL	EAX
	MOVL	EAX, win_basscnypos[EBX]
	SUBL	EAX, win_basscnypos[EDI]
	PUSHL	EAX
	CALL	xwsRgnDupOffset

	CHECKRGN EAX
	CHECKRGN ESI

	PUSHL	EAX
	PUSHL	EAX
	PUSHL	ESI
	CALL	xwsRgnIntersect#	;Get the intersection

	CHECKRGN EAX

	PUSHL	ESI
	MOVL	ESI, EAX
	CALL	xwsRgnDestroy#		;Give up the regions we are finished
	CALL	xwsRgnDestroy#		;  with
	POPL	EBX			;Restore offset of ancestor window
	JMP	4$			;Continue

;Here with the initial mask region created

6$:	MOVL	win_basmaskrgn[EDI], ESI

;Now we must check all windows that can possibly overlap this window.  This
;  includes all sibling windows with a higher z-order value, all siblings of
;  this windows parent with a higher z-order value than the parent, and so on
;  up to the root window.  Fortunately, for most real screens, this is not
;  really very many windows!
;	c{EDI} = Offset of WIN for this window

	MOVL	EBX, EDI

8$:	MOVL	ESI, win_bassibling[EBX] ;Get next sibling window
	TESTL	ESI, ESI
	JE	14$			;If no higher z-order siblings
10$:	MOVL	EAX, win_basscnxpos[EDI] ;Get left edge of first window
	MOVL	EDX, EAX		;Get right edge of first window
	ADDL	EDX, win_baswidth[EDI]
	MOVL	ECX, win_basscnxpos[ESI]
	CMPL	EDX, ECX		;Is 1st window to left of 2nd window?
	JLE	12$			;Yes - no overlap
	ADDL	ECX, win_baswidth[ESI] ;Is 2nd window to left of 1st window?
	CMPL	ECX, EAX		;Yes - no overlap
	JLE	12$	
	MOVL	EAX, win_basscnypos[EDI] ;Get top edge of first window
	MOVL	EDX, EAX		;Get bottom edge of first window
	ADDL	EDX, win_basheight[EDI]
	MOVL	ECX, win_basscnypos[ESI]
	CMPL	EDX, ECX		;Is 1st window above 2nd window?
	JLE	12$			;Yes - no overlap
	ADDL	ECX, win_basheight[ESI] ;Is 2nd window above 1st window?
	CMPL	ECX, EAX
	JLE	12$			;Yes - no overlap
	MOVL	rmw_saveebx[EBP], EBX
	PUSHL	win_basbasergn[ESI]	 ;Get the base region for the
	MOVL	EAX, win_basscnxpos[ESI] ;  overlapping window relative
	SUBL	EAX, win_basscnxpos[EDI] ;  to this window
	PUSHL	EAX
	MOVL	EAX, win_basscnypos[ESI]
	SUBL	EAX, win_basscnypos[EDI]
	PUSHL	EAX
	CALL	xwsRgnDupOffset#
	PUSHL	EAX
	PUSHL	win_basmaskrgn[EDI]	;Subtract its base region from our
	PUSHL	EAX			;  mask region
	CALL	xwsRgnDiff#

	CHECKRGN EAX

	PUSHL	win_basmaskrgn[EDI]
	MOVL	win_basmaskrgn[EDI], EAX ;Update our mask region
	CMPB	rgn_type[EAX], #RGNTYPE_NULL ;Is the mask region null now?
	JE	16$			;Yes
	CALL	xwsRgnDestroy#		;Give up the old mask region
	CALL	xwsRgnDestroy#		;Also give up the offset region
	MOVL	EBX, rmw_saveebx[EBP]
12$:	MOVL	ESI, win_bassibling[ESI] ;No - advance to next sibling window
	TESTL	ESI, ESI
	JNE	10$			;Continue if another sibling

;Here if no more siblings of this window - advance to its parent

14$:	MOVL	EBX, win_basparent[EBX]
	TESTL	EBX, EBX
	JNE	8$
	JMP	18$

;Here if we have a null mask region - in this case there is no reason to
;  do any more here

16$:	CALL	xwsRgnDestroy#		;Give up the old mask region
	CALL	xwsRgnDestroy#		;Also give up the offset region
	JMP	32$			;Go finish up

;Here with all higher level sibling windows handled - now handle any child
;  windows

18$:	MOVL	ESI, win_basfchild[EDI]	;Get first child
	TESTL	ESI, ESI
	JE	32$			;If no children

;Construct a base region for the child window relative to this window.  We
;  ensure that this region does not extend outside of our client region so
;  it will not affect our border.

20$:	PUSHL	win_basbasergn[ESI]	;Get the base region for the child
	PUSHL	win_basxpos[ESI]	;  window relative to this window
	PUSHL	win_basypos[ESI]
	CALL	xwsRgnDupOffset#
	PUSHL	EAX
	PUSHL	EAX
	PUSHL	win_basclientrgn[EDI]
	CALL	xwsRgnIntersect#	;Mask it to our client region
	PUSHL	EAX
	PUSHL	win_basmaskrgn[EDI]
	PUSHL	EAX
	CALL	xwsRgnDiff#

	CHECKRGN EAX

	PUSHL	win_basmaskrgn[EDI]
	MOVL	win_basmaskrgn[EDI], EAX
	CALL	xwsRgnDestroy#
	CALL	xwsRgnDestroy#
	CALL	xwsRgnDestroy#
30$:	MOVL	ESI, win_bassibling[ESI] ;Advance ot next sibling window
	TESTL	ESI, ESI
	JNE	20$

;Here with the window's mask region recalculated - now update the clipping
;  region

32$:	LEAL	ESI, win_bascliprgn[EDI]
	CMPL	win_bassavcliprgn[EDI], #0
	JE	34$
	ADDL	ESI, #win_bassavcliprgn-win_bascliprgn
34$:	PUSHL	[ESI]
	CALL	xwsRgnDestroy#
	PUSHL	win_basuserrgn[EDI]
	PUSHL	win_basclientrgn[EDI]
	CALL	xwsRgnIntersect#
	PUSHL	EAX
	PUSHL	EAX
	PUSHL	win_basmaskrgn[EDI]
	CALL	xwsRgnIntersect#
	MOVL	[ESI], EAX
	CALL	xwsRgnDestroy#		;Give up the temporary region (its
					;  already on the stack)

;Now set the draw region to be the difference between the new and old
;  mask regions

	MOVL	EAX, rmw_oldmaskrgn[EBP]

	CHECKRGN EAX
	CHECKRGN win_basmaskrgn[EDI]

	PUSHL	win_basmaskrgn[EDI]
	PUSHL	EAX
	CALL	xwsRgnDiff#

	CHECKRGN EAX

	CMPL	win_basdrawrgn[EDI], #0 ;Do we already have a draw region?
	JE	36$			;No
	MOVL	ESI, EAX		;Yes
	PUSHL	win_basdrawrgn[EDI]	;Get its intersection with our new
	PUSHL	win_basmaskrgn[EDI]	;  mask region
	CALL	xwsRgnIntersect#
	PUSHL	EAX
	PUSHL	ESI
	PUSHL	EAX			;Now get the union of the old and new
	CALL	xwsRgnUnion#		;  draw regions

	CHECKRGN EAX

	XCHGL	EAX, [ESP]
	PUSHL	EAX
	CALL	xwsRgnDestroy#
	PUSHL	win_basdrawrgn[EDI]
	CALL	xwsRgnDestroy#
	PUSHL	ESI
	CALL	xwsRgnDestroy#
	POPL	EAX
36$:	CMPB	rgn_type[EAX], #RGNTYPE_NULL ;Is the draw region null?
	JNE	38$			;No
	PUSHL	EAX			;Yes - give it up
	CALL	xwsRgnDestroy#
	CLRL	EAX
	MOVL	win_basdrawrgn[EDI], EAX
	MOVL	EAX, rmw_oldmaskrgn[EBP]
	LEAVE
	POPL	ESI
	RET

;Here if draw region is not null

38$:	MOVL	win_basdrawrgn[EDI], EAX
	CMPB	rmw_draw[EBP], #0	;Want to queue a draw request?
	JE	40$			;No
	CALL	xwsreqdraw#		;Yes
40$:	MOVL	EAX, rmw_oldmaskrgn[EBP]
	LEAVE
	POPL	ESI
	RET
.PAGE