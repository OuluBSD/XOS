#include "install.h"



XWSWIN *winerr;
XWSWIN *btncon;

int   savedstate;

static char btext[] = "<ENTER>-Continue";


//******************************************************
// Function: error - Display a serious error in a window
// Returned: Nothing
//******************************************************

// If the first character of modmsg is ! it indicates a fatal error and
//   this function does not return.

void error(
	long  code,
	char *fmt, ...)

{
	va_list pi;
	XWSRECT ersz;
	long  len;
	long  ypos;
	uchar fatal;
	char  text[200];

   	va_start(pi, fmt);

	if (fmt[0] == '!')
	{
		fatal = TRUE;
		fmt++;
	}
	else
		fatal = FALSE;
   	len = vsprintf(text, fmt, pi);
	savedstate = state;
	state = STATE_ERROR;

	// Create our window

	winparms.cntr = xws_WinParContainerDefault;
	winparms.bgdcolor = 0xFF4040;
	winparms.bdrwidtho = 2250;
	winparms.bdrwidthc = 2250;
	winparms.bdrwidthi = 2250;
	winparms.bdrcolor = 0x000000;
	winparms.zorder = 241;
	xwsWinCreateContainer(xws_ScreenWIN, 52500, 90000, -52500, 93750,
			&winparms, NULL, 0, &edb, &winerr);
	xwsWinGetClient(winerr, &ersz);

	txtparms = xws_TxtParContainerDefault;
	txtparms.fheight = 9750;
	txtparms.txtcolor = 0xFFFFFF;
	txtparms.tbattr = TBA_WORDWRAP;
	txtparms.topmargin = 0;
	txtparms.btmmargin = 0;
	txtparms.lspace = 375;
	ypos = xwsWinTextNew(winerr, 2, 3750, 3000, -3750, 1, &txtparms, text,
			len) + 1500;
	if (code < 0)
	{
		len = svcSysErrMsg(code, 0x03, text);
		ypos += (xwsWinTextNew(winerr, 3, 3750, ypos, -3750, 1, &txtparms,
				text, len) + 55000);
	}
	else
		ypos += 50000;
	txtparms.tbattr = 0;

	xwsWinMove(winerr, 52500, 90000, -52500, ypos);
	winparms.butn = xws_WinParButtonDefault;
	txtparms = xws_TxtParButtonDefault;
	txtparms.fheight = 7000;
	winparms.evarg1 = '\r' << 8;
	xwsWinCreateButton(winerr, ersz.xsize/2 - 40000, ypos - 35000, 80000,
			15000, &winparms, &txtparms, buttonevent, XWS_EVMASK_WINDOW,
			&edb, &btncon, btext, sizeof(btext) -1);
	fbrCtlSuspend(NULL, -1);
}


void errorclose(void)
{
	if (winerr != NULL)
	{
		xwsWinDestroy(winerr);
		winerr = NULL;
		state = savedstate;
		fbrCtlWake(wrkfbr);
	}
}
