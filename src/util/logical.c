//--------------------------------------------------------------------------*
// Logical.c
// Command to set or display logical names
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment block at start of file
// 05/08/94(brn) - Document new options in the help command
// 04/02/95(sao) - added progasst package
// 05/14/95(sao) - chaged description, added mute option
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

// Command format:
//	LOGICAL {-l{evel}=n} {-o{nly}}
//		Displays all logical names
//   or
//	LOGICAL devname: {-l{evel}=n} {-o{nly}}
//		Displays specified logical names, wild cards are valid
//   or
//	LOGICAL devname: = {-l{evel}=n} {-q{uiet}}
//		Deletes a logical name
//   or
//	LOGICAL devname: = definition {-l{evel}=n} {-q{uiet}}
//		Defines a logical name

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrtn.h>
#include <xoserr.h>
#include <progarg.h>
#include <xoserrmsg.h>
#include <xosstr.h>

#define VERSION 3
#define EDITNO  3

long  loglevel;		// Level
long  subst;		// 0x10000000 if defining a substituted name
long  setbits;
long  clrbits;
char *defpnt;		// Pointer to definition string
char  havevalue;
char  kwbfr[34];	// Keyword buffer
long  editflag = FALSE; // Not in edit mode
long  renflag  = FALSE; // Not renaming a variable
long  quiet=FALSE;        // TRUE if no output wanted
long  mute=FALSE;         // TRUE if no output wanted
char  haveeq;		// TRUE if = specified

void asserror(char *, char *, long);
void display(char *);
//int  edithave(arg_data *);
void editlogical(char *);
void getname(char *bfr);
int  levelhave(arg_data *);
//int  helphave(void);
//void help_print(char *help_string, int state, int newline);
int  namehave(arg_data *);
int  onlyhave(arg_data *);
//int  quiethave(arg_data *);
void renamelogical(char *, char *);
//int  renhave(arg_data *);
int  sethave(arg_data *);
int  clrhave(arg_data *);
void printit(char *name, int namemax, char *def, int bits, int level);

arg_spec keywords[] =
{   {"*",  ASF_LSVAL, NULL, namehave, 0, NULL},
    {NULL, 0        , NULL, NULL    , 0, NULL}
};

arg_spec options[] =
{   {"E*DIT"   , ASF_BOOL|ASF_STORE, NULL, AF(&editflag), TRUE            , "Edit the selected logical name." },
    {"RE*NAME" , ASF_BOOL|ASF_STORE, NULL, AF(&renflag) , TRUE            , "Rename the selected logical name."},
    {"SU*BST"  , ASF_NEGATE        , NULL,    sethave   , LGNMB_SUBSTD<<24, "Make a substitute logical name."},
    {"RO*OTED" , ASF_NEGATE        , NULL,    sethave   , LGNMB_ROOTED<<24, "Make a rooted logical name."},
    {"SY*STEM" , 0                 , NULL,    levelhave , 0xFF            , "Operate on a system level logical name."},
    {"SE*SSION", 0                 , NULL,    levelhave , 0               , "Operate on a session level logical name."},
    {"Q*UIET"  , ASF_BOOL|ASF_STORE, NULL, AF(&quiet)   , TRUE            , "Suppress all but error messages."},
    {"M*UTE"   , ASF_BOOL|ASF_STORE, NULL, AF(&mute)    , TRUE            , "Suppress all but error messages."},
    {"H*ELP"   , 0                 , NULL, AF(opthelp)  , 0               , "This message."},
    {"?"       , 0                 , NULL, AF(opthelp)  , 0               , "This message."},
    {NULL}
};


PROGINFO pib;
char  copymsg[] = "";
char  prgname[] = "LOGICAL";
char  envname[] = "^XOS^LOGICAL^OPT";
char  example[] = "{/option} name{={definition}}";
char  description[] = "This command provides the means for controlling " \
    "logical device names.  XOS provides a complete system of logical " \
    "names which are used for many system functions.  See chapter 2, page " \
    "14 of the 'XOS Command Reference Manual' for a complete " \
    "discussion of logical names.  If no name is specified, all logical " \
    "names at the indicated level (SESISON or SYSTEM) are displayed.  If " \
    "a logical name is specified without a value (no '=' after the name), " \
    "the current value of the logical name is displayed.  If a logical " \
    "name is specified with a value, the value is changed.  If a null " \
    "value is specified (an '=' after the name but no value), the logical " \
    "name is deleted.  If a logical name is to be defined as a \"search " \
    "list\" logical, the names in the list are seperated with commas.  " \
    "The entire list must be enclosed in double quotes.";

int main(
    int   argc,
    char *argv[])
{
    long  rtn;
    char *foo[2];

    argc = argc;

	reg_pib(&pib);

	// set Program Information Block variables

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

    if (svcSysGetEnv(envname, (char const **)&foo[0]) > 0)
    {
		foo[1] = '\0';
    	progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
                (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    setbits = clrbits = 0;
    ++argv;
    progarg(argv, PAF_EATQUOTE, options, keywords, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

	if (mute == TRUE)					// Resolve conflicts between quiet and
		quiet=TRUE;						//   mute

    // Here with all arguments processed - now do what we need to do

    if (!haveeq || editflag)			// Have = ?
    {									// No
        if (renflag)
        {
			if (!mute)
            	fputs("? LOGICAL: Two names not specified for rename\n",
						stderr);
            exit(EXIT_INVSWT);
        }
		if (editflag)					// Do we edit an existing string?
        {
            if (defpnt != NULL)
            {
				if (!mute)
					fputs("LOGICAL: Warning - new definition ignored\n",
							stderr);
                defpnt = NULL;
            }
			editlogical(kwbfr);
        }
		else
	        display((kwbfr[0]) ? kwbfr : "*:"); // No - display something
    }
	else if (renflag)					// Rename an existing name?
		renamelogical(kwbfr, defpnt);
    else if ((rtn=svcLogDefine(loglevel | subst, kwbfr, defpnt)) < 0)
        asserror((defpnt==NULL)?"deleting logical name":
                "defining logical name", kwbfr, rtn);
    if (!quiet)
    {
        if (defpnt == NULL)
            printf("Logical name \"%s\" deleted at ", kwbfr);
        else
            printf("Logical name \"%s\" defined as \"%s\" at ", kwbfr, defpnt);
		if (loglevel == 0xFF)
			fputs("system level\n", stdout);
		else
			printf("session level %d\n", loglevel);
    }
    return (0);
}


int namehave(
    arg_data *arg)

{
    if (kwbfr[0])
    {
		if (!mute)
			fputs("? LOGICAL: More than one logical name specified\n", stderr);
        exit(EXIT_INVSWT);
    }
    haveeq = ((arg->flags & ADF_NONE) == 0);
    strcpy(kwbfr, arg->name);
	if (haveeq)
    	defpnt = arg->val.s;
	else
		defpnt = NULL;
    return (TRUE);
}


// Function to process /SUBST and /ROOTED switches

int sethave(
    arg_data *arg)
{
	if (arg->flags & ASF_NEGATE)
		clrhave(arg);
	else
	{
		subst |= arg->data;
		setbits |= arg->data;
		clrbits &= ~(arg->data);
	}
	return (TRUE);
}


// Function to process /NOSUBST and /NOROOTED switches

int clrhave(
    arg_data *arg)

{
    subst &= !(arg->data);
    clrbits |= arg->data;
    setbits &= ~(arg->data);
    return (TRUE);
}


// Function to process /Session and /System switches


// Function to edit existing logical device names
int  levelhave(arg_data *arg)
{
    loglevel = (strncmp(arg->name, "SE", 2) == 0) ? 0 : 0xFF;
    return 0;
}

void editlogical(
    char *name)

{
	char buffer[512];
	char namebfr[512];
///	char device[12];
	long rtn;

	if (name == NULL)
	{
		if (!mute)
			fputs("? LOGICAL: No name specified\n", stderr);
		exit(EXIT_INVSWT);
    }
    if ((rtn = svcLogGet(loglevel, name, buffer, 512)) < 0)
		asserror("searching for name", name, rtn);

//	printf("#### rtn = %08.8X, set = %08.8X, clr = %08.8X\n", rtn, setbits, clrbits);

    subst = ((rtn & 0xFFFF0000) | setbits) & ~clrbits;
    if (renflag)						// Rename variable with edit?
    {
		if (!quiet)
			fprintf(stdtrm, "LOGICAL: Rename %s to ", name);
		if ((rtn = svcTrmWrtInB(STDTRM, name, strlen(name))) < 0)
			femsg(prgname, rtn, name);
		getname(namebfr);				// Get revised string
		renamelogical(name, namebfr);
    }
	if (!quiet)
		fprintf(stdtrm, "LOGICAL: %s = ", name);
    if ((rtn = svcTrmWrtInB(STDTRM, buffer, strlen(buffer))) < 0)
		femsg(prgname, rtn, name);
    getname(buffer);					// Get revised string
    if ((rtn=svcLogDefine(loglevel | subst, name, buffer)) < 0)
		asserror("defining logical name", name, rtn);
    if (!quiet)
	{
		if (loglevel == 0xFF)
			strmov(namebfr, "system level");
		else
			sprintf(namebfr, "session level %d", loglevel);
        printf("Logical name \"%s\" defined as \"%s\" at %s\n", name, buffer,
			namebfr);
	}
    exit(EXIT_NORM);
}


// Function to get revised string

void getname(
    char *bfr)
{
    char chr;

    fgets(bfr, 512, stdtrm);			// Get revised string
    while ((chr = *bfr++) != '\0')
    {
        if (chr == '\n')
        {
            bfr[-1] = '\0';
            return;
        }
    }
}


// Function to rename existing logical names

void renamelogical(
    char *oldname,
    char *newname)
{
    long  rtn;
    char  buffer[512];
///    char  device[12];

    if (oldname == NULL || newname == NULL)
    {
		if (!mute)
			fputs("? LOGICAL: Two names not specified for rename\n", stderr);
        exit(EXIT_INVSWT);
    }
    if ((rtn = svcLogGet(loglevel, oldname, buffer, 512)) < 0)
		asserror("searching for name", oldname, rtn);
    strupper(newname);
    if ((rtn=svcLogDefine(loglevel, newname, buffer)) < 0)
		asserror("defining logical name", newname, rtn);
    if ((rtn=svcLogDefine(loglevel, oldname, NULL)) < 0)
		asserror("deleting logical name", oldname, rtn);
	if (!quiet)
		printf("Logical %s has been renamed %s\n",oldname,newname);
    exit(EXIT_NORM);
}


// Function to display logical names

void display(
    char *name)
{
	typedef struct
	{	ushort len;
		uchar  bits;
		uchar  level;
		char   name[16];
		char   def[1];
	} DEF;

	uchar *bufr;
	char  *pnt;
    long   rtn;
    long   length;
	long   left1;
	long   left2;
	long   namemax;
	union
	{	DEF   *d;
		uchar *c;
	}      pnt1;
	union
	{	DEF   *d;
		uchar *c;
	}      pnt2;
	char   chr;
	char   text[260];

	pnt = name;							// Have any wild card characters?
	while ((chr = *pnt++) != 0 && chr != '*')
		;
	if (chr != 0)
	{									// Yes - get all of the definitions
		left1 = 3;
		do
		{
    		if ((rtn = svcLogGetAll(loglevel, NULL, 0)) < 0)
				asserror("determining space needed", "", rtn);
			if ((bufr = (uchar *)malloc(rtn + 200)) == NULL)
				asserror("allocating data buffer", "", -errno);
			if ((length = svcLogGetAll(loglevel, bufr, rtn + 200)) < 0)
				asserror("getting logical name definitions", "", length);
		} while (length > (rtn + 198) && --left1 > 0);
		if (left1 == 0)
		{
			fputs("? LOGICAL: Returned data truncated\n", stderr);
			return;
		}
		if (loglevel == 0xFF)
			strmov(text, "system level");
		else
 			sprintf(text, "session level %d", loglevel);
		if (length < 21)
		{
			printf("No logical names are visible at %s\n", text);
			exit(1);
		}
		namemax = 0;
		pnt1.c = bufr;
		left1 = length;
		while (left1 >= 21)
		{
			if (pnt1.d->len < sizeof(DEF))
			{
				fputs("? LOGICAL: Returned data is invalid\n", stderr);
				exit(1);
			}
			if (pnt1.d->name[0] != 0)
			{
				if ((rtn = strnlen(pnt1.d->name, 16)) > namemax)
					namemax = rtn;
				pnt2.c = pnt1.c + pnt1.d->len;
				left2 = left1 - pnt1.d->len;
				while (left2 > 21)
				{
					if (strncmp(pnt1.d->name, pnt2.d->name, 16) == 0)
					{
						printf("Found dup: %.16s at level %d from level %d\n",
								pnt2.d->name, pnt2.d->level, pnt1.d->level);
						pnt2.d->name[0] = 0;
					}
					left2 -= pnt2.d->len;
					pnt2.c += pnt2.d->len;
				}
			}
			left1 -= pnt1.d->len;
			pnt1.c += pnt1.d->len;
		}
		printf("Logical names visible at %s\n", text);
		pnt1.c = bufr;
		while (length >= 21)
		{
			if (pnt1.d->name[0] != 0)
				printit(pnt1.d->name, namemax, pnt1.d->def, pnt1.d->bits,
						pnt1.d->level);
			length -= pnt1.d->len;
			pnt1.c += pnt1.d->len;
		}
	}
	else								// If should display a single name
	{
	    if ((rtn = svcLogGet(loglevel, name, text, 512)) < 0)
		{
			if (rtn == ER_NTDEF)
			{
				printf("? LOGICAL: Logical name %s is not defined\n", name);
				exit(1);
			}
			else
				femsg2(prgname, "Error getting definition", rtn, NULL);
		}
		printit(name, strlen(name), text, rtn >> 24, (rtn >> 16) & 0xFF);
	}
	exit(0);
}


void printit(
	char *name,
	int   namemax,
	char *def,
	int   bits,
	int   level)
{
	printf("%c%c%c %c %*.16s = %s\n", (bits & LGNMB_ROOTED) ? 'R' : '-',
			(bits & LGNMB_SUBSTD) ? 'S' : '-', (bits & LGNMB_MULT) ? 'M' : '-',
			(level == 0xFF) ? 'S' : (level + '0'), namemax, name, def);
}


// Function to report fatal error

void asserror(
    char *msg,
    char *name,
    long  code)
{
    char buffer[100];

    svcSysErrMsg(code, 3, buffer); // Get error message string
	if (!mute)
		fprintf(stderr, "? LOGICAL: Error %s %s\n"
                    "           %s\n", msg, name, buffer);
    exit(EXIT_SVCERR);
}
