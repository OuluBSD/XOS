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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xosstr.h>
#include <xos.h>
#include <xoserrmsg.h>
#include <xossvc.h>
#include <xoserr.h>
#include <signal.h>
#include <errno.h>
#include <xossnd.h>

typedef struct
{	long  xxx[15];
	short datacnt;
	short signum;
	long  dataval;
	long  errcode;
	long  offset;
} SIGSTK;

struct
{	byte4_parm vect;
	uchar      end;
} sigparms =
{	{PAR_SET | REP_HEXV, 4, IOPAR_SIGVECT1, 60}
};

struct
{	byte4_parm class;
	byte4_parm start;
	uchar      end;
} sndparms =
{	{PAR_SET | REP_TEXT, 4, IOPAR_CLASS, 'DNS'},
	{PAR_SET | REP_HEXV, 4, IOPAR_SND_CMD, SCMD_START}
};

char  prgname[] = "STEST";

extern char buffer[];

void sndsignal(SIGSTK stk);

void main(
    int   argc,
    char *argv[])

{
	long *pnt;
	int   cnt;
	long  value;
	long  rtn;
	long  hndl;

	argc = argc;
	argv = argv;

	if ((rtn = svcMemChange(buffer, 2, 0x2000)) < 0)
		femsg2(prgname, "Cannot allocate buffer", rtn, NULL);
	
	if ((hndl = svcIoOpen(0, "SND:", NULL)) < 0)
		femsg2(prgname, "Cannot open sound device", hndl, NULL);

	if (setvector(60, 3, sndsignal) < 0)
		femsg2(prgname, "Cannot set signal vector", -errno, NULL);

	if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &sigparms)) < 0)
		femsg2(prgname, "Cannot specify signal vector", rtn, NULL);

	svcSchSetLevel(0);

	if ((rtn = svcIoOutBlock(hndl, buffer, 0x2000)) < 0)
		femsg2(prgname, "Error setting buffer address", rtn, NULL);

	cnt = 0x800;
	pnt = (long *)buffer;
	value = 0x0;
	do
	{
		*pnt++ = value;
		value += 0x01000100;
		value &= 0x7F007F00;
	} while (--cnt > 0);

	if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &sndparms)) < 0)
		femsg2(prgname, "Error starting sound output", rtn, NULL);


	while (TRUE)
		svcSchSuspendT(NULL, -1, -1);
}


void sndsignal(SIGSTK stk)

{
	printf("%06X  ", stk.offset);
}
