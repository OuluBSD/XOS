//---------------------------------------------------------------------
// MKBOOT.C - Program to install XOS bootstrap on a disk
//
// Written by: John R. Goltz
//
// Edit History:
// 08/01/91(brn) - Add NOQUIET switch
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

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XCSTRING.H>
#include <PROCARG.H>
#include <MALLOC.H>
#include <XCMALLOC.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRUN.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include <XOSRTN.H>
#include "MKBOOT.H"

#define VERSION 6
#define EDITNO  0

// v1.5 - 16-May-94
//	Changed to allow version 2 IMG files
// v3.0
//	Changed version number to match system conventions
// v3.1 - 30-Aug-94
//	Changed to support ALLEGRO directory conventions
// v3.3 - 6-Sep-94
//	Changed image file extension to .RUN
// v3.4 - 10-Oct-94
//	Changed to support Allegro directory conventions
// v3.5 - 3-Dec-94
//	Changed default program to load to ALLEGRO (was KERNEL)
// v4.0 - 9-Sep-97
//	Changed for XOS.
// v4.1 - 25-Nov-98
//	Changed to handle bootstrap version 5.4 which stores the drive type
//	byte in a different place.
// v4.2 - 8-Dec-98
//	Fixed problem with copying boot block header for bootstrap version 
//	5.4 and above.
// v5.0 - 20-Jan-99
//	Major rewrite to fully support booting from FAT32 file systems.  Also
//	has initial hooks for the XOS file system.  There are major changes to
//	the format of the BOOT.RUN file.  The format of the XOS.BSF file has
//	also changed.  This version will only load bootstrap versions 6.0 and
//	above, although it will preserve the saved boot block from older
//	versions.
// v5.1 - 6-Mar-00
//	Minor change to support bootstrap version 6.2 which supports loading
//	the bootstrap from other than the active partition on FAT12/FAT16
//	disks.  This allows it to be used with boot managers such as Boot
//	Magic.
// v6.0 - 15-Jun-03
//	Modified to handle 3 level FAT32 bootstrap. WARNING: An XOS FAT32
//  bootstrap written with a previous version MUST be removed before
//  a new bootstrap is installed with this version! This version is required
//  to load bootstraps 7.0 and above for FAT32 file systems.  Also, if
//  anything is done that moves the locaiton of the XOS.BSF file on the
//  disk, the FAT32 bootstrap must be removed and reinstalled.

// NOTE:  There was an error in version numbering with bootstrap version
//	  5.3.  As a result, bootstrap version 5.3 should not be used at all!
//	  Due to the bug fixed in MKBOOT 4.2, MKBOOT 4.1 cannot correctly load
//    any bootstrap version 5.4 or above!  Bootstrap versions 5.4 and above
//	  require MKBOOT 4.2 or above.

int  timeout;					// Time-out value
int  flags;

char copymsg[] = "";
char disktype;					// Disk type: 0x80 = hard disk, 0 = floppy
char autoboot;					// TRUE if want auto-boot enabled
char defprog[42] = "XOS";		// Default program to boot

char prgname[] = "MKBOOT";		// Name of this program
char diskname[12];				// Name of disk
char fname[12][64];				// Function key name definitions

void fail(char *str1, long code, char *str2);
void help_print(char *help_string, int state, int newline);
int  hvdisk(char *arg);
void notice(char *str);
int  optauto(arg_data *);
int  optdebug(arg_data *);
int  optdefault(arg_data *);
int  optfx(arg_data *);
int  optquiet(arg_data *);
int  optnoquiet(arg_data *);
int  opttimeout(arg_data *);
void optusage(void);

// Switch settings functions

#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{   {"?"        , 0                   , NULL, OPT(optusage) , 0},
    {"HELP"     , 0                   , NULL, OPT(optusage) , 0},
    {"HEL"      , 0                   , NULL, OPT(optusage) , 0},
    {"H"        , 0                   , NULL, OPT(optusage) , 0},
    {"QUIET"    , 0                   , NULL,     optquiet  , TRUE},
    {"QUI"      , 0                   , NULL,     optquiet  , TRUE},
    {"Q"        , 0                   , NULL,     optquiet  , TRUE},
    {"NOQUIET"  , 0                   , NULL,     optquiet  , FALSE},
    {"NOQUI"    , 0                   , NULL,     optquiet  , FALSE},
    {"NOQ"      , 0                   , NULL,     optquiet  , FALSE},
    {"DEBUG"    , 0                   , NULL,     optdebug  , TRUE},
    {"DEB"      , 0                   , NULL,     optdebug  , TRUE},
    {"NODEBUG"  , 0                   , NULL,     optdebug  , FALSE},
    {"NODEB"    , 0                   , NULL,     optdebug  , FALSE},
    {"TIMEOUT"  , ASF_NVAL            , NULL,     opttimeout, TRUE},
    {"TIM"      , ASF_NVAL            , NULL,     opttimeout, TRUE},
    {"NOTIMEOUT", 0                   , NULL,     opttimeout, FALSE},
    {"NOTIM"    , 0                   , NULL,     opttimeout, FALSE},
    {"NOT"      , 0                   , NULL,     opttimeout, FALSE},
    {"DEFAULT"  , ASF_LSVAL|ASF_VALREQ, NULL,     optdefault, TRUE},
    {"DEF"      , ASF_LSVAL|ASF_VALREQ, NULL,     optdefault, TRUE},
    {"NODEFAULT", 0                   , NULL,     optdefault, FALSE},
    {"NODEF"    , 0                   , NULL,     optdefault, FALSE},
    {"NOD"      , 0                   , NULL,     optdefault, FALSE},
    {"AUTO"     , 0                   , NULL,     optauto   , TRUE},
    {"AUT"      , 0                   , NULL,     optauto   , TRUE},
    {"NOAUTO"   , 0                   , NULL,     optauto   , FALSE},
    {"NOAUT"    , 0                   , NULL,     optauto   , FALSE},
    {"NOA"      , 0                   , NULL,     optauto   , FALSE},
    {"F1"       , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 0},
    {"F2"       , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 1},
    {"F3"       , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 2},
    {"F4"       , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 3},
    {"F5"       , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 4},
    {"F6"       , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 5},
    {"F7"       , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 6},
    {"F8"       , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 7},
    {"F9"       , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 8},
    {"F10"      , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 9},
    {"F11"      , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 10},
    {"F12"      , ASF_LSVAL|ASF_VALREQ, NULL,     optfx     , 11},
    {NULL}
};

// This program reads the file XOSSYS:BOOT.RUN and the boot block of the
//   disk and generates the file SYS:\XOS.BSF and modifies the boot block

//******************************
// Function: main - Main program
// Returned: Exits status
//******************************

int main(argc, argv)
int   argc;
char *argv[];

{
	long maxresrvd;
    int  num;
    char menu;
	char fstype[16];
	char text[64];

    if (argc >= 2)
    {
		++argv;
		procarg((cchar **)argv, PAF_EATQUOTE, options, NULL, hvdisk,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if ((flags & MBF_QUIET) == 0)
        fprintf(stderr, "MKBOOT - version %d.%d\n", VERSION, EDITNO);
    if (diskname[0] == '\0')
    {
        fputs("? MKBOOT: No disk specified\n", stderr);
        exit(1);
    }
    if (autoboot && defprog[0] == '\0')
    {
        fputs("? MKBOOT: Must specify a default program when /AUTO specified\n",
                stderr);
        exit(1);
    }
    if (autoboot && timeout != 0)
        fputs("%% MKBOOT: Time-out value ignored, does not apply with /AUTO\n",
                stderr);
    if (!mkbootf(diskname, &disktype, fstype, &maxresrvd, (flags & MBF_DEBUG) ?
			"XOSSYS:bootx.run" : "XOSSYS:boot.run", fname, defprog, autoboot,
			timeout, flags, fail, notice))
        exit(1);
    if ((flags & MBF_QUIET) == 0)
    {
        printf("MKBOOT: XOS %s %sbootstrap written to %s %s with \n"
                "          the following parameters:\n", fstype,
				(flags & MBF_DEBUG) ? "debug " : "", (disktype) ?
				"hard disk partition": "unpartitioned disk", diskname);
		printf("        Maximum reserved block used: %d\n", maxresrvd);
        if (autoboot)
        printf("                   Interactive mode: %s\n", (autoboot) ?
				"No" : "Yes");
		if (timeout)
			sprintf(text, "%d second%s", timeout, (timeout==1) ? "" : "s");
		else
			strmov(text, "None");
		printf("                    Time-out period: %s\n", text);
		printf("            Default program to load: %s\n", defprog);
		menu = FALSE;
		num = 0;
		do
		{
			if (fname[num][0])
			{
				if (!menu)
				{
					fputs("            Menu items:\n", stdout);
					menu = TRUE;
				}
				printf("              F%d:%s %s\n", num + 1, (num < 9) ?
							" " : "", fname[num]);
			}
		} while (++num < 12);
		if (!menu)
			fputs("            No menu items defined\n", stderr);
    }
    return (0);
}

int hvdisk(
    char *arg)

{
    char *pnt;

    if (diskname[0])
    {
        fputs("? MKBOOT: More than one disk specified\n", stderr);
        exit(1);
    }
    if (strlen(arg) > 10)
    {
        fputs("? MKBOOT: Illegal disk name\n", stderr);
        exit(1);
    }
    pnt = strmov(diskname, strupr(arg)); // Copy disk name
    if (pnt[-1] != ':')
        *pnt = ':';
    return (TRUE);
}


//**************************************************
// Function: help_print - Prints help option entries
// Returned: Nothing
//**************************************************

void help_print(char *help_string, int state, int newline)
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


void optusage(void)
{
    fprintf(stderr, "\n%s version %d.%d (%s) %s\n\n", prgname,
			VERSION, EDITNO, __DATE__, copymsg);
    fprintf(stderr, "%s {/option} {...\}file_specs\n", prgname);
    fprintf(stderr, "\nOptions:\n");
    help_print(" HELP or ? - this message", FALSE, TRUE);
    help_print(" {NO}QUIET   - Supress routine output messages",
			flags & MBF_QUIET, TRUE);
    help_print(" {NO}DEBUG   - Install debug version", flags & MBF_QUIET, TRUE);
    help_print(" {NO}TIMEOUT - Specify timeout before booting default", timeout, TRUE);
    help_print(" DEFAULT     - Specify default program to load", FALSE, TRUE);
    help_print(" {NO}AUTO    - Automatically boot default program", autoboot, TRUE);
    help_print(" F1          - Specify program to load when F1 is typed", FALSE, TRUE);
    help_print(" F2          - Specify program to load when F2 is typed", FALSE, TRUE);
    help_print(" F3          - Specify program to load when F3 is typed", FALSE, TRUE);
    help_print(" F4          - Specify program to load when F4 is typed", FALSE, TRUE);
    help_print(" F5          - Specify program to load when F5 is typed", FALSE, TRUE);
    help_print(" F6          - Specify program to load when F6 is typed", FALSE, TRUE);
    help_print(" F7          - Specify program to load when F7 is typed", FALSE, TRUE);
    help_print(" F8          - Specify program to load when F8 is typed", FALSE, TRUE);
    help_print(" F9          - Specify program to load when F9 is typed", FALSE, TRUE);
    fprintf(stderr, "\nA * shows this option is the current default.\n");
    fprintf(stderr, "All options and values may be abbreviated to 1 or 3 letters.\n");
    exit(EXIT_INVSWT);          // Return as if invalid option
}


//***********************************************************
// Function: optquiet - Process the QUIET and NOQUIET options
// Returned: TRUE always
//***********************************************************

int optquiet(
    arg_data *arg)
{
    if (arg->data)
		flags |= MBF_QUIET;
	else
		flags &= ~MBF_QUIET;
    return (TRUE);
}


//***********************************************************
// Function: optdebug - Process the DEBUG and NODEBUG options
// Returned: TRUE always
//***********************************************************

int optdebug(
    arg_data *arg)
{
    if (arg->data)
		flags |= MBF_DEBUG;
	else
		flags &= ~MBF_DEBUG;
    return (TRUE);
}


//**************************************************
// Function: opttimeout - Process the TIMEOUT option
// Returned: TRUE always
//**************************************************

int opttimeout(
    arg_data *arg)
{
    timeout = (arg->data)? (int)(arg->val.n): 0;
    return (TRUE);
}


//********************************************************
// Function: optauto - Process the AUTO and NOAUTO options 
// Returned: TRUE always
//********************************************************

int optauto(
    arg_data *arg)
{
    autoboot = arg->data;
    return (TRUE);
}


//**************************************************
// Function: optdefault - Process the DEFAULT option
// Returned: TRUE always
//**************************************************

int optdefault(
    arg_data *arg)
{
    if (arg->data && arg->val.s)
    {
        if (strlen(arg->val.s) > 40)
        {
            fputs("? MKBOOT: Default program name is too long\n", stderr);
            exit(1);
        }
        strmov(defprog, arg->val.s);
    }
    else
        defprog[0] = '\0';
    return (TRUE);
}


//*****************************************
// Function: optfx - Process the Fn options
// Returned: TRUE always
//*****************************************

int optfx(
    arg_data *arg)
{
    if (strlen(arg->val.s) > 63)
    {
        fprintf(stderr, "? MKBOOT: Name for F%d is too long\n", arg->data + 1);
        exit(1);
    }
    strmov(fname[arg->data], arg->val.s);
    return (TRUE);
}


void notice(char *str)
{
    puts(str);
}


void fail(char *str1, long code, char *str2)
{
    femsg2(prgname, str1, code, str2);
}
