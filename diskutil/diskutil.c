//---------------------------------------------------------------------
// diskutil.c - Program to initialize partitions on a hard disk
//
// Written by: John R. Goltz
//
// Edit History:
//
//---------------------------------------------------------------------

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
#include <stdarg.h>
#include <string.h>
#include <xcstring.h>
#include <ctype.h>
#include <xos.h>
#include <xoserrmsg.h>
#include <xossvc.h>
#include <xosdisk.h>
#include <procarg.h>
#include <diskutilfuncs.h>

#define VERSION 2
#define EDITNO  0

long beginblk = -1;
long length = 0;
long wipeamnt = 0;
int  removeval = 0;
int  logicalval = 0;
int  activeval = 0;
int  typeval = 0;

#define offsetof(strct, member) ((long)(((struct strct *)(NULL))->member))

char quiet = FALSE;				// TRUE if /QUIET specified
char doinit = FALSE;			// TRUE if should initialize the MBR
char doclear = FALSE;			// TRUE if should clear the MBR
char prgname[] = "DISKUTIL";	// Name of this program
char dskname[12];				// Name of disk

static void confirm(char *name, char *text);
static void errfunc(si32 code, char *fmt, ...);

static int  hvdisk(char *arg);
static int  optactive(arg_data *arg);
static int  optclear(arg_data *arg);
static int  optwipe(arg_data *arg);
static int  optcreate(arg_data *arg);
static int  optinit(arg_data *arg);
static int  optlength(arg_data *arg);
static int  optlogical(arg_data *arg);
static int  optremove(arg_data *arg);
static int  opttype(arg_data *arg);
static void opthelp(void);

// Switch settings functions

#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{   {"?"         , 0                   , NULL, OPT(opthelp)  , 0},
    {"H"         , 0                   , NULL, OPT(opthelp)  , 0},
    {"HEL"       , 0                   , NULL, OPT(opthelp)  , 0},
    {"HELP"      , 0                   , NULL, OPT(opthelp)  , 0},
    {"CRE"       , ASF_NVAL|ASF_VALREQ , NULL,     optcreate , 0},
    {"CREATE"    , ASF_NVAL|ASF_VALREQ , NULL,     optcreate , 0},
    {"LEN"       , ASF_NVAL|ASF_VALREQ , NULL,     optlength , 0},
    {"LENGTH"    , ASF_NVAL|ASF_VALREQ , NULL,     optlength , 0},
    {"TYP"       , ASF_NVAL|ASF_VALREQ , NULL,     opttype   , 0},
    {"TYPE"      , ASF_NVAL|ASF_VALREQ , NULL,     opttype   , 0},

    {"LOG"       , ASF_NVAL|ASF_VALREQ , NULL,     optlogical, 0},
    {"LOGICAL"   , ASF_NVAL|ASF_VALREQ , NULL,     optlogical, 0},

    {"REM"       , ASF_NVAL|ASF_VALREQ , NULL,     optremove , 0},
    {"REMOVE"    , ASF_NVAL|ASF_VALREQ , NULL,     optremove , 0},

    {"INITIALIZE", 0                   , NULL,     optinit   , 0},
    {"CLEAR___"  , 0                   , NULL,     optclear  , 0},
    {"WIPE___"   , ASF_NVAL|ASF_VALREQ , NULL,     optwipe   , 0},
    {"ACT"       , ASF_LSVAL|ASF_VALREQ, NULL,     optactive , 0},
    {"ACTIVE"    , ASF_LSVAL|ASF_VALREQ, NULL,     optactive , 0},
    {NULL        , 0                   , NULL,     NULL      , 0}
};

// This file provides a main program for calling the hard disk partitioning
//   routines.

//******************************
// Function: main - Main program
// Returned: Exit status
//******************************

int main(
    int   argc,
    char *argv[])

{
	MBR   mbr;
	PTBL *pnt;
	int   num;

	if (argc >= 2)
	{
		++argv;
		procarg((cchar **)argv, PAF_EATQUOTE, options, NULL, hvdisk,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
	}
	if (!quiet)
		fprintf(stderr, "DISKUTIL - version %d.%d\n", VERSION, EDITNO);
	if (dskname[0] == '\0')
	{
		fputs("? DISKUTIL: No disk specified\n", stderr);
		exit(1);
	}
	if (beginblk != -1)
	{
		if (removeval != 0 || doinit || doclear)
			errormsg(0, "!Conflicting options specified");
		if (typeval == 0)
			errormsg(0, "!No partition type specified");
		if (logicalval != 0)
			errormsg(0, "!This version does can not create logical "
					"partitions");
		if (typeval == PRTN_FAT16 || typeval == PRTN_FAT16X)
			typeval = (diskutil_lbamode) ? PRTN_FAT16X : PRTN_FAT16;
		if (typeval == PRTN_FAT32 || typeval == PRTN_FAT32X)
			typeval = (diskutil_lbamode) ? PRTN_FAT32X : PRTN_FAT32;
		if (!diskutilopen(dskname, errfunc) || !diskutilmakepart(&mbr, typeval,
				activeval, beginblk, length) || !diskutilfinish())
			exit(1);
	}
	else if (removeval != 0)
		errormsg(0, "!/REMOVE not implmented yet");
	else if (doinit)
	{
		confirm(dskname, "initialize");
		if (doclear)
			errormsg( 0, "!Conflicting options specified");
		if (!diskutilopen(dskname, errfunc) || !diskutilinitmbr() ||
				!diskutilfinish())
			exit(1);
	}
	else if (doclear)
	{
		confirm(dskname, "clear");
		if (!diskutilopen(dskname, errfunc) || !diskutilwipe(1) ||
				!diskutilfinish())
			exit(1);
	}
	else if (wipeamnt > 0)
	{
		confirm(dskname, "wipe");
		if (!diskutilopen(dskname, errfunc) || !diskutilwipe(wipeamnt) ||
				!diskutilfinish())
			exit(1);
	}
	else
	{
		if (!diskutilopen(dskname, errfunc) || !diskutilreadmbr(&mbr))
			exit(1);
		printf("\nDisk ID: 0x%08X\n", mbr.diskid);
		fputs("\nSlot Boot ----First----- Prtn -----Last-----\n"
				"num  flag Head Sect Cyln type Head Sect Cyln    First block "
				"Nmbr of blocks\n", stdout);
		pnt = mbr.ptbl;
		num = 1;
		do
		{
			if (*(llong *)pnt == 0 && *(llong *)&pnt->begin == 0)
				printf(" %2d    Empty\n", num);
			else
				printf(" %2d  0x%02X %4d %4d %4d 0x%02X %4d %4d %4d %,14d "
						"%,14d\n", num, pnt->boot, pnt->bgnhead,
						pnt->bgnsect & 0x3F, pnt->bgncyln +
						((pnt->bgnsect & 0x0C0) << 2), pnt->type,
						pnt->endhead, pnt->endsect & 0x3F, pnt->endcyln +
						((pnt->endsect & 0xC0) << 2), pnt->begin, pnt->length);
			pnt++;
		} while (++num <= 4);
		fputs("\n", stdout);
	}
	if (!quiet)
		fputs("DISKUTIL: Complete\n", stdout);
	return (0);
}


//***************************************************************
// Function: errfunc - Called the the diskutil functions on error
// Returned: Nothing
//***************************************************************

static void errfunc(
	si32  code,
	char *fmt, ...)
{
	va_list pi;
	char    text[300];
	char    emsg[100];

	if (fmt != NULL)
	{
    	va_start(pi, fmt);
    	vsprintf(text, fmt, pi);
	}
	else
		text[0] = 0;
	if (code < 0)
		svcSysErrMsg(code, 0x03, emsg);
	else
		emsg[0] = 0;
	if (text[0] == 0)
		fprintf(stderr, "? DISKUTIL: %s\n", emsg);
	else
	{
		fprintf(stderr, "? DISKUTIL: %s\n", text);
		if (emsg[0] != 0)
			fprintf(stderr, "            %s\n", emsg);
	}
}


//********************************************
// Function: opthelp - Process the HELP option
// Returned: Never returns
//********************************************

void opthelp(void)

{
	fprintf(stderr, "\n%s version %d.%d (%s)\n\n", prgname,
			VERSION, EDITNO, __DATE__);
    fputs("Command usage:\n    DISKUTIL {/option ...} device: {/option ...}\n"
"Options:\n"
"  BEGIN      - Beginning block number for partition\n"
"  LENGTH     - Partition length in blocks\n"
"  TYPE       - Partition type\n"
"  ACTIVE     - Set a partition active\n"
"  LOGICAL    - Create a logical partition is to be created. The value gives\n"
"                 the extended partition to contain the new partition.\n"
"  REMOVE     - Remove partition (number)\n"
"  INITIALIZE - Initialize the MBR (WARNING: This deletes EVERYTHING on the\n"
"                 disk!\n"
"  HELP or ?  - Display this message\n"
"If no options are specified, a list of partitions is generated. To create\n"
"  a partition BEGIN, LENGTH, TYPE, and (optionally) ACTIVE must be specified.\n"
"  To remove a partition REMOVE (only) must be specified. To initialize the\n"
"  MRB INITIALIZE (only) must be specified.\n"
"All options except INIAIALIZE may be abbreviated to 1 or 3 letters.\n",
		stdout);
    exit(0);
}


//**********************************************************
// Function: optquiet - Process the QUIET or NOQUIET options
// Returned: TRUE
//**********************************************************

int optquiet(
    arg_data *arg)
{
    quiet = arg->data;
    return (TRUE);
}


//************************************************
// Function: optcreate - Process the CREATE option
// Returned: TRUE
//************************************************

int optcreate(
    arg_data *arg)
{
    if ((beginblk = arg->val.n) == 0)
		errormsg(0, "!Initial block for partition cannot be 0");
    return (TRUE);
}


//************************************************
// Function: optlength - Process the LENGTH option
// Returned: TRUE
//************************************************

int optlength(
    arg_data *arg)
{
    length = arg->val.n;
    return (TRUE);
}


//************************************************
// Function: optactive - Process the ACTIVE option
// Returned: TRUE
//************************************************

int optactive(
    arg_data *arg)
{
    activeval = arg->val.n;
    return (TRUE);
}


//**************************************************
// Function: optlogical - Process the LOGICAL option
// Returned: TRUE
//**************************************************

int optlogical(
    arg_data *arg)
{
    logicalval = arg->val.n;
    return (TRUE);
}


//********************************************
// Function: opttype - Process the TYPE option
// Returned: TRUE
//********************************************

int opttype(
    arg_data *arg)
{
    typeval = arg->val.n;
    return (TRUE);
}


//************************************************
// Function: optremove - Process the REMOVE option
// Returned: TRUE
//************************************************

int optremove(
    arg_data *arg)

{
    removeval = arg->val.n;
    return (TRUE);
}


//**************************************************
// Function: optinit - Process the INITIALIZE option
// Returned: TRUE
//**************************************************

int optinit(
    arg_data *arg)

{
	arg = arg;

	doinit = TRUE;
    return (TRUE);
}


//*************************************************
// Function: optclear - Process the CLEAR___ option
// Returned: TRUE
//*************************************************

// This is an "undocumented" function intended for use when debugging
//   the disk setup stuff

int optclear(
    arg_data *arg)

{
	arg = arg;

	doclear = TRUE;
    return (TRUE);
}


//***********************************************
// Function: optwipe - Process the WIPE___ option
// Returned: TRUE
//***********************************************

// This is an "undocumented" function intended for use when debugging
//   the disk setup stuff

int optwipe(
    arg_data *arg)

{
	wipeamnt = arg->val.n;
    return (TRUE);
}


//***************************************************
// Function: hvdisk - Process non-option arguments
// Returned: TRUE if normal, does not return if error
//***************************************************

int hvdisk(
    char *arg)

{
    char *pnt;

    if (dskname[0])
    {
        fputs("? FORMAT: More than one disk specified\n", stderr);
        exit(1);
    }
    if (strlen(arg) > 10)
    {
        fputs("? FORMAT: Illegal disk name\n", stderr);
        exit(1);
    }
    pnt = strmov(dskname, strupr(arg)); // Copy disk name
    if (pnt[-1] != ':')
        *pnt = ':';
    return (TRUE);
}



static void confirm(
	char *name,
	char *text)

{
	int  len;
	char bufr[64];
	char conf[64];

	printf("WARNING: You are about to %s the MBR on disk %s. Do you really\n"
			"         want to do this? ", text, name);
	fgets(bufr, sizeof(bufr), stdin);
	if (toupper(bufr[0]) != 'Y')
	{
		fputs("Canceled!\n", stdout);
		exit(0);
	}
	sprintf(conf, "CONFIRM %s", name);
	printf("Please confirm this by typing \"%s\": ", conf);
	fgets(bufr, sizeof(bufr), stdin);
	len = strlen(bufr);
	if (bufr[len - 1] == '\n')
		bufr[len - 1] = 0;
	if (strcmp(bufr, conf) != 0)
	{
		fputs("Canceled!\n", stdout);
		exit(0);
	}
}


int initready(
	char *text)

{
	text = text;

	return (TRUE);
}
