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
#include <ctype.h>
#include <errno.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.H>
#include <xosstr.h>
#include <global.h>

// This program creates the specified number of smalls files in a directory.
//   This is intended to test the OS code which extends directories.

// Command usage is:
//   filldir num spec 
// Where:
//   num  = Number of files to create
//   spec = File spec for files to create. An incrementing 6 digit decimal
//          number is inserted at the end of the file name for each file
//          created.

char  prgname[] = "FILLDIR";

int main(
	int   argc,
	char *argv[])
{   
	char *epnt;
	long  rtn;
	long  hndl;
	int   nlen;
	int   num;
	int   cnt;
	char  name[512];

	if (argc != 3)
	{
		fputs("? FILLDIR: Invalid arguments, usage is:\n"
			  "             filldir num spec\n"
			  "           Where:\n"
			  "             num  = Number of files to create\n"
			  "             spec = File spec for files to create. An "
					"incrementing 6 digit\n"
			  "                    decimal number is inserted at the "
					"end of the file name\n"
			  "             for each file created.\n", stderr);
		return (1);
	}
	cnt = atol(argv[1]);
	printf("Creating %d file%s\n", cnt, (num == 1) ? "" : "s");
    if ((epnt = strrchr(argv[2], '.')) == NULL)
	{
		nlen = strlen(argv[2]);
		epnt = "";
	}
	else
	{
		nlen = epnt - argv[2];
		epnt++;
	}
	num = 1;
	while (--cnt >= 0)
	{
		sprintf(name, "%.*s%06d.%s", nlen, argv[2], num++, epnt);
		if ((hndl = svcIoOpen(XO_CREATE|XO_OUT|XO_IN, name, NULL)) < 0)
		{
			fprintf(stderr, "? FILLDIR: Error creating %s\n", name);
			svcSysErrMsg(hndl, 0x03, name);
			fprintf(stderr, "           %s\n", name);
			return (1);
		}
		if ((rtn = svcIoOutBlock(hndl, "Test file\n", 10)) < 0)
		{
			fprintf(stderr, "? FILLDIR: Error writing %s\n", name);
			svcSysErrMsg(rtn, 0x03, name);
			fprintf(stderr, "           %s\n", name);
			return (1);
		}
		if ((rtn = svcIoClose(hndl, 0)) < 0)
		{
			fprintf(stderr, "? FILLDIR: Error closing %s\n", name);
			svcSysErrMsg(rtn, 0x03, name);
			fprintf(stderr, "           %s\n", name);
			return (1);
		}
		printf("%s ", name);
	}
	fputs("\nComplete\n", stdout);
	return (0);
}
