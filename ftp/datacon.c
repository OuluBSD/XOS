//--------------------------------------------------------------------------*
// FTPSRVDTA.C - XOS FTP server FTP data routines
//
// Written by: John Goltz
//--------------------------------------------------------------------------*

//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <xosnet.h>
#include <procarg.h>
#include <xosthreads.h>
#include <xosservert.h>
#include <xosudf.h>
#include <xosvfyuser.h>
#include <xosgetusersig.h>
#include <xosstr.h>
#include "ftpsrv.h"

// This file contains the main routines for the FTP client data thread.
//   There is a client data thread for each client session.

// This thread operates as a slave to the client command thread.  The command
//   thread set cdb->datadone to 0, stores a non-zero function value in
//   cdb->datafunc, and wakes this thread.  This thread sets cdb->datafunc
//   to 0 and performs the indicated function.  When the function is complete,
//   a non-zero value is stored in cdb->datadone (A positive value indicates
//   successful or normal completion while a negative value indicates an
//   error.).  After store the value, this thread wakes the corresponding
//   client command thread.


static long dataclose(CDB *cdb);
static long datafuncabort(CDB *cdb);
static long datafuncclose(CDB *cdb);
static long datafunclist(CDB *cdb);
static long datafuncnlst(CDB *cdb);
static long datafuncpasv(CDB *cdb);
static void datafuncquit(CDB *cdb);
static long datafuncretr(CDB *cdb);
static long datafuncstor(CDB *cdb);
static long dataoutput(CDB *cdb, char *data, int len);
static long diskclose(CDB *cdb);


static long (*cmdtbl[])(CDB *cdb) =
{	(long (*)(CDB *cdb))datafuncquit,	// DATAFUNC_QUIT  = 1
	datafuncabort,						// DATAFUNC_ABORT = 2
	datafuncclose,						// DATAFUNC_CLOSE = 3
	datafuncpasv,						// DATAFUNC_PASV  = 4
	datafunclist,						// DATAFUNC_LIST  = 5
	datafuncnlst,						// DATAFUNC_NLST  = 6
	datafuncretr,						// DATAFUNC_RETR  = 7
	datafuncstor						// DATAFUNC_STOR  = 8
};

//***********************************************************************
// Function: datathread - Main thread function for the client data thread
// Returned: Never returns
//***********************************************************************

void datathread(
    CDB *cdb)

{
	int  tmp;
	long rtn;

	while (TRUE)
	{
		while (cdb->datafunc == 0)
			thdCtlSuspendT(NULL, -1, -1);
		tmp = cdb->datafunc;
		cdb->datafunc = 0;
		cdb->dataerror = NULL;
		if ((rtn = (cmdtbl[tmp - 1])(cdb)) != 0)
		{
			cdb->datadone = rtn;
			thdCtlWake((long)cdb);
		}
	}
}


static void datafuncquit(
	CDB *cdb)

{
	cdb->datadone = 1;
	thdCtlWake((long)cdb);
	thdCtlTerminate();
}


static long datafuncabort(
	CDB *cdb)

{
	cdb = cdb;

	return (1);
}


static long datafuncclose(
	CDB *cdb)

{
	long rtn;

	return (((rtn = dataclose(cdb)) == 0) ? 1: rtn);
}


//*****************************************************
// Function: datafuncpasv - Handles the PASSIVE command
// Returned: ????
//*****************************************************

// Note that this function indicates completion immediately but does not
//   return until a data connection has been established.  This function
//   must be immediately followed by a data transfer function which will be
//   done as soon as this function returns.

static long datafuncpasv(
	CDB *cdb)

{
	long rtn;
	long hndl;
	struct
	{	byte4_parm limit;
		byte4_parm port;
		uchar      end;
	}    po1parms =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_NETCONLIMIT, 1},
		{PAR_GET|REP_HEXV, 4, IOPAR_NETLCLPORT}
	};
	struct
	{	byte4_parm hndl;
		byte4_parm addr;
		byte4_parm port;
		uchar      end;
	}    po2parms =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_NETCONHNDL},
		{PAR_GET|REP_HEXV, 4, IOPAR_NETRMTNETAR},
		{PAR_GET|REP_HEXV, 4, IOPAR_NETRMTPORTR}
	};

	if (cdb->pasvhndl > 0)
	{
		hndl = cdb->pasvhndl;
		cdb->pasvhndl = -1;
		thdIoClose(hndl, 0);
	}
	if (cdb->datahndl > 0)
	{
		hndl = cdb->datahndl;
		cdb->datahndl = -1;
		thdIoClose(hndl, 0);
	}
	rtn = thdIoOpen(0, cdb->idb->devname, &po1parms);
	if (cdb->dataabort)
		return (ER_ABORT);
	if (rtn < 0)
	{
		cdb->dataerror = "425 Error opening passive data connection";
		return (rtn);
	}
	po2parms.hndl.value = cdb->pasvhndl = rtn;
	cdb->lcldataport.v = po1parms.port.value;
	cdb->datacon = DATACON_WAITING;
	cdb->datadone = 1;
	thdCtlWake((long)cdb);

	// At this point we have indicated successful completion of the PASV
	//   command.  We are, however, now going to wait for an actual connection.
	//   If there is an error after this point, we cannot report to the client.
	//   All we can do is set the data connection state to DATACON_ERROR.  We
	//   can, of course, log serious errors.

	rtn = thdIoOpen(XO_IN|XO_OUT|XO_PARTIAL, cdb->idb->devname,
			&po2parms);
	hndl = cdb->pasvhndl;
	cdb->pasvhndl = -1;
	thdIoClose(hndl, 0);
	cdb->rmtdataport.v = po2parms.port.value;
	if (rtn < 0)
	{
		cdb->datacon = DATACON_ERROR;
		if (rtn != ER_NCLST && rtn != ER_NCCLR && rtn != ER_NORSP &&
				rtn != ER_ABORT && rtn != ER_CANCL)
			logerror(cdb->slot, rtn, "Error opening passive data connection");
		else if (srvDebugLevel >= 2)
			debug(cdb->slot, "DTA", 0, "Error %s opening passive data "
					"connection", xoscode(rtn));
		return (0);
	}
	if (po2parms.addr.value != cdb->rmtaddr.v)
	{
		cdb->datacon = DATACON_ERROR;
		logerror(cdb->slot, 0, "Passive data connection failed: IPA was "
				"%d.%d.%d.%d, should be %d.%d.%d.%d",
				((uchar *)(po2parms.addr.value))[0],
				((uchar *)(po2parms.addr.value))[1],
				((uchar *)(po2parms.addr.value))[2],
				((uchar *)(po2parms.addr.value))[3], cdb->rmtaddr.c[0],
				cdb->rmtaddr.c[1], cdb->rmtaddr.c[2], cdb->rmtaddr.c[3]);
		return (0);
	}
	if (srvDebugLevel >= 2)
		debug(cdb->slot, "DTA", 0, "Passive data connection opened to "
				"%d.%d.%d.%d.%d using port %d", cdb->rmtaddr.c[0],
				cdb->rmtaddr.c[1], cdb->rmtaddr.c[2], cdb->rmtaddr.c[3],
				cdb->rmtdataport.v, cdb->lcldataport.v);
	cdb->datahndl = rtn;
	cdb->datacon = DATACON_PASSIVE;
	return (0);
}


static long datafunclist(
	CDB *cdb)

{
	char *dpnt;
	long  rtn;
	int   readchr;
	char  bufr[1400];
	uchar spec[500];

	struct
	{	byte4_parm  hndl;
		byte1_parm  sattr;
		byte2_parm  fattr;
		byte4_parm  optn;
		lngstr_parm spec;
		byte4_parm  length;
		time8_parm  mdate;
		uchar       end;
		uchar       xxx[2];
	}    dsparms =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_DIRHNDL},
		{PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR, (char)(XA_NORMAL|XA_DIRECT)},
		{PAR_GET|REP_HEXV, 2, IOPAR_FILATTR},
		{PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_NOPREFIX|XFO_FILENAME},
		{PAR_GET|REP_STR , 0, IOPAR_FILSPEC, 0, sizeof(spec), sizeof(spec)},
		{PAR_GET|REP_HEXV, 4, IOPAR_LENGTH},
		{PAR_GET|REP_HEXV, 8, IOPAR_MDATE}
	};
	char timestr[32];

	if ((rtn = setupdatacon(cdb)) < 0)
	{
		thdIoClose(cdb->diskhndl, 0);
		cdb->diskhndl = -1;
		cdb->dataerror = "425 Can't open data connection";
		return (rtn);
	}
	dsparms.hndl.value = cdb->diskhndl;
	dsparms.spec.buffer = spec;
	dpnt = bufr;
	while (TRUE)
	{
		if ((rtn = thdIoDevParm(0, cdb->specbufr, &dsparms)) < 0)
		{
			thdIoClose(cdb->diskhndl, 0);
			cdb->diskhndl = -1;
			cdb->datacon = DATACON_IDLE;
			cdb->lcldataport.v = cdb->idb->dataport.v;
			cdb->rmtdataport.v = 0;
			if (rtn == ER_FILNF)
			{
				diskclose(cdb);
				if ((rtn = dataoutput(cdb, bufr, dpnt - bufr)) < 0 ||
						(rtn == dataclose(cdb)) < 0)
				{
					cdb->dataerror = "550 Error writing to data connection";
					return (rtn);
				}
				return (1);
			}
			cdb->dataerror = "550 Error writing to data connection";
			return (rtn);
		}
		if (dpnt > (bufr + sizeof(bufr) - (60 + dsparms.spec.strlen)))
		{
			if ((rtn = dataoutput(cdb, bufr, dpnt - bufr)) < 0)
			{
				diskclose(cdb);
				return (rtn);
			}
			dpnt = bufr;
		}
		sdt2str(timestr, "%r/%d/%l %h:%m",
				(time_sz *)&(dsparms.mdate.value));
		readchr = (dsparms.fattr.value & XA_RDONLY) ? '-' : 'w';
		dpnt += sprintf(dpnt, "%cr%c%cr%c-r%c- XOS  XOS %s %10d %s\r\n",
				(dsparms.fattr.value & XA_DIRECT) ? 'd' : '-', readchr,
				(dsparms.fattr.value & XA_DIRECT) ? '-' : 'x', readchr,
				readchr, timestr, dsparms.length.value, spec);
	}
}


static long datafuncnlst(
	CDB *cdb)

{
	char  *dpnt;
	uchar *pnt;
	uchar *begin;
	int    len;
	long   rtn;
	char   bufr[1400];
	uchar  spec[500];
	uchar  chr;

	struct
	{	byte4_parm  hndl;
		byte1_parm  attr;
		byte4_parm  optn;
		lngstr_parm spec;
		uchar       end;
		uchar       xxx[2];
	}    dsparms =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_DIRHNDL},
		{PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR, (char)XA_NORMAL},
		{PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_FILENAME},
		{PAR_GET|REP_STR , 0, IOPAR_FILSPEC, 0, sizeof(spec), sizeof(spec)}
	};

	if ((rtn = setupdatacon(cdb)) < 0)
		return (rtn);
	dsparms.hndl.value = cdb->diskhndl;
	dsparms.spec.buffer = spec;
	dpnt = bufr;
	while (TRUE)
	{
		if ((rtn = thdIoDevParm(XO_REPEAT, cdb->specbufr, &dsparms)) < 0)
		{
			thdIoClose(cdb->diskhndl, 0);
			cdb->diskhndl = -1;
			cdb->datacon = DATACON_IDLE;
			cdb->lcldataport.v = cdb->idb->dataport.v;
			cdb->rmtdataport.v = 0;
			if (rtn == ER_FILNF)
			{
				diskclose(cdb);
				if ((rtn = dataoutput(cdb, bufr, dpnt - bufr)) < 0 ||
						(rtn == dataclose(cdb)) < 0)
				{
					cdb->dataerror = "550 Error writing to data connection";
					return (rtn);
				}
				return (1);
			}
			return (rtn);
		}
		pnt = spec + 1;
		while (pnt[-1] != 0)
		{
			begin = pnt;
			while ((chr = *pnt++) != 0 && chr != XFS_FILENAME)
				;
			len = pnt - begin - 1;

			if (dpnt > (bufr + sizeof(bufr) - (len + 2)))
			{
				if ((rtn = dataoutput(cdb, bufr, dpnt - bufr)) < 0)
				{
					diskclose(cdb);
					return (rtn);
				}
				dpnt = bufr;
			}
			memcpy(dpnt, begin, len);
			dpnt += len;
			*dpnt++ = '\r';
			*dpnt++ = '\n';
		}
	}
}


static long datafuncretr(
	CDB *cdb)

{
	long  rtn;
	uchar bufr[1400];

	if ((rtn = setupdatacon(cdb)) < 0)
	{
		diskclose(cdb);
		return (rtn);
	}
	while ((rtn = thdIoInBlock(cdb->diskhndl, bufr, sizeof(bufr))) > 0)
	{
		if ((rtn = dataoutput(cdb, bufr, rtn)) < 0)
		{
			diskclose(cdb);
			return (rtn);
		}
	}
	diskclose(cdb);
	if (rtn != 0 && rtn != ER_EOF)
	{
		dataclose(cdb);
		cdb->dataerror = "550 Error reading from disk file";
		return (rtn);
	}
	if ((rtn = dataclose(cdb)) < 0)
	{
		cdb->dataerror = "550 Error writing to data connection";
		return (rtn);
	}
	return (1);
}


static long datafuncstor(
	CDB *cdb)

{
	long  rtn;
	uchar bufr[1400];

	if ((rtn = setupdatacon(cdb)) < 0)
		return (rtn);
	while ((rtn = thdIoInBlock(cdb->datahndl, bufr, sizeof(bufr))) >= 0)
	{
		if (srvDebugLevel >= 2)
			debug(cdb->slot, "DTA", 0, "Writing %d byte%s to disk file",
					rtn, (rtn == 1) ? "" : "s");
		if ((rtn = thdIoOutBlock(cdb->diskhndl, bufr, rtn)) < 0)
		{
			diskclose(cdb);
			dataclose(cdb);
			cdb->dataerror = "550 Error writing to disk file";
			return (rtn);
		}
	}
	dataclose(cdb);
	if (rtn != ER_NCCLR)
	{
		diskclose(cdb);
		cdb->dataerror = "550 Error reading from data connection";
		return (rtn);
	}
	if ((rtn = diskclose(cdb)) < 0)
	{
		cdb->dataerror = "550 Error writing to diskfile";
		return (rtn);
	}
	return (1);
}


long setupdatacon(
	CDB *cdb)

{
	if (cdb->datacon == DATACON_IDLE)
	{
		long rtn;
		struct
		{	byte4_parm addr;
			byte4_parm rmtport;
			byte4_parm lclport;
			uchar      end;
			uchar      xxx[3];
		}    actopenparms =

		{	{PAR_SET|REP_HEXV, 4, IOPAR_NETRMTNETAS},
			{PAR_SET|REP_HEXV, 4, IOPAR_NETRMTPORTS},
			{PAR_GET|REP_HEXV, 4, IOPAR_NETLCLPORT}
		};

		// Here if no PASV command was received. The data connection must be
		//   made by the server.

		if (srvDebugLevel > 0)
			debug(cdb->slot, "DTA", 0, "Connecting to client port %d for "
					"data connection", cdb->rmtdataport.v);
		cdb->datacon = DATACON_CONNECTING;
		actopenparms.addr.value = cdb->rmtaddr.v;
		actopenparms.rmtport.value = cdb->rmtdataport.v;

		if ((rtn = thdIoOpen(XO_IN|XO_OUT|XO_PARTIAL, cdb->idb->devname,
				&actopenparms)) < 0)
		{
			diskclose(cdb);
			cdb->datacon = DATACON_IDLE;
			cdb->lcldataport.v = 0;
			cdb->rmtdataport.v = cdb->idb->dataport.v;
			cdb->dataerror = "425 Can't open data connection";
			return (rtn);
		}
		cdb->datahndl = rtn;
		cdb->datacon = DATACON_ACTIVE;
	}
	return (0);
}


//**********************************************************
// Function: dataoutput - Output data on the data connection
// Returned: Nothing
//**********************************************************

static long dataoutput(
	CDB   *cdb,
	char  *data,
	int    len)

{
	long rtn;

	if (srvDebugLevel > 0)
		debug(cdb->slot, "DTA", 0, "Writing %d byte%s to data connection",
				len, (len == 1) ? "" : "s");
	if ((rtn = thdIoOutBlock(cdb->datahndl, data, len)) < 0)
	{
		dataclose(cdb);
		cdb->dataerror = "550 Error writing to data connection";
	}
	return (rtn);
}


//************************************************
// Function: dataclose - Close the data connection
// Returned: Nothing
//************************************************

static long dataclose(
	CDB   *cdb)

{
	long rtn;

	if (srvDebugLevel > 0)
		debug(cdb->slot, "DTA", 0, "Closing data connection");
	if (cdb->datahndl > 0)
	{
		rtn = thdIoClose(cdb->datahndl, 0);
		cdb->datahndl = -1;
	}
	cdb->datacon = DATACON_IDLE;
	cdb->lcldataport.v = cdb->idb->dataport.v;
	cdb->rmtdataport.v = 0;
	return (rtn);
}


//******************************************
// Function: diskclose - Close the disk file
// Returned: Nothing
//******************************************

static long diskclose(
	CDB *cdb)

{
	long rtn;

	if (srvDebugLevel > 0)
		debug(cdb->slot, "DTA", 0, "Closing data file");
	rtn = thdIoClose(cdb->diskhndl, 0);
	cdb->diskhndl = -1;
	return (rtn);
}
