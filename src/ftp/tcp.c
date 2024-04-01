//==========================================================
// FTPSRVT.C
// Written by John Goltz
//==========================================================

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xos.h>
#include <xosstr.h>
#include <xostime.h>
#include <errmsg.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xosacct.h>
#include <xoserr.h>
#include <procarg.h>
#include <xosthreads.h>
#include <xosservert.h>
#include "ftpsrv.h"

//**********************************************************
// Function: tcpthread - Main thread function for the thread
//		that listens for TCP connect requrests
// Returned: Never returns
//**********************************************************

void tcpthread(void)

{
	struct
	{   byte4_parm  conhndl;
		byte4_parm  rmtaddr;
		byte4_parm  rmtport;
	    char        end;
	} tcplparms =
	{   {PAR_SET|REP_HEXV, 4, IOPAR_NETCONHNDL, 0},
		{PAR_GET|REP_HEXV, 4, IOPAR_NETRMTNETAR},
		{PAR_GET|REP_HEXV, 4, IOPAR_NETRMTPORTR}
	};
	IDB  *idb;
	CDB  *cdb;
	long *pnt;
	int   cnt;
	long  slot;
	long  hndl;
	long  rtn;
	long  args[6];

	idb = (IDB *)thdData;

    strcpy(idb->devname, devname);
	idb->instance = instance;
    idb->cmdport.v = cmdport;
	idb->dataport.v = dataport;
	idb->maxclients = maxclients;
	idb->nologin = nologin;

	idb->ipaddr.v = tcpopenparms.ipaddr.value;
	pnt = (long *)(idb->cdbtbl);
	cnt = 2;
	do
	{
		*pnt++ = cnt++;
	} while (cnt <= maxclients);
	idb->cdbfree = 1;
	tcplparms.conhndl.value = tcphndl;
	instance = 0;
	thdCtlWake(cmdtdb);
	while (TRUE)
	{
		// Wait for a connection request

		if ((hndl = thdIoOpen(XO_IN|XO_OUT|XO_PARTIAL, devname,
				&tcplparms)) < 0)
		{
			logerror(0, hndl, "TCP passive open failed");
			thdCtlSuspendT(NULL, 0, ST_SECOND);
			thdIoClose(hndl, 0);
			continue;
		}
/*
		if (reqtermination)
		{
			thdIoClose(hndl, 0);
			thdCtlTerminate();
		}
*/

		// Have a connection request, allocate a CDB and create the
		//   client thread

		if ((slot = (idb->cdbfree - 1)) < 0)
		{
			logerror(0, 0, "Too many clients - connection request ignored");
			thdIoClose(hndl, 0);
			continue;
		}

		cdb = (CDB *)(((long)idb) + CDBBASE + (slot << 16));
		idb->cdbfree = (long)(idb->cdbtbl[slot]);
		idb->cdbtbl[slot] = cdb;
		args[0] = (long)idb;
		args[1] = (long)cdb;
		args[2] = slot + 1;
		args[3] = hndl;
		args[4] = tcplparms.rmtaddr.value;
		args[5] = tcplparms.rmtport.value;
		if ((rtn = thdCtlCreate((long)&(cdb->tdb), 0x2200, sizeof(CDB),
				commandthread, args, 6)) < 0)
		{
			logerror(slot + 1, rtn, "Could not create client thread");
			thdIoClose(hndl, 0);
			continue;
		}
	}
}
