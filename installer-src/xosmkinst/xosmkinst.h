#define WINVER 0x0500

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <time.h>
#include <dos.h>
#include <io.h>
#include <winioctl.h>
#include <process.h>
#include <commdlg.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "xosxdfs.h"
#include "swcwindows.h"

#pragma pack (push)
#pragma pack (1)

typedef struct devitem_ DEVITEM;
struct devitem_
{	DEVITEM *next;
	uint     dnum;
	long     mask;
	llong    length;
};

#define RESERVED 200	// Total number of reserved blocks
#define RBLKBOOT 10		// First block used for bootstrap
#define MAXL2LEN ((RESERVED-RBLKBOOT-3)*512)

#define DSKBUFRSZ ((RESERVED-1) * 512)
							//   cluster we can create
#define DSKBUFRNM (RESERVED-1)


#define XIDBUFRSZ 0x4000
#define XISBUFRSZ 0x4000

typedef struct
{	char  label[4];
	char  hlen;
	char  fmtver;
	short fill;
	llong cdt;
	ulong totalsize;
	ulong totalitems;
	ulong crcvalue;
} XIDHEAD;

typedef struct
{	ushort sync;
	uchar  seqnum;
	uchar  flag;
	ushort namelen;
	ulong  filelen;
	llong  cdt;
} ITEMHEAD;

// Define values for flag in ITEMHEAD

#define IF_COPY 0x0080		// Copy to boot disk
#define IF_ISET 0x0040		// Add to install set
#define IF_TYPE 0x000F		// Item type

// Define item types (low 4 bits of flag in ITEMHEAD

#define IT_BSTRP  0
#define IT_OSISET 1
#define IT_APISET 2
#define IT_DTISET 3
#define IT_RTDIR  8
#define IT_SBDIR  9
#define IT_FILE   10
#define IT_END    15

typedef struct
{	ITEMHEAD h;
	char name[300];
} HEADBUFR;

extern int      stsnum;
extern int      targetnum;
extern HANDLE   xidhndl;
extern HANDLE   xishndl;
extern HANDLE   phyhndl;
extern HANDLE   hndltbl[25];
extern HANDLE   waithndl;
extern HANDLE   xSishndl;
extern uint     devnum;
extern long     devbits;
extern llong    devsize;
extern int      drvltr;
extern uchar   *dskbufr;
extern llong    tzone;
extern char     xisname[300];
extern char     filespec[512];
extern char     xidspec[512];
extern ulong    crctable[256];
extern HEADBUFR itemhead;
extern XIDHEAD  xishead;
extern uchar   *bootimg;
extern uchar    mbrblk[512];
extern uchar    bootblk[512];
extern uchar    terminate;
extern HANDLE   installthd;
extern uint     installid;

extern __int64 curadone;
extern __int64 adone;
extern __int64 amount;

#pragma pack (pop)

void install();
uint __stdcall installthread(void *arg);
uint __stdcall scandisksthread(void *arg);

int createdir(char *spec);
int  copyitems();
int  fmtfat32();
int  finishxid();
int  finishxis();
int  initmbr();
int  makeinstallsets();
int  mkboot();
int  readitemhead();
long readxid(uchar *bufr, long size);
void seterrmsg(char *fmt, ...);
void setmsg(char *fmt, ...);
int  skipdata();
int  startxid();
int  startxis(char *name, llong cdt);
long writexis(uchar *bufr, long size);
void zlibfail(int code, char *fmt, ...);
