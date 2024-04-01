//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

typedef struct pda__   PDA;
typedef struct tda__   TDA;
typedef struct tmr__   TMR;
typedef struct slb__   SLB;
typedef struct iorrb__ IORRB;
typedef struct irqrb__ IRQRB;
typedef struct rmb__   RMB;
typedef struct sesdb__ SESDB;
typedef struct ecdb__  ECDB;
typedef struct mdb__   MDB;
typedef struct hdlb__  HDLB;

// Define structure for the Timer Data Block (TMR)

struct tmr__
{	long  label;				// Label = 'TMR*'
	TMR  *next;					// Address of next TMR in the timer list
	PDA  *pda;					// Actual address of the associated PDA
	long  handle;				// Timer handle
	llong dttm;					// Date/time when this TMR expires (system ticks)
	llong repeat;				// Repeat interval (system ticks)
	void *disp;					// Address of function to call at end of interval
	long  data1;				// First data value
	long  data2;				// Second data value
};

;Define structure for the Signal Link Block (SLB)

struct slb__
{	long  label;				// Label = 'SLB*'
	SLB  *next;					// Address of next SLB for this level
	SLB  *prev;					// Address of previous SLB for this level
	long  vector;				// Vector number (user) or offset (exec)
	char  dcnt;					// Data count
	char  fill[3];
	void *gfunc;				// Address of grant function
	long  gdata;				// Data value for grant function
	long  data[];				// Data items (up to 9)
};

// Define function values passed to the signal grant subroutine

#define SGF_GRANTED 0			// Signal granted
#define SGF_CLEARED 1			// Signal ignored because cleared
#define SGF_NOVECT  2			// Signal ignored because no vector
#define SGF_ALLCLR  3			// All signals cleared

;Define structure for the IO Register Registration Block (IORRB)

struct iorrb__
{	long   label;				// Label = 'IORB'
	long   next;				// Address of next IORRB
	ushort first;				// First IO register in block
	ushort last;				// Last IO register in block
	char   name[16];			// Name of device
iorrb_SIZE=!$$$

;Define structure for the interrupt request registeration block (IRQRB)

struct irqrb__
{	long   label;				// Label = 'IRQB'
	IRQRB *next;				// Address of next IRQRB
	char   bits;				// Bits
	char   number;				// Interrupt number
	char   fill[2];
	long   entry;				// Entry offset
	void   exit;				// Address of subroutine to store exit link
	char   name[16];			// Name of device
irqrb_SIZE=!$$$

// Define bits for irqrb_bits

#define IRQRB_SHAREPCI 0x80		// Interrupt is a sharable PCI interrupt

// Define structure for the signal vector table entry (SVT)

struct svt__
{	void  *addr;				// Address of service routine
	ushort gtnum;				// Bits 15-12: Signal level
								// Bits 11- 0: Global thread number for target
								//               thread
	ushort ltseq;				// Local thread sequence number for target
								//   thread
};

// Define structure for the RMB (Resource Management Block) used to manage
//   critical resources

// NOTE: Since this block is always embedded in another blocks, it does not
//       start with a label.

struct rmb__
{	TDA   *waittda, 4			// Actual address of TDA for first thread
								//   waiting for this resource
	TDA   *locktda, 4			// Actual address of TDA with exclusive access
								//   to this resource
    char   slock;				// Spin lock byte
	char   status;				// Resource status bits
	ushort usecnt;				// Use count
};

// Define bits rmb_status

#define RMBS_WLOCK 0x01	// One or more threads are waiting to lock the resource

// Define bits for the bits argument of the sysSchGetResource call

#define RMBB$NOWAIT 0x04		// Do not wait for resource
#define RMBB$LOCKED 0x02		// Get resource locked
#define RMBB$UPGRD  0x01		// Upgrate current use to locked (must have
								//   resource for normal use)


// Define structure for an Event Cluster Definition Block (ECDB)

struct ecdb__
{	long  label;				// Label = 'ECD*'
	ECDB *next;					// Address of next block
	char  name[16];				// Name of event cluster
	char  max;					// Maximum event number for cluster
	char  baseint;				// Base interrupt for cluster
	char  fill[2];
	long  reserve;				// Reserve bits
	long  events[];				// First event table entry
};

// Define structure for the session data block

// NOTE: This has never been fully implimented!

struct sesdb__
{	long label;					// Label = 'SESD'
	char  sesseq[16];			// Session sequence number
	char  grpname[16];			// Group name
	char  usrname[16];			// User name
	char  actdev[16];			// Name of accounting device
	char  access[4];			// Access class name
	long  actnode;				// Accounting node
	long  actport;				// Accounting port
	long  uprgvrsn;				// User program version number
	char  portname[16];			// Acces port name
	char  useraddr[32];			// User address
	llong billtime;				// Last date/time billing time added in
	char  billcls;				// Billing class
	char  numper;				// Number of time periods in use
	char  billlvl;				// Current billing level
	char  fill;
	long  billkey;				// Billing key value
	llong atime1;				// First accounting time break
	llong atime2;				// Second accounting time break
	llong atime3;				// Third accounting time break
	long  rates1[16];			// First rate values
	long  rates2[16];			// Second rate values
	long  rates3[16];			// Third rate values
ses_SIZE=!$$$

// Define structure for the Msect Data Block (MDB)

struct mdb__
{	long  label;				// Label = MDB*
	MDB  *next;					// Address of next MDB for same segment
	long  base;					// Address of start of this msect
	long  size;					// Size of this msect (in memory pages)
	PDA  *pda;					// Actual address of PDA
	char  devtype;				// Type of device which has this msect mapped
	char  sts;					// Status bits
	char  fill[2];
	void *remove;				// Address of subroutine to call when removing
	union						//   msect
	{ MDB *shrnext;				// Address of next MDB for shared section
	  MDB *devnext;				// Next MDB mapped for same device
	};
	union
	{ SDA *shrsda;				// Actual address of SDA for shared msect
	  DCB *devpnt;				// Address of data block for device which has
	};							//   this msect mapped
	long lockcnt;				// Number of locked pages in the section
								//   (shared sections only)
	void *fault;				// Address of function to call on page faults
};

// Define bits for mdb_sts

#define MSS$FIXED    0x40		// Fixed size msect
#define MSS$NODEALOC 0x20		// Do not deallocate pages

// Define structure for the Held Device List Block

struct hdlb__
{	long label;					// Label = 'HDLB'
	HDLB *nextp;				// Address of next HDLB for same process
	PDA  *pda;					// Actual address of the session PDA
	HDLB *nexts;				// Address of next HDLB in system
	DCB  *dcb;					// Address of device DCB
	long  count;				// Time-out counter
};

// Define structure for the Process Data Area (PDA). The minimum PDA is one
//   page (4KB). It is expanded if more space is needed for the device handle
//   table (HndlTbl).

struct pda__
{	long    Label;				// Label ('PDA&')
	SPY    *Spy;				// Addres of process spy data
	PDA    *Addr;				// Actual address for this PDA
	long    Pid;				// Process ID
	char    Status1;			// First process status byte
	char    Status2;			// Second process status byte
	char    Status3;			// Third process status byte
	char    fill1;
	long    CR3;				// Value for CR3
	long    WSLimit;			// Working set size limit
	long    WSAllow;			// Working set size allowed
	long    TMLimit;			// Total user memory limit
	long    TMAllow;			// Total user memory allowed
	long    UMLimit;			// Protected mode memory limit
	long    UMAllow;			// Protected mode memory allowed
	long    OMLimit;			// Overhead memory limit
	long    OMAllow;			// Overhead memory allowed

	llong   AvlPriv;			// Available privilage bits
	llong   ActPriv;			// Active privilege bits

	long    FileProt;			// Default file protection value
	long    DirProt;			// Default directory protection value

	long    WPHits;				// Watchpoint hit bits
	long    WPEnable;			// Enable mask for local watchpoints

	long    Country;			// Country code

	TDA    *TdaHead;			// Actual address of first (base) TDA for process
	TDA    *TdaTail;			// Actual address of last TDA for process

	long    TdaNum;				// Number of TDAs
	long    TdaLimit;			// Maximum number of TDAs

	SECT   *Section;			// Address of section list data block
	long    Junk;

	LNB    *Lnb;				// Address of first local logical name block
	RMB     LnbRmb;				// RMB for the LNB list (must follow pdaLnb)
	PDB    *PhyPath;			// Address of first physical device path name
								//   block
	RMB     PhyPRmb;			// RMB for the physical device path list (must
								//   follow pdaPhyPath)
	PDB    *LogPath;			// Address of first logical device path name block
	RMB     LogPRmb;			// RMB for the logical device path list (must
								//   follow pdaLogPath)

	SVT     SigVTbl[256];		// Signal vector table
	SESDB  *SesData;			// Address of session data block

	char    PLevel;				// Process level
	char    fill2[3];

	long    AlrmCnt;			// Number of active alarms

	llong   CDateTime;			// Date/time process was created
	PDA    *SesPda;				// Actual address of PDA for session process
	PDA    *ParPda;				// Actual address of PDA for parent process
	PDA    *SibPda;				// Actual address of PDA for next sibling of this
								//   process
	PDA    *ChildPda;			// Actual address of PDA for last child of this
								//   process
	char   *Top;				// Highest allocated address for PDA + 1
	ECDB   *Events;				// Address of 1st event cluster definition block
	MDB    *FirstMdb;			// Address of first direct allocation MDB

	long    LdtSel;				// Selector for the LDT
	char   *LdtPnt;
	long    LdtSize;

	MLTBL   MLockTbl;			// Memory lock table
	HDLB   *HdlbList;			// Pointer to first HDLB for process
	long    HndlNum;
	HTENT   HndlTbl[IHNDLNUM+1];
};

;Define structure for the TDA (Task Data Area). Each TDA consists of a minimum
;  of 2 pages (8KB). This includes the thread's exec stack, the thread's TSS,
;  and all thread local exec data. Up to an additional 8KB may be used if it
;  is needed for an IO permission bit map. There is always one selector
;  associated with each TDA. This is a TSS selector that specifies the
;  thread's TSS. Since XOS does not use the hardware task switch mechanism,
;  this TSS is almost unused but is still needed to specify the initial level
;  0 stack pointer and an IO bit map (if one is used).

;TDAs are used with both user and IO threads. (In previous versions of XOS
;  Extended Fork Frames (XFFs) took the place of IO threads.)

;Since IO threads are created and destroyed often, a small number of idle TDAs
;  are kept in a free list to minimize the cost of creating and destroying
;  a thread.

struct tda__
{	char    stk[0x1A00-84]
	union
	{	long XStkBegin;
		long URegGS;			// Saved user GS
	};
	long    URegFS;				// Saved user FS
	long    URegES;				// Saved user ES
	long    URegDS;				// Saved user DS
	long    URegEDI;			// Saved user EDI
	long    URegESI;			// Saved user ESI
	long    URegEBP;			// Saved user EBP
	long    URegxxx;			// Saved user dummy ESP
	long    URegEBX;			// Saved user EBX
	long    URegEDX;			// Saved user EDX
	long    URegECX;			// Saved user ECX
	long    URegEAX;			// Saved user EAX
	long    URegEIP;			// Saved user EIP
	long    URegCS;				// Saved user CS
	long    URegEFR;			// Saved user EFR
	long    URegESP;			// Saved user ESP
	long    URegSS;				// Saved user SS
	union
	{	long XStackP;			// Process exec stack top for protected mode
		long URegVES;			// Saved user ES (V86 mode only)
	};
	long    URegVDS;			// Saved user DS (V86 mode only)
	long    URegVFS;			// Saved user FS (V86 mode only)
	long    URegVGS;			// Saved user GS (V86 mode only)e

	// Following items are preserved across putting the TDA on the free list.

	union
	{	long XStackV;			// Process exec stack top for real mode
		long Label;				// Label ('TDA&')
	};
	PDA    *Addr;				// Actual address of this TDA
	long    Tid;				// Slot number and sequence number for the thread
								//   (Thread ID)
	long    Spy;				// Addres of thread spy data
	long    PtaPTP0;			// First per-thread area page table pointer
	long    PtaPTP1;			// Second per-thread area page table pointer
	long    PtaPTPT;			// Per-thread area user page page table pointer
	long    TssSel;				// Selector for the TSS

	// End of items preserved across putting the TDA on the freelist.

	// The following items are used in all TDAs

	PDA    *Pda;				// Actual address of the PDA
	char    Type;				// TDA type
	char    fill1[3];
	TDA    *Next;				// Actual address of next TDA for process
	TDA    *Prev ;				// Actual address of previous TDA for process

	char    TSts1;				// First thread status byte
	char    TSts2;				// Second thread status byte
	char    TSts3;				// Third thread status byte
	char	SSigType;			// System signal type bits

	long    Tss;				// Start of task state segment for process
	long    TssESP0;
	long	TssSS0;
	long    TssESP1;
	long	TssSS1;
	long    TssESP2;
	long	TssSS2;
	long    TssCR3;

	// The following items occupy unused TSS locations

	union
	{
	  long fill1[17];
	  struct
	  { long  SRegESP;			// ESP value while thread not running
		long  QueNext;			// Actual address of next TDA in scheduler queue
		long  QuePrev;			// Actual address of previous PDA in scheduler
								//   queue
		long  State;			// Current thread state

		char  SigReq;			// Signal request bits
		char  SigMask;			// Signal request mask
		char  SigLvl;			// Current signal level
		char  ResWType;			// Resource wait type

		llong RunTime;			// Total run-time (clock ticks)
		long  InQueTime;		// In-queue run time remaining (clock ticks)
		long  WaitIndex;		// Thread wait index
		long  WaitNext;			// Next TDA in the wait hash list
	  };
	long    TssIOBM;

	// This is the end of the TSS

	// The following items are used during IO operations and may be used by
	//   either user or IO threads

	long    TmOutSub;			// Offset of subroutine to call on time-out
	long    Cmd;				// Device command bits
	long    Delay;				// IO delay value (system ticks)
	llong   TimeOut;			// IO time-out value (system ticks)

	void   *Adb;				// Address of additional data block for device

	long    CLockCnt;			// Number of locked disk cache buffers

	// The following locations are an image of the first part of an IOQB. These
	//   item are copied to or from the IOQB and must be in the same order as
	//   the corresponding items in the IOQB

	union
	{ long Ioqb;
	  long IoqbLabel;			// 'IOQB'
	};
	QAB    *Qab;				// Address of user's QAB

	// The following items match both the IOQB and the QAB

	ushort  Func;				// IO function bits
	ushort  Status;				// IO status bits
	long    Error;
	long    Amount;				// Amount transfered
	long    Handle;				// Device handle
	char    Vector;	
	char    rsvd1[3];
	long    Option;				// Options or command
	long    Count;				// Buffer 1 size
	char   *Buffer1;			// Address of user's first buffer
	void   *Buffer2;			// Address of user's second buffer
	void   *Parm;				// Address of user's parameter list

	// End of QAB items

	char    Frame;				// Offset of dcb_inpframe or dcb_outframe
								//   relative to the DCB
	char    QSts1;				// Status bits
	char    fill2[2];
	DCB    *Dcb;				// Address of DCB for device
	TDA    *UserTda;			// Address of TDA for user thread that started IO
	void   *Routine;

	// End of items copied to or from the IORB

	long    RtnValue0;			// Returned value for IO operation
	long    RtnValue1;
	long    RtnValue2;
	long    RtnValue3;
	long    RtnValue4;

	long    DevAmnt;			// Amount transfered by device

	char    DevName[16];		// Device name buffer

	union
	{ char *BgnClr;				// Start of items to clear when starting IO
	  SLB  *CloseSlb;			// Address of close SLB (Used when opening a
	};							//   spooled defice - We should find a better
								//   place to put this!)
	long    PVFilOpt;			// File option bits
	long    PPFilSpc;			// File specification buffer address
	long    PLFilSpc;			// File specification length word address
	long    PCFilSpc;			// File specification byte count
	long    PXFilSpc;			// File specification saved byte count
	long    PSFilSpc;			// File specification buffer size
	long    HvValue1;			// Value flag bits
	long    HvValue2;
	long    NdValue1;
	long    NdValue2;

	char    FilAttr;			// Value of IOPAR_FILATTR
	char    SrcAttr;			// Value of IOPAR_SRCATTR
	char    CurAttr;			// Current file attribute bits
	char    fill3;
	long    DirHndl;
	long    DirOfs;
	long    IoPos;
	long    GrpSize;
	long    ReqAlloc;
	long    RqrAlloc;
	long    Length;
	long    Prot;
	llong   ADate;
	llong   CDate;
	llong   MDate;

	long    OpenDisp;			// Dispatch for doopen
	ushort  Vect1Num;
	ushort  Vect2Num;
	PDA    *Vect1Pda;
	PDA    *Vect2Pda;
	long    ConLimit;			// Value of IOPAR_NETCONLIMIT
	long    ConHndl;			// Value of IOPAR_NETCONHNDL
	long    Prot;				// Value of IOPAR_PROT
	llong   LockVal;			// Value of IOPAR_SETLOCK or IOPAR_CLRLOCK
	long    ShrParms;			// Value of IOPAR_SHRPARMS
	long    AvlSpace;			// Value of IOPAR_AVLSPACE
	long    TtlSpace;			// Value of IOPAR_TTLSPACE
	char    ClsSize;			// Value of IOPAR_CLSSIZE
	char    FSType;				// Value of IOPAR_FSTYPE
	char    fill4[2];
	ushort  SectSize;			// Value of IOPAR_SECTSIZE
	ushort  NumSect;			// Value of IOPAR_NUMSECT
	ushort  NumHead;			// Value of IOPAR_NUMHEAD
	ushort  NumCyln;			// Value of IOPAR_NUMCYLN

	char    InpQLmt;			// Value of IOPAR_INPQLMT
	char    OutQLmt;			// Value of IOPAR_OUTQLMT
	char    BufrLmt;			// Value of IOPAR_BUFRLMT
	char    fill5;
	long    Protocol;			// Value of IOPAR_NETPROTOCOL (network only)
	long    LclPort;			// Value of IOPAR_NETLCLPORT (network only)
	long    RmtPortS;			// Value of IOPAR_NETRMTPORTS (network only)
	long    RmtNetAS;			// Value of IOPAR_NETRMTNETAS (network only)
	ushort  KATime;				// Value of IOPAR_NETKATIME (network only)
	ushort  PktMax;				// Value of IOPAR_NETPKTMAX (network only)
	ushort  LRmtAddrS;			// Length of IOPAR_NETRMTADDRS string
	char    fill6[2];
	long    VRmtAddrS;			// Pointer to IOPAR_NETRMTADDRS string
	union
	{ long  SRmtAddrS;			// Pointer to IOPAR_NETRMTADDRS size value
	  char  LastClr[1];
	};

	char    Owner[36];

	TDA    *WaitList;			// Address of next TDA waiting for a buffer
	BFR    *WaitBufr;			// Address of buffer we are waiting for
	TMR    *WakeTmr;			// Offset of time-out timer block

	// End of items used in all TDAs

	// The following items are only used with user threads

	// WARNING: The FpuState state item MUST be aligned on a 16-byte boundry.
	//          Unfortunately there is no way to do this automatically so the
	//          numberof items before this must be such as to insure this!
	//          An error message will be generated when compiling if this
	//          condition is not meet.

	FPUSS   FpuState , 512t		// FPU state save area
	char    FpuUsed  , 1		// Non-zero if FPU has been used
	char    fill6[3];

	// SVC arguments are copied to the following items. Some of them are also
	//   overloaded to hold values for some routines that do not use all of the
	//   arguments.

	union
	{ long  Ccb;				// Address of Ccb (4 bytes)
	  long  Arg10;				// Tenth SVC argument
	};
	union
	{ char *CharData;			// Global data item used when processing
								//   characteristics (4 bytes)
	  long  Arg9;				// Ninth SVC argument
	};
	union
	{ char  CharValue[16];		// Characteristics value buffer (16 bytes)
	  struct
	  {	long Arg8;				// Eight SVC argument
		long Arg7;				// Seventh SVC argument
		long Arg6;				// Sixth SVC argument
		long Arg5;				// Fifth SVC argument
	  };
	};
	long    Arg4;				// Fourth SVC argument
	long    Arg3;				// Third SVC argument
	long    Arg2;				// Second SVC argument
	long    Arg1;				// First SVC argument
	long    ArgCnt;				// Arguement count for SVC
	long    ArgPnt;				// Arguement pointer for SVC (points to first
								//   argument on user's stack)

	// The following items are the head and tail pointers for the signal queues

	long    L1HeadP;			// Level 1 signal list head pointer
	long    L2HeadP;			// Level 2 signal list head pointer
	long    L3HeadP;			// Level 3 signal list head pointer
	long    L4HeadP;			// Level 4 signal list head pointer
	long    L5HeadP;			// Level 5 signal list head pointer
	long    L6HeadP;			// Level 6 signal list head pointer
	long    L7HeadP;			// Level 7 signal list head pointer
	long    L8HeadP;			// Level 8 signal list head pointer
	long    L1TailP;			// Level 1 signal list tail pointer
	long    L2TailP;			// Level 2 signal list tail pointer
	long    L3TailP;			// Level 3 signal list tail pointer
	long    L4TailP;			// Level 4 signal list tail pointer
	long    L5TailP;			// Level 5 signal list tail pointer
	long    L6TailP;			// Level 6 signal list tail pointer
	long    L7TailP;			// Level 7 signal list tail pointer
	long    L8TailP;			// Level 8 signal list tail pointer
SET1 xxTP, tdaL1TailP-tdaL1HeadP

	long    URegDR0;			// Virtual DR0
	long    URegDR1;			// Virtual DR1
	long    URegDR2;			// Virtual DR2
	long    URegDR3;			// Virtual DR3
	long    URegDR6;			// Virtual DR6
	long    URegDR7;			// Virtual DR7
	long    URegCR2;	
	long    TrapErr;	

	long    VRegEFR;			// Virtual EFR value

	SLB     TrapSlb;			// Partial SLB used when granting a trap
	long    TrapVect;	
	long    TrapData;	
	long    TrapCode;	

	char    TermEvNum;			// Event number for process termination
	char    fill7[2];
	char    CritState;			// Thread's critical region state
	TDA    *CritNext;			// Actual address of TDA for next thread in
								//   critical region wait list

	long    TermEvPid;			// PID for termination event
	long    TermSigPid;			// PID for termination signal
	long    TermSts;			// Termination status value
	long    TermData;			// Termination data value
	long    Response;			// Response value
	long    ErrCode;			// Hardware error code value
	long    ThrdData;			// User thread data (stored in hum_thrddata when
								//   thread is running

	// End of items used with user threads.

	// The following items should be used with IO threads only to manage queued
	//   IO requests

	long    IoQHead;			// IO queue head pointer
	long    IoQTail;			// IO queue tail pointer
	char    IoQCnt;				// IO queue count
	char    fill8[3];
	long    SpoolUnit;
	long    IoBMTop;			// Hightest available IO bitmap address + 1
};

#if 0

;The IO bit map starts just before the beginning of the third page. One or two
;  additional pages are allocated when an IO bitmap is needed. Note that the
;  IO bitmap pages are only mapped at their actual addresses. Given how seldom
;  they are accessed, it is not worth the overhead to doubly map them. (The
;  overhead is significant since the second mapping has to be modified for
;  each page on every context switch!)

SET0 $$$, 0x2000-4
	long    IoBitMap , 0		// IO permission bitmap

	.ENDM
.PAGE
	.MACRO	sdadef
SET0 $$$, 0x1000
DEF Magic    , 4		// Magic number which identifies an SDA
DEF Sid      , 4		// Segment ID
DEF Next     , 4		// Selector for next SDA
DEF Type     , 1		// Segment type
SKIP           3
DEF UseCnt   , 4		// Use count
DEF Level    , 4		// Compatibility level
DEF Name     , 48t		// Name of this segment
DEF MdbHead  , 4		// Offset of first MDA for this segment
DEF MdbTail  , 4		// Offset of last MDA for this segment
DEF TotalSize, 4		// Total length of the segment (pages)
DEF SdaSize  , 4		// Length of this SDA (pages)
DEF Prot     , 4		// Protection value
DEF ACList   , 4		// Offset of start of access control list
DEF FPnt     , 0		// First memory pointer for segment
	.ENDM

#endif
