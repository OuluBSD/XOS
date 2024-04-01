//------------------------------------------------------------------------
// RMDIR.C - Directory delete utility for XOS
//
// Utility to remove empty sub-directories from the file structure
//
// Revision History
// ??/??/??(xxx) - Initial release
// 04/11/95(sao) - Added progasst package
// 05/14/95(sao) - changed example, added mute option
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
// 24May95 (fpj) - Fixed bug with dangling else after check for arguments.
//------------------------------------------------------------------------

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

// This program will remove a single directory or an entire directory tree,
//   deleting all files in the directories.
// The command format is a bit complex but is consistant with other XOS
//   commands.
// The final name specified (with or without wild cards) is the name of a
//   directory to remove. All matching directories are removed. If /r is
//   specified, the entire tree under each matching directory is removed
//   along with the matching directory. If /delete is specified all files in
//   a directory are deleted before the directory is removed. If /delete is
//   not specified and one of the directories to be removed is not empty
//   (other than subdirectories which would also be removed, the command
//   will fail.
// Note that the specification "/r dir1\dir2\" is not valid. If allowed,
//   this would mean "/r dir1\dir2\*". It is not allowed to help prevent
//   some unintended removal of directores. Also for file systems which use
//   separate name and extension parts (FAT and XFS), "*" is taken as "*.",
//   that is, it matches only names with no extensions (which do not contain
//   any periods). To match all names, specify "*.*".
// /DELETE also implies /R.
// The ... syntax is not allowed here since the meaning of "recursive" here
//   is somewhat different that in other commands. In this command any wild-
//   card specification only applies at the top level, not a every as in
//   other commands. It is felt that allowing ... in this case would be
//   misleading.
// The most common usages are:
//   To remove a sigle directory (directory must be empty):
//		rmdir dirname
//   To remove a complete directory tree, including the base directory and all
//   files in the directories:
//      rmdir /delete basename
//   To remove all directories under a base directory without removing the base
//   directory or deleting any files in the base directory itself:
//		rmdir /delete basename\*.*
// /clear is not valid without /r.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xcmalloc.h>
#include <xoserr.h>
#include <xosrtn.h>
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
#define EDITNO  3

typedef struct filespec_ FILESPEC;
struct filespec_
{	FILESPEC *next;				// Must be first item in structure
	FILESPEC *sort;				// For heapsort
	long      error;			// Error associated with file
	char      attr;
	char      filespec[1];		// File specification
};

typedef struct dirspec_ DIRSPEC;
struct dirspec_
{	DIRSPEC  *next;
	FILESPEC *firstfile;
	char      path[1];
};

void chkpause(void);
int  comp(FILESPEC *one, FILESPEC *two);
void dodelete(char *name, char *label);
void dofiles(void);
void showerrmsg(const char *, long);
void hvellip(void);
int  nonopt(char *arg);
int  procfile1(void);
int  procfile2(void);
int  prompt(char *msg);
void summary(void);

PROGINFO pib;
long  delete = FALSE;			// Delete files from directories
long  quiet = FALSE;			// No output unless error if TRUE
long  mute = FALSE;				// No output unless error if TRUE
long  confirm = FALSE;			// Confirm before deleting if TRUE
long  recur = FALSE;

arg_spec options[] =
{
///	{"C*ONFIRM"  , ASF_BOOL|ASF_STORE, NULL, AF(&confirm), TRUE, "Ask for confirmation before taking action."},
    {"R*ECURSIVE", ASF_BOOL|ASF_STORE, NULL, AF(&recur)  , TRUE, "Do recursive search."},
    {"DELETE"    , ASF_BOOL|ASF_STORE, NULL, AF(&delete) , TRUE, "Do recursive search and delete all files in each direcory."},
    {"Q*UIET"    , ASF_BOOL|ASF_STORE, NULL, AF(&quiet)  , TRUE, "Suppress all but error messages."},
    {"M*UTE"     , ASF_BOOL|ASF_STORE, NULL, AF(&mute)   , TRUE, "Suppress all messages."},
    {"H*ELP"     , 0                 , NULL, AF(opthelp) , 0   , "This message."},
    {"?"         , 0                 , NULL, AF(opthelp) , 0   , "This message."},
    {NULL}
};

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE2PARM  srcattr;
    BYTE2PARM  filattr;
    char       end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILEOPTN , XFO_MARKDIR|XFO_FILE|XFO_MARKDIR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILESPEC , NULL, FILESPCSIZE+10},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR , XA_DIRECT},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILEATTR},
};

struct
{	BYTE2PARM  srcattr;
	char       end;
} delparm =
{    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR}
};

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,	// parmlist - Address of parameter list
    (PROCFILE *)procfile1,	// func     - Function called for each match
    showerrmsg,				// error    - Function called on error
    DSSORT_ASCEN			// sort     - Directory sort order
};

int   scrnwidth  = 80;			// Width of screen in columns
int   scrnheight = 25;			// Height of screen
int   maxname;					// Maximum name length
int   left;						// Number of lines left on screen
int   dirtotal;					// Total number of directories deleted
int   filetotal;				// Total number of files deleted

// Misc. variables

char  copymsg[] = "";
char  prgname[] = "RMDIR";
char  envname[] = "^XOS^RMDIR^OPT";
char  example[] = "{/options} directory-spec";
char  description[] = "This command deletes the directories named.  " \
    "Each dirctory must be empty, except for the . and .. entries.  " \
    "RMDIR supports wildcards and ellipsis directory name specifications.";

char *pathbufr;
char  doall;					// TRUE if should not prompt any more
char  console;					// TRUE if STDOUT is console

FILESPEC *firstbase;
FILESPEC *lastbase;
FILESPEC *firstfile;			// Pointer to first file to process
FILESPEC *lastfile;				// Pointer to last file to process

DIRSPEC  *dirhead;

char      curpath[300];
int       curpathlen;

char      basename[300];
int       basenamelen;

struct argstr
{	struct argstr *next;
	char   argstr[1];			// Argument string
};

struct argstr *firstarg;
struct argstr *lastarg = (struct argstr *)&firstarg;

struct
{   BYTE4PARM  options;
    LNGSTRPARM spec;
    char       end;
} openparm =
{	{(PAR_SET|REP_HEXV), 4, IOPAR_FILEOPTN, XFO_XOSDEV|XFO_PATH},
	{(PAR_GET|REP_STR),  0, IOPAR_FILESPEC, NULL, FILESPCSIZE}
};


//**********************************
// Function: main - Main program
// Returned: 0 if normal, 1 if error
//**********************************

int main(
    int   argc,
    char *argv[])
{
    char      *envargs[2];
	char      *baseend;
    struct     argstr *nextarg;
    TRMMODES   data;
	FILESPEC **fppnt;
	FILESPEC  *thisfile;
	FILESPEC  *nextfile;
    long       rtn;
    char       filspec[FILESPCSIZE+10];
	char       buffer[300];

    reg_pib(&pib);

	// set Program Information Block variables

	pib.opttbl=options; 		// Load the option table
    pib.kwdtbl=NULL;
	pib.build=__DATE__;
	pib.majedt = VERSION; 			// major edit number
	pib.minedt = EDITNO; 			// minor edit number
	pib.copymsg=copymsg;
	pib.prgname=prgname;
	pib.desc=description;
	pib.example=example;
	pib.errno=0;
	getTrmParms();
	getHelpClr();

    pathbufr = getspace(1024);
    fileparm.filspec.buffer = filspec;

    // Check global configuration parameters and enviroment variable

    global_parameter(TRUE);

    if (gcp_dosquirk)
		quiet = TRUE;

///    if (gcp_dosdrive)
///        fileparm.filoptn.value ^= (XFO_DOSDEV|XFO_XOSDEV|XFO_RDOSDEV|
///                XFO_RXOSDEV);

    if(svcSysGetEnv(envname, (const char **)&envargs[0]) > 0)
    {
		envargs[1] = '\0';
        progarg(envargs, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    // Process the command line

    ++argv;

    progarg(argv, PAF_PSWITCH, options, NULL, nonopt,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if (argc < 2 || firstarg == NULL )			// Have any arguments?
    {
        if (!mute)
			fputs("? RMDIR: No directory was specified\n", stderr);
		return (1);
    }
	recur |= delete;

	if ((console=isctrm(DH_STDOUT)) != FALSE) // Check console output state
	{
		if ((rtn = svcTrmDspMode(STDTRM, DM_RTNDATA, &data)) < 0)
										// Save old term
			console = FALSE;
		else
		{
			scrnwidth = (int)(data.dm_columns);
			scrnheight = (int)(data.dm_rows);
		}
	}
	left = scrnheight - 2;

	do
	{
		// Construct the list of directores for this argument

		firstbase = NULL;
		lastbase = NULL;
		fileparm.srcattr.value = XA_DIRECT;
		dsd.func = (PROCFILE *)procfile1;
		dirscan(firstarg->argstr, &dsd, FALSE); // Scan the directory

		if (firstbase == 0)
		{
			fprintf(stderr, "? RMDIR: No directories matching %s found\n",
					firstarg->argstr);
			exit (1);
		}

		do
		{
///			printf("### BEGIN:: |%s| |%s|\n", basename,
///					firstbase->filespec);

			baseend = buffer + sprintf(buffer, "%s%s*.*", basename,
					firstbase->filespec) - 4;

///			printf("### BEGIN: %s\n", buffer);

			if (recur)
			{
				// Process each matching base directory - First construct the
				//   list of directories and files to be deleted.

				fileparm.srcattr.value = (delete) ? (XA_DIRECT | XA_FILE) :
						XA_DIRECT;
				dsd.func = (PROCFILE *)procfile2;
				dirscan(buffer, &dsd, DSO_RECUR); // Scan the directory
				if (curpath[0] != 0)
					dofiles();

				// Here with the list of directories and files to be deleted
				//   constructed. The list is ordered from bottom to top. The
				//   directory tree has been converted to a simple list by
				//   traversing each directory in alphabetical order.

				while (dirhead != NULL)
				{
///					printf("### LEVEL: %s\n", dirhead->path);

					fppnt = &dirhead->firstfile;

					// Delete any directories we found at this level

					delparm.srcattr.value = XA_DIRECT;
					while ((thisfile = *fppnt) != NULL)
					{
						sprintf(filspec, "%s%s", dirhead->path,
								thisfile->filespec);
						if ((thisfile->attr & XA_DIRECT))
						{
							dodelete(filspec, "directory");
							dirtotal++;
							*fppnt = thisfile->next;
///							free(thisfile);
						}
						else
							fppnt = &thisfile->next;
					}

					// Delete any files we found at this level

					thisfile = dirhead->firstfile;
					delparm.srcattr.value = XA_FILE;
					while (thisfile != NULL)
					{
						sprintf(filspec, "%s%s", dirhead->path,
								thisfile->filespec);
						dodelete(filspec, "file");
						filetotal++;
						nextfile = thisfile->next;
///						free(thisfile);
						thisfile = nextfile;
					}
					dirhead = dirhead->next;
				}
			}
			*baseend = 0;
			delparm.srcattr.value = XA_DIRECT;
			dodelete(buffer, "directory");
			dirtotal++;
		} while ((firstbase = firstbase->next) != NULL);

		nextarg = firstarg->next;

	} while ((firstarg = nextarg) != NULL);

	summary();
    return (0);
}


void dodelete(
	char *spec,
	char *label)
{
	long rtn;
	char etxt[100];

///	printf("### dodelete: |%s|\n", spec);

	if (label[0] == 'd')				// If directory, remove the trailing
	{									//   back-slash
		rtn = strlen(spec);
		if (spec[rtn - 1] == '\\')
			spec[rtn - 1] = 0;
	}
	if ((rtn = svcIoDelete(0, spec, &delparm)) < 0)
	{
		if (!mute)
		{
			chkpause();
			svcSysErrMsg(rtn, 0x03, etxt);
			fprintf(stderr, "? RMDIR: Error deleting %s %s%s\n         %s\n",
				label, spec, (label[0] == 'd') ? "\\" : "", etxt);
			summary();
		}
		exit(1);
	}
	else if (!mute && !quiet)
	{
		chkpause();
		printf("  %s%s%s\n", (label[0] == 'd') ? "" : "  ", spec,
				(label[0] == 'd') ? "\\" : "");
	}
}


void summary(void)
{
	if (!mute && !quiet)
	{
		chkpause();
		if (delete)
			printf("%,d director%s and %,d file%s deleted\n", dirtotal,
					(dirtotal != 1)? "ies": "y", filetotal, (filetotal != 1) ?
					"s" : "");
		else
			printf("%d director%s deleted\n", dirtotal, (dirtotal != 1) ?
					"ies": "y");
	}
}


//************************************************
// Function: nonopt - Process a file specification
// Returned: TRUE if OK, does not return if error
//************************************************

int nonopt(
    char *arg)
{
    struct argstr *thisarg;

    if ((thisarg = (struct argstr *)malloc(sizeof(struct argstr) +
            strlen(arg) + 1)) == NULL)
    {
        if (!mute) fputs("? RMDIR: Not enough memory available\n", stderr);
        exit(1);
    }
    if (strmov(thisarg->argstr, arg)[-1] == '\\')
	{
		fprintf(stderr, "? RMDIR: Invalid directory specification (Trailing \\ "
				"is not allowed.)\n           %s\n", arg);
		exit(1);
	}
    thisarg->next = NULL;
    lastarg->next = thisarg;
    lastarg = thisarg;
    return (TRUE);
}


//*******************************************
// Function: hvellip - Confirm use of ellipis
// Returned: TRUE
//*******************************************

void hvellip(void)
{
    fputs("% RMDIR: Ellipsis delete selected, are you sure [Y/N]? ", stderr);
    for (;;)
    {
        switch (toupper(getch()))
        {
         case 'Y':
            fputs("\n", stderr);
            return;

         case 'N':
            fputs("\n", stderr);
            exit(EXIT_NORM);
        }
    }
}


//*************************************************************
// Function: procfile1 - Function called by dirscan for each
//				directory found during the base scan
// Returned: TRUE if should continue, FALSE if should terminate
//*************************************************************

// This is a non-recursive scan

int procfile1(void)
{
    FILESPEC *thisbase;

///	printf("### procfile1: %d |%s| %d |%s|\n", dsd.pathnamelen,
///			dsd.pathname, dsd.filenamelen, dsd.filename);

    if (dsd.filenamelen < 0)
	{
		strmov(basename, dsd.pathname);
		basenamelen = dsd.pathnamelen;
		return (TRUE);
	}

    if (dsd.error < 0 &&
            (dsd.error != ER_FILAD && dsd.error != ER_FBFER &&
            dsd.error != ER_FBFER && dsd.error != ER_FBWER &&
            dsd.error != ER_BUSY))
    {
        char buffer[100];

        showerrmsg(buffer, dsd.error);
        return (FALSE);
    }
    if ((thisbase = (FILESPEC *)malloc(sizeof(FILESPEC) +
            dsd.filenamelen + 1)) == NULL)
    {
        if (!mute) fputs("? RMDIR: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
    strmov(thisbase->filespec, dsd.filename);
    if (firstbase == NULL)
        firstbase = thisbase;
    else
        lastbase->next = thisbase;
    lastbase = thisbase;
    thisbase->next = NULL;
    return (TRUE);
}


//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

int procfile2(void)
{
    FILESPEC *thisfile;

///	printf("### procfile2: |%s| |%s|\n", dsd.pathname, dsd.filename);

    if (dsd.filenamelen < 0)
    {
		if (curpath[0] != 0)
	        dofiles();
		strmov(curpath, dsd.pathname);
		curpathlen = dsd.pathnamelen;
		return (TRUE);
    }

    if (dsd.error < 0 &&
            (dsd.error != ER_FILAD && dsd.error != ER_FBFER &&
            dsd.error != ER_FBFER && dsd.error != ER_FBWER &&
            dsd.error != ER_BUSY))
    {
        char buffer[100];

        showerrmsg(buffer, dsd.error);
        return (FALSE);
    }

    if ((thisfile = (FILESPEC *)malloc(sizeof(FILESPEC) +
            dsd.filenamelen + 1)) == NULL)
    {
        if (!mute) fputs("? RMDIR: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
    if (maxname < dsd.filenamelen)
        maxname = dsd.filenamelen;
	thisfile->attr = (char)fileparm.filattr.value;
    strmov(thisfile->filespec, dsd.filename);
    if (firstfile == NULL)
        firstfile = thisfile;
    else
        lastfile->next = thisfile;
    lastfile = thisfile;
    thisfile->next = NULL;

    return (TRUE);
}


//**********************************************************
// Function: dofiles - Process list of directories to delete
// Returned: Nothing
//**********************************************************

// This function simply adds the list to a higher level list to we can
//   delete the directories from the bottom up so recursvie delete will
//   work!

void dofiles(void)
{
    DIRSPEC *thisdir;

///	printf("### dofiles: %s\n", curpath);

    if (firstfile == NULL)
        return;

    if (firstfile->next != NULL)
        firstfile = heapsort(firstfile, (int (*)(void *a, void *b,
				void *d))comp, NULL);
    if ((thisdir = (DIRSPEC *)malloc(sizeof(DIRSPEC) + curpathlen + 1)) == NULL)
    {
        if (!mute)
			fputs("? RMDIR: Not enough memory available\n", stderr);
        exit(EXIT_MALLOC);
    }
	thisdir->firstfile = firstfile;
	strmov(thisdir->path, curpath);
	thisdir->next = dirhead;
	dirhead = thisdir;	
	firstfile = NULL;
	lastfile = NULL;
}


//****************************************************
// Function: comp - Compare two filenames for heapsort
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//****************************************************

int comp(
    FILESPEC *one,
    FILESPEC *two)
{
    char *pnt1;
    char *pnt2;
    char  chr1;
    char  chr2;

    pnt1 = one->filespec;
    pnt2 = two->filespec;
    while (((chr1 = *pnt1++) == (chr2 = *pnt2++)) && chr1 != 0)
        ;
    if (chr1 == '\\' || chr1 == ':')
        chr1 = 0;
    if (chr2 == '\\' || chr2 == ':')
        chr2 = 0;
    return (chr1 - chr2);
}


//*****************************************************
// Function: prompt - Ask about deleting a file
// Returned: TRUE if should delete, FALSE if should not
//*****************************************************

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
            return (TRUE);

         case 'N':
            return (FALSE);

         case 'Q':
		 case 0x03:
            fputs("\n", stderr);
            exit(0);

         case '?':
            fputs("\n\n  Enter one of the following single characters:\n"
                    "    Y = Yes, delete the directory\n"
                    "    N = No, do not delete the directory\n"
                    "    G = Go, delete this and all following directories\n"
                    "    Q = Quit, do not delete this or any following "
                    "directories\n"
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

    if (!console || --left > 0)
        return;
    msg = TRUE;
    for (;;)
    {
        if (msg)
        {
            fputs("\33[7m-MORE- G, Q, ?, <Enter> or <Space> \33[0m", stdout);
            msg = FALSE;
        }
        switch (toupper((getch()) & 0x7F))
        {
         case 'G':			// Don't ask for any more
            console = FALSE;
         case '\r':
         case ' ':			// Do another screen full
            fputs("\r\33[K", stdout);
            left = scrnheight - 2;
            return;

         case 'Q':
		 case 0x03:
            fputs("\n", stderr);
            exit(0);

         case '?':
            fputs("\r\33[K\33[7m"
                    "  G      - Go, don't ask for -MORE- \n"
					"  Q      - Quit                     \n"
                    "  ?      - Help, this message       \n"
                    " <Enter> - Next screen              \n"
                    " <Space> - Next screen              \n", stdout);
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
	const char *arg,
	long  code)
{
    char buffer[80];		// Buffer to receive error message

	if ( mute )
		return;
    svcSysErrMsg(code, 3, buffer); // Get error message
    fprintf(stderr, "\n? %s: %s", prgname, buffer);
    if (arg != NULL && *arg != '\0')	// Have returned name?
	fprintf(stderr, ", file %s", arg); // Yes - output it too
    fputc('\n', stderr);
}
