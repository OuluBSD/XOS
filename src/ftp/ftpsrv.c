//--------------------------------------------------------------------------*
// FTPSRV.C
// FTP server for XOS
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
#include "ftpsrv.h"

// This program is the FTP file server.  It is initialized as a symbiont with
//   the following command:

//	SYMBIONT FTPSRV UNIT=n LOGLEVEL=l LOGFILE=log
//	  Where:
//		UNIT     = Server unit number (default = 1)
//		LOGLEVEL = Specifies the logging level, default is 3 if LOGFILE
//			   is specified, 0 otherwise
//				0 = No logging
//				1 = Log major events
//				2 = Log all network messages
//		LOGFILE  = Specifies file for log output, if not specified no
//			   log output is generated

// Each copy of XFPSRV running on a system must have a unique unit number.  The
//   actual connections between network RCP devices and the server are made
//   after the server is loaded using the SERVER utility.

// Description of data structures used:

//   The server can support up to 31 server instances.  Each instance is
//   associated with an RCP unit/port number pair and can support up to 4088
//   open files (in theory!).  For each instance an IDB is allocated (at the
//   base of a 16MB address space which is reserved for the IDB and the
//   associated CDBs) and contains all data needed to operate the instance.
//   The IDB includes a table of pointers to each possible CDB.  The size of
//   this table is equal to the maximum number of open files allowed.  CDBs
//   (each of which is 4KB) are only allocated when a file is opened.  Each
//   one is allocated as a separate 1 page msect.

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

int  slot;
int  instance;
long instx;

char devname[32];
char phyname[32];
long cmdport;
long dataport;
int  conlimit;
int  maxclients;
long tcphndl;
long cmdtdb;

VERSION xosver;

long version = MAJVER;
long editnum = EDITNUM;

static void  banner(void);
static void  cmdadd(char *cmd);
static void  cmdend(char *cmd);
static void  cmdstatus(char *cmd);
static char *fmtaddr(uchar *pnt);
static int   fncaddcmdport(arg_data *arg);
static int   fncadddataport(arg_data *arg);
static int   fncadddevice(arg_data *arg);
static int   fncaddlimit(arg_data *arg);
static int   fncaddmaximum(arg_data *arg);
static int   fncaddnologin(arg_data *arg);
static int   fncendslot(arg_data *arg);
static int   fncinstance(arg_data *arg);
static IDB  *requireinst(char *cmd, arg_spec *keyword);
static void  showinst(IDB *idb);

arg_spec srvCmdWord[] =
{   {"ADD"   , 0, NULL, srvFncCommand, (long)cmdadd},
    {"STATUS", 0, NULL, srvFncCommand, (long)cmdstatus},
    {"STA"   , 0, NULL, srvFncCommand, (long)cmdstatus},
    {"END"   , 0, NULL, srvFncCommand, (long)cmdend},
    {0}
};

arg_spec keywordadd[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL , NULL, fncinstance   , 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL , NULL, fncinstance   , 0},
	{"DEVICE"  , ASF_VALREQ|ASF_LSVAL, NULL, fncadddevice  , 0},
	{"DEV"     , ASF_VALREQ|ASF_LSVAL, NULL, fncadddevice  , 0},
	{"CMDPORT" , ASF_VALREQ|ASF_NVAL , NULL, fncaddcmdport , 0},
	{"CMD"     , ASF_VALREQ|ASF_NVAL , NULL, fncaddcmdport , 0},
	{"DATAPORT", ASF_VALREQ|ASF_NVAL , NULL, fncadddataport, 0},
	{"DAT"     , ASF_VALREQ|ASF_NVAL , NULL, fncadddataport, 0},
	{"LIMIT"   , ASF_VALREQ|ASF_NVAL , NULL, fncaddlimit   , 0},
	{"LIM"     , ASF_VALREQ|ASF_NVAL , NULL, fncaddlimit   , 0},
	{"MAXIMUM" , ASF_VALREQ|ASF_NVAL , NULL, fncaddmaximum , 0},
	{"MAX"     , ASF_VALREQ|ASF_NVAL , NULL, fncaddmaximum , 0},
	{"NOLOGIN" , 0                   , NULL, fncaddnologin , 0},
	{0}
};

arg_spec keywordsts[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {0}
};

arg_spec keywordend[] =
{   {"INSTANCE", ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {"INS"     , ASF_VALREQ|ASF_NVAL, NULL, fncinstance, 0},
    {"SLOT"    , ASF_VALREQ|ASF_NVAL, NULL, fncendslot , 0},
    {"SLO"     , ASF_VALREQ|ASF_NVAL, NULL, fncendslot , 0},
    {0}
};

char xosrootstr[20];
int  xosrootlen;

cstr nullmsg = {0, NULL};

DEFCSTR(msg150a, ("150 Opening ASCII data connection for directory "\
		"listing\r\n"));
DEFCSTR(msg200 , ("200 Command OK\r\n"));
DEFCSTR(msg202a, ("202 User account not needed, command ignore\r\n"));
DEFCSTR(msg202b, ("202 Allocation not needed, command ignored\r\n"));
DEFCSTR(msg202c, ("202 No site specific command are implemented, command "\
		"ignored\r\r"));
DEFCSTR(msg220a, ("220 FTP (XOS) Please log in\r\n"));
DEFCSTR(msg220b, ("220 FTP (XOS) Ready\r\n"));
DEFCSTR(msg220c, ("220 Connection reset: Please log in\r\n"));
DEFCSTR(msg220d, ("220 Connection reset\r\n"));
DEFCSTR(msg221 , ("221 Closing control connection\r\n"));
DEFCSTR(msg226 , ("226 Transfer complete\r\n"));
DEFCSTR(msg230a, ("230 User logged in, proceed\r\n"));
DEFCSTR(msg230b, ("230 User logged in (all-files mode allowed), proceed\r\n"));
DEFCSTR(msg230c, ("230 User logged in (all-files mode enabled), proceed\r\n"));
DEFCSTR(msg250a, ("250 File transfer is complete\r\n"));
DEFCSTR(msg250b, ("250 Directory listing is complete\r\n"));
DEFCSTR(msg250c, ("250 File deleted\r\n"));
DEFCSTR(msg250d, ("250 File renamed\r\n"));
DEFCSTR(msg331 , ("331 User name OK, need password\r\n"));
DEFCSTR(msg350 , ("350 \"Rename From\" name OK, need \"Rename To\" name\r\n"));
DEFCSTR(msg452 , ("452 Not done: Not enough memory available\r\n"));
DEFCSTR(msg500a, ("500 Command unreconnized\r\n"));
DEFCSTR(msg500b, ("500 Syntax error in command\r\n"));
DEFCSTR(msg500c, ("500 No directory specified\r\n"));
DEFCSTR(msg501a, ("501 Invalid file or directory specification\r\n"));
DEFCSTR(msg501b, ("502 Invalid parameter\r\n"));
DEFCSTR(msg502 , ("502 Command not implemented\r\n"));
DEFCSTR(msg503a, ("503 Password specified without user name\r\n"));
DEFCSTR(msg503b, ("503 Invalid user name or password\r\n"));
DEFCSTR(msg503c, ("503 Not done: RNFR did not immediately preceed RNTO\r\n"));
DEFCSTR(msg504a, ("504 Requested mode is not implemented\r\n"));
DEFCSTR(msg530 , ("530 Not logged in\r\n"));
DEFCSTR(msg533 , ("533 Not done: Not allowed\r\n"));
DEFCSTR(msg550a, ("550 Not done: Access not allowed\r\n"));
DEFCSTR(msg550b, ("550 Not done: No file name specified\r\n"));
DEFCSTR(msg550c, ("550 Not done: Wild card characters are not allowed\r\n"));

struct
{	byte4_parm  optn;
	lngstr_parm name;
	uchar       end;
} xdparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_NOPREFIX|XFO_DOSNAME},
	{PAR_GET|REP_STR , 0, IOPAR_FILSPEC, xosrootstr, 0, sizeof(xosrootstr) - 2}
};

TCPOPENPARMS tcpopenparms =
{   {PAR_GET|REP_TEXT, 8, IOPAR_CLASS},
    {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_XOSNAME},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, phyname, 0, sizeof(phyname),
        sizeof(phyname)},
    {PAR_GET|REP_HEXV, 4, IOPAR_NETLCLNETA, 0},
    {PAR_SET|REP_HEXV, 4, IOPAR_NETLCLPORT, 0},
    {PAR_SET|REP_HEXV, 4, IOPAR_NETCONLIMIT, 0}
};

struct
{	byte4_char version;
	uchar      end;
}	verchars =
{	{PAR_GET|REP_VERN, 4, "XOSVER"}
};

char prgname[] = "FTPSRV";
char srvname[] = "FTPSRV";

uchar debugflg;
uchar nologin;


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
	long rtn;

	debugflg = (srvReqName[0] == 0);
	svcIoClsChar("SYSTEM:", &verchars);
	xosver.v = verchars.version.value;
	if ((rtn = svcIoDevParm(XO_RAW, "XOSDEV:", &xdparms)) < 0)
		srvSetupFail(rtn, "Error obtaining name of XOS boot device", NULL);
	xosrootstr[xdparms.name.strlen] = '\\';
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
	conlimit = 10;
	maxclients = 30;
	nologin = FALSE;
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
	long  rtn;
	int   inst;
	char  bufr[256];

	devname[0] = 0;
	cmdport = 21;
	dataport = 20;
	conlimit = 20;
	maxclients = 100;
    if ((idb = requireinst(cmd, keywordadd)) == NULL)
		return;
    if (instx & (1 << instance))
    {
		srvCmdErrorResp(0, "Instance already exists", NULL, srvMsgDst);
		return;
    }
    if (devname[0] == 0)
    {
		srvCmdErrorResp(0, "DEVICE value not specified", NULL, srvMsgDst);
		return;
    }
    strupr(devname);
    tcpopenparms.tcpport.value = cmdport;
    tcpopenparms.conlimit.value = conlimit;
    if ((tcphndl = svcIoOpen(XO_IN|XO_OUT, devname, &tcpopenparms)) < 0)
    {
		sprintf(bufr, "Cannot open TCP device %s", devname);
		srvCmdErrorResp(tcphndl, bufr, NULL, srvMsgDst);
		return;
    }
    if (strcmp(tcpopenparms.class.value, "TCP") != 0)
    {
		sprintf(bufr, "Device %s is not a TCP device", devname);
		srvCmdErrorResp(0, bufr, NULL, srvMsgDst);
		svcIoClose(tcphndl, 0);
		return;
    }

    // Create a TCP connection thread which accepts connections for clients.
	//   The data area for this thread contains the IDB.

    idb = (IDB *)((instance << 27) + 0x200);
    if ((rtn = thdCtlCreate((long)idb, 0x3200, sizeof(IDB), tcpthread,
			NULL, 0)) < 0)
    {
		srvLogSysLog(rtn, "Error creating TCP connection thread - Cannot "
				"continue");
		srvCmdErrorResp(rtn, "Error creating TCP connection thread - Cannot "
				"continue", NULL, srvMsgDst);
		return;
    }
	inst = instance;
	while (instance != 0)
		thdCtlSuspendT(NULL, -1, -1);
	instx |= (1 << inst);
	sprintf(bufr, STR_MT_FINALMSG"FTPSRV: "ver" - Instance %d created\n"
			"        TCP device: %s, Cmd port: %d, Data port: %d\n"
			"        User login is %s", inst, devname, cmdport, dataport,
			(nologin) ? "not used" : "required");
	srvLogSysLog(0, bufr + 9);
	srvCmdResponse(bufr, srvMsgDst);
}


//*****************************************************
// Function: fncadddevice - Process the DEVICE argument
//				for the ADD command
// Returned: TRUE if successful, FALSE if error
//*****************************************************

static int  fncadddevice(
	arg_data *arg)

{
	char *pnt;

	if (arg->length > 28)
	{
		srvCmdErrorResp(0, "DEVICE name is too long", NULL, srvMsgDst);
		return (FALSE);
	}
	strcpy(devname, arg->val.s);

	pnt = devname + arg->length - 1;
	if (*pnt++ != ':')
		*pnt++ = ':';
	*pnt = 0;
	return (TRUE);
}


//*******************************************************
// Function: fncaddcmdport - Process the CMDPORT argument
//				for the ADD command
// Returned: TRUE if successful, FALSE if error
//*******************************************************

static int  fncaddcmdport(
	arg_data *arg)

{
	cmdport = arg->val.n;
	return (TRUE);
}


//*********************************************************
// Function: fncadddataport - Process the DATAPORT argument
//				for the ADD command
// Returned: TRUE if successful, FALSE if error
//*********************************************************

static int  fncadddataport(
	arg_data *arg)

{
	dataport = arg->val.n;
	return (TRUE);
}


//***************************************************
// Function: fncaddlimit - Process the LIMIT argument
//				for the ADD command
// Returned: TRUE if successful, FALSE if error
//***************************************************

static int  fncaddlimit(
	arg_data *arg)

{
	conlimit = arg->val.n;
	if (conlimit < 5)
		conlimit = 5;
	else if (conlimit > 50)
		conlimit = 50;
	return (TRUE);
}


//*******************************************************
// Function: fncaddmaximum - Process the MAXIMUM argument
//				for the ADD command
// Returned: TRUE if successful, FALSE if error
//*******************************************************

static int  fncaddmaximum(
	arg_data *arg)

{
	maxclients = arg->val.n;

	if (maxclients < 10)
		maxclients = 10;
	else if (maxclients > 1200)
		maxclients = 1200;
	return (TRUE);
}


//*******************************************************
// Function: fncaddnologin - Process the NOLOGIN argument
//				for the ADD command
// Returned: TRUE if successful, FALSE if error
//*******************************************************

static int  fncaddnologin(
	arg_data *arg)

{
	arg = arg;

	nologin = TRUE;
	return (TRUE);
}


//*************************************************
// Function: cmdstatus - Process the STATUS command
// Returned: Nothing
//*************************************************

static void cmdstatus(
    char *cmd)

{
	char *cmdpntr[2];
	IDB  *idb;
	CDB  *cdb;
	long *cpnt;
	ulong bits;
	int   slot;
	int   inst;
	int   cnt;
	char  rmtdp[8];
	char  lcldp[8];
	char  condt[20];
	char  bufr[256];

	static char consttbl[] =
	{	' ',					// DATACON_IDLE       = 0
		'P',					// DATACON_WAITING    = 1
		'A',					// DATACON_CONNECTING = 2
		'P',					// DATACON_PASSIVE    = 3
		'A'						// DATACON_ACTIVE     = 4
	};
	static char insthead[] = STR_MT_INTRMDMSG"\n  Inst Tcp Device   IP "
			"Address   Cmd Port Data Port Login Clients";

    cmdpntr[0] = cmd;
	cmdpntr[1] = NULL;
    if (!procarg(cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keywordsts, NULL,
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
				showinst((IDB *)((inst << 27) + 0x200));
				cnt++;
			}
			inst++;
		}
		sprintf(bufr, STR_MT_FINALMSG"   There %s %d instance%s", 
				(cnt == 1) ? "is" : "are", cnt, (cnt == 1) ?
				"" : "s");
	    srvCmdResponse(bufr, srvMsgDst);
	}
	else
	{
		if ((instx & (1 << instance)) == 0)
		{
			srvCmdErrorResp(0, "Instance %d does not exist", NULL, srvMsgDst,
					instance);
			return;
		}
		srvCmdResponse(insthead, srvMsgDst);
	    idb = (IDB *)((instance << 27) + 0x200);
		showinst(idb);
		srvCmdResponse(STR_MT_INTRMDMSG"\n                     Command Remote "
				"Local\n  Slot IP Address       Port   Data  Data  Connected "
				"At    User Name", srvMsgDst);
		slot = 1;
		cnt = 0;
		bufr[0] = 0;
		cpnt = (long *)(idb->cdbtbl);
		do
		{
			if (*cpnt > 10000)
			{
				if (bufr[0] != 0)
				    srvCmdResponse(bufr, srvMsgDst);
				cdb = (CDB *)*cpnt;
				if (cdb->rmtdataport.v == 0)
					strcpy(rmtdp, "  ---");
				else
					sprintf(rmtdp, "%5d", cdb->rmtdataport.v);
				if (cdb->lcldataport.v == 0)
					strcpy(lcldp, "  ---");
				else
					sprintf(lcldp, "%5d", cdb->lcldataport.v);
				sdt2str(condt, "%h:%m %d-%3n-%y", (time_sz *)&(cdb->connectdt));
				sprintf(bufr, STR_MT_INTRMDMSG"   %3d %-15s %5d  %s %s%c %s "
						"%s", slot, fmtaddr(cdb->rmtaddr.c), cdb->rmtcmdport,
						rmtdp, lcldp, consttbl[cdb->datacon], condt,
						cdb->username);
				cnt++;
			}
			cpnt++;
		} while (++slot <= idb->maxclients);
		if (bufr[0] != 0)
		{
			bufr[0] = MT_FINALMSG;
		    srvCmdResponse(bufr, srvMsgDst);
		}
		else
		    srvCmdResponse(STR_MT_FINALMSG"  There are no active clients",
					srvMsgDst);
	}
}


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


static char *fmtaddr(
	uchar *pnt)

{
	static char addr[32];

	sprintf(addr, "%u.%u.%u.%u", pnt[0], pnt[1], pnt[2], pnt[3]);
	return (addr);
}


//*******************************************
// Function: cmdend - Process the END command
// Returned: Nothing
//*******************************************

static void cmdend(
    char *cmd)

{
	IDB *idb;
	CDB *cdb;
	char bufr[128];

	slot = 0;
    if ((idb = requireinst(cmd, keywordend)) == NULL)
		return;
    if ((instx & (1 << instance)) == 0)
    {
		srvCmdErrorResp(0, "Instance does not exists", NULL, srvMsgDst);
		return;
    }
    if (slot == 0)
    {
		srvCmdErrorResp(0, "SLOT value not specified", NULL, srvMsgDst);
		return;
    }
	if ((cdb = idb->cdbtbl[slot - 1]) <= (CDB *)10000)
    {
		srvCmdErrorResp(0, "SLOT %d is not active", NULL, srvMsgDst, slot);
		return;
    }
	cdb->terminate = TRUE;
	canceldevice(cdb->cmdhndl);
	thdCtlWake((long)cdb);
	sprintf(bufr, STR_MT_FINALMSG"Slot %d has been terminated", slot);
    srvCmdResponse(bufr, srvMsgDst);
}


//*************************************************
// Function: fncendslot - Process the SLOT argument
//				for the END command
// Returned: TRUE if successful, FALSE if error
//*************************************************

static int  fncendslot(
	arg_data *arg)

{
	slot = arg->val.n;
	return (TRUE);
}


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
	char *cmdpntr[2];

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
		sprintf(bufr, STR_MT_INTRMDMSG"FTPSRV: "ver" - Unit %d, Instance %d",
				srvUnitNum, instance);
    else
		sprintf(bufr, STR_MT_INTRMDMSG"FTPSRV: "ver" - Unit %d", srvUnitNum);
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
