//------------------------------------------------------------------------
// DELETE.C - File delete utility for XOS
//
// Revision history
// ??/??/?? 1.0 - (???) Initial development
// ??/??/?? 3.0 - (???) Updated to 32 bit
// 03/08/95 3.1 - (SAO) Added progasst package
// 03/13/95 3.2 - (sao) Changed 'option' indicator
// 03/16/95 -.- - (sao) Changed exit codes to reflect ALGRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp(). Changed setup for dirscan() so
//                 that wildcards are NOT the default - this fixed problem
//                 with deleting "xxx.*" when "xxx" was specified.
//  5Jun95 (fpj) - Re-ordered option table.
//
//------------------------------------------------------------------------

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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <xoserr.h>
#include <xosrtn.h>
#include <xcmalloc.h>
#include <xos.h>
#include <xossvc.h>
#include <heapsort.h>
#include <xostrm.h>
#include <progarg.h>
#include <dirscan.h>
#include <global.h>
#include <xoserrmsg.h>
#include <xosstr.h>

// Local defines

#define VERSION 3
#define EDITNO  4

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE2PARM  srcattr;
    BYTE2PARM  filattr;
    BYTE4PARM  alloc;
	BYTE4PARM  length;
    char       end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILEOPTN, XFO_FILE|XFO_MARKDIR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILESPEC, NULL, FILESPCSIZE+10},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR, XA_FILE},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILEATTR},
    {PAR_GET|REP_HEXV, 4 , IOPAR_REQALLOC},
    {PAR_GET|REP_HEXV, 4 , IOPAR_LENGTH}
};

int   maxname;					// Maximum name length
int   left;						// Number of lines left on screen
long  totalfiles;				// Total number of files deleted
long  totallength;
long  totalalloc;

// Misc. variables

PROGINFO pib;
char  copymsg[] = "";
char  prgname[] = "DELETE";
char  envname[] = "^XOS^DELETE^OPT";
char  example[] = "{/options} file-list";
char  description[] = "The file(s) specified are deleted.  Once they are "
		"removed, they can no longer be accessed, and the space they occupied "
		"on the disk will be reused by XOS.  Wildcard characters are allowed. "
		"More than one file specification per command line is allowed.";

char *pathbufr;
long  grandtotal = 0;		// Total number of files processed
char  nameseen;				// TRUE if saw a file name on the cmd line
char  fileseen;				// TRUE if file found
char  doall;				// TRUE if should not prompt any more
char  ellipok;				// TRUE if ellipsis has been confirmed

typedef struct filespec_ FS;
struct filespec_
{	FS  *next;				// Must be first item in structure
	FS  *sort;				// For heapsort
	long error;				// Error associated with file
	long alloc;
	long length;
	int  namelen;
	char name[1];			// File name
};

FS   *fshead;

int   maxlen;

typedef struct argstr_ ARGSTR;
struct argstr_
{	ARGSTR *next;
	char    argstr[1];		// Argument string
};

ARGSTR *firstarg;
ARGSTR *lastarg = (ARGSTR *)&firstarg;

_Packed struct
{   BYTE4PARM  options;
    LNGSTRPARM spec;
    char       end;
} openparm =
{	{PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_XOSDEV|XFO_PATH},
	{PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, FILESPCSIZE}
};

void  chkpause(void);
int   comp(FS *one, FS *two);
void  dodelete(void);
void  dodirectory(void);
void  showerrmsg(char const *, long);
void  finish(void);
void *getmem(size_t size);
void  helpprint(char *helpstring, int state, int newline);
int   hvellip(void);
int   nonopt(char *arg);
int   procfile(void);
int   prompt(char *msg);

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,		// parmlist - Address of parameter list
    (PROCFILE *)procfile,		// func     - Function called for each match
    showerrmsg,					// error    - Function called on error
    DSSORT_ASCEN				// sort     - Directory sort order
};

long  sorted = TRUE;            // Sort file list if TRUE
long  quiet = FALSE;            // No output unless error if TRUE
long  mute = FALSE;             // Suppress all mesages
long  confirm = FALSE;          // Confirm before deleting if TRUE
long  recur = FALSE;

char  curpath[FILENAME_MAX + 20];

arg_spec options[] =
{	{"S*ORT"     , ASF_BOOL|ASF_STORE, NULL, AF(&sorted) , TRUE, "Sort the file list."},
    {"C*ONFIRM"  , ASF_BOOL|ASF_STORE, NULL, AF(&confirm), TRUE, "Require confirmation before deleting.  This option over-rides both quiet and mute."},
    {"R*ECURSIVE", ASF_BOOL|ASF_STORE, NULL, AF(&recur)  , TRUE, "Delete recursively."},
    {"Q*UIET"    , ASF_BOOL|ASF_STORE, NULL, AF(&quiet)  , TRUE, "Only errors are reported."},
    {"M*UTE"     , ASF_BOOL|ASF_STORE, NULL, AF(&mute)   , TRUE, "No messages.  Error code reported on exit."},
    {"H*ELP"     , 0                 , NULL, AF(opthelp) , 0   , "This message."},
    {"?"         , 0                 , NULL, AF(opthelp) , 0   , "This message."},
    {NULL}
};


//**********************************
// Function: main - Main program
// Returned: 0 if normal, 1 if error
//**********************************

int main(
    int   argc,
    char *argv[])
{
	char   *pnt;
	char   *name;
    char   *envargs[2];
    ARGSTR *nextarg;
    char    filspec[FILESPCSIZE+10];
	char    chr;

	reg_pib(&pib);

	// set Program Information Block variables

	pib.opttbl = options; 		// Load the option table
    pib.kwdtbl = NULL;
	pib.build = __DATE__;
	pib.majedt = VERSION; 			// major edit number
	pib.minedt = EDITNO; 			// minor edit number
	pib.copymsg = copymsg;
	pib.prgname = prgname;
	pib.desc = description;
	pib.example = example;
	pib.errno = 0;
	getTrmParms();
	getHelpClr();

    pathbufr = getspace(1024);
    fileparm.filspec.buffer = filspec;

    // Check global configuration parameters and enviroment variable

    global_parameter(TRUE);
    if (gcp_dosquirk)
	quiet = TRUE;

    if(svcSysGetEnv(envname, (char const **)&envargs[0]) > 0)
    {
		envargs[1] = '\0';
    	progarg(envargs, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    // Process the command line

    ++argv;
    progarg(argv, PAF_PSWITCH, options, NULL, nonopt,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    if (argc < 2 || firstarg==NULL)		// Have any arguments?
	{
		if (!mute) fprintf(stderr,"? %s: A file specification must be "
				"provided.\n",prgname);
		exit(EXIT_FNF);
	}
	else
	{
		left=pib.screen_height-2;
		do
		{
			nameseen = TRUE;
			fileseen = FALSE;
			if (!recur)
			{
			    pnt = name = firstarg->argstr;
			    while ((chr = *pnt++) != 0)	// Find the start of the name part
			    {
					if (chr == ':' || chr == '\\' || chr == '/')
					    name = pnt;
			    }
				if ((name == (firstarg->argstr + 4) ||
					(name > (firstarg->argstr+4) && name[-5] == '\\' ||
					name[-5] == ':')) && strncmp(name - 4, "...", 3) == 0)
				{
					strmov(name - 4, name);
					recur = DSO_RECUR;
				}
			}
			dirscan(firstarg->argstr, &dsd, recur); // Scan the directory
			nextarg = firstarg->next;
			free(firstarg);
		} while ((firstarg = nextarg) != NULL);
		dodirectory();
	}
	chkpause();
	finish();
	return (0);
}


void finish(void)
{
	if ( !mute && !quiet )
		printf("  %,d file%s deleted (%,d byte%s written, %,d allocated)\n",
				totalfiles, (totalfiles != 1)? "s": "",
				totallength, (totallength != 1)? "s": "", totalalloc);
}


//************************************************
// Function: nonopt - Process a file specification
// Returned: TRUE if OK, does not return if error
//************************************************

int nonopt(
    char *arg)
{
    ARGSTR *thisarg;

    if ((thisarg = (ARGSTR *)malloc(sizeof(ARGSTR) + strlen(arg) + 8)) == NULL)
    {
        if ( !mute )
            fputs("? DELETE: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
    strmov(thisarg->argstr, arg);
    thisarg->next = NULL;
    lastarg->next = thisarg;
    lastarg = thisarg;
    return (TRUE);
}


//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

int procfile(void)
{
	FS *pfs;

	if (dsd.filenamelen < 0)
	{
		dodirectory();
		strmov(curpath, dsd.pathname);
		maxlen = 0;
	}
	else
	{
		pfs = (FS *)getmem(offsetof(FS, name) + 1 + dsd.filenamelen);
		if (maxlen < dsd.filenamelen)
			maxlen = dsd.filenamelen;
		strmov(pfs->name, dsd.filename);
		pfs->alloc = fileparm.alloc.value;
		pfs->length = fileparm.length.value;
		pfs->namelen = dsd.filenamelen;
		pfs->next = fshead;
		fshead = pfs;
	}
	return (TRUE);
}


void dodirectory(void)
{
	FS   *next;
	char *npnt;
	int   ncnt;
	int   numperline;
	int   fill;
	char  linebufr[84];

	if (fshead != NULL)
	{
		if (sorted && fshead->next != NULL)
			fshead = (FS *)heapsort(fshead, (HEAPCMP *)comp, 0);
		if (!confirm)
		    chkpause();
		printf("Deleting from %s\n", curpath);
		if ((numperline = 80 / (maxlen + 1)) == 0)
			numperline = 1;
		ncnt = 0;
		fill = 0;
		npnt = linebufr;
		do
		{
			if (confirm)
			{
				switch(prompt(curpath + 1))
				{
				 case 0:
					fputs(" skipped\n", stdout);
					break;

				 case 1:
					dodelete();
					fputs(" deleted\n", stdout);
					break;
				}
			}
			else
			{
				dodelete();				
				if (ncnt == numperline)
				{
					chkpause();
					printf("%s\n", linebufr);
					ncnt = 0;
					fill = 0;
					npnt = linebufr;
				}
				else if (fill != 0)
					npnt += sprintf(npnt, "%*s", fill, "");
				npnt = strmov(npnt, fshead->name);
				fill = maxlen - fshead->namelen + 1;
				ncnt++;
			}
			next = fshead->next;
			free(fshead);
		} while ((fshead = next) != NULL);
		if (ncnt != 0)
		{
			chkpause();
			printf("%s\n", linebufr);
		}
	}
}


void dodelete(void)
{
	long rtn;
	char bufr[520];

	sprintf(bufr, "%s%s", curpath, fshead->name);

///	printf("### deleting: |%s|\n", bufr);

	if ((rtn = svcIoDelete(0, bufr, NULL)) < 0)
	{
		fprintf(stderr, "? DELETE: Error deleting %s\\%s\n", curpath,
				fshead->name);
		svcSysErrMsg(rtn, 0x03, bufr);
		fprintf(stderr, "          %s\n", bufr);
		finish();
		exit (1);
	}
	totalfiles++;
	totallength += fshead->length;
	totalalloc += fshead->alloc;
}


//****************************************************
// Function: comp - Compare two filenames for heapsort
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//****************************************************

int comp(
    FS *one,
    FS *two)
{
    char *pnt1;
    char *pnt2;
    char  chr1;
    char  chr2;

//    if (revsort)			/* Want reverse order sort? */
//    {
//        struct filespec *temp;
//
//        temp = one;
//        one = two;
//        two = temp;
//    }

    pnt1 = one->name;
    pnt2 = two->name;
    while (((chr1 = *pnt1++) == (chr2 = *pnt2++)) && chr1 != 0)
        ;
    if (chr1 == '\\' || chr1 == ':')
        chr1 = 0;
    if (chr2 == '\\' || chr2 == ':')
        chr2 = 0;
    return (chr1 - chr2);
}


//**********************************************
// Function: prompt - Ask about deleting a file
// Returned: 1 if should delete, 0 if should not
//**********************************************

int prompt(
    char *msg)
{
    char display;

    if (doall)
    {
        fputs(msg, stderr);
        return (1);
    }
    display = TRUE;
    for (;;)
    {
        if (display)
        {
            fprintf(stderr, "Delete %s [Y,N,G,Q,?] ?", msg);
            display = FALSE;
        }
        switch (toupper(getch()))
        {
         case 'G':
            doall = TRUE;
         case 'Y':
            return (1);

         case 'N':
            return (0);

         case 'Q':
            fputs("\n", stderr);
			finish();
            exit(EXIT_NORM);

         case '?':
            fputs("\n\n  Enter one of the following single characters:\n"
                    "    Y = Yes, delete the file\n"
                    "    N = No, do not delete the file\n"
                    "    G = Go, delete this and all following files\n"
                    "    Q = Quit, do not delete this or any following files\n"
                    "    ? = Display this text\n\n", stderr);
            display = TRUE;
        }
    }
}


//********************************************
// Function: chkpause - Check for screen pause
// Returned: Nothing
//********************************************

void chkpause(void)
{
    char msg;

    if ( mute || quiet )
        return;

    if (!pib.console || --left > 0)
        return;
    msg = TRUE;
    for (;;)
    {
        if (msg)
        {
            fputs("\33[7m-MORE- G, H, ?, <Enter> or <Space>\33[0m", stdout);
            msg = FALSE;
        }
        switch (toupper((getch()) & 0x7F))
        {
         case 'G':			// Don't ask for any more
            pib.console = FALSE;
         case '\r':
         case ' ':			// Do another screen full
            fputs("\r\33[K", stdout);
            left = pib.screen_height - 2;
            return;

         case 'H':
         case '?':
            fputs("\r\33[K\33[7m"
                    "  G      - Go, don't ask for -MORE-\n"
                    "  H or ? - Help, this message      \n"
                    " <Enter> - Next screen             \n"
                    " <Space> - Next screen             \n", stdout);
            msg = TRUE;
            break;
        }
    }
}


//*********************************************
// Function: showerrmsg - Display error message
// Returned: Nothing
//*********************************************

void showerrmsg(
	char const *arg,
	long        code)
{
    char buffer[80];		// Buffer to receive error message

    fileseen = TRUE;
    if ( mute )
        return;
    svcSysErrMsg(code, 3, buffer); // Get error message
    fprintf(stderr, "\n? %s: %s", prgname, buffer);
    if (arg != NULL && *arg != '\0')	// Have returned name?
	fprintf(stderr, ", file %s", arg); // Yes - output it too
    fputc('\n', stderr);
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
		finish();
        exit(EXIT_MALLOC);
    }
    return (ptr);
}
