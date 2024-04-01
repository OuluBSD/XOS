#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <xos.h>
#include <xossvc.h>
#include <xosdisk.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <xosstr.h>
#include <xostime.h>
#include <xostrm.h>
#include <xosfibers.h>
#include <xosxid.h>
#include <bmf.h>
#include <xws.h>
#include <fmtfuncs.h>
#include <diskutilfuncs.h>
#include <mkboot.h>
#include "xosgdp.h"

#define __DISKMAN  0		// Set to 1 to include disk management function

#define OUTBUFRSZ 0x5000
#define XISBUFRSZ 0x4000

#define BTNWIDTH 80000
#define BTNSPACE ((mwsz.xsize - 10000 - 4 * BTNWIDTH)/3)
#define BTNXPOS(N) (5000 + (N-1)*(BTNWIDTH+BTNSPACE))

#define SELXPOS   12000
#define SELXSIZE -12000
#define SELYPOS   29000
#define SELYSIZE -32000

#define TEXTYPOS  11000

#define HEADSIZE  10000

// Define positions in the selection list

#define POS_VENDOR 50000
#define POS_TYPE  100000
#define POS_DEV   110000
#define POS_NDATE 140000 
#define POS_NVER  180000
#define POS_CDATE 230000
#define POS_CVER  270000

// Define colors for the selection list

#define COL_NORMAL 0x00FFFFFF
#define COL_NOSEL  0x00FFFFFE
#define COL_GOOD   0x00A0FFA0
#define COL_ERROR  0x00FFA0A0
#define COL_WARN   0x00FFFFB0
#define COL_FAIL   0x00FF8080
#define COL_DEBUG  0x00E0FFE0
#define TEMP       0x80000000

#define FUNC_SCAN 1
#define FUNC_INST 2
#define FUNC_INIT 3

// Define initial states

#define STATE_ERROR      1
#define STATE_QUESTION   2
#define STATE_INITIALIZE 3
#define STATE_SELECT     4
#define STATE_INST       5
#define STATE_DONE       6

/*

#define STATE_NEWDSK 1		// Disk does not contain a valid MBR
#define STATE_NOPART 2		// Disk has no partitions defined
#define STATE_BDPART 3		// First partition does not contain a DOS16 or
							//   DOS32 file system. It probably contains an
							//   NTFS file system.
#define STATE_HVPART 4		// First partition contains a DOS16 or DOS32 file
							//   system but XOS has not been installed. This
							//   also assumes that the application has not
							//   been installed.
#define STATE_HVXOS  5		// XOS has been installed in the firs partition
							//   but the application has not been installed
#define STATE_HVAPP  6		// The application has been installed but is an
							//   older version.
#define STATE_CURAPP 7		// The installed application is the current
							//   version.

// Define additional states

#define STATE_VRFY 8		// Verifying disk initialization
#define STATE_INIT 9		// Initialize MBR
#define STATE_CREP 10		// Create partition
#define STATE_FMTP 11		// Format partition
#define STATE_COSF 12		// Copy operating system (XOS) files
#define STATE_CAPF 13		// Copy application files
#define STATE_IDLE 14
#define STATE_DONE 15		// Done
#define STATE_SEL  16

*/

#define NAMEMAX 64
#define DIRMAX  128
#define NOTEMAX 1024

typedef union
{	long l;
	struct
	{	uchar  maj;
		uchar  min;
		ushort edit;
	};
} VERNUM;

typedef struct
{	uchar  day;
	uchar  month;
	ushort year;
} DATE;

typedef struct
{	char   vendor[NAMEMAX+4];
	char   program[NAMEMAX+4];
	VERNUM version;
	DATE   date;
	uchar  needboot;
	uchar  xxx[3];
	char   notes[NOTEMAX+4];
} VERSN;

typedef struct vblk_ VBLK;
struct vblk_
{	VBLK  *next;
	VBLK  *sort;
	int    index;
	char   dev[20];			// Name of device containing install set
	char   iset[256];		// Name of the install set file
	char   vname[NAMEMAX+4];// Vendor name
	char   pname[NAMEMAX+4];// Program name
	VERNUM newver;			// New version
	DATE   newdate;			// New date
	VERNUM curver;			// Current version
	DATE   curdate;			// Current date
	char   app;				// TRUE if application
	char   needboot;
};

extern VBLK    *vblkhead;
extern VBLK    *vsel;


extern XWSRECT  mwsz;

extern XWSEDB    edb;
extern XWSWINPAR winparms;
extern XWSTXTPAR txtparms;
extern XWSTXTPAR txtwinb;
extern XWSWIN   *winb;
extern XWSWIN   *winw;
extern XWSWIN   *winlst;
extern XWSWIN   *wininit;

extern XWSWIN   *btnexit;
extern XWSWIN   *btnrescan;
extern XWSWIN   *btncancel;

extern FILE     *logfile;

extern XIDHEAD  xishead;
extern ITEMHEAD itemhead;

extern char *xisbufr;
extern char *outbufr;

extern long  xishndl;
extern long  totalsize;
extern long  crcvalue;
extern int   itemnum;
extern int   seqnum;

extern long      wrkfbr;

extern long  xpos;
extern long  ypos;
extern long  errcode;
extern int   index;
extern int   state;

extern char  dispname[260];
extern char  insttitle[300];
extern int   insttitlelen;

extern char *unitnames;
extern long  numunits;

extern char  tarprtn[20];
extern char  tarbase[20];
extern char  havexos;

XWSEVENT buttonevent;

extern uchar  dobackup;
extern uchar  continueinst;
extern uchar  showdierror;
extern uchar  didinit;

void  clearmain(char *msg);
void  debugtoggle(void);
void  disprescan(void);
void  doinst(void);
void  doinstwork(void);
void  doinit(char *msg, int msglen, VBLK *vpnt);
void  doinitwork(void);
void  doscan(void);
void  doscanwork(void);
void  dowork(int func);
void  error(long code, char *fmt, ...);
void  errorclose(void);
int   fmtdate(char *text, DATE *version);
int   fmtver(char *text, VERNUM *version);
int   getitemhead(void);
long  getinput(char *bufr, long size);
int   getrunver(VERSN *versn, char *spec, char *vname, long version, int quiet);
void  getstate(void);
void  goidle(void);
int   keymakeevent(long charcd, long scancd);
void  puterror(long code, char *fmt, ...);
void  putline(long color, char *fmt, ...);
int   question(long bgcolor, long bdcolor, int chr1, char *txt1, int chr2,
		char *txt2, char *fmt, ...);
long  questionevent(long arg);
void  scanfiber(void);
int   startinput(void);
void  startdebug(void);
void  workfiber(void);
