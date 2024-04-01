//--------------------------------------------------------------------------*
// usbwait.c - Program to wait unit the USBCTL is idle
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

// Command format:
//	usbwait interval idle timeout
//    where:
//      interval = Sample interval in millisec
//      idle     = Required idle time in millisec
//      timeout  = Maximum time to wait in millisec

// An ACTIVE request is sent to USBSRV_1 every "interval" millisec. USBSRV is
//   considered to be idle if the current active thread count (first value) is
//   0 and the total active thread count (second value) has not changed.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoswildcard.h>
#include <xoserrmsg.h>
#include <dirscan.h>
#include <global.h>
#include <xosstr.h>

#define VERSION 4
#define EDITNO  0


long curnumact;
long curttlact;
long ipmhndl;

char prgname[] = "USBWAIT";
char sysinit[] = "SYS^USBCTL^1";

struct
{   BYTE4PARM  timeout;
    LNGSTRPARM dstname;
    char       end;
} outparm =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT   , 4*ST_SECOND},
    {PAR_SET|REP_STR , 0, IOPAR_MSGRMTADDRS, sysinit, sizeof(sysinit) - 1}
};

struct
{   BYTE4PARM  timeout;
    char       end;
} inparm =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT, 4*ST_SECOND}
};


void msgfail(void);
long getvalue(char *arg);
void sample(void);


void main(
    int   argc,
    char *argv[])

{
	llong interval;
	long  idle;
	long  timeout;

	long  ttlactive;
	int   ttlcnt;
	int   idlenum;
	int   idlecnt;

	if (argc != 4)
	{
		fputs("? USBWAIT: Invalid command syntax", stderr);
		exit(1);
	}
	interval = getvalue(argv[1]);
	idle = getvalue(argv[2]);
	timeout = getvalue(argv[3]);
	ttlcnt = (timeout + ((long)interval) - 1) / (long)interval;
	idlenum = (idle + ((long)interval) - 1) / (long)interval;
	printf("USBWAIT: Waiting for USBCTL (interval = %d, idlenum = %d, "
			"maxcnt = %d)\n", (long)interval, idlenum, ttlcnt);
	interval *= ST_MILLISEC;
    if ((ipmhndl = svcIoOpen(XO_IN|XO_OUT, "IPM:", NULL)) < 0)
        errormsg(ipmhndl, "!Cannot open message device IPM:");
	sample();							// Get the initial values
	ttlactive = curttlact;
	idlecnt = idlenum;
	do
	{
		svcSchSuspend(NULL, interval);
		sample();
		if (curnumact != 0 || curttlact != ttlactive)
		{
			fputs("+", stdout);
			idlecnt = idlenum;
		}
		else
		{
			fputs("-", stdout);
			if (--idlecnt <= 0)
			{
				fputs(" Done\n", stdout);
				exit(0);
			}
		}
		ttlactive = curttlact;
	} while (--ttlcnt > 0);
	fputs(" Timed out\n", stdout);
	exit(1);
}


long getvalue(
	char *arg)
{
	char *endp;
	long  val;

	val = strtol(arg, &endp, 0);
	if (*endp != 0)
	{
		fputs("? USBWAIT: Invalid numeric value\n", stderr);
		exit(1);
	}
	return (val);
}


void sample(void)
{
	char *endp;
	long  rtn;
	char  mbufr[32];

	static char msg[] = STR_MT_SRVCMD"ACT";

    if ((rtn=svcIoOutBlockP(ipmhndl, msg, 4, &outparm)) < 0)
        errormsg(rtn, "!Error sending message to USBCTL");
	if ((rtn = svcIoInBlockP(ipmhndl, mbufr, sizeof(mbufr), &inparm)) < 0)
		errormsg(rtn, "!Error receiving response from USBCTL");
	if (mbufr[0] != MT_FINALMSG)
		errormsg(0, "!Incorrect message type received from USBGCTL");
	mbufr[rtn] = 0;

	// Here with a response from USBCTL. This should consist of two decimal
	//   values seperate by a single space character

	curnumact = strtol(mbufr + 1, &endp, 0);
	if (*endp != ' ')
		msgfail();
	curttlact = strtol(endp + 1, &endp, 0);
	if (*endp != 0)
		msgfail();
}


void msgfail(void)
{
	errormsg(0, "!Invalid format in message received from USBGCTL");
}
