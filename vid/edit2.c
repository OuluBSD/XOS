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


void setattrib(int attrib, long lowchrv, long lowline, long highchrv,
		long highline);

static void dspnumx(int value, int pos);
static void findline(long linenum);


//================================================================
// Function: delete - Delete characters/lines from the file buffer
// Returned: Nothing
//================================================================

// The possible resulting hole in the file buffer is not closed! (This makes
//   deletes followed by inserts faster.)

	uchar *lastpnt;
	uchar *nextpnt;
	long   len;
	long   lines;
	long   cnt;
	long   newlen;

void delete(
	long firstchar,
	long firstline,
	long lastchar,
	long lastline)
{

	if (cw == &wdb0)					// In command mode?
	{									// Yes
		if (cw->curlincnt > 1)			// Is there something to delete?
		{								// Yes
			if ((cnt = cw->curlincnt - lastchar - 1) > 0)
				memcpy(cw->curlinbuf + firstchar, cw->curlinbuf + lastchar + 1,
						cnt);
			cw->curlincnt -= (lastchar - firstchar + 1);
		}
	}
	else								// Input mode
	{
		if (firstline == lastline)		// Deleting on a single line?
		{								// Maybe
			if (firstchar == lastchar && firstchar >= (cw->curlincnt - 1))
			{							// Deleting EOL?
				lastchar = -1;			// Yes - really a mulit-line delete
				lastline++;
			}
			else						// No - just a simple delete in the
			{							//   current line
				len = lastchar - firstchar + 1;
				if ((cnt = cw->curlincnt - lastchar - 1) > 0)
					memcpy(cw->curlinbuf + firstchar,
							cw->curlinbuf + lastchar + 1, cnt);

				cw->curlincnt -= len;
				cw->fdb->numchr -= len;
///
				dspcurline(cw);
				dspnumchars(cw);

				return;
			}
		}

		// Here for a multi-line delete

		findline(firstline);				// Make sure have the first line
		lines = cnt = lastline - firstline; //   in the line buffer
		lastpnt = cw->curpnt;			// Find the last line
		while (--cnt >= 0)
			lastpnt += *lastpnt + 1;
		len = *lastpnt;
		cnt = len - lastchar - 2;
		if ((newlen = firstchar + cnt + 1) < 255) // Is the new length valid?
		{								// Yes


			if (cnt > 0)				// Need to move something?
				memcpy(cw->curlinbuf + firstchar, lastpnt + lastchar + 2, cnt);
			cw->curlincnt = newlen;
			holesize = lastpnt - cw->curpnt + *lastpnt;
			cw->fdb->numlin -= lines;
			cw->fdb->numchr -= (lastpnt - cw->curpnt - lastchar +
					firstchar - 1);
			putline();
			dspnumlines(cw);
			dspcurline(cw);
			dspnumchars(cw);
			updwinfrc();
			getline();
		}
///		else							// If new line would be too long
///			beep();
	}
}


//=======================================================
// Function: findline - Find the n-th line in file buffer
// Returned: Nothing
//=======================================================

// The n-th line in the file buffer is set as the current line. The screen
//   is not updated.

static void findline(
	long linenum)

{
	uchar *fpnt;
	long   linecnt;
	long   len;

	if (linenum == cw->curlin)			// Is it already in the buffer?
		return;							// Yes - nothing to do here
	if (linenum > (cf->numlin - 1))
		linenum = cf->numlin - 1;
	if (linenum < cw->curlin)			// No - before current line?
	{
		fpnt = cf->filbfr;				// Yes - set to beginning of buffer
		linecnt = linenum;
	}
	else
	{
		linecnt = linenum - cw->curlin;	// No - get amount to advance
		fpnt = cw->curpnt;
	}
	while (--linecnt >= 0)
	{
		len = *fpnt;
		fpnt += len + 1;
		

	}
	cw->curpnt = fpnt;
	cw->curlin = linenum;
	len = *fpnt;
	memcpy(cw->curlinbuf, fpnt + 1, len - 1);
	cw->haveline = TRUE;
}


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
			setattrib(invattrb, cw->hllowchrv, oldline, cw->hlhighchrv,
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
			setattrib(invattrb, newchrv, cw->curlin, cw->hlhighchrv, oldline);
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
			setattrib(invattrb, prevchrv, prevline, cw->hlhighchrv, cw->curlin);
		}
		else
		{
			if (cw->curlin < cw->hllowline || // Did the type change?
					cw->curlin == cw->hllowline &&
							newchar < (cw->hllowchar - 1))
			{							// Yes - remove current highlight
				setattrib(txtattrb, cw->hllowchrv, cw->hllowline,
						cw->hlhighchrv, cw->hlhighline);
				cw->hlhighchar = cw->hllowchar - 1;
				cw->hlhighchrv = cw->hllowchrv - 1;	///// ####
				cw->hlhighline = cw->hllowline;
				
				cw->hllowchar = newchar;
				cw->hllowchrv = newchrv;		///// ####
				cw->hllowline = cw->curlin;
				setattrib(invattrb, newchrv, cw->curlin, cw->hlhighchrv,
						cw->hlhighline);
				cw->hlstate = HLS_DOWN;
			}
			else
			{
				prevchrv = cw->hlhighchrv;
				prevline = cw->hlhighline;
				cw->hlhighchar = newchar - 1; // Reduce current region
				cw->hlhighchrv = newchrv - 1;
				cw->hlhighline = cw->curlin;
				setattrib(txtattrb, newchrv, cw->curlin, prevchrv, prevline);
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
				setattrib(txtattrb, cw->hllowchrv, cw->hllowline,
						cw->hlhighchrv, cw->hlhighline);
				cw->hllowchar = cw->hlhighchar + 1;
				cw->hllowchrv = cw->hlhighchrv + 1;	///// ####
				cw->hllowline = cw->hlhighline;
				cw->hlhighchar = newchar - 1;
				cw->hlhighchrv = newchrv - 1;		///// ####
				cw->hlhighline = cw->curlin;
				setattrib(invattrb, cw->hllowchrv, cw->hllowline,
						cw->hlhighchrv, cw->curlin);
				cw->hlstate = HLS_UP;
			}
			else
			{
				prevchrv = cw->hllowchrv;
				prevline = cw->hllowline;
				cw->hllowchar = newchar; // Reduce current region
				cw->hllowchrv = newchrv;
				cw->hllowline = cw->curlin;
				setattrib(txtattrb, prevchrv, prevline, newchrv - 1,
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
			setattrib(invattrb,  newchrv, cw->curlin, prevchrv - 1,
					prevline);
		}
		break;
	}

	dspnumx(cw->hlstate, 0);
	dspnumx(cw->hllowline, 6);
	dspnumx(cw->hllowchar, 12);
	dspnumx(cw->hllowchrv, 18);
	dspnumx(cw->hlhighline, 30);
	dspnumx(cw->hlhighchar, 36);
	dspnumx(cw->hlhighchrv, 42);
}


//===============================================
// Function: setattrib - Set character attributes
// Returned: Nothing
//===============================================

void setattrib(
	int  attrib,
	long lowchrv,
	long lowline,
	long highchrv,
	long highline)

{
	long ofs;
	long lastchr;

	if (lowline < cw->toplin)
	{
		lowline = cw->toplin;
		lowchrv = 0;
	}
	else if (lowchrv < 0)
		lowchrv = 0;
	while (lowline <= highline)
	{
		ofs = ((cw->vpos + lowline - cw->toplin) * swidth +
				lowchrv - cw->left) * 2;
		if (lowline == highline)		// If on last line of region
		{
			lastchr = highchrv - cw->left;
			while (lowchrv <= lastchr)
			{
				(scrnbufr + 1)[ofs] = attrib;
				ofs += 2;
				++lowchrv;
			}
			return;
		}

		lastchr = cw->linelen[lowline - cw->toplin] + 1;


		while (lowchrv < lastchr)
		{
			(scrnbufr + 1)[ofs] = attrib;
			ofs += 2;
			lowchrv++;
		}
		lowchrv = cw->left;
		lowline++;
	}

}


//===================================================================
// Function: rmvhilite - Remove the current highlight region (if any)
// Returned: Nohting
//===================================================================

void rmvhilite(void)
{
	if (cw->hlstate != HLS_NONE)
	{
		setattrib(txtattrb, cw->hllowchrv, cw->hllowline, cw->hlhighchrv,
				cw->hlhighline);
		cw->hllowchar = 0;
		cw->hllowchrv = 0;
		cw->hllowline = 0;
		cw->hlhighchar = 0;
		cw->hlhighchrv = 0;
		cw->hlhighline = 0;
		cw->hlstate = HLS_NONE;

		dspnumx(cw->hlstate, 0);
		dspnumx(cw->hllowline, 6);
		dspnumx(cw->hllowchar, 12);
		dspnumx(cw->hllowchrv, 18);
		dspnumx(cw->hlhighline, 30);
		dspnumx(cw->hlhighchar, 36);
		dspnumx(cw->hlhighchrv, 42);
	}
}


static void dspnumx(
	int value,
	int pos)

{
	if (value < 0)
		value += 100000;
	dspnum(value, 5, cw->stslinpos + 80 + pos * 2);
}
