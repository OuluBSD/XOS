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

// There are a number of colors associated with a window, all of which are
//   specified using window parameters. (NOTE: The word "highlight" is
//   overloaded
//   here. When refering to a window color it refers to the color of a border
//   area that is not shadowed. When refering to a text color it refers to colors
//   used for selected text.

//  Parameter    Use
//    The following apply to all window types
//  bgdcolor     Normal window background color
//  bdrcolor     Center border color
//  hlcolor      Normal border highlight color
//  shdcolor     Nornal border shadow color
//    The following only apply to buttons
//  bgdcoloralt  Alternate window background color
//  hlcoloralt   Alternate border highlight color
//  shcoloralt   Alternate border shadow color


//  bgdcolor     Text box background color
//  txtcolor     Text color
//  bgdcolorsel  Selected background color (used as the text box background
//                 color for selected text in place of bgcolor)
//  txtcolorsel  Selected text color (used as the text color for selected
//                 text in place of txtcolor)
//  caretcolor   Caret color

// When the "selected" colors are used, any text or character background colors
//   specified by formating functions are ignored. In buttons "selected" colors
//   are are applied to all text boxes when the button is pressed. In list
//   windows, the "selected" colors are applied to the entire selected text
//   box. In all other window types, any contiguous group of characters in a
//   single text box can be "selected" at any time.



// There are a number of parameters that specify various background and border
//   colors associated with a window. All windows allows specification of
//   background and border colors with the following parameters:
//     bgdcolor = Window background color
//     bdrcolor = Color of the center part of the boarder
//     hlcolor  = Color of the highlighted part of the inner and outer parts
//                  of the boarder
//     shdcolor = Color of the shadowed part of the inner and outer parts of
//                  the boarder.

// Buttons also allow specification of the following altername colors. These
//   colors are used when the button is pressed.
//     bgdcoloralt = Window background color
//     hlcoloralt  = Color of the highlighted part of the inner and outer parts
//                     of the border.
//     shdcoloralt = Color of the shadowed part of the inner and outer parts of
//                     of the border.
//   There is no alternate value for bdrcolor. Buttons are not normally drawn
//   with a center border area.

// Text colors and the background colors within a text box are specified with
//   the text parameters for a text box. The following text parameters are
//   used with all window types:
//     bgdcolor    = Normal text box background color (may be transparent)
//     txtcolor    = Normal text color
//     bgdcolorsel = Background color for selected text for all but buttons and
//                     lists, backgound color for text when button is pressed
//                     for buttons, background color for selected text box for
//                     lists.
//     txtcolorsel = Text color for selected text for all but buttons and
//                     lists, text color when button is pressed for buttons,
//                     text color for selected text box for lists.
// Edit windows also allow:
//     caretcolor  = Caret color
// Background colors for individual text elements can also be specified
//   independent of the above. These are only specified using formatting
//   functions. These colors are displayed on top of the normal background
//   color but are overlayed by the "selected" background. Likewise, any
//   text colors specified by formatting fucntions are overriden by the
//   "selected" text colors.





static void XOSFNC dofocusevent(XWSWIN *win, long focus);
static void XOSFNC linkchild(XWSWIN *win, int zval);

long xxxxpos;
long xxxypos;
long xxxxsize;
long xxxysize;


//************************************************************************//
// Function: xwscreatewindow - Internal function to create a basic window //
// Returned: 0 if normal or a negative error code if error                //
//************************************************************************//

long XOSFNC xwscreatewindow(
	XWSWIN    *parent,		// Parent WIN
	int        wtype,		// Window type
	long       size,		// Size of the WIN to create
	long       xpos,
	long       ypos,
	long       xsize,
	long       ysize,
	XWSWINPAR *parms,		// Window parameters
	XWSEVENT  *event,
	long       evmask,
	XWSEDB    *edb,
	XWSDRW    *drw,			// DRW for the window
	XWSWIN   **pwin)
{
	XWSWIN *win;
	XWSWIN *child;
	XWSGETCOLOR
	       *getcolor;
	long    bdrwidtho;
	long    bdrwidthc;
	long    bdrwidthi;
	long    bdrtotal;
	long    adj;

///	xxxxpos = xpos;
///	xxxypos = ypos;
///	xxxxsize = xsize;
///	xxxysize = ysize;
///	INT3;

	if ((win = xwsMalloc(size + parms->udatalen)) == NULL)
		xwsFail(-errno, "Cannot allocate memory for XWSWIN object");

	win->label = 'WIN#';
	win->prevmodal = NULL;
	win->udata = (parms->udatalen > 0) ? (((char *)win) + size) : 0;

	// Get scale factors for the window

	win->xsfd = (parms->xsfd != 0) ? parms->xsfd : (parent != NULL) ?
			parent->xsfd : xwsbasexsfd;
	win->xsfr = win->xsfd >> 1;

	win->ysfd = (parms->ysfd != 0) ? parms->ysfd : (parent != NULL) ?
			parent->ysfd : xwsbaseysfd;
	win->ysfr = win->ysfd >> 1;

	if (xsize <= 0 && parent != NULL)	// Is it the right offset?
		xsize += (parent->clntwidth - xpos /*  + parent->hbwdith */ );

	if (ysize <= 0 && parent != NULL)	// Is it the bottom offset?
		ysize += (parent->clntheight - ypos /*  + parent->vbheight */ );

	win->bits = parms->bits2;
	win->type = wtype;
	win->travlist = 0xFF;
	win->travnext = NULL;
	win->config = 0;
	win->parent = parent;
	win->evmask = evmask;

	// Verify the border sizes.  This is somewhat complicated because of the
	//   need to be able to highlight a window to indicate focus.  We do this
	//   by expanding the line around the window from 1 to 2 or 3 pixels.
	//   This is easy if the window has a normal border.  If it does not have
	//   a border at all, if the border is narrow, or if there is no outer
	//   border region, special fixup is needed as follows:
	//     For now, the minimum border size is 3 pixels. If the total border
	//     area is the minimum or greater, no fixup is needed, otherwise if
	//     the outer border area is non-zero, expand it to make the total
	//     border area equal to the minimum, otherwise if the inner border
	//     area is non-zero, expand it to make the total border area equal
	//     to the minimum, otherwise if the center border area is non-zero,
	//     expand it to make the total border area the minimum, otherwise
	//     (no border case), create a center border of the minimum size with
	//     the same color as the window's normal background color.

	// NOTE: Border widths are always scaled, even is xwsnoscale is TRUE.
	//       Border widths are scaled using the horizontal scale factor.

	bdrwidtho = SCALE(win->xsfr, win->xsfd, parms->bdrwidtho);
	bdrwidthc = SCALE(win->xsfr, win->xsfd, parms->bdrwidthc);
	bdrwidthi = SCALE(win->xsfr, win->xsfd, parms->bdrwidthi);
	while (TRUE)
	{
		bdrtotal = bdrwidtho + bdrwidthc + bdrwidthi; // Get total border size
		if (win->bits & WB2_LINEO)
			bdrtotal++;
		if (win->bits & WB2_LINEI)
			bdrtotal++;
		if (bdrtotal >= 3 || (win->bits & (WB2_NOFOCUS|WB2_NOSHWFCS)))
			break;
		else
		{
			// Here if want to show focus and the border size is not at least 3
			//   pixels.

			adj = 3 - bdrtotal;
			if (bdrwidtho != 0)			// Adjust outer border if it's not 0
			{
				bdrwidtho += adj;
				continue;
			}
			if (bdrwidthi != 0)			// Adjust center border if it's not 0
			{
				bdrwidthi += adj;
				continue;
			}
			if (bdrwidthc == 0)
				win->bdrcolor = win->bgdcolor;
			bdrwidthc += adj;
			continue;
		}
	}
	win->bdrwidtho = bdrwidtho;
	win->bdrwidthc = bdrwidthc;
	win->bdrwidthi = bdrwidthi;
	win->lbwidth = win->rbwidth = win->tbheight = win->bbheight = win->xorg =
			win->yorg = bdrtotal;
	bdrtotal <<= 1;

	// Here with the border sizes adjusted, if necessary, and stored

	if (xsize <= bdrtotal)
		xsize = bdrtotal + 1;
	win->clntwidth = (win->width = xsize) - bdrtotal;
	if (ysize <= bdrtotal)
		ysize = bdrtotal = 1;
	win->clntheight = (win->height = ysize) - bdrtotal;

	if (parent == NULL)
	{
		win->xpos = win->scnxpos = xpos;
		win->ypos = win->scnypos = ypos;
	}
	else
	{
		win->scnxpos = (win->xpos = xpos + parent->xorg) + parent->scnxpos;
		win->scnypos = (win->ypos = ypos + parent->yorg) + parent->scnypos;
	}

	win->drw = drw;
	win->offset = (win->scnypos * drw->pixelspan + win->scnxpos) * 2 +
			drw->buffer;

	win->basergn = xwsrgncreaterectns(0, 0, xsize, ysize); // Create the base
														   //   region
///	CHECKRGN(win->basergn);

	win->clientrgn = xwsrgncreaterectns(win->lbwidth, win->tbheight,
			win->clntwidth, win->clntheight); // Create the client region
	win->userrgn = &xwsunivrgn;
	win->maskrgn = &xwsnullrgn;
	win->cliprgn = &xwsnullrgn;

	win->drawrgn = NULL;
	win->drwmaskrgn = NULL;
	win->savcliprgn = NULL;
	win->moving = FALSE;
	win->drawflg = FALSE;
	win->fchild = NULL;
	win->sibling = NULL;
	win->status = 0;
	win->tbhead = NULL;
	win->tbtail = NULL;
	win->ibhead = NULL;

///	MOVL	win_bashlbgn+hla_tl[EDI], EAX
///	MOVL	win_bashlend+hla_tl[EDI], EAX

///	win->caret = CARET_NONE;

	win->drawnext = (XWSWIN *)-1;

	win->usrevent = event;
	win->edb = edb;

	win->caret.tl = NULL;
	win->anchor.tl = NULL;

	// Store the parameters that are common to all window types

	win->usrdraw = (parms->draw != NULL) ? parms->draw : xwsdefaultdraw;

	win->evarg1 = parms->evarg1;
	win->evarg2 = parms->evarg2;

	getcolor = drw->funcdsp->getcolor;

	win->bgdcolor = getcolor(parms->bgdcolor);

	win->bgdbm = parms->bgdbm;
	win->bgdbmalt = NULL;

	win->hlcolor = getcolor(parms->hlcolor);
	win->shdcolor = getcolor(parms->shdcolor);
	win->bdrcolor = getcolor(parms->bdrcolor);

	win->cursor = parms->cursor;

///	MOVL	EAX, wp_bdbm[EBX]
///	MOVL	win_basbdbm[EDI], EAX

	switch (wtype)
	{
	 case XWS_WINTYPE_SIMPLE:
		break;

	 case XWS_WINTYPE_CONTAINER:
		goto sbar;

	 case XWS_WINTYPE_LIST:
		goto sbar;

	 case XWS_WINTYPE_RESPONSE:
		break;

	 case XWS_WINTYPE_EDIT:


	 sbar:
		win->config |= WCB_SCRLABLE;	// Window can have scroll bars
		win->versb = NULL;
		win->horsb = NULL;
		break;

	 case XWS_WINTYPE_BUTTON:
		win->shift = parms->shift;
		win->bgdcoloralt = getcolor(parms->bgdcoloralt);
		win->hlcoloralt = getcolor(parms->hlcoloralt);
		win->shdcoloralt = getcolor(parms->shdcoloralt);
		win->bgdbmalt = parms->bgdbmalt;
		break;

	 case XWS_WINTYPE_CHECKBOX:
		break;

	 case XWS_WINTYPE_SCROLLBAR:
		break;

	 case XWS_WINTYPE_DROPDOWN:
		break;

	 case XWS_WINTYPE_INCDEC:
		break;
	}

	linkchild(win, parms->zorder);	// Link into the child list

///	CHECKRGN win_basbasergn[EDI]

	xwsremaskwindow(win, 1);			// Remask this window (don't worry about
										//   the original mask region since we
										//   know its the null region which is
										//   static)

///	CHECKRGN win_basbasergn[EDI]

	if (win->parent != NULL)
	{
///		CHECKRGN(win->parent->basergn)

		xwsRgnDestroy(xwsremaskwindow(win->parent, 1));
	}

	// Now we find and remask all windows whose covering might be changed
	//   because this window has been created. This is all sibling windows
	//   below this one in z-order and all child windows of each of these
	//   windows.

	if (win->parent != NULL && win->parent->fchild != win)
	{
		child = win->parent->fchild;
		while (child != win)
		{
			xwsnewremasksibling(win, child);
			child = child->sibling;
		}
	}

	// Here with the parent and all necessary sibling windows remasked

	if (parms->travinx != 0)
		xwsWinSetTraversal(win, parms->travlist, parms->travinx);
	if (win->bits & WB2_MODAL)			// Should this window be modal?
		xwsmakemodal(win);				// Yes
	else if (parms->bits1 & WB1_FOCUS)	// Should this window get focus?
		xwssetfocus(win);				// Yes

	*pwin = win;
	return (0);
}


//****************************************************************//
// Function: xwssetfocus - Set focus to a window                  //
// Returned: TRUE if focus changed, FALSE if focus did not change //
//****************************************************************//

// Focus is given to the specified window if possible. If not it is given to
//   it's first parent that can take focus. If no parent can take focus, no
//   change is made (The current focus window retains focus.)

int XOSFNC xwssetfocus(
	XWSWIN *win)
{
	XWSWIN *prevsib;
	XWSWIN *wpnt;

	while (win != NULL && (win->bits & WB2_NOFOCUS)) // Find a window that
		win = win->parent;							 //   can have focus
	if (win == NULL)
		return (FALSE);

	// Here with a window which can have focus

	if ((win->bits & WB2_ZORDER) && win->sibling != NULL &&
			(win->zorder | 0x0F) >= win->sibling->zorder)
	{
		// Here if need to modify the zorder when this window gets focus

		prevsib = win->sibling;
		xwsunlinkchild(win);			// Unlink the window
		linkchild(win, win->zorder | 0x0F); // And link it at its new place

		// Here with this window linked in its new Z-order position in its
		//   parent's child window list - now we remask any windows we have
		//   moved in front of.

		wpnt = win->sibling;
		do
		{
			xwsremaskallchilds(wpnt);
		}
		while ((wpnt = wpnt->sibling) != prevsib);
	}

	if (win != xwsfocuswin)
	{
		if (xwsfocuswin != NULL)
		{
			if ((xwsfocuswin->bits & WB2_NOSHWFCS) == 0)
				xwsWinInvalidateRgn(xwsfocuswin, 
						xwsRgnDiff(xwsfocuswin->maskrgn,
						xwsfocuswin->clientrgn));
			if (xwsfocuswin->config & WCB_CARET)
				xwstextinvalidatecaret(xwsfocuswin);
			if (xwsfocuswin->config & WCB_LIST)
				xwslistinvalidatesel(xwsfocuswin);
			if (xwsfocuswin->evmask & XWS_EVMASK_FOCUS) // Want an event?
				dofocusevent(xwsfocuswin, FALSE);
		}
		xwsfocuswin = win;
		if ((win->bits & WB2_NOSHWFCS) == 0)
			xwsWinInvalidateRgn(win, xwsRgnDiff(win->maskrgn, win->clientrgn));
		if (win->config & WCB_LIST)
			xwslistinvalidatesel(win);
		if (win->config & WCB_CARET)
			xwstextinvalidatecaret(win);
		if (win->evmask & XWS_EVMASK_FOCUS) // Want an event?
			dofocusevent(win, TRUE);
		return (TRUE);
	}
	return (FALSE);
}


static void XOSFNC dofocusevent(
	XWSWIN *win,
	long    focus)
{
	XWSEDBLK_FOCUS edblk;

	edblk.label = 'EDB#';
	edblk.length = sizeof(XWSEDBLK_FOCUS);
	edblk.type = XWS_EVENT_FOCUS;
	edblk.subtype = 0;
	edblk.orgwin = win;
	edblk.focus = focus;
	xwsdoevent((XWSEDBLK *)&edblk);
}


//***********************************************************************//
// Function: xwsunlinkchild - Unlink XWSWIN from its parent's child list //
// Returned: Nothing                                                     //
//***********************************************************************//

void XOSFNC xwsunlinkchild(
	XWSWIN *win)
{
	XWSWIN **cpntp;
	XWSWIN  *child;

	cpntp = &win->parent->fchild;
	while ((child = *cpntp) != NULL)
	{
		if (child == win)
		{
			*cpntp = win->sibling;
			win->sibling = NULL;
			return;
		}
		cpntp = &child->sibling;
	}
	xwsFail(0, "Invalid window sibling list");
}


//*****************************************************//
// Function: linkchild - Link XWSWIN into its parent's //
//             child list based on the Z-order         //
// Returned: Nothing                                   //
//*****************************************************//

static void XOSFNC linkchild(
	XWSWIN *win,
	int     zval)
{
	XWSWIN **cpntp;
	XWSWIN  *child;

	win->zorder = zval;
	if (win->parent != NULL)
	{
		cpntp = &win->parent->fchild;
		while ((child = *cpntp) != NULL)
		{
			if (zval < child->zorder)
				break;
			cpntp = &child->sibling;
		}
		win->sibling = child;
		*cpntp = win;
	}
}


//**************************************************************************//
// Function: xwsWinSetTraversal - Set window's position in a travseral list //
// Returned: Nothing                                                        //
//**************************************************************************//

// If the travinx value is 0 the window is removed from its current
//   travseral list. (The travlist value is ignored in this case.)

// Generally, all windows in a travseral list should allow focus. If a window
//   does not allow focus, focus will be given to the first parent of the
//   window that does allow focus, which may or may not be in the same (or
//   any) traversal list! Also, all windows in a travseral list should have
//   at least one of WB2_HTTRAV, WB2_RHTTRAV, and WB2_CRTRAV set to allow
//   traversal away for the window.

void XOSFNC xwsWinSetTraversal(
	XWSWIN *win,
	int     travlist,			// Travseral list number (0 to 7)
	int     travinx)			// Travseral number
{
	XWSWIN **ppnt;
	XWSWIN  *wpnt;

	if (win->travlist != 0xFF)			// In a traversal list now?
	{
		if (win == xwstravhead[win->travlist])
		{
			if ((xwstravhead[win->travlist] = win->travnext) == NULL)
				xwstravtail[win->travlist] = NULL;
		}
		else
		{
			wpnt = xwstravhead[win->travlist];

			while (win != wpnt->travnext)
			{
				if ((wpnt = wpnt->travnext) == NULL)
					xwsFail(0, "Bad window travseral list");
			}
			if ((wpnt->travnext = win->travnext) == NULL)
				xwstravtail[win->travlist] = wpnt;
		}
		win->travlist = 0xFF;
		win->travnext = NULL;
	}
	if (travinx != 0 && travlist < 8)	// Want to insert it into a list?
	{
		ppnt = &xwstravhead[travlist];	// Yes - find where it goes
		while ((wpnt = *ppnt) != NULL && travinx > wpnt->travinx)
			ppnt = &wpnt->travnext;
		win->travlist = travlist;		// Put it into the list
		win->travinx = travinx;
		if ((win->travnext = wpnt) == NULL)
			xwstravtail[win->travlist] = win;
		*ppnt = win;
	}
}


//***********************************************************//
// Function: xwstravahead - Give focus to next window in the //
//             travseral list                                //
// Returned: TRUE if focus changed, FALSE if left unchanged  //
//***********************************************************//

int XOSFNC xwstravahead(
	XWSWIN *win)
{
	return (xwssetfocus((win->travnext != NULL) ? win->travnext :
		xwstravhead[win->travlist]));

}


//**********************************************************//
// Function: xwstravback - Give focus to previous window in //
//             the travseral list                           //
// Returned: TRUE if focus changed, FALSE if left unchanged //
//**********************************************************//

int XOSFNC xwstravback(
	XWSWIN *win)
{
	XWSWIN *wpnt;

	if (win == xwstravhead[win->travlist]) // First window in the list?
		return (xwssetfocus(xwstravtail[win->travlist]));

	wpnt = xwstravhead[win->travlist];
	while (win != wpnt->travnext)
	{
		if ((wpnt = wpnt->travnext) == NULL)
			xwsFail(0, "Bad window travseral list");
	}
	return (xwssetfocus(wpnt));
}
