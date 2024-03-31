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
#include <xossignal.h>
#include <xosacct.h>
#include <xoserr.h>
#include <procarg.h>
#include <xossinfo.h>
#include <lkeload.h>
#include <atomic.h>
#include <xosserverfwt.h>
#include <xosusb.h>

/// #define HDBBASE 0x2400000

#define IOBUFR  0x0100000

#define NUMDEVS 16
#define NUMHUBS 64

extern CRIT cfgcrit;
extern CRIT vencrit;

typedef struct ddb_ DDB;
typedef struct cdb_ CDB;
typedef struct hdb_ HDB;
typedef struct req_ REQ;
typedef struct cfg_ CFG;
typedef struct ifd_ IFD;
typedef struct ept_ EPT;

// Define the CFG (configuration description)

struct cfg_
{	long   label;
	CFG   *next;				// Next CFG for device
	IFD   *fifd;				// First interface for configuration
	uchar  value;				// Value for specifying this configuration
	uchar  attrib;				// Configuration attributes
	ushort maxpower;			// Maximum power taken from bus
	int    index;				// Index (needed for some requests)
	int    numttlifd;			// Total number of IFDs for this configuration
	int    numunqifd;			// Number of unique IFDs
  	char  *desc;				// Configuration description
};

// Define the IFD (interface description)

struct ifd_
{	long   label;
	IFD   *nexta;				// Next alternate IFD for this interface
	IFD   *next;				// Next IFD for this configuration
	EPT   *fept;				// First EPT for this interface
	uchar *hiddesc;				// HID descriptors
	int    altsel;				// Alternate selection value
	int    number;				// Interface number
	int    numept;				// Number of end-points for this interface
	long   seqnum;
	long   error;
	char  *desc;				// Interface description
	uchar  class;				// Interface class
	uchar  subclass;			// Interface sub-class
	uchar  protocol;			// Interface protocol
	uchar  xxx2;

	int    unit;				// XOS unit number for associated device
	int    allocofs;			// Offset for allocated unit numbers
	int    allocfact;			// Factor for allocated unit numbers

	char   name[16];			// Base device name (no unit number or trailing
								//   modifiers)
	char   clss[32];			// Device class
	char  *devlst;
	char  *devtxt;
};

// Define the EPT (end-point description)

struct ept_
{	long   label;
	EPT   *next;				// Next EPT for this interface
	uchar  number;				// End-point number
	uchar  attrib;				// End-point attributes
	ushort maxpktsz;			// Maximum packet size
	int    pktrate;				// Packet rate (interval)
};

// Define the DDB (Device Data Block) - There is a DDB for each seperate
//   USB device known to the system including hubs. There is also a DDB for
//   each USB controller.

struct ddb_
{	long  label;
	CDB  *rootcdb;				// CDB for root controller for device
	union						// Associated CDB or HDB if this device is a
	{	CDB *cdb;				//   controller or hub
		HDB *hdb;
	};
	union						// CDB or HDB which owns this device
	{	CDB *cdb;
		HDB *hdb;
	}     owner;
	int   addr;					// USB address for device
	long  gport;				// Global port for this device
	long  lport;
	long  seqnum;
	long  error;
	int   portshift;			// Number of bits to shift local port number
	uchar speed;
	uchar xxx1[3];
	int   numcfgs;				// Number of configurations
	CFG  *fcfg;					// First CFG for device
	CFG  *curcfg;				// Current CFG for device
	IFD  *fifd;					// First IFD for current configuration
	int   maxpktsz0;			// Maximum packet size for end-point 0
	int   usbspec;
	int   usbven;				// USB vendor code
	int   usbdev;				// USB device code
	int   usbrel;				// USB device release level
	char *usbmnftr;				// Manufactor string from device descriptor
	char *usbprod;				// Product string from device descriptor
	char *usbsernm;				// Serial number string from device descriptor
	uchar class;				// USB class from configuration descriptor
	uchar subclass;				// USB subclass from configuration descriptor
	uchar protocol;				// USB protocol from configuration descriptor
	uchar xxx2;
	char *devtxt;
};


// Define the HDB (Hub Data Block) - There is one HUB for each hub. It contains
//   links to the DDBs for devices connected to the hub and the information
//   needed to communicate with the hub.

struct hdb_
{	long   label;
	DDB   *ddbtbl[16];			// DDBs for devices connected to hub
	DDB   *ddb;					// DDB for this device
	long   addrmap;
	int    numports;			// Number of ports on hub
	long   portsts[16];			// Port status
	long   retrycnt[16];
	long (*fncresetport)(HDB *hdb, int port);
	long (*fncdisableport)(HDB *hdb, int port);
	long   usbhndl;				// Handle for USB device for interrupt pipe
	long   tid;
	char   type[4];
								// End of common part
	int	   intpktrate;			// Packet rate for interrupt pipe
	uchar  intmaxpktsz;			// Maximum packet size for interrupt pipe
	uchar  intendpnt;			// End-point number for interrupt pipe
	ushort attrib;
	uchar  on2good;
	uchar  contrcurrent;
	uchar  removable[4];
	uchar  portpower[4];
};

// Define the CDB (Controller Data Block) - There is one CDB for each USB
//   controller in the system. It also serves as the HDB for the virtual root
//   hub associated with the controller.

struct cdb_
{	long     label;
	DDB     *ddbtbl[16];		// DDBs for devices connected to hub
	DDB     *ddb;				// DDB for this device
	long     addrmap;
	int      numports;			// Number of ports on hub
	int      portsts[16];		// Port status
	int      retrycnt[16];
	long   (*fncresetport)(HDB *hdb, int port);
	long   (*fncdisableport)(HDB *hdb, int port);
	long     usbhndl;
	long     tid;
	char     type[4];
								// End of common part
	long   unit;
	long   ready;
	long   error;
	long   dbntmr;				// Handle for debounce timer
	int    vector;
	int    irq;
	int    ioreg;
	ushort resetting;			// Bit n set if resetting port n
	char   started;
	char   debugport;
	CRIT   lock;	

///	REQ   *reqhead;
///	REQ   *reqtail;
};

// Define the request structure - this is used to queue requests to a
//   hub thread

#if 0
struct req_
{	long label;
	REQ *next;
	HDB *hdb;
	int  func;
	int  lport;
	long data;
};

// Define request functions:

#define REQ_STATUS 1			// Status from port
#define REQ_TIMER  2			// Timer

#endif

typedef _Packed struct
{	TEXT4PARM class;
	uchar     end;
} SPECPARMS;

typedef struct
{	char  name[20];
	char  func[32];
	char  drvr[4][32];
	char  clss[32];
	char  dtype[12];
	int   bimin;
	int   bimax;
	int   bomin;
	int   bomax;
	int   iimin;
	int   iimax;
	int   iomin;
	int   iomax;
	int   unbase;
	int   uninc;
} DRV;

// Define the NAMEDEF (name definition) - This block is used to manage device
//   numbers. 

typedef struct namedef_ NAMEDEF;
struct namedef_
{	NAMEDEF *next;
	char     name[16];
	long     mask;
};

extern SPECPARMS specparms;
extern NAMEDEF  *namehead;
extern FILE     *debugfile;
extern char      debugname[128];

extern long  dskanumber;
extern long  ptranumber;
extern long  kbdanumber;
extern long  mseanumber;

extern long  unitnum;
extern long  vector;
extern long  ready;
extern long  cmdfdb;
extern long  usbhndl;
extern DDB  *roottbl[];

extern long  startcnt;

extern long  memtotal;
extern long  memblks;
extern long  numactive;
extern long  ttlactive;
extern long  dvseqnum;
extern long  ifseqnum;

extern uchar debugflg;

extern char  debugbase[];
extern char *debugdefer;
extern long  debugsize;

extern char *cmdpntr[2];

extern char  lkename[];

long   addtotext(char **pnt, char *txt);
int    allocunitnumber(char *clsname);
long   hidbootsetup(CDB *cdb, DDB *ddb, IFD *ifd, char *classname,
		char *devname, char *devtype, char *lke1, char *lke2);
int    checklke(char *name);
void   cleanddb(DDB *ddb);
void   cmdactive(char *cmd, int len);
void   cmdlog(char *cmd, int len);
void   cmdretry(char *cmd, int len);
void   cmdstatus(char *cmd, int len);
void   debugmem(void);
void   debugprint(char *fmt, ...);
void   deleteusbdevice(HDB *hdb, long port);
void   deverror(DDB *ddb, long code, char *fmt, ...);
long   dskasetup(CDB *cdb, DDB *ddb, IFD *ifd);
void   dumpit(uchar *data, int size);
long   dosetup(CDB *cdb, DDB *ddb, int size, int lowspeed, int addr,
		int endpnt, int type, int request, int value, int index,
		uchar *buffer, int length);
int    findvendor(ulong ven, ulong dev, DRV *drv);
int    fncunit(arg_data *arg);
long   genericsetup(CDB *cdb, DDB *ddb, IFD *ifd, DRV *drv);
int    getaddress(CDB *cdb);
void   giveaddress(CDB *cdb, int addr);
void   giveunitnumber(char *clsname, int number);
long   hubresetport(HDB *hdb, int port);
long   hubsetup(CDB *cdb, DDB *ddb, IFD *ifd);
void   hubterminate(HDB *hdb);
void   hubthread(DDB *ddb, int hubnum);
void   logerror(long code, char *fmt, ...);
long   makeddb(HDB *hdb, long lport, DDB **pddb);
void   parsehid(uchar *bufr, int left);
void   procargerror(char *msg1, char *msg2);
long   ptrasetup(CDB *cdb, DDB *ddb, IFD *ifd);
void   removexosdevice(HDB *hdb, long port);
long   servercmd(char *symname, char *msgdst, char *msgtext, int msglen);
void   setupdevice(HDB *hdb, long port, int speed);
long   setupinterface(DDB *ddb, IFD *ifd, DRV *drv);
void   setupfail(long code, char *fmt, ...);
long   rootdisableport(HDB *hdb, int lport);
long   rootresetport(HDB *hdb, int lport);
long   rootthread(long unitnum, long usbhndl, long vector);
void   usbfree(void *pnt);
uchar *usbmalloc(int size);
