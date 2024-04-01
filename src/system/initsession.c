/************************************************/
/*                                              */
/* initsession.c - Routines to create a session */
/*                                              */
/************************************************/
/*                                              */
/* Written by John Goltz                        */
/*                                              */
/************************************************/

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
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <xcstring.h>
#include <utility.h>
#include <xos.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xostime.h>
#include <xosmsg.h>
#include <xoserr.h>
#include <procarg.h>
#include "init.h"


static _Packed struct
{   TEXT8CHAR  class;
	BYTE4CHAR  opencnt;
	LNGSTRCHAR password;
	LNGSTRCHAR program;
	uchar      end;
} trmchar =
{	{PAR_GET|REP_TEXT, 8, "CLASS"},
	{PAR_GET|REP_DECV, 4, "OPENCNT"},
	{PAR_GET|REP_STR , 0, "PASSWORD", NULL, 68, 0},
	{PAR_GET|REP_STR , 0, "PROGRAM" , NULL, 256, 0}
};

static _Packed struct
{	BYTE4PARM cinpmode;
	BYTE4PARM sinpmode;
	BYTE4PARM coutmode;
	BYTE4PARM soutmode;
	char      end;
} trmclrparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
	{PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_ECHO|TIM_ELFWCR},
	{PAR_SET|REP_HEXV, 4, IOPAR_TRMCOUTMODE, 0xFFFFFFFF},
	{PAR_SET|REP_HEXV, 4, IOPAR_TRMSOUTMODE, TOM_ICRBLF}
};

typedef struct
{   ushort src;
    ushort dst;
    long   bits;
} DEVITEM;

static struct
{   DEVITEM dlstdin;
    DEVITEM dlstdout;
    DEVITEM dlstderr;
    DEVITEM dlstdtrm;
	long    end;
} devlist =									// Device list for running a program
{  {0, STDIN , 0x00000004|XO_IN},			//   STDIN  = 1
   {0, STDOUT, 0x00000004|XO_IN|XO_OUT},	//   STDOUT = 2
   {0, STDERR, 0x00000004|XO_IN|XO_OUT},	//   STDERR = 3
   {0, STDTRM, 0x80000004|XO_IN|XO_OUT},	//   STDTRM = 5
};


struct
{   LNGSTRPARM arglist;
    LNGSTRPARM devlist;
    char       end;
} runparm =
{	{PAR_SET|REP_STR, 0, IOPAR_RUNCMDTAIL},
	{PAR_SET|REP_STR, 0, IOPAR_RUNDEVLIST, (char *)&devlist, sizeof(devlist),
			sizeof(devlist)}
};

QAB runqab =
{   RFNC_WAIT|RFNC_RUN,		// func    - Function
    0,						// status  - Returned status
    0,						// error   - Error code
    0,						// amount  - Process ID
    0,						// handle  - Device handle
    0,						// vector  - Vector for interrupt
    {0},					//         - Reserved
    XR_SESSION,				// option  - Options or command
    0,						// count   - Count
    NULL,					// buffer1 - Pointer to file spec
    NULL,					// buffer2 - Unused
    (char *)&runparm		// parm    - Pointer to parameter area
};


//***************************************************
// Function: makesession - Create a session
// Returned: 0 if normal or a negative XOS error code
//***************************************************

// If a cmdline is specified it is used. If not the program associated with
//   the terminal is used. In either case, if no group or used name is
//   specified or if there is a system password associated with the terminal,
//   login is run instead with the group name, user name, and cmdline passed
//   to it in its command line.

long makesession(
	char *trmname,		// Terminal name (colon is optional)
	char *cmdline,		// Command line
	char *group,		// Group name (default is "user")
	char *user)			// User name (default is "user")
{
	char *ppnt;
	char *bpnt;
	char *epnt;
	char *cmdtail;
	long  dev;
	long  rtn;
	int   len;
	char  chr;
	char  pwbufr[68];
	char  trmprgm[256];
	char  prgmspec[256];
	char  buffer[512];
	char  msgbufr[512];

    if ((dev = svcIoOpen(XO_IN|XO_OUT, trmname, NULL)) < 0)
        return (dev);
	trmchar.password.buffer = pwbufr;
	trmchar.program.buffer = trmprgm;
	if ((rtn = svcIoDevChar(dev, (char *)&trmchar)) < 0)
	{
		svcIoClose(dev, 0);
		return (rtn);
	}
	if (*(long *)trmchar.class.value != 'TRM')
	{
		svcIoClose(dev, 0);
		return (ER_NTTRM);
	}
	if (trmchar.opencnt.value != 1)
	{
		svcIoClose(dev, 0);
		return (ER_BUSY);
	}

	// If get here, it appears to be OK to start a session on the terminal

	svcTrmDspMode(dev, DM_BIOSMODE|0x03, 0);
    svcTrmFunction(dev, TF_CLROUT);
    svcIoOutStringP(dev, welcome, 0, (char *)&trmclrparms);
										// Output initial banner
    svcTrmFunction(dev, TF_CLRINP);		// Clear terminal input buffers
	ppnt = (cmdline != NULL && cmdline[0] != 0) ? // Was a program specified?
			cmdline :					// Yes - use it
			(trmprgm[0] != 0) ?			// No - is there a program associated
										//   with the terminal?
			trmprgm :					// Yes - use it
			"XOSCMD:xshell.run";		// Nothing specified - use the default
	cmdtail = ppnt;
	while ((chr = *cmdtail) != 0 && !isspace(chr))
		cmdtail++;

	if ((len = cmdtail - ppnt) > 246)
	{
		svcIoClose(dev, 0);
		return (ER_NTLNG);
	}
	bpnt = prgmspec;
	if (strchr(ppnt, ':') == NULL)		// No - was a device specified?
		bpnt = strmov(bpnt, "CMD:");	// No - use default
	epnt = strnmov(bpnt, ppnt, len);
	*epnt = 0;
	if (strchr(bpnt, '.') == NULL)		// Was an extension specified?
		strmov(epnt, ".run");			// No - use default

	// Now see if we need to run login

    if (pwbufr[0] == 0 && user != NULL && user[0] != 0) // Do we need to run
	{													//   login?
		runparm.arglist.buffer = (cmdline != NULL && cmdline[0] != 0) ?
				cmdline : prgmspec;
		runqab.buffer1 = prgmspec;
	}
	else								// Yes
	{
		runqab.buffer1 = "XOSSYS:login.run";
		sprintf(buffer, "login /trm=%s: /grp=%s /usr=%s %s %s", trmname,
				(group == NULL || group[0] == 0) ? "user" : group,
				(user == NULL || user[0] == 0) ? "user" : user, prgmspec,
				cmdtail);
		runparm.arglist.buffer = buffer;
	}
	runparm.arglist.strlen = runparm.arglist.bfrlen =
			strlen((char *)runparm.arglist.buffer);
    devlist.dlstdin.src = (ushort)dev;
    devlist.dlstdout.src = (ushort)dev;
    devlist.dlstderr.src = (ushort)dev;
    devlist.dlstdtrm.src = (ushort)dev;
	rtn = svcIoRun(&runqab);
	svcIoClose(dev, 0);
    if (rtn < 0 || (rtn=runqab.error) < 0)
    {
		svcSysErrMsg(rtn, 3, trmprgm);	// Get error message string
		len = sprintf(msgbufr, STR_MT_SYSLOG"---INIT    ----Error loading "
				"command processor %s for %s\n%s", (char *)runqab.buffer1,
				trmname, trmprgm);
		*(long *)&msgbufr[12] = initpid;	// Log the error
		svcSysLog(msgbufr, len);
		return (rtn);
    }
	len = sprintf(msgbufr, STR_MT_SYSLOG"---SESNCRTD----Session created "
			"using %s for %s", (char *)runqab.buffer1, trmname);
	*(long *)&msgbufr[12] = runqab.amount & 0xFFFF0FFF;
	svcSysLog(msgbufr, len);
	return (0);
}
