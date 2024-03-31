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
// Function: xwsWinCreateButton - Create a button window       //
// Returned: 0 if normal or a negative XOS error code if error //
//*************************************************************//

long XOSFNC xwsWinCreateButton(
	XWSWIN    *parent,		// Parent window
	long       xpos,		// X position
	long       ypos,		// Y position
	long       xsize,		// Width
	long       ysize,		// Height
	XWSWINPAR *winparms,	// Window parameters
	XWSTXTPAR *txtparms,	// Text parameters
	XWSEVENT  *event,		// Pointer to caller's event function (may be null)
	long       evmask,		// Event mask bits
	XWSEDB    *edb,			// Pointer to caller's environment data block
	XWSWIN   **pwin,		// Pointer to location to receive address of the
							//   XWSWIN created
	char      *text,		// Text to display
	long       length)		// Length of the text string
{
	XWSWIN *win;
	long    rtn;

	if (parent == NULL)
		return (ER_VALUE);
	xpos = scalex(parent, xpos);
	ypos = scaley(parent, ypos);
	xsize = scalex(parent, xsize);
	ysize = scaley(parent, ysize);
	if ((rtn = xwscreatewindow(parent, XWS_WINTYPE_BUTTON, sizeof(XWSCNTR),
			xpos, ypos, xsize, ysize, winparms, event, evmask, edb, parent->drw,
			&win)) < 0)
		return (rtn);
	if (length > 0)
		xwsWinTextNew(win, 1, 0, 0, 0, 0, txtparms, text, length);

	if (pwin != NULL)
		*pwin = win;

	win->drw->funcdsp->shwcursor();
	return (0);
}


//**************************************************************//
// Function: xwseventbutton - System event function for buttons //
// Returned: Event return code                                  //
//**************************************************************//

int XOSFNC xwseventbutton(
	XWSEDBLK *edblk)
{
	XWSEDBLK edblkbtn;
	XWSWIN  *win;

	win = edblk->orgwin;
	switch (edblk->type)
	{
	 case XWS_EVENT_MOUSEMV:
		if (win == xwscursorgrabwin)
		{
			if ((edblk->xpos < 0 || edblk->ypos < 0 ||
					((edblk->xpos + win->xsfr) / win->xsfd) >= win->clntwidth ||
					((edblk->ypos + win->ysfr) / win->ysfd) >= win->clntheight))
			{
				if (win->status & WSB_PRESSED)
				{
					win->status &= ~(WSB_PRESSED|WSB_ALTCOL);
					xwsWinInvalidateRgn(win, NULL); // IS THIS NEEDED???
				}
			}
			else
			{
				if ((win->status & WSB_PRESSED) == 0)
				{
					win->status |= (WSB_PRESSED|WSB_ALTCOL);
					xwsWinInvalidateRgn(win, NULL); // IS THIS NEEDED???
				}
			}
		}
		break;

	 case XWS_EVENT_MOUSEBT:
		switch (edblk->subtype)
		{
		 case XWS_EVENT_MBTN_LDN:
			xwscursorgrabwin = win;		// Grab the cursor
			win->status |= (WSB_PRESSED|WSB_ALTCOL);
			xwsWinInvalidateRgn(win, NULL); // IS THIS NEEDED???
			break;

		 case XWS_EVENT_MBTN_LUP:
			if (xwscursorgrabwin == win) // Have we grabbed the cursor?
			{
				xwscursorgrabwin = NULL; // Yes - but not any more
				xwsupdatecursor();
			}
			if ((win->status & WSB_PRESSED) == 0)
				break;
			win->status &= ~(WSB_PRESSED|WSB_ALTCOL);
			xwsWinInvalidateRgn(win, NULL);
			if (edblk->xpos >= 0 && edblk->ypos >= 0 &&
					((edblk->xpos + win->xsfr) / win->xsfd) < win->clntwidth &&
					((edblk->ypos + win->ysfr) / win->ysfd) < win->clntheight)
			edblkbtn.label = 'EDB#';
			edblkbtn.length = sizeof(edblkbtn);
			edblkbtn.type = XWS_EVENT_WINDOW;
			edblkbtn.subtype = XWS_EVENT_WIN_BTN_PRESS;
			edblkbtn.orgwin = edblkbtn.dlvwin = win;
			edblkbtn.arg1 = 0;
			edblkbtn.arg2 = NULL;
			xwsdoevent(&edblkbtn);
			break;
		}
		break;

	 case XWS_EVENT_KEYRELS:
		break;

	 case XWS_EVENT_KEYMAKE:			// Keyboard key make?
		switch (edblk->charcd)
		{
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
	return (XWS_EVRTN_DONE);
}
