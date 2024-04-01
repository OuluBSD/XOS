// ++++
//  This software is in the public domain.  It may be freely copied and used
//  for whatever purpose you see fit, including commerical uses.  Anyone
//  modifying this software may claim ownership of the modifications, but not
//  the complete derived code.  It would be appreciated if the authors were
//  told what this software is being used for, but this is not a requirement.

//    THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES//  LOSS
//    OF USE, DATA, OR PROFITS//  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//    USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----

// This is the server framework for a thread environment. It provides the
//   basic routines to set up and communicate with a detached server
//   (a symbiont).

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <xcstring.h>
#include <xos.h>
#include <xossvc.h>
#include <xoslib.h>
#include <xoserr.h>
#include <procarg.h>
#include <errno.h>
#include "serverfwt.h"


static int fncunit(arg_data *arg);
static int fncdbgfile(arg_data *arg);
static int fncdbglevel(arg_data *arg);
static int fnclogfile(arg_data *arg);
static int fncloglevel(arg_data *arg);


static arg_spec keyword[] =
{	{"UNIT"      , ASF_VALREQ|ASF_NVAL , 0, fncunit    , 0},
	{"LOGLEVEL"  , ASF_VALREQ|ASF_NVAL , 0, fncloglevel, 0},
	{"LOGFILE"   , ASF_VALREQ|ASF_LSVAL, 0, fnclogfile , 0},
	{"DEBUGLEVEL", ASF_VALREQ|ASF_NVAL , 0, fncdbglevel, 0},
	{"DEBUGFILE" , ASF_VALREQ|ASF_LSVAL, 0, fncdbgfile , 0},
	{0}
};


static char *dbgname;			// Debug log file specification
static long  msghndl;
static int   pnlen;

int   sfwUnitNum = 1;
char  sfwIpmName[32];
int   sfwDebugLevel;
char *sfwDebugFile;
FILE *sfwDebugStream;
CRIT  sfwTrmCrit = {'TIRC'};
int   sfwLogLevel;
char *sfwLogFile;

static char  cmdmsgbfr[1024];

char  sfwReqName[64];


char sfwMsgName[64];

static _Packed struct			// Parameters list used when sending response
{	LNGSTRPARM reqname;
	char       end;
} msgoutparms =
{	{PAR_SET|REP_STR, 0, IOPAR_MSGRMTADDRS, sfwMsgName, sizeof(sfwMsgName),
			sizeof(sfwMsgName)}
};

static _Packed struct
{	LNGSTRPARM src;
	char       end;
} cmdmsginparms =
{	{PAR_GET|REP_STR, 0, IOPAR_MSGRMTADDRR, sfwMsgName, sizeof(sfwMsgName), 0}
};


static void argerr(char *str1, char *str2);
static void response(char *msg, int len);
static int filearg(arg_data *arg, char **spec);


//************************************************************
// Function: sfwThreadInit - Initialize the server framework
// Returned: Never returns
//************************************************************


long sfwThreadInit(
	HUMA *args)				// altmain argument string
{
	char      *pnt;
	char      *apnt;
	char      *argpntr[2];
	SFWCMDTBL *ctpnt;
	int        cnt;
	int        clen;
	char       cmdword[32];
	char       chr;

	pnt = args->cmdaddr;
	while ((chr = *pnt) != 0 && !isspace(chr)) // Skip the initial atom
		pnt++;
	while ((chr = *pnt) != 0 && isspace(chr)) // Skip following whitespace
		pnt++;
	apnt = sfwReqName;					// Collect the IPM address of the
	cnt = sizeof(sfwReqName) - 1;		//   requestor
	while ((chr = *pnt) != 0 && !isspace(chr))
	{
		pnt++;
		if (--cnt > 0)
			*apnt++ = chr;
	}
	*apnt = 0;
	if (sfwReqName[0] == '0' && sfwReqName[1] == 0)
		sfwReqName[0] = 0;
	strcpy(msgoutparms.reqname.buffer, sfwReqName);
	while ((chr = *pnt) != 0 && isspace(chr)) // Skip following whitespace
		pnt++;
	argpntr[0] = pnt;
	argpntr[1] = NULL;
	procarg((cchar **)argpntr, PAF_INDIRECT|PAF_EATQUOTE, 0, &keyword, 0,
			argerr, 0, 0);

	snprintf(sfwIpmName, sizeof(sfwIpmName), "IPM:SYS^%s^%d", prgname,
			sfwUnitNum);				// Construct the IPM device name
	clen = strlen(prgname);
	cnt = sizeof(prgname) - clen;
	if ((pnlen = snprintf(prgname + clen, cnt, "_%d", sfwUnitNum)) >= cnt)
		pnlen = clen - 1;				// Add the unit number to the name
	svcSysSetPName(prgname);			// Change our process name
	if ((msghndl = svcIoOpen(XO_IN|XO_OUT, sfwIpmName, NULL)) < 0)
	{
		if (msghndl == ER_ALDEF && (msghndl = svcIoOpen(XO_IN|XO_OUT, "IPM:",
				NULL)) > 0)
			sfwResponse(0x80000000, SFWRSP_SEND|SFWRSP_PREFIX, "Server unit "
					"%d is already running", sfwUnitNum);
		exit(1);						// If can't open our IPM device there
	}									//   isn't much we can do!

	// Here with the message device open

	if (sfwDebugLevel != 0)				// Debug level specified?
	{
		if (dbgname[0] != 0)			// Yes - was a log file specified?
			sfwDebugStream = stdout;	// No - use stdout
		else
		{
			if ((sfwDebugStream = fopen(dbgname, "a+")) == NULL)
				sfwResponse(-errno, SFWRSP_SEND|SFWRSP_PREFIX, "Cannot open "
						"debug file");
		}
	}

	// Here with all command line processing done

	if (srvsetup())
		sfwResponse(0, SFWRSP_SEND|SFWRSP_PREFIX|SFWRSP_FINAL, "%d.%d.%d - "
				"Initialization complete for unit %d", srvmajver, srvminver,
				srveditnum);
	sfwReqName[0] = 1;					// This is needed for sfwResponse
	sfwMsgName[0] = 0;					// This is set by the input call below

	// Here with set up complete - This is the main program loop which reads
	//   and processes server commands.

	while (TRUE)
	{
		if ((clen = svcIoInBlockP(msghndl, cmdmsgbfr, sizeof(cmdmsgbfr) - 1,
				&cmdmsginparms)) < 0)
		{
			// Here if error - It is tempting to just ignore the error and try
			//   to continue but this would most likely cause a tight loop!

			sfwResponse(clen, SFWRSP_LOG, "Error getting IPM input");
			exit (1);
		}
		cmdmsgbfr[clen] = 0;
		if (cmdmsgbfr[0] != MT_SRVCMD)	// Server command?
			srvmessage(cmdmsgbfr, clen); // No - give it to the program
		else
		{
			// Here with a server command message. Commands always start with
			//   a simple command keyword which may be followed by 0 or more
			//   procarg style keyword=value pairs.

			pnt = cmdmsgbfr + 1;		// Find the end of the command keyword
			apnt = cmdword;
			cnt = 32;
			while ((chr = *pnt) != 0 && isalnum(chr))
			{
				pnt++;
				if (--cnt > 0)
					*apnt++ = chr;
			}
			*apnt = 0;
			if (chr != 0)
				pnt++;
			ctpnt = srvcmdtbl;
			while (ctpnt->name != NULL)
			{
				if (stricmp(ctpnt->name, cmdword) == 0)
					break;
				ctpnt++;
			}
			if (cnt < 0 || ctpnt->name == NULL)
				sfwResponse(0x80000000, SFWRSP_SEND|SFWRSP_PREFIX|SFWRSP_FINAL,
						"Invalid command");
			else
			{
				while ((chr = *pnt) != 0 && isspace(chr)) // Skip whitespace
					pnt++;
				(ctpnt->func)(pnt, clen - (pnt - cmdmsgbfr));
			}
		}
	}
}


//******************************************************
// Function; srvResponse - Send general response message
// Returned: Does not return!
//******************************************************

void sfwResponse(
	long  code,				// Error code
	int   bits,
	char *fmt, ...)
{
	va_list pi;
	char   *spnt;
	char   *tpnt;
	int     plen;
	int     mlen;
	int     mtype;
	int     cnt;
	int     cnt2;
	char    chr;
	char    texte[100];
	char    textp[80];
	char    text1[100];
	char    text2[160];
	char    text3[160];

   	va_start(pi, fmt);					// Expand the string
   	vsnprintf(text1, sizeof(text1), fmt, pi);

	if (code & 0x40000000)
		sysLibErrMsg(code, 0x03, texte);
	else
		texte[0] = 0;

	if (bits & SFWRSP_LOG)				// Should we log this?
	{
		if ((mlen = snprintf(text2, sizeof(text2), "----%8.8s%s%s%s%s",
				prgname, (code & 0x80000000) ? "? " : "", text1, (texte[0]) ?
				"\n  " : "", texte)) >= sizeof(text2))
			mlen = sizeof(text2) - 1;
		svcSysLog(text2, mlen);
	}
	if (bits & SFWRSP_SEND)				// Should we send a response?
	{
		mtype = (bits & SFWRSP_FINAL) ?
			((code & 0x80000000) ? MT_FINALERR : MT_FINALMSG) :
			((code & 0x80000000) ? MT_INTRMDERR : MT_INTRMDMSG);
		if (bits & SFWRSP_PREFIX)
		{
			if ((plen = snprintf(textp, sizeof(textp), "%s%s: ",
					(code & 0x80000000) ? "? " : "", prgname)) >= sizeof(textp))
				plen = sizeof(textp) - 1;
		}
		else if (code & 0x80000000)
		{
			textp[0] = '?';
			textp[1] = ' ';
			textp[2] = 0;
			plen = 2;
		}
		else
		{
			textp[0] = 0;
			plen = 0;
		}

		if (plen > 0)
		{
			// Here if may need to insert leading spaces to line things up

			spnt = text1;
			tpnt = text2;
			cnt = sizeof(text2) - 1;
			while (--cnt > 0 && (chr = *spnt++) != 0)
			{
				*tpnt++ = chr;
				if (chr == '\n')
				{
					if ((cnt -= plen) <= 0)
						break;
					cnt2 = plen;
					do
					{
						*tpnt++ = ' ';
					} while (--cnt2 > 0);
				}
			}
			*tpnt = 0;
			tpnt = text2;
		}
		else
			tpnt = text1;
		mlen = (texte[0]) ?
			snprintf(text3, sizeof(text3) - 1, "%c%s%s\n%*s%s", mtype, textp,
					tpnt, plen, "", texte) :
			snprintf(text3, sizeof(text3) - 1, "%c%s%s", mtype, textp, tpnt);

		if (sfwReqName[0] == 0)			// Are we running from a terminal?
		{
			critBegin(&sfwTrmCrit);
			text3[mlen] = '\n';
			text3[mlen + 1] = 0;
			svcIoOutBlock(DH_STDERR, text3 + 1, mlen);
			critEnd(&sfwTrmCrit);
		}
		else
			svcIoOutBlockP(msghndl, text3, mlen, &msgoutparms);
	}
}


// Function called by procarg on errors

static void argerr(
	char *str1,
	char *str2)
{
	char text[200];

	if (str2 != NULL)
	{
		snprintf(text, sizeof(text), "%s\n%s", str1, str2);
		sfwResponse(0x80000000, SFWRSP_SEND|SFWRSP_PREFIX, text);
	}
	else
		sfwResponse(0x80000000, SFWRSP_SEND|SFWRSP_PREFIX, str1);
}


//*********************************************
// Function: fncunit - Process the UNIT keyword 
// Returned: TRUE if OK, FALSE if error
//*********************************************

static int fncunit(
    arg_data *arg)
{
	sfwUnitNum = arg->val.n;
	return (TRUE);
}


//***************************************************
// Function: fnclogfile - Process the LOGFILE keyword
// Returned: TRUE if OK, FALSE if error
//***************************************************

static int fnclogfile(
    arg_data *arg)
{
	return (filearg(arg, &sfwLogFile));
}


//***************************************************
// Function: fncdbgfile - Process the DBGFILE keyword
// Returned: TRUE if OK, FALSE if error
//***************************************************

static int fncdbgfile(
    arg_data *arg)
{
	return (filearg(arg, &sfwDebugFile));
}



static int filearg(
	arg_data *arg,
	char    **spec)
{
	char *pnt;

	if ((pnt = sbrk(arg->length + 1)) == (char *)-1)
	{
		sfwResponse(ER_NOMEM, SFWRSP_SEND|SFWRSP_PREFIX, "Error allocating "
				"memory for file name");
		return (FALSE);
	}
	memcpy(pnt, arg->val.s, arg->length);
	pnt[arg->length] = 0;
	*spec = pnt;
	return (TRUE);
}


//*****************************************************
// Function: fnclogLEVEL - Process the LOGLEVEL keyword
// Returned: TRUE if OK, FALSE if error
//*****************************************************

static int fncloglevel(
    arg_data *arg)
{
	sfwLogLevel = arg->val.n;
	return (TRUE);
}


//*****************************************************
// Function: fncdbglevel - Process the DBGLEVEL keyword
// Returned: TRUE if OK, FALSE if error
//*****************************************************

static int fncdbglevel(
    arg_data *arg)
{
	sfwDebugLevel = arg->val.n;
	return (TRUE);
}
