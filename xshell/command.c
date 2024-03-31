//------------------------------------------------------------------------------
//  command.c - The command processor
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

typedef struct
{   ushort src;
    ushort dst;
    long   bits;
} DEVITEM;

struct
{   DEVITEM dlstdin;
    DEVITEM dlstdout;
    DEVITEM dlstderr;
    DEVITEM dlstdtrm;
	long    end;
} devlist =						// Device list for running a program
{  {0, STDIN , XO_IN},			//   STDIN = 1
   {0, STDOUT, XO_IN|XO_OUT},	//   STDOUT = 2
   {0, STDERR, XO_IN|XO_OUT},	//   STDERR = 3
   {0, STDTRM, XO_IN|XO_OUT},	//   STDTRM = 5
};

struct
{   LNGSTRPARM arglist;
    LNGSTRPARM devlist;
    char       end;
} runparm =
{	{PAR_SET|REP_STR, 0, IOPAR_RUNCMDTAIL},
	{PAR_SET|REP_STR, 0, IOPAR_RUNDEVLIST, (char *)&devlist,
			sizeof(devlist), sizeof(devlist)}
};

QAB runqab =
{	RFNC_WAIT|RFNC_RUN,		// func    - Function
	0,						// status  - Returned status
	0,						// error   - Error code
	0,						// amount  - Process ID
	0,						// handle  - Device handle
	0,						// vector  - Vector for interrupt
	{0},
	XR_ACSENV|XR_CHGENV,	// option  - Options
	0,						// count   - Count
	NULL,					// buffer1 - Pointer to file spec
	NULL,					// buffer2 - Unused
	(uchar *)&runparm		// parm    - Pointer to parameter area
};

typedef struct
{	char *name;
	int   rsthist;
	int (*func)(char *args);
} CMDTBL;

CMDTBL cmdtbl[] =
{	{"BATOPT" , TRUE , cmdbatopt},
	{"CD"     , TRUE , cmdcd},
	{"CHDIR"  , TRUE , cmdcd},
	{"CLS"    , TRUE , cmdcls},
	{"DELAY"  , TRUE , cmddelay},
	{"EXIT"   , TRUE , cmdexit},
	{"LOGOUT" , TRUE , cmdlogout},
	{"BYE"    , TRUE , cmdlogout},
	{"CALL"   , TRUE , cmdcall},
	{"ECHO"   , TRUE , cmdecho},
	{"FOR"    , TRUE , cmdfor},
	{"GOTO"   , TRUE , cmdgoto},
	{"HISTORY", FALSE, cmdhistory},
	{"IF"     , TRUE , cmdif},
	{"JUMP"   , TRUE , cmdjump},
	{"PAUSE"  , TRUE , cmdpause},
	{"REM"    , TRUE , cmdrem},
	{"RETURN" , TRUE , cmdreturn},
	{"SHIFT"  , TRUE , cmdshift},
	{"VER"    , TRUE , cmdver}
};
#define NUMCMDS (sizeof(cmdtbl)/sizeof(CMDTBL))

char   *exttbl[] = {".RUN", ".BAT", NULL};

static void  cmdtoolong(void);
static char *collectname(char *cmd, char *name);
static void  finishredir(void);
static char *redirinput(char *cmd);
static char *rediroutput(char *cmd);


//****************************************************************
// Function: docmd - Do a single command
// Returned: TRUE if should terminate batch level, FALSE otherwise
//****************************************************************

int docmd(
	char *cmd)
{
	CMDST   cmdst;
	FILE   *file;
	cchar  *epnt;
	char   *apnt;
	char   *cpnt;
	char   *ppnt;
	CMDTBL *tpnt;
	long    rtn;
	int     rval;
	int     ccnt;
	int     acnt;
	char    atom[128];
	char    chr;
	char    haveext;
	char    havepath;
	char    tohist;
	char    cmdbfr[CMDMAX];
	char    prgbfr[512];

///	printf("### cmd1: |%s|\n", cmd);

	// Copy the command line and expand environment string and batch
	//   arguments.

	while ((chr = *cmd) != 0 && isspace(chr))
		cmd++;
	if (*cmd == 0)
		return (FALSE);

	if (*cmd == '!')
	{
		retrievefromhistory(cmd + 1);
		return (FALSE);
	}
	tohist = TRUE;
	while (TRUE)
	{
		if ((chr = *cmd) == '@')
		{
			cmd++;
			continue;
		}
		if (chr == '-')
		{
			cmd++;
			tohist = FALSE;
			continue;
		}
		break;
	}
	if (!putinhistory(cmd, strlen(cmd)))
		return (FALSE);
	cpnt = cmdbfr;
	ccnt = CMDMAX - 3;
	while ((chr = *cmd) != 0 && --ccnt > 0)
	{
		cmd++;
		if (chr != '%')
			*cpnt++ = chr;
		else
		{
			chr = *cmd++;
			if (!xosarg && chr != 0 && isdigit(chr))
			{
				if ((cpnt = insertbatarg(cpnt, &ccnt, chr & 0x0F)) == NULL)
				{
					cmdtoolong();
					return (FALSE);
				}
			}
			else
			{
				acnt = sizeof(atom) - 2;
				apnt = atom;
				do
				{
					if (--acnt >= 0)
						*apnt++ = chr;
				} while ((chr = *cmd++) != 0 && chr != '%');
				*apnt = 0;

				if (xosarg && isdigit(atom[0]))
				{
					rtn = strtol(atom, &ppnt, 10);
					if (*ppnt == 0)
					{
						if ((cpnt = insertbatarg(cpnt, &ccnt, chr & 0x0F)) ==
								NULL)
						{
							cmdtoolong();
							return (FALSE);
						}
						continue;
					}
				}

///				printf("### ccnt: %d, looking for |%s|\n", ccnt, atom);

				if ((rtn = svcSysGetEnv(atom, &epnt)) < 0)
				{
					if ((rtn = apnt - atom) < ccnt)
					{
						cmdtoolong();
						return (FALSE);
					}
					memcpy(cpnt, atom, rtn);
				}
				else
				{
					rtn &= 0x00FFFFFF;

///					printf("### found it, len = %d\n", rtn);

					if (rtn > ccnt)
					{
						cmdtoolong();
						return (FALSE);
					}
					memcpy(cpnt, epnt, rtn);
				}
				cpnt += rtn;
				ccnt -= rtn;
			}
		}
	}

	svcSchSetLevel(8);					// Can't have a signal once we change
	if (flags & FLG_HAVECC)				//   the device handles
	{
		svcSchSetLevel(0);
		flags &= ~FLG_HAVECC;
		histcurblk = NULL;
		return (FALSE);
	}
	flags |= FLG_HOLDCC;				//  Indicate should hold off CC when
										//    we enable signals
	*cpnt = 0;

	if (cmdbfr[0] == ':' && ccs->batch)	// Batch label?
	{

		svcSchSetLevel(0);
		return (FALSE);
	}

	// Now scan the line again and handle possible redirection

///	printf("### cmd2: |%s|\n", cmdbfr);

	ccs->savein = ccs->datain;
	ccs->saveout = ccs->cmdout;
	ccs->saveerr = ccs->cmderr;
	cpnt = cmd = cmdbfr;
	while ((chr = *cmd++) != 0)
	{
		if (chr == '\\')
		{
			if ((chr = *cmd) == '>' || chr == '<')
			{
				*cpnt++ = chr;
				cmd++;
			}
			else
				*cpnt++ = '\\';
		}
		else if (chr == '>')			// Output redirection?
			cmd = rediroutput(cmd);
		else if (chr == '<')			// Input redirection?
			cmd = redirinput(cmd);
		else
			*cpnt++ = chr;
	}
	*cpnt = 0;

	// Remove any trailing whitespace

	cpnt = cmdbfr + strlen(cmdbfr);
	while (--cpnt >= cmdbfr && isspace(*cpnt))
		;
	cpnt[1] = 0;

	// Collect a possible command atom

///	printf("### cmd3: |%s|\n", cpnt);

	cpnt = cmdbfr;
	apnt = atom;
	acnt = 11;
	while ((chr = *cpnt) != 0 && chr != ':' && chr != '\\' && chr != '/' &&
			!isspace(chr))
	{
		cpnt++;
		if (--acnt < 0)
		{
			atom[0] = 0;
			break;
		}
		*apnt++ = toupper(chr);
	}
	if (atom[0] != 0)
	{
		*apnt = 0;
		if (chr == ':' && ((chr = *++cpnt) == 0 || isspace(chr)))
		{
			if (chr != 0) 
			{
				while ((chr = *++cpnt) != 0 && isspace(chr))
					;
			}
			if (chr != 0)
				cmnderror(0, "Have extra arguments after device name");
			else
			{
				apnt[0] = ':';
				apnt[1] = 0;
			    if ((rtn = svcLogDefine(0, "Z:", atom)) < 0)
        			cmnderror(rtn, "Error setting current disk");
			}
			finishredir();
			svcSchSetLevel(0);
			return (FALSE);
		}
		tpnt = cmdtbl;
		ccnt = NUMCMDS;
		do
		{
			if (strcmp(tpnt->name, atom) == 0)
			{
				while((chr = *cpnt) != 0 && isspace(chr))
					cpnt++;
				rval = tpnt->func(cpnt); // Do the internal command
				if (tpnt->rsthist)
					histcurblk = NULL;
				finishredir();			// Finished now
				svcSchSetLevel(0);
				return (rval);
			}
			tpnt++;
		} while (--ccnt > 0);
	}

	// Here if not an internal command. Try to do it as an external command.

	// Copy the rest of the program specfification. This also determines
	//   if an extension was specified and finds the end of what was
	//   specified.

	ppnt = prgbfr;
	cpnt = cmdbfr;
	ccnt = 500;
	haveext = FALSE;
	havepath = FALSE;
	while ((chr = *cpnt) != 0 && chr != '/' && !isspace(chr))
	{
		if (--ccnt < 0)
		{
			cmnderror(0, "Command name is too long");
			finishredir();
			svcSchSetLevel(0);
			return (FALSE);
		}
		if (chr == '.')
			haveext = TRUE;
		if (chr == '\\')
		{
			havepath = TRUE;
			haveext = FALSE;
		}
		if (chr == ':')
			havepath = TRUE;
		cpnt++;
		*ppnt++ = chr;
	}
	*ppnt = 0;
	if ((chr = *cpnt) != 0 && !isspace(chr))
	{
		ccnt = strlen(cpnt);
		memmove(cpnt + 1, cpnt, ccnt + 1);
		*cpnt = ' ';
		ccnt += ((cpnt - cmdbfr) + 1);
	}
	else
		ccnt = strlen(cmdbfr);
	runparm.arglist.buffer = cmdbfr;
	runparm.arglist.strlen = runparm.arglist.bfrlen = ccnt;
    devlist.dlstdin.src = ccs->datain->_handle;
    devlist.dlstdout.src = ccs->cmdout;
    devlist.dlstderr.src = ccs->cmderr;
    devlist.dlstdtrm.src = DH_STDTRM;
	runqab.buffer1 = prgbfr;
	childflag = 1;
	histcurblk = NULL;

	if ((rtn = sysIoRunCmd(&runqab, cmdbfr, "PATH", exttbl)) < 0)
	{
		if (rtn == ER_FILNF || rtn == ER_DIRNF || rtn == ER_NSDEV)
		{
			if (!havepath && applist != NULL)
			{
				if ((rtn = sysIoRunCmd(&runqab, cmdbfr, applist, exttbl)) < 0)
				{
					if (rtn == ER_FILNF || rtn == ER_DIRNF || rtn == ER_NSDEV)
						cmnderror(0, "Invalid command");
					else
						cmnderror(rtn, "Error running program");
					return (FALSE);
				}
			}
			else
			{
				cmnderror(0, "Invalid command");
				return (FALSE);
			}
		}
		else
		{
			cmnderror(rtn, "Error running program");
			return (FALSE);
		}
	}
	if (rtn == 0)
	{
		memset(&childdata, 0, sizeof(childdata));
		childdata.pid = runqab.amount & 0xFFFF0FFF;
		svcSchSetLevel(0);
		svcSchSuspend(&childflag, -1);	
		svcSchSetLevel(8);
    	svcTrmDspMode(STDTRM, DM_USEBASE, NULL); // Use base display mode
    	svcTrmFunction(STDTRM, TF_RESET);	// Set terminal to normal state
		childreport(&childdata);
		childdata.pid = 0;
	}
	else
	{
		if ((file = fopen(cmdbfr, "r")) == NULL)
		{
			if (errno == -ER_FILNF || errno == -ER_DIRNF || errno == -ER_NSDEV)
				return (-errno);
			cmnderror(-errno, "Error executing batch file");
			return (2);
		}
		cmdbfr[0] = 0;
		cmdst = *ccs;
		cmdst.prev = ccs;
		cmdst.cmdin = file;
		cmdst.batch = TRUE;
		ccs = &cmdst;
		cmdlevel();
		fclose(file);
	}
	finishredir();
	svcSchSetLevel(0);
	return (FALSE);
}


static void cmdtoolong(void)
{
	svcIoOutString(ccs->cmderr, "? SHELL: Command line is too long\n", 0);
}


//********************************************
// Function: rediroutput - Do output direction
// Returned: Updated command pointer
//********************************************

static char *rediroutput(
	char *cmd)
{
	long mode;
	long handle;
	char chr;
	char name[FILESPECMAX + 4];

	mode = XO_CREATE | XO_TRUNCA | XO_OUT | XO_IN;
	if ((chr = *cmd) == '>')
	{
		mode = XO_CREATE | XO_APPEND | XO_OUT | XO_IN;
		chr = *++cmd;
	}
	if (chr == '?')
		cmd++;
	if ((cmd = collectname(cmd, name)) != NULL)
	{
		if ((handle = svcIoOpen(mode, name, NULL)) < 0)
		{
			cmnderror(handle, "Error opening file for redirected output");
			return (NULL);
		}

		if (chr == '?')
			ccs->cmderr = handle;
		else
			ccs->cmdout = handle;
	}
	return (cmd);
}


//******************************************
// Function: redirinput - Do input direction
// Returned: Updated command pointer
//******************************************

static char *redirinput(
	char *cmd)
{
	FILE *file;
	char  name[FILESPECMAX + 4];

	if ((cmd = collectname(cmd, name)) != NULL)
	{
		if ((file = fopen(name, "rb")) == NULL)
		{
			cmnderror(-errno, "Error opening file for redirected input");
			return (NULL);
		}
		ccs->datain = file;
	}
	return (cmd);
}


static char *collectname(
	char *cmd,
	char *name)
{
	int  cnt;
	char chr;

	while ((chr = *cmd) != 0 && isspace(chr))
		cmd++;
	if (chr != 0)
	{
		cnt = FILESPECMAX;
		while ((chr = *cmd) != 0 && !isspace(chr))
		{
			cmd++;
			if (--cnt <= 0)
			{
				cmnderror(0, "File name for redirection is too long");
				return (NULL);
			}
			*name++ = chr;
		}
		*name = 0;
	}
	return (cmd);
}



static void finishredir(void)
{
	long rtn;

	if (ccs->savein != ccs->datain)
	{
		fclose(ccs->cmdin);
		ccs->datain = ccs->savein;
	}
	if (ccs->saveout != ccs->cmdout)
	{
		if ((rtn = svcIoClose(ccs->cmdout, 0)) < 0)
			cmnderror(rtn, "Error closing redirected standard output");
		ccs->cmdout = ccs->saveout;
	}
	if (ccs->saveerr != ccs->cmderr)
	{
		if ((rtn = svcIoClose(ccs->cmderr, 0)) < 0)
			cmnderror(rtn, "Error closing redirected error output");
		ccs->cmderr = ccs->saveerr;
	}
}
