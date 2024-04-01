//--------------------------------------------------------------------------*
// ptrsrv.c
// Print server for XOS
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
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <xossignal.h>
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

// This program is the XOS printer server. It is used to transfer spooled
//   printer files to a printer. The current version only supports PCL or
//   PCL3GUI printers. A furture version may support additional formats. It
//   is initialized as a symbiont with the following command:

//	SYMBIONT PTRSRV UNIT=n LOGLEVEL=l LOGFILE=log
//	  Where:
//		UNIT     = Server unit number (default = 1)
//		LOGLEVEL = Specifies the logging level, default is 3 if LOGFILE
//			   is specified, 0 otherwise
//				0 = No logging
//				1 = Log major events
//				2 = Log all network messages
//		LOGFILE  = Specifies file for log output, if not specified no
//			   log output is generated

// Each copy of PTRSRV running on a system must have a unique unit number.
//   The actual connections between physical printers and the server are
//   make after the server is loaded using the SERVER utility.

// Note that this server does not make instance numbers visible externally.
//   Instances are identified by the associated spool device name. (This is
//   the name of the associated SPL class device without the PTR prefix.)

// Each of the following server commands is described in the file that
//   implements it.
//		ADD
//		REMOVE
//		PAUSE
//		HOLD
//		CONTINUE
//		CANCEL
//		NAME
//		STATUS
//		TERMINATE


// REMOVE command: Removes a server instance (Does not change the status of
//   any spooled files. If a file is being printed, printing will continue
//   when (if) a new instance is created for the same printer.)
//  SERVER {unit} PTRSRV REM{OVE} NAME=splname {PRG}
//		unit = PTRSRV unit number (default is 1)
//		NAME = Name of spooled device
//		PRG  = If present indicates that program readable (rather than
//				 human readable) responses should be returned (system log
//				  entries are still human readable)

// PAUSE command: Pauses printing
//  SERVER {unit} PTRSRV {P} PAU{SE} splname
//		unit    = PTRSRV unit number (default is 1)
//		splname = Name of spooled device

// HOLD command: Holds a queue entry for later printing. Has no effect if the
//               queue entry is being printed.
//  SERVER {unit} PTRSRV {P} HOL{D} splname number
//		unit    = PTRSRV unit number (default is 1)
//		splname = Name of spooled device
//      number  = Queue entry to hold

// CONTINUE command: Continues printing
//  SERVER {unit} PTRSRV {P} CON{TINUE} splname {number} 
//		unit    = PTRSRV unit number (default is 1)
//		splname = Name of spooled device
//		number  = If present specifies the held queue entry to make printable,
//				    if not specied and the printer has been paused printing is
//                  restarted.

// CANCEL command: Deletes the specified queue entry. If the entry is being
//                 printed printing is terminated immediately, although
//                 anything already sent to the printer will be printed.
//  SERVER {unit} PTRSRV {P} CAN{CEL} splname number
//		unit    = PTRSRV unit number (default is 1)
//		splname = Name of spooled device
//		number  = Number (as indicated by the LIST command) of queue entry to
//                  delete

// STATUS command: Returns status summary
//  SERVER {unit} PTRSRV {P} STA{TUS} {splname}
//		unit    = PTRSRV unit number (default is 1)
//		splname = Name of spooled device (If not specified, a global status
//				    summary is returned, if specified, a detailed status report
//				    on the specified spooled is returned)

// TERMINATE command: Terminates the server
//  SERVER {unit} PTRSRV {P} TERMINATE
//		unit = PTRSRV unit number (default is 1)

// When the single letter P preceeds the keyword, the response consists of an
//   8 byte or longer message. The first 4 bytes specify the PTRSRV version
//   number (LOBF) and the second 4 bytes specify a positive successive code
//   (usually 0) or a negative XOS error code if error (LOBF). See the detailed
//   description of each command for a description of any additional data
//   returned.

// Description of data structures used:

//   The server can support up to 31 server instances.  Each instance uses
//   one  thread. All data associated with the instance is in an ICB located
//   in the thread's local data. Each instance is associated with a physical
//   printer and a spooled device.

//   This server does not support any client connections (other than the
//   standard SERVER interface).

// Description of the print file format:

// PTRSRV uses a counted record structure, with each record containing
//   one page or formatting information. Each record starts with an 8 byte
//   header. The first 4 bytes contain the record type and also act as a
//   verification of correct file position synchronization. The second 4
//   bytes contain the length of the record (not including the 8 byte header)
//   in bytes (LOBF). The record type is encoded as follows (LOBF):
//     0xF0F00F0F = Header record
//     0xF0F00F0C = Data record
//     0xF0F00F03 = Trailer record
//   The file begins with the 4 character sequence "PTR@". This is immediately
//   followed by a single header record which is followed by as many data
//   records as needed.

// The data in each record is, with one exception, raw printer data. The one
//   exception is that the header record must begin with the name of the
//   printer language (usually PCL or PCL3GUI) followed by newline. This is
//   the language name which is used with the PJL ENTER LANGUAGE command.
//   Note that no PJL commands are included in the file. These are inserted
//   as necessary by PTRSRV.

// The header record is sent to the printer immediately before the first
//   page. After this each page is sent in sequence. Some PJL commands are
//   inserted by PTRSRV before the header record, but otherwise nothing is
//   added. If an error occures (such as a paper jam) the header record is
//   resent, followed by the page which follows the last page that was
//   succesfully printed. If the last succesfully printed page cannot be
//   determined, printing is restarted with the first page.

// Optionally, all pages may be included in a single data record. In this
//   case the record length is specified as 0. In this case printing will
//   always restart after an error with the first page of the file. This
//   option should not be used unless absolutely necessary. Page records are
//   generally long enough that it is not practical to buffer a complete
//   data record before writing to disk. Using random access to insert the
//   record length in the record header normally does not add significant
//   overhead and is the recommended way to create the spooled file.

// Spooled files are written to a directory under the XOSSPL: directory.
//   The name of the directory is the same as the name of the spooled device
//   (without the PTR prefix or the trailing colon).

long cmdtdb;

IDB *cmdidb;
long cmdptrhndl;
long cmddirhndl;

IDB *instbl[INSTMAX];

char splname[32];
char ptrdev[32];
char phyname[32];

long spllimit;
long itemnum;

long version = MAJVER;
long editnum = EDITNUM;

static IDB  *requireinst(char *cmd, arg_spec *keyword);
static void  showinst(IDB *idb);

arg_spec srvCmdWord[] =
{   {"ADD"      , 0, NULL, srvFncCommand, (long)cmdadd},
    {"REMOVE"   , 0, NULL, srvFncCommand, (long)cmdremove},
    {"REM"      , 0, NULL, srvFncCommand, (long)cmdremove},
    {"PAUSE"    , 0, NULL, srvFncCommand, (long)cmdpause},
    {"PAU"      , 0, NULL, srvFncCommand, (long)cmdpause},
    {"CONTINUE" , 0, NULL, srvFncCommand, (long)cmdcontinue},
    {"CON"      , 0, NULL, srvFncCommand, (long)cmdcontinue},
    {"CANCEL"   , 0, NULL, srvFncCommand, (long)cmdcancel},
    {"CAN"      , 0, NULL, srvFncCommand, (long)cmdcancel},
    {"NAME"     , 0, NULL, srvFncCommand, (long)cmdname},
    {"NAM"      , 0, NULL, srvFncCommand, (long)cmdname},
    {"STATUS"   , 0, NULL, srvFncCommand, (long)cmdstatus},
    {"STA"      , 0, NULL, srvFncCommand, (long)cmdstatus},
    {"TERMINATE", 0, NULL, srvFncCommand, (long)cmdterminate},
    {0}
};

char prgname[] = "PTRSRV";
char srvname[] = "PTRSRV";

uchar threadready;
uchar debugflg;
uchar debugtrm;
FILE *debugfile;
char  debugname[128];


void mainalt(
    char *args)
{
	srvInitialize(args, (void *)CMDTDB, 0x2000, VECT_SERVER, INSTMAX * 2 + 3,
			MAJVER, MINVER, EDITNUM);
}


//**********************************************************
// Function: srvSetup2 - Called by server routine at startup
//				before response is sent
// Returned: Nothing
//**********************************************************

void srvSetup1(void)
{
	time_s dt;
	char   bufr[256];

	debugflg = debugtrm = (srvReqName[0] == 0);
	if (srvLogName != NULL)
	{
		debugflg = TRUE;
		if (strlen(srvLogName) > 124)
			setupfail(9, "Debug log file name is too long - Cannot continue");
		if ((debugfile = fopen(srvLogName, "a+")) == NULL)
			setupfail(-errno, "Error opening debug file - Cannot continue");
		strcpy(debugname, srvLogName);
		svcSysDateTime(T_GTSYSDTTM, &dt);
		sdt2str(bufr, "%H:%m:%s on %D-%3m-%y", (time_sz *)&dt);
		debugprint("| ");
		debugprint("|***********************************************");
		debugprint("*   PTRSRV starting at %s   *\n"
				"***********************************************\n ", bufr);
	}
    setvector(VECT_CLOSE, 0x01, closesignal); // Set close vector
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


//***********************************************
// Function: procargs - Process command arguments
// Returned: TRUE if OK, FALSE if error
//***********************************************

// All arguments are positional. First argument, if present, is the spool
//   device name. If mode is 2, the second argument (required) is the printer
//   device name and the third argument (optional) is the queue limit. In this
//   case all three arguments must be present. Other the second argument is
//   the queue item number (optional).

int procargs(
    char *cmd,
	int   mode)				// 0 = Existing spool device required
							// 1 = Existing or no spool device required
							// 2 = New spool device requried
{
	IDB  *idb;
	IDB **inspnt;
	char *pnt;
	int   cnt;

	cmdidb = NULL;
	splname[0] = 0;
	ptrdev[0] = 0;
	itemnum = 0;
	spllimit = 0x7FFFFFFF;
    banner();
	if ((pnt = strtok(cmd, " \t")) == NULL)
	{
		if (mode == 1)
			return (TRUE);
		else
		{
			cmderror(ER_BDSPC, "No spool device specified");
			return (FALSE);
		}
	}
 	if (strlen(pnt) > 28)
	{
		cmderror(ER_BDSPC, "Name of spool device is too long");
		return (FALSE);
	}
	pnt = strmov(splname, pnt);
	if (pnt[-1] == ':')
		pnt[-1] = 0;

	inspnt = instbl;
	cnt = INSTMAX;
	do
	{
		if ((idb = *inspnt) != NULL && stricmp(idb->splname, splname) == 0)
		{
			cmdidb = idb;
			if (mode == 2)
			{
				cmderror(ER_DUADF, "Spooled device already defined");
				return (FALSE);
			}
			return (TRUE);
		}
		inspnt++;
	} while (--cnt > 0);
	if (mode == 2)
	{
		if ((pnt = strtok(NULL, " \t")) == NULL)
		{
			cmderror(ER_DUADF, "No printer device specified");
			return (FALSE);
		}
		if (strlen(pnt) > 28)
		{
			cmderror(ER_BDSPC, "Name of printer device is too long");
			return (FALSE);
		}
		pnt = strmov(ptrdev, pnt);
		if (pnt[-1] != ':')
		{
			pnt[0] = ':';
			pnt[1] = 0;
		}
		if ((pnt = strtok(NULL, " \t")) != NULL)
		{
			spllimit = strtol(pnt, &pnt, 0);
			if (*pnt != 0)
			{
				cmderror(0, "Invalid spool limit value specified");
				return (FALSE);
			}
			if (strtok(NULL, " \t") != NULL)
			{
				cmderror(0, "Syntax error");
				return (FALSE);
			}
		}
	}
	else
	{
		itemnum = strtol(pnt, &pnt, 0);
		if (*pnt != 0)
		{
			cmderror(0, "Invalid item number specified");
			return (FALSE);
		}
	}
	return (TRUE);
}


//**********************************************
// Function: banner - Display the initial banner
// Returned: Nothing
//**********************************************

void banner(void)
{
    char bufr[80];

    if (cmdidb != NULL)
		sprintf(bufr, STR_MT_INTRMDMSG"PTRSRV: "ver" - Unit %d, Spool device "
				"%s", srvUnitNum, cmdidb->splname);
    else
		sprintf(bufr, STR_MT_INTRMDMSG"PTRSRV: "ver" - Unit %d", srvUnitNum);
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


//***********************************************
// Function: closesignal - Signal function called
//				when a spooled file is closed
// Returned: Nothing
//***********************************************

void closesignal(
	SIGDATA sd)
{
	long rtn;
	char buffer[128];

	if (debugflg)
	{
		sdt2str(buffer, "%z%H:%m:%s %D-%3n-%y", (time_sz *)&sd.cdt);
		debugprint("closesignal: sd.seqnum: %08X %s sd.idb: %08X", sd.seqnum,
				buffer, sd.idb);
	}
	if ((rtn = quefile(sd.idb, sd.seqnum, sd.cdt, FALSE)) < 0)
	{
		debugprint("closesignal: quefile failed: %d", rtn);

	}
	else
		thdCtlWake((long)sd.idb);
}


long quefile(
	IDB   *idb,
	long   seqnum,
	time_s cdt,
	int    held)
{
	QENT **prev;
	QENT  *next;
	QENT  *qent;

	if ((qent = (QENT *)malloc(sizeof(QENT))) == NULL)
		return (-errno);
	qent->seqnum = seqnum;
	qent->dt = cdt;
	qent->held = held;
	prev = &idb->qhead;
	while ((next = *prev) != NULL &&
			sdtcmp(&cdt, &next->dt) > 0)
		prev = &next->next;
	qent->next = next;
	*prev = qent;
	return (0);
}


void logcmderror(
	long  code,
	char *fmt, ...)
{
	va_list pi;
	char    text[512];
	char    bufr[512];

	va_start(pi, fmt);
	vsprintf(text, fmt, pi);
	sprintf(bufr, "{SPL=%s} %s", splname, text);
	debugprint(bufr);
	srvLogSysLog(code, bufr);
	srvCmdErrorResp(code, bufr, NULL, srvMsgDst);
}


void setupfail(
	long  code,
	char *fmt, ...)
{
	va_list pi;
	char    text1[200];
	char    text2[300];
	char    textxos[100];

    va_start(pi, fmt);
    vsprintf(text1, fmt, pi);
	textxos[0] = 0;
	if (code < 0)
		svcSysErrMsg(code, 0x03, textxos);
	svcSysLog(text2, sprintf(text2, "xxxxUSBCTL  %s%s", text1, textxos));
	sprintf(text2, (code >= 0) ? STR_MT_FINALMSG"USBCTL: %s" :
			STR_MT_FINALMSG"USBCTL: %s\n      %s", text1, textxos);
	srvSetUpResponse(text2);
	exit(1);
}




void cmderror(
	long  code,
	char *fmt, ...)

{
	va_list  pi;
	char    text[512];
	char    bufr[512];

	va_start(pi, fmt);
	vsprintf(text, fmt, pi);
	sprintf(bufr, "{SPL=%s} %s", splname, text);
	debugprint(bufr);
	srvCmdErrorResp(code, bufr, NULL, srvMsgDst);
}


void logerror(
	long  code,
	char *fmt, ...)
{
	va_list pi;
	char   *prefix;
	char    text1[200];
	char    text2[300];
	char    textxos[100];

    va_start(pi, fmt);
    vsprintf(text1, fmt, pi);
	textxos[0] = 0;
	prefix = "";
	if (code < 0)
	{
		prefix = "? ";
		svcSysErrMsg(code, 0x03, strmov(textxos, "\n  "));
	}
	svcSysLog(text2, sprintf(text2, "xxxxUSBCTL  %s%s%s", prefix, text1,
			textxos));
	if (debugflg)
		debugprint("%s%s%s", prefix, text1, textxos);
}


void debugprint(
	char *fmt, ...)
{
	time_s  dt;
	va_list pi;
	char   *pnt;
	char   *bgn;
	char    text[200];
	char    time[32];
	char    chr;

	if (debugflg)
	{
	    va_start(pi, fmt);
    	vsprintf(text, fmt, pi);
		pnt = text;
		if (*pnt == '|')
			pnt++;
		do
		{
			bgn = pnt;
			while ((chr = *pnt) != 0 && chr != '\n' && chr != '\r')
				pnt++;
			*pnt = 0;
			if (bgn != pnt)
			{
				if (bgn == text)
				{
					svcSysDateTime(T_GTHRDTTM, &dt);
					sdt2str(time, "%m:%s.%3f", (time_sz *)&dt);
				}
				else
					time[0] = 0;
				if (debugtrm)
					printf("%9s %s\n", time, bgn);
				if (debugfile != 0)
				{
					fprintf(debugfile, "%9s %2d %s\n", time, (cmdidb == NULL) ?
							0 : cmdidb->instance + 1, bgn);
					fclose(debugfile);
					if ((debugfile = fopen(debugname, "a+")) == NULL)
						logerror(-errno, "Could not reopen the debug log file");
				}
			}
			pnt++;
		} while (chr != 0);
	}
}
