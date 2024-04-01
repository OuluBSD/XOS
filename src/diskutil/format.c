//---------------------------------------------------------------------
// FORMAT.C - Program to format and initialize file structured disks
//
// Written by: John R. Goltz
//
// Edit History:
//
// 11Nov94  FPJ  Changed FTE to ALG so we could get a clean compile.
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
#include <string.h>
#include <ctype.h>
#include <xcstring.h>
#include <procarg.h>
#include <malloc.h>
#include <xcmalloc.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrun.h>
#include <xoserr.h>
#include <xosdisk.h>
#include <xoserrmsg.h>
#include <xosrtn.h>
#include <xosthreads.h>
#include "fmtfuncs.h"

#define VERSION 3
#define EDITNO  0


#define THRD_BASE  0x40000000 // Base thread

// Define vectors

#define VECT_THRD   60		// First vector used by the thread routines

#define offsetof(strct, member) ((long)(((struct strct *)(NULL))->member))

static long clussize;
static long reserved;

char copymsg[] = "";
char quiet = FALSE;			// TRUE if /QUIET specified
char fatfs = FALSE;			// TRUE if initializing FAT file system
char nofs = FALSE;			// TRUE if shoule not initialize disk
char initonly = FALSE;		// TRUE if only initializing (no format)
char confirm = TRUE;		// TRUE if should confirm before proceeding
char prgname[] = "FORMAT";	// Name of this program

char vollabel[32];
char diskname[12];			// Name of disk


void helpprint(char *help_string, int state, int newline);
int  hvdisk(char *arg);
int  optactive(arg_data *arg);
int  optblocks(arg_data *arg);
int  optclus(arg_data *arg);
int  optconf(arg_data *arg);
int  optfat(arg_data *arg);
int  optgroup(arg_data *arg);
void opthelp(void);
int  optinit(arg_data *arg);
int  optlabel(arg_data *arg);
int  optnofs(void);
int  optquiet(arg_data *arg);
int  optresrvd(arg_data *arg);
int  optrootsz(arg_data *arg);

// Switch settings functions

#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{   {"?"         , 0                   , NULL, OPT(opthelp) , 0},
	{"ACT"       , 0                   , NULL,     optactive, ACT_SET},
	{"ACTIVE"    , 0                   , NULL,     optactive, ACT_SET},
	{"NOACT"     , 0                   , NULL,     optactive, ACT_CLR},
	{"NOACTIVE"  , 0                   , NULL,     optactive, ACT_CLR},
    {"BLO"       , ASF_NVAL|ASF_VALREQ , NULL,     optblocks, 0},
    {"BLOCKS"    , ASF_NVAL|ASF_VALREQ , NULL,     optblocks, 0},
    {"CLU"       , ASF_NVAL|ASF_VALREQ , NULL,     optclus  , 0},
    {"CLUSTER"   , ASF_NVAL|ASF_VALREQ , NULL,     optclus  , 0},
    {"CON"       , 0                   , NULL,     optconf  , TRUE},
    {"CONFIRM"   , 0                   , NULL,     optconf  , TRUE},
    {"FAT"       , 0                   , NULL,     optfat   , 0},
    {"FAT12"     , 0                   , NULL,     optfat   , FS_FAT12},
    {"FAT16"     , 0                   , NULL,     optfat   , FS_FAT16},
    {"FAT32"     , 0                   , NULL,     optfat   , FS_FAT32},
    {"FOR"       , 0                   , NULL,     optinit  , FALSE},
    {"FORMAT"    , 0                   , NULL,     optinit  , FALSE},
	{"RES"       , ASF_NVAL|ASF_VALREQ , NULL,     optresrvd, 0},
	{"RESERVED"  , ASF_NVAL|ASF_VALREQ , NULL,     optresrvd, 0},
	{"ROO"       , ASF_NVAL|ASF_VALREQ , NULL,     optrootsz, 0},
	{"ROOTSIZE"  , ASF_NVAL|ASF_VALREQ , NULL,     optrootsz, 0},
    {"GRO"       , ASF_NVAL|ASF_VALREQ , NULL,     optgroup , 0},
    {"GROUP"     , ASF_NVAL|ASF_VALREQ , NULL,     optgroup , 0},
    {"H"         , 0                   , NULL, OPT(opthelp) , 0},
    {"HEL"       , 0                   , NULL, OPT(opthelp) , 0},
    {"HELP"      , 0                   , NULL, OPT(opthelp) , 0},
    {"INI"       , 0                   , NULL,     optinit  , TRUE},
    {"INIT"      , 0                   , NULL,     optinit  , TRUE},
    {"INITIALIZE", 0                   , NULL,     optinit  , TRUE},
    {"LAB"       , ASF_LSVAL|ASF_VALREQ, NULL,     optlabel , 0},
    {"LABEL"     , ASF_LSVAL|ASF_VALREQ, NULL,     optlabel , 0},
    {"NOCON"     , 0                   , NULL,     optconf  , FALSE},
    {"NOC"       , 0                   , NULL,     optconf  , FALSE},
    {"NOCONFIRM" , 0                   , NULL,     optconf  , FALSE},
    {"NOF"       , 0                   , NULL, OPT(optnofs) , 0},
    {"NOFS"      , 0                   , NULL, OPT(optnofs) , 0},
    {"NOQ"       , 0                   , NULL,     optquiet , FALSE},
    {"NOQUI"     , 0                   , NULL,     optquiet , FALSE},
    {"NOQUIET"   , 0                   , NULL,     optquiet , FALSE},
    {"Q"         , 0                   , NULL,     optquiet , TRUE},
    {"QUI"       , 0                   , NULL,     optquiet , TRUE},
    {"QUIET"     , 0                   , NULL,     optquiet , TRUE},
    {NULL        , 0                   , NULL,     NULL     , 0}
};

// This file provides a main program for calling the disk
//   initialization/formating routines.


//******************************
// Function: main - Main program
// Returned: Exit status
//******************************

int main(
    int   argc,
    char *argv[])
{
///	long   rtn;

    if (argc >= 2)
    {
		++argv;
		procarg((cchar **)argv, PAF_EATQUOTE, options, NULL, hvdisk,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (!quiet)
        fprintf(stderr, "FORMAT - version %d.%d\n", VERSION, EDITNO);
    if (diskname[0] == '\0')
    {
        fputs("? FORMAT: No disk specified\n", stderr);
        exit(1);
    }
	thdCtlInitialize(3, VECT_THRD, THRD_BASE + 0x20000 - sizeof(THDDATA),
			0x1F000, 0);
    if (fatfs)
		fmtfat(diskname, fstype, clussize, reserved, fmtready);
    else
	{
///		fmtxfs();

		printf("? XFS not implemented yet!\n");
		exit (1);
	}
    if (!quiet)
        printf("%% FORMAT: %s complete\n", (initonly)? "Initialization":
                "Format");
    return (0);
}


//********************************************
// Function: opthelp - Process the HELP option
// Returned: Never returns
//********************************************

void opthelp(void)

{
    fprintf(stderr, "\n%s version %d.%d (%s)\n%s\n\n", prgname,
			VERSION, EDITNO, __DATE__, copymsg);
    fputs("Command usage:\n    FORMAT {/option ...} device: {/option ...}\n",
            stdout);
    fprintf(stderr, "\nOptions:\n");
    helpprint(" BLOCKS     - Specify number of blocks on disk", FALSE, TRUE);
    helpprint(" CLUSTER    - Specify cluster size", FALSE, TRUE);
    helpprint(" CONFIRM    - Confirm before preceeding", confirm, TRUE);
    helpprint(" FAT        - Initialize FAT file system on disk",
            !nofs && fatfs, TRUE);
    helpprint(" FORMAT     - Format and initialize disk", !initonly, TRUE);
    helpprint(" GROUP      - Specify allocation group size", FALSE, TRUE);
    helpprint(" HELP or ?  - Display this message", FALSE, TRUE);
    helpprint(" INITIALIZE - Initialize only (do not format)", initonly, TRUE);
    helpprint(" LABEL      - Specify volume label", FALSE, TRUE);
    helpprint(" NOFS       - Do not initialize file system on disk", nofs,
            TRUE);
    helpprint(" {NO}QUIET  - {Do not} supress routine output messages", quiet,
            TRUE);
    helpprint(" XOS        - Initialize XOS file system on disk",
            !nofs && !fatfs, TRUE);
    fprintf(stderr, "\nA * indicates this option is the current default.\n");
    fprintf(stderr, "All options and values may be abbreviated to 1 or 3 "
            "letters.\n");
    exit(EXIT_INVSWT);          // Return as if invalid option
}


//**************************************************
// Function: helpprint - Prints help option entries
// Returned: Nothing
//**************************************************

void helpprint(char *help_string, int state, int newline)

{
    char str_buf[132];

    strcpy(str_buf, help_string);
    if (state)
        strcat(str_buf, " *");

    if (newline)
    {
        fprintf(stderr, "%s\n", str_buf);
    }
    else
        fprintf(stderr, "%-38s", str_buf);

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


//*************************************************************
// Function: optconf - Process the CONFIRM or NOCONFIRM options
// Returned: TRUE
//*************************************************************

int optconf(
    arg_data *arg)

{
    confirm = arg->data;
    return (TRUE);
}


//**************************************************
// Function: optcluster - Process the CLUSTER option
// Returned: TRUE
//**************************************************

int optclus(
    arg_data *arg)
{
    clussize = arg->val.n;
    return (TRUE);
}


//**************************************************
// Function: optresrvd - Process the RESERVED option
// Returned: TRUE
//**************************************************

int optresrvd(
	arg_data *arg)
{
	reserved = arg->val.n;
	return (TRUE);
}


//**********************************************
// Function: optgroup - Process the GROUP option
// Returned: TRUE
//**********************************************

int optgroup(
    arg_data *arg)
{
    groupsize = arg->val.n;
    return (TRUE);
}


//************************************************
// Function: optblocks - Process the BLOCKS option
// Returned: TRUE
//************************************************

int optblocks(
    arg_data *arg)
{
    blocks = arg->val.n;
    return (TRUE);
}


//**************************************************
// Function: optrootsz - Process the ROOTSIZE option
// Returned: TRUE
//**************************************************

int optrootsz(
    arg_data *arg)
{
    rootsize = arg->val.n;
    return (TRUE);
}


//****************************************************
// Function: optblocks - Process the {NO}ACTIVE option
// Returned: TRUE
//****************************************************

int optactive(
    arg_data *arg)

{
    active = (uchar)(arg->data);
    return (TRUE);
}


//**********************************************
// Function: optlabel - Process the LABEL option
// Returned: TRUE
//**********************************************

int optlabel(
    arg_data *arg)

{
    strnmov(vollabel, arg->val.s, 32);
    return (TRUE);
}


//***********************************************
// Function: optfat - Process the FAT{xx} options
// Returned: TRUE
//***********************************************

int optfat(
    arg_data *arg)

{
    fstype = arg->data;
	fatfs = TRUE;
    nofs = FALSE;
    return (TRUE);
}


//********************************************
// Function: optnofs - Process the NOFS option
// Returned: TRUE
//********************************************

int optnofs(void)

{
    nofs = TRUE;
    return (TRUE);
}


//*************************************************************
// Function: optinit - Process the INITIALIZE or FORMAT options
// Returned: TRUE
//*************************************************************

int optinit(
    arg_data *arg)

{
    initonly = arg->val.n;
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

    if (diskname[0])
    {
        fputs("? FORMAT: More than one disk specified\n", stderr);
        exit(1);
    }
    if (strlen(arg) > 10)
    {
        fputs("? FORMAT: Illegal disk name\n", stderr);
        exit(1);
    }
    pnt = strmov(diskname, strupr(arg)); // Copy disk name
    if (pnt[-1] != ':')
        *pnt = ':';
    return (TRUE);
}


int fmtready(
	char *diskname,
	char *basename,
	long  blocks,
	long  clussize,
	long  clusters,
	long  reserved,
	int   lbamode,
	int   fstype,
	long  fatsize,
	long  rootsize)
{
	char bufr1[64];
	char bufr2[64];
	char chr;

	if (basename[0] != 0)
		sprintf(bufr1, "partition %s on disk %s", diskname, basename);
	else
		sprintf(bufr1, "disk %s", diskname);

	if (fstype != FS_FAT32)
		sprintf(bufr2, "   Root directory entires: %d\n", rootsize);
	else
		bufr2[0] = 0;
	printf("%% FORMAT: Ready to format %s\n"
            "         Number of blocks: %,d\n"
            "       Blocks per cluster: %,d\n"
            "       Number of clusters: %,d\n"
			"    Total reserved blocks: %,d\n"
			"                 LBA mode: %s\n"
			"              File system: %s\n"
			"           Blocks per FAT: %,d\n%s", bufr1, blocks, clussize,
			clusters, reserved, (lbamode) ? "Yes" : "No",
			(fstype == FS_FAT12) ? "FAT12" : (fstype == FS_FAT16) ? "FAT16" :
					"FAT32", fatsize, bufr2);
    if (confirm)
    {
        fputs("% FORMAT: Proceed? ", stdout);
        chr = getche();
        fputs("\n", stdout);
        if (toupper(chr) != 'Y')
			return (FALSE);
    }
	return (TRUE);
}

uchar needtext = TRUE;

void formatstatus(
	char *text,
	long  value)
{
	if (needtext)
	{
		printf("%26s: %,7d ", text, value);
		needtext = FALSE;
	}
	else if (value >= 0)
		printf("\b\b\b\b\b\b\b\b%,7d ", value);
	else
	{
		fputs("- Complete\n", stdout);
		needtext = TRUE;
	}
}



void formaterror(long  code, char *msg)
{
	femsg2(prgname, msg, code, NULL);
	exit(1);
}
