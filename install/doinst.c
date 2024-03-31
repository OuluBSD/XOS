#include "install.h"



void doinst(void)
{
	VBLK *vtmp;
	char  vtxt[64];

	continueinst = TRUE;
	didinit = FALSE;
	if ((index = xwsWinListGetSelItem(winlst)) >= 0)
	{
		// Here with the index of the selected item. Find the corrsponding
		//   VERSN and remove it from the list.

		vsel = vblkhead;
		vtmp = NULL;
		while (vsel != NULL)
		{
			if (vsel->index == index)
			{
				if (vtmp == NULL)
					vblkhead = vsel->next;
				else
					vtmp->next = vsel->next;
				vsel->next = NULL;		// Debug!
				break;
			}
			vtmp = vsel;
			vsel = vsel->next;
		}
	}
	if (vsel == NULL)
	{
		error(0, "No program is selected");
		return;
	}
	while (vblkhead != NULL)			// Give up all other VERSNs
	{
		vtmp = vblkhead->next;
		xwsFree(vblkhead);
		vblkhead = vtmp;
	}
	state = STATE_INST;

	// Set up the screen for this

	if (!fmtver(vtxt, &vsel->newver))
		strmov(vtxt, "???");
	sprintf(dispname, "%s_%s %s", vsel->vname, vsel->pname, vtxt);
	insttitlelen = sprintf(insttitle, "%s %s", (vsel->curver.maj == 0) ?
			"Installing" : "Updating", dispname);
	clearmain(insttitle);
	winparms.butn = xws_WinParButtonDefault;
	txtparms = xws_TxtParButtonDefault;
	txtparms.fheight = 7000;
	winparms.evarg1 = ISC_F11;
	xwsWinCreateButton(winw, BTNXPOS(3), mwsz.ysize - 20000, BTNWIDTH,
			12000, &winparms, &txtparms, buttonevent, XWS_EVMASK_WINDOW,
			&edb, &btncancel, "F11-Cancel", 10);

	txtparms = xws_TxtParContainerDefault;
	winparms.list = xws_WinParListDefault;
	winparms.bgdcolor = 0xE0E0E0;
	winparms.bits2 = WB2_NOSHWFOC;
	winparms.bits3 = /* WB3_VERSB | */ WB3_RECESSO | WB3_LINEI;
	xwsWinCreateList(winw, SELXPOS, SELYPOS, SELXSIZE, SELYSIZE, &winparms,
			NULL, 0, &edb, &winlst);
	index = 0;

///	putline(COL_DEBUG, "dspec: %s", dspec);
///	putline(COL_DEBUG, "ispec: %s", ispec);

	dowork(FUNC_INST);
}

