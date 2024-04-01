//=======================================
// XFPSRV - XOS remote file system server
// Written by John Goltz
//=======================================

#include "xfpsrv.h"

// This program is a server which provides access to an MMS inventory master
//   file.  It handles requestes to read and write the file and implements the
//   compression and weekly update functions. It is initialized as a symbiont
//   with the following command:

//	SYMBIONT XFPSRV UNIT=n DEBUGLEVEL=l DEBUGFILE=log
//	  Where:
//		UNIT       = Server unit number (default = 1)
//		DEBUGLEVEL = Specifies the logging level, default is 3 if
//			       DEBUGLEVEL is specified, 0 otherwise
//				 0 = No logging
//				 1 = Log major events
//				 2 = Log all network messages
//		DEBUGFILE  = Specifies file for log output, if not specified
//			       no log output is generated

// The operation of the program is controlled using SERVER commands.  The
//   following commands are supported:
//	SERVER XFPSRV {unit} ADD INS=ins {LIMIT=limit} {CLIENT=max}
//		where:	unit  = Unit number (defaut is 1)
//			limit = CONLIMIT value for the TCP device (default is
//				      32 if not specified)
//			max   = Maximum number of client threads (default is
//				      100 if not specified)
//	SERVER XFPSRV {unit} STATUS
//		where:	unit = Unit number (defaut is 1)
//		This command reports current status of the server.

// Each copy of XFPSRV running on a system must have a unique unit number.
//   The actual connections between network TCP devices and the server are made
//   after the server is loaded using the SERVER utility. Even though this
//   server uses the standard XOS server structure it only supports a single
//   server instance. This is mainly because of the large amount of address
//   space (and to a lesser extent memory) that may be required if the
//   database is large.  Given the potential size of the databse it seems
//   better to require a seperate process for each database.

// Description of data structures used:

//   Since only a single instance is supported, no ICBs are needed or used.
//   The initial server command TDB is located at 0x60000000. This leaves the
//   space between 0x200000 and 0x5FFFFFFF (1534MB) for program data including
//   the heap. The in-memory index data blocks are allocated in this area.
//   The additional TDBs are allocated as follows:
//     0x40004000 - TCP connection thread
//     0x40008000 - File access thread
//	   0x4000C000 - Timer thread
//     0x40010000 - First client thread
//   Additional client thread TDBs are allocated every 0x4000 bytes.
//   The index pointer tables are located starting at 0x80000000. Each table
//   is allocated 0x1000000 (256MB) of address space, allowing up to 128
//   tables.

// Memory usage for this program:
//  0x00001000  Code segment
//  0x00200000  Main stack segment
//  0x00220200  Data segment (includes malloc heap)
//	0x20000000  Command TDB (CMDTDB)
//  0x20010000  Connection TDB (CONTDB)
//  0x20020000  Timer TDB (TMRTDB)
//  0x20080000  Client TDBs (each TDB uses 0x80000 bytes)

#define MAJVER  1
#define MINVER  0
#define EDITNUM 0

///time_s onesec = {ST_SECOND, 0};

uchar msgoutbufr[16];

ulong instance;
char *cmdpntr[2];
CDB **clienttbl;

long  clientfree;
long  clientcnt;

long  tcphndl;
long  conlimit;
long  clientlimit;
long  tcpport;

char  prgname[] = "XFPSRV";
char  srvname[] = "XFPSRV";

char  tcpdev[32];
char  phyname[20];

uchar debug = FALSE;

///static struct
///{   byte4_parm length;
///    char       end;
///} fileoparms =
///{  {PAR_GET|REP_HEXV, 4, IOPAR_LENGTH}
///};

///static struct
///{   byte4_parm pos;
///    char       end;
///} fileinparms =
///{  {PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS}
///};

static struct
{   text8_parm  class;
    byte4_parm  filoptn;
    lngstr_parm filspec;
    byte4_parm  tcpport;
    byte4_parm  conlimit;
    char        end;
} tcpoparms =
{   {PAR_GET|REP_TEXT, 8, IOPAR_CLASS},
    {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_XOSNAME},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, phyname, 0, sizeof(phyname),
        sizeof(phyname)},
    {PAR_SET|REP_HEXV, 4, IOPAR_NETLCLPORT, 0},
    {PAR_SET|REP_HEXV, 4, IOPAR_NETCONLIMIT, 0}
};

static void cmdadd(char *cmd);
static void cmdstatus(char *cmd);
static void cmdterminate(char *cmd);
static void commanderror(long code, char *msg, ...);
static void commandresponse(int type, char *msg, ...);
static int  fncaddclient(arg_data *arg);
static int  fncadddevice(arg_data *arg);
static int  fncaddlimit(arg_data *arg);
static int  fncaddport(arg_data *arg);
static int  fncinstance(arg_data *arg);
static void timerthread(void);

arg_spec srvCmdWord[] =
{   {"ADD"      , 0, NULL, srvFncCommand, (long)cmdadd},
    {"STATUS"   , 0, NULL, srvFncCommand, (long)cmdstatus},
    {"STA"      , 0, NULL, srvFncCommand, (long)cmdstatus},
	{"TERMINATE", 0, NULL, srvFncCommand, (long)cmdterminate},
    {0}
};

arg_spec keywordadd[] =
{   {"INSTANCE" , ASF_VALREQ|ASF_NVAL , NULL, fncinstance , 0},
    {"INS"      , ASF_VALREQ|ASF_NVAL , NULL, fncinstance , 0},
	{"LIMIT"    , ASF_VALREQ|ASF_NVAL , NULL, fncaddlimit , 0},
	{"LIM"      , ASF_VALREQ|ASF_NVAL , NULL, fncaddlimit , 0},
	{"CLIENT"   , ASF_VALREQ|ASF_NVAL , NULL, fncaddclient, 0},
	{"CLI"      , ASF_VALREQ|ASF_NVAL , NULL, fncaddclient, 0},
	{"TCPDEVICE", ASF_VALREQ|ASF_LSVAL, NULL, fncadddevice, 0},
	{"TCPDEV"   , ASF_VALREQ|ASF_LSVAL, NULL, fncadddevice, 0},
	{"PORT"     , ASF_VALREQ|ASF_NVAL , NULL, fncaddport  , 0},
	{"POR"      , ASF_VALREQ|ASF_NVAL , NULL, fncaddport  , 0},
	{0}
};

arg_spec keywordsts[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {0}
};

void mainalt(
    char *args)

{
	srvInitialize(args, (void *)CMDTDB, 0x2000, 70, 50, MAJVER, MINVER,
			EDITNUM);
}


void srvSetup1(void)

{
	debug = (srvReqName[0] == 0);
}

void srvSetup2(void)

{

}

void srvSetupCmd(void)

{
	instance = 0;
}

void srvFinishCmd(void)

{

}

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
    long   rtn;
	int    cnt;
    char   bufr[1024];

	conlimit = 32;
	clientlimit = 100;
	tcpport = TCPP_XFP;
    cmdpntr[0] = cmd;
    if (!procarg(cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keywordadd, NULL,
			srvCmdError, NULL, NULL))
		return;
	commandresponse(MT_INTRMDMSG, "XFPSRV: Version %d.%d.%d, Unit %d", MAJVER,
		MINVER, EDITNUM, srvUnitNum);
	if (instance > 1)
	{
		sprintf(bufr, "Illegal instance number (%d) specified - Cannot "
				"continue", instance);
		srvCmdErrorResp(0, bufr, NULL, srvMsgDst);
		return;
	}
///	if (filehndl > 0)
///	{
///		srvCmdErrorResp(0, "Instance 1 exists - Cannot continue", NULL,
///				srvMsgDst);
///		return;
///	}
	if (tcpdev[0] == 0)
	{
		srvCmdErrorResp(0, "No TCP device specified - Cannot continue", NULL,
				srvMsgDst);
		return;
	}
    tcpoparms.tcpport.value = tcpport;
    tcpoparms.conlimit.value = conlimit;
    if ((tcphndl = svcIoOpen(XO_IN|XO_OUT, tcpdev, &tcpoparms)) < 0)
    {
		commanderror(tcphndl, "Cannot open device %s - Cannot "
				"continue", tcpdev);
		return;
    }
    if (strcmp(tcpoparms.class.value, "TCP") != 0)
    {
		commanderror(rtn, "Device %s is not a TCP device - Cannot continue",
				tcpdev);
		svcIoClose(tcphndl, 0);
		return;
    }

    // Allocate and set up the client pointer table. Each free entry in the
    //   table contains the index of the next free entry. We use the fact
    //   all data space addresses are greater than 0x200000 to identify
    //   slots that are in use.

    if ((clienttbl = (CDB **)sbrk((clientlimit + 1) * sizeof(CDB *))) == NULL)
    {
		srvCmdErrorResp(0, "Cannot allocate memory", NULL, srvMsgDst);
		svcIoClose(tcphndl, 0);
		return;
    }
    cnt = 0;
    while (cnt < clientlimit)
    {
		clienttbl[cnt] = (CDB *)(cnt + 1);
		cnt++;
    }
    clienttbl[clientlimit] = 0;
    clientfree = 1;

    // Create the timer thread

    if ((rtn = thdCtlCreate(TMRTDB, 0x2000-sizeof(THDDATA), sizeof(THDDATA),
			timerthread, NULL, 0)) < 0)
    {
		commanderror(0, "Error creating timer thread - Cannot continue");
		svcIoClose(tcphndl, 0);
		return;
    }
    // Create the TCP connection thread which accepts connections for clients.

    if ((rtn = thdCtlCreate(CONTDB, 0x2000-sizeof(THDDATA), sizeof(THDDATA),
			tcpthread, NULL, 0)) < 0)
    {
		commanderror(0, "Error creating TCP connection thread - Cannot "
				"continue");
		svcIoClose(tcphndl, 0);
		return;
    }

    // Start-up is complete. Send the final message to whoever started us.

    srvCmdResponse(STR_MT_FINALMSG"XFPSRV: Startup is complete", srvMsgDst);
}


//*************************************************
// Function: cmdstatus - Process the STATUS command
// Returned: Nothing
//*************************************************

static void cmdstatus(
    char *cmd)

{
	CDB *cdb;
	int  cnt;
	int  slot;
	char text[32];

    cmd = cmd;

    commandresponse(MT_INTRMDMSG, "FTPSRV: Version %d.%d.%d, Unit %d", MAJVER,
			MINVER, EDITNUM, srvUnitNum);

///	if (filehndl <= 0)
///	{
///	    srvCmdResponse(STR_MT_FINALERR "        No instance has been created.",
///				srvMsgDst);
///		return;
///	}

	if (clientcnt == 0)
	    srvCmdResponse(STR_MT_FINALMSG"        There are no client sessions",
				srvMsgDst);
	else
	{
	    commandresponse(MT_INTRMDMSG, "        There %s %d client session%s\n"
				"        Slot IP Address      Port",
				(clientcnt == 1) ? "is" : "are", clientcnt, (clientcnt == 1) ?
				"" : "s");
		cnt = clientcnt;
		slot = 1;
		do
		{
			if (clienttbl[slot] >= (CDB *)FIRSTCDB)
			{
				cdb = clienttbl[slot];
				sprintf(text, "%u.%u.%u.%u", cdb->ipaddr.c[0], cdb->ipaddr.c[1],
						cdb->ipaddr.c[2], cdb->ipaddr.c[3]);

			    commandresponse((--cnt > 0) ? MT_INTRMDMSG : MT_FINALMSG,
						"        %-4d %-15s %d", slot, text, cdb->port);
			}
		} while (cnt > 0 && ++slot <= clientlimit);
	}
}


//*******************************************************
// Function: cmdterminate - Process the TERMINATE command
// Returned: Nothing
//*******************************************************

static void cmdterminate(
	char *cmd)

{
///	CDB  *cdb;
///	int   loop;
///	char  text[32];

    cmd = cmd;

	exit(0);
}


//******************************************************************
// Function: fncinstance - Process INSTANCE keyword for all commands
// Returned: TRUE if OK, FALSE if error
//******************************************************************

static int fncinstance(
    arg_data *arg)

{
    instance = arg->val.n;
    return (TRUE);
}


//**************************************************************
// Function: fncaddlimit - Process LIMIT keyword for ADD command
// Returned: TRUE if OK, FALSE if error
//**************************************************************

static int fncaddlimit(
    arg_data *arg)

{
    conlimit = arg->val.n;
    return (TRUE);
}


//****************************************************************
// Function: fncaddclient - Process CLIENT keyword for ADD command
// Returned: TRUE if OK, FALSE if error
//****************************************************************

static int fncaddclient(
    arg_data *arg)

{
    clientlimit = arg->val.n;
    return (TRUE);
}


//************************************************************
// Function: fncaddport - Process PORT keyword for ADD command
// Returned: TRUE if OK, FALSE if error
//************************************************************

static int fncaddport(
    arg_data *arg)

{
    tcpport = arg->val.n;
    return (TRUE);
}


//****************************************************************
// Function: fncadddevice - Process DEVICE keyword for ADD command
// Returned: TRUE if OK, FALSE if error
//****************************************************************

static int fncadddevice(
    arg_data *arg)

{
	char *pnt;

	if (arg->length > 28)
	{
		srvCmdErrorResp(0, "DEVICE name is too long", NULL, srvMsgDst);
		return (FALSE);
	}
	strcpy(tcpdev, arg->val.s);

	pnt = tcpdev + arg->length - 1;
	if (*pnt++ != ':')
		*pnt++ = ':';
	*pnt = 0;
	return (TRUE);
}


static void commanderror(
	long  code,
	char *msg,
	...)

{
	va_list pi;
	char    text[256];

    va_start(pi, msg);
    vsprintf(text, msg, &va_arg(pi, long));
	srvLogSysLog(code, text);
	srvCmdErrorResp(code, text, NULL, srvMsgDst);
}


static void commandresponse(
	int   type,
	char *msg,
	...)

{
	va_list pi;
	char    text[512];
	char    bufr[512];

    va_start(pi, msg);
    vsprintf(text, msg, &va_arg(pi, long));
	srvLogSysLog(0, text);
	sprintf(bufr, "%cFTPSRV: %s", type, text);
	srvCmdResponse(bufr, srvMsgDst);
}


//******************************************************************
// Function: timerthread - Main thread function for the timer thread
// Returned: Never returns
//******************************************************************

static void timerthread(void)

{
	while (TRUE)
	{

		thdCtlSuspendT(NULL, 0, ST_SECOND);
	}
}


void logerror(
	long  code,
	char *msg,
	...)

{
	va_list pi;
	char    text[256];
	char    errtxt[128];

    va_start(pi, msg);
    vsprintf(text, msg, &va_arg(pi, long));
	srvLogSysLog(code, text);
	if (debug)
	{
		if (code < 0)
		{
			svcSysErrMsg(code, 0x03, errtxt);
			printf("? %s\n  %s\n", text, errtxt);
		}
		else
			printf("? %s\n", text);
	}
}
