//--------------------------------------------------------------------------*
// DUMP.C
// Program to dump binary files
//
// Written by: John R. Goltz
//
// Edit History:
// 1.0 - 08/20/92(brn) - Created first version
// 1.2 - 19-Jun-97 - Changed block numbering to start with 0
//
//-------------------------------------------------------------------------*/

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
#include <xcmalloc.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.H>
#include <xosstr.h>
#include <global.h>
#include <progarg.h>

#define WIDTH     16			// Number of bytes for each line
#define BLOCKSIZE 512

#define VERS      2
#define EDIT      0

char   *fspec;					// File specification
int     fslen;
long    beginval;				// Beginning offset in file
long    endval = 0x7FFFFFFF;	// Ending offset in file

PROGINFO pib;

char    copymsg[] = "";
char    prgname[] = "DUMP";		// Our programe name
char    envname[] = "^XOS^DUMP^OPT"; // The environment option name
char    example[] = "{/options} filespec";
char    description[] = "This command produces a text dump of the file "
	    "specified.  Output is to standard output which can be redirected "
		"if desired";

long   indev;
uchar *buffer;

struct
{   BYTE1PARM  fileatr;
    char       end;
} openparms =
{   {PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR, XA_FILE|XA_DIRECT}
};

int nonopt(char *arg);
int optbegin(arg_data *);
int optend(arg_data *);

arg_spec options[] =
{	{"BEG*IN", ASF_VALREQ|ASF_NVAL, NULL,    optbegin, 0, "Specify beginning offset in file (rounded down to beginning of a block)"},
    {"END"   , ASF_VALREQ|ASF_NVAL, NULL,    optend  , 0, "Specify ending offset in file (rounded up to end of a block)"},
    {"H*ELP" , 0                  , NULL, AF(opthelp), 0, "Display this message"},
    {"?"     , 0                  , NULL, AF(opthelp), 0, "Display this message"},
    {NULL}
};


int main(
	int   argc,
	char *argv[])

{   
    long   size;
    int    blockcnt;
	int    blockend;
    int    offset;
    int    ncnt;
    int    acnt;
    int    fill;
    uchar *inpnt;
    uchar *begin;
    char  *linepnt;
    char  *envpnt[2];
	uchar  buffer[512];
    char   strbuf[256];			// String buffer
    uchar  chr;

    // Set defaults

	reg_pib(&pib);
    pib.opttbl=options; 				// Load the option table
    pib.kwdtbl=NULL;
    pib.build=__DATE__;
    pib.majedt = VERS; 					// Major edit number
    pib.minedt = EDIT;	 				// Minor edit number
    pib.copymsg=copymsg;
    pib.prgname=prgname;
    pib.desc=description;
    pib.example=example;
    pib.errno=0;
    getTrmParms();
    getHelpClr();

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    // Check Local Configuration Parameters

    if(svcSysGetEnv(envname, (const char **)&envpnt[0]) > 0)
    {
		envpnt[1] = NULL;
		progarg(envpnt, 0, options, NULL, NULL, NULL, NULL, NULL);
    }

    // Process the command line

    if (argc >= 2)
    {
		++argv;
		progarg(argv, 0, options, NULL, nonopt, NULL, NULL, NULL);
    }
    printf("\nDUMP Version %d.%d\n", VERS, EDIT);

	// Open the input file and set up for IO

    if (fspec == NULL)
        indev = DH_STDIN;		/* If no filename, use stdin */
    else
    {
        if ((indev = svcIoOpen((fspec != NULL && fspec[fslen-1] == ':') ?
                (XO_IN|XO_PHYS) : XO_IN, fspec,
				(char *)&openparms)) < 0)
            femsg2(prgname, "Error opening input file", indev, argv[1]);
    }
    blockcnt = beginval >> 9;
	if ((size = svcIoSetPos(indev, beginval, 0)) < 0)
		femsg2(prgname, "Error setting IO position for input file", size, NULL);
	blockend = (endval - beginval) >> 9;

	// Read and dump each block

    do
    {
        if ((size = svcIoInBlock(indev, buffer, 512)) < 0)
        {
            if (size != ER_EOF)
                femsg2(prgname, "Error reading input file", size, NULL);
            fputs("\nEnd of file\n", stdout);
            svcIoClose(indev, 0);
            return (0);
        }
        inpnt = buffer;
        printf("\nBlock %d  Offset %lX\n\n", blockcnt, ((long)blockcnt) << 9);
        offset = 0;
        while (size > 0)
        {
            linepnt = strbuf + sprintf(strbuf, "%4.4X> ", offset);
            offset += WIDTH;

            ncnt = WIDTH;
            if (ncnt > size)
                ncnt = (int)size;
            size -= ncnt;
            acnt = ncnt;
            fill = WIDTH - ncnt;
            begin = inpnt;
            while (--ncnt >= 0)
                linepnt += sprintf(linepnt, "%2.2X ", *inpnt++);
            while (--fill >= 0)
                linepnt = strmov(linepnt, "   ");
            linepnt = strmov(linepnt, "  | ");
            while (--acnt >= 0)
            {
                chr = *begin++ & 0x7F;
                *linepnt++ = isprint(chr)? chr: '.';
            }
            strmov(linepnt, " |\n");
            fputs(strbuf, stdout);
        }
    } while (++blockcnt < blockend);
	return (0);
}

//********************************************
// Function: nonopt - Process non-option input
// Returned: Nothing
//********************************************

int nonopt(
    char *arg)

{
	if (fspec != NULL)
	{
		fputs("? DUMP: More than one file specified\n", stderr);
		exit (1);
	}
	fslen = strlen(arg);
	fspec = getspace(fslen + 1);
	memcpy(fspec, arg, fslen + 1);
	return (TRUE);
}


//******************************************
// Function: optbegin - Process BEGIN option
// Returned: Nothing
//******************************************

int optbegin(
	arg_data *arg)

{
	beginval = arg->val.n & ~0x1FF;
	return (TRUE);
}


//**************************************
// Function: optend - Process END option
// Returned: Nothing
//**************************************

int  optend(
	arg_data *arg)

{
	endval = (arg->val.n + 0x1FF) & ~0x1FF;
	return (TRUE);
}
