#include "install.h"

XWSWIN *winsel;


//*******************************************************************
// Function: scan - Scans the system to find all installable programs
// Returned: Nothing
//*******************************************************************

void doscan(void)
{
	VBLK *vpnt;
	long  rtn;
	char  text[512];

	state = STATE_SELECT;
	clearmain("Scanning system ... Please wait");
	while (vblkhead != NULL)
	{
		vpnt = vblkhead->next;
		xwsFree(vblkhead);
		vblkhead = vpnt;
	}

	// Create the container that contains the selection list

	winparms.cntr = xws_WinParContainerDefault;
	winparms.bgdcolor = 0xA0A0E0;
///	winparms.bdrwidtho = 1800;
///	winparms.bdrwidthc = 2800;
///	winparms.bdrwidthi = 1800;
	winparms.bits2 = WB2_NOSHWFOC;
	winparms.bits3 |= WB3_LINEI | WB3_RECESSO;
	xwsWinCreateContainer(winw, SELXPOS, SELYPOS, SELXSIZE, SELYSIZE,
			&winparms, NULL, 0, &edb, &winsel);

	// Display the selection list column captions

	txtparms = xws_TxtParContainerDefault;
	txtparms.txtcolor = 0x000000;
///	txtparms.txtcoloralt = 0xFFFFFF;
	txtparms.fheight = 7000;
	txtparms.lspace = 0;
	txtparms.leftmargin = 4000;
	txtparms.topmargin = 900;
	txtparms.btmmargin = 900;
	txtparms.bgdcolor = 0xA0A0E0;
	rtn = sprintf(text, "Vendor%vName%vType%vLocation%vNew Version%vCurrent "
			"Version", TF_HORPOS, POS_VENDOR, TF_HORPOS, POS_TYPE-10000,
			TF_HORPOS, POS_DEV, TF_HORPOS, POS_NDATE, TF_HORPOS, POS_CDATE);
	xwsWinTextNew(winsel, 1, 0, 1800, 0, 1, &txtparms, text, rtn);

	// Create the selection list list window

	txtparms = xws_TxtParContainerDefault;
	winparms.list = xws_WinParListDefault;
	winparms.bdrwidtho = 0;
	winparms.bgdcolor = 0xE0E0E0;
	winparms.bits2 = WB2_NOSHWFOC;
	winparms.bits3 = WB3_VERSB;
	xwsWinCreateList(winsel, 0, 12000, 0, 0, &winparms, NULL, 0, &edb, &winlst);
	index = 0;
	dowork(FUNC_SCAN);
}

