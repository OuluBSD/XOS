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
#include <stdlib.h>
#include <string.h>
#include <time.h>
///#include <xoserr.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>


// This program simultainiously writes to two data files so as to produce
//   maximum fragmentation. This is intended to test handling of fragmented
//   files which require multiple FIBs.

long file1;
long file2;

char buffer1[512] = {1};
char buffer2[512] = {0, 0, 0, 0, 1};

_Packed struct
{	BYTE4PARM grpsz;
	char      end;
} opnparms =
{	{PAR_SET|REP_DECV, 4, IOPAR_GRPSIZE, 1}
};

char prgname[] = "FRAGMENT";

void main(
    int   argc,
    char *argv[])
{
	int  cnt;
	long rtn;

	if (argc != 3)
	{
		fputs("? FRAGMENT: Comand error, usage is:\n"
			  "              fragment file1 file2\n", stderr);
		exit(1);
	}
	if ((file1 = svcIoOpen(XO_CREATE|XO_TRUNCA|XO_OUT, argv[1], &opnparms)) < 0)
		femsg2(prgname, "Error opening first file", file1, NULL);
	if ((file2 = svcIoOpen(XO_CREATE|XO_TRUNCA|XO_OUT, argv[2], &opnparms)) < 0)
		femsg2(prgname, "Error opening second file", file1, NULL);
	cnt = 2500;
	do
	{
		if ((rtn = svcIoOutBlock(file1, buffer1, 512)) < 0)
			femsg2(prgname, "Error writing to file 1", rtn, NULL);
		*(long *)buffer1 = *(long *)buffer1  + 1;
		if ((rtn = svcIoOutBlock(file2, buffer2, 512)) < 0)
			femsg2(prgname, "Error writing to file 1", rtn, NULL);
		*(long *)(buffer2 + 4) = *(long *)(buffer2 + 4)  + 1;
		if ((buffer1[0] & 0x3F) == 0)
			fputs(".", stdout);
	} while(--cnt > 0);
	fputs(" Done\n", stdout);
	exit(0);
}
