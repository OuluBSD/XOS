//-----------------------------------------------------------------------
// EXPORTDMP
// Program to display exported exec mode symbols
// 
// Written by: John R. Goltz
//
// Edit History:
// ------------
//-----------------------------------------------------------------------

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

// Command format:
//   EXPORTDMP name

#include <ctype.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xosstr.h>
#include "lkeload.h"

#define VERSION 1
#define EDITNO  0


char *sympnt;
union
{	char   *c;
	ushort *s;
	long   *l;
}     valpnt;
int   valsize;
long  value;
union
{	char  c[8];
	llong ll;
}   prefix;

char header1;
char header2;

EXPORTTABLE *exportpnt;


//**********************************
// Function: main - Main program
// Returned: 0 if normal, 1 if error
//**********************************

void main(
    int   argc,
    char *argv[])
{
	int len;
	int cnt;

	if (argc != 1 && argc != 2)
	{
		fputs("? EXPORTDMP: Command error, usage is:\n"
				"               exportdmp {name}\n", stderr);
		exit(1);
	}
	exportpnt = knlExportTable;
	if (argc == 1)
	{
		fputs("Defined export tables:\n    Size Prefix\n", stdout);
		do
		{
			printf("%8d %.8s\n", exportpnt->num, exportpnt->prefix);
		} while ((exportpnt = exportpnt->next) != NULL);
	}
	else
	{
		if ((len = strlen(argv[1])) > 8)
		{
			fputs("? EXPORTDMP: Table name is too long\n", stderr);
			exit(1);
		}
		prefix.ll = 0;
		memcpy(prefix.c, argv[1], len);
		do
		{
			if (*(llong *)(exportpnt->prefix) == prefix.ll)
				break;
		} while ((exportpnt = exportpnt->next) != NULL);
		if (exportpnt == NULL)
		{
			fprintf(stderr, "? EXPORTDMP: Export table %.8s was not found\n",
					prefix.c);
			exit(1);
		}
		cnt = exportpnt->num;
		printf("Export table %.8s has %d entr%s\n     Value Name\n", prefix.c,
				cnt, (cnt == 1) ? "y" : "ies");
		sympnt = (char *)&exportpnt->entry;
		do
		{
			header1 = sympnt[0];
			header2 = sympnt[1];
			valpnt.c = sympnt + (header2 & 0x1F) + 3;
			valsize = header1 - (header2 & 0x1F) - 3;
			switch (valsize)
			{
			 case 0:
				value = (header2 & 0x20)? -1: 0;
				break;

			 case 1:
				value = valpnt.c[0];
				if (header2 & 0x20)
					value |= 0xFFFFFF00;
				break;

			 case 2:
				value = valpnt.s[0];
				if (header2 & 0x20)
					value |= 0xFFFF0000;
				break;

			 case 3:
				value = valpnt.s[0] + (valpnt.c[2] << 16);
				if (header2 & 0x20)
					value |= 0xFF000000;
				break;

			 case 4:
				value = valpnt.l[0];
				break;

			 default:
				fprintf(stderr, "? Illegal value size for symbol %s\n",
					sympnt + 2);
				exit(1);
			}

			printf("  %8X %.8s%.*s\n", value, prefix.c, (header2 & 0x1F) + 1,
					sympnt + 2);
			sympnt += *sympnt;
		} while (--cnt > 0);
	}
	fputs("Complete\n", stdout);
	exit(0);
}
