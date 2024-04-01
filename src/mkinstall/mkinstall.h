#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <xos.h>
#include <xossvc.h>
#include <xoslib.h>
#include <xosrtn.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <xcstring.h>
#include <errno.h>
#include <zlib.h>
#include <xosxid.h>

#define INBUFRSZ  0x4000
#define XIDBUFRSZ 0x5000

typedef struct dirb_ DIRB;
typedef struct subb_ SUBB;
typedef struct filb_ FILB;

_Packed struct dirb_
{	DIRB *next;
	int   linenum;
    SUBB *subhead;
    FILB *filehead;
	FILB *xcldhead;
	FILB *gblxhead;
	char *tgtname;
	int   flag;
	int   len;
	llong cdt;
	char  srcname[];
};

_Packed struct subb_
{	SUBB *next;
	int   linenum;
    FILB *filehead;
	FILB *xcldhead;
	char *tgtname;
	int   flag;
	int   len;
	llong cdt;
	char  srcname[];
};

_Packed struct filb_
{	FILB *next;
	int   linenum;
	char *tgtname;			// Destination name
	int   flag;
	int   len;
	char  srcname[];
};

// Define extended bits for flag (Bits for the low order 8 bits are defined
//   in xosxid.h.)

#define IF_ACOPY 0x8000		// Copy all files in directory to boot disk
#define IF_AISET 0x4000		// Add all files in directory to install set


extern char     prgname[];

extern char     bootstrapsrc[256];
extern char     bootstrapdflt[256];
extern long     bootstraptime;

extern char    *inpbufr;
extern char    *xidbufr;
extern z_stream stream;
extern FILE    *ctlfile;
extern char    *bootspec;
extern int      boottime;
extern char    *osspec;
extern int      seqnum;
extern DIRB    *dirhead;
extern XIDHEAD  xidhead;
extern ulong    crctable[256];
extern char     hviset;
extern int      spininx;
extern long     lastspin;

extern char     zippath[512];
extern char    *zipdir;
extern char    *zipsbdir;
extern char    *zipfile;


void  inserror(char *msg, int linenum);

void *allocmem(int size);

void  fail(long code, char *fmt, ...);
void  readins(void);
void  findfiles(void);
long  putinxid(char *bufr, long size);

void  showall(void);
void  showfiles(FILB *gblx, FILB *file, FILB *xcld, char *indent);
void  store1item(char *fspec, char *ispec, int iflags, llong cdt, DIRB *dir);
void  storeitems();
