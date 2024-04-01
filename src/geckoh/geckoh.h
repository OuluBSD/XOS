#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <xosx.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoswildcard.h>
#include <xoserrmsg.h>
#include <progarg.h>
#include <proghelp.h>
#include <heapsort.h>
#include <dirscan.h>
#include <global.h>
#include <xosstr.h>
#include <xossignal.h>

#define MAJVER  1
#define MINVER  2
#define EDITNUM 0

#define VECT_CONINP 71
#define VECT_USBINP 72

#define VRTLIN 0xB3		// Character to use as vertical line in dump output
#define BRDATR 0x07		// Attribute byte for border
#define INVBRD 0x70
#define TXTATR 0x02		// Attribute byte for normal text
#define INVATR 0x20		// Attribute byte for inverse text
#define ERRATR 0x0C		// Attribute byte for error message text

#define CCHR  (' '+TXTATR*0x100)	// Character + attributes to clear screen
#define VBCHR (0xB3+BRDATR*0x100)	// Character + attributes for vert border
#define HBCHR (0xCD+BRDATR*0x100)	// Character + attributes for horiz border

#define CMD_NORMAL 0x80
#define CMD_F1     0x81


#define DBREQ_NOOP    0x00
#define DBREQ_STATE   0x01
#define DBREQ_LKELIST 0x02
#define DBREQ_RDMEM   0x03
#define DBREQ_WRTMEM  0x04
#define DBREQ_RDREG   0x05
#define DBREQ_WRTREG  0x06
#define DBREQ_RDIO    0x07
#define DBREQ_WRTIO   0x08
#define DBREQ_BPCTRL  0x09
#define DBREQ_BPLIST  0x0A
#define DBREQ_WPCTRL  0x0B
#define DBREQ_WPLIST  0x0C
#define DBREQ_XCTRL   0x0D
#define DBREQ_GETVA   0x0E
#define DBREQ_GETPA   0x0F
#define DBREQ_RDPCI   0x10
#define DBREQ_WRTPCI  0x11
#define DBREQ_ERROR   0x1F

#define DBRSP_STATE   0x21


#define ENTRY_CALLED 1
#define ENTRY_BKTPNT 2
#define ENTRY_WCHPNT 3
#define ENTRY_SINGLE 4
#define ENTRY_UNXPBP 5
#define ENTRY_UNXWP  6
#define ENTRY_UNXTP  7


typedef _Packed struct
{	char  *name;
	uchar  inx;
	uchar  size;
	short  offset;
} REGNTBL;

#define REG_EAX  1
#define REG_EBX  2
#define REG_ECX  3
#define REG_EDX  4
#define REG_ESP  5
#define REG_EBP  6
#define REG_EDI  7
#define REG_ESI  8
#define REG_CS   9
#define REG_SS   10
#define REG_DS   11
#define REG_ES   12
#define REG_FS   13
#define REG_GS   14
#define REG_EIP  15
#define REG_EFR  16

#define REG_LDTR 17
#define REG_TR   18
#define REG_GDTR 19
#define REG_IDTR 20
#define REG_CR0  21
#define REG_CR2  22
#define REG_CR3  23
#define REG_CR4  24
#define REG_MSW  25
#define REG_DR0  26
#define REG_DR1  27
#define REG_DR2  28
#define REG_DR3  29
#define REG_DR6  30
#define REG_DR7  31
#define REG_TR6  32
#define REG_TR7  32
#define REG_ST0  34
#define REG_ST1  35
#define REG_ST2  36
#define REG_ST3  37
#define REG_ST4  38
#define REG_ST5  39
#define REG_ST6  40
#define REG_ST7  41


///#define VAL_NUMERIC 0
///#define VAL_ADDR    1


typedef union
{	long l;
	_Packed struct
	{	ushort edit;
		uchar  min;
		ushort maj;
	};
} TARGET;


typedef _Packed struct
{	char  name[4][3];
	uchar bit;
	uchar mask;
} FLGS;

typedef _Packed struct
{	int omode;
	int oradix;
	int iradix;
} MODE;

typedef _Packed struct
{	long   val;
	ushort seg;
	ushort xxx;
} VAL;


// Format of symbol table entry in memory:
// 	Length   (1 byte)   = Length of symbol name (in bytes)
// 	Flags    (1 byte)   = Symbol flags:
// 				SF$MLT = 40h - Symbol is multiply defined
// 				SF$ADR = 20h - Symbol is an address

// 				SF$SUP = 10h - Symbol is suppressed
// 				SF$GLB = 08h - Symbol is global
// 				SF$MOD = 02h - Symbol is a module name


// 				SF$V86 = 01h - Symbol is a V86 address

// 	Offset   (4 bytes)  = Offset part of value
// 	Selector (2 bytes)  = Selector part of value
// 	Name     (variable) = Symbol name

#define SYMMAX 128

typedef _Packed struct
{	uchar  length;
	uchar  flags;
	long   value;
	uchar  name[SYMMAX];
} SYM;

#define SF_ADR 0x20		// Value is an address
#define SF_SUP 0x10		// Symbol is suppressed
#define SF_GBL 0x08		// Symbol is global
#define SF_MOD 0x02		// Symbol is a module name
#define SF_REL 0x01		// Symbol is relocatable


#define OM_NUMERIC  1
#define OM_SYMBOLIC 2
#define OM_CHAR     3

typedef _Packed struct
{	uchar  enttype;
	uchar  xxx[3];
	struct
	{	ushort edit;
		uchar  min;
		uchar  maj;
	}      version;
	ushort lkeinmem;
	ushort lkeloaded;
	long   pid;
	long   eax;
	long   ebx;
	long   ecx;
	long   edx;
	long   esp;
	long   ebp;
	long   edi;
	long   esi;
	ushort cs;
	ushort ss;
	ushort ds;
	ushort es;
	ushort fs;
	ushort gs;
	long   eip;
	long   efr;
} STATERP;

typedef _Packed struct
{	ushort number;
	uchar *codeaddr;
	uchar *dataaddr;
	uchar  namelen;
	char   name[1];
} LKEITEM;

typedef _Packed struct
{	ushort  number;
	ushort  lketop;
	LKEITEM data[1];
} LKELISTRP;

typedef _Packed struct
{	ushort count;
	uchar  size;
	long   addr;
	uchar  type;
	uchar  data[1];
}     RDMEMRP;

// Define bits for type in RDMEMRP
//   If SGTP_CODE and SGTP_DATA are both 0, the address is flat if SGTP_16BIT
//   is clear or is a real-mode address if SGTP_16BIT set.

#define SGTP_CODE  0x04		// Code segment
#define SGTP_DATA  0x02		// Data segment
#define SGTP_16BIT 0x01		// 16-bit segment or real-mode address

typedef _Packed struct
{	uchar reg;
}     WRTREGRP;

typedef _Packed struct
{	uchar  size;
	ushort port;
	long   value;
}	  RDIORP;

typedef _Packed struct
{	uchar  size;
	ushort port;
}	  WRTIORP;

typedef _Packed struct
{	long addr;
	long value;
}	  RDPCIRP;

typedef _Packed struct
{	long addr;
}	  WRTPCIRP;

typedef _Packed struct
{	uchar func;
	long  addr;
	long  count;
}     BPCTRLRP;

typedef _Packed struct
{	long addr;
	long count;
	long pid;
}     BPITEM;

typedef _Packed struct
{	BPITEM item[1];
}     BPLISTRP;

typedef _Packed struct
{	uchar  func;
	uchar  size;
	long   addr;
	ushort count;
}     WPCTRLRP;

typedef _Packed struct
{	uchar  type;
	uchar  size;
	long   addr;
	ushort count;
	long   pid;
}     WPITEM;

typedef _Packed struct
{	WPITEM item[1];
}     WPLISTRP;

typedef _Packed struct
{	uchar  func;
	union
	{	long addr;
		long count;
	};
	ushort seg;
}     XCTRLRP;

typedef _Packed struct
{	long paddr;
	long vaddr[1];
}     GETVARP;

typedef _Packed struct
{	long vaddr;
	long paddr;
}     GETPARP;

typedef _Packed struct
{	uchar func;
	long  code;
	long  detail;
}     ERRORRP;

#define RSPBFRSZ 2048

typedef union
{	uchar     c[RSPBFRSZ];
	STATERP   state;
	LKELISTRP lkelist;
	RDMEMRP   rdmem;
	WRTREGRP  wrtreg;
	RDIORP    rdio;
	WRTIORP   wrtio;
	RDPCIRP   rdpci;
	WRTPCIRP  wrtpci;
	BPCTRLRP  bpctrl;
	BPLISTRP  bplist;
	WPCTRLRP  wpctrl;
	WPLISTRP  wplist;
	XCTRLRP   xctrl;
	GETVARP   getva;
	GETPARP   getpa;
	ERRORRP   error;
}     RSPBFR;

typedef _Packed struct
{	ushort number;
} LKELISTRQ;

typedef _Packed struct
{	ushort count;
	uchar  size;
	long   addr;
}     RDMEMRQ;

typedef _Packed struct
{	uchar  size;
	long   addr;
	uchar  data[1];
}     WRTMEMRQ;

typedef _Packed struct
{	uchar reg;
}     RDREGRQ;

typedef _Packed struct
{	uchar reg;
	long  value;
}     WRTREGRQ;

typedef _Packed struct
{	uchar  size;
	ushort port;
}	  RDIORQ;

typedef _Packed struct
{	uchar  size;
	ushort port;
	long   value;
}	  WRTIORQ;

typedef _Packed struct
{	long addr;
}	  RDPCIRQ;

typedef _Packed struct
{	long addr;
	long value;
}	  WRTPCIRQ;

typedef _Packed struct
{	uchar  func;
	long   addr;
	long   count;
}     BPCTRLRQ;

typedef _Packed struct
{	uchar  func;
	uchar  size;
	long   addr;
	ushort count;
}     WPCTRLRQ;

typedef _Packed struct
{	uchar func;
	long  addr;
	long  count;
}     XCTRLRQ;

typedef _Packed struct
{	long paddr;
}     GETVARQ;

typedef _Packed struct
{	long vaddr;
}     GETPARQ;

typedef union
{	uchar     c[128];
	LKELISTRQ lkelist;
	RDMEMRQ   rdmem;
	WRTMEMRQ  wrtmem;
	RDREGRQ   rdreg;
	WRTREGRQ  wrtreg;
	RDIORQ    rdio;
	WRTIORQ   wrtio;
	RDPCIRQ   rdpci;
	WRTPCIRQ  wrtpci;
	BPCTRLRQ  bpctrl;
	WPCTRLRQ  wpctrl;
	XCTRLRQ   xctrl;
	GETVARQ   getva;
	GETPARQ   getpa;
}     REQBFR;

typedef _Packed struct
{	union
	{	long    value;
		ushort  word;
		_Packed struct
		{	uchar bytel;
			uchar byteh;
		};
	};
	ushort *spos;
	char    name[4];
} RINFO;

typedef struct
{	RINFO eax;
	RINFO ebx;
	RINFO ecx;
	RINFO edx;
	RINFO esp;
	RINFO ebp;
	RINFO edi;
	RINFO esi;
	RINFO cs;
	RINFO ss;
	RINFO ds;
	RINFO es;
	RINFO fs;
	RINFO gs;
	RINFO eip;
	RINFO efr;
} REGS;


// Define values returned by command functions

#define CMD_CMD    1
#define CMD_ROPEN  2
#define CMD_MOPEN  3
#define CMD_PAUSED 4
#define CMD_WAIT   5
#define CMD_WWTO   6


#define CCX_EB  0
#define CCX_EW  1
#define CCX_EL  2
#define CCX_EI  3
#define CCX_OB  4
#define CCX_OW  5
#define CCX_OL  6


#define DMEM_DUMPDATA 0x10
#define DMEM_DUMPINST 0x20
#define DMEM_DISPDATA 0x30
#define DMEM_DISPINST 0x40
#define DMEM_DISPPREV 0x50
#define DMEM_OPENDATA 0x60
#define DMEM_DISPLIST 0x70
#define DMEM_DUMPLST1 0x80
#define DMEM_DUMPLST2 0x90

#define DMEM_INST16   0x00
#define DMEM_INST32   0x01

#define DMEM_BYTE     0x00
#define DMEM_WORD     0x01
#define DMEM_LONG     0x02
#define DMEM_LLONG    0x03

// Define function values for XCTRL requests

#define XCTRL_START    1
#define XCTRL_PROCEED  2
#define XCTRL_SINGLE   3
#define XCTRL_BOOT     4

extern int    curcmdinx;
extern uchar *cmdpnt;
extern uchar  cmdline[200];
extern uchar  cmdstate;
extern uchar  localsym;
extern uchar  entrytype;
extern uchar  doingdump;
extern uchar  hvgdrsp;
extern uchar  debugout;
extern uchar  havenoop;
extern uchar  suspaddr;
extern uchar  showgmn;
extern FLGS   flgtbl[];
extern int    flgtblsz;

extern long   curline;
extern long   curchar;
extern long   dheight;

extern int    dumptype;
extern int    stopper;
extern int    hldchar;
extern int    linenum;

extern MODE   curmode;
extern MODE   prmmode;
extern uchar  rmaddr;

extern int    rsptype;
extern int    rspseq;
extern int    rsplen;
extern int    rspleft;
extern RSPBFR rspbufr;
extern uchar *rsppnt;

extern int    lastlkenum;

extern char  *txtpnt;

extern REQBFR reqbufr;

extern REGS   regtbl;

extern long   curval;
extern SYM   *curmodule;

extern VAL    curaddr;
extern int    cursize;
extern int    curszx;
extern VAL    lstaddr;
extern REGNTBL *curreg;

extern VAL	  listaddr;
extern long   listoffset;
extern long   listcount;
extern long   listsize;

typedef struct symtbl_ SYMTBL;
struct symtbl_
{	SYMTBL *next;			// Pointer to next SYMTBL
	long    count;			// Number of symbols in this SYMTBL
	short   number;			// Number (negative if program, 0 if kernel, or
							//   positive if LKE)
	char    name[118];		// LKE or program name (NULL for kernel)
	SYM     sym[1];			// Symbol entries
};

extern SYMTBL symtbl;

extern TARGET  target;

extern REGNTBL regntbl[];
extern int     regntblsz;

extern int (*contfunc)(void);

extern uchar scrnbufr[];

void  begincmd(void);
void  docommand(void);
void  usbinpsig(void);
void  consoleinit(void);
void  setupfail(long code, char *text);

void  ioerror(long code, char *msg);

int   usbinit(void);
void  syminit(char *path);
void  doresponse(void);
int   sendrequest(int type, int len);
void  topline(void);

void  listsymtbls(void);


void  putchr(int chr);
void  putchrns(ushort *pos, int chr);
void  putstr(uchar *text, ...);
int   linechk(int (*func)(void));
void  putstrns(ushort *pos, uchar *str);
void  putraw(uchar *str, int len);
void  ensureleft(void);
void  clearscreen(void);

int   addsymtbl(char *name, uchar *codeaddr, uchar *dataaddr, int number);

SYM  *findsymbol(char *name, SYM *module);
SYM  *findmodule(char *name);

char *fmtnumeric(char *pnt, long value);
char *fmtvalue(char *valtxt, VAL *value);

int   expression(VAL *value);
int   lastexpression(VAL *value);
int   numeric(long *value);
int   lastnumeric(long *value);
void  echochrs(void);

void unexpected(int type);

SYM *findbestsym(ulong val, SYM **mod);

void removesymtbl(int number, int quiet);

void rsperror(void);
void rspstate(void);
void rsplkelist(void);
void rsprdmem(void);
void rspwrtmem(void);
void rspwrtreg(void);
void rsprdio(void);
void rspbpctrl(void);
void rspbplist(void);
void rspwpctrl(void);
void rspwplist(void);
void rspxctrl(void);
void rspgetva(void);
void rspgetpa(void);
void rsprdpci(void);
int  nxtchar(void);
int  nxtnb0(int chr);
int  nxtnbc(void);

SYM *wildsearch(SYM *sym, char *name, long val, int flags, SYM **mod);
int  setmodule(char *name);

int  getname(int wildok, char *atom, int size);

int  wildlist(void);

int  instdisp(int type, uchar *inst, char *text);

int  modifyreg(void);
int  modifymem(void);
int  showentry(void);

