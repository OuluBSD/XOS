//========================================================
// TCP.C - MMS inventory file server TCP connection thread
// Written by John Goltz
//========================================================

#include "xfpsrv.h"


struct
{   byte4_parm conhndl;
	byte4_parm ipaddr;
	byte4_parm port;
    char       end;
} tcplparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_NETCONHNDL, 0},
	{PAR_GET|REP_HEXV, 4, IOPAR_NETRMTNETAR},
	{PAR_GET|REP_HEXV, 4, IOPAR_NETRMTPORTR}
};

//**********************************************************
// Function: tcpthread - Main thread function for the thread
//		that listens for TCP connect requrests
// Returned: Never returns
//**********************************************************

void tcpthread(void)

{
	CDB *cpnt;
	long slot;
	long hndl;
	long rtn;
	long args[5];

	while (1)
	{
		if (clientfree == 0)
		{
			logerror(0, "No more clients available - pausing");
			thdCtlSuspendT(NULL, 0, ST_SECOND * 20);
			continue;
		}

		// Wait for a connection request

		tcplparms.conhndl.value = tcphndl;
		if ((hndl = thdIoOpen(XO_IN|XO_OUT, tcpdev, &tcplparms)) < 0)
		{
			logerror(hndl, "TCP passive open failed");
			thdCtlSuspendT(NULL, 0, ST_SECOND * 20);
			continue;
		}

		// Have a connection request, allocate memory for the CDB and create
		//   the client thread

		if ((slot = clientfree) == 0)
		{
			logerror(0, "No more clients available - pausing");
			thdCtlSuspendT(NULL, 0, ST_SECOND * 20);
			continue;
		}
		cpnt = (CDB *)(FIRSTCDB + (slot - 1) * CDBSIZE);
		clientfree = (long)(clienttbl[slot]);
		clienttbl[slot] = cpnt;
		args[0] = (long)cpnt;
		args[1] = slot;
		args[2] = hndl;
		args[3] = tcplparms.ipaddr.value;
		args[4] = tcplparms.port.value;
		if ((rtn = thdCtlCreate((long)&(cpnt->tdb), sizeof(cpnt->stack),
			sizeof(CDB) - offsetof(CDB, tdb) , clientthread, args, 5)) < 0)
		{
			logerror(rtn, "Could not create client thread");
			continue;
		}
	}
}
