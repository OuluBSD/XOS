//--------------------------------------------------------------------------*
// conv.c 
// Utility to convert list of files to makefile lines
//
// Written by: John R. Goltz
//
// Edit History:
//-------------------------------------------------------------------------*

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
//      CONV infile outfile

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xos.h>
#include <xostime.h>
#include <xossvc.h>
#include <xoserrmsg.h>
#include <errno.h>

#define VERSION 1
#define EDITNO  0

char prgname[] = "CONV";

int main(
	int   argc, 
	char *argv[])

{
	FILE *ifile;
	FILE *ofile;
	char *epnt;
	char *bspnt;
	int   len;
	char  chr;
	char  bufr[300];

	if (argc != 3)
	{
		fputs("? CONV: Command error, usage is:\n"
			  "          CONV infile outfile\n", stderr);
		exit(1);
	}
	if ((ifile = fopen(argv[1], "r")) == NULL)
		femsg2(prgname, "Error opening input file", -errno, NULL);
	if ((ofile = fopen(argv[2], "w")) == NULL)
		femsg2(prgname, "Error creating output file", -errno, NULL);

	while (fgets(bufr, sizeof(bufr), ifile) != NULL)
	{
		len = strlen(bufr);
		epnt = bufr + len - 1;
		while (epnt >= bufr && (chr = *epnt) == '\n' || chr == '\r')
			epnt--;
		epnt[1] = 0;
		if ((epnt = strchr(bufr, '.')) == NULL ||
				(bspnt = strchr(bufr, '\\')) == NULL)
			printf("! Invalid line skipped: %s\n", bufr);
		else
			fprintf(ofile, "%.*sobj%.*s.obj:	%s\n", bspnt - bufr,
					bufr, epnt - bspnt, bspnt, bufr);
	}
	if (fclose(ofile) < 0)
		femsg2(prgname, "Error closing output file", -errno, NULL);
	fclose(ifile);
	printf("Complete\n");
	return (0);
}
