#include "install.h"


int queschr1;
int queschr2;

static XWSWIN *winques;
static XWSWIN *btnq1;
static XWSWIN *btnq2;

static int   quertn;
static int   savedstate;
static uchar waiting;

//***********************************************************
// Function: question - Display a window which ask a question
// Returned: Nothing
//***********************************************************

// This function must only be called from the work thread. It blocks and
//   does not return until a selection is made.

int question(
	long  bgcolor,
	long  bdcolor,
	int   chr1,
	char *txt1,
	int   chr2,
	char *txt2,
	char *fmt, ...)

{
	va_list pi;
	XWSRECT ersz;
	long    len;
	long    xpos1;
	long    xpos2;
	long    ypos;
	char    text[200];

   	va_start(pi, fmt);
   	len = vsprintf(text, fmt, pi);
	savedstate = state;
	state = STATE_QUESTION;
	queschr1 = chr1;
	queschr2 = chr2;

	// Create our window

	winparms.cntr = xws_WinParContainerDefault;
	winparms.bgdcolor = bgcolor;
	winparms.bdrwidtho = 2250;
	winparms.bdrwidthc = 2250;
	winparms.bdrwidthi = 2250;
	winparms.bdrcolor = bdcolor;
	winparms.zorder = 241;
	xwsWinCreateContainer(xws_ScreenWIN, 52500, 90000, -52500, 93750,
			&winparms, NULL, 0, &edb, &winques);
	xwsWinGetClient(winques, &ersz);

	txtparms = xws_TxtParContainerDefault;
	txtparms.fheight = 9000;
	txtparms.txtcolor = 0x000000;
	txtparms.tbattr = TBA_WORDWRAP;
	txtparms.topmargin = 0;
	txtparms.btmmargin = 0;
	txtparms.lspace = 375;
	ypos = xwsWinTextNew(winques, 2, 3750, 3000, -20000, 1, &txtparms,
			text, len) + 52000;
	xwsWinMove(winques, 52500, 90000, -52500, ypos);
	winparms.butn = xws_WinParButtonDefault;
	txtparms = xws_TxtParButtonDefault;
	txtparms.fheight = 7000;
	winparms.evarg1 = chr2;

	ypos -= 35000;
	xpos2 = ersz.xsize/2;
	if (txt2 == NULL)
		xpos1 = xpos2 - 50000;
	else
	{
		xpos1 = xpos2 - 110000;
		xwsWinCreateButton(winques, xpos2 + 10000, ypos, 100000, 15000,
				&winparms, &txtparms, buttonevent, XWS_EVMASK_WINDOW,
				&edb, &btnq2, txt2, strlen(txt2));
	}
	winparms.butn.evarg1 = chr1;
	xwsWinCreateButton(winques, xpos1, ypos, 100000, 15000, &winparms,
			&txtparms, buttonevent, XWS_EVMASK_WINDOW, &edb, &btnq1, txt1,
			strlen(txt1));
	waiting = TRUE;
	quertn = 0;
	fbrCtlSuspend(NULL, -1);
	return (quertn);
}



long questionevent(
	long arg)
{
	if ((arg & 0xFF00) != 0)
		arg &= 0xFF00;
	if (arg == queschr1 || arg == queschr2)
	{
		quertn = (arg == queschr1) ? 1 : 2;
		xwsWinDestroy(winques);
		winques = NULL;
		state = savedstate;
		if (waiting)
		{
			waiting = FALSE;
			fbrCtlWake(wrkfbr);
		}
		return (XWS_EVRTN_DONE);
	}
	return (XWS_EVRTN_SYS);
}
