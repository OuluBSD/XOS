#include "install.h"

///int queschr1;
///int queschr2;

#if __DISKMAN

static XWSWIN *winerase;

///static XWSWIN *btnq1;
///static XWSWIN *btnq2;

///static int   quertn;
///static int   savedstate;
///static uchar waiting;

static XWSEVENT eventinit;


//*************************************
// Function: doinit - Initialize a disk
// Returned: Nothing
//*************************************

// This function creates a pop-up window which asks the user to confirm the
//   initialize operation. The response generates a request to the work
//   thread to do the initialization.

void doinit(
	char *msg,
	int   msglen,
	VBLK *vpntarg)
{
	XWSRECT initsz;
	long    ypos;
	int     len;
	char    text[400];

	vpntarg = vpntarg;

	state = STATE_INITIALIZE;

	// Create our window

	winparms.cntr = xws_WinParContainerDefault;
	winparms.bgdcolor = 0xFFC0C0;
	winparms.bdrwidtho = 2250;
	winparms.bdrwidthc = 2250;
	winparms.bdrwidthi = 2250;
	winparms.bdrcolor = 0xFF0000;
	winparms.zorder = 241;
	xwsWinCreateContainer(xws_ScreenWIN, 25000, 90000, -25000, 300000,
			&winparms, NULL, 0, &edb, &wininit);
	xwsWinGetClient(wininit, &initsz);
	txtparms = xws_TxtParContainerDefault;
	txtparms.fnum = 4;
	txtparms.fheight = 12000;
	txtparms.txtcolor = 0x000000;
	txtparms.lfmt = TLF_BCENTER;
	txtparms.topmargin = 0;
	txtparms.btmmargin = 0;
	txtparms.lspace = 375;
	len = sprintf(text, "Initialize Disk %s", tarbase);
	xwsWinTextNew(wininit, 2, 4000, 10000, -4000, 1, &txtparms, text, len);
	txtparms.fnum = 1;
	txtparms.fheight = 8000;
	txtparms.tbattr = TBA_WORDWRAP;
	ypos = xwsWinTextNew(wininit, 3, 6000, 30000, -4000, 1, &txtparms, msg,
			msglen);
	winparms.edit = xws_WinParEditDefault;
	winparms.edit.bits2 = WB2_NOSHWFOC;
	txtparms = xws_TxtParContainerDefault;
	txtparms.fheight = 8000;
	txtparms.leftmargin = 5000;
	txtparms.tbattr = TBA_SINGLE|TBA_VERCEN;
///	txtparms.bghlcolor = 0x8080C0;
	xwsWinCreateEdit(wininit, initsz.xsize/2 - 100000, ypos + 40000, 70000,
			15000, &winparms, &txtparms, eventinit, XWS_EVMASK_KEYMAKE, &edb,
			&winerase, NULL, 0);
	winparms.butn = xws_WinParButtonDefault;
	txtparms = xws_TxtParButtonDefault;
	txtparms.fheight = 7000;
	winparms.evarg1 = ISC_ESC;
	xwsWinCreateButton(wininit, initsz.xsize/2 + 30000, ypos + 40000 , 70000,
			15000, &winparms, &txtparms, buttonevent, XWS_EVMASK_WINDOW,
			&edb, &btncancel, "ESC-Cancel", 12);
	xwsWinMove(wininit, 25000, 90000, -25000, ypos + 78000);
	xwsWinSetFocus(winerase, 0);
}


//********************************************************************
// Function: eventinit - Event function for the init disk edit window
// Returned: XWS_EVRTN_DONE if have CR or ESC, otherwise XWS_EVRTN_SYS
//********************************************************************

// Note: The check for the event type is not really needed since we only
//       requested KEYMAKE events, but this is safe if we should change it
//       to add other events.

long eventinit(
	XWSEDBLK *edblk)

{
	char bufr[100];

	if (edblk->type == XWS_EVENT_KEYMAKE &&
			(((XWSEDBLK_KEYBRD *)edblk)->charcd == 0x1B ||
			((XWSEDBLK_KEYBRD *)edblk)->charcd == '\r'))
	{
		if (((XWSEDBLK_KEYBRD *)edblk)->charcd == '\r' &&
				xwsWinTextGet(winerase, 0, 0, bufr, sizeof(bufr) - 1) >= 0)
		{
			bufr[sizeof(bufr) - 1] = 0;
			if (strcmp(bufr, "ERASE") == 0)
			{
				xwsWinDestroy(wininit);
				sprintf(bufr, "Initializing Disk %s", tarbase);
				clearmain(bufr);
				winparms.butn = xws_WinParButtonDefault;
				txtparms = xws_TxtParButtonDefault;
				txtparms.fheight = 7000;
				winparms.evarg1 = ISC_F11;
				xwsWinCreateButton(winw, BTNXPOS(3), mwsz.ysize - 20000,
						BTNWIDTH, 12000, &winparms, &txtparms, buttonevent,
						XWS_EVMASK_WINDOW, &edb, &btncancel, "F11-Cancel", 10);
				txtparms = xws_TxtParContainerDefault;
				winparms.list = xws_WinParListDefault;
				winparms.bgdcolor = 0xE0E0E0;
				winparms.bits2 = WB2_NOSHWFOC;
				winparms.bits3 = WB3_VERSB | WB3_RECESSO | WB3_LINEI;
				xwsWinCreateList(winw, SELXPOS, SELYPOS, SELXSIZE, SELYSIZE,
						&winparms, NULL, 0, &edb, &winlst);
				index = 0;
				dowork(FUNC_INIT);
			}
			else
				xwsWinClear(winerase);
		}
		else
		{
			xwsWinDestroy(wininit);
			state = STATE_SELECT;
		}
	    return (XWS_EVRTN_DONE);
	}
    return (XWS_EVRTN_SYS);
}

#endif
