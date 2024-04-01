//--------------------------------------------------------------------------*
// MKDIR.C
//
// Written by: John R. Goltz
//
// Edit History:
// 01/07/90(brn) - Fixed old use of /
// 08/20/92(brn) - Change reference to global.h from local to library
// 04/25/94(brn) - Make success return 0
// 05/12/94(brn) - Fix command abbreviations
// 04/03/95(sao) - Added progasst package
// 05/14/95(sao) - Changed example,  added mute option
// 05/16/95(sao) - Changed exit codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrtn.h>
#include <progarg.h>
#include <global.h>

#define VERSION 4
#define EDITNO  0

PROGINFO pib;
char  copymsg[] = "";
char  example[] = "{/option} dir-name";
char  description[] = "This command creates the specified directory.  " \
    "The directory to be created must not already exist, but any parent " \
    "directories or device names specified in the path must exist.";
char  prgname[] = "MKDIR";
char  envname[] = "^XOS^MKDIR^OPT";

int  namecount = 0;

int  ignore_str(void);
int  nonopt(char *);
long quiet = FALSE;
long mute = FALSE;

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{	{"Q*UIET"  , ASF_BOOL|ASF_STORE, NULL, AF(&quiet) , 1, "Suppress all but error messages."},
///	{"NOQ*UIET", ASF_BOOL|ASF_STORE, NULL, AF(&quiet) , 0, "Do not suppress all but error messages."},
    {"M*UTE"   , ASF_BOOL|ASF_STORE, NULL, AF(&mute)  , 1, "Suppress all messages."},
///	{"NOM*UTE" , ASF_BOOL|ASF_STORE, NULL, AF(&mute)  , 0, "Do Not suppress all messages."},
	{"?"       , 0                 , NULL, AF(opthelp), 0, "Display this message."},
	{"H*ELP"   , 0                 , NULL, AF(opthelp), 0, "Display this message."},
	{NULL}
};

struct							// Paramters for device information
{	BYTE4PARM  options;
	LNGSTRPARM spec;
	char       end;
} parmopn =
{	{(PAR_SET|REP_HEXV), 4, IOPAR_FILEOPTN, XFO_VOLUME|XFO_PATH|XFO_FILE},
	{(PAR_GET|REP_STR),  0, IOPAR_FILESPEC, NULL, FILESPCSIZE}
};

char rtnname[FILESPCSIZE+4] = "";


void main(
	int   argc,
	char *argv[])
{
    char *foo[2];

	reg_pib(&pib);

	// set Program Information Block variables

	pib.opttbl = options; 		// Load the option table
    pib.kwdtbl = NULL;
	pib.build = __DATE__;
	pib.majedt = VERSION; 		// Major edit number
	pib.minedt = EDITNO; 		// Minor edit number
	pib.copymsg = copymsg;
	pib.prgname = prgname;
	pib.desc = description;
	pib.example = example;
	pib.errno = 0;
	getTrmParms();
	getHelpClr();

    global_parameter(TRUE);
    if (gcp_dosquirk)
		quiet = TRUE;

    if (svcSysGetEnv(envname, (char const **)&foo[0]) > 0)
    {
        foo[1] = '\0';
        progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

	// Process command-line parameters

    if (argc > 1)
    {
        ++argv;
        progarg(argv, 0, options, NULL, (int (*)(char *))ignore_str,
                    (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
        progarg(argv, 0, NULL, NULL, nonopt, (void (*)(char *, char *))NULL,
                    (int (*)(void))NULL, NULL);
    }
    if (namecount == 0)
    {
		if ( !mute )
        fprintf(stderr, "? %s: No directory name specified\n", prgname);
        exit(EXIT_INVSWT);
    }
    exit(EXIT_NORM);					// No errors
}


int ignore_str(void)
{
    return (TRUE);
}


int nonopt(
    char *arg)
{
    char *pnt1;
    long  rtn;
    char  namebfr[512];
    char  chr;

    ++namecount;						// Increase number of names found

    pnt1 = namebfr;						// Copy name over and append trailing
    do									//   slash if needed
    {
		chr = *pnt1++ = *arg++;
    } while (*arg != '\0');

    if (chr != '\\')					// Trailing slash found?
		*pnt1++ = '\\';					// No - add one!
    *pnt1 = '\0';						// Terminate string properly

    parmopn.spec.buffer = (char *)rtnname;
    if (gcp_dosdrive)
        parmopn.options.value = XFO_DOSDEV|XFO_PATH|XFO_FILE;
	if ((rtn = svcIoDevParm(XO_CREATE|XO_ODFS|XO_FAILEX, namebfr,
			(char *)&parmopn)) < 0)
	{
		svcSysErrMsg(rtn, 3, rtnname);	// Get error message string
		if ( !mute )
			fprintf(stderr, "? %s: Error creating directory %s\n"
					"         %s\n", prgname, namebfr, rtnname);
    }
    else if (!quiet && !mute)
        printf("%s: Directory %s created\n", prgname, rtnname);
    return (TRUE);
}
