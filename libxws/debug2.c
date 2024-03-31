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

char xws_DebugOn = FALSE;

void (*xws_DebugFunc)(void) = NULL;

extern long xwscursorx;
extern long xwscursory;

static XWSWIN *winstatus;
static XWSWIN *winmouse;
static XWSFNT *fontdebug;

static long cursorxdsp;
static long cursorydsp;
static long totalmemdsp = -1;
static long totalblkdsp = -1;
static long alloccntdsp = -1;
static long freecntdsp = -1;

static XWSDRAW  drawstatus;
static XWSDRAW  drawmouse;
static XWSTIMER timerdebug;

// This file contains routines for various debugging displays

//******************************************************//
// Function: xwsDebugToggle - Toggle debugging features //
// Returned: Nothing                                    //
//******************************************************//

void XOSFNC xwsDebugToggle(void)

{
	XWSCNTRPAR winparms;

	static long tmrsts;

	if (xws_DebugOn)
	{
		xwsTimerStop(tmrsts);
		xwsWinDestroy(winstatus);
		winstatus = NULL;
		xwsWinDestroy(winmouse);
		winmouse = NULL;
		xws_DebugOn = FALSE;
	}
	else
	{
		winparms = xws_WinParContainerPlain;
    	winparms.cursor = xws_CurPoint;
		winparms.bgdcolor = 0x80F0F0;
		winparms.bits2 |= WB2_NOFOCUS;
		winparms.zorder = 254;
		winparms.draw = drawstatus;
		xwsWinCreateContainer(xws_ScreenWIN, 0, 0, 160000, 5000,
				(XWSWINPAR *)&winparms, NULL, 0, &xwsedb, &winstatus);
		winparms.draw = drawmouse;
		if (fontdebug == NULL)
			fontdebug = xwsFontLoad("DejaVu_Serif|Book", winstatus, 5000, 0, 0);
		xwsWinCreateContainer(xws_ScreenWIN, 333375, 0, 50000, 5000,
				(XWSWINPAR *)&winparms, NULL, 0, &xwsedb, &winmouse);
		tmrsts = xwsTimerStart(timerdebug, 0, (1LL << 63) + ST_SECOND/20);
		xws_DebugOn = TRUE;
	}
}


//*********************************************************
// Function: drawmouse - Draw function for the mouse window
// Returned: Nothing
//*********************************************************

static void drawmouse(
	XWSWIN *win)

{
	int  len;
	char bufr[100];

    xwsWinDrawBegin(win);
	len = sprintf(bufr, "X: %d", cursorxdsp);
    xwsDrawText(win, bufr, len, 4000, 4400, fontdebug, 0x000000);
	len = sprintf(bufr, "Y: %d", cursorydsp);
    xwsDrawText(win, bufr, len, 26000, 4400, fontdebug, 0x000000);
    xwsWinDrawEnd(win);
}


//***********************************************************
// Function: drawstatus - Draw function for the status window
// Returned: Nothing
//***********************************************************

static void drawstatus(
	XWSWIN *win)
{
	long alloccnt;
	long freecnt;
	long totalblk;
	long totalmem;
	int  len;
    char stsbufr[100];

	// The order of updating the saved values means that the values displayed
	//   will not reflect memory changes caused by outputing the values but
	//   it does ensure that the values will be stable. If the saved values
	//   are updated first, the alloc and free counts will never stablize.

	alloccnt = xws_AllocCnt;
	freecnt = xws_FreeCnt;
	totalblk = xws_TotalBlk;
	totalmem = xws_TotalMem;

    xwsWinDrawBegin(win);

	len = sprintf(stsbufr, "A: %,d        ", alloccnt);
    xwsDrawText(win, stsbufr, len, 4000, 4400, fontdebug, 0x000000);

	len = sprintf(stsbufr, "F: %,d        ", freecnt);
    xwsDrawText(win, stsbufr, len, 34000, 4400, fontdebug, 0x000000);

	len = sprintf(stsbufr, "B: %,d       ", totalblk);
    xwsDrawText(win, stsbufr, len, 71000, 4400, fontdebug, 0x000000);

	len = sprintf(stsbufr, "M: %,d       ", totalmem);
    xwsDrawText(win, stsbufr, len, 109000, 4400, fontdebug, 0x000000);

    xwsWinDrawEnd(win);

	alloccntdsp = xws_AllocCnt;
	freecntdsp = xws_FreeCnt;
	totalblkdsp = xws_TotalBlk;
	totalmemdsp = xws_TotalMem;
}


//********************************************************************//
// Function: timerdebug - Timer callback function for the debug timer //
// Returned: Nothing                                                  //
//********************************************************************//

static void timerdebug(
	long hndl,
	long data)
{
	if (alloccntdsp != xws_AllocCnt || freecntdsp != xws_FreeCnt ||
			totalmemdsp != xws_TotalMem || totalblkdsp != xws_TotalBlk)
		xwsWinInvalidateAll(winstatus);
	if (cursorxdsp != xwscursorx || cursorydsp != xwscursory)
	{
		cursorxdsp = xwscursorx;
		cursorydsp = xwscursory;
		xwsWinInvalidateAll(winmouse);
	}
}


//===============================================================//
// Debug functions - XWS programs should always be loaded with a //
//   debugger when using an XWS version with __DEBUG enabled.    //
//===============================================================//

#if __DEBUG

void XOSFNC xws__checksta(
	XWSWIN *win)
{
	if (win->caret.tl == win->anchor.tl)
	{
		if (win->caret.xpos >= win->anchor.xpos)
		{
			if (win->caret.cinx < win->anchor.cinx ||
					win->caret.cnum < win->anchor.cnum)
				INT3;
		}
		else
		{
			if (win->caret.cinx >= win->anchor.cinx ||
					win->caret.cnum >= win->anchor.cnum)
				INT3;
		}
	}
}

#endif
