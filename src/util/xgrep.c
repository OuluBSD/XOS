//--------------------------------------------------------------------------*
// xgrep.c
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
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

#define VERSION 4
#define EDITNO  0


typedef struct
{	uchar val[256];
} PATTERN;

#define MATCH 1

long     totalfiles;
long     matchfiles;
long     matchnum;
PATTERN *pattern;
long     patsize;
char     filespc[264];
char     patstr[68];
char     firstchr;

// Function prototypes

///int   comp(FILEDESCP *one, FILEDESCP *two);
void *getmem(size_t size);
int   nonopt(char *arg);
void  pagecheck(void);
int   procfile(void);
void  showerrmsg(const char *arg, long  code);
void  init_Vars(void);

void  makepattern(char *str, int icase);
int   usepattern(char *str);

// Switch settings functions

int  optignore(void);
int  optpause(arg_data *);

// Stuff needed by dirscan

///char owner[36];

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE2PARM  srcattr;
    BYTE2PARM  filattr;
    char       end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILEOPTN , XFO_FILE|XFO_MARKDIR},
    {PAR_GET|REP_STR , 0 , IOPAR_FILESPEC , NULL, 0, 0},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR , XA_FILE},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILEATTR , 0},
};

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,	// parmlist - Address of parameter list
    (PROCFILE *)procfile,	// func     - Function called for each file matched
    showerrmsg,				// error    - Function called on error
    DSSORT_ASCEN			// sort     - Directory sort order
};

// Variables addressed directly in option table

long  debug = FALSE;		// TRUE if want debug output
long  ppause = TRUE;		// TRUE if should pause at end of page
long  igncase = FALSE;		// TRUE if should ignore case when comparing
long  files = FALSE;		// TRUE if should only list file names
long  recur = FALSE;

arg_spec options[] =
{	{"DE*BUG"    , ASF_BOOL|ASF_STORE, NULL, AF(&debug)  , 1, NULL},
	{"H*ELP"     , 0                 , NULL, AF(opthelp) , 0, "Display this message"},
	{"?"         , 0                 , NULL, AF(opthelp) , 0, "Display this message"},
	{"I*GNORE"   , ASF_BOOL|ASF_STORE, NULL, AF(&igncase), 1, "Ignore case when comparing" },
	{"F*ILES"    , ASF_BOOL|ASF_STORE, NULL, AF(&files)  , 1, "Only display file names"},
	{"P*AUSE"    , ASF_BOOL|ASF_STORE, NULL, AF(&ppause) , 1, "Pause at end of page"},
	{"R*ECURSIVE", ASF_BOOL|ASF_STORE, NULL, AF(&recur)  , 1, "Do recursive search"},
    {NULL}
};

// Option related variables

int    curline = -1;		// Current line on the screen


// Misc. variables

PROGINFO pib;
char    copymsg[] = "";
char    prgname[] = "XGREP";	// Our programe name
char    envname[] = "^XOS^XGREP^OPT"; // The environment option name

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

extern uint _malloc_amount;
uint   maxmem;

void main(
    int   argc,
    char *argv[])

{
	char *envpnt[2];

	if (argc < 3)
	{
		fputs("? XGREP: Command error, usage is:\n           xgrep string "
				"file\n", stderr);
		exit(1);
	}

    // Set defaults

	dsd.needwcc = TRUE;

	reg_pib(&pib);
	init_Vars();

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    // Check Local Configuration Parameters

    if(svcSysGetEnv(envname, (const char **)&envpnt[0]) > 0)
    {
		envpnt[1] = NULL;
		progarg(envpnt, PAF_EATQUOTE, options, NULL, (int (*)(char *))NULL,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    // Process the command line

    if (argc >= 2)
    {
		++argv;
		progarg(argv, PAF_EATQUOTE, options, NULL, nonopt,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    // Set up the conditions

///	if (gcp_dosdrive)
///		fileparm.filoptn.value = XFO_DOSNAME|XFO_NODENUM|XFO_NODENAME|
///				XFO_RDOSNAME|XFO_PATHNAME|XFO_ATTR;

    if (!pib.console)
        ppause = FALSE;

	printf("XGREP: Searching for \"%s\" in %s (%s)\n", patstr, filespc,
			(igncase) ? "IC" : "UC");

	makepattern(patstr, igncase);
	dirscan(filespc, &dsd, recur);
	printf("%d file%s searched and %d match%s found in %d file%s\n", totalfiles,
			(totalfiles == 1) ? " was" : "s were", matchnum, (matchnum == 1) ?
			" was" : "es were", matchfiles, (matchfiles == 1) ? "" : "s");

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
    pib.desc=NULL;
    pib.example=NULL;
    pib.errno=0;
    getTrmParms();
    getHelpClr();
}


//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

// This function is called by dirscan for each file found.

int procfile(void)
{
	FILE *file;
    char *pnt;
	long  linenum;
	int   binx;
	int   tabsize;
	char  linebuf[300];
	char  prtbuf[300];
	char  chr;
	uchar needfile;

	if (dsd.filenamelen > 0)
	{
		sprintf(prtbuf, "%s%s", dsd.pathname, dsd.filename);
		if (dsd.error < 0)
			femsg2(prgname, "Error opening file", dsd.error, prtbuf);
		if ((file = fopen(prtbuf, "r")) == NULL)
			femsg2(prgname, "Error opening file", -errno, prtbuf);
		totalfiles++;
		needfile = TRUE;
		linenum = 0;
		while ((fgets(linebuf, sizeof(linebuf), file)) != NULL)
		{
			linenum++;
			if (usepattern(linebuf))
			{
				if (needfile)
				{
					needfile = FALSE;
				    pagecheck();
					fputc('\n', stdout);
				    pagecheck();
					printf("File: %s\n", prtbuf);
					matchfiles++;
					if (files)
						break;
					tabsize = ((pnt = strrchr(prtbuf, '.')) != NULL &&
							(pnt[2] == 0 && (chr = toupper(pnt[1])) == 'C' ||
							chr == 'H')) ? 4 : 8;
				}
			    pagecheck();
				pnt = linebuf;
				binx = sprintf(prtbuf, "%7d|", linenum);
				while (binx < (sizeof(prtbuf) - 10) && (chr = *pnt++) != 0 &&
						chr != '\n')
				{
					if (chr == '\t')
						do
						{
							prtbuf[binx++] = ' ';
						} while ((binx & (tabsize - 1)) != 0);
					else
						prtbuf[binx++] = chr;
				    if (pib.console && binx >= 80)
						break;
				}
				prtbuf[binx] = '\n';
				prtbuf[binx + 1] = 0;
				fputs(prtbuf, stdout);
				matchnum++;
			}
		}
		fclose(file);
	}
   	return (TRUE);
}


//******************************************************************
// Function: pagecheck - Check the page count and pause if necessary
// Returned: Nothing
//******************************************************************

void pagecheck(void)
{
    long temp;					// Temp int for keyboard status
    int  more;

    if (ppause && (++curline >= pib.screen_height - 2))
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
                ppause = FALSE;
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


//********************************************
// Function: nonopt - process non-option input
// Returned: Nothing
//********************************************

int nonopt(
    char *arg)
{
	int len;

	len = strlen(arg);
	if (patstr[0] == 0)
	{
		if (len > 64)
		{
			fputs("? XGREP: Pattern is too long\n", stderr);
			return (FALSE);
		}
		memcpy(patstr, arg, len);
		firstchr = patstr[0];
	}
	else if (filespc[0] == 0)
	{
		if (len > 260)
		{
			fputs("? XGREP: File specification is too long\n", stderr);
			return (FALSE);
		}
		memcpy(filespc, arg, len);
	}
	else
	{
		fputs("? XGREP: Too many arguments\n", stderr);
		return (FALSE);
	}
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
        fputs("? XGREP: Not enough memory available\n", stderr);
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

    pagecheck();
    if (code != 0)
    {
        svcSysErrMsg(code, 3, buffer);	// Get error message
        fprintf(stderr, "\n? XGREP: %s%s%s\n", buffer, (arg[0] != '\0') ?
				"; ": "", arg);			// Output error message
    }
    else
        fprintf(stderr, "\n? XGREP: %s\n", arg);
///	errdone = TRUE;
}


void makepattern(
	char *str,
	int   icase)
{
	PATTERN *ppnt;
	char     chr;

	patsize = strlen(str);
	ppnt = pattern = (PATTERN *)getmem(patsize * sizeof(PATTERN));
	memset(pattern, 0, patsize * sizeof(PATTERN));
	while ((chr = *str++) != 0)
	{
		ppnt->val[chr] = MATCH;
		if (icase)
		{
			if (islower(chr))
				ppnt->val[chr - 0x20] = MATCH;
			else if (isupper(chr))
				ppnt->val[chr + 0x20] = MATCH;
		}
		ppnt++;
	}
}


int usepattern(
	char *str)
{
	uchar   *spnt;
	PATTERN *ppnt;
	int      pcnt;
	uchar    chr;

	while ((chr = *str++) != 0 && chr != '\n')
	{
		if (pattern[0].val[chr])
		{
			pcnt = patsize - 1;
			spnt = str;
			ppnt = pattern + 1;
			while ((chr = *spnt++) != 0 && chr != '\n' && ppnt->val[chr] &&
					--pcnt >= 0)
				ppnt++;
			if (pcnt <= 0)
				return (TRUE);
		}
	}
	return (FALSE);
}
