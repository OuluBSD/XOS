#include "install.h"


extern long xwscursorx;
extern long xwscursory;

static long debughndl;

static XWSWIN *winmouse;
static XWSWIN *winstatus;
static XWSFNT  *fontsts;

static long cursorxdsp;
static long cursorydsp;
static long totalmemdsp = -1;
static long totalblkdsp = -1;
static long alloccntdsp = -1;
static long freecntdsp = -1;

static XWSDRAW  drawstatus;
static XWSDRAW  drawmouse;
static XWSTIMER timerstatus;



void debugtoggle(void)
{
	if (winstatus == NULL)
	{
		winparms.cntr = xws_WinParContainerPlain;
   		winparms.cursor = xws_CurPoint;
		winparms.bgdcolor = 0x80F0F0;
		winparms.bits2 |= WB2_NOFOCUS;
		winparms.zorder = 200;
		winparms.draw = drawstatus;
		xwsWinCreateContainer(xws_ScreenWIN, 0, 0, 157500, 5250, &winparms,
				NULL, 0, &edb, &winstatus);
		winparms.cntr.draw = drawmouse;
		if (fontsts == NULL)
			fontsts = xwsFontLoad("DejaVu_Serif|Book", winstatus, 4500,
					0, 0);
		xwsWinCreateContainer(xws_ScreenWIN, 333375, 0, 50625, 5250,
				&winparms, NULL, 0, &edb, &winmouse);
		debughndl = xwsTimerStart(timerstatus, 0, (1LL < 63) | ST_SECOND/20);
	}
	else
	{
		xwsWinDestroy(winstatus);
		winstatus = NULL;
		xwsWinDestroy(winmouse);
		winmouse = NULL;
		xwsTimerStop(debughndl);
		debughndl = 0;
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
    xwsDrawText(win, bufr, len, 1500, 4125, fontsts, 0x000000);
	len = sprintf(bufr, "Y: %d", cursorydsp);
    xwsDrawText(win, bufr, len, 24000, 4125, fontsts, 0x000000);
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
    xwsDrawText(win, stsbufr, len, 3750, 4125, fontsts, 0x000000);

	len = sprintf(stsbufr, "F: %,d        ", freecnt);
    xwsDrawText(win, stsbufr, len, 37500, 4125, fontsts, 0x000000);

	len = sprintf(stsbufr, "B: %,d       ", totalblk);
    xwsDrawText(win, stsbufr, len, 71250, 4125, fontsts, 0x000000);

	len = sprintf(stsbufr, "M: %,d       ", totalmem);
    xwsDrawText(win, stsbufr, len, 108750, 4125, fontsts, 0x000000);

    xwsWinDrawEnd(win);

	alloccntdsp = xws_AllocCnt;
	freecntdsp = xws_FreeCnt;
	totalblkdsp = xws_TotalBlk;
	totalmemdsp = xws_TotalMem;
}


//*********************************************************************
// Function: timerstatus - Timer callback function for the status timer
// Returned: Nothing
//*********************************************************************

static void timerstatus(
	long hndl,
	long data)

{
	hndl = hndl;
	data = data;

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
