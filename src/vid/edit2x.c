#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <global.h>
#include <xosstr.h>
#include "vid.h"


void sethilitedown(int attrib, long lowchrv, long lowline, long highchrv,
		long highline);
void sethiliteup(int attrib, long lowchrv, long lowline, long highchrv,
		long highline);


//===============================================================
// Function: chkhilite - Check for change in the highlight region
// Returned: Nothing
//===============================================================

void chkhilite(
	long oldchar,
	long oldchrv,
	long oldline)

{
	long  prevchrv;
	long  prevline;
	long  newchar;
	long  newchrv;
///	long  inx;
	uchar increase;

	newchar = cw->cursorinb;
	newchrv = cw->cursorinv;

	if (cw->curlin > oldline)
		increase = TRUE;
	else if (cw->curlin < oldline)
		increase = FALSE;
	else if (newchar > oldchar)
		increase = TRUE;
	else if (newchar < oldchar)
		increase = FALSE;
	else
		return;							// If no change

	if ((kbbufr[2] & (SPC3_LFTSHF | SPC3_RHTSHF)) == 0)
	{
		if (cw->hlstate != HLS_NONE)
			rmvhilite();
		return;
	}


///	if (cw->hlstate == HLS_NONE && increase || cw->hlstate == HLS_UP)
///	{
///		// Move current position one to the left.
///
///			newchar--;
///			newchrv--;
///	}
///	else if (cw->hlstate == HLS_NONE)
///	{
///		// Move current position one to the right		
///
///		if (curlinbuf[newchar++] == '\t')
///			newchrv = (newchrv | tabmask) + 1;
///		else
///			newchrv++;
///	}

	switch (cw->hlstate)
	{
	 case HLS_NONE:					// If no highlight region now
		if (increase)
		{
			cw->hlhighchar = newchar - 1;	// Create one now
			cw->hlhighchrv = newchrv - 1;
			cw->hlhighline = cw->curlin;
			cw->hllowchar = oldchar;
			cw->hllowchrv = oldchrv;
			cw->hllowline = oldline;
			sethiliteup(invattrb, cw->hllowchrv, oldline, cw->hlhighchrv,
					cw->curlin);
			cw->hlstate = HLS_UP;
		}
		else
		{
			cw->hllowchar = newchar;	// Create one now
			cw->hllowchrv = newchrv;
			cw->hllowline = cw->curlin;
			cw->hlhighchar = oldchar - 1;
			cw->hlhighchrv = oldchrv - 1;
			cw->hlhighline = oldline;
			sethilitedown(invattrb, newchrv, cw->curlin, cw->hlhighchrv,
					oldline);
			cw->hlstate = HLS_DOWN;
		}
		break;

	 case HLS_UP:					// If have upwards region
		if (increase)
		{
			prevchrv = cw->hlhighchrv;
			prevline = cw->hlhighline;
			cw->hlhighchar = newchar - 1; // Extend current region up
			cw->hlhighchrv = newchrv - 1;
			cw->hlhighline = cw->curlin;
			sethiliteup(invattrb, prevchrv, prevline, cw->hlhighchrv,
					cw->curlin);
		}
		else
		{
			if (cw->curlin < cw->hllowline || // Did the type change?
					cw->curlin == cw->hllowline &&
							newchar < (cw->hllowchar - 1))
			{							// Yes - remove current highlight
				sethiliteup(txtattrb, cw->hllowchrv, cw->hllowline,
						cw->hlhighchrv, cw->hlhighline);
				cw->hlhighchar = cw->hllowchar - 1;
				cw->hlhighchrv = cw->hllowchrv - 1;	///// ####
				cw->hlhighline = cw->hllowline;
				
				cw->hllowchar = newchar;
				cw->hllowchrv = newchrv;		///// ####
				cw->hllowline = cw->curlin;
				sethilitedown(invattrb, newchrv, cw->curlin, cw->hlhighchrv,
						cw->hlhighline);
				cw->hlstate = HLS_DOWN;
			}
			else
			{
				prevchrv = cw->hlhighchrv;
				prevline = cw->hlhighline;
				cw->hlhighchar = newchar - 1; // Reduce current region
				cw->hlhighchrv = newchrv;
				cw->hlhighline = cw->curlin;
				sethiliteup(txtattrb, newchrv, cw->curlin, prevchrv, prevline);
				if (cw->hlhighchar < cw->hllowchar &&
						cw->hlhighline == cw->hllowline)
					rmvhilite();
			}
		}
		break;

	 case HLS_DOWN:						// If have downwards region
		if (increase)
		{
			if (cw->curlin > cw->hlhighline || // Did the type change?
					cw->curlin == cw->hlhighline && newchar > (cw->hlhighchar + 1))
			{							// Yes - remove current highlight
				sethilitedown(txtattrb, cw->hllowchrv, cw->hllowline,
						cw->hlhighchrv, cw->hlhighline);
				cw->hllowchar = cw->hlhighchar + 1;
				cw->hllowchrv = cw->hlhighchrv + 1;	///// ####
				cw->hllowline = cw->hlhighline;
				cw->hlhighchar = newchar - 1;
				cw->hlhighchrv = newchrv - 1;		///// ####
				cw->hlhighline = cw->curlin;
				sethiliteup(invattrb, cw->hllowchrv, cw->hllowline,
						newchrv - 1, cw->curlin);
				cw->hlstate = HLS_UP;
			}
			else
			{
				prevchrv = cw->hllowchrv;
				prevline = cw->hllowline;
				cw->hllowchar = newchar; // Reduce current region
				cw->hllowchrv = newchrv;
				cw->hllowline = cw->curlin;
				sethilitedown(txtattrb, prevchrv, prevline, newchrv - 1,
						cw->curlin);
				if (cw->hllowchar > cw->hlhighchar &&
						cw->hllowline == cw->hlhighline)
					rmvhilite();
			}
		}
		else
		{
			prevchrv = cw->hllowchrv;
			prevline = cw->hllowline;
			cw->hllowchar = newchar; // Extend current region down
			cw->hllowchrv = newchrv;
			cw->hllowline = cw->curlin;
			sethilitedown(invattrb,  newchrv, cw->curlin, prevchrv - 1,
					prevline);
		}
		break;
	}

	dspnum(cw->hlstate, 3, cw->stslinpos+160);
	dspnum(cw->hllowline, 5, cw->stslinpos+172);
	dspnum(cw->hllowchar, 5, cw->stslinpos+184);
	dspnum(cw->hlhighline, 5, cw->stslinpos+196);
	dspnum(cw->hlhighchar, 5, cw->stslinpos+208);
}


//==============================================================
// Function: sethiliteup - Set characters as highlighted (inverse)
// Returned: Nothing
//==============================================================

void sethiliteup(
	int  attrib,
	long lowchrv,
	long lowline,
	long highchrv,
	long highline)

{
	long ofs;

	if (lowline < cw->toplin)
	{
		lowline = cw->toplin;
		lowchrv = 0;
	}

	while (lowline <= highline)
	{
		ofs = ((cw->vpos + lowline - cw->toplin) * swidth +
				lowchrv - cw->left) * 2;
		if (lowline == highline)		// If on last line of region
		{
			while (lowchrv <= highchrv)
			{
				(scrnbufr + 1)[ofs] = attrib;
				ofs += 2;
				++lowchrv;
			}
			return;
		}
		while (lowchrv < cw->width)
		{
			(scrnbufr + 1)[ofs] = attrib;
			ofs += 2;
			lowchrv++;
		}
		lowchrv = 0;
		lowline++;
	}

}


//==============================================================
// Function: sethilitedown - Set characters as highlighted (inverse)
// Returned: Nothing
//==============================================================

void sethilitedown(
	int  attrib,
	long lowchrv,
	long lowline,
	long highchrv,
	long highline)

{
	long ofs;

	if (highline > (cw->toplin + cw->height - 1))
	{
		highline = cw->toplin + cw->height - 1;
		highchrv = cw->width - 1;
	}
	while (highline >= lowline)
	{
		ofs = ((cw->vpos + highline - cw->toplin) * swidth +
				highchrv - cw->left) * 2;
		if (highline == lowline)
		{
			while (highchrv >= lowchrv)
			{
				(scrnbufr + 1)[ofs] = attrib;
				ofs -= 2;
			    highchrv--;
			}
			return;
		}
		while (highchrv >= 0)
		{
			(scrnbufr + 1)[ofs] = attrib;
			ofs -= 2;
			highchrv--;
		}
		highchrv = cw->width - 1;
		highline--;
	}
}


//===================================================================
// Function: rmvhilite - Remove the current highlight region (if any)
// Returned: Nohting
//===================================================================

void rmvhilite(void)

{
	((cw->hlstate == HLS_UP) ? sethiliteup : sethilitedown)(txtattrb,
			cw->hllowchrv, cw->hllowline, cw->hlhighchrv, cw->hlhighline);
	cw->hllowchar = 0;
	cw->hllowchrv = 0;
	cw->hllowline = 0;
	cw->hlhighchar = 0;
	cw->hlhighchrv = 0;
	cw->hlhighline = 0;
	cw->hlstate = HLS_NONE;

	dspnum(cw->hlstate, 3, cw->stslinpos+160);
	dspnum(cw->hllowline, 5, cw->stslinpos+172);
	dspnum(cw->hllowchar, 5, cw->stslinpos+184);
	dspnum(cw->hlhighline, 5, cw->stslinpos+196);
	dspnum(cw->hlhighchar, 5, cw->stslinpos+208);
}
