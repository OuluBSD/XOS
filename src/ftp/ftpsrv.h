#define MAJVER  1
#define MINVER  0
#define EDITNUM 0

#define CMDTDB   0x00800000
#define TMRTDB   0x00820000

#define ver ver2(MAJVER,MINVER,EDITNUM)
#define ver2(a,b,c) ver3(a,b,c)
#define ver3(maj,min,edit) #maj "." #min "." #edit

#define INSTMAX  15				// Maximum instance number

#define IDBBASE  0x08000000
#define CDBBASE  0x00006200		// Base offset for first CDB
#define MAXOPENS 30
#define DFTOPENS 3
#define MAXCONS  4095
#define DFTCONS  20

#define DATAFUNC_QUIT  1
#define DATAFUNC_ABORT 2
#define DATAFUNC_CLOSE 3
#define DATAFUNC_PASV  4
#define DATAFUNC_LIST  5
#define DATAFUNC_NLST  6
#define DATAFUNC_RETR  7
#define DATAFUNC_STOR  8


typedef struct
{	int   len;
	char *str;
} cstr;

typedef union
{	long v;
	struct
	{	ushort editnum;
		uchar minor;
		uchar major;
	};
} VERSION;

#define DEFCSTR(label,str) cstr label = {sizeof str -1, str}

typedef struct
{   text8_parm  class;
    byte4_parm  filoptn;
    lngstr_parm filspec;
    byte4_parm  ipaddr;
    byte4_parm  tcpport;
    byte4_parm  conlimit;
    char        end;
} TCPOPENPARMS;

typedef struct _cdb CDB;
typedef struct _idb IDB;

// Define the CDB (Connection Data Block)

struct _cdb
{	THDDATA tdb;
	IDB    *idb;
	int     slot;
	union
	{	long  v;
		uchar c[4];
	}       rmtaddr;			// Client IP address
	union
	{	long  v;
		uchar c[2];
	}       rmtcmdport;			// Client command port
	union
	{	long  v;
		uchar c[2];
	}       rmtdataport;		// Client data port
	union
	{	long  v;
		uchar c[2];
	}       lcldataport;		// Server data port
	long    dtdb;				// Offset of data thread TDB
	long    cmdhndl;			// Handle for command connection
	long    pasvhndl;			// Handle for connection queue device for
								//   passive open
	long    datahndl;			// Handle for data connection
	long    diskhndl;			// Handle for disk file or directory
	int     datafunc;			// Function passed to data thread
	long    datadone;			// Response from data thread
	char   *dataerror;			// Error message returned by data thread
	char   *homdirstr;			// Home directory string (allocated)
	int     homdirlen;			// Lenght of home directory string
	char   *curdirstr;			// Current directory string (allocated)
	int     curdirlen;			// Length of current directory string
	char   *rnfrstr;			// RNFR file specification (allocated)
	uchar   loggedin;			// TRUE if user logged in
	uchar   allfiles;			// All-files mode state:
								//   0 = Not allowed
								//   1 = Allowed but not enabled
								//   2 = Enabled
	uchar   datacon;			// Data connection state
	uchar   terminate;			// TRUE if terminating client
	uchar   dataabort;			// TRUE if aborting data transfer
	uchar   xxx[3];
	time_sz filedt;				// File date and time
	time_s  connectdt;			// Connected date and time
	char    username[20];		// Logged in user name
	char    specbufr[600];		// File specification buffer
};

// Define values for datacon in CDB

#define DATACON_IDLE       0
#define DATACON_WAITING    1
#define DATACON_CONNECTING 2
#define DATACON_PASSIVE    3
#define DATACON_ACTIVE     4
#define DATACON_ERROR      5

// Define the IDB (Instance Data Block)

struct _idb
{	THDDATA tdb;
	char  devname[32];
	union
	{	long  v;
		uchar c[4];
	}     ipaddr;
	union
	{	long  v;
		uchar c[2];
	}     cmdport;
	union
	{	long  v;
		uchar c[2];
	}     dataport;
	int   instance;				// Instance number
	int   numclients;			// Current number of client connections
	int   maxclients;
	long  cdbfree;				// Index for first free CDB
	uchar nologin;
	uchar xxx[3];
	CDB  *cdbtbl[1];			// CDB table
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
    long  intEIP;
    long  intCS;
    long  intEFR;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    short intcnt;
    short intnum;
    long  qab;
} INTDATA;


extern cstr    nullmsg;
extern cstr    msg150a;
extern cstr    msg200;
extern cstr    msg202a;
extern cstr    msg202b;
extern cstr    msg202c;
extern cstr    msg220a;
extern cstr    msg220b;
extern cstr    msg220c;
extern cstr    msg220d;
extern cstr    msg221;
extern cstr    msg226;
extern cstr    msg230a;
extern cstr    msg230b;
extern cstr    msg230c;
extern cstr    msg250a;
extern cstr    msg250b;
extern cstr    msg250c;
extern cstr    msg250d;
extern cstr    msg331;
extern cstr    msg350;
extern cstr    msg452;
extern cstr    msg500a;
extern cstr    msg500b;
extern cstr    msg500c;
extern cstr    msg501a;
extern cstr    msg501b;
extern cstr    msg502;
extern cstr    msg503a;
extern cstr    msg503b;
extern cstr    msg503c;
extern cstr    msg504a;
extern cstr    msg530;
extern cstr    msg533;
extern cstr    msg550a;
extern cstr    msg550b;
extern cstr    msg550c;

extern char    xosrootstr[];
extern int     xosrootlen;

extern char    devname[];
extern int     instance;
extern long    cmdport;
extern long    dataport;
extern int     maxclients;

extern TCPOPENPARMS
               tcpopenparms;
extern long    tcphndl;
extern long    cmdtdb;
extern VERSION xosver;

extern uchar   debugflg;
extern uchar   nologin;

void  canceldevice(long hndl);
int   checkpriv(char *data, char *name);
void  commandthread(IDB *idb, CDB *cdb, long slot, long hndl, long ipaddr,
		long port);
void  cmddone(CDB *cdb, long code);
void  cmdresponse(CDB *cdb, cstr msg);
void  datafunc(CDB *cdb, int func);
void  datathread(CDB *cdb);
void  datawait(CDB *cdb);
void  debug(int slot, char *label, int len, char *fmt, ...);
void  docommand(CDB *cdb, char *pnt);
void  ftpcmdabor(CDB *cdb, char *pnt);
void  ftpcmdacct(CDB *cdb, char *pnt);
void  ftpcmdallo(CDB *cdb, char *pnt);
void  ftpcmdappe(CDB *cdb, char *pnt);
void  ftpcmdcdup(CDB *cdb, char *pnt);
void  ftpcmdcwd(CDB *cdb, char *pnt);
void  ftpcmddele(CDB *cdb, char *pnt);
void  ftpcmdhelp(CDB *cdb, char *pnt);
void  ftpcmdlist(CDB *cdb, char *pnt);
void  ftpcmdmkd(CDB *cdb, char *pnt);
void  ftpcmdmode(CDB *cdb, char *pnt);
void  ftpcmdnlst(CDB *cdb, char *pnt);
void  ftpcmdnoop(CDB *cdb, char *pnt);
void  ftpcmdpass(CDB *cdb, char *pnt);
void  ftpcmdpasv(CDB *cdb, char *pnt);
void  ftpcmdport(CDB *cdb, char *pnt);
void  ftpcmdpwd(CDB *cdb, char *pnt);
void  ftpcmdquit(CDB *cdb, char *pnt);
void  ftpcmdrein(CDB *cdb, char *pnt);
void  ftpcmdrest(CDB *cdb, char *pnt);
void  ftpcmdretr(CDB *cdb, char *pnt);
void  ftpcmdrmd(CDB *cdb, char *pnt);
void  ftpcmdrnto(CDB *cdb, char *pnt);
void  ftpcmdrnfr(CDB *cdb, char *pnt);
void  ftpcmdsite(CDB *cdb, char *pnt);
void  ftpcmdsmnt(CDB *cdb, char *pnt);
void  ftpcmdstat(CDB *cdb, char *pnt);
void  ftpcmdstor(CDB *cdb, char *pnt);
void  ftpcmdstou(CDB *cdb, char *pnt);
void  ftpcmdstru(CDB *cdb, char *pnt);
void  ftpcmdsyst(CDB *cdb, char *pnt);
void  ftpcmdtype(CDB *cdb, char *pnt);
void  ftpcmduser(CDB *cdb, char *pnt);
void  ftpcmdxafl(CDB *cdb, char *pnt);
void  getatom(char **ppnt, char *atom, int len);
int   getbyteval(char **ppnt, uchar *val, int stopper);
int   getdirspec(CDB *cdb, char *pnt, char *buffer, int length);
int   getfiledatetime(CDB *cdb, char **pnt);
int   getfilespec(CDB *cdb, char **ppnt, char *buffer, int length, int *status);
void  logerror(int slot, long code, char *fmt, ...);
void  passiveopendone(CDB *cdb, long code);

int   requireend(CDB *cdb, char *pnt);

long  setupdatacon(CDB *cdb);
void  sigcmdclosedone(INTDATA idata);
void  sigcmdconnectdone(INTDATA idata);
void  sigcmdinputdone(INTDATA idata);
void  sigcmdoutputdone(INTDATA idata);
void  sigdataclose(INTDATA idata);
void  sigdatadone(INTDATA idata);
void  sigdiskclose(INTDATA idata);
void  sigdiskdone(INTDATA idata);
void  tcpthread(void);
void  tellworkingdir(CDB *cdb);
void  terminate(CDB *cdb);
char *xoscode(long code);
void  xoserror(CDB *cdb, long code, char *msg);
