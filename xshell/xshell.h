//------------------------------------------------------------------------------
//
//  shell.h - Parameter file for SHELL
//
//  Copyright (c) 1984-1994 Saguaro Software, Ltd.  All rights reserved.
//  Copyright (c) 1994-1995 Allegro Systems, Ltd.  All rights reserved.
//
//
//  Written by John R. Goltz and Bruce R. Nevins
//
//  Edit History:
//  -------------
//  05/17/89(brn) - Add support for cmdver
//  12/05/89(brn) - remove reference to intscreen, remove errno
//  03/28/94(brn) - define blkungetc and add saved char to block header
//  04/01/94(brn) - Remove definition for getsingle
//
//------------------------------------------------------------------------------

#ifndef __SHELL_H__

#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <xossignal.h>
#include <stdlib.h>
#include <string.h>
#include <xos.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xostime.h>
#include <global.h>
#include <xosstr.h>
#include <errno.h>

#define TRUE  1
#define FALSE 0

#define CMDMAX      512		// Maximum length of single command line
#define FILESPECMAX 256		// Maximum length of a file spec

#define MAXARGS     100		// Maximum number of command arguments
#define MAXCHILD    4		// Maximum number of children
#define CMDLVL      3		// Maximum command file levels
#define HISTDEFAULT 200		// Default size of history buffer
#define HISTMAX     1024	// Maximum size of the history buffer
#define TRMCLASS    0x4D5254

#define uchar  unsigned char
#define ushort unsigned short
#define uint   unsigned int
#define ulong  unsigned long

typedef struct argb_ ARGB;
struct argb_
{	ARGB *next;
	char  data[20];
};

typedef struct cmdstate_ CMDST;
struct cmdstate_
{	CMDST *prev;
	FILE  *datain;
	FILE  *cmdin;
	long   cmdout;
	long   cmderr;
	FILE  *savein;
	long   saveout;
	long   saveerr;
	uchar  batch;
	uchar  echo;
	uchar  xxx1;
	uchar  xxx2;
	ARGB  *args;
};

typedef struct
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  sigEIP;
    long  sigCS;
    long  sigEFR;
    long  sigGS;
    long  sigFS;
    long  sigES;
    long  sigDS;
    short sigcnt;
    short signum;
    long  pid;
    long  term;
    long  pEIP;
    long  pCS;
    long  pEFR;
    long  poffset;
    long  pseg;
    long  data;
} SIGDATA;

// Define structure which contains child process data

typedef struct
{   long state;			// Process state
    long pid;			// Process ID
    long trm;			// Termination type
    long status;		// Termination status
    long pEFR;			// Child's EFR
    long pEIP;			// Child's EIP
    long pCS;			// Child's CS
    long data;			// Data item
    long offset;		// Offset part of address
    long segment;		// Segment part of address
} CDATA;


typedef struct hist_ HIST;
struct hist_
{	HIST *next;
	HIST *prev;
	long  num;
	uchar len;
	char  txt[1];
};

extern char prgname[];

extern QAB   inpqab;
extern HIST *histfirstblk;
extern HIST *histcurblk;
extern HIST *histlastblk;
extern int   histmax;

extern char  cmdbfr[];

extern CMDST *ccs

;;;;extern uchar xosarg;

extern CDATA childdata;
extern long  childflag;
extern uchar flags;

extern int   majversion;
extern int   minversion;
extern char *builddate;
extern char *applist;

// define values for flags

#define FLG_HAVECC 0x80		// Have seen ^C
#define FLG_HOLDCC 0x01		// Need to hold of handling ^C


void   childsignal(SIGDATA sigdata);
void   childreport(CDATA *pnt);

///char   chkcmdc(void);

///int    cmdalssrch(void);

///int    cmdchngdrv(char *drive);

///int    cmdhstsrch(void);

///int    cmdmode(void);

int    cmdbatopt(char *args);
int    cmdcd(char *args);
int    cmdcls(char *args);
int    cmdexit(char *args);
int    cmddelay(char *args);
int    cmdlogout(char *args);
int    cmdcall(char *args);
int    cmdecho(char *args);
int    cmdfor(char *args);
int    cmdgoto(char *args);
int    cmdhistory(char *args);
int    cmdif(char *args);
int    cmdjump(char *args);
int    cmdpause(char *args);
int    cmdrem(char *args);
int    cmdreturn(char *args);
int    cmdshift(char *args);
int    cmdver(char *args);

void   cmdlevel(void);
void   cmnderror(long code, char *name);
void   cntlcsignal(void);
int    docmd(char *cmd);
char  *expbatarg(char *txt, int argnum);
int    fileerr(char *fsp, long code);
void   findapps(void);

///long   gethexval(char *str);
///int    getkeyword(FIRSTBLK *blkptr);
///long   getline(void);

char  *getonearg(char *args);
int    getonenum(char *cmd, long *valp);
long   getpath(char *name, char *buffer, int len, long filoptn);
ulong  getsyschar(char *str);
void   hungupsignal(void);
void   killdet(void);
char  *insertbatarg(char *pnt, int *avail, int num);
void   nextfromhistory(void);
long   pathfunc(long options, char *name, char *buffer, int length);
void   prevfromhistory(void);
void   prompt(int prmtype);
void   putcmdc(char chr);
int    putinhistory(char *txt, int len);
void   resetscrn(void);
void   retrievefromhistory(char *cmd);
void   setupbatargs(char *pnt, int size);
void   showprompt(void);
int    srchatom(char *atom, struct command *table, short size);
///int    do_function_key(void);
///int    errpause(void);

#endif
