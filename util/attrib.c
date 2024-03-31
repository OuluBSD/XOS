//--------------------------------------------------------------------------*
// ATTRIB.C - File attribute modification utility for XOS
//
// Written by: Tom Goltz
//
// Edit History:
// 04/15/91(tmg) - Created initial version
// 08/20/92(brn) - Change reference to global.h from local to library
// 05/12/94(brn) - Fixed command abbreviations
// 04/04/95(sao) - Added progasst package
// 05/13/95(sao) - Changed 'optional' indicators, added MUTE
// 05/16/95(sao) - Changed return codes to reflect XOSRTN
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
//  5Jun95 (fpj) - Re-ordered option table.
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

/// THIS NEEDS TO BE UPDATED TO SUPPORT XOS 4.5.0

/// It is actually overkill since this verion of XOS only supports DOS
///   attributes!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xoserr.h>
#include <xosrtn.h>
#include <xos.h>
#include <xossvc.h>
#include <xcmalloc.h>
#include <heapsort.h>
#include <progarg.h>
#include <proghelp.h>
#include <dirscan.h>
#include <global.h>
#include <xoserrmsg.h>
#include <xosstr.h>

// Local defines

#define VERSION 3
#define EDITNO  5

#define REPLACE 1
#define ADD     2
#define REMOVE  3

int  comp(struct filespec *one, struct filespec *two);

void dserrormsg(cchar *, long);
int  nonopt(char *arg);
int  optattrib(arg_data *arg);
int  optowner(arg_data *arg);
int  optprot(arg_data *arg);
int  procfile(void);


long  sorted = TRUE;        // Sort file list if TRUE
long  quiet = FALSE;        // No output unless error if TRUE
long  mute  = FALSE;		// Suppress all messages
long  confirm = FALSE;      // Confirm before changing attributes if TRUE

arg_spec options[] =
{   {"S*YSTEM"   , ASF_MINUS           , NULL,    optattrib,~XA_SYSTEM,"Modify (+=ON/-=OFF) the file's SYSTEM attribute."},
    {"S*YSTEM"   , ASF_PLUS            , NULL,    optattrib, XA_SYSTEM,NULL},
    {"A*RCHIVE"  , ASF_MINUS           , NULL,    optattrib,~XA_ARCH,"Modify the file's ARCHIVE attribute."},
    {"A*RCHIVE"  , ASF_PLUS            , NULL,    optattrib, XA_ARCH,NULL},
    {"H*IDDEN"   , ASF_MINUS           , NULL,    optattrib,~XA_HIDDEN,"Modify the file's HIDDEN attribute."},
    {"H*IDDEN"   , ASF_PLUS            , NULL,    optattrib, XA_HIDDEN,NULL},
    {"R*EADONLY" , ASF_MINUS           , NULL,    optattrib,~XA_RDONLY,"Modify the file's READONLY attribute."},
    {"R*EADONLY" , ASF_PLUS            , NULL,    optattrib, XA_RDONLY,NULL},
    {"P*ROTECT"  , ASF_VALREQ|ASF_LSVAL, NULL,    optprot  , 0, "Set the file protection levels."},
    {"O*WNER"    , ASF_VALREQ|ASF_LSVAL, NULL,    optowner , 0, "Set the file owner value."},
    {"SO*RT"     , ASF_BOOL|ASF_STORE  , NULL, AF(&sorted) , TRUE,"Sort files before taking action."},
    {"C*ONFIRM"  , ASF_BOOL|ASF_STORE  , NULL, AF(&confirm), TRUE,"Ask for confirmation before taking actioin."},
    {"Q*UIET"    , ASF_BOOL|ASF_STORE  , NULL, AF(&quiet)  , TRUE,"Suppress all but error messages."},
    {"M*UTE"     , ASF_BOOL|ASF_STORE  , NULL, AF(&mute)   , TRUE,"Suppress all messages."},
    {"H*ELP"     , 0                   , NULL, AF(opthelp) , 0, "This message."},
    {"?"         , 0                   , NULL, AF(opthelp) , 0, "This message."},
    {NULL}
};

char user[36];			// Specified user name
char ownername[36];		// Owner information from file

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE2PARM  srcattr;
    BYTE2PARM  filattr;
    BYTE4PARM  prot;
    LNGSTRPARM owner;
    char       end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILEOPTN, XFO_XOSDEV|XFO_PATH|XFO_FILE},
    {PAR_GET|REP_STR , 0 , IOPAR_FILESPEC, NULL, FILESPCSIZE+10, 0},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR , XA_FILE|XA_DIRECT|XA_HIDDEN|XA_SYSTEM},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILEATTR, 0},
    {PAR_GET|REP_HEXV, 4 , IOPAR_PROT    , 0},
    {PAR_GET|REP_STR , 0 , IOPAR_OWNER   , ownername, 36, 36}
};

DIRSCANDATA dsd =
{   (DIRSCANPL *)&fileparm,	// parmlist - Address of parameter list
    (PROCFILE *)procfile,	// func     - Function called for each match
    dserrormsg,				// error    - Function called on error
    DSSORT_ASCEN			// sort     - Directory sort order
};

struct protdata
{   uchar spc;
    uchar new;
    uchar val;
    uchar add;
    uchar rmv;
};

struct protdata network = {FALSE, FALSE, 0, 0, 0xFF};
struct protdata world   = {FALSE, FALSE, 0, 0, 0xFF};
struct protdata group   = {FALSE, FALSE, 0, 0, 0xFF};
struct protdata owner   = {FALSE, FALSE, 0, 0, 0xFF};

char  modify;				// Modify attributes if TRUE
char  protected;			// TRUE if have protected file system
char  firsttime;
int   ownerlen;
int   pagewidth  = 80;		// Width of display in columns
int   pageremain = 80;		// Remaining space on current line

// Misc. variables

Prog_Info pib;
char  copymsg[] = "";
char  example[] = "{+|-options} {/options} filelist";
char  description[] = "ATTRIB is a DOS-style command with some XOS " \
		"extensions.  It is used to look at or change any of the file " \
		"attributes which are part of the directory entry for each file.  " \
		"Files may be specified using wildcard or ellipsis specifications.";
char  prgname[] = "ATTRIB";
char  envname[] = "^XOS^ATTRIB^OPT";

int   setattribute =  0;	// File attributes to set
int   clrattribute = -1;	// File attributes to clear

long  grandtotal = 0;		// Total number of files processed
int   nameseen;				// We saw a file name on the cmd line

struct filespec
{
    struct filespec *next;	// Must be first item in structure
    struct filespec *sort;	// For heapsort
    long   error;			// Error associated with file
    long   protect;			// File protection value
    char   setattrib;		// Attributes to set
    char   clrattrib;		// Attributes to clear
    char   attributes;		// Current file attributes
    char   owner[32];
    char   filespec[1];		// File specification
};

struct
{   TEXT4CHAR  protect;
    char       end;
} protectchar =
{  {PAR_GET|REP_TEXT, 4 , "PROTECT", 0}
};

struct filespec *firstfile;	// Pointer to first file to process
struct filespec *lastfile;	// Pointer to last file to process

struct
{   BYTE4PARM  options;
    LNGSTRPARM spec;
    char       end;
} openparm =
{
    {(PAR_SET|REP_HEXV), 4, IOPAR_FILEOPTN, XFO_XOSDEV|XFO_PATH},
    {(PAR_GET|REP_STR),  0, IOPAR_FILESPEC, NULL, FILESPCSIZE}
};


//**********************************
// Function: main - Main program
// Returned: 0 if normal, 1 if error
//**********************************

int main(
    int argc,
    char *argv[])
{
    char  *envargs[2];
    char   filspec[FILESPCSIZE+10];

	reg_pib(&pib);

	// Set Program Information Block variables

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

    if (argc < 2)
	{									// Have any arguments?
		if ( !mute )
			fprintf(stderr,"? ERROR %s:  NO PARAMETERS SPECIFIED\n",prgname);
		exit(EXIT_INVSWT);
	}

    ++argv;
    progarg(argv, PAF_PSWITCH, options, NULL, nonopt,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

	if (mute)
		quiet=TRUE;

    if (!nameseen)			// Must have something specified
	{
		if (!mute)
			fprintf(stderr,"? ERROR %s:  NO FILESPEC PROVIDED\n",prgname);
		exit(EXIT_INVSWT);
	}

    return (EXIT_NORM);
}

//************************************************
// Function: nonopt - Process a file specification
// Returned: TRUE if OK, false if error
//************************************************

int nonopt(
    char *arg)

{
    struct filespec *thisfile;
    union
    {   BYTE1PARM  *n1;
        BYTE4PARM  *n4;
        LNGSTRPARM *s;
        char       *e;
    } setpnt;
    union
    {   long l;
        char s[4];
    } letters;
    char   setparms[40];
    ulong  protection;
    ulong  newprot;
    long   newattrib;
    long   rtn;
    char   newowner;
    char   attributes;
    char   value;

    firstfile = lastfile = NULL;
    nameseen = TRUE;
    firsttime = TRUE;
    ownerlen = 1;
    dirscan(arg, &dsd, 0);				// Scan the directory
    if (firstfile == NULL)
    {
        if ( !mute )
			fprintf(stdout, "? ATTRIB: File not found, %s\n", arg);
        exit(EXIT_FNF);
    }
    if (sorted)
        firstfile = heapsort(firstfile,
                (int (*)(void *a, void *b, void *d))comp, NULL);
    thisfile = firstfile;
    do
    {
        attributes = thisfile->attributes;
        protection = thisfile->protect;

        newattrib = (attributes | setattribute) & clrattribute;

        newprot = protection;
        if (network.spc)
        {
            value = (network.new)? network.val: (protection/FP_NETWORK);
            value |= network.add;
            value &= network.rmv;
            newprot &= ~(0xFF * (ulong)FP_NETWORK);
            newprot |= value * (ulong)FP_NETWORK;
        }
        if (world.spc)
        {
            value = (world.new)? world.val: (protection/FP_WORLD);
            value |= world.add;
            value &= world.rmv;
            newprot &= ~(0xFF * (ulong)FP_WORLD);
            newprot |= value * (ulong)FP_WORLD;
        }
        if (group.spc)
        {
            value = (group.new)? group.val: (protection/FP_GROUP);
            value |= group.add;
            value &= group.rmv;
            newprot &= ~(0xFF * (ulong)FP_GROUP);
            newprot |= value * (ulong)FP_GROUP;
        }
        if (owner.spc)
        {
            value = (owner.new)? owner.val: (protection/FP_OWNER);
            value |= owner.add;
            value &= owner.rmv;
            newprot &= ~(0xFF * (ulong)FP_OWNER);
            newprot |= value * (ulong)FP_OWNER;
        }

        newowner = ((user[0] != 0) && (strcmp(user, thisfile->owner) != 0));

        if ((attributes != newattrib) || (protection != newprot) || newowner)
        {
            setpnt.e = setparms;
            setpnt.n1->desp = PAR_SET|REP_HEXV;
            setpnt.n1->size = 1;
            setpnt.n1->index = IOPAR_SRCATTR;
            setpnt.n1->value = (char)(XA_FILE|XA_DIRECT|XA_HIDDEN|XA_SYSTEM);
            setpnt.n1++;
            if (attributes != newattrib)
            {
                setpnt.n1->desp = PAR_SET|REP_HEXV;
                setpnt.n1->size = 1;
                setpnt.n1->index = IOPAR_FILEATTR;
                setpnt.n1->value = (char)newattrib;
                setpnt.n1++;
            }
            if (newprot != protection)
            {
                setpnt.n4->desp = PAR_SET|PAR_GET|REP_HEXV;
                setpnt.n4->size = 4;
                setpnt.n4->index = IOPAR_PROT;
                setpnt.n4->value = newprot;
                setpnt.n4++;
            }
            if (newowner)
            {
                setpnt.s->desp = PAR_SET|PAR_GET|REP_STR;
                setpnt.s->size = 0;
                setpnt.s->index = IOPAR_OWNER;
                setpnt.s->buffer = thisfile->owner;
                setpnt.s->bfrlen = 36;
                setpnt.s->strlen = 36;
                setpnt.s++;
            }
            *setpnt.e = 0;
            if ((rtn = svcIoDevParm(0, thisfile->filespec, setparms)) < 0)
                femsg2(prgname, "Error changing file attributes", rtn,
                        thisfile->filespec);
            attributes = newattrib;
//            protection = newprot;
        }

        if (confirm || (!quiet && !mute) )
        {
            if (protected)
            {
                letters.l = (attributes & XA_DIRECT)? 'C?SA': 'WERX';
                printf("  %c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c "
                    "%-*.*s  %s\n", (attributes & XA_ARCH)? 'A': '-',
                    (attributes & XA_HIDDEN)? 'H': '-',
                    (attributes & XA_SYSTEM)? 'S': '-',
                    (protection & (FP_NETWORK*FP_EXEC))? letters.s[0]: '-',
                    (protection & (FP_NETWORK*FP_READ))? letters.s[1]: '-',
                    (protection & (FP_NETWORK*FP_EXTEND))? letters.s[2]: '-',
                    (protection & (FP_NETWORK*FP_WRITE))? letters.s[3]: '-',
                    (protection & (FP_NETWORK*FP_MODIFY))? 'M': '-',
                    (protection & (FP_WORLD*FP_EXEC))? letters.s[0]: '-',
                    (protection & (FP_WORLD*FP_READ))? letters.s[1]: '-',
                    (protection & (FP_WORLD*FP_EXTEND))? letters.s[2]: '-',
                    (protection & (FP_WORLD*FP_WRITE))? letters.s[3]: '-',
                    (protection & (FP_WORLD*FP_MODIFY))? 'M': '-',
                    (protection & (FP_GROUP*FP_EXEC))? letters.s[0]: '-',
                    (protection & (FP_GROUP*FP_READ))? letters.s[1]: '-',
                    (protection & (FP_GROUP*FP_EXTEND))? letters.s[2]: '-',
                    (protection & (FP_GROUP*FP_WRITE))? letters.s[3]: '-',
                    (protection & (FP_GROUP*FP_MODIFY))? 'M': '-',
                    (protection & (FP_OWNER*FP_EXEC))? letters.s[0]: '-',
                    (protection & (FP_OWNER*FP_READ))? letters.s[1]: '-',
                    (protection & (FP_OWNER*FP_EXTEND))? letters.s[2]: '-',
                    (protection & (FP_OWNER*FP_WRITE))? letters.s[3]: '-',
                    (protection & (FP_OWNER*FP_MODIFY))? 'M': '-',
                    ownerlen, ownerlen, thisfile->owner, thisfile->filespec);
            }
            else
                printf("  %c%c%c%c  %s\n", (attributes & XA_ARCH)? 'A': '-',
                        (attributes & XA_HIDDEN)? 'H': '-',
                        (attributes & XA_RDONLY)? 'R': '-',
                        (attributes & XA_SYSTEM)? 'S': '-', thisfile->filespec);
        }

    } while ((thisfile = thisfile->next) != NULL);

    return (TRUE);
}


//************************************************
// Function: optattrib - Process attribute options
// Returned: TRUE always
//************************************************

int optattrib(
    arg_data *arg)
{
    if (arg->data > 0)
    {
        setattribute |= arg->data;
        clrattribute |= arg->data;
    }
    else
    {
        setattribute &= arg->data;
        clrattribute &= arg->data;
    }
    modify = TRUE;
    return (TRUE);
}

//*********************************************************************
// Function: optprot - Process options which set file protection values
// Returned: TRUE always
//*********************************************************************

// Protecton values are specified as X:ABC, where X is N (network), W (world),
//   G (group), or O (owner) to specify the protection field, and A, B, or C
//   specifies the access allowed as follows:
//     File access Letter  Directory access Letter
//     None          N     None               N
//     Execute       X     Access file        A
//     Read          R     Search             S
//     Extend        E
//     Write         W     Create file        C
//     Modify attr   M     Modify attr        M
//   A letter can be preceeded with - to remove the access right or + to add
//   the access right to the current rights.  If no - or + preceeds the
//   specifications, the current rights are overwritten.  Multiple fields can
//   be specified in one option value with or without seperating spaces.  There
//   can be no spaces within the specification of a field.

int optprot(
    arg_data *arg)
{
    char  *pnt;
    struct protdata *protpnt;
    char   chr;
    char   func;
    char   value;

    pnt = arg->val.s;
    while ((chr = toupper(*pnt++)) != 0)
    {
        switch (chr)
        {
         case 'N':
            protpnt = &network;
            network.spc = TRUE;
            network.new = FALSE;
            network.val = 0;
            network.add = 0;
            network.rmv = 0xFF;
            break;

         case 'W':
            protpnt = &world;
            world.spc = TRUE;
            world.new = FALSE;
            world.val = 0;
            world.add = 0;
            world.rmv = 0xFF;
            break;

         case 'G':
            protpnt = &group;
            group.spc = TRUE;
            group.new = FALSE;
            group.val = 0;
            group.add = 0;
            group.rmv = 0xFF;
            break;

         case 'O':
            protpnt = &owner;
            owner.spc = TRUE;
            owner.new = FALSE;
            owner.val = 0;
            owner.add = 0;
            owner.rmv = 0xFF;
            break;

         default:
            if (!mute)
				fprintf(stderr, "? ATTRIB: Illegal protection field specification"
                    " %c\n          Valid values are N (network), W (world),"
                    " G (group), or O (owner)\n", chr);
            exit(EXIT_INVSWT);
        }
        if (*pnt++ != ':')		// Must have colon next
        {
            if (!mute)
				fprintf(stderr, "? ATTRIB: Protection field specification %c not"
                    " followed by colon\n", chr);
            exit(EXIT_INVSWT);
        }
        if (*pnt == 0)
        {
            if (!mute)
				fprintf(stderr, "? ATTRIB: No access specified for protection"
                    " field %c\n", chr);
            exit(EXIT_INVSWT);
        }

        func = REPLACE;
        while ((chr = toupper(*pnt)) != 0 && pnt[1] != ':')
        {
            pnt++;
            switch (chr)
            {
             case '+':
                func = ADD;
                continue;

             case '-':
                func = REMOVE;
                continue;

             case 'N':
                value = 0;
                break;

             case 'X':
             case 'A':
                value = FP_EXEC;
                break;

             case 'R':
             case 'S':
                value = FP_READ;
                break;

             case 'E':
                value = FP_EXTEND;
                break;

             case 'W':
             case 'C':
                value = FP_WRITE;
                break;

             case 'M':
                value = FP_MODIFY;
                break;
            }
            switch (func)
            {
             case REPLACE:
                protpnt->new = TRUE;
                protpnt->val = value;
                func = ADD;
                break;

             case ADD:
                protpnt->add |= value;
                protpnt->rmv |= value;
                break;

             case REMOVE:
                protpnt->add &= ~value;
                protpnt->rmv &= ~value;
            }
        }
    }
    return (TRUE);
}

//******************************************
// Function: optowner - Process OWNER option
// Returned: TRUE always
//******************************************

int optowner(
    arg_data *arg)
{
    strnmov(user, arg->val.s, 32);
    return (TRUE);
}


//**************************************************************
// Function: procfile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

int procfile(void)
{
    char  *pnt;
    struct filespec *thisfile;
    int    left;
    char   buffer[1024];

    if (dsd.error < 0 &&
            (dsd.error != ER_FILAD && dsd.error != ER_FBFER &&
            dsd.error != ER_FBFER && dsd.error != ER_FBWER &&
            dsd.error != ER_BUSY))
    {
        dserrormsg(buffer, dsd.error);
        return (FALSE);
    }
    if (firsttime)
    {
        firsttime = FALSE;
        protected = FALSE;
        if (svcIoDevChar(fileparm.dirhndl.value, (char *)&protectchar) >= 0)
            protected = (protectchar.protect.value[0] == 'Y');
    }
    if (dsd.filenamelen < 0)		// Finished here if no name
        return (TRUE);

    thisfile = (struct filespec *)getspace(sizeof(struct filespec) +
            strlen(buffer) + 1);
    strmov(thisfile->filespec, buffer);
    thisfile->setattrib = setattribute;
    thisfile->clrattrib = clrattribute;
    thisfile->error = dsd.error;
    thisfile->attributes = dsd.attr;
    thisfile->protect = fileparm.prot.value;
    strnmov(thisfile->owner, ownername, 32);
    if (fileparm.owner.strlen > ownerlen)
        ownerlen = fileparm.owner.strlen;
    if (firstfile == NULL)
        firstfile = thisfile;
    else
        lastfile->next = thisfile;
    lastfile = thisfile;
    thisfile->next = NULL;
    return (TRUE);
}

//****************************************************
// Function: comp - Compare two filenames for heapsort
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//****************************************************

int comp(
    struct filespec *one,
    struct filespec *two)

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

//*******************************************
// Function: dserrormsg - Display error message
// Returned: Nothing
//*******************************************

void dserrormsg(
	cchar *arg,
	long   code)
{
    char buffer[80];		// Buffer to receive error message

    svcSysErrMsg(code, 3, buffer); // Get error message
    if (!mute)
		fprintf(stderr, "\n? %s: %s", prgname, buffer);
    if (arg != NULL && *arg != '\0')	// Have returned name?
		if (!mute)
		{
			fprintf(stderr, ", file %s", arg); // Yes - output it too
    		fputc('\n', stderr);
		}
}

