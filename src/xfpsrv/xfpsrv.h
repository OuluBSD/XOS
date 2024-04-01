#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <xos.h>
#include <xosstr.h>
#include <xostime.h>
#include <errmsg.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xosacct.h>
#include <xoserr.h>
#include <time.h>
#include <procarg.h>
#include <xosthreads.h>
#include <xosservert.h>
#include <xosswap.h>
#include <xosnet.h>


#define CMDTDB   0x20000000
#define CONTDB   0x20010000
#define TMRTDB   0x20020000
#define FIRSTCDB 0x20080000
#define CDBSIZE  0x80000

#define BUFRSZ    1600
#define PARMSZ    1600
#define STRSZ     1600
#define NUMHNDL   4

#define MAXDATA   1024

typedef struct
{   char    stack[0x6000];
    THDDATA tdb;
    long    slot;
    long    tcphndl;
	union
	{	long n;
		char c[4];
	}		ipaddr;
	long    port;
	uchar   active;
	uchar   xxx[3];
	QAB     diskqab;
	QAB     tcpqab;
	long    fsleft;				// Number of FILSPEC parameter bytes left
	int     fscnt;				// Current FILSPEC byte count
	int     channel;
	long    hndltbl[NUMHNDL];
	long    total[NUMHNDL];
	uchar   ignore[NUMHNDL];
    uchar   buffer[BUFRSZ];
	uchar   parm[PARMSZ];
	uchar   strings[STRSZ];
} CDB;

typedef struct
{	long  high;
    ulong low;
    short tzone;
    short dlst;
} time_nz;

typedef struct
{	ushort length;
    uchar  func;
	uchar  channel;
} XFPHEAD;

extern CDB **clienttbl;
extern long  clientfree;
extern char  devname[];
extern char  tcpdev[];
extern long  tcphndl;
extern long  clientcnt;
extern uchar debug;

void   clientdone(CDB *cdb);
void   clientfail(CDB *cdb, long code, char *fmt, ...);
void   clientthread(CDB *cdb, long slot, long hndl, long ipaddr, long port);
void   debugdump(CDB *cdb, char *lbl, uchar *data, int len);
void   debugout(CDB *cdb, char *lbl, char *fmt, ...);
void   funcclose(CDB *cdb, uchar *msg, int len);
void   funccommit(CDB *cdb, uchar *msg, int len);
void   funcconnect(CDB *cdb, uchar *msg, int len);
void   funcdata(CDB *cdb, uchar *msg, int len);
void   funcdatareq(CDB *cdb, uchar *msg, int len);
void   funcdelete(CDB *cdb, uchar *msg, int len);
void   funcdisconnect(CDB *cdb, uchar *msg, int len);
void   funckeepalive(CDB *cdb, uchar *msg, int len);
void   funcopen(CDB *cdb, uchar *msg, int len);
void   funcparm(CDB *cdb, uchar *msg, int len);
void   funcpassowrd(CDB *cdb, uchar *msg, int len);
void   funcrename(CDB *cdb, uchar *msg, int len);
void   funcsession(CDB *cdb, uchar *msg, int len);
void  *getmem(int size);
int    getvalue(uchar **pmsg, int *plen, int *pval);
void   logerror(long code, char *msg, ...);
void   notify(CDB *cdb, int type, long code, char *fmt, ...);
uchar *putvalue(uchar *pmsg, int val);
void   sendnormrsp(CDB *cdb, long error, long amount, uchar *parm);
long   setupparms(CDB *cdb, uchar *xpnt, int xcnt);
uchar *storeparms(CDB *cdb, uchar *xpnt, uchar *parm);
void   tcpthread(void);
