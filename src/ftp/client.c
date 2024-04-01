//==================================================
// FTPSRVCLT.C
// Written by John Goltz
//==================================================

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <xos.h>
#include <xosstr.h>
#include <xostime.h>
#include <errmsg.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xosacct.h>
#include <xoserr.h>
#include <time.h>
#include <procarg.h>
#include <xosthreads.h>
#include <xosservert.h>
#include "ftpsrv.h"

static struct
{   byte8_parm  timeout;
    char        end;
} cmdinparms =
{   {PAR_SET|REP_DECV, 8, IOPAR_TIMEOUT, {0x30E23400, 0x04}} // 30 minutes
};


//*********************************************************************
// Function: commandthread - Main thread function for the client thread
// Returned: Never returns
//*********************************************************************

void commandthread(
	IDB *idb,
    CDB *cdb,
    long slot,
    long hndl,
	long ipaddr,
	long port)

{
	char *inpnt;
	char *cpnt;
	int   cnt;
	long  rtn;
    long  avail;
	long  amount;
	long  args[1];
	char  chr;
	char  cmdinbufr[600];

	cdb->idb = idb;
    cdb->slot = slot;
    cdb->cmdhndl = hndl;
	cdb->rmtaddr.v = ipaddr;
	cdb->rmtcmdport.v = port;
	cdb->lcldataport.v = cdb->rmtdataport.v = 0;
	if (srvDebugLevel >= 2)
		debug(slot, "SES", 0, "Client session created");
	args[0] = (long)cdb;
	avail = (long)&(cdb->tdb) + 0x8000;
	if ((rtn = thdCtlCreate(avail, 0x2200, sizeof(CDB), datathread,
			args, 1)) < 0)
	{
		logerror(slot, rtn, "Could not create client data thread");
		thdIoClose(cdb->cmdhndl, 0);
		thdCtlTerminate();
	}
	svcSysDateTime(T_GTSYSDTTM, &(cdb->connectdt));
	idb->numclients++;
	cdb->dtdb = avail;
	if (idb->nologin)
	{
		cdb->loggedin = TRUE;
		cdb->allfiles = 2;
		cmdresponse(cdb, msg220b);
	}
	else
	{
		cdb->loggedin = FALSE;
		cdb->allfiles = 0;
		cmdresponse(cdb, msg220a);
	}
    while (TRUE)
    {
		avail = sizeof(cmdinbufr);
		inpnt = cmdinbufr;
		while (TRUE)
		{
			if ((amount = thdIoInBlockP(cdb->cmdhndl, inpnt, avail,
					&cmdinparms)) < 0)
    		{
				if (amount != ER_NCCLR && amount != ER_NCLST &&
						amount != ER_NORSP && amount != ER_ABORT &&
						amount != ER_CANCL)
					logerror(cdb->slot, amount, "Error reading from client "
							"connection");
				cdb->terminate = TRUE;
			}
			if (cdb->terminate)
				terminate(cdb);
			cnt = amount;
			cpnt = inpnt;
			inpnt += amount;
			avail -= amount;
			while (--cnt >= 0)
			{
				if ((chr = *cpnt++) == '\r')
				{
					cpnt[-1] = 0;
					continue;
				}
				if (chr == '\n')
				{
					cpnt[-1] = 0;
					docommand(cdb, cmdinbufr);
					if (cnt > 0)
					{
						memcpy(cmdinbufr, cpnt, cnt);
						inpnt = cmdinbufr + cnt;
						avail = sizeof(cmdinbufr) - cnt;
					}
					else
					{
						inpnt = cmdinbufr;
						avail = sizeof(cmdinbufr);
					}
				}
			}
		}
    }
}
