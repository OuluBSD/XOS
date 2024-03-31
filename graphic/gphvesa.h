#define VECT_MESSAGE 50

typedef struct
{	long rgEAX;
	long rgEBX;
	long rgECX;
	long rgEDX;
	long rgEBP;
	long rgEDI;
	long rgESI;
	long rgDS;
	long rgES;
	long rgFS;
	long rgGS;
} REGS;

typedef struct
{	ushort offset;
	ushort segment;
} RPNTR;

typedef struct
{	long label;
	ushort version;
	RPNTR  oemstr;
	uchar  caps[4];
	RPNTR  modelist;
	ushort memsize;
	ushort oemrev;
	RPNTR  venstr;
	RPNTR  prodstr;
	RPNTR  revstr;
	uchar  reserved[222];
} VBEINFO;

typedef struct
{	struct
	{	ushort  modeok:1;
		ushort        :1;
		ushort  ttyout:1;
		ushort   color:1;
		ushort graphic:1;
		ushort  notvga:1;
		ushort  nowinm:1;
		ushort  linear:1;
		ushort dblscan:1;
		ushort intlace:1;
		ushort tripbfr:1;
		ushort  stereo:1;
		ushort dualdsp:1;
	}      attr;
	uchar  winaattr;
	uchar  winbattr;
	ushort wingran;
	ushort winsize;
	ushort winaseg;
	ushort winbset;
	RPNTR  winfunc;
	ushort bpsl;
	ushort xres;
	ushort yres;
	uchar  xcharsz;
	uchar  ycharsz;
	uchar  planes;
	uchar  pixsz;
	uchar  banknm;
	uchar  memmdl;
	uchar  banksz;
	uchar  imgnm;
	uchar  rsvd1;
	uchar  redsize;
	uchar  redpos;
	uchar  grnsize;
	uchar  grnpos;
	uchar  blusize;
	uchar  blupos;
	uchar  rsvd2[2];
	uchar  dircolinfo;
	long   linaddr;
	long   rsvd3;
	ushort rsvd4;
	ushort linlinesz;
	uchar  bnkimgnm;
	uchar  linimgnm;
	uchar  linredsize;
	uchar  linredpos;
	uchar  lingrnsize;
	uchar  lingrnpos;
	uchar  linblusize;
	uchar  linblupos;
	uchar  rsvd5[2];
	long   maxpixclk;
	uchar  rsvd6[189];
} VBEMODE;

// Define bits for attr in VBEMODE

typedef struct mdata_ MDATA;
struct mdata_
{	MDATA *next;
	long   vesamode;
	long   xres;
	long   yres;
	long   bufrpa;
	long   bufrsz;
	long   maxpixclk;
};

#if 0
long v86call(REGS *regs, long stk, long start);

void errmsg(long code, char *msg);
void fail(long code, char *msg, ...);
void msgsignal(void);

void response(char *msg, int len);
#endif
