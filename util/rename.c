//----------------------------
// RENAME.C
// File rename utility for XOS
//
// Written by John Goltz
//
//----------------------------

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

// This program is a complete rewrite of the XOS rename utility.

// 4.0 24-Mar-99 First version after rewrite (derived from new COPY.C)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <heapsort.h>
#include <xoserrmsg.h>
#include <xossvc.h>
#include <xostime.h>
#include <progarg.h>
#include <global.h>
#include <xosstr.h>

typedef struct fs FS;
struct  fs
{   FS  *next;
    char name[4];
};

typedef struct fd FD;
struct  fd
{   FD  *nxtfile;
    FD  *nxtdir;
    long level;
    char name[1];
};

int version = 4;
int editno  = 0;

FS    dfltspec = {NULL, "*.*"};
FS   *srcspec;
FS   *thisspec;
FS   *dstspec;

FD   *firstdir;
FD   *lastdir;
FD   *firstfile;
FD   *lastfile;
FD   *thisfile;

ui8   srcwild;			// TRUE if have source wild-card character
ui8   dstnamewild;		// TRUE if wild-card in destination name
ui8   dstextwild;		// TRUE if wild-card in destination extension

si32  filecnt;

si32  fileopts;

char *srcname;
char *srcext;
char *dstname;
char *dstext;

char *srcbufr;

struct
{	BYTE4PARM  filoptn;
	LNGSTRPARM filspec;
	BYTE1PARM  srcattr;
	char       end;
} renparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN},
    {PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, 512, 512},
    {PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR , XA_DIRECT|XA_FILE}
};

char   *prgname = "RENAME";	// Our programe name
char   *envname = "^XOS^RENAME^OPT"; // The environment option name
char   *example = "{{/}{-}option{=sub}} source destination";
char   *description = "This program will rename one or more files. Source "
		"file names may contain general wild card characters. Destination "
		"file names may contain * as the entire name and/or entire extension "
		"only. Recusive directory searches (ellipsis notation) are not "
		"supported.";

struct
{   BYTE4PARM  filoptn;
	LNGSTRPARM filspec;
	BYTE2PARM  srcattr;
	BYTE2PARM  filattr;
	char       end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILEOPTN, XFO_VOLUME|XFO_NODENUM|XFO_NODENAME|
            XFO_RVOLUME|XFO_PATH|XFO_FILE},
    {PAR_GET|REP_STR , 0 , IOPAR_FILESPEC, NULL},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR, XA_FILE},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILEATTR},
};

int    select = FALSE;
uchar  dosort = TRUE;
int    quiet = FALSE;
int    verbose = TRUE;
int    overwrite = FALSE;
int    erraction = -1;

uchar  totalfiles = FALSE;

extern int _malloc_amount;
int    maxmem;

PROGINFO pib;

// Function prototypes

int   askuser(char *msg, int quit);
void  baddwc(void);
int   comp(FD *one, FD *two);
int   dorename(long opts, char *src, char *dst);
void  showerrmsg(const char *arg, long  code);
void  fileerror(int nl, long errcode, char *text, char *name, int last);
char *findname(char *bgn);
void  finish(void);
void *getmem(size_t size);
int   nonopt(char *arg);
int   opterror(arg_data *);
void  opthelp(void);
int   optnodir(void);
int   optnototals(arg_data *);
int   opttotals(arg_data *);
void  renamefile(char *srcstr, char *dststr, int last);

// progarg data

SUBOPTS totalscmd[] =
{   {"F*ILES"  , "Display total number of files renamed"},
    {"NOF*ILES", "Don't display total number of files renamed"},
    {NULL}
};

SUBOPTS errorcmd[] =
{   {"Q*UIT"    , "Quit renaming after error"},
    {"A*SK"     , "Ask user for instructions after error"},
    {"C*ONTINUE", "Continue renaming after error"},
    {NULL}
};

arg_spec options[] =
{
//  {"BE*FORE"  , ASF_VALREQ|ASF_LSVAL, NULL     ,    optbefore     , 0    , "Transfer all matching files before this Date/Time"},
//  {"SI*NCE"   , ASF_VALREQ|ASF_LSVAL, NULL     ,    optsince      , 0    , "Transfer all matching files since this Date/Time"},
    {"S*ELECT"  , ASF_BOOL|ASF_STORE  , NULL     , AF(&select)      , TRUE , "Select which files to rename"},
    {"O*VER"    , ASF_BOOL|ASF_STORE  , NULL     , AF(&overwrite)   , FALSE, "Replace existing files without confirmation"},
    {"ERR*OR"   , ASF_XSVAL           , errorcmd ,   opterror       , 0    , "Specify action to take on error"},
    {"DOS*DRIVE", ASF_BOOL|ASF_STORE  , NULL     , AF(&gcp_dosdrive), TRUE , "Override the global DOSDRIVE setting"},
    {"Q*UIET"   , ASF_BOOL|ASF_STORE  , NULL     , AF(&quiet)       , TRUE , "Don't display any status"},
    {"V*ERBOSE" , ASF_BOOL|ASF_STORE  , NULL     , AF(&verbose)     , TRUE , "Display full status"},
    {"SO*RT"    , ASF_BOOL|ASF_STORE  , NULL     , AF(&dosort)      , TRUE , "Sort file list before renaming"},
    {"T*OTALS"  , ASF_XSVAL           , totalscmd,    opttotals     , 0    , "Display summary when complete"},
    {"NOT*OTALS", 0                   , NULL     ,    optnototals   , 0    , "Don't display summary"},
    {"H*ELP"    , 0                   , NULL     , AF(opthelp)      , 0    , "Display this message"},
    {"?"        , 0                   , NULL     , AF(opthelp)      , 0    , "Display this message"},
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
    char  chr;
    char  srcrtn[512];
    char  dstrtn[512];

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

    srcbufr = srcrtn;
    renparms.filspec.buffer = dstrtn;

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    // Check Local Configuration Parameters

    if (svcSysGetEnv(envname, (char const **)&envpnt[0]) > 0)
    {
		envpnt[1] = NULL;
        progarg(envpnt, PAF_EATQUOTE, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
////    isterm = fisctrm(stdout);

    // Process the command line

	if (argc >= 2)
	{
		++argv;
        progarg(argv, PAF_EATQUOTE, options, NULL, nonopt,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
	}
	if (overwrite == 0)
		overwrite = XO_FAILEX;

	// Determine if we have any wild-card characters in any source specs.
	//   (This is a very simple check.  If there are wild-card characters in
	//   the device name or directory fields, we will see them here.  In
	//   this case, we just let dirscan complain about the illegal wild-card
	//   usage!)

	// Destination wild cards are allowed only to indicate complete fields.
	//   That is, they can specify the entire name and/or extension.  When a
	//   destination wild card is specified in a field, no other character
	//   can appear in that field.

	if (srcspec == NULL)
	{
		fputs("? RENAME: No files specified, type RENAME /H for help\n",
				stderr);
		exit(1);
	}
	pnt = srcname = findname(srcspec->name); // Find start of source name
	while ((chr = *pnt++) != 0)
	{
		if (chr == '.')
			srcext = pnt;
		if (chr == '*' || chr == '?')
			srcwild = TRUE;
	}
	if (dstspec == NULL)
    {
		fputs("? RENAME: No destination specified, type RENAME /H for help\n",
				stderr);
		exit(1);
    }
	pnt = dstname = findname(dstspec->name); // Find start of destination name
	dstext = NULL;
	if (pnt[0] == '*')					// Have wild card name?
	{
		if (pnt[1] != '.' && pnt[1] != 0)
			baddwc();
		dstnamewild = TRUE;
		pnt += 2;
		if (pnt[-1] == '.')
			dstext = pnt;
	}
	else
	{
		while ((chr = *pnt++) != 0 && chr != '.')
		{
			if (chr == '*' || chr == '?')
				baddwc();
		}
		if (chr == '.')
			dstext = pnt;
	}
	if (dstext != NULL)
	{
		if (pnt[0] == '*' && pnt[1] == 0)
			dstextwild = TRUE;
		else
		{
			while ((chr = *pnt++) != 0)
				if (chr == '*' || chr == '?')
					baddwc();
		}
	}

	// CODE GOES HERE TO SET UP FOR DESTINATION NAME REPLACEMENT

	fileopts = (gcp_dosdrive) ?
			XFO_DOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH|XFO_FILE :
			XFO_XOSDEV|XFO_NODENUM|XFO_NODENAME|XFO_RVOLUME|XFO_PATH|XFO_FILE;
	if (srcwild != NULL)				// Did we find a source wild-card?
	{									// Yes


		// DO DIRECTORY SEARCH HERE

	}
	else								// No wild cards - just rename the
										//   single file
		renamefile(srcspec->name, dstspec->name, TRUE);

	// Display final summary if need to

	finish();
}


//**************************************************
// Function: finish - Display final summary and exit
// Returend: Never returns
//**************************************************

void finish(void)
{
    if (totalfiles)
		printf("\t%d file%s renamed\n", filecnt, (filecnt != 1) ? "s" : "");
    exit(0);				// Return with no error
}


//********************************************
// Function: renamefile - Rename a single file
// Returned: Nothing
//********************************************

// All source wild cards have been resolved before this function is called.
//   Destination wild cards are resolved here.

void renamefile(
    char *srcstr,		// Source string
    char *dststr,		// Destination string
    int   last)			// TRUE if this is the last file to rename
{
    char *srcext;
    char *dstext;
    char *pnt;
    long  rtn;
    char  chr;
    char  dst[512];

    if (select)
    {
		sprintf(dst, "Rename file %s", srcstr);
		switch(askuser(dst, TRUE))
		{
		 case 0:						// No
			return;

		 case 2:						// All
			select = FALSE;
		 case 1:						// Yes
			break;
		}
    }
    if (dstnamewild | dstextwild)		// Have a destination wild card?
    {
		pnt = srcname;
		srcext = NULL;
		while ((chr = *pnt) != 0)		// Find start of source extension
		{
			pnt++;
			if (chr == '.')
				srcext = pnt;
		}
		if (srcext == NULL)				// Null extension?
			srcext = pnt;				// Yes - point to end of string
		pnt = dststr;					// Find start of destination extension
		dstext = NULL;
		while ((chr = *pnt) != 0)
		{
			pnt++;
			if (chr == '.')
				dstext = pnt;
		}
		if (dstext == NULL)				// Null extension?
			dstext = pnt;				// Yes - point to end of string
		pnt = (dstnamewild) ? strnmov(dstname, srcname, srcext - srcname) :
				strnmov(dstname, dststr, dstext - dststr);
		strmov(pnt, (dstextwild) ? srcext : dstext);
		rtn = dorename(overwrite, srcstr, dst);
	}
	else
		rtn = dorename(overwrite, srcstr, dststr);
	if (rtn == ER_FILEX)
	{
		INT3;

///		sprintf(srcbufr, "Replace %s",
///				(char *)(renparms.filspec.buffer));

		switch (askuser(srcbufr, TRUE))
		{
		 case 0:				// No
			return;

		 case 2:				// All
			overwrite = TRUE;
		 case 1:				// Yes
			break;
		}
		rtn = dorename(0, srcstr, dst);
	}
	if (rtn < 0)
		fileerror(FALSE, rtn, "Error renaming file", srcstr, last);
}


//****************************************************
// Function: dorename - Rename file for MOVE command
// Returned: 0 if normal, negative error code if error
//****************************************************

int dorename(
	long  opts,
    char *src,
    char *dst)
{
	char *npnt;
	long  rtn;

	renparms.filoptn.value = fileopts | opts;
    if ((rtn = svcIoRename(0, src, dst, (char *)&renparms)) >= 0)
    {
		filecnt++;
		if (!quiet)
		{
			strtok(renparms.filspec.buffer, "\xFF");
			if (verbose)
			{
				if ((npnt = strtok(NULL, "\xFF")) != NULL)
				{
					printf("  %s => %s\n", renparms.filspec.buffer, npnt);
					return (rtn);
				}
			}
			printf("  %s\n", renparms.filspec.buffer);

		}
	}
	return (rtn);
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
    return (stricmp(one->name, two->name)); // Always sort on name
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
	{	{PAR_GET|REP_HEXV, 4, IOPAR_TRMSINPMODE},
		{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
		{PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, TIM_IMAGE}
	};

	static struct
	{	BYTE4PARM  clrtim;
		BYTE4PARM  settim;
		char       end;
	} trmparm2 =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
		{PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE}
	};

    svcIoInBlockP(DH_STDIN, NULL, 0, (char *)&trmparm1);
    fprintf(stderr, "%s (Yes/No/All%s)? ", msg, (quit) ? "/Quit" : "");
    svcTrmFunction(DH_STDIN, TF_CLRINP);
    rtn = -1;
    for (;;)
	{
		svcIoInBlock(DH_STDIN, bufr, 1);
		bufr[0] = toupper(bufr[0]);
		if (bufr[0] == '\r' && rtn >= 0)
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
    if (rtn == 3)
		finish();
    return (rtn);
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
	 case 0:				// Files
		totalfiles = TRUE;
		break;

     case 1:				// Nofiles
		totalfiles = FALSE;
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

    totalfiles = FALSE;
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
     case 0:							// Stop
		erraction = 0;
		break;

     case 1:							// Ask
		erraction = -1;
		break;

     case 2:							// Continue
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
    spec = getmem(size + sizeof(FS));
    strmov(spec->name, arg);
	if (srcspec == NULL)
		srcspec = spec;
    else if (dstspec == NULL)
		dstspec = spec;
    else
    {
		fputs("? RENAME: Too many files specified\n", stderr);
		exit(EXIT_INVSWT);
    }
    spec->next = NULL;
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
    int   last)			// TRUE if this is the last file to rename
{
    int  action;
    char msg[100];

    // First display the error message

    svcSysErrMsg(errcode, 0x03, msg);
    fprintf(stderr, "%s? RENAME: %s %s\n          %s\n", (nl) ? "\n" : "",
			text, name, msg);

    // Determine if we should continue

    if (erraction != 0)
    {
		action = 0;
		if (srcwild && !last)
			action = (erraction < 0) ? askuser("% RENAME: Continue renaming",
					FALSE) : erraction;
		if (action == 2)
			erraction = 1;
		if (action != 0)
			return;
    }
    finish();
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
        fputs("? RENAME: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
    if (maxmem < _malloc_amount)
        maxmem = _malloc_amount;
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
    char buffer[100];		// Buffer to receive error message

	if (code != 0)
	{
		svcSysErrMsg(code, 3, buffer);	// Get error message
		fprintf(stderr, "\n? RENAME: %s%s%s\n", buffer, (arg[0] != '\0') ?
				"; ": "", arg);			// Output error message
	}
	else
		fprintf(stderr, "\n? RENAME: %s\n", arg);
	exit(1);
}

//************************************************************************
// Function: baddwc - Display fatal error on invalid destination wild card
// Returned: Never returns
//************************************************************************

void baddwc(void)
{
	fputs("? RENAME: Invalid destination wild card usage\n", stderr);
	exit(1);
}
