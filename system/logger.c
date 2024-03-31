//--------------------------------------------------------------------------*
// LOGGER.C
// System logging server for XOS
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
#include <xosvfyuser.h>
#include <xosservert.h>
#include <xosstr.h>

// This program is the system logging server.  It is initialized as a symbiont
//   with the following command:

//	SYMBIONT LOGGER UNIT=n LOGLEVEL=l LOGFILE=log
//	  Where:
//		UNIT     = Server unit number (default = 1)
//		LOGLEVEL = Specifies the debug logging level, default is 3 if LOGFILE
//			   is specified, 0 otherwise
//				0 = No debug logging
//				1 = Log major events
//				2 = Log all network messages
//		LOGFILE  = Specifies file for log output, if not specified no
//			   log output is generated

// Memory layout for a single instance (starts on a 16MB boundry, base for
//   unit 1 starts at 16MB):
//    base + 00000000:        IDB
//    base + idb_cdbtbl:      CDB table (first entry is for CDB number 8)
//    base + 00008000:        First CDB
//    base + 00009000:        Second CDB
//    ...

//   Free CDBs are managed using a free list linked through the CDB table.
//   each free list entry contains the NUMBER of the next free CDB.  The number
//   is the difference between the CDBs offset and the base of the IDB divided
//   by 1000h.  The first CDB thus has a number of 8.

// Logger will normally be started at the beginning of the config.cfg startup
//   file. At this point XOSSYS: points to the once-only ramdisk. There are no
//   other disks available. In this case logger will buffer all messages in
//   memory until the definition of XOSSYS: is changed. This is only allowed
//   for a single instance, which will normally write to XOSACT:xos.log. An
//   attempt to start more than one instance while using the ramdisk will fail.

#define MAJVER  1
#define MINVER  1
#define EDITNUM 0

#define DEBUGFILE 0

typedef union
{	long v;
	struct
	{	ushort editnum;
		uchar  minor;
		uchar  major;
	};
} VERSION;

typedef struct
{	ushort id;
	ushort len;
	long   pid;
	time_s dt;
	char   label[8];
	char   text[1];
} MEMMSG;

#define INSTMAX  31
#define CMDTDB   0x00800000
#define TMRTDB   0x00820000
#define FMSGTDB  0x00900000
#define MEMBUFR  0x00400000

#define ver ver2(MAJVER,MINVER,EDITNUM)
#define ver2(a,b,c) ver3(a,b,c)
#define ver3(maj,min,edit) #maj "." #min "." #edit

// Define the IDB (Instance Data Block)

typedef struct _idb IDB;
struct _idb
{	THDDATA tdb;
	IDB    *next;
	char    logdev[24];
	char    logname[32];
	int     instance;			// Instance number
	time_s  logdate;			// Creation date/time of long file
	long    filehndl;
	long    msghndl;
	uchar   commit;
	uchar   xxx[3];
};


VERSION xosver;

int   slot;
int   instance;
long  instx;
long  ready;
long  ourpid;
char  logdev[24];
char  logname[32];
long  cmdtdb;
long  version = MAJVER;
long  editnum = EDITNUM;
char *errormsgpnt;
IDB  *firstidb;
#if DEBUGFILE
  FILE *debugfile;
#endif

static void banner(void);
static void cmdadd(char *cmd);
static void cmdremove(char *cmd);
static void cmdstatus(char *cmd);
static int  fncaddname(arg_data *arg);
static int  fncinstance(arg_data *arg);
static void msgthread(void);
static void putinlog(IDB *idb, long pid, char *label, char *text);
static IDB *requireinst(char *cmd, arg_spec *keyword);
       void rolllog(IDB *idb, time_s date, char *text);
static void showinst(IDB *idb);
static void tmrthread(void);
#if DEBUGFILE
  static void debugdump(char *msg, int size);
#endif

arg_spec srvCmdWord[] =
{   {"ADD"   , 0, NULL, srvFncCommand, (long)cmdadd},
    {"REMOVE", 0, NULL, srvFncCommand, (long)cmdremove},
    {"REM"   , 0, NULL, srvFncCommand, (long)cmdremove},
    {"STATUS", 0, NULL, srvFncCommand, (long)cmdstatus},
    {"STA"   , 0, NULL, srvFncCommand, (long)cmdstatus},
    {0}
};

arg_spec keywordadd[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL , NULL, fncinstance, 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL , NULL, fncinstance, 0},
	{"NAME"    , ASF_VALREQ|ASF_LSVAL, NULL, fncaddname , 0},
	{"NAM"     , ASF_VALREQ|ASF_LSVAL, NULL, fncaddname , 0},
	{0}
};

arg_spec keywordins[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {0}
};

static struct
{	TIME8PARM dt;
	uchar     end;
} loparms =
{	{PAR_GET|REP_DT, 8, IOPAR_CDATE}
};

char prgname[] = "LOGGER";
char srvname[] = "LOGGER";

uchar debugflg;
uchar havetmr = FALSE;

uchar havedisk;

// Following variables are only used during startup

union
{	char   *c;
	MEMMSG *m;
}     mempnt;
char *memtop;


int  checkdisk(void);
void putinmem(time_sz *dt, long pid, char *label, char *text);
void puttodisk(IDB *idb, time_sz *dt, long pid, char *label, char *text);
void setupdisk(IDB *idb, char *filename);
void setupmem(IDB *idb, char *filename);


void mainalt(
    char *args)

{
	srvInitialize(args, (void *)CMDTDB, 0x2000, 70, 50, MAJVER, MINVER,
			EDITNUM);
}


//**********************************************************
// Function: srvSetup2 - Called by server routine at startup
//				before response is sent
// Returned: Nothing
//**********************************************************

void srvSetup1(void)

{
	debugflg = (srvReqName[0] == 0);
    ourpid = svcSysGetPid();			// Get our process ID
}


//**********************************************************
// Function: srvSetup2 - Called by server routine at startup
//				after response is sent
// Returned: Nothing
//**********************************************************

void srvSetup2(void)

{

}


//********************************************************
// Function: srvSetupCmd - Called by server routine before
//				a command is processed
// Returned: Nothing
//********************************************************

void srvSetupCmd(void)

{
	cmdtdb = (long)thdData;
	instance = 0;
}


//********************************************************
// Function: srvFinishCmd - Called by server routine after
//				a command is processed
// Returned: Nothing
//********************************************************

void srvFinishCmd(void)

{

}


//*****************************************************
// Function: srvMessage - Called by server routine when
//				non-server message received
// Returned: Nothing
//*****************************************************

void srvMessage(
    char *msg,
    int   size)

{
	msg = msg;
	size = size;
}


//*******************************************
// Function: cmdadd - Process the ADD command
// Returned: Nothing
//*******************************************

static void cmdadd(
    char *cmd)
{
	IDB  *idb;
	char *pnt;
	long  rtn;
	int   inst;
	char  bufr[256];

	if (!havedisk && mempnt.c != NULL && !checkdisk())
	{
		srvCmdErrorResp(0, "System start up is not complete", NULL, srvMsgDst);
		return;
	}
	logname[0] = 0;
	if ((idb = requireinst(cmd, keywordadd)) == NULL)
		return;
	if (instx & (1 << instance))
	{
		srvCmdErrorResp(0, "Instance already exists", NULL, srvMsgDst);
		return;
	}
	if (logname[0] == 0)
	{
		srvCmdErrorResp(0, "NAME value not specified", NULL, srvMsgDst);
		return;
	}

	if ((pnt = strchr(logname, ':')) == NULL || pnt == (logname + 1))
		strmov(logdev, "XOSACT");
	else
	{
		if ((rtn = pnt - logname) > 20)
		{
			srvCmdErrorResp(0, "Device name is too long", NULL, srvMsgDst);
			return;
		}
		strnmov(logdev, logname, rtn);
		logdev[rtn] = 0;
		strmov(logname, pnt + 1);
	}

	// Create the timer thread if it does not already exist

	if (!havetmr)
	{
	    if ((rtn = thdCtlCreate(TMRTDB + 0x200, 0x3200, 0, tmrthread, NULL,
				0)) < 0)
    	{
			srvCmdErrorResp(rtn, "Error creating timer thread - No logging "
					"will be done", NULL, srvMsgDst);
			return;
		}
		havetmr = TRUE;
	}

	// Create a message fiber which reads IPM messages from clients.
	//   The data area for this fiber contains the IDB.

	ready = 0;
    idb = (IDB *)(((instance - 1) << 20) + 0x200 + FMSGTDB);
    if ((rtn = thdCtlCreate((long)idb, 0x3200, sizeof(IDB), msgthread,
			NULL, 0)) < 0)
    {
		srvCmdErrorResp(rtn, "Error creating message connection thread - "
				"No logging will be done", NULL, srvMsgDst);
		return;
    }
	inst = instance;
	while (ready == 0)
		thdCtlSuspend(NULL, -1);
	if (ready < 0)
	{
		srvCmdErrorResp(ready, errormsgpnt, NULL, srvMsgDst);
		return;
    }
	instx |= (1 << inst);
	sprintf(bufr, STR_MT_FINALMSG"LOGGER: "ver" - Instance %d created\n"
			"        Log name: %s:%s\n", inst, logdev, logname);
	srvCmdResponse(bufr, srvMsgDst);
}


//*************************************************
// Function: fncaddname - Process the NAME argument
//				for the ADD command
// Returned: TRUE if successful, FALSE if error
//*************************************************

static int  fncaddname(
	arg_data *arg)

{
	if (arg->length > 28)
	{
		srvCmdErrorResp(0, "Log NAME is too long", NULL, srvMsgDst);
		return (FALSE);
	}
	strcpy(logname, arg->val.s);
	return (TRUE);
}


//*************************************************
// Function: cmdremove - Process the REMOVE command
// Returned: Nothing
//*************************************************

static void  cmdremove(
	char *cmd)

{
	cmd = cmd;
}


//*************************************************
// Function: cmdstatus - Process the STATUS command
// Returned: Nothing
//*************************************************

static void cmdstatus(
    char *cmd)

{
	cchar *cmdpntr[2];
///	IDB   *idb;

	ulong bits;
	int   inst;
	int   cnt;
	char  bufr[256];

	static char insthead[] = STR_MT_INTRMDMSG"\n  Inst Tcp Device   IP "
			"Address   Cmd Port Data Port Login Clients";

    cmdpntr[0] = cmd;
	cmdpntr[1] = NULL;
    if (!procarg(cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keywordins, NULL,
		    srvCmdError, NULL, NULL))
		return;
    banner();
    if (instance == 0)
    {
		srvCmdResponse(insthead, srvMsgDst);
		bits = instx;
		inst = 1;
		cnt = 0;
		while ((bits >>= 1) != 0)
		{
			if (bits & 0x01)
			{
///				showinst((IDB *)((inst << 27) + 0x200));
				cnt++;
			}
			inst++;
		}
		sprintf(bufr, STR_MT_FINALMSG"   There %s %d instance%s", 
				(cnt == 1) ? "is" : "are", cnt, (cnt == 1) ?
				"" : "s");
	    srvCmdResponse(bufr, srvMsgDst);
	}
}

/*
static void showinst(
	IDB *idb)

{
	char bufr[256];

	sprintf(bufr, STR_MT_INTRMDMSG"   %3d %-12s %-15s %5d %9d   %s %7d",
			idb->instance, idb->devname, fmtaddr(idb->ipaddr.c),
			idb->cmdport.v, idb->dataport.v, (idb->nologin) ? " No" : "Yes",
			idb->numclients);
    srvCmdResponse(bufr, srvMsgDst);
}
*/


//******************************************************
// Function: fncinstance - Process the INSTANCE argument
//				for all commands
// Returned: TRUE if successful, FALSE if error
//******************************************************

static int  fncinstance(
	arg_data *arg)

{
    instance = arg->val.n;
	return (TRUE);
}


//***********************************************************
// Function: requireinst - Do initial command processing when
//		an instance must be specified
// Returned: Address of IDB if OK, NULL if error
//***********************************************************

static IDB *requireinst(
    char     *cmd,
    arg_spec *keyword)

{
	cchar *cmdpntr[2];

    cmdpntr[0] = cmd;
	cmdpntr[1] = NULL;
    if (!procarg(cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keyword, NULL,
		    srvCmdError, NULL, NULL))
		return (NULL);
    banner();
    if (instance == 0)
    {
		srvCmdErrorResp(0, "No instance specified", NULL, srvMsgDst);
		return (NULL);
    }
    if (instance > INSTMAX)		// Valid instance?
    {
		srvCmdErrorResp(0, "Invalid instance number", NULL, srvMsgDst);
		return (NULL);
    }
    return ((IDB *)((instance << 27) + 0x200));
}


//**********************************************
// Function: banner - Display the initial banner
// Returned: Nothing
//**********************************************

static void banner(void)

{
    char bufr[80];

    if (instance != 0)
		sprintf(bufr, STR_MT_INTRMDMSG"LOGGER: "ver" - Unit %d, Instance %d",
				srvUnitNum, instance);
    else
		sprintf(bufr, STR_MT_INTRMDMSG"LOGGER: "ver" - Unit %d", srvUnitNum);
    srvCmdResponse(bufr, srvMsgDst);
}


//*********************************************
// Function: giveinstance - Give up an instance
// Returned: Nothing
//*********************************************

void giveinstance(
	IDB *idb)

{
	idb = idb;

}


//*******************************************
// Function: tmrthread - Main thread function
//				for the timer thread
// Returned: Never returns
//*******************************************

static void tmrthread(void)

{
	IDB *idb;
	long rtn;

	while (TRUE)
	{
		if (havedisk)
		{
			idb = firstidb;
			while (idb != NULL)
			{
				if (idb->commit)
				{
					idb->commit = FALSE;
					if ((rtn = svcIoCommit(idb->filehndl)) < 0)
					{
						svcIoClose(0, idb->filehndl);
						idb->filehndl = rtn;
					}
				}
				idb = idb->next;
			}
		}
		thdCtlSuspend(NULL, ST_SECOND);
	}
}


//**********************************************************
// Function: msgthread - Main thread function for the thread
//				that reads IPM messages
// Returned: Never returns
//**********************************************************

static void msgthread(void)

{
	time_s bgndt;
	IDB   *idb;
	char  *pnt;
	long   rtn;
	char   buffer[300];
	char   msgbfr[2000];
	char   srcbfr[64];
	char   filename[48];
	struct
	{	LNGSTRPARM src;
		uchar      end;
	}      msginpparms =
			{{(PAR_GET|REP_STR), 0, IOPAR_MSGRMTADDRR, NULL, 64, 0}};

	msginpparms.src.buffer = srcbfr;
	idb = (IDB *)thdData;
	strcpy(idb->logdev, logdev);
    strcpy(idb->logname, logname);
	idb->instance = instance;

	if ((pnt = strchr(logname, ':')) != NULL)
		pnt++;
	else
		pnt = logname;

	sprintf(filename, "IPM:SYS^%s^%s^LOG", idb->logdev, idb->logname);
	strupper(filename + 8);
	if ((idb->msghndl = svcIoOpen(XO_IN|XO_OUT, filename, NULL)) < 0)
	{
		errormsgpnt = "Error opening IPM device - No logging will be done";
		ready = idb->msghndl;
		thdCtlWake(cmdtdb);
		thdCtlTerminate();
	}
    svcSysDateTime(T_GTSYSDTTM, &bgndt); // Get current date and time
	sprintf(filename, "%s:%s.log", logdev, logname);

	if ((rtn = svcLogGet(0xFF, "XOSSYS:", buffer, sizeof(buffer))) < 0 ||
			strcmp("OORDSK:", buffer) == 0)
	{
		// Here if still using the once-only ramdisk. We must buffer log
		//   messages until we have a real disk.

		setupmem(idb, filename);
	}
	else
	{
		// Here if have a real disk. No initial buffering is needed.

		setupdisk(idb, filename);
	}
	putinlog(idb, ourpid, prgname, "Logging started");

	// Here with everything ready - link this IDB into the timer list and let
	//   the command thread continue

	idb->next = firstidb;
	firstidb = idb;
	ready = 1;
	thdCtlWake(cmdtdb);
	while (TRUE)
	{
		if ((rtn = thdIoInBlockP(idb->msghndl, msgbfr, sizeof(msgbfr),
				&msginpparms)) < 0)		// Get a message
		{
#if DEBUGFILE
			if (debugfile != NULL)
			{
				fprintf (debugfile, "### ? IPM input error: %d\n", rtn);
				fflush(debugfile);
			}
#endif
			continue;
		}
#if DEBUGFILE
		if (debugfile != NULL)
		{
			fprintf(debugfile, "### Input message:\n");
			debugdump(msgbfr, rtn);
			fflush(debugfile);
		}
#endif
		if (rtn < 12)
		{
			continue;
		}
		if (msgbfr[0] != MT_SYSLOG)
		{
#if DEBUGFILE
			if (debugfile != NULL)
			{
				fprintf(debugfile, "### ? Invalid IPM message type: %d\n",
						msgbfr[0]);
				fflush(debugfile);
			}
#endif
			continue;
		}
#if DEBUGFILE
		if (debugfile != NULL)
		{
			fprintf(debugfile, "### Have IPM message: len=%d, func=%d\n", rtn,
					msgbfr[1]);
			fflush(debugfile);
		}
#endif
		msgbfr[rtn] = '\0';
		switch (msgbfr[1])
		{
		 case 1:							// Event at address
			sprintf(buffer, "%4.4s event, Address = %04.4X:%08.8lX, "
					"Data = %08.8lX", &(msgbfr[12]), *((short *)&srcbfr[3]),
					*((short *)&srcbfr[1]), *((short *)&msgbfr[20]),
					*((long *)&msgbfr[16]), *((long *)&msgbfr[22]));
	        goto putin;

		 case 2:							// Message from process
			sprintf(buffer, "Message from process \"%.160s\"", &msgbfr[12]);
			goto putin;

		 case 3:							// General message
			if (*(long *)&srcbfr[1] == 0x00010001)
				putinlog(idb, *((long *)&msgbfr[12]), &msgbfr[4], &msgbfr[16]);
			else
				putinlog(idb, *((long *)&srcbfr[1]), &msgbfr[4], &msgbfr[12]);
			break;

		 case 0xFF:							// Roll log file
			if (rtn > 12 && msgbfr[12] != 0)
				putinlog(idb, *((long *)&srcbfr[1]), &msgbfr[4], &msgbfr[12]);
			rolllog(idb, idb->logdate, "Requested");
			break;

		 default:
			sprintf(buffer, "Illegal log message #%d\r\n", &msgbfr[1]);
		 putin:
			putinlog(idb, *((long *)&srcbfr[1]), &msgbfr[4], buffer);
			break;
		}
	}
}


//*********************************************
// Function: rolllog - Renames current log file
//				and creates new log file
// Returned: Nothing
//*********************************************

void rolllog(
	IDB   *idb,
	time_s date,
	char  *text)

{
	time_s dt;
	char  *pnt;
	char   extension[16];
	char   logname[128];
	char   arcname[128];
	char   bufr[300];

	dt = date;
	idb->logdate.dt = 0x1FFFFFFFFFFFFFFF; // Needed to keep putinlog from going
										  //   endlessly recurive!
	sprintf(bufr, "Log file terminated: %s", text);
	putinlog(idb, ourpid, prgname, bufr);

	sdt2str(extension, "%L-%r-%d", (time_sz *)&dt);

	sprintf(logname, "%s:%s.log", idb->logdev, idb->logname);
	pnt = arcname + sprintf(arcname, "%slog.%s", idb->logname, extension);
	svcIoClose(idb->filehndl, 0);
	while ((idb->filehndl = svcIoRename(XO_FAILEX, logname, arcname, NULL)) < 0)
	{
		if (pnt[0] == 0)
		{
			pnt[0] = 'a';
			pnt[1] = 0;
		}
		else if (pnt[0] == 'z')
			return;
		else
			(pnt[0])++;
	}
	if ((idb->filehndl = svcIoOpen(XO_IN|XO_OUT|XO_CREATE|XO_EXWRITE, logname,
			(char *)&loparms)) > 0)
	{
		sprintf(bufr, "Log file created: %s", text);
		putinlog(idb, ourpid, prgname, bufr);
	}
	idb->logdate = loparms.dt.value;
}


//***************************************************
// Function: putinlog - Write data prefix to log file
// Returned: TRUE if OK, FALSE if error
//***************************************************

void putinlog(
	IDB  *idb,
	long  pid,
	char *label,
	char *text)

{
	time_sz dt;

#if DEBUGFILE
	if (debugfile != NULL)
		fprintf(debugfile, "### in putinlog: filehndl=%d pid=%08X label=%.8s "
			"text=%s\n", idb->filehndl, pid, label, text);
#endif
	if (idb->filehndl < 0)
		return;
	svcSysDateTime(T_GTSYSDTTM, &dt);	// Get current date and time
	if (!havedisk && !checkdisk())
		putinmem(&dt, pid, label, text);
	else
		puttodisk(idb, &dt, pid, label, text);
}


void puttodisk(
	IDB     *idb,
	time_sz *dt,
	long     pid,
	char    *label,
	char    *text)
{
	char *dpnt;
	char *spnt;
	long  curdays;
	long  logdays;
	long  rtn;
	char  bufr[450];
	char  chr;

#if DEBUGFILE
	if (debugfile == NULL)
	{
		debugfile = fopen("\\logger.log", "w");
		fflush(debugfile);
	}
#endif
	curdays = svcSysDateTime(T_CVSYS2DAYS, dt);
	logdays = svcSysDateTime(T_CVSYS2DAYS, &idb->logdate);
	if (logdays < curdays)
		rolllog(idb, idb->logdate, "Date changed");
	dpnt = bufr + sdt2str(bufr, "%D-%3n-%y %H:%m:%s.%f", dt);
	dpnt += sprintf(dpnt, " %5d.%-3d %-8.8s ", pid >> 16, pid&0xFFFF, label);
	spnt = text;
	while ((chr = *spnt++) != 0 && dpnt < (bufr + 396))
	{
		if (chr == '\r')
		{
			if (*spnt == '\n')
				spnt++;
			if ((chr = *spnt) != 0 && chr != '\r' && chr != '\n')
				dpnt = strmov(dpnt, "\r\n\t\t\t\t\t  ");
		}
		else if (chr == '\n')
		{
			if (*spnt == '\r')
				spnt++;
			if ((chr = *spnt) != 0 && chr != '\r' && chr != '\n')
				dpnt = strmov(dpnt, "\r\n\t\t\t\t\t  ");
		}
		else
			*dpnt++ = chr;
	}
	*dpnt++ = '\r';
	*dpnt++ = '\n';
	if ((rtn = svcIoOutBlock(idb->filehndl, bufr, dpnt - bufr)) < 0)
		idb->filehndl = rtn;
	else
		idb->commit = TRUE;
}


//======================================================================
// Following functions are used at start up only - they really should be
//   in a separate msect and be given up once they are no longer needed!
//======================================================================

// Only one log file can be deferred at startup. A second attempt to set up
//   a log file before we have a real disk will fail.

//****************************************************
// Function: setupdisk - Open (or create) the log file
// Returned: Nothing
//****************************************************

void setupdisk(
	IDB  *idb,
	char *filename)
{
	while ((idb->filehndl = svcIoOpen(XO_IN|XO_OUT|XO_EXWRITE|XO_APPEND,
			filename, (char *)&loparms)) < 0)
	{
		if (idb->filehndl == ER_FILNF)
		{
			if ((idb->filehndl = svcIoOpen(XO_IN|XO_OUT|XO_CREATE|XO_EXWRITE,
					filename, (char *)&loparms)) > 0)
				break;
		}
		errormsgpnt = (idb->filehndl == ER_WPRER) ?
				"Log file is not writable - No logging will be done" :
				"Error opening log file - No logging will be done";
		ready = idb->filehndl;
		thdCtlWake(cmdtdb);
		thdCtlTerminate();
	}
	havedisk = TRUE;
	idb->logdate = loparms.dt.value;
}


//************************************************************
// Function: setupmem - Initialize our temporary memory buffer
// Returned: Nothing
//************************************************************

void setupmem(
	IDB  *idb,
	char *filename)
{
	long rtn;

	if ((rtn = svcMemChange((char *)MEMBUFR, PG_READ|PG_WRITE, 0x1000)) < 0)
	{
		errormsgpnt = "Cannot allocate buffer for defered messages";
		ready = rtn;
		thdCtlWake(cmdtdb);
		thdCtlTerminate();
	}
	*(IDB **)MEMBUFR = idb;
	strncpy((char *)MEMBUFR + 4, filename, 44);
	mempnt.c = ((char *)MEMBUFR) + 48;
	memtop = ((char *)MEMBUFR) + rtn;
}


//*********************************************************************
// Function: checkdisk - Check to see if we have a real disk and, if
//				so, switch to using it
// Returned: TRUE if have a real disk, FALSE if still using the ramdisk
//*********************************************************************

int checkdisk(void)
{
	long rtn;
	IDB *idb;
	union
	{	char   *c;
		MEMMSG *m;
	}    pnt;
	char buffer[128];

	if ((rtn = svcLogGet(0xFF, "XOSSYS:", buffer, sizeof(buffer))) < 0 ||
			strcmp("OORDSK:", buffer) == 0)
		return (FALSE);
	idb = *(IDB **)MEMBUFR;
	setupdisk(idb, (char *)MEMBUFR + 4);
	pnt.m = (MEMMSG *)(MEMBUFR + 48);
	while (pnt.m < mempnt.m && pnt.m->id == 0xAA55)
	{
		puttodisk(idb, (time_sz *)&pnt.m->dt, pnt.m->pid, pnt.m->label,
				pnt.m->text);
		pnt.c += (offsetof(MEMMSG, text) + pnt.m->len);
	}
	svcMemChange((char *)MEMBUFR, 0, 0); // Give up the buffer
	return (TRUE);
}


//*******************************************************************
// Function: putinmem - Store log text in our temporary memory buffer
// Returned: Nothing
//*******************************************************************

void putinmem(
	time_sz *dt,
	long     pid,
	char    *label,
	char    *text)
{
	int  len;
	long rtn;

	len = strlen(text);
	text[len] = 0;
	len++;
	if ((mempnt.c + offsetof(MEMMSG, text) + len) <= memtop)
	{
		if ((rtn = svcMemChange((char *)MEMBUFR, PG_READ|PG_WRITE,
				mempnt.c - ((char *)MEMBUFR) + len)) < 0)
			return;
		memtop = ((char *)MEMBUFR) + rtn;
	}
	mempnt.m->id = 0xAA55;
	mempnt.m->len = len;
	mempnt.m->pid = pid;
	mempnt.m->dt = *(time_s *)dt;
	strncpy(mempnt.m->label, label, 8);
	memcpy(mempnt.m->text, text, len);
	mempnt.c += (offsetof(MEMMSG, text) + len);
}


#if DEBUGFILE
static void debugdump(
	char *msg,
	int   size)
{
    char *begin;
    char *linepnt;
	int   offset;
	int   ncnt;
	int   acnt;
	int   fill;
    char  strbuf[256];			// String buffer
    char  chr;

	offset = 0;
	while (size > 0)
	{
		linepnt = strbuf + sprintf(strbuf, "%4.4X> ", offset);
		offset += 16;

		ncnt = 16;
		if (ncnt > size)
			ncnt = (int)size;
		size -= ncnt;
		acnt = ncnt;
		fill = 16 - ncnt;
		begin = msg;
		while (--ncnt >= 0)
			linepnt += sprintf(linepnt, "%2.2X ", *msg++);
		while (--fill >= 0)
			linepnt = strmov(linepnt, "   ");
		linepnt = strmov(linepnt, "  | ");
		while (--acnt >= 0)
		{
			chr = *begin++ & 0x7F;
			*linepnt++ = isprint(chr)? chr: '.';
		}
		strmov(linepnt, " |\n");
		fputs(strbuf, debugfile);
	}
}
#endif
