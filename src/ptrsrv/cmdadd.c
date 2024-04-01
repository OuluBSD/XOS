//--------------------------------------------------------------------------*
// cmdadd.c
// PCL print server for XOS ADD command
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
#include <errno.h>
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
#include <xosstr.h>
#include "ptrsrv.h"

// ADD command: Creates a new server instance
//  SERVER {unit} PTRSRV {P} ADD NAM{E}=n PTR=n {LIMIT=n}
//    Where:
//		unit    = PTRSRV unit number (default is 1)
//		SPLNAME = Spool name
//		PTRDEV  = Physical printer name
//		LIMIT   = Maximum number of bytes than can be spooled at one time
//				    (default is unlimited)

// WARNING: THIS VERSION ONLY SUPPORTS ONE INSTANCE!!

#define FULLSPLPATHSZ 256

ulong seqnum;

long  dirhndl;

static struct
{	BYTE4CHAR  unit;
	TEXT16CHAR splname;
	uchar      end;
} sauchars =
{	{PAR_SET|REP_DECV,  4, "UNIT", 0},
	{PAR_SET|REP_TEXT, 16, "SPLNAME"},
};

char splcls[] = "SPL:";

QAB splqab =
{
    QFNC_WAIT|QFNC_CLASSFUNC,	// func    - Function
    0,							// status  - Returned status
    0,							// error   - Error code
    0,							// amount  - Amount
    0,							// handle  - Device handle
    0,							// vector  - Vector for interrupt
    {0},
    CF_ADDUNIT,					// option  - Options or command
    0,							// count   - Count
	splcls,						// buffer1 - Pointer to file spec
    &sauchars,					// buffer2 - Unused
    NULL						// parm    - Pointer to parameter area
};

static struct
{	LNGSTRCHAR spldev;
	LNGSTRCHAR splpath;
	LNGSTRCHAR ptrdev;
	LNGSTRCHAR ptrlang;
	BYTE4CHAR  seqnum;
	BYTE4CHAR  clspid;
	BYTE4CHAR  clsvect;
	BYTE4CHAR  clsdata;
	uchar      end;
} splchars =
{	{PAR_SET|REP_STR , 0, "SPLDEV"},
	{PAR_SET|REP_STR , 0, "SPLPATH"},
	{PAR_SET|REP_STR , 0, "PTRDEV"},
	{PAR_SET|REP_STR , 0, "PTRLANG"},
	{PAR_SET|REP_DECV, 4, "SEQNUM"},
	{PAR_SET|REP_HEXV, 4, "CLSPID"},
	{PAR_SET|REP_DECV, 4, "CLSVECT"},
 	{PAR_SET|REP_HEXV, 4, "CLSDATA"}
};

struct
{	BYTE4PARM  bits;
	LNGSTRPARM phyname;
	uchar      end;
} opnparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_XOSDEV},
	{PAR_GET|REP_STR , 0, IOPAR_FILSPEC, phyname, 32, 32}
};

struct
{	BYTE4PARM  filoptn;
	LNGSTRPARM filspec;
	uchar      end;
}
osparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_XOSDEV|XFO_PATH|XFO_FILE},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, FULLSPLPATHSZ, FULLSPLPATHSZ}
};

struct
{	TEXT8CHAR  class;
	LNGSTRCHAR ptrcmd;
	char       end;
} ptrchars =
{	{PAR_GET|REP_TEXT, 8, "CLASS"},
	{PAR_GET|REP_STR , 0, "PTRCMD", NULL, 254, 254},
};

struct
{	BYTE4PARM  bits;
	LNGSTRPARM filename;
	TIME8PARM  dt;
///	BYTE4PARM  dirhndl;
	BYTE4PARM  numopen;
	uchar      end;
} dirparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_FILE},
	{PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, 32, 32},
	{PAR_GET|REP_DT  , 8, IOPAR_CDATE},
///	{PAR_SET|REP_HEXV, 4, IOPAR_DIRHNDL},
	{PAR_GET|REP_DECV, 4, IOPAR_NUMOPEN}
};



static IDB  *requireinst(char *cmd, arg_spec *keyword);
static int   scanspl(char *ext, int held);
static void  showinst(IDB *idb);


/*
arg_spec keywordadd[] =
{	{"PTRDEV" , ASF_VALREQ|ASF_LSVAL, NULL, fncaddptrdev , 0},
	{"PTR"    , ASF_VALREQ|ASF_LSVAL, NULL, fncaddptrdev , 0},
	{"SPLNAME", ASF_VALREQ|ASF_LSVAL, NULL, fncxxxsplname, 0},
	{"SPL"    , ASF_VALREQ|ASF_LSVAL, NULL, fncxxxsplname, 0},
	{"LIMIT"  , ASF_VALREQ|ASF_NVAL , NULL, fncaddlimit  , 0},
	{"LIM"    , ASF_VALREQ|ASF_NVAL , NULL, fncaddlimit  , 0},
	{"PRG"    , 0                   , NULL, fncxxxprg    , 0},
	{0}
};
*/


//*******************************************
// Function: cmdadd - Process the ADD command
// Returned: Nothing
//*******************************************

void cmdadd(
    char *cmd)
{
	IDB **inspnt;
	IDB  *newidb;
	char *spnt;
	char *dpnt;
	long  rtn;
	long  hndl;
	int   cnt;
	char  spldev[20];
	char  bufr[256];
	char  splspec[64];
	char  fullsplpath[FULLSPLPATHSZ];
	char  chr;

    if (!procargs(cmd, 2))
		return;

    if (ptrdev[0] == 0)
    {
		cmderror(ER_NOARG, "PTRDEV value not specified");
		return;
    }

	// Find an empty entry in the IDB table

	inspnt = instbl;
	cnt = INSTMAX;
	do
	{
		if (*inspnt == NULL)
			break;
		inspnt++;
	} while (--cnt > 0);
	if (cnt <= 0)
	{
		cmderror(ER_ERROR, "No more server instances are available (%s)",
				splname);
		return;
	}

	// Open the printer device

	if ((cmdptrhndl = svcIoOpen(XO_IN|XO_OUT, ptrdev, &opnparms)) < 0)
	{
		cmderror(cmdptrhndl, "Cannot open printer device");
		return;
	}

	// Determine what printer language this printer supports - We do this by
	//   parsing the CMD printer string and making some reasonable guesses!

	ptrchars.ptrcmd.buffer = bufr;
	if ((rtn = svcIoDevChar(cmdptrhndl, &ptrchars)) < 0)
	{
		logcmderror(rtn, "Error getting printer characteristics - Cannot "
				"continue");
		svcIoClose(cmdptrhndl, 0);
		return;
	}

	debugprint("Printer class: %s", (char *)&ptrchars.class.value);

	if (*(long *)(ptrchars.class.value) != 'RTP')
	{
		cmderror(ER_PDTYP, "Printer device is not a PTR class device - Cannot "
				"continue");
		svcIoClose(cmdptrhndl, 0);
		return;
	}
	bufr[ptrchars.ptrcmd.strlen] = ',';
	bufr[ptrchars.ptrcmd.strlen + 1] = 0;
	if (strstr(bufr, "PCL3GUI,") != NULL ||
			strstr(bufr, "DESKJET,") != NULL)
	{
		// If get here we have a PCL3GUI printer (probably HP). We use the
		//   $*g20W command to set the configuration. We support mono data
		//   using mode 9 compression and color data useing mode 10.

		splchars.ptrlang.buffer = "GUI-MC";
		splchars.ptrlang.strlen = splchars.ptrlang.bfrlen = 6;
	}
	else if (strstr(bufr, "PCL,") != NULL || strstr(bufr, "HBP,") != NULL)
	{
		// If get here we have a PCL or HBP (Brother) printer. We use the
		//   $t#R command to set resolution and $*b#M to set compression
		//   mode. Currently all we support in this case is mono mode using
		//   mode 9 compression.

		splchars.ptrlang.buffer = "PCL-M";
		splchars.ptrlang.strlen = splchars.ptrlang.bfrlen = 5;
	}
	else
	{
		cmderror(ER_ERROR,"Unsupported printer command set for printer %s "
				"for %s", ptrdev, splname);
		svcIoClose(cmdptrhndl, 0);
		return;
	}

	// Open the spool directory (create it if its not there) and build a
	//   list of all files to print.

	sprintf(splspec, "XOSSPL:\\%s\\", splname);
	osparms.filspec.buffer = fullsplpath;
	if ((dirhndl = svcIoOpen(XO_ODFS, splspec, &osparms)) < 0 &&
			dirhndl != ER_DIRNF && dirhndl != ER_FILNF)
	{
		cmderror(dirhndl, "Error opening spool directory for printer for %s - "
				"Cannot continue", splname);
		svcIoClose(cmdptrhndl, 0);
		return;
	}
	seqnum = 0;

	debugprint("rtnd str: |%s|", fullsplpath);

	if (dirhndl < 0)
	{
		// Here if the spool directory was not found - create it now

		if ((dirhndl = svcIoDevParm(XO_ODFS|XO_CREATE, splspec, NULL)) < 0 ||
				(dirhndl = svcIoOpen(XO_ODFS, splspec, &osparms)) < 0)
		{
			cmderror(dirhndl, "Error creating spool directory for %s - Cannot "
					"continue", splname);
			svcIoClose(dirhndl, 0);
			svcIoClose(cmdptrhndl, 0);
			return;
		}

		debugprint("crtd str: |%s|", fullsplpath);

	}

	debugprint("add: seqnum = %08X", seqnum);

	// Try to open the spool device to see if it already exits (this should
	//   only be the case when the server is restarted). If it does not exist
	//   create it.

	sprintf(bufr, "PTR%s:", splname);
	if ((hndl = svcIoOpen(XO_RAW, bufr, NULL)) < 0)
	{
		if (hndl != ER_NSDEV)
		{
			cmderror(hndl, "Error initializing spool device for %s - Cannot "
					"continue", splname);
			svcIoClose(dirhndl, 0);
			svcIoClose(cmdptrhndl, 0);
			return;
		}

		// Here if the spool device does not exist - create it.

		strncpy(sauchars.splname.value, splname, 16);;
		if ((rtn = svcIoQueue(&splqab)) < 0 || (rtn = splqab.error) < 0)
		{
			cmderror(rtn, "Error creating spool device for %s - Cannot "
					"continue", splname);
			svcIoClose(dirhndl, 0);
			svcIoClose(cmdptrhndl, 0);
			return;
		}

		// Try to open it now

		if ((hndl = svcIoOpen(XO_RAW, bufr, NULL)) < 0)
		{
			cmderror(hndl, "Error initializing spool device for %s - Cannot "
					"continue", splname);
			svcIoClose(dirhndl, 0);
			svcIoClose(cmdptrhndl, 0);
			return;
		}
	}

	newidb = (IDB *)((IDBBASE + ((inspnt - instbl) << IDBSHIFT)) + IDBINC/2 -
			sizeof(IDB));
	splchars.clspid.value = svcSysGetPid();
	splchars.clsvect.value = VECT_CLOSE;
	splchars.clsdata.value = (long)newidb;
	splchars.seqnum.value = seqnum + 1;
	splchars.ptrdev.buffer = ptrdev;
	splchars.ptrdev.strlen = splchars.ptrdev.bfrlen = strlen(ptrdev);
	spnt = fullsplpath;
	dpnt = spldev;
	do
	{
		chr = *spnt++;
		*dpnt++ = chr;
	} while (chr != ':');
	*dpnt = 0;

	splchars.spldev.buffer = spldev;
	splchars.spldev.strlen = splchars.spldev.bfrlen = dpnt - spldev;

	splchars.splpath.buffer = spnt;

	while (*spnt != 0)
		spnt++;
	if (spnt == splchars.splpath.buffer)
	{
		cmdidb = NULL;
		cmderror(rtn, "Spool directory for %s is null - Cannot "
				"continue", splname);
		svcIoClose(hndl, 0);
		svcIoClose(dirhndl, 0);
		svcIoClose(cmdptrhndl, 0);
		return;
	}
	if (spnt[-1] != '\\')
	{
		spnt[0] = '\\';
		spnt[1] = 0;
	}
	splchars.splpath.strlen = splchars.splpath.bfrlen =
			strlen(splchars.splpath.buffer);

	debugprint("add: PTRDEV:  %s", splchars.ptrdev.buffer);
	debugprint("add: SPLDEV:  %s", splchars.spldev.buffer);
	debugprint("add: SPLPATH: %s", splchars.splpath.buffer);

	if ((rtn = svcIoDevChar(hndl, &splchars)) < 0)
	{
		cmdidb = NULL;
		cmderror(rtn, "Error initializing spool device for %s - Cannot "
				"continue", splname);
		svcIoClose(hndl, 0);
		svcIoClose(dirhndl, 0);
		svcIoClose(cmdptrhndl, 0);
		return;
	}

    // Create a printer output thread which will do all output to the printer.
	//   The data area for this thread contains the IDB.

	threadready = FALSE;
    if ((rtn = thdCtlCreate((long)newidb, IDBINC/2 - sizeof(IDB), sizeof(IDB),
			outthread, NULL, 0)) < 0)
    {
		logcmderror(rtn, "Error creating printer output thread - Cannot "
				"continue");
		svcIoClose(dirhndl, 0);
		svcIoClose(cmdptrhndl, 0);
		return;
    }
	cmdidb = newidb;
	cmdidb->ptrlang = (char *)splchars.ptrlang.buffer;
	while (!threadready)
		thdCtlSuspend(NULL, -1);

	// Create a printer input thread which will do all input from the printer.

	threadready = FALSE;
    if ((rtn = thdCtlCreate(((long)cmdidb) + IDBINC/2,
			IDBINC/2 - sizeof(THDDATA), 0, inthread, NULL, 0)) < 0)
    {
		logcmderror(rtn, "Error creating printer input thread - Cannot "
				"continue");
		thdCtlKill((long)cmdidb);
		svcIoClose(dirhndl, 0);
		svcIoClose(cmdptrhndl, 0);
		return;
    }
	while (!threadready)
		thdCtlSuspend(NULL, -1);

	// Find all of the valid .SPL and .HLD files (the name must be exactly 8
	//   characters and must contain only hex digits) and build our list of
	//   files to print.

	if (!scanspl("SPL", FALSE) || !scanspl("HLD", TRUE))
	{
		svcIoClose(dirhndl, 0);
		svcIoClose(cmdptrhndl, 0);
		return;
	}
	svcIoClose(dirhndl, 0);

	thdCtlWake((long)cmdidb);			// Wake up the output thread

	// When get here everything has been set up

	sprintf(bufr, STR_MT_FINALMSG"PTRSRV: Instance %d created for printer "
			"%s spooled as PTR%s:", cmdidb->instance + 1, ptrdev, splname);
	srvLogSysLog(0, bufr + 9);
	debugprint(bufr + 9);
	srvCmdResponse(bufr, srvMsgDst);
}


static int scanspl(
	char *ext,
	int   held)
{
	char *pnt;
	long  num;
	long  rtn;
	char  name[128];
	char  filename[32];
	char  chr;

	if ((rtn = svcIoSetPos(dirhndl, 0, 0)) < 0)
	{
		cmderror(rtn, "Error scanning spool directory for %s - Cannot "
				"continue", splname);
		return (FALSE);
	}
	sprintf(name, "XOSSPL:\\%s\\*.%s", splname, ext);
	dirparms.filename.buffer = filename;
	while ((rtn = sysIoNextFile(dirhndl, name, &dirparms)) >= 0)
	{
		debugprint("scanspl: Have file: %s", filename);

		num = 0;
		pnt = filename;
		while ((chr = *pnt++) != 0 && chr != '.')
		{
			if (!isxdigit(chr))
				break;
			if (chr > '9')
				chr -= 0x37;
			num = (num << 4) + (chr & 0x0F);
		}
		if (chr == '.' && pnt == (filename + 9))
		{
			// Here if have a valid spooled file

			if (seqnum < num)
				seqnum = num;
			if ((rtn = quefile(cmdidb, num, dirparms.dt.value, held)) < 0)
			{
				cmderror(-errno, "Error creating queue list entry for %s - "
						"Cannot continue", splname);
				return (FALSE);
			}
		}
	}
	if (rtn != ER_FILNF)
	{
		cmderror(rtn, "Error scanning spool directory for %s - Cannot "
				"continue", splname);
		return (FALSE);
	}
	return (TRUE);
}
