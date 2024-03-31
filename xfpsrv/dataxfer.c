//====================================================
// DATAXFER.C - XFP file server data transfer routines
// Written by John Goltz
//====================================================

#include "xfpsrv.h"


void funcdata(
	CDB   *cdb,
	uchar *msg,
	int    len)

{
	uchar *dpnt;
	long   rtn;
	int    hd3;
	int    chnl;
	int    dlen;

///	printf("### Function: DATA\n");

	hd3 = cdb->buffer[3];
	if ((chnl = hd3 & 0x0F) > NUMHNDL || chnl == 0 ||
			(cdb->diskqab.qab_handle = cdb->hndltbl[chnl - 1]) == 0)
	{
		sendnormrsp(cdb, ER_NPERR, 0, NULL);
		return;
	}
	cdb->channel = chnl;
	if (cdb->ignore[chnl - 1])
	{
///		printf("### ignoring data\n");

		if (hd3 & 0x10)
			cdb->ignore[chnl - 1] = FALSE;
		return;
	}
	if (hd3 & 0x20)
	{
 		if ((dlen = sw(*(ushort *)msg)) > (len - 2))
		{
			sendnormrsp(cdb, ER_NPERR, 0, NULL);
			return;
		}
		dpnt = msg + 2;
		if ((rtn = setupparms(cdb, dpnt + dlen , len - 2 - dlen)) < 0)
		{
			sendnormrsp(cdb, rtn, 0, NULL);
			return;
		}
	}
	else
	{
		dlen = len;
		dpnt = msg;
		cdb->diskqab.qab_parm = NULL;
	}
	cdb->diskqab.qab_buffer1 = dpnt;
	cdb->diskqab.qab_count = dlen;
	cdb->diskqab.qab_func = QFNC_OUTBLOCK;
	if ((rtn = thdIoFunc(&cdb->diskqab)) < 0)
		rtn = cdb->diskqab.qab_error;
	cdb->total[chnl - 1] += cdb->diskqab.qab_amount;
	if (rtn > 0)
		rtn = 0;
	if (rtn < 0 || (hd3 & 0x10))
	{
		sendnormrsp(cdb, rtn, cdb->total[chnl - 1],
				(uchar *)(cdb->diskqab.qab_parm));
		if (hd3 & 0x10)
			cdb->total[chnl - 1] = 0;
		else
			cdb->ignore[chnl - 1] = TRUE;
	}
}


void funcdatareq(
	CDB   *cdb,
	uchar *msg,
	int    len)

{
	uchar *endpnt;
	uchar *tpnt;
	uchar *hpnt;
	long   rtn;
	int    chnl;
	int    amnt;
	int    needed;
	int    actual;
	int    hdr;

///	printf("### Function: DATAREQ\n");

	if ((chnl = cdb->buffer[3] & 0x0F) > NUMHNDL || chnl == 0 ||
			(cdb->diskqab.qab_handle = cdb->hndltbl[chnl - 1]) == 0)
	{
		BREAK();

		sendnormrsp(cdb, ER_NPERR, 0, NULL);
		return;
	}
	cdb->channel = chnl;
	if (!getvalue(&msg, &len, &needed) || needed < 0)
	{
		BREAK();

		sendnormrsp(cdb, ER_NPERR, 0, NULL);
		return;
	}
	if (len > 0)
	{
		if ((rtn = setupparms(cdb, msg, len)) < 0)
		{
			BREAK();

			sendnormrsp(cdb, rtn, 0, NULL);
			return;
		}
	}
	else
		cdb->diskqab.qab_parm = NULL;
	cdb->diskqab.qab_func = QFNC_INBLOCK;
	cdb->diskqab.qab_buffer1 = cdb->buffer + 10;
	do
	{
		if ((amnt = needed) > MAXDATA)
			amnt = MAXDATA;
		cdb->diskqab.qab_count = amnt;
		if ((rtn = thdIoFunc(&cdb->diskqab)) < 0)
			rtn = cdb->diskqab.qab_error;

		actual = cdb->diskqab.qab_amount;

		endpnt = cdb->buffer + 10 + actual;

		hpnt = cdb->buffer + 6;
		hdr = cdb->channel;
		if (cdb->diskqab.qab_parm != NULL)
		{
			hpnt -= 2;
			hdr |= 0x20;
			*(ushort *)(cdb->buffer + 8) = sw((ushort)actual);

			if ((tpnt = storeparms(cdb, endpnt,
					(uchar *)(cdb->diskqab.qab_parm))) == NULL)
			{
				BREAK();

				rtn = ER_NPERR;
			}
			else
				endpnt = tpnt;
			cdb->diskqab.qab_parm = NULL;
		}
		if (rtn < 0)
		{
			BREAK();

			hpnt -= 4;
			hdr |= 0x90;
			*(ulong *)(hpnt + 4) = sl(rtn);			
		}
		if (actual < amnt || needed <= amnt)
			hdr |= 0x10;
		needed -= amnt;
		len = endpnt - hpnt;
		*(ushort *)(hpnt) = sw(len - 4);
		hpnt[2] = XFPSF_DATARESP;
		hpnt[3] = hdr;
		if ((rtn = thdIoOutBlock(cdb->tcphndl, hpnt, len)) < 0)
			clientfail(cdb, rtn, "Error sending client response");
	} while ((hdr & 0x10) == 0);
}
