//-----------------------------------------------------------------------
//  DISMOUNT.C - XOS utility to dismount a disk drive
//
//  Written by: John R. Goltz
//
//  Edit history:
//  ------------
//  08/20/92(brn) - Add comment header
//  03/16/95(sao) - Added progasst package
//  05/13/95(sao) - Changed 'optional' indicators
//  05/16/95(sao) - Changed exit codes to reflect XOSRTN
//  18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                  optusage() to opthelp().
//  16Jun95 (fpj) - Cleaned up code; got rid up femsg() calls.
//-----------------------------------------------------------------------

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
#include <errno.h>
#include <xos.h>
#include <xosdisk.h>
#include <xoserr.h>
#include <xosrtn.h>
#include <xoserrmsg.h>
#include <xossvc.h>
#include <progarg.h>

#define PROGRAM     DISMOUNT	// Program name
#define VERSION     3			// Program version number
#define EDIT        4			// Program edit number

#define string(x)   stringx(x)
#define stringx(x)  #x

char *diskname = NULL;			// Name of disk to dismount

long quiet = FALSE;				// TRUE if /QUIET specified

// Prototypes

static void getarg(char *argv[]);
static int  getkey(char *keyword);

static arg_spec options[] =
{	{"Q*UIET", ASF_BOOL|ASF_STORE, NULL, AF(&quiet) , TRUE, "No output except error messages" },
    {"H*ELP" , 0                 , NULL, AF(opthelp), 0   , "Output this message" },
    {"?"     , 0                 , NULL, AF(opthelp), 0   , "Output this message" },
    {NULL}
};

PROGINFO pib;

char  copymsg[] = "";
char  prgname[] = string(PROGRAM);
char  envname[] = "^ALG^" string(PROGRAM) "^OPT";
char  example[] = "{/options} disk:";
char  description[] = "DISMOUNT removes a disk from the system.  All " \
    "information about the disk is discarded.  If there are any unwritten " \
    "blocks in the disk cache for the disk, they are discarded.  This " \
    "command is normally not needed.  It should never be needed for " \
    "fixed (non-removable) disks.  It can be used with removable disks " \
    "to insure that data does not remain in memory after a disk has been " \
    "changed.  The system normally does this automatically.  This command " \
    "should be used with care.  It should not be issued when any output " \
    "files are open on the disk unless data loss can be tolerated.";

struct diskparm
{   TEXT8PARM  class;
    char       end;
} diskparm =
{   {(PAR_SET | REP_TEXT), 8, IOPAR_CLASS, "DISK"}
};

static QAB diskqab =
{   QFNC_WAIT | QFNC_OPEN,	// open
    0,						// status
    0,						// error
    0,						// amount
    0,						// handle
    0,						// vector
	{0},
    XO_RAW | XO_NOMOUNT,	// option
    0,						// count
    NULL,					// buffer1
    NULL,					// buffer2
    (char *)&diskparm		// parm
};


//*********************************************************
// Function: main - Main program entry for DISMOUNT utility
// Returned: The usual EXIT_xxx codes
//*********************************************************

int main(
    int   argc,
    char *argv[])

{
    long rval;					// Returned SVC code

    (void)argc;

    getarg(++argv);						// Parse the arguments

    diskqab.buffer1 = diskname;

    // First check to see if the disk is real

    if ((rval = svcIoQueue(&diskqab)) < 0 || (rval = diskqab.error) < 0)
    {
		if (rval == ER_PARMV)
			errormsg(0, "!Device %s is not a disk", diskname);
		else
		    errormsg(rval, "!Error accessing disk %s", diskname);
	}

    // Then try to dismount it

    diskqab.func = QFNC_WAIT | QFNC_SPECIAL;
    diskqab.option = DSF_DISMOUNT;

    if ((rval = svcIoQueue(&diskqab)) < 0 || (rval = diskqab.error) < 0)
	    errormsg(rval, "!Error dismounting disk %s", diskname);
    if (!quiet)
    {
        if (diskqab.amount)
            printf("Disk %s dismounted\n", diskname);
        else
            printf("Disk %s was not mounted\n", diskname);
    }
    return (0);
}


//**********************************************
// Function: getarg - Get command-line arguments
// Returned: Nothing
//**********************************************

static void getarg(
    char *argv[])
{
    int  rval;

    static char *array[] = { NULL, NULL };

    reg_pib(&pib);						// FIXME: what does this do?

    // set Program Information Block variables

    pib.majedt = VERSION;				// Version number
    pib.minedt = EDIT;					// Edit number
    pib.errno = 0;
    pib.copymsg = copymsg;
    pib.prgname = prgname;
    pib.build = __DATE__;
    pib.desc = description;
    pib.example = example;
    pib.opttbl = options;				// Load the option table
    pib.kwdtbl = NULL;

    getTrmParms();						// FIXME: what does this do?
    getHelpClr();						// FIXME: what does this do?

    // Check for environment variable first

    if (svcSysGetEnv(envname, (const char **)&array[0]) > 0)
        progarg(array, PAF_EATQUOTE, options, NULL, getkey, NULL, NULL,
                NULL);

    // Process the command-line options

    rval = progarg(argv, PAF_EATQUOTE, options, NULL, getkey, NULL, NULL,
                NULL);

    if (rval < 0)
        errormsg(rval, "!Option or keyword error");
    if (diskname == NULL)
        errormsg(0, "!No disk specified");
}


//**************************************************
// Function: getkey - Get general keyword
// Returned: 0 if no error, else negative error code
//**************************************************

static int getkey(
    char *keyword)
{
    char *pnt;
    int   len;

    if (diskname != NULL)
        errormsg(0, "!Too many disk devices specified");
    len = strlen(keyword);    
    if ((diskname = malloc(len + 2)) == NULL)
		errormsg(-errno, "!Can't get memory for disk name");
    strcpy(diskname, keyword);
    if ((pnt = strchr(diskname, ':')) == NULL)
    {
		diskname[len] = ':';
		diskname[len+1] = 0;
    }
    else if (pnt[1] != 0)
		errormsg(0, "!%s is not a valid disk name", keyword);
    strupr(diskname);
    return (0);
}
