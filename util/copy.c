//--------------------------------------------------------------------------*
// COPY.C
// File copy utility for XOS
//
// Written by John Goltz
//
//--------------------------------------------------------------------------*

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

// 4.0 11-Mar-99 First version after rewrite
// 4.1 This is another complete rewrite of the XOS copy utility.  It supports
//   most of the features from the previous (3.6) and adds several new
//   features and major bug fixes:
//	   Ellipsis copies work correctly when copying directories with no files
//	   /ERROR option added to control error handling on wild-card copies
//	   /Y option added which is the same as the DOS COPY /Y option
//	   /EMPTY option added to control creating empty directories when
//	     doing ellipsis copy
//	  /NOTOTALS option added to easily suppress totals
//	  /QUIET and /VERBOSE work better
// 4.2 This version mostly has changes to support XOS 4.3.0 and the
//    corresponding rewrite of dirscan. It also removes the "move" command
//    features (which never really worked right anyway). The "ellipsis"
//    notation is only supported as the final directory spec. A /recursive
//    option has been added which is equivalent to both source and destination
//    ellipsis.

// Valid usage:
//   copy src dst
//     Copies src to dst. Any attempt to copy a file to itself fails. Wildcards
//     are allowed in both src and dst. Recursion is allowed.
//   copy src
//     Same as copy src *.*, that is, copies src to the current directory with
//     the same name and extension.
//   copy src1+src2+... dst
//     Concatanates all source files and copies to dst. Wildcards are not
//     allowed.

// If a destination file is not specified, *.* is assumed. Destination
//   wildcards specify the entire name or extension. *.ext, name.*, and *.*
//   are the only allowed forms. If there are multiple periods in a name,
//   the extension part is everything following the last period. Everything
//   before the last period (including any periods) is part of the name part.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <HEAPSORT.H>
#include <xoserrmsg.h>
#include <xossvc.h>
#include <xostime.h>
#include <progarg.h>
#include <dirscan.h>
#include <global.h>
#include <xosstr.h>

typedef struct fs FS;
struct fs
{   FS  *next;
    char spec[8];
};

typedef struct fd FD;
struct fd
{   FD   *nxtfile;
    FD   *nxtdir;
    char *dstspec;
    long  level;
    char  dironly;
    char  spec[1];
};

int version = 4;
int editno  = 2;

long  srcdev = -1;		// Source device handle
long  dstdev = -1;		// Destination device handle

char *spintbl[] = {"\b/", "\b-", "\b\\", "\b|"};
long  spin;
long  stslstchr;

ullong bytecnt = 0;
ullong result;
time_s starttime;
time_s stoptime;
time_s xfertime;

FS    dfltspec = {NULL, "*.*"};
FS   *srcspec;
FS   *thisspec;
FS   *dstspec;
char *dstname;
char *dstext;
char *srcname;

FD   *firstdir;
FD   *lastdir;
FD   *firstfile;
FD   *lastfile;
FD   *thisfile;

long  srcwild;				// TRUE if have source wild-card longacter
long  dstnamewild;			// TRUE if wild-card in destination name
long  dstextwild;			// TRUE if wild-card in destination extension
long  isterm;				// TRUE if stdout is controlling terminal
long  doconcat;				// TRUE if doing concatanation
long  concatnxt;			// TRUE if saw + last in command line
long  recur;				// Recursive options
  #define DSTRECUR 0x1000	// Destination recursion
  #define SRCRECUR 0x0001	// Source recursion
long  filecnt;
long  dircnt;

char  owner[36];

char *recurstr;
int   recurlen;

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE4PARM  length;
    BYTE8PARM  cdate;
    BYTE8PARM  mdate;
    char       end;
} srcparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN},
    {PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, 512, 512},
    {PAR_GET|REP_DECV, 4, IOPAR_LENGTH},
    {PAR_GET|REP_HEXV, 8, IOPAR_CDATE},
    {PAR_GET|REP_HEXV, 8, IOPAR_MDATE}
};

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE8PARM  cdate;
    BYTE8PARM  mdate;
    char       end;
} dstparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN},
    {PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, 512, 512},
    {PAR_SET|REP_HEXV, 8, IOPAR_CDATE},
    {PAR_SET|REP_HEXV, 8, IOPAR_MDATE}
};

struct
{	BYTE4PARM  filoptn;
	LNGSTRPARM filspec;
	char       end;
} renparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, 0},
    {PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, 512, 512},
};

int    deloutfile = FALSE;
int    image = FALSE;
int    copysize = 64*1024;
uchar *copybufr;

char   dirpath[512];
long   dirlevel;

char   prgname[] = "COPY";			// Our programe name
char   envname[] = "^XOS^COPY^OPT"; // The environment option name
char   example[] = "{{/}{-}option{=sub}} {...\\}source1{+sourcen} "
	"{...\\}destination";
char   description[] = "This program will copy one or more files from most "
	"standard devices.";

struct 
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE2PARM  srcattr;
    char       end;
} pathparm =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_VOLUME|XFO_NODENUM|XFO_NODENAME|
			XFO_RVOLUME|XFO_PATH|XFO_FILE},
    {PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, 512, 512},
    {PAR_SET|REP_HEXV, 2, IOPAR_SRCATTR, XA_DIRECT}
};

struct
{	BYTE4PARM  filoptn;
	LNGSTRPARM filspec;
	BYTE2PARM  srcattr;
	BYTE2PARM  filattr;
	char       end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_MARKDIR|XFO_FILE},
    {PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL},
    {PAR_SET|REP_HEXV, 2, IOPAR_SRCATTR, XA_FILE},
    {PAR_GET|REP_HEXV, 2, IOPAR_FILEATTR},
};

long   select = FALSE;
long   dosort = TRUE;
long   quiet = FALSE;
long   verbose = TRUE;
long   overwrite = FALSE;
long   erraction = -1;
long   crempty = TRUE;
char  *begindir = NULL;

uchar  totalfiles = TRUE;
uchar  totalbytes = TRUE;
uchar  rate = TRUE;

PROGINFO pib;

// Function prototypes

int   askuser(char *msg, int quit);
void  closeoutfile(int last);
int   comp(FD *one, FD *two);
void  copyfile(char *srcstr, int last);
int   dodirectory(void);
void  errordel(char *delspec, int last);
void  showerrmsg(const char *arg, long  code);
void  fileerror(int nl, long errcode, char *text, char *name, char *delspec,
    int last);
void  finish(void);
void *getmem(size_t size);
long  makedir(char *spec, long level);
int   nonopt(char *arg);
int   optbegin(arg_data *arg);
int   optbuffer(arg_data *);
int   opterror(arg_data *);
void  opthelp(void);
int   optnodir(void);
int   optnototals(arg_data *);
int   opttotals(arg_data *);
int   procfile(void);

// dirscan data

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,
						// parmlist    - Address of parameter list
    (int (*)(DIRSCANDATA *))procfile,
						// func        - Function called for each file matched
    showerrmsg,			// error       - Function called on error
    DSSORT_ASCEN		// sort        - Directory sort order
};

// progarg data

SUBOPTS totalscmd[] =
{   {"F*ILES"  , "Display total number of files copied"},
    {"NOF*ILES", "Don't display total number of files copied"},
    {"B*YTES"  , "Display total number of bytes copied"},
    {"NOB*YTES", "Don't display total number of bytes copied"},
    {"R*ATE"   , "Display total transfer rate for copy"},
    {"NOR*ATE" , "Don't display total transfer rate for copy"},
    {NULL}
};

SUBOPTS errorcmd[] =
{   {"Q*UIT"    , "Quit copying after error"},
    {"A*SK"     , "Ask user for instructions after error"},
    {"C*ONTINUE", "Continue copying after error"},
    {NULL}
};

#define RCB (SRCRECUR|DSTRECUR)

arg_spec options[] =
{
//  {"BE*FORE"   , ASF_VALREQ|ASF_LSVAL, NULL     ,    optbefore    , 0    ,"Transfer all matching files before this Date/Time"},
//  {"SI*NCE"    , ASF_VALREQ|ASF_LSVAL, NULL     ,    optsince     , 0    ,"Transfer all matching files since this Date/Time"},
    {"BU*FFER"   , ASF_VALREQ|ASF_NVAL , NULL     ,    optbuffer    , 0    ,"Specify data buffer size"},
	{"BEGIN"     , ASF_LSVAL           , NULL     ,    optbegin     , 0    ,"Specify beginning directory for recursive transfer"},
    {"S*ELECT"   , ASF_BOOL|ASF_STORE  , NULL     ,AF(&select)      , TRUE ,"Select which files to transfer"},
    {"O*VER"     , ASF_BOOL|ASF_STORE  , NULL     ,AF(&overwrite)   , TRUE ,"Overwrite overwritting existing files without confirmation"},
    {"EMP*TY"    , ASF_BOOL|ASF_STORE  , NULL     ,AF(&crempty)     , TRUE ,"Create empty directries during recursive transfer"},
    {"D*ELETE"   , ASF_BOOL|ASF_STORE  , NULL     ,AF(&deloutfile)  , TRUE ,"Delete incomplete output files"},
    {"ERR*OR"    , ASF_XSVAL           , errorcmd ,    opterror     , 0    ,"Specify action to take on error"},
    {"DOS*DRIVE" , ASF_BOOL|ASF_STORE  , NULL     ,AF(&gcp_dosdrive), TRUE ,"Override the global DOSDRIVE setting"},
    {"I*MAGE"    , ASF_BOOL|ASF_STORE  , NULL     ,AF(&image)       , TRUE ,"Enable image mode (non-file) I/O"},
    {"Q*UIET"    , ASF_BOOL|ASF_STORE  , NULL     ,AF(&quiet)       , TRUE ,"Don't display any status"},
    {"R*ECURSIVE", ASF_BOOL|ASF_STORE  , NULL     ,AF(&recur)       , RCB  ,"Don't display any status"},
    {"V*ERBOSE"  , ASF_BOOL|ASF_STORE  , NULL     ,AF(&verbose)     , TRUE ,"Display full status"},
    {"SO*RT"     , ASF_BOOL|ASF_STORE  , NULL     ,AF(&dosort)      , TRUE ,"Sort file list before transferring"},
    {"T*OTALS"   , ASF_XSVAL           , totalscmd,    opttotals    , 0    ,"Display summary when complete"},
    {"NOT*OTALS" , 0                   , NULL     ,    optnototals  , 0    ,"Don't display summary"},
    {"H*ELP"     , 0                   , NULL     , AF(opthelp)     , 0    ,"Display this message"},
    {"?"         , 0                   , NULL     , AF(opthelp)     , 0    ,"Display this message"},
    {NULL}
};


//***************************************
// Function: main - Main program function
// Returned: Never returns
//***************************************

void main(
    int   argc,
    char *argv[])
{
    char *envpnt[2];
    char *pnt;
    int   rtn;
    char  chr;
    char  srcrtn[512];
    char  dstrtn[512];
    char  strbuf[512];		// String buffer

    // Set defaults

    reg_pib(&pib);

    // Set Program Information Block variables

    pib.opttbl = options; 				// Load the option table
    pib.kwdtbl = NULL;
    pib.build = __DATE__;
    pib.majedt = version; 				// Major edit number
    pib.minedt = editno; 				// Minor edit number
    pib.copymsg = "";
    pib.prgname = prgname;
    pib.desc = description;
    pib.example = example;
    pib.errno = 0;
    getTrmParms();
    getHelpClr();

    srcparms.filspec.buffer = srcrtn;
    dstparms.filspec.buffer = dstrtn;

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    // Check Local Configuration Parameters

    if(svcSysGetEnv(envname, (char const **)&envpnt[0]) > 0)
    {
		envpnt[1] = NULL;
		progarg(envpnt, 0, options, NULL, (int (*)(char *))NULL,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
	}
	isterm = fisctrm(stdout);

    // Process the command line

    if (argc >= 2)
    {
		++argv;
		progarg(argv, 0, options, NULL, nonopt,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if (dstspec == NULL)				// Destination specified?
		dstspec = &dfltspec;			// No - use default

    // Determine if we have any wild-card characters or if recursion is
	//   indicated in any source specs. (This is a very simple check.  If
	//   there are wild-card characters in the device name or directory
	//   fields, we will see them here. In this case, we just let dirscan
	//   complain about the illegal wild-card usage!) Source wild-cards or
	//   recursion are NOT allowed if concatanation is specified.

    if ((thisspec = srcspec) == NULL)
    {
		fprintf(stderr, "? %s: No files specified, type %s /H for help\n",
				prgname, prgname);
		exit(0);
    }
	do
    {
		pnt = thisspec->spec;
		while ((chr = *pnt++) != 0)
		{
			if (chr == '*' || chr == '?')
				srcwild = TRUE;
		}
		if (doconcat && srcwild)
		{
			fputs("? COPY: Wild-cards are not allowed with concatenation\n",
					stderr);
			exit (1);
		}
    } while ((thisspec = thisspec->next) != NULL);

	// Determine if we have any destination wild-card characters.

    // Destination wild cards are allowed only to indicate complete fields.
    //   That is, they can specify the source path, name, or extension.  When
    //   a destination wild card is specified, no other character can appear
    //   in the name or extension field.  The * must be the last character
    //   in the path field.  A ... is allowed as the last element in the
	//   destination path only when there is a corresponding ... as the last
	//   element in the source path.

    pnt = dstname = dstspec->spec;	// Find start of the destination name
	while ((chr = *pnt++) != 0)
	{
		if (chr == '\\' || chr == '/')
			dstname = pnt;
	}
	if (!image && pnt[-2] == ':')	// If not image and only a destination
	{								//   device was specified, add a \ so
		pnt[-1] = '\\';				//   the stuff below will work right
		pnt[0] = 0;
		dstname = pnt;
	}
	if ((dstname == (dstspec->spec + 4) || dstname[-5] == '\\' ||
			dstname[-5] == ':') && strncmp(dstname - 4, "...", 3) == 0)
	{
		strmov(dstname - 4, dstname);
		dstname -= 4;
		recur |= DSTRECUR;
	}
    if (*dstname == 0)					// Null name?
    {
		dstname = "*.*";			// Yes
		dstext = dstname + 2;
		dstnamewild = dstextwild = TRUE;
    }
    else
    {
		dstext = NULL;					// Find start of destination extension
		pnt = dstname;
		while ((chr = *pnt) != 0)
		{
			pnt++;
			if (chr == '.')
				dstext = pnt;
		}
		if (dstext == NULL)
			dstext = pnt;
		if (dstname[0] == '*')			// Wild card destination name?
		{								// Yes
			if (dstname[1] != '.' && dstname[1] != 0) // Is it valid?
			{
				fputs("? COPY: Illegal wild-card usage in destination name\n",
						stderr);
				exit (1);
			}
			dstnamewild = TRUE;
		}
		if (dstext[0] == '*')			// Wild card destination extension?
		{
			if (dstext[1] != 0)			// Yes - is it valid?
			{							// No
				fputs("? COPY: Illegal wild-card usage in destination "
						"extension\n", stderr);
				exit (1);
			}
			dstextwild = TRUE;
		}
   	}
	srcname = pnt = srcspec->spec;		// Find start of source name
	while ((chr = *pnt++) != 0)
	{
		if (chr == ':' || chr == '\\' || chr == '/')
			srcname = pnt;
	}

	if ((srcname == (srcspec->spec + 4) || srcname[-5] == '\\' ||
			srcname[-5] == ':') && strncmp(srcname - 4, "...", 3) == 0)
	{
		strmov(srcname - 4, srcname);
		srcname -= 4;
		recur |= SRCRECUR;
	}
	if (srcwild && !(dstnamewild || dstextwild))
	{
		fputs("? COPY: Source wild-card specified without matching "
				"destination wild-card\n", stderr);
		exit (1);
	}
	if ((recur & DSTRECUR) && (recur & SRCRECUR) == 0)
	{
		fputs("? COPY: Destination recursion specified without matching "
				"source recursion\n", stderr);
		exit (1);
	}
	if (recur)
	{
		// Find the canonical form of the path which is needed when
		//   constructing destination specfications.

		pathparm.filspec.buffer = strbuf;
		if ((rtn = svcIoDevParm(XO_ODFS, srcspec->spec, &pathparm)) < 0)
			femsg2(prgname, "Cannot open source directory for search", rtn,
					NULL);

///		printf("### RECUR: %d |%s|\n", pathparm.filspec.strlen,
///				pathparm.filspec.buffer);

		recurstr = getmem(pathparm.filspec.strlen + 1);
		recurlen = strmov(recurstr, strbuf) - recurstr;
	}

   	copybufr = getmem(copysize);		// Allocate our data buffer

   	srcparms.filoptn.value = XFO_FILE;
   	if (verbose)
		srcparms.filoptn.value |= ((gcp_dosdrive) ?
				XFO_DOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH :
				XFO_XOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH);
    dstparms.filoptn.value = (gcp_dosdrive) ?
			XFO_DOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH|XFO_FILE :
			XFO_XOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH|XFO_FILE;

    if (rate)							// Get current time if need to display
										//   the transfer rate
		svcSysDateTime(T_GTHRDTTM, &starttime);

	// Here when ready to do the copy - There are three cases:
	//   1) Simple src to dst copy
	//   2) src to dst copy with wild-cards and/or recursion
	//   3) Concatenation

	if (doconcat)
	{
		// Here if concatenating files

		thisspec = srcspec;
		do
		{
			copyfile(srcspec->spec, FALSE);
		} while ((thisspec = thisspec->next) != NULL);
	}
	else
	{
		if (recur || srcwild)
		{
			dirscan(srcspec->spec, &dsd, recur);
			dsd.pathnamelen = 0;
			dodirectory();
		}
		else
			copyfile(srcspec->spec, TRUE);
	}
	finish();							// Display final summary if need to
}


//**************************************************
// Function: finish - Display final summary and exit
// Returend: Never returns
//**************************************************

void finish(void)
{
	char *pnt;
	char  text[1024];

    if (dstdev > 0)						// Make sure the output file is closed
		closeoutfile(TRUE);
	pnt = text;
    if (totalbytes | totalfiles | rate)
    {
		if (rate)
			svcSysDateTime(T_GTHRDTTM, &stoptime);
		if (totalfiles | totalbytes)
		{
			if (dircnt > 0)
				pnt += sprintf(pnt, "    %,d director%s created\n", dircnt,
						(dircnt != 1) ? "ies" : "y");
			pnt += sprintf(pnt, "    %,d file%s copied", filecnt,
					 (filecnt != 1) ? "s" : "");
		}
		if (totalbytes)
			pnt += sprintf(pnt, ", %,d byte%s ", (long)bytecnt, (bytecnt != 1) ?
					"s" : "");
		if (rate)
		{
			sdtsub(&xfertime, &stoptime, &starttime);
			if (bytecnt != 0 && xfertime.dt != 0)
				result = (bytecnt * ST_SECOND) / xfertime.dt;
			else
				result = 0;
			if (result != 0)
				pnt += sprintf(pnt, "(%,u byte%s/sec)", (long)result,
						(result != 1) ? "s" : "");
		}
		*pnt++ = '\n';
		*pnt = 0;
		fputs(text, stdout);
	}
	exit(0);							// Return with no error
}


//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

int procfile(void)
{
	char *pnt;
	long  len;

    // We build a list of files to copy in each directory.  When we get a
    //   different directry, we sort (if necessary) the list and copy the
    //   files.  Since we have to open each file to copy it, all we need
    //   in our list here is the file specification.

///	printf("### PROCFILE: %d |%s| %d |%s|\n", dsd.pathnamelen, dsd.pathname,
///			dsd.filenamelen, dsd.filename);

	if (dsd.filenamelen < 0)
	{
		if (!dodirectory())				// If have a new directory
			return (FALSE);
		pnt = strmov(dirpath, dsd.pathname);
		if (pnt[-1] != '\\')
			*(ushort *)pnt = '\\';
		dirlevel = dsd.dirlvl;
	}
	else
	{
		// Here if have a file

	    len = dsd.pathnamelen + dsd.filenamelen;
	    thisfile = getmem(sizeof(FD) + len);
    	thisfile->dironly = (dsd.filenamelen == 0);
    	strmov(thisfile->spec, dsd.filename);
    	if (dsd.error < 0)					// Error reported by DIRSCAN?
			fileerror(FALSE, dsd.error, "Error searching directory for",
					thisfile->spec, NULL, FALSE);
		thisfile->level = dsd.dirlvl;
		if (firstfile == NULL)
			firstfile = thisfile;
		else
			lastfile->nxtfile = thisfile;
		lastfile = thisfile;
		thisfile->nxtfile = NULL;
	}
	return (TRUE);
}


int dodirectory(void)
{
	FD   *next;
	long  rtn;
	char  srcstr[512];
	char  dststr[512];

	if (recur && strnicmp(dsd.pathname, recurstr, recurlen) != 0)
	{
		fprintf(stderr, "? COPY: Internal error: Invalid path on recursion\n"
				"          Returned  |%.*s|\n          Should be |.*%s|\n",
				recurlen, recurstr, recurlen, dsd.pathname);
		return (FALSE);
	}
	if (dirpath[0] != 0)				// Do we have previous directory?
	{
		if (firstfile != NULL)			// Yes - have any files to copy in it?
		{
			if (dosort && firstfile->nxtfile != NULL)
										// Yes - should we sort the list?
				firstfile = heapsort(firstfile,	// Yes
						(int (*)(void *a, void *b, void *d))comp, NULL);
			do
			{
				// Construct the source file spec

///				printf("### in dodirectory: dirpath: |%s| spec: |%s|\n",
///						dirpath, firstfile->spec);

				strmov(strmov(srcstr, dirpath), firstfile->spec);

///				printf("### FILE: %s\n", srcstr);

				copyfile(srcstr, FALSE);

				next = firstfile->nxtfile;
				free(firstfile);
			} while((firstfile = next) != NULL);
		}
		else
		{
///			printf("### have empty directory %s (crempty = %d)\n", dirpath,
///					crempty);
///			printf("### dstspec: %s\n", dstspec->spec);
///			printf("### dstname: %s\n", dstname);
///			printf("### recurstr = %d |%s|\n", recurlen, recurstr);

			// Here if we have an empty directory

			if (recur && crempty)		// Should we create it?
			{
				strmov(strmov(dststr, dstspec->spec), dirpath + recurlen);

///				printf("### CREATE: |%s|\n", dststr);

				if ((rtn = makedir(dststr, dirlevel)) < 0 && rtn != ER_FILEX &&
							rtn != ER_DIREX)
					fileerror(FALSE, rtn, "Error creating directory", dststr,
							NULL, FALSE);
			}
		}
	}
	return (TRUE);
}


//****************************************
// Function: copyfile - Copy a single file
// Returned: Nothing
//****************************************

// All source wild cards have been resolved before this function is called.
//   Destination wild cards are resolved here.

void copyfile(
    char *srcstr,		// Source string
    int   last)			// TRUE if this is the last file to copy
{
	char *snbgn;
	char *snend;
	char *sxbgn;
	char *dpnt;
	char *spnt;
	char *dname;
	char *fpnt;
    long  rtn;
    long  bits;
    long  amount;
    char  chr;
    char  dst[512];

///	printf("### in copyfile: src: |%s| dstspec: |%s|\n", srcstr, dstspec->spec);

    if (select)
    {
		sprintf(copybufr, "Copy file %s", srcstr);
		switch(askuser(copybufr, TRUE))
		{
		 case 0:						// No
			svcIoClose(srcdev, 0);
			srcdev = -1;
			return;

		 case 2:						// All
			select = FALSE;
		 case 1:						// Yes
			break;
		}
	}

	// Construct the destination specification

	dname = (recur & DSTRECUR) ? strmov(strnmov(dst, dstspec->spec, dstname -
			dstspec->spec - 1), dirpath + recurlen + 1) :
			strnmov(dst, dstspec->spec, dstname - dstspec->spec);
	if (recur || dstnamewild || dstextwild) // Need to construct the destination
	{										//   name?
		snbgn = spnt = srcstr;			// Yes - find start of source name
		while ((chr = *spnt) != 0)
		{
			spnt++;
			if (chr == ':' || chr == '\\')
				snbgn = spnt;
		}
		sxbgn = snend = spnt;			// Find start of source extension
		spnt = snbgn;
		while ((chr = *spnt++) != 0)
		{
			if (chr == '.')
			{
				sxbgn = spnt;
				snend = spnt - 1;
			}
		}
///		printf("### SN: |%.*s| SX: |%s|\n", snend - snbgn, snbgn, sxbgn);
///		printf("### DN: |%.*s| DX: |%s|\n", dstext - dstname, dstname, dstext);

		dpnt = dstname;
		fpnt = dname;
		spnt = snbgn;
		if (dpnt[0] == '*')				// Wildcard destination name?
		{								// Yes - copy source name
			while (spnt < snend)
				*fpnt++ = *spnt++;
			if (*dpnt++ != 0)			// Advance to destination extension
				dpnt++;
		}
		else							// Not wildcard destination name
		{								// Copy given destination name
			while ((chr = *dpnt) != 0)
			{
				dpnt++;
				if (chr == '.')
					break;
				*fpnt++ = chr;
			}
			while ((chr = *spnt) != 0)	// Advance to source extension
			{
				spnt++;
				if (chr != '.')
					break;
			}
		}
		if (dpnt[0] == '*')				// Wildcard destination extension?
		{								// Yes - copy source extension if
			if (sxbgn[0] != 0)			//   there is one
			{
				*fpnt++ = '.';
				spnt = sxbgn;
				while ((chr = *spnt) != 0)
				{
					spnt++;
					if (chr == '.')
						break;
					*fpnt++ = chr;
				}
			}
		}
		else
		{					
			if (*dpnt != 0)
			{
				*fpnt++ = '.';
				while ((chr = *dpnt) != 0)
				{
					dpnt++;
					if (chr == '.')
						break;
					*fpnt++ = chr;
				}
			}
		}
		*fpnt = 0;
	}
	else								// If can use the destination name
		strmov(dname, dstname);			//   as is.

///	printf("### DNAME: |%s|\n", dname);

	// Open the source file

	bits = XO_IN;
	if (image)
	{
		amount = strlen(srcstr) - 1;
		if (amount < 0 || srcstr[amount] == '\\' ||
				srcstr[amount] == '/' || srcstr[amount] == ':')
			bits = XO_IN|XO_PHYS;
	}

///	printf("### SRC: |%s|\n", srcstr);

    if ((srcdev = svcIoOpen(bits, srcstr, &srcparms)) < 0)
    {
		fileerror(FALSE, srcdev, "Error opening source file", srcstr,
				(dstdev > 0) ? dst : NULL, last);
		return;
    }

	// Open the destination if need to

    if (dstdev < 0)						// Need to open the destination?
    {
		dstparms.cdate.value = srcparms.cdate.value;
		dstparms.mdate.value = srcparms.mdate.value;
		bits = XO_IN|XO_OUT|XO_CREATE|XO_TRUNCW|XO_EXWRITE|XO_EXREAD;
		if (image)
		{
			amount = strlen(dst) - 1;
			if (amount < 0 || dst[amount] == '\\' ||
					dst[amount] == '/' || dst[amount] == ':')
				bits = XO_OUT|XO_PHYS;
		}
		if (!overwrite && (bits & XO_PHYS) == 0)
			bits |= XO_FAILEX;

///		printf("### DST: |%s|\n", dst);

		if ((dstdev = svcIoOpen(bits, dst, &dstparms)) < 0)
		{
			if (recur && dstdev == ER_DIRNF && thisfile->level > 0)
			{
				if ((rtn = makedir(dst, thisfile->level)) < 0)
				{
					fileerror(FALSE, rtn, "Error creating directory for file",
							dst, NULL, last);
					return;
				}
				dstdev = svcIoOpen(bits, dst, (char *)&dstparms);
			}
			if (dstdev == ER_FILEX)
			{
				if (!overwrite)
				{
					sprintf(copybufr, "Overwrite %s", dst);
					switch (askuser(copybufr, TRUE))
					{
					 case 0:			// No
						return;

					 case 2:			// All
						overwrite = TRUE;
						bits &= ~XO_FAILEX;
					 case 1:			// Yes
						dstdev = svcIoOpen(bits & ~XO_FAILEX, dst, &dstparms);
						break;
					}
				}
			}
			if (dstdev < 0)
			{
				fileerror(FALSE, dstdev, "Error creating destination file",
						dst, NULL, last);
				return;
			}
		}
		if (isterm && isctrm(dstdev))
			isterm = FALSE;
    }
    filecnt++;

	// Copy the file

    if (!quiet)
	{
		if (verbose)
		{
		    spin = 0;
			rtn = sprintf(copybufr, "  %s %c> %s%s", srcparms.filspec.buffer,
					(doconcat) ? '+' : '-', dstparms.filspec.buffer,
					(isterm) ? " |" : "\n");
			fputs(copybufr, stdtrm);
			stslstchr = copybufr[rtn - 3];
		}
		else
			printf("  %s\n", srcparms.filspec.buffer);
	}
    while ((amount = svcIoInBlock(srcdev, copybufr, copysize)) > 0)
    {
		if ((rtn = svcIoOutBlock(dstdev, copybufr, amount)) != amount)
		{
			if (rtn >= 0)
				rtn = ER_ICMIO;
			fileerror(TRUE, rtn, "Error writing destination file",
					(char *)(dstparms.filspec.buffer),
					(char *)(dstparms.filspec.buffer), last);
			return;
		}
		bytecnt += amount;
		if (isterm && verbose && !quiet)
		{
			fputs(spintbl[spin], stdout);	
			spin = (spin + 1) & 0x03;
		}
	}
	if (amount != 0 && amount != ER_EOF)
    {
		fileerror(TRUE, amount, "Error reading source file",
				(char *)(srcparms.filspec.buffer),
				(char *)(dstparms.filspec.buffer), last);
		return;
	}
	if (!doconcat)
		closeoutfile(last);
	svcIoClose(srcdev, 0);
    srcdev = -1;

	// NOTE: The funny stuff with stslstchr is intended to make sure the
	//       extra LF is suppressed if the line is exactly the same as the
	//       display width!

    if (isterm && verbose && !quiet)
		printf("\b \b\b\b%c\n", stslstchr);
}


//************************************************
// Function: comp - Compare two filenames for sort
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//************************************************

int comp(
    FD *one,
    FD *two)
{
	return (stricmp(one->spec, two->spec)); // Always sort on name
}


//************************************************************
// Function: findname - Find name part of a file specification
// Returend: Pointer to first character of name
//************************************************************

char *findname(
    char *bgn)
{
    char *end;
    char  chr;

    end = bgn;
    while ((chr = *bgn++) != 0)
    {
		if (chr == ':' || chr == '\\' || chr == '/')
			end = bgn;
    }
    return (end);
}


//*******************************************
// Function: makedir - Create directory
// Returned: 0 if OK, negative error if error
//*******************************************

long makedir(
    char *spec,
    long  level)
{
    char *src;
    char *dst;
    char *end;
    long  rtn;
    char  chr;
    char  bufr[512];

	static struct
	{	BYTE4PARM  filoptn;
		LNGSTRPARM filspec;
		char       end;
    } dirparms =
	{	{PAR_SET|REP_HEXV, 4 ,IOPAR_FILEOPTN},
		{PAR_GET|REP_STR , 0 ,IOPAR_FILESPEC ,NULL, 512, 512},
	};

///	printf("### MAKEDIR: |%s|\n", spec);

    dirparms.filspec.buffer = bufr;
    dirparms.filoptn.value = (gcp_dosdrive) ?
			XFO_DOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH|XFO_FILE :
			XFO_XOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH|XFO_FILE;
    if ((rtn = svcIoDevParm(XO_CREATE|XO_ODFS, spec, (char *)&dirparms)) < 0)
    {
		if (rtn == ER_DIRNF && --level > 0)
		{
			src = spec;
			dst = end = bufr;
			while ((chr = *src++) != 0)
			{
				*dst = chr;
				if (chr == '\\' || chr == '/')
					end = dst;
				dst++;
			}
			*end = 0;
			if ((rtn = makedir(bufr, level)) < 0)
				return (rtn);
			dirparms.filspec.buffer = bufr;
			dirparms.filoptn.value = (gcp_dosdrive) ?
					XFO_DOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH|
						XFO_FILE :
					XFO_XOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH|
						XFO_FILE;
			if ((rtn = svcIoDevParm(XO_CREATE|XO_ODFS, spec,
					(char *)&dirparms)) < 0)
				return (rtn);
		}
		else
			return (rtn);
	}
	dircnt++;
	printf("Directory %s\\ created\n", bufr);
	return (0);
}


//***************************************************
// Function: closeoutfile - Close the output file
// Returned: Nothing (does not return if final error)
//***************************************************

void closeoutfile(
    int last)			// TRUE if this is the last file to copy
{
    long rtn;

    if ((rtn = svcIoClose(dstdev, 0)) < 0)
    {
		dstdev = -1;
		fileerror(TRUE, rtn, "Error closing destination file", dstspec->spec,
				dstspec->spec, last);
		return;
    }
    dstdev = -1;
}


//****************************************
// Function: askuser - Ask user what to do
// Returned: 0 if No, 1 if Yes, 2 if All
//****************************************

int askuser(
    char *msg,
    int   quit)
{
    long rtn;
    char bufr[] = {0, '\b', 0};

    static struct
    {	BYTE4PARM  oldtim;
		BYTE4PARM  clrtim;
		BYTE4PARM  settim;
	char       end;
    } trmparm1 =
    {	{PAR_GET|REP_HEXV, 4, IOPAR_TRMSINPMODE, 0},
		{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
		{PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, TIM_IMAGE}
    };

    static struct
    {	BYTE4PARM  clrtim;
		BYTE4PARM  settim;
		char       end;
    } trmparm2 =
    {	{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
		{PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, 0}
    };

    time_s entertime;
    time_s leavetime;
    time_s difftime;

    svcSysDateTime(T_GTHRDTTM, &entertime);
    svcIoInBlockP(DH_STDIN, NULL, 0, (char *)&trmparm1);
    fprintf(stderr, "%s (Yes/No/All%s)? ", msg, (quit) ? "/Quit" : "");
    svcTrmFunction(DH_STDIN, TF_CLRINP);
    rtn = -1;
    for (;;)
    {
		svcIoInBlock(DH_STDIN, bufr, 1);
		bufr[0] = toupper(bufr[0]);
		if (bufr[0] == 'C' - 0x40)
		{
			fputs("^C\n", stderr);
			exit(0);
		}
		else if (bufr[0] == '\r' && rtn >= 0)
			break;
		else if (bufr[0] == 'N')
		{
			rtn = 0;
			goto echo;
		}
		else if (bufr[0] == 'Y')
		{
			rtn = 1;
			goto echo;
		}
		else if (bufr[0] == 'A')
		{
			rtn = 2;
			goto echo;
		}
		else if (!quit)
			continue;
		else if (bufr[0] == 'Q')
		{
			rtn = 3;
			goto echo;
		}
		continue;

		echo: fputs(bufr, stderr);
    }
    fputs("\n", stderr);
    trmparm2.settim.value = trmparm1.oldtim.value;
    svcIoInBlockP(DH_STDIN, NULL, 0, (char *)&trmparm2);
    svcTrmFunction(DH_STDIN, TF_CLRINP);
    svcSysDateTime(T_GTHRDTTM, &leavetime);
    sdtsub(&difftime, &leavetime, &entertime);
    sdtadd(&starttime, &starttime, &difftime);
    if (rtn == 3)
		finish();
    return (rtn);
}


//*************************************************
// Function: optbuffer - Process BUFFER size option
// Returned: TRUE
//*************************************************

int optbuffer(
    arg_data *arg)
{
    copysize = ((arg->val.n + 511) / 512) * 512;
    if (copysize < 1024)
    {
        fprintf(stderr, "\n? %s: Buffer size (%d) is too small\n",
		pib.prgname, copysize);
        exit(EXIT_MALLOC);
    }
    return (TRUE);
}


//*****************************************
// Function: optbegin- Process BEGIN option
// Returned: TRUE
//*****************************************

int optbegin(
    arg_data *arg)
{
    begindir = getmem(arg->length + 2);
    strmov(begindir, arg->val.s);
    return (TRUE);
}


//*******************************************
// Function: opttotals - Proces TOTALS option
// Returned: TRUE
//*******************************************

int opttotals(
     arg_data *arg)
{
    if ((arg->flags & ADF_XSVAL) == 0)
		return (TRUE);

    switch ((int)arg->val.n)
    {
     case 0:							// Files
		totalfiles = TRUE;
		break;

     case 1:							// Nofiles
		totalfiles = FALSE;
		break;

     case 2:							// Bytes
		totalbytes = TRUE;
		break;

     case 3:							// Nobytes
		totalbytes = FALSE;
		break;

     case 4:							// Rate
		rate = TRUE;
		break;

     case 5:							// Norate
		rate = FALSE;
		break;
    }
    return (TRUE);
}


//***********************************************
// Function: optnototals - Proces NOTOTALS option
// Returned: TRUE
//***********************************************

int optnototals(
     arg_data *arg)
{
    arg = arg;

    totalbytes = totalfiles = rate = FALSE;
    return (TRUE);
}


//******************************************
// Function: opterror - Process ERROR option
// Returned: TRUE
//******************************************

int opterror(
    arg_data *arg)
{
    if ((arg->flags & ADF_XSVAL) == 0)
		return (TRUE);

    switch ((int)arg->val.n)
    {
     case 0:						// Stop
		erraction = 0;
		break;

     case 1:						// Ask
		erraction = -1;
		break;

     case 2:						// Continue
		erraction = 1;
		break;
    }
    return (TRUE);
}


//********************************************
// Function: nonopt - process non-option input
// Returned: Nothing
//********************************************

int nonopt(
	char *arg)
{
	FS *spec;
	int size;

	size = strlen(arg);					// Get length of argument
	if (size == 1 && arg[0] == '+')		// Is it +
	{									// Yes - this means concatanation
		if (concatnxt || srcspec == NULL || dstspec != NULL)
		{
			fprintf(stderr, "? %s: Illegal syntax for concatanation\n",
					prgname);
			exit(EXIT_INVSWT);
		}
		doconcat = TRUE;
		concatnxt = TRUE;
		return (TRUE);
	}
	spec = getmem(size + sizeof(FS));
	strmov(spec->spec, arg);
	spec->next = NULL;
	if (srcspec == NULL)
		srcspec = spec;
	else
	{
		if (dstspec != NULL)
		{
			fprintf(stderr, "? %s: Too many files specified\n", prgname);
			exit(EXIT_INVSWT);
		}
		if	(concatnxt)
		{
			concatnxt = FALSE;
			thisspec->next = spec;
		}
		else
			dstspec = spec;
	}
	thisspec = spec;
	return (TRUE);
}


//**********************************************************
// Function: fileerror - Report errors associated with files
// Returned: Nothing, only returns if should continue
//**********************************************************

void fileerror(
    int   nl,			// TRUE if need initial NL
    long  errcode,		// XOS error code
    char *text,
    char *name,			// Name of file associated with error
    char *delspec,		// File spec for file to delete
    int   last)			// TRUE if this is the last file to copy
{
    int  action;
    char msg[100];

    // First display the error message

    svcSysErrMsg(errcode, 0x03, msg);
    fprintf(stderr, "%s? %s: %s %s\n        %s\n", (nl) ? "\n" : "", prgname,
			text, name, msg);

    // Close the source file if its open

    if (srcdev > 0)
    {
		svcIoClose(srcdev, 0);
		srcdev = -1;
    }

    // Then determine if we should see about continueing

    if (erraction != 0)
    {
		action = 0;
		if ((srcwild || recur) && !last)
			action = (erraction < 0) ? askuser("% COPY: Continue copying",
					FALSE) : erraction;
		if (action == 2)
			erraction = 1;
		if (action != 0)
		{
			if (!doconcat)
				errordel(delspec, last);
			return;
		}
    }
    errordel(delspec, last);
    finish();
}


//*****************************************************************
// Function: errordel - Delete output file after error if necessary
// Returned: Nothing
//*****************************************************************

void errordel(
    char *delspec,
    int   last)
{
    long rtn;
	char msg[100];

    if (!deloutfile || delspec == NULL)	// Want to delete the output file?
		return;							// No
    if (dstdev > 0)						// Yes - make sure its closed
		closeoutfile(last);
    if ((rtn = svcIoDelete(0, delspec, NULL)) < 0) // Delete the file
    {									// If error
		svcSysErrMsg(rtn, 0x03, msg);	// Tell him about it
		fprintf(stderr, "? %s: Error deleting partial output file\n"
				"        %s; %s\n", prgname, msg, delspec);
    }
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
        fprintf(stderr, "? %s: Not enough memory available\n", prgname);
        exit(EXIT_MALLOC);
    }
    return (ptr);
}


//*************************************************************
// Function: showerrmsg - Display XOS error message for dirscan
// Returned: Never returns
//*************************************************************

void showerrmsg(
    const char *arg,
    long  code)
{
    char buffer[100];			// Buffer to receive error message

    if (code != 0)
    {
		svcSysErrMsg(code, 3, buffer);	// Get error message
		fprintf(stderr, "\n? %s: %s%s%s\n", prgname, buffer, (arg[0] != '\0') ?
				"; ": "", arg);			// Output error message
    }
    else
		fprintf(stderr, "\n? %s: %s\n", prgname, arg);
    exit(1);
}
