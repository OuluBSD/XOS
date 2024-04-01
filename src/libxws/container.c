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
// Function: xwsWinCreateContainer - Create a container window //
// Returned: 0 if normal or a negative XOS error code if error //
//*************************************************************//

long XOSFNC xwsWinCreateContainer(
	XWSWIN    *parent,	// Parent window
	long       xpos,	// X position
	long       ypos,	// Y position
	long       xsize,	// X size (width)
	long       ysize,	// Y size (height)
	XWSWINPAR *parms,	// Parameter structure
	XWSEVENT  *event,	// Pointer to caller's event function
	long       evmask,	// Event mask bits
	XWSEDB    *edb,		// Pointer to caller's environment data block
	XWSWIN   **pwin)	// Pointer to location to receive address of the WIN
{						//   created
	XWSDRW *drw;
	long    rtn;

	if (parent != NULL)
	{
		xpos = scalex(parent, xpos);
		ypos = scaley(parent, ypos);
		xsize = scalex(parent, xsize);
		ysize = scaley(parent, ysize);
		drw = parent->drw;
	}
	else
		drw = &xws_ScreenDRW;
	if ((rtn = xwscreatewindow(parent, XWS_WINTYPE_CONTAINER, sizeof(XWSCNTR),
			xpos, ypos, xsize, ysize, parms, event, evmask, edb, drw,
			pwin)) < 0)
		return (rtn);
	drw->funcdsp->shwcursor();
	return (0);
}


//********************************************************************//
// Function: xwseventcontainer - System event function for containers //
// Returned: XWS_EVRTN_PASS always                                    //
//********************************************************************//

int XOSFNC xwseventcontainer(
	XWSEDBLK *edblk)
{
	return (XWS_EVRTN_PASS);
}
