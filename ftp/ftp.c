//--------------------------------------------------------------------------*
// FTP.C
// FTP client for XOS
//
// Written by: John Goltz
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
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <xosnet.h>
#include <xosftpfunc.h>
#include <progarg.h>
#include <proghelp.h>
#include <global.h>
#include <xosstr.h>

#define VERSION 1
#define EDITNO  0

#define NAMEMAX  1024

///typedef struct filedescp FILEDESCP;

typedef struct cmdtbl CMDTBL;

typedef struct _nblk NBLK;

struct _nblk
{	NBLK *next;
	char  name[1];
};

NBLK   *namehead;
NBLK  **nameppnt;

char   *hostaddr;
char   *cmdpnt;

int     trmcnt;
char   *trmpnt;

char    trmbfr[128];
char    atom[64];

time_sz filedt;
time_s  starttime;
time_s  stoptime;
int     totalbytes;
long    cmdhndl = -1;
long    datahndl = -1;
long    filehndl = -1;
int     debug = FALSE;
long    rspnum;
char    rspstr[256];
long    have150;
int     tcpavl;
long    openflag;

char   *typestr = "A N";
char   *tcppnt;

char    cmdbufr[256];
char    xferbufr[1400];
char    lclpath[512] = "";
char    datadev[32];
char    lclname[NAMEMAX];
uchar   prompt = 0x01;
uchar   rspdone;

struct
{   byte4_parm  rmtport;
    byte4_parm  lclport;
    char        end;
} cmdopnparms =
{   {PAR_SET|REP_HEXV, 4, IOPAR_NETRMTPORTS, TCPP_FTPCMD},
    {PAR_GET|REP_HEXV, 4, IOPAR_NETLCLPORT}
};

struct
{   byte4_parm  rmtport;
    byte4_parm  lclport;
    char        end;
} dataopnparms =
{   {PAR_GET|REP_HEXV, 4, IOPAR_NETRMTPORTR},
    {PAR_SET|REP_HEXV, 4, IOPAR_NETLCLPORT}
};

int     askuser(char *msg);
long    cmdappend(char *arg);
long    cmddebug(char *arg);
long    cmddir(char *arg);
long    cmdget(char *arg);
long    cmdhelp(char *arg);
long    cmdlcd(char *arg);
long    cmdls(char *arg);
long    cmdmget(char *arg);
long    cmdmput(char *arg);
long    cmdopen(char *arg);
long    cmdprompt(char *arg);
long    cmdput(char *arg);
long    cmdquit(char *arg);
long    cmdquote(char *arg);
long    cmdrhelp(char *arg);
long    cmdren(char *arg);
long    cmduser(char *arg);
int     datatofile(char *buffer, int size);
int     datatouser(char *buffer, int size);
void    errormsg(char *msg, int code);
CMDTBL *getcmd(void);
char   *getname(char **str);
long    getxfer(char *bufr, int size);
long    listxfer(char *bufr, int size);
int     nonopt(char *arg);
void    notify(char *str);
void    opendone(void);
void    opthelp(void);
long    putxfer(char *bufr, int size);
int     setuplocal(char *name);
long    trmxfer(char *bufr, int size);


#define OPT(name) ((int (*)(arg_data *))name)

arg_spec options[] =
{	{"DEB*UG", ASF_BOOL|ASF_STORE, NULL, &debug , 1, "Run in debug mode"},
	{"H*ELP" , 0                 , NULL, opthelp, 0, "Display this message"},
    {"?"     , 0                 , NULL, opthelp, 0, "Display this message"},
    {NULL}
};

// FTP command table

struct cmdtbl
{   char  *name;
    long (*func)(char *arg);
    char  *help;
};

char helptext[] =
		"Valid commands are:\n\n"
		"   ?      ALLFILES APPEND ASCII      BINARY BYE    CD   CLOSE\n"
		"   DELETE DEBUG    DIR    DISCONNECT GET    HELP   LCD  LITERAL\n"
		"   LS     MGET     MKDIR  MPUT       OPEN   PROMPT PUT  PWD\n"
		"   QUIT   USER     RECV   REMOTEHELP RENAME RMDIR  SEND STATUS\n"
		"   TYPE\n\n"
		"For a description of a command, type \"HELP cmdname\"\n\n";

char msgallfls[] =
		"Toggle all-files mode (XOS specific command, user must have\r\n"
		"              the FTPALL privilege available)";

char msgdir[] = "Display directory listing";
char msgget[] = "Transfer file from the remote system";
char msgput[] = "Transfer file to the remote system";
char msghelp[] = "Display help text";
char msgclose[] = "Terminate the network connection";
char msgquit[] = "Quit FTP";


CMDTBL cmdtbl[] =
{   {"?"          , cmdhelp    , msghelp},
    {"AL*LFILES"  , ftpAllFiles, msgallfls},
    {"AP*PEND"    , cmdappend  , "Append to remote file"},
    {"AS*CII"     , ftpAscii   , "Set transfer type to ASCII non-print (AN)"},
    {"BI*NARY"    , ftpBinary  , "Set transfer type to Image (I)"},
    {"BY*E"       , cmdquit    , msgquit},
    {"CD*"        , ftpCd      , "Change remote directory path"},
    {"CL*OSE"     , ftpClose   , msgclose},
    {"DEL*ETE"    , ftpDelete  , "Delete remote file"},
    {"DEB*UG"     , cmddebug   , "Toggle debug mode"},
    {"DIR*"       , cmddir     , msgdir},
    {"DIS*CONNECT", ftpClose   , msgclose},
    {"G*ET"       , cmdget     , msgget},
    {"HE*LP"      , cmdhelp    , msghelp},
    {"LC*D"       , cmdlcd     , "Change local directory path"},
    {"LI*TERAL"   , ftpLiteral , "Send literal command"},
    {"LS*"        , cmdls      , "List remote directory"},
    {"MG*ET"      , cmdmget    , "Transfer multiple files from the remote system"},
    {"MK*DIR"     , ftpMkDir   , "Create directory on remote system"},
    {"MP*UT"      , cmdmput    , "Transfer multiple files to the remote system"},
    {"O*PEN"      , cmdopen    , "Establish network connection"},
    {"PR*OMPT"    , cmdprompt  , "Toggle prompt mode"},
    {"PU*T"       , cmdput     , msgput},
    {"PW*D"       , ftpPWD     , "Display current remote directory path"},
    {"QUI*T"      , cmdquit    , msgquit},
    {"QUO*TE"     , cmdquote   , "????"},
    {"REC*V"      , cmdget     , msgget},
    {"REM*OTEHELP", cmdrhelp   , "Display help text from remote system"},
    {"REN*AME"    , cmdren     , "Rename file on remote system"},
    {"RM*DIR"     , ftpRmDir   , "Remove directory on remote system"},
    {"SE*ND"      , cmdput     , msgput},
    {"ST*ATUS"    , ftpStatus  , "Display current status"},
    {"TY*PE"      , ftpType    , "Set transfer type (ASCII or BINARY)"},
    {"U*SER"      , ftpUser    , "Specify new user name"},
    {NULL}
};

// Misc. variables

Prog_Info pib;
char copymsg[] = "";
char prgname[] = "FTP";			// Our programe name
char envname[] = "^XOS^FTP^OPT"; // The environment option name
char example[] = "{/options} {hostaddr}";
char description[] = "This command implements an FTP client.  If a host "
		"address is specified, a command connection is established to that host.";
char synerr[] = "? Incorrect syntax in network address\n";


void main(
    int   argc,
    char *argv[])

{
    CMDTBL *tbl;
    char   *envpnt[2];
    long    rtn;
    char    strbuf[256];

    // Set defaults

    reg_pib(&pib);
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

    // Check Global Configuration Parameters

    global_parameter(TRUE);

    // Check Local Configuration Parameters

    if(svcSysFindEnv(0, envname, NULL, strbuf, sizeof(strbuf), NULL) > 0)
    {
		envpnt[0] = strbuf;
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
	ftpInit(&rspnum, rspstr, sizeof(rspstr), debug);
    if (hostaddr != NULL)
    {
		if ((rtn = cmdopen(hostaddr)) < 0)
			errormsg("Error connecting", rtn);
    }
    trmcnt = 0;
    while (TRUE)
    {
		fputs("FTP> ", stdout);
		if (!gettrm())
		{

		}
		rspdone = FALSE;
		if ((tbl = getcmd()) == NULL)
			fputs("? Invalid command\n", stdout);
		else
		{
			skipws();
			if ((rtn = (tbl->func)(cmdpnt)) < 0)
				errormsg("Error executing command", rtn);
			else if (!rspdone)
				printf("%s\n", rspstr);
		}
    }
}


int gettrm(void)

{
    char *pnt;
    char  chr;

    pnt = cmdbufr;
    for (;;)
    {
		if (--trmcnt < 0)
		{
			if ((trmcnt = svcIoInBlock(DH_STDIN, trmbfr, sizeof(trmbfr))) < 0)
				return (FALSE);
			trmpnt = trmbfr;
			while (--trmcnt >= 0)
			{
				if ((chr = *trmpnt++) == '\n')
				{
					*pnt = 0;
					cmdpnt = cmdbufr;
					return (TRUE);
				}
				if (chr != '\r')
				{
					if ((pnt - cmdbufr) > (sizeof(cmdbufr) - 2))
					{
						// COMPLAIN ABOUT LONG LINE HERE!!!!

						return (FALSE);
					}
					*pnt++ = chr;
				}
			}
		}
    }
}

//********************************************
// Function: nonopt - Process non-option input
// Returned: Nothing
//********************************************

int nonopt(
    char *arg)

{
    int size;

    if (hostaddr != NULL)
    {
		fputs(synerr, stdout);
		exit(1);
    }
    size = strlen(arg);
    hostaddr = getspace(size + 2);
    strcpy(hostaddr, arg);
    return (TRUE);
}

//****************************************************************
// Function: getcmd - Get command keyword and search command table
// Returned: Pointer to command table entry or NULL if bad command
//****************************************************************

CMDTBL *getcmd(void)

{
    char   *pnt1;
    char   *pnt2;
    CMDTBL *tbl;
    int     cnt;
    char    chr;
    char    tc;
    char    exact;

    skipws();
    pnt1 = atom;
    cnt = sizeof(atom) - 2;
    while ((chr = toupper(*cmdpnt)) != 0 && !isspace(chr))
    {
		cmdpnt++;
		if (--cnt <= 0)
			return (NULL);
		*pnt1++ = chr;
    }
    *pnt1 = 0;
    tbl = cmdtbl;
    do
    {
		pnt1 = atom;
		pnt2 = tbl->name;
		exact = TRUE;
		do
		{
			chr = *pnt1++;
			if ((tc = *pnt2++) == '*')
			{
				exact = FALSE;
				tc = *pnt2++;
			}
		} while (chr != 0 && chr == tc);
		if (chr == 0 && (tc == 0 || !exact))
			return (tbl);
	} while ((++tbl)->name != NULL);
	return (NULL);
}

void skipws(void)

{
    char chr;

    while ((chr = *cmdpnt) != 0 && isspace(chr))
		cmdpnt++;
}


//*************************************************
// Function: cmdopen - Process the OPEN FTP command
// Returned: Nothing
//*************************************************

long cmdopen(
	char *arg)

{
	long rtn;

	if ((rtn = ftpOpen(arg)) < 0)
		return (rtn);
	printf("%s\n", rspstr);
    if (rspnum == 220)					// Need a user name?
    {									// Yes
		fputs("User name for remote system: ", stdout);
		gettrm();
		if ((rtn = ftpUser(cmdpnt)) < 0)
		{
			errormsg("Error sending user name", rtn);
			return (0);
		}
		printf("%s\n", rspstr);
		if (rspnum == 331)				// Need a password?
		{								// Yes
			fputs("Password for remote system: ", stdout);
			svcTrmFunction(DH_STDIN, TF_DSECHO);
			gettrm();
			fputs("\n", stdout);
			svcTrmFunction(DH_STDIN, TF_ENECHO);
			if ((rtn = ftpPassword(cmdpnt)) < 0)
			{
				errormsg("Error sending password", rtn);
				return (0);
			}
			printf("%s\n", rspstr);
		}
	}
	ftpSystemType();
	return (0);
}


//*****************************************************
// Function: cmdappend - Process the APPEND FTP command
// Returned: Nothing
//*****************************************************

long cmdappend(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}


//***************************************************
// Function: cmddebug - Process the DEBUG FTP command
// Returned: Nothing
//***************************************************

long cmddebug(
	char *arg)

{
	arg = arg;

    debug ^= 0x01;
	ftpDebug(debug);
    printf("%% Debug mode is %s\n", (debug) ? "enabled" : "disabled");
	rspdone = TRUE;
	return (0);
}


//***********************************************
// Function: cmdlcd - Process the LCD FTP command
// Returned: Nothing
//***********************************************

long cmdlcd(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}


//***********************************************
// Function: cmddir - Process the DIR FTP command
// Returned: Nothing
//***********************************************

long cmddir(
	char *arg)

{
	return (ftpDir(arg, notify, trmxfer, xferbufr, sizeof(xferbufr), TRUE));
}


//*********************************************
// Function: cmdls - Process the LS FTP command
// Returned: Nothing
//*********************************************

long cmdls(
	char *arg)

{
	return (ftpLs(arg, notify, trmxfer, xferbufr, sizeof(xferbufr), TRUE));
}


//***********************************************
// Function: cmdget - Process the GET FTP command
// Returned: Nothing
//***********************************************

long cmdget(
	char *arg)

{
    char *lclpnt;
    char *rmtpnt;
	long  rtn;

	rmtpnt = getname(&arg);
	lclpnt = getname(&arg);
    if (lclpnt == NULL)
		lclpnt = rmtpnt;
    if (!setuplocal(lclpnt))
		return (0);
	starttime.high = 0;
	totalbytes = 0;
	if ((rtn = ftpGet(rmtpnt, notify, getxfer, xferbufr, sizeof(xferbufr),
			&filedt, TRUE)) < 0)
	{
		if (filehndl > 0)
		{
			svcIoClose(filehndl, 0);
			filehndl = -1;
		}
		return (rtn);
	}
	if (filehndl > 0)
	{
		svcSysDateTime(T_GTHRDTTM, &stoptime);
		dispstat();
		rtn = svcIoClose(filehndl, 0);
		filehndl = -1;
		if (rtn < 0)
			errormsg("Error closing local file", rtn);
	}
	return (0);
}

//*************************************************
// Function: cmdhelp - Process the HELP FTP command
// Returned: Nothing
//*************************************************

long cmdhelp(
	char *arg)

{
    CMDTBL *tbl;
    char   *pnt1;
    char   *pnt2;
    char    chr;

	arg = arg;

    if (atend())
		fputs(helptext, stdout);
    else
    {
		if ((tbl = getcmd()) == NULL)
			printf("? %s is not a valid command\n\n", atom);
		else
		{
			pnt1 = tbl->name;
			pnt2 = atom;
			do
			{
				if ((chr = *pnt1++) != '*')
					*pnt2++ = chr;
			} while (chr != 0);
			printf("   %s:  %s\n\n", atom, tbl->help);
		}
    }
	return (0);
}

//*************************************************
// Function: cmdmget - Process the MGET FTP command
// Returned: Nothing
//*************************************************

long cmdmget(
	char *arg)

{
	NBLK *next;
	long  rtn;
	uchar ask;
	uchar doit;

	// First do an NLST command to get a list of files to transfer.

	rspdone = TRUE;
	nameppnt = &namehead;
	namehead = NULL;
	if ((rtn = ftpLs(arg, NULL, listxfer, xferbufr, sizeof(xferbufr),
			TRUE)) < 0)
		return (rtn);

	ask = prompt;
	while (namehead != NULL)
	{
		doit = TRUE;
		if (ask)
		{
			sprintf(xferbufr, "Transfer %.520s", namehead->name);
			if ((rtn = askuser(xferbufr)) == 0)
				doit = FALSE;
			else if (rtn == 2)
				ask = FALSE;
			else if (rtn == 3)
				break;
		}
		if (doit)
		{
			if (!setuplocal(namehead->name))
				return (0);
			starttime.high = 0;
			totalbytes = 0;
			if ((rtn = ftpGet(namehead->name, notify, getxfer, xferbufr,
					sizeof(xferbufr), &filedt, TRUE)) < 0)
			{
				if (filehndl > 0)
				{
					svcIoClose(filehndl, 0);
					filehndl = -1;
				}
				return (rtn);
			}
			svcSysDateTime(T_GTHRDTTM, &stoptime);
			if (filehndl > 0)
			{
				rtn = svcIoClose(filehndl, 0);
				filehndl = -1;
				if (rtn < 0)
				{
					errormsg("Error closing local file", rtn);
					return (0);
				}
			}
			printf("%s\n", rspstr);
			dispstat();
		}
		next = namehead->next;
		free(namehead);
		namehead = next;
	}
	return (0);
}

//*************************************************
// Function: cmdmput - Process the MPUT FTP command
// Returned: Nothing
//*************************************************

long cmdmput(
	char *arg)

{
	time_sz fdt;
	long    rtn;
	uchar   doit;
	uchar   ask;
	char    spec[300];

	static struct
	{	byte4_parm  hndl;
		byte4_parm  optn;
		lngstr_parm spec;
		time8_parm  dt;
		uchar       end;
	} oparms =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_DIRHNDL},
		{PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_NOPREFIX|XFO_FILENAME},
		{PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, 0, sizeof(spec),
				sizeof(spec)},
		{PAR_GET|REP_DT  , 8, IOPAR_MDATE}
	};

	rspdone = TRUE;
	if ((oparms.hndl.value = svcIoOpen(XO_ODF, arg, NULL)) < 0)
	{
		errormsg("Error opening directory for search", oparms.hndl.value);
		return (0);
	}
	oparms.spec.buffer = spec;
	while ((filehndl = svcIoOpen(XO_IN, arg, &oparms)) > 0)
	{
		doit = TRUE;
		if (ask)
		{
			sprintf(xferbufr, "Transfer %.520s", spec);
			if ((rtn = askuser(xferbufr)) == 0)
				doit = FALSE;
			else if (rtn == 2)
				ask = FALSE;
			else if (rtn == 3)
				break;
		}
		if (doit)
		{
			*(time_s *)&fdt = oparms.dt.value;
			fdt.tzone = 0;
			fdt.dlst = 0;
			starttime.high = 0;
			totalbytes = 0;
			rtn = ftpPut(spec, &fdt, notify, putxfer, xferbufr,
					sizeof(xferbufr), TRUE);
			svcSysDateTime(T_GTHRDTTM, &stoptime);
			svcIoClose(filehndl, 0);
			filehndl = -1;
			if (rtn < 0)
				return (rtn);
			printf("%s\n", rspstr);
			dispstat();
		}
		svcIoClose(filehndl, 0);
		filehndl = -1;
	}
	return (0);
}

//*****************************************************
// Function: cmdprompt - Process the PROMPT FTP command
// Returned: Nothing
//*****************************************************

long cmdprompt(
	char *arg)

{
	arg = arg;

    prompt ^= 0x01;
    printf("%% Prompt mode is %s\n", (prompt) ? "enabled" : "disabled");
	rspdone = TRUE;
	return (0);
}

//***********************************************
// Function: cmdput - Process the PUT FTP command
// Returned: Nothing
//***********************************************

long cmdput(
	char *arg)

{
	static struct
	{	time8_parm dt;
		uchar      end;
	} opnparms =
	{	{PAR_GET|REP_DT, 8, IOPAR_MDATE}
	};

	time_sz fdt;
    char   *lclpnt;
    char   *rmtpnt;
	long    rtn;

	lclpnt = getname(&arg);
	rmtpnt = getname(&arg);
    if (rmtpnt == NULL)
		rmtpnt = lclpnt;
    if (!setuplocal(lclpnt))
		return (0);
	if (debug)
		printf("ftp: Opening local file %s for reading\n", lclname);
	if ((filehndl = svcIoOpen(XO_IN, lclname, &opnparms)) < 0)
	{
		errormsg("Error opening local file", filehndl);
		return (0);
	}
	*(time_s *)&fdt = opnparms.dt.value;
	fdt.tzone = 0;
	fdt.dlst = 0;
	starttime.high = 0;
	totalbytes = 0;
	if ((rtn = ftpPut(rmtpnt, &fdt, notify, putxfer, xferbufr, sizeof(xferbufr),
			TRUE)) >= 0)
		dispstat();
	svcIoClose(filehndl, 0);
	filehndl = -1;
	return (rtn);
}

//*************************************************
// Function: cmdquit - Process the QUIT FTP command
// Returned: Nothing
//*************************************************

long cmdquit(
	char *arg)

{
	arg = arg;

    exit(0);
	return (0);
}

//***************************************************
// Function: cmdquote - Process the QUOTE FTP command
// Returned: Nothing
//***************************************************

long cmdquote(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}

//*************************************************
// Function: cmdhelp - Process the HELP FTP command
// Returned: Nothing
//*************************************************

long cmdrhelp(
	char *arg)

{
	arg = arg;

	return (ER_NIYT);
}

//**************************************************
// Function: cmdren - Process the RENAME FTP command
// Returned: Nothing
//**************************************************

long cmdren(
	char *arg)

{
    char *pnt;
    char  chr;

	arg = arg;

    pnt = cmdpnt;
    while ((chr = *pnt) != 0 && !isspace(chr))
		pnt++;
    if (chr == 0)
    {
		fputs("? No new name specified\n", stdout);
		return (0);
    }
    *pnt = 0;

	return (ER_NIYT);
}

//******************************************************
// Function: atend - Determine if at end of command line
// Returned: TRUE if at end, FALSE if not
//******************************************************

int atend(void)

{
    skipws();
    	return (*cmdpnt == 0);
}

//**********************************************************
// Function: getname - Get a file name from the command line
// Returned: Pointer to first charater in name
//**********************************************************

char *getname(
	char **str)

{
    char *begin;
	char *spnt;
	char *dpnt;
    char  chr;
    char  quote;

    quote = 0;
	dpnt = spnt = *str;

	while (isspace(*spnt))
		spnt++;
    begin = spnt;
    while ((chr = *spnt) != 0)
    {
		if (chr == '"')
		{
			spnt++;
			if (dpnt[1] != '"')
			{
				quote ^= 0x01;
				continue;
			}
		}
		else
		{
			if (!quote && isspace(chr))
				break;
		}
		spnt++;
		*dpnt++ = chr;
    }
    if (begin == spnt)
		begin = NULL;
    *dpnt = 0;
	if (chr != 0)
		spnt++;
	*str = spnt;
    return (begin);
}

//*******************************************************
// Function: setuplocal - Set up local file specification
// Returned: TRUE if OK, FALSE if error
//*******************************************************

int setuplocal(
    char *name)

{
    if (name == NULL)
    {
		fputs("? No file name specified\n", stdout);
		return (FALSE);
    }
    strmov(strmov(lclname, lclpath), name);
    return (TRUE);
}

//**********************************************
// Function: dispstat - Display transfer statics
// Returned: Nothing
//**********************************************

void dispstat(void)

{
    int    millisec;
    ulong  result[2];
    time_s xfertime;

    sdtsub(&xfertime, &stoptime, &starttime); // Get the time
	millisec = div64by32(xfertime.low, xfertime.high, ST_MILLISEC);
										// Change to milliseconds
    printf("%% %,d byte%s transfered in %,d.%03.3d second%s", totalbytes,
	    (totalbytes == 1) ? "" : "s", millisec/1000, millisec % 1000,
	    (millisec == 1000) ? "" : "s");
    if (totalbytes != 0 && millisec != 0)
		ulongmuldiv(result, totalbytes, 1000, millisec);
    else
		result[0] = 0;
    if (result[0] != 0)
		printf(" (%,u byte%s/sec)", result[0], (result[0] != 1) ? "s" : "");
    fputs("\n", stdout);
}


//*******************************************************
// Function: listxfer - Called when have input data which
//				is a list of files for mget
// Returned: 0 if normal, negative error code if error
//*******************************************************

long listxfer(
	char *bufr,
	int   size)

{
	NBLK *npnt;
	char *pnt;
	char *bgn;
	char *end;
///	long  rtn;
	int   len;
	char  chr;

	pnt = bufr;
	bufr[size] = 0;
	while (*pnt != 0)
	{
		end = NULL;
		bgn = pnt;
		while ((chr = *pnt) != 0)
		{
			pnt++;
			if (chr == '\r')
			{
				end = pnt;
				pnt[-1] = 0;
			}
			else if (chr == '\n')
			{
				if (end == NULL)
					end = pnt;
				pnt[-1] = 0;
				break;
			}
		}
		if (end == NULL)
			break;
		len = end - bgn;
		if ((npnt = (NBLK *)malloc(offsetof(NBLK, name) + len)) == NULL)
		{
			errormsg("Cannot allocate memory", -errno);
			return (-errno);
		}
		memcpy(npnt->name, bgn, len);
		npnt->next = NULL;
		*nameppnt = npnt;
		nameppnt = &(npnt->next);
	}
	return (0);
}


//*****************************************************
// Function: trmxfer - Called when have input data that
//				is to be displayed on the terminal
// Returned: 0 if normal, negative error code if error
//*****************************************************

long trmxfer(
	char *bufr,
	int   size)

{
	fwrite(bufr, 1, size, stdout);
	return (0);
}

long getxfer(
	char *bufr,
	int   size)

{
	static struct
	{	time8_parm cdate;
		time8_parm mdate;
		uchar      end;
	} lfparms =
	{	{PAR_SET|REP_DT, 8, IOPAR_CDATE},
		{PAR_SET|REP_DT, 8, IOPAR_MDATE}
	};

	long rtn;

	if (filehndl <= 0)
	{
		if (debug)
			printf("ftp: Opening local file %s for writing\n", lclname);
		lfparms.cdate.value = *(time_s *)&filedt;
		lfparms.mdate.value = *(time_s *)&filedt;
		if ((filehndl = svcIoOpen(XO_OUT|XO_TRUNCA|XO_CREATE, lclname,
				&lfparms)) < 0)
		{
			errormsg("Error opening local file", filehndl);
			return (filehndl);
		}
		totalbytes = 0;
		svcSysDateTime(T_GTHRDTTM, &starttime);
	}
	if ((rtn = svcIoOutBlock(filehndl, bufr, size)) > 0)
		totalbytes += rtn;
	return (rtn);
}

long putxfer(
	char *bufr,
	int   size)

{
	long rtn;

	if (starttime.high == 0)
		svcSysDateTime(T_GTHRDTTM, &starttime);
	if ((rtn = svcIoInBlock(filehndl, bufr, size)) < 0)
	{
		if (rtn == ER_EOF)
			rtn = 0;
		svcSysDateTime(T_GTHRDTTM, &stoptime);
		return (rtn);
	}
	totalbytes += rtn;
	return (rtn);
}


void notify(
	char *str)

{
	printf("%s\n", str);
///	str[0] = 0;
}


//*************************************************
// Function: askuser - Ask user what to do
// Returned: 0 if No, 1 if Yes, 2 if All, 3 if Quit
//*************************************************

int askuser(
    char *msg)

{
    long rtn;
    char bufr[] = {0, '\b', 0};

    static struct
    {	byte4_parm  oldtim;
		byte4_parm  clrtim;
		byte4_parm  settim;
	char        end;
    } trmparm1 =
    {	{PAR_GET|REP_HEXV, 4, IOPAR_TRMSINPMODE, 0},
		{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
		{PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, TIM_CHAR}
    };

    static struct
    {	byte4_parm  clrtim;
		byte4_parm  settim;
		char        end;
    } trmparm2 =
    {	{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0x7FFFFFFF},
		{PAR_SET|REP_DECV, 4, IOPAR_TRMSINPMODE, 0}
    };

    svcIoInBlockP(DH_STDIN, NULL, 0, &trmparm1);
    fprintf(stderr, "%s (Yes/No/All/Quit)? ", msg);
    svcTrmFunction(DH_STDIN, TF_CLRINP);
    rtn = -1;
    while (TRUE)
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
    svcIoInBlockP(DH_STDIN, NULL, 0, &trmparm2);
    svcTrmFunction(DH_STDIN, TF_CLRINP);
    return (rtn);
}


//*******************************************
// Function: errormsg - Display error message
// Returned: Nothing
//*******************************************

void errormsg(
    char *msg,
    int   code)

{
    char bufr[100];

    svcSysErrMsg(code, 0x03, bufr);
    printf("? %s\n  %s\n\n", msg, bufr);
}
