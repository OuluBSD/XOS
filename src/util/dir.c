//--------------------------------------------------------------------------*
// DIR.C
// Directory display utility for XOS
//
// Written by: Bruce R. Nevins, pretty much rewritten by John Goltz
//
// Edit History:
// 08/11/88(brn) - Created first version
// 04/13/89(brn) - Fix to display version numbers if used
// 04/20/89(brn) - Add support for long file names
// 05/02/89(brn) - Add ... construct for recursive directories
// 05/25/89(brn) - Add code to get size of screen we are on
// 09/08/89(brn) - Change options and environment string name
// 10/05/89(brn) - Make file names with no extension or "." default to ".*"
//               - Fix case sensitivity in option names and values
//               - Fix options to complain when they have a bad value
// 11/12/89(brn) - Remove support for / in path names
// 	 - Add support for dos device names
// 11/16/89(brn) - Make / or - be the option characters
// 12/10/89(brn) - Fix bug in getlabel returning a . in the name
// 12/30/89(brn) - Add support for procarg
// 01/27/90(brn) - Fix minor bugs and add WIDE switch for DOS compatibility
// 02/09/90(brn) - Make page default to false and listing default to wide
// 04/19/90(brn) - V1.11 Fixed long filename wrapping for VMS files
// 12/13/90(jrg) - Converted to run under XOS v1.6
// 02/27/91(brn) - Fix spelling of ASCENDING
// 03/28/91(brn) - Add support for dynamic configuration of row and column
// 		size.
// 03/28/91(brn) - Add support for global_parameter checks
// 03/29/91(brn) - Add dosquirk support with dos compatible output
// 04/02/91(brn) - Fix bug in doing multiple files and directorys from
// 		command line.
// 05/18/91(brn) - Add code to handle new ioinsinglep call
// 06/17/91(brn) - Fix header print even if no files we found.  Fix bug with
// 		two file not found messages.
// 10/06/91(brn) - Fix print_header to search for the device name from
// 		the right.
// 	   Fix getlabel to not print any errors and to save and
// 		restore fileparm.options.value
// 	   Fix file not found handling
// 12/23/91(brn) - Add before and after switch to handle including and
// 		excluding of dates to be displayed
// 01/07/92(brn) - Fix to get dates if sorting by date with filename only
// 		display
// 1.21 24-Feb-92 (JRG) Major changes to clean up recursive and multiple
// 		directory listings
// 1.22 10-Mar-92 (JRG) Added ER_BUSY to non-fatal errors
// 1.23 23-Apr-92 (JRG) Major changes to use dirscan library routine
// 08/20/92(brn) - Change reference to global.h from local to library
// 05/12/94(brn) - Change version number to reflect 32 bit version
// 03/15/95(sao) - Add progasst package
// 04/14/94(sao) - Changed input type on ALL option (fixes help display)
// 05/13/95(sao) - Changed 'optional' indicator from [] to {}
//  6Jul95 (fpj) - Changed time_t to time_s, to correct bug in handling of
//                 volume ID for disks.
// 3.6  ????????? (JRG) Added several options to support short/long filenames
// 3.7  10-Mar-99 (JRG) Changed to merge multiple file list in same directory,
//                added /X option
// 3.8  26-Mar-99 (JRG) Changed date display to 4 digits
// 4.0   4-Jeb-10 (JRG) Modifed for XOS V4.4.x, added /R switch
//--------------------------------------------------------------------------*


// KNOWN PROBLEMS:
//	Multile specs with recursion do not work. Only the first spec is used.


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
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoswildcard.h>
#include <xoserrmsg.h>
#include <progarg.h>
#include <heapsort.h>
#include <dirscan.h>
#include <global.h>
#include <xosstr.h>

#define WILDCARD "*"
#define COLWIDTH 16

#define VERSION 3
#define EDITNO  8

typedef struct filedescp FILEDESCP;

// Structure for file specification blocks

typedef struct fs FS;
struct  fs
{   FS   *next;					// Pointer to next independent file spec
    FS   *link;					// Pointer to next file spec for same
								//   directory
    long  glbid[4];				// Global device ID for path
    long  devsts;				// Device status bits
    char *ellip;				// Offset of start of ellipsis
    char *name;					// Offset of start of name part
	char  recur;				// DSO_RECUR if elipsis in spec, otherwise 0
    char  spec[4];				// File specification
};

FS *firstspec;
FS *thisspec;

typedef struct xname XNAME;
struct  xname
{   XNAME *next;
    char   name[4];
};

XNAME *firstname;

// File description data structure

struct filedescp
{	FILEDESCP *next;			// Address of next file block
	FILEDESCP *sort;			// Used by heapsort
    long       error;			// Error code	
	long       alloc;
	long       length;
	long       dirpos;
	long       prot;
	time_s     adt;
	time_s     cdt;
	time_s     mdt;
	char      *owner;
	char       filattr;			// File attributes
	char       name[2];
};

char dirspec[300];

FILEDESCP *firstfile;
FILEDESCP *lastfile;
FILEDESCP *thisfile;

// Function prototypes

int   comp(FILEDESCP *one, FILEDESCP *two);
void *getmem(size_t size);
int   nonopt(char *arg);
void  pagecheck(void);
void  printfiles(void);
int   procfile(void);
char *wideentry(FILEDESCP *file, char *linebufr, char *linepnt);
void  showerrmsg(const char *arg, long  code);
void  init_Vars(void);

// Switch settings functions

int  optafter(arg_data *);
int  optall(void);
int  optbefore(arg_data *);
int  optdebug(arg_data *);
int  optdir(void);
int  optdisplay(arg_data *);
int  optfull(void);
int  optlong(void);
int  optnames(void);
int  optnodir(void);
int  optnonames(void);
int  optone(void);
int  optpause(arg_data *);
int  optshort(void);
int  optsort(arg_data *);
int  optx(arg_data *);
int  allhave( arg_data *);

// Stuff needed by dirscan

char owner[36];

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE2PARM  srcattr;
    BYTE2PARM  filattr;
    BYTE4PARM  devsts;
	BYTE4PARM  dirpos;
    BYTE4PARM  length;
    BYTE4PARM  alloc;
    BYTE8PARM  cdate;
    BYTE8PARM  mdate;
    BYTE8PARM  adate;
    BYTE4PARM  prot;
    LNGSTRPARM owner;
    char       end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILEOPTN , XFO_FILE|XFO_VERSION|XFO_MARKDIR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILESPEC , NULL, 0, 0},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR , XA_FILE|XA_DIRECT},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILEATTR , 0},
    {PAR_GET|REP_HEXV, 4 , IOPAR_DEVSTS  , 0},
    {PAR_GET|REP_DECV, 4 , IOPAR_DIRPOS  , 0},
    {PAR_GET|REP_DECV, 4 , IOPAR_LENGTH  , 0},
    {PAR_GET|REP_DECV, 4 , IOPAR_REQALLOC, 0},
    {PAR_GET|REP_HEXV, 8 , IOPAR_CDATE   , 0},
    {PAR_GET|REP_HEXV, 8 , IOPAR_MDATE   , 0},
    {PAR_GET|REP_HEXV, 8 , IOPAR_ADATE   , 0},
    {PAR_GET|REP_HEXV, 4 , IOPAR_PROT    , 0},
    {PAR_GET|REP_STR , 0 , IOPAR_OWNER   , owner, 36, 36}
};

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,	// parmlist - Address of parameter list
    (PROCFILE *)procfile,	// func     - Function called for each file matched
    showerrmsg,				// error    - Function called on error
    DSSORT_ASCEN,			// sort     - Directory sort order
};


SUBOPTS discmd[] =
{   {"T*OTALS"      , "Show statistical totals"},
    {"NOT*OTALS"    , "Do not show totals"},
    {"H*EADING"     , "Show heading"},
    {"NOH*EADING"   , "Do not show heading"},
    {"AT*TRIBUTES"  , "Show file attributes"},
    {"NOAT*TRIBUTES", "Do not show file attributes"},
    {"D*OWN"        , "Sort in columns"},
    {"A*CROSS"      , "Sort in rows"},
    {"E*XPAND"      , "Expand column widths for longest name"},
    {"NOE*XPAND"    , "Do not expand column widths"},
	{"DIR"          , "Include directories"},
	{"NODIR"        , "Do not include directories"},
	{"POS"          , "Show position in directory"},
	{"NOPOS"        , "Do not show position in directory"},
    { NULL          , NULL}
};

SUBOPTS sortcmd[] =
{   {"NO*NE"      , "Do not sort"},
    {"A*SCENDING" , "Sort in ascending order"},
    {"R*EVERSE"   , "Sort in reverse order"},
    {"N*AME"      , "Sort by name"},
    {"E*XTENSION" , "Sort by extension"},
    {"D*ATE"      , "Sort by date"},
    {"SI*ZE"      , "Sort by size"},
    {"DI*RFIRST"  , "Sort directories first"},
    {"NODI*RFIRST", "Do not sort directories first"},
    { NULL        , NULL}
};

// Variables addressed directly in option table

long   shownames = TRUE;	// TRUE if file names should be shown
long   debug = FALSE;		// TRUE if want debug output
long   page = TRUE;			// Page flag
long   lsum = TRUE;			// TRUE if should list summary at end
long   onlytotals = FALSE;	// TRUE if should list totals only
long   recur = FALSE;		// TRUE for recursive listing
long   position = FALSE;	// TRUE if should show directory position

#define AB XA_FILE|XA_RDONLY|XA_HIDDEN|XA_SYSTEM|XA_DIRECT

arg_spec options[] =
{
/// {"AF*TER"    , ASF_VALREQ|ASF_LSVAL, NULL   ,    optafter   , 0 , "Show files after"},
    {"AL*L"      , 0                   , NULL   ,    allhave    , AB, "Show all files"},
/// {"B*EFORE"   , ASF_VALREQ|ASF_LSVAL, NULL   ,    optbefore  , 0 , "Show files before"},
    {"DE*BUG"    , ASF_BOOL|ASF_STORE  , NULL   ,AF(&debug)     , 1 , NULL},
    {"DIR"       , 0                   , NULL   , AF(optdir)    , 0 , "Show directories only"},
    {"NODIR"     , 0                   , NULL   , AF(optnodir)  , 0 , "Do not show directories"},
    {"DI*SPLAY"  , ASF_XSVAL           , &discmd,    optdisplay , 0 , "Specify display options"},
    {"F*ULL"     , 0                   , NULL   , AF(optfull)   , 0 , "Display full listing"},
    {"H*ELP"     , 0                   , NULL   , AF(opthelp)   , 0 , "Display this message" },
    {"L*ONG"     , 0                   , NULL   , AF(optlong)   , 0 , "Show file details"},
    {"N*AMES"    , ASF_BOOL|ASF_STORE  , NULL   ,AF(&shownames) , 1 , "Show file names"},
    {"O*NE"      , 0                   , NULL   , AF(optone)    , 0 , "Display single column, short listing"},
    {"R*ECURSIVE", ASF_BOOL|ASF_STORE  , NULL   ,AF(&recur)     , 1 , "Recursive list" },
    {"P*AUSE"    , ASF_BOOL|ASF_STORE  , NULL   ,AF(&page)      , 1 , "Pause at end of page" },
    {"SH*ORT"    , 0                   , NULL   , AF(optshort)  , 0 , "Display short listing" },
    {"SO*RT"     , ASF_XSVAL           , sortcmd,    optsort    , 0 , "Specify output order" },
    {"T*OTALS"   , ASF_BOOL|ASF_STORE  , NULL   ,AF(&onlytotals), 1 , "Show totals only (no names)" },
    {"W*IDE"     , 0                   , NULL   , AF(optshort)  , 0 , "Display short listing"},
    {"X"         , ASF_VALREQ|ASF_LSVAL, NULL   ,    optx       , 0 , "Specify files to exclude"},
    {"?"         , 0                   , NULL   , AF(opthelp)   , 0 , "Display this message"},
    {NULL}
};

// Option related variables

char   longfmt = FALSE;		// TRUE if long listing
char   fullfmt = FALSE;		// TRUE if full detailed listing
char   prtheading = TRUE;	// TRUE if should print directory heading
char   showattr = TRUE;		// TRUE if should show file attributes
char   onecol = FALSE;		// TRUE if one column simple display
char   datesort = FALSE;	// TRUE if should sort by date
char   extsort = FALSE;		// TRUE if should sort by file extension
char   revsort = FALSE;		// TRUE for reverse sort listing
char   dirsort = TRUE;		// TRUE if should sort subdirectories first
char   sizesort = FALSE;	// TRUE if should sort on file size
char   nosort = FALSE;		// TRUE if should not sort
char   prtacross = TRUE;	// TRUE if should print across
char   prtexpand = FALSE;	// TRUE if should expand columns
char   showdir = TRUE;
char   changed;
int    needed;				// Spaces needed for longest name
int    namesize;			// Length of last file name seen
int    needlength = -1;		// Offset of file length in filedescp
int    needalloc = -1;		// Offset of file allocation in filedescp
int    needcdate = -1;		// Offset of creation date/time in filedecscp
int    needmdate = -1;		// Offset of modify date/time in filedescp
int    needadate = -1;		// Offset of access date/time in filedescp
int    needowner = -1;		// Offset of file owner names in filedescp
int    needprot = -1;		// Offset of file protection in filedescp
int    needname = 0;		// Offset of file name in filedescp
int    curline = -1;		// Current line on the screen
int    needwid = 6;
int    tmpwid;
time_s after_time;			// Time after to display filenames
time_s before_time;			// Time before to display filenames
char   errdone;				// TRUE if have output an error message
char   hvdefault;
int    numlisted = 0;		// Number of directories listed
long   numfiles;
long   filecnt;
long   dircnt;
llong  totallen;
llong  totalalloc;
long   gfilecnt;
long   gdircnt;
llong  gtotallen;
llong  gtotalalloc;
char  *fspnt;

// Misc. variables

PROGINFO pib;
char    copymsg[] = "";
char    prgname[] = "DIR";	// Our programe name
char    envname[] = "^XOS^DIR^OPT"; // The environment option name
char    example[] = "{/options} filespec";
char    description[] = "This command produces a directory listing of the "
    "files on a specified disk drive.  The file specificationx given "
    "determines which files are included in the listing.  Wildcard and "
    "elipsis notation are allowed in the file specification.  If no file "
    "specification is given, *.* is assumed.  More than one file and/or "
    "wildcard may be specified on the command line.  Many options are "
    "possible to the directory listing.  It is recommended that the "
    "user select a preferred directory format using command line options "
    "and enter it as the default in the USTARTUP.BAT file using the "
    "DEFAULT command.";

struct
{   BYTE4PARM modev;
    BYTE4PARM modec;
    BYTE4PARM modes;
    char      end;
} snglparm =
{   {PAR_GET|REP_HEXV, 4, IOPAR_TRMSINPMODE, 0},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, TIM_ECHO},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_IMAGE}
};

struct
{   BYTE4PARM modec;
    BYTE4PARM modes;
    char      end;
} normparm =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
    {PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, 0}
};

struct diparm
{   BYTE4PARM cblksz;
    BYTE4PARM clssz;
    BYTE4PARM avail;
    char     end;
} diparm =
{   {PAR_GET|REP_DECV, 4, IOPAR_DSKSECTSIZE},
    {PAR_GET|REP_DECV, 4, IOPAR_DSKCLSSIZE},
    {PAR_GET|REP_DECV, 4, IOPAR_DSKAVLSPACE}
};

struct
{   char   type[8];
    time_s date;
    char   xxx[8];
    char   name[36];
} vollabel;

QAB lblqab =
{   QFNC_WAIT|QFNC_LABEL,	// open
    0,						// status
    0,						// error
    0,						// amount
    0,						// handle
    0,						// vector
    {0},
    1,						// option
    sizeof(vollabel),		// count
    (char *)&vollabel,		// buffer1
    NULL,					// buffer2
    NULL					// parm
};

extern uint _malloc_amount;
uint   maxmem;

void main(
    int   argc,
    char *argv[])
{
    char *pnt;
    char *envpnt[2];
    FS   *temp;
    long  rtn;
    char  strbuf[256];			// String buffer

    // Set defaults

	reg_pib(&pib);

	init_Vars();

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    // Check Local Configuration Parameters

    if(svcSysGetEnv(envname, (char const **)&envpnt[0]) > 0)
    {
		envpnt[1] = NULL;
		progarg(envpnt, 0, options, NULL, (int (*)(char *))NULL,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    // Process the command line

    if (argc >= 2)
    {
		++argv;
		progarg(argv, 0, options, NULL, nonopt,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    // Set up the conditions

    if (onlytotals)
    {
		lsum = TRUE;
		shownames = FALSE;
    }
    if (firstspec == NULL)
		nonopt(WILDCARD);
    if (!pib.console)
        page = FALSE;

    // Call DIRSCAN to get the names

	if (!showdir)
		fileparm.srcattr.value = XA_FILE;

    thisspec = firstspec;
    do									// Scan the directory
    {
///		printf("### calling dirscan\n");

		if (thisspec->link != NULL)
		{
			rtn = thisspec->name - thisspec->spec;
			pnt = getmem(strlen(thisspec->name) + rtn + 4);
			strmov(strnmov(pnt, thisspec->spec, rtn), "*.*");

			dirscan(pnt, &dsd, recur | thisspec->recur);
			free(pnt);
		}
		else
			dirscan(thisspec->spec, &dsd, recur | DSO_WILDNAME | DSO_WILDEXT |
					thisspec->recur);

///		printf("### back from dirscan\n");

		if (dirspec[0] != 0)			// Anything left to print?
			printfiles();				// Yes - print it
		temp = thisspec->next;
		free(thisspec);					// Give up storage for file spec
	} while ((thisspec = temp) != NULL); // Continue if more to do

	// Display final summary if need to

	if (lsum && (numlisted > 1))
	{
		pagecheck();
		putchar('\n');
		pagecheck();
		printf("[%,d director%s listed containing a grand total of\n", numlisted,
				(numlisted == 1) ? "y": "ies");
		pnt = strbuf;
		if (gdircnt != 0)
			pnt += sprintf(pnt, " %,ld director%s,", gdircnt, (gdircnt == 1) ?
					"y": "ies");
		pnt += sprintf(pnt, " %,ld file%s", gfilecnt, (gfilecnt == 1) ?
				"" : "s");
		if (longfmt)
	    	pnt += sprintf(pnt, ", %,lld byte%s written, %,lld byte%s allocated",
		    		gtotallen, (gtotallen == 1) ? "" : "s", gtotalalloc,
					(gtotalalloc == 1) ? "" : "s");
		pagecheck();
		strmov(pnt, "]\n");
		fputs(strbuf, stdout);
	}
    if (!errdone && (gdircnt + gfilecnt) == 0)
		fputs("? DIR: No matching file(s) found\n", stderr);
    if (debug)
    {
		pagecheck();
		putchar('\n');
		printf("Maximum memory: %,ld, current memory: %,ld\n", maxmem,
				_malloc_amount);
	}
	exit(EXIT_NORM);					// Return with no error
}


//***********************************************************
// Function: init_Vars - Set up the program information block
// Returned: Nothing
//***********************************************************

void init_Vars(void)
{
    // Set Program Information Block variables

    pib.opttbl=options; 				// Load the option table
    pib.kwdtbl=NULL;
    pib.build=__DATE__;
    pib.majedt = VERSION; 				// Major edit number
    pib.minedt = EDITNO; 				// Minor edit number
    pib.copymsg=copymsg;
    pib.prgname=prgname;
    pib.desc=description;
    pib.example=example;
    pib.errno=0;
    getTrmParms();
    getHelpClr();
}


//*****************************************************************
// Function: allhave - Function called progarg when ALL is selected
// Returned: TRUE if should continue, FALSE if should terminate
//*****************************************************************

int allhave( arg_data *arg )
{
    fileparm.srcattr.value=arg->data;
    return TRUE;
}


//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

// This function is called by dirscan for each file found.

long xxesp;

int procfile(void)
{
    FS    *spec;
    XNAME *name;

///	__asm {" mov xxesp, esp"};
///	printf("### procfile: %0x8 %d |%s| %d |%s| %02X %d\n", xxesp,
///			dsd.pathnamelen, dsd.pathname, dsd.filenamelen, dsd.filename,
///			dsd.attr, dsd.error);

	if (dsd.filenamelen < 0)
	{
		if (dirspec[0] != 0)
			printfiles();
		if (dsd.error < 0)				// Bad directory?
		{
			printf("Error scanning directory %s\n", dsd.pathname);
			dirspec[0] = 0;
		}
		else
		{
			strncpy(dirspec, dsd.pathname, 299);
			needwid = 6;
		}
		return (TRUE);
	}
    if (thisspec->link != NULL)			// Is this a complex search?
    {
		spec = thisspec;				// Yes - scan through the list of names
		do								//   and check each one
		{
			if (wildcmp(spec->name, dsd.filename,
					(spec->devsts & DS_CASESEN) ? 1 : 0) == 0)
				break;
		} while ((spec = spec->link) != NULL);
		if (spec == NULL)				// Did we find a match?
			return (TRUE);				// No - skip this one
	}
    if (firstname != NULL)				// Have any excluded names?
    {
		name = firstname;				// Yes - check for match
		do
		{
			if (wildcmp(name->name, dsd.filename,
					(spec->devsts & DS_CASESEN) ? 1 : 0) == 0)
				return (TRUE);			// Match - skip this one
		} while ((name = name->next) != NULL);
    }

    // Here if want to process this file - It is not excluded and we are doing
    //   a simple search or it matches one of our names for a complex search

    if (dsd.error < 0 && !longfmt && !fullfmt)
        dsd.filenamelen += svcSysErrMsg(dsd.error, 2, dsd.filename +
				dsd.filenamelen);
	thisfile = getmem(sizeof(FILEDESCP) + 3 + dsd.filenamelen +
			fileparm.owner.strlen);		// Allocate memory for entry
	namesize = dsd.filenamelen;

	if (dsd.attr & XA_DIRECT)
		namesize++;
	if (needed < namesize)				// Is this a new maximum size?
		needed = namesize;				// Yes - remember it
	strmov(thisfile->name, dsd.filename); // Copy the file name
	if (fileparm.owner.strlen != 0)
	{
		thisfile->owner = thisfile->name + dsd.filenamelen + 1;
		strcpy(thisfile->owner, fileparm.owner.buffer);
	}
	else
		thisfile->owner = NULL;

    if (needlength >= 0)
	{
		thisfile->length = fileparm.length.value;
		if (fileparm.length.value < 10000)
			tmpwid = 6;
		else if (fileparm.length.value < 100000)
			tmpwid = 7;
		else if (fileparm.length.value < 1000000)
			tmpwid = 8;
		else if (fileparm.length.value < 10000000)
			tmpwid = 10;
		else if (fileparm.length.value < 100000000)
			tmpwid = 11;
		else if (fileparm.length.value < 1000000000)
			tmpwid = 12;
		else
			tmpwid = 14;
		if (needwid < tmpwid)
			needwid = tmpwid;
	}
	thisfile->dirpos = fileparm.dirpos.value;
	thisfile->alloc = fileparm.alloc.value;
	thisfile->cdt = *(time_s *)(&fileparm.cdate.value);
	thisfile->mdt = *(time_s *)(&fileparm.mdate.value);
	thisfile->adt = *(time_s *)(&fileparm.adate.value);
	thisfile->prot = fileparm.prot.value;
    if ((thisfile->filattr = dsd.attr) & XA_DIRECT)
		dircnt++;
	else
		filecnt++;
	if (dsd.error >= 0)
	{
		totallen += fileparm.length.value;
		totalalloc += fileparm.alloc.value;
	}
    numfiles++;
    thisfile->error = dsd.error;
    if (firstfile == NULL)
        firstfile = thisfile;
    else
        lastfile->next = thisfile;
    lastfile = thisfile;
    thisfile->next = NULL;
    return (TRUE);
}


//**************************************************************
// Function: printfiles - Print file names for directory listing
// Returned: Nothing
//**************************************************************

int        colwidth;
int        collength;
int        fill;
int        left;
int        numcolumns;
FILEDESCP *colarray[5];

void printfiles(void)
{
    FILEDESCP  *file;
    FILEDESCP  *next;
    FILEDESCP **pnt;
    char       *linepnt;
    int         cnt;
    int         temp;
    char        linebufr[200];

	if (firstfile == NULL)
		return;
	pagecheck();
	putchar('\n');
    if (prtheading)
	{
		pagecheck();
		printf("Directory %s\n", dirspec);
	}
    if (shownames)
    {
        if (longfmt)
            prtacross = TRUE;
        if (!prtacross)
            prtexpand = TRUE;
        if (!nosort)
            firstfile = heapsort(firstfile,
                    (int (*)(void *a, void *b, void *d))comp, NULL);

        if (!longfmt)					// Want short (wide) listing?
        {								// Yes
			// Here to generate a short format listing - This consists of
			//   just the names with as many as will fit on each line and
			//   still line up.

			if ((numcolumns = (onecol)? 1: (pib.screen_width /
					((prtexpand) ? (needed + 1) : COLWIDTH))) > 5)
				numcolumns = 5;
			colwidth = pib.screen_width/numcolumns;
			linepnt = linebufr;
            left = numcolumns;
            if (prtacross)
            {
                do						// Loop for each file
                {
                    linepnt = wideentry(firstfile, linebufr, linepnt);
                    next = firstfile->next;
                    free(firstfile);	// Give up description block
                } while ((firstfile = next) != NULL);
                pagecheck();
                strmov(linepnt, "\n");
                fputs(linebufr, stdout); // Output the final line
            }
            else
            {
                do
                {   collength = (int)((numfiles+numcolumns-1)/numcolumns);
                    if (page)
                    {
                        if ((temp = pib.screen_height - curline - 3) <= 0)
                            temp = pib.screen_height - 3;
                        if (collength > temp)
                            collength = temp;
                    }
                    numfiles -= collength * numcolumns;
                    cnt = numcolumns;
                    pnt = colarray;
                    do
                    {   *pnt++ = firstfile;
                        temp = collength;
                        while (firstfile != NULL && --temp > 0)
                            firstfile = firstfile->next;
                        if (firstfile != NULL)
                        {
                            next = firstfile->next;
                            firstfile->next = NULL;
                            firstfile = next;
                        }
                    } while (--cnt > 0);
                    do
                    {   cnt = numcolumns;
                        pnt = colarray;
                        while ((file = *pnt) != NULL && --cnt >= 0)
                        {
                            linepnt = wideentry(file, linebufr, linepnt);
                            *pnt = file->next;
                            free(file);
                            pnt++;
                        }
                        pagecheck();
                        strmov(linepnt, "\n");
                        fputs(linebufr, stdout);
                        linepnt = linebufr;
                        left = numcolumns;
                    } while (colarray[0] != NULL);
                    if (page && firstfile != NULL)
                    {
                        pagecheck();
                        putchar('\n');
					}
                } while (firstfile != NULL);
            }
		}
		else if (!fullfmt)				// Want normal listing?
		{
			// Here to generate a normal format listing - This lists each
			//   item on a line with attribute, creation date/time and size
			//   with each file.

			while (firstfile != NULL)	// Loop for each file
            {
				linepnt = linebufr;
				if (showattr)		// Want to display attributes?
				{					// Yes
					*linepnt++ = ((firstfile->filattr & XA_RDONLY) != 0) ?
							'R' : '-';
					*linepnt++ = ((firstfile->filattr & XA_HIDDEN) != 0) ?
							'H' : '-';
					*linepnt++ = ((firstfile->filattr & XA_SYSTEM) != 0) ?
							'S' : '-';
					*linepnt++ = ((firstfile->filattr & XA_ARCH) != 0) ?
							'M' : '-';
				}
				linepnt += sdt2str(linepnt, "%Z %h:%m:%s %D-%3n-%l",
						(time_sz *)&firstfile->mdt);
				linepnt += sprintf(linepnt, "%,*d", needwid,
						firstfile->length);
				*linepnt++ = ' ';
				linepnt = strmov(linepnt, firstfile->name);
				if (position)
					linepnt += sprintf(linepnt, " (0x%X)", firstfile->dirpos);
				pagecheck();
                strmov(linepnt, "\n");
                fputs(linebufr, stdout); // Output the line
				if (firstfile->error < 0)
				{
					linepnt = strmov(linebufr, "     ");
					linepnt += (int)svcSysErrMsg(firstfile->error, 3, linepnt);
					*linepnt++ = '\n';
					*linepnt = 0;
					pagecheck();
	                fputs(linebufr, stdout); // Output the error line
				}
                next = firstfile->next;
                free(firstfile);		// Give up description block
            	firstfile = next;
			}
        }
        else							// Must want full listing
        {
			// Here to generate a full listing - This uses several lines for
			//   each files and includes all normally available information
			//   about the item.

			do							// Loop for each file
			{
				pagecheck();
				printf((firstfile->filattr & XA_DIRECT) ? "Directory: %s\n" :
						"File: %s\n", firstfile->name);

				if (firstfile->error < 0)
				{
					pagecheck();
                    printf("  ? Error accessing %s:\n", (firstfile->filattr &
							XA_DIRECT) ? "directory" : "file");
					svcSysErrMsg(firstfile->error, 3, linebufr);
					pagecheck();
					printf("    %s\n", linebufr);
                }

				pagecheck();
				linepnt = strmov(linebufr, "  File attributes: ");
				linepnt = strmov(linepnt, ((firstfile->filattr & XA_RDONLY)
						!= 0)? "Read only, ": "Read/write, ");
				linepnt = strmov(linepnt, ((firstfile->filattr & XA_HIDDEN)
						!= 0)? "Hidden, ": "Visible, ");
				linepnt = strmov(linepnt, ((firstfile->filattr & XA_SYSTEM)
						!= 0)? "System, ": "User, ");
				linepnt = strmov(linepnt, ((firstfile->filattr & XA_ARCH)
						!= 0)? "Modified\n": "Not modified\n");
				fputs(linebufr, stdout);
				sdt2str(linebufr, "%Z  Created:  %h:%m:%s %D-%3n-%l\n",
						(time_sz *)&firstfile->cdt);
				pagecheck();
				fputs(linebufr, stdout);
				sdt2str(linebufr, "%Z  Modified: %h:%m:%s %D-%3n-%l\n",
						(time_sz *)&firstfile->mdt);
				pagecheck();
				fputs(linebufr, stdout);
				sdt2str(linebufr, "%Z  Accessed: %h:%m:%s %D-%3n-%l\n",
						(time_sz *)&firstfile->adt);
				pagecheck();
				fputs(linebufr, stdout);
				pagecheck();
				printf("  Written length:   %,ld\n", firstfile->length);
				pagecheck();
				printf("  Allocated length: %,ld\n", firstfile->alloc);
                next = firstfile->next;
                free(firstfile);		// Give up description block
            } while ((firstfile = next) != NULL);
        }
    }
    if (lsum)
    {
        pagecheck();
		linepnt = strmov(linebufr, "[");
		if (dircnt != 0)
			linepnt += sprintf(linepnt, "%,d director%s, ", dircnt,
					(dircnt==1)? "y": "ies");
		linepnt += sprintf(linepnt, "%,d file%s", filecnt,
				(filecnt==1) ? "" : "s");
		if (longfmt)
			linepnt += sprintf(linepnt, ",%s %,lld bytes written, %,lld bytes"
					" allocated", (longfmt && dircnt != 0) ? "\n" : "",
					totallen, totalalloc);
		strmov(linepnt, "]\n");
		fputs(linebufr, stdout);
    }
    firstfile = NULL;
    gfilecnt += filecnt;
    filecnt = 0;						// Add in to the grand totals and
    gdircnt += dircnt;					//   reset some counts
    dircnt = 0;
    gtotallen += totallen;
    totallen = 0;
	gtotalalloc += totalalloc;
    totalalloc = 0;
    numfiles = 0;
    numlisted++;						// Count the directory we have listed
    return;
}


//************************************************
// Function: wideentry - Put file name into buffer
//		for wide format listing
// Returned: Updated buffer pointer
//************************************************

char *wideentry(
    FILEDESCP *file,
    char      *linebufr,
    char      *linepnt)
{
    int   size;

    size = strlen(file->name);			// Get length of file name
    needed = (size + colwidth)/colwidth; // Get number of columns
    if (needed > left)					// Do we have enough left?
    {									// No - start a new line
        pagecheck();
        strmov(linepnt, "\n");
        fputs(linebufr, stdout);
        linepnt = linebufr;
        left = numcolumns;
    }									// Have more space on line - get
    else if (left != numcolumns)		//   number of spaces to fill
    {									//   out previous field
        fill = colwidth - (int)(linepnt - linebufr) % colwidth;
        do
        {   *linepnt++ = ' ';			// Output that many spaces
        } while(--fill > 0);
    }
    linepnt = strmov(linepnt, file->name);
    left -= needed;						// Copy file name
    return (linepnt);
}


//************************************************
// Function: comp - Compare two filenames for sort
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//************************************************

int comp(
    FILEDESCP *one,
    FILEDESCP *two)
{
    char  *aext;
    char  *bext;
    int    retval;
    char   onetype;

    if (dirsort)			// Sorting directories first?
    {					// Yes - see if only one is directory
        onetype = one->filattr & XA_DIRECT;
        if (onetype != (two->filattr & XA_DIRECT))
            return ((onetype)? -1: 1);	// Yes - this overrides all else!
    }
    if (revsort)						// Want reverse order sort?
    {
        FILEDESCP *temp;

        temp = one;
        one = two;
        two = temp;
    }
    if (datesort)						// Date sort?
    {
        if (one->mdt.dt != two->mdt.dt)	// Yes
            return ((one->mdt.dt < two->mdt.dt)? -1: 1);
    }									// If same time, sort by name
    else if (extsort)					// Extension sort?
    {
        aext = strrchr(one->name, '.');	// Yes - find extensions
        bext = strrchr(two->name, '.');
        if (aext != NULL && bext != NULL) // Have both extensions?
        {								// Yes - base it on this if different
            if ((retval = strcmp(aext, bext)) != 0)
                return (retval);
        }								// If same, base it on name
        else if (aext != bext)			// One or no extensions
            return ((aext == NULL)? -1: 1); // One with no extension is first
    }									// No extensions - base it on name
    else if (sizesort)					// File length sort?
    {									// Yes
        if (one->length != two->length) // If different, use length
            return ((one->length < two->length)? -1: 1);
    }									// If same length, use name
    return (stricmp(one->name, two->name)); // Sort on name
}


//******************************************************************
// Function: pagecheck - Check the page count and pause if necessary
// Returned: Nothing
//******************************************************************

void pagecheck(void)
{
    long temp;					// Temp int for keyboard status
    int  more;

    if (page && (++curline >= pib.screen_height - 2))
    {
        fputs("\33[7m-MORE- ^C, G, H, Q, <Enter> or <Space>\33[0m", stdout);
        more = TRUE;
        while (more == TRUE)
        {
            temp = svcIoInSingleP(DH_STDTRM, (char *)&snglparm);
            normparm.modes = snglparm.modev;
            svcIoInBlockP(DH_STDTRM, NULL, 0, (char *)&normparm);
            more = FALSE;				// Assume good input

            switch (toupper(((int)temp) & 0x7F))
            {
             case 'Q':					// Quit!, don't print the rest
             case 0x03:
                fputs("\r\33[K", stdout);
                exit(EXIT_NORM);

             case 'G':					// Don't ask for any more
                page = FALSE;
                break;

             case '\r':
             case ' ':					// Do another screen full
                curline = 0;
                break;

             case 'H':
             case '?':
             default:					// Tell him this is wrong
				fputs("\r\33[K\33[7m"
						" ^C      - Exit                    \n"
						"  G      - Go, don't ask for -MORE-\n"
						"  H or ? - Help, this message      \n"
						"  Q      - Quit program            \n"
						" <Enter> - Next screen             \n"
						" <Space> - Next screen             \n"
						"-MORE- ^C, G, H, Q, <Enter> or <Space>\33[0m",
						stdout);

              case 0:
                more = TRUE;			// Loop back and check again
                break;
            }
        }
        fputs("\r\33[K", stdout);
    }
}


//**********************************************
// Function: optdisplay - Process DISPLAY option
// Returned: Nothing
//**********************************************

int optdisplay(
    arg_data *arg)
{
    if ((arg->flags & ADF_XSVAL) == 0)
	return (TRUE);
    switch ((int)arg->val.n)
    {
     case 0:			// TOTALS
        lsum = TRUE;
        break;

     case 1:			// NOTOTALS
        lsum = FALSE;
        break;

     case 2:			// HEADING
        prtheading = TRUE;
        break;

     case 3:			// NOHEADING
        prtheading = FALSE;
        break;

     case 4:			// ATTRIBUTES - Show file attributes
		showattr = TRUE;
		break;

     case 5:			// NOATTRIBUTES - Do not show file attributes
		showattr = FALSE;
		break;

     case 6:			// DOWN
        prtacross = FALSE;
        break;

     case 7:			// ACROSS
        prtacross = TRUE;
        break;

     case 8:			// EXPAND
        prtexpand = TRUE;
        break;

     case 9:			// NOEXPAND
        prtexpand = FALSE;
        break;

	 case 10:			// DIR
		showdir = TRUE;
		break;

	 case 11:			// NODIR
		showdir = FALSE;
		break;

	 case 12:			// POS
		position = TRUE;
		break;

	 case 13:			// NOPOS
		position = FALSE;
		break;

     default:
		fprintf(stderr, "? %s: Invalid DISPLAY option value, %d\n",
				prgname, arg->val.n);
		exit(EXIT_INVSWT);
    }
    return (TRUE);
}


//******************************************
// Function: optafter - Process AFTER option
// Returned: Nothing
//******************************************

int optafter(
    arg_data *arg)
{
    char *time_ptr;

    time_ptr = arg->val.s;
    printf("after time = %s\n", time_ptr);
    return (TRUE);
}


//**************************************
// Function: optall - Process ALL option
// Returned: Nothing
//**************************************

int optall(void)
{
    fileparm.srcattr.value = XA_FILE|XA_RDONLY|XA_HIDDEN|XA_SYSTEM|XA_DIRECT;
    return (TRUE);
}


//********************************************
// Function: optbefore - Process BEFORE option
// Returned: Nothing
//********************************************

int optbefore(
    arg_data *arg)
{
    char *time_ptr;

    time_ptr = arg->val.s;
    printf("before time = %s\n", time_ptr);
    return (TRUE);
}


//**************************************
// Function: optdir - Process DIR option
// Returned: Nothing
//**************************************

int optdir(void)
{
    fileparm.srcattr.value = XA_DIRECT;	// Look for directories only
    return (TRUE);
}


//******************************************
// Function: optnodir - Process NODIR option
// Returned: Nothing
//******************************************

int optnodir(void)
{
    fileparm.srcattr.value &= ~XA_DIRECT; // Do not look for directories
    return (TRUE);
}


//**************************************
// Function: optone - Process ONE option
// Returned: Nothing
//**************************************

int optone(void)
{
    onecol = TRUE;
    optshort();
    return (TRUE);
}


//****************************************
// Function: optfull - Process FULL option
// Returned: Nothing
//****************************************

int optfull(void)
{
    fullfmt = TRUE;
    longfmt = TRUE;
    needalloc = 0;
    needowner = 4;
    needprot = 40;
    needadate = 44;
    needmdate = 52;
    needcdate = 60;
    needlength = 68;
    needname = 72;
    return (TRUE);
}


//******************************************
// Function: optshort - Process SHORT option
// Returned: Nothing
//******************************************

int optshort(void)
{
    longfmt = FALSE;
    fullfmt = FALSE;
    needalloc = -1;
    needowner = -1;
    needprot = -1;
    needadate = -1;
    needmdate = -1;
    needcdate = -1;
    needlength = -1;
    needname = 0;
    return (TRUE);
}


//****************************************
// Function: optlong - Process int option
// Returned: Nothing
//****************************************

int optlong(void)
{
    longfmt = TRUE;
    fullfmt = FALSE;
    needalloc = -1;
    needowner = 0;
    needprot = 36;
    needadate = -1;
    needmdate = -1;
    needcdate = 40;
    needlength = 48;
    needname = 52;
    return (TRUE);
}


//****************************************
// Function: optsort - Process SORT option
// Returned: Nothing
//****************************************

int optsort(
    arg_data *arg)
{
    if ((arg->flags & ADF_XSVAL) == 0)
    {
        datesort = TRUE;
        revsort = TRUE;
        extsort = FALSE;
        nosort = FALSE;
		return (TRUE);
    }
    switch ((int)arg->val.n)
    {
     case 0:				// NONE - Do not sort
        nosort = TRUE;
        dirsort = FALSE;
        revsort = FALSE;
        extsort = FALSE;
        datesort = FALSE;
        break;

     case 1:				// ASCENDING - Sort in ascending order
        revsort = FALSE;
        nosort = FALSE;
        break;

     case 2:				// REVERSE - Sort in reverse order
        revsort = TRUE;
        nosort = FALSE;
        break;

     case 3:				// NAME - Sort by name
        datesort = FALSE;
        extsort = FALSE;
        sizesort = FALSE;
        nosort = FALSE;
        break;

     case 4:				// EXTENSION - Sort by extension
        extsort = TRUE;
        datesort = FALSE;
        sizesort = FALSE;
        nosort = FALSE;
        break;

     case 5:				// DATE - Sort by date
        datesort = TRUE;
        extsort = FALSE;
        sizesort = FALSE;
        nosort = FALSE;
        break;

     case 6:				// SIZE - Sort by size
        sizesort = TRUE;
        extsort = FALSE;
        datesort = FALSE;
        nosort = FALSE;
        break;

     case 7:				// DIRFIRST - Sort directories first
        dirsort = TRUE;
        nosort = FALSE;
        break;

     case 8:				// NODIRFIRST - Do not sort directories
        dirsort = FALSE;		//   first
        nosort = FALSE;
        break;

     default:
        fprintf(stderr, "? %s: Invalid SORT option value, %d\n",
                prgname, arg->val.n);
        exit(EXIT_INVSWT);
    }
    return (TRUE);
}

//**********************************
// Function: optx - Process X option
// Returned: Nothing
//**********************************

int optx(
    arg_data *arg)
{
    XNAME *name;

    name = getmem(arg->length + sizeof(XNAME) + 2);
    strmov(name->name, arg->val.s);
    name->next = firstname;
    firstname = name;
    return (TRUE);
}


//********************************************
// Function: nonopt - process non-option input
// Returned: Nothing
//********************************************

int nonopt(
    char *arg)
{
    FS   *spec;
    FS   *spnt;
    char *pnt;
    long  rtn;
    int   size;
    char  chr;

    static struct
    {	BYTE16PARM  glbid;
		BYTE4PARM   devsts;
		char        end;
    } glbidparm =
    {	{PAR_GET|REP_HEXV, 16, IOPAR_GLBID , 0},
		{PAR_GET|REP_HEXV,  4, IOPAR_DEVSTS, 0}
    };

	size = strlen(arg);
	spec = getmem(size + sizeof(FS) + 8); // Allow for stuff that might be
	spec->link = NULL;					  //   appended to the file spec
	strmov(spec->spec, arg);

    // Get the global device ID for the directory we are searching. This value
	//   is used to determine if directories are identical and can be merged
	//   for a single listing.

	if ((rtn = svcIoDevParm(XO_ODFS, spec->spec, (char *)&glbidparm)) < 0)
    {									// Error getting global device ID?
		spec->glbid[0] = 0xFFFFFFFF;	// Yes - make a fake unique ID
		spec->glbid[1] = (long)spec;
		spec->glbid[2] = spec->glbid[3] = 0;
    }
    else								// No - use the real global device ID
		memcpy(spec->glbid, glbidparm.glbid.value, 16);
    spec->devsts = glbidparm.devsts.value;
    spec->name = pnt = spec->spec;
    while ((chr = *pnt++) != 0)			// Find the start of the name part
    {
		if (chr == ':' || chr == '\\' || chr == '/')
		    spec->name = pnt;
    }
	if ((spec->name == (spec->spec + 4) || spec->name[-5] == '\\' ||
			spec->name[-5] == ':') && strncmp(spec->name - 4, "...", 3) == 0)
	{
		strmov(spec->name - 4, spec->name);
		spec->recur = DSO_RECUR;
	}
	else
		spec->recur = 0;

    if (firstspec == NULL)				// Is this the first file spec?
        firstspec = spec;				// Yes - put in on the list
    else								// No
    {	
        spnt = firstspec;				// See if have a duplicate path
        rtn = spec->name - spec->ellip;
		do
		{
	    	if (memcmp(spec->glbid, spnt->glbid, 16) == 0)
			{
				if (spec->ellip != NULL)
				{
					if (rtn != (spnt->name - spnt->ellip) ||
						strnicmp(spec->ellip, spnt->ellip, rtn) != 0)
					continue;
				}
				break;
			}
		} while ((spnt = spnt->next) != NULL);
		if (spnt != NULL)				// Is it a duplicate?
		{
			spnt->link = spec;			// Yes
			spnt->recur |= spec->recur;
		}
		else
			thisspec->next = spec;
	}
	spec->next = spec->link = NULL;
	thisspec = spec;
	return (TRUE);
}


//*****************************************************
// Function: getmem - Get memory with malloc, exit with
//		error if out of memory
// Returned: Pointer to memory obtained
//*****************************************************

void *getmem(
    size_t size)
{
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL)
    {
        fputs("? DIR: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
    if (maxmem < _malloc_amount)
        maxmem = _malloc_amount;
    return (ptr);
}


//*************************************************
// Function: showerrmsg - Display XOS error message
// Returned: Nothing
//*************************************************

void showerrmsg(
    const char *arg,
    long  code)
{
    char buffer[80];            // Buffer to receive error message

    if (numfiles != 0)
        printfiles();
    pagecheck();
    if (code != 0)
    {
        svcSysErrMsg(code, 3, buffer);	// Get error message
        fprintf(stderr, "\n? DIR: %s%s%s\n", buffer, (arg[0] != '\0')? "; ": "",
                arg);			// Output error message
    }
    else
        fprintf(stderr, "\n? DIR: %s\n", arg);
    errdone = TRUE;
}
