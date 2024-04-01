/***************************************/
/* INIT	- Main program                 */
/* General process initializer for XOS */
/***************************************/
/* Written by John Goltz               */
/***************************************/

// ++++
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <xcstring.h>
#include <utility.h>
#include <xos.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xossignal.h>
#include <xosmsg.h>
#include <xoserr.h>
#include <xostime.h>
#include <xossysp.h>
///#include <xosusr.h>
#include "init.h"

extern int errno;

#define TRMCLASS 0x4D5254L

// Main program


SESNREQ  *sesnhead;
SESNREQ **sesnppnt = &sesnhead;


static char *conname   = "OOCD0:";
static char  msgname[] = "IPM:SYS^INIT";

char oncename[] = "XOSSYS:once.run";

uchar startdone = FALSE;

char msgbfr[2000];
char srcbfr[64];

struct
{   LNGSTRPARM srcname;
    char        end;
} msginpparm =
{   {(PAR_GET|REP_STR), 0, IOPAR_MSGRMTADDRR, srcbfr, 64, 0}
};

struct
{   LNGSTRPARM srcname;
    char       end;
} msgoutparm =
{   {(PAR_SET|REP_STR), 0, IOPAR_MSGRMTADDRS, srcbfr, 64, 0}
};

QAB msginpqab =
{   QFNC_INBLOCK,			// func    - Function
    0,						// status  - Returned status
    0,						// error   - Error code
    0,						// amount  - Amount transfered
    0,						// handle  - Device handle
    VECT_MESSAGE,			// vector  - Vector for interrupt
    {0},					//         - Reserved
    0,						// option  - Options or command
    2000,					// count   - Amount to transfer
    msgbfr,					// buffer1 - Pointer to data buffer
    NULL,					// buffer2 - Pointer to source string
    (char *)&msginpparm		// parm    - Pointer to parameter area
};

QAB onceqab =
{   RFNC_WAIT|RFNC_RUN,		// func    - Function
    0,						// status  - Returned status
    0,						// error   - Error code
    0,						// amount  - Process ID
    0,						// handle  - Device handle
    0,						// vector  - Vector for interrupt
    {0},					//         - Reserved
    XR_CHILDTERM,			// option  - Options or command
    0,						// count   - Count
    oncename,				// buffer1 - Pointer to file spec
    NULL,					// buffer2 - Unused
    NULL					// parm    - Pointer to parameter area
};

#define VERSION 1
#define EDITNO  6

// 1.5  18-Apr-99
//	Changed names so all module names begin with INIT; removed some obsolete
//	stubs.

static void startup(void);
static void errmsg(long code, char *msg);

void mainmin(
    img_data *args)
{
    svcMemChange(args, 0, 0);			// Give up the argument msect
    initpid = svcSysGetPid();			// Get the INIT process ID

    svcIoQueue(&cfgqab);

	startup();
	while (TRUE)
		svcSchSuspend(NULL, -1);		// Wait for something to happen
}


static void startup(void)
{
	SESNREQ *sesnnext;
	char     buffer[80];

	svcSysSetPName("Init");
    setvector(VECT_MESSAGE, 0x04, message); // Set message received vector
    if ((msginpqab.handle = svcIoOpen(XO_IN|XO_OUT, msgname, NULL)) < 0)
		errmsg(msginpqab.handle, "Cannot open message device "
				"IPM:SYS^INIT");
    else
    {
        msginpparm.srcname.bfrlen = 64;
        if ((rtnval=svcIoQueue(&msginpqab)) < 0 ||
                (rtnval=msginpqab.error) < 0)
            errmsg(rtnval, "Cannot start message input on IPM:SYS^INIT");
    }
    setvector(VECT_PTERM, 0x04, childgone); // Set process termination vector
	setvector(VECT_SECOND, 0x04, onceasecond); // Set the once-a-second vector
///	svcSchAlarm(4, 0, VECT_SECOND, 0, 0, ST_SECOND); // Request once-a-second
													 //   signal
    svcSchSetLevel(0);					// Enable signals

	if ((rtnval = svcIoRun(&onceqab)) < 0 ||
			(rtnval = onceqab.error) < 0)
		errmsg(rtnval, "Error loading system initializaton program");
	startdone = TRUE;

	// Put our initial message in the log

	rtnval = sprintf(buffer, "----INIT    ----%d.%d Startup is complete",
			VERSION, EDITNO);
	*(long *)&buffer[12] = initpid;
	svcSysLog(buffer, rtnval);
	startupdone = TRUE;
	while (sesnhead != NULL)
	{
		sesnnext = sesnhead->next;
		makesession(sesnhead->trm, sesnhead->cmd, sesnhead->grp, sesnhead->usr);
		free(sesnhead);
		sesnhead = sesnnext;
	}
}


static void errmsg(
	long  code,
	char *msg)
{
	long conhndl;
	char text[100];

    if ((conhndl = svcIoOpen(XO_IN|XO_OUT, conname, NULL)) < 0)
		exit(1);
	svcIoOutString(conhndl, "? INIT: ", 0);
	svcIoOutString(conhndl, msg, 0);
	if (code < 0)
	{
		svcSysErrMsg(code, 0x03, strmov(text, "\r\n        "));
		strcat(text, "\r\n");
		svcIoOutString(conhndl, text, 0);
	}
	svcIoOutString(conhndl, "\r\n", 0);
	svcIoClose(conhndl, 0);
}

// Signal routine called once each second

void onceasecond(void)

{
/*	time_s dt;

	if (--mincnt < 0)
	{
		mincnt = 60;
		svcSysDateTime(T_GTXDTTM, &dt);
		if (logdate < dt.date)
			rolllog(logdate, "Date changed");
	}
	if (logdev > 0 && logcommit)
	{
		svcIoCommit(logdev);
		logcommit = FALSE;
	}
*/
}

// Function to service message available signal

void message(void)

{
    for (;;)
    {
        switch (msgbfr[0])
        {
		 case MT_SYMBREQ:				// Symbiont request
			symbiontreq(msgbfr + 1, msginpqab.amount - 1);
			break;

		 case MT_SESNREQ:				// Session request
			sessionreq(msgbfr + 1, msginpqab.amount - 1);
			break;

		 case MT_TERMDATA:				// Data from idle terminal
			terminaldata(msgbfr, msginpqab.amount);
			break;
		}
		msginpqab.vector = 0;
		msginpparm.srcname.bfrlen = 64;
		if (svcIoQueue(&msginpqab) >= 0)
		{
			msginpqab.vector = VECT_MESSAGE;
			if (msginpqab.status & QSTS_DONE)
				continue;
		}
		break;
    }
}

// Function to send response

void response(header, msg)
char *header;
char *msg;

{
    strmov(strmov(strmov(msgbfr, header), "INIT: "), msg);
    svcIoOutBlockP(msginpqab.handle, msgbfr, strlen(msgbfr),
			(char *)&msgoutparm);
}
