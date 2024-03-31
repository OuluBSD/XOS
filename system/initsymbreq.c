/*****************************************************/
/*                                                   */
/* initsymbreq.c - Symbiont request handler for INIT */
/*                                                   */
/*****************************************************/
/*                                                   */
/* Written by John Goltz                             */
/*                                                   */
/*****************************************************/

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
#include "init.h"

static struct
{	LNGSTRPARM arglist;
	uchar      end;
} symbparm =
{	{PAR_SET|REP_STR, 0, IOPAR_RUNCMDTAIL},
};

static QAB symbqab =
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
    (char *)&symbparm		// parm    - Pointer to parameter area
};

// Function to process symbiont request message

// The message (which is in msgbfr) consists of the command line which would
//   be used to run the symbiont as a normal program starting with the 2nd
//   character in the message.  XOSSYS: is prepended to the program name,
//   so it cannot specify a device.  It can specify a directory(s) under
//   XOSSYS: if desired.

void symbiontreq(
	uchar *msg,
	int    len)
{
    char *cpnt;
    char *apnt;
    char *tpnt;
    long  rtn;
    int   cnt;
    char  symname[200];		// Name of symbiont
    char  symargs[300];		// Buffer for argument list
	char  msgbufr[500];
    char  havedev;
    char  haveext;
    char  chr;

    apnt = msg;
    cnt = len;

    while ((--cnt >= 0) && ((chr=*apnt++) != '\0') && !isspace(chr))
        ;
    if ((len - cnt) > 85)
    {
        response("\7? ", "Symbiont name too long");
        return;
    }
    if (cnt >= 0)
        --apnt;
    *apnt++ = '\0';

    // First setup the name of the symbiont for the run SVC

    tpnt = msg;
    havedev = FALSE;
    haveext = FALSE;
    while ((chr=*tpnt++) != '\0')
    {
        if (chr == ':')
            havedev = TRUE;
        else if (chr == '.')
            haveext = TRUE;
    }
    if (havedev)
        cpnt = symname;
    else
        cpnt = strmov(symname, "XOSSYS:");
    cpnt = strmov(cpnt, msg);
    if (!haveext)
        strmov(cpnt, ".RUN");

    // Now setup the command arguments

    cpnt = strmov(symargs, msg);		// Put name of program first
    if (cnt > 280 - (cpnt-symargs))
    {
        response("\7? ", "Symbiont arguments too long");
        return;
    }
    *cpnt++ = ' ';						// Put source address in as 1st
    cpnt = strmov(cpnt, srcbfr);		//   argument
    *cpnt++ = ' ';
    while ((--cnt >= 0) && (*cpnt++ = *apnt++) != '\0')
		;								// Put in remaining data as arugments
    *cpnt = '\0';						// Make sure have null at end
    symbqab.buffer1 = symname;
    symbparm.arglist.buffer = symargs;
    symbparm.arglist.bfrlen = symbparm.arglist.strlen = cpnt - symargs;
    if ((rtn = svcIoRun(&symbqab)) < 0 || (rtn = symbqab.error) < 0)
    {
        svcSysErrMsg(rtn, 3, symargs);	// Get error message string
        response("\7? ", symargs);		// And complain
		len = sprintf(msgbufr, STR_MT_SYSLOG"---SYMBERR ----Error creating "
				"symbiont %s:\n%s", symname, symargs);
    }
	else
		len = sprintf(msgbufr, STR_MT_SYSLOG"---SYMBCRTD----Symbiont %s "
				"created", symname);
	*(long *)&msgbufr[12] = symbqab.amount & 0xFFFF0FFF;
	svcSysLog(msgbufr, len);

	// If everything worked, the symbiont will send the response(s) to the
	//   requester.
}
