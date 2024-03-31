#define MAJVER  1
#define MINVER  0
#define EDITNUM 0

#define __DEBUG 0

#define CMDTDB   0x00800000
#define TMRTDB   0x00820000

#define ver ver2(MAJVER,MINVER,EDITNUM)
#define ver2(a,b,c) ver3(a,b,c)
#define ver3(maj,min,edit) #maj "." #min "." #edit

#define INSTMAX  15			// Maximum instance number

#define OUTBFRSZ 4096

#define IDBBASE  0x08000000
#define IDBINC   0x00020000
#define IDBSHIFT 17

#define VECT_CLOSE  69
#define VECT_SERVER 70

typedef union
{	long v;
	struct
	{	ushort editnum;
		uchar minor;
		uchar major;
	};
} VERSION;

// Define the file queue entry

typedef struct qent_ QENT;
struct qent_
{	QENT  *next;
	long   seqnum;
	time_s dt;
	int    held;
};

// Define the IDB (Instance Data Block)

typedef struct idb_ IDB;
struct idb_
{	THDDATA tdb;
	char    splname[32];
	char    ptrdev[32];
	int     instance;			// Instance number
	long    ptrhndl;
	char   *ptrlang;
	char    jobname[32];
	QENT   *qhead;
	long    outcnt;
	char   *outpnt;
	char    outbfr[OUTBFRSZ];
};

typedef struct
{	long   regs[15];
	short  intcnt;
	short  intnum;
	long   seqnum;
	IDB   *idb;
	time_s cdt;
} SIGDATA;

extern char    phyname[];
extern char    ptrdev[];
extern char    splname[];
extern long    spllimit;
extern long    itemnum;

extern long    cmdtdb;
extern long    cmdptrhndl;
extern long    cmddirhndl;

extern VERSION xosver;

extern IDB    *instbl[];
extern uchar   debugflg;
extern uchar   debugtrm;
extern FILE   *debugfile;
extern char    debugname[128];

extern uchar   threadready;
extern uchar   prgmode;

extern IDB    *cmdidb;

void  canceldevice(long hndl);
int   checkpriv(char *data, char *name);

///void  debug(int slot, char *label, int len, char *fmt, ...);

///void  logerror(int slot, long code, char *fmt, ...);


void  banner(void);

void  closesignal(SIGDATA sd);

void  cmdadd(char *cmd);

void  cmderror(long code, char *fmt, ...);


void  cmdremove(char *cmd);
void  cmdpause(char *cmd);
void  cmdcontinue(char *cmd);
void  cmdcancel(char *cmd);
void  cmdname(char *cmd);
void  cmdstatus(char *cmd);
void  cmdterminate(char *cmd);
void  debugprint(char *fmt, ...);
int   fncxxxprg(arg_data *arg);
int   fncxxxsplname(arg_data *arg);

void  inthread(void);

void  logcmderror(long code, char *fmt, ...);
void  logerror(long code, char *fmt, ...);

void  outthread(void);

int   procargs(char *cmd, int mode);

long  quefile(IDB *idb, long seqnum, time_s cdt, int held);

void  setupfail(long code, char *fmt, ...);
