#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrtn.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <xcstring.h>
#include <errno.h>
#include <zlib.h>
#include <xosxid.h>

#define OUTBUFRSZ 0x5000
#define XIDBUFRSZ 0x4000

extern char     prgname[];
extern char    *outbufr;
extern char    *xidbufr;
extern z_stream stream;

extern XIDHEAD xidhead;
extern ulong   crctable[256];

void  inserror(char *msg, int linenum);

void *allocmem(int size);

void  fail(long code, char *fmt, ...);
