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

#include <stddef.h>
#include <xos.h>
#include <xoslib.h>
#include <xoserr.h>
#include <xosnet.h>
#include <xosx.h>
#include <xosxlke.h>
#include <xosxchar.h>
#include <xosxparm.h>
#include <xosxpci.h>
#include <xosxnet.h>
#include <xosxen.h>

// 4.0.0 - 25-Sep-10
//		Initial version, created from XOS V3 version with same name. Version
//		number set to indicate XOS V4.

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0

extern char ENRTAINTSZ[];
extern char *enrtaint;

static char haveoas = FALSE;

static INITLKE initenrta;

static OASFNC  enrtaoas;

typedef struct eidb__ EIDB;

static void XOSFNC enrtafork(EIDB *idb);
#pragma aux enrtafork parm [EDI];

static void XOSFNC enrtareset(EIDB *idb);
static void setlinkmode(EIDB *idb);

#pragma data_seg(_HEAD);
	LKEHEAD(ENRTADEV, initenrta, MAJVER, MINVER, EDITNUM, DEVICE); 
#pragma data_seg();

// This driver supports the follow RealTek Ethernet interface chips: 8129,
//   8139, 8139A, 8139AG, 8139B, 8130, 8139C, and 8139C+. The 8139C+ is only
//   supported in "C" mode (non-descriptor mode).

// These are all PCI, 10/100MHz chips. These all have basically the same
//   register interface.

// NOTE: UNDOCUMENTED CHIP BUG - At least some of these chips do not mask off
//       the high order bits from the F_rbput register when using less than
//       a 64KB buffer! The chip otherwise works right, only the specified
//       number of bits are used to index into the ring buffer. When the
//       register is read, however, the higher bits are NOT masked off! The
//       work-around is to mask this register to the width of the counter
//       when reading it.

// NOTE: UNDOCUMENTED "FEATURE" - The chip may update the input putter pointer
//       before completely storing a packet, making it appear that a packet
//       is available before the length has been stored in the header (which
//       is stored last)! In this case the length will have a value of 0xFFF0.

// Define our type value for each supported chip type. These are internal
//   values. They are never visible outside of this driver.

#define RTT_8139        1
#define RTT_8139A       2
#define RTT_8139AG_C    3
#define RTT_8139B_8130  4
#define RTT_8139D_8100B 5
#define RTT_8181        6
#define RTT_8139CP      7
#define RTT_MAX         7

// Define structure which specifies the device registers

typedef volatile struct
{	char   R_netaddr[6];		// 00 Network address
	char   R_xxx[2];			// 06 Reserved
	char   R_multimap[8];		// 08 Multicast map

/// RTD_DTCCMD  =!0x10		;( 8) Dump tally counter command

	long   R_tsts[4];			// 10 Transmit status registers
	long   R_taddr[4];			// 20 Transmit start address registers
	long   R_rbaddr;			// 30 Receive buffer address
	ushort R_erbc;				// 34 Early receive byte count register
	char   R_ers;				// 36 Early receive status register
	char   R_cmd;				// 37 Command register
	ushort R_rbtake;			// 38 Receive buffer taker pointer
	ushort R_rbput;				// 3A Receive buffer putter pointer
	ushort R_intmask;			// 3C Interrupt mask register
	ushort R_intsts;			// 3E Interrupt status register
	long   R_txcfg;				// 40 Transmit configuration register
	long   R_rxcfg;				// 44 Receive configuration register
	long   R_timecnt;			// 48 Timer count register
	long   R_mpc;				// 4C Missed packet count register
	char   R_eepcfg;			// 50 EEPROM command register
	char   R_config0;			// 51 Configuration register 0
	char   R_config1;			// 52 Configuration register 1
	char   R_config2;			// 53 Configuration register 2
	long   R_timeint;			// 54 Timer interrupt register
	char   R_mediasts;			// 58 Media status register
	char   R_config3;			// 59 Configuration register 3
	char   R_config4;			// 5A Configuration register 4
	char   fill0[1];
	ushort R_multint;			// 5C Multiple interupt select
	char   R_pcirev;			// 5E PCI revision ID
	char   fill1[1];			// 5F
	ushort R_tsad;				// 60 Transmit status of all descriptors
	ushort R_bmctl;				// 62 Basic mode control reg
	ushort R_bmsts;				// 64 Basic mode status reg
	ushort R_anadv;				// 66 Auto-negotiation advertisement reg
	ushort R_anlpar;			// 68 Auto-negotiation link partner reg
	ushort R_anexp;				// 6A Auto-negotiation expansion reg
	ushort R_discnt;			// 6C Discount count reg
	ushort R_fsccnt;			// 6E False carrier sense count reg
	ushort R_nwaytst;			// 70 N-way test register
	ushort R_rxercnt;			// 72 RX_ER count register
	ushort R_cscfg;				// 74 CS configuration register
	char   fill2[2];			// 76
	ulong  R_phy1parm;			// 78 PHY parameter 1 register
	long   R_twister;			// 7C Twister parameter register
	char   R_phy2parm;			// 80 PHY parameter 2 register
	char   fill3[1];
	ushort R_tdokla;			// 82 Low address of a TX descriptor
	char   R_pmcrc[8];			// 84 Power management CRCs
	char   R_pmwake0[8];		// 8C Power management wakeup frame 0
	char   R_pmwake1[8];		// 94 Power management wakeup frame 1
	char   R_pmwake2[8];		// 9C Power management wakeup frame 2
	char   R_pmwake3[8];		// A4 Power management wakeup frame 3
	char   R_pmwake4[8];		// AC Power management wakeup frame 4
	char   R_pmwake5[8];		// B4 Power management wakeup frame 5
	char   R_pmwake6[8];		// BC Power management wakeup frame 6
	char   R_pmwake7[8];		// C4 Power management wakeup frame 7
	char   R_pmlsb[8];			// CC Power management LSBs of mask
	long   R_flash;				// D4 Flash memory read/write register
	char   R_config5;			// D8 Configuration register 5
	char   R_txreq;				// D9 Transmit request register !!!!
	ushort R_rxmxpkt;			// DA Maximum received packet size !!!!
	char   fill4[20];			// DC
	long   R_cbfne;				// F0 Cardbus function event register
	long   R_cbevm;				// F4 Cardbus event mask register
	long   R_cbpss;				// F8 Cardbus present state register
	long   R_cbfre;				// FC Cardbus force event register
} REGS;

// Define bits for R_cmd

#define R_CMD_RESET 0x10		// Reset
#define R_CMD_RXE   0x08		// Receiver enable
#define R_CMD_TXE   0x04		// Transfer enable
#define R_CMD_BFRE  0x01		// Receive buffer empty

// Define bits in the transmit descriptor status registers

#define R_TSD_CRS    0x80000000 // Carrier sense lost
#define R_TSD_TABT   0x40000000 // Transmit abort
#define R_TSD_OWC    0x20000000 // Out of window collision
#define R_TSD_CDH    0x10000000 // CD heart beat
#define R_TSD_NCC    0x0F000000 // Number of collisions count
#define R_TSD_ERTXTH 0x003F0000 // Early transmit threshold
#define R_TSD_TOK    0x00008000 // Transmit OK
#define R_TSD_TUN    0x00004000 // Transmit FIFO underrun
#define R_TSD_OWN    0x00002000 // Driver owns descriptor
#define R_TSD_SIZE   0x00001FFF // Buffer size

// Define bits for R_intmask and R_intsts

#define R_INT_SER 0x8000		// System error interrupt
#define R_INT_TO  0x4000		// Time-out interrupt
#define R_INT_CLC 0x2000		// Cable length change interrupt
#define R_INT_RFO 0x0040		// Receive FIFO overflow interrupt
#define R_INT_ULC 0x0020		// Packet underrun/link change interrupt
#define R_INT_RBO 0x0010		// Receive buffer overflow interrupt
#define R_INT_TER 0x0008		// Transmit error interrupt
#define R_INT_TOK 0x0004		// Transmit OK interrupt
#define R_INT_RER 0x0002		// Receive error interrupt
#define R_INT_ROK 0x0001		// Receive OK interrupt

// Define bits for R_rxcfg

#define R_RXC_ERTH     0x0F0000000 // Early Rx threshold bits
#define R_RXC_MULERINT 0x000020000 // Mutilple early interrupt select
#define R_RXC_RER8     0x000010000 // ????
#define R_RXC_RXFTH    0x00000E000 // Receive FIFO threshold
#define R_RXC_RBLEN    0x000001800 // Receive buffer length
#define R_RXC_MXDMA    0x000000700 // Maximum DMA burst size
#define R_RXC_WRAP     0x000000080 // Disable packet wrap in buffer
#define R_RXC_EEPTYPE  0x000000040 // EEPROM type
#define R_RXC_AER      0x000000020 // Accept error packets
#define R_RXC_AR       0x000000010 // Accept runt packets
#define R_RXC_AB       0x000000008 // Accept broadcast packets
#define R_RXC_AM       0x000000004 // Accept multicast packets
#define R_RXC_APM      0x000000002 // Accept physical address match packets
#define R_RXC_AAP      0x000000001 // Accept all packets (promiscuous mode)

// Define bits for R_mediasts

#define R_MEDIASTS_TXFCE   0x80	// Tx flow control enable
#define R_MEDIASTS_RXFCE   0x40	// Rx flow control enable
#define R_MEDIASTS_AUXSTS  0x10	// Aux. power present status
#define R_MEDIASTS_SPEED10 0x08	// Speed is 10MHz
#define R_MEDIASTS_LINKDWN 0x04	// Link is down
#define R_MEDIASTS_TXPF    0x02	// Pause packet sent (input is paused)
#define R_MEDIASTS_RXPF    0x01	// Pause packet received (output is paused)

// Define bits for R_bmctl

#define R_BMCTL_RESET   0x8000	// Reset PHY
#define R_BMCTL_SPEED   0x2000	// Speed set (1 = 100MHz, 0 = 10MHz)
#define R_BMCTL_AUTO    0x1000	// Auto negotiation enable
#define R_BMCTL_RESTART 0x0200	// Restart auto-negotiation
#define R_BMCTL_DUPLEX  0x0100	// Duplex mode (1 = FD, 0 = HD)

// Define bits for R_bmsts

#define R_BMSTS_AUTOCMP 0x0020	// Auto-negotiation is complete
#define R_BMSTS_RMTFLT  0x0010	// Remote fault
#define R_BMSTS_LINK    0x0004	// Link status
#define R_BMSTS_JABBER  0x0002	// Jabber condition detected


#define TX_DMA_BURST   4
#define RX_DMA_BURST   4
#define RX_FIFO_THRESH 7

// Define structure for packets in the input ring buffer

typedef volatile _Packed struct
{	ushort sts;
	ushort len;
	ENPKT  pkt;
} BPKT;

// Define bits in the receive status register in the received packet header

#define R_RSR_MAR  0x8000		// Multicase address received
#define R_RSR_PAM  0x4000		// Physical address received
#define R_RSR_BAR  0x2000		// Broadcast address received
#define R_RSR_ISE  0x0020		// Invalid symbol error
#define R_RSR_RUNT 0x0010		// Runt packet
#define R_RSR_LONG 0x0008		// Long packet
#define R_RSR_CRC  0x0004		// CRC error in packet
#define R_RSR_FAE  0x0002		// Frame alignment error
#define R_RSR_ROK  0x0001		// Receive OK


// Define structure for the IDB for the ENRTA interface

typedef struct eidb__
{	IDB;						// Common part of the IDB
	char    enintreq;			// Interrupt request number
	char    enctype;			// Chip type
	char    enflowctl;
	char    enbufrszx;			// Non-descriptor receive buffer size index
	char    enxmtmax;			// Maximum number of buffered output packets
	char    fill1[3];
	ushort  enbus;				// PCI bus bus number
	ushort  enslot;				// PCI bus slot number
	ushort  enfunc;				// PCI bus function number
	char    enrevsn;			// PCI version ID
	char    fill2[1];
	long    endevice;			// PCI vendor ID and device ID
	PCIB   *enpcib;				// Address of the PCIB (needed for calls to
								//   xospciReadCfgReg and xospciWriteCfgReg)
	REGS   *enregs;				// Base address of the memory mapped registers
	long    enintsts;			// Used to transfer interrupt status to fork
								//   level
	long    enbmctl;			// Current value for R_BMCTL
	char   *enirng;				// Address of beginning of the input ring
								//   buffer
	long    enirngend;			// Byte offset of end of the input ring
	long    enirngphy;			// Physical address of the physical input
								//   descriptor ring if using descriptors or
								//   physical address of the input ring buffer
	long    enitaker;			// Input buffer taker pointer (byte offset into
								//   the virtual input header ring if using
								//   descriptors or into data ring buffer
	long    enphyreg;
	long    enbufrsz;			// Total receive buffer size in KB
	long    enibsmask;			// Input buffer size mask
	long    enocnt;				// Output buffer count
	ENNPB  *enorng[4];			// Ouput buffer pointer ring
	long    enoputter;			// Output buffer ring putter pointer (index)
	long    enotaker;
};

static EIDB *eidbhead;
static EIDB *eidbtail;


NAMETBL2 modetbl[] =
{	{"AUTO"  , R_BMCTL_AUTO},
	{"10HD"  , 0},
	{"10FD"  , R_BMCTL_DUPLEX},
	{"100HD" , R_BMCTL_SPEED},
	{"100FD" , R_BMCTL_SPEED|R_BMCTL_DUPLEX},
	{"1000FD", 0}
};

NAMETBL2 endistbl[] =
{	{"ENABLED" , (R_MEDIASTS_RXFCE << 8) + R_MEDIASTS_TXFCE},
	{"DISABLED", 0}
};


long badpcnt;


// Generate the PCI device id table. This table is only used to determine if
//   the device is a valid type. The determination of the device sub-type is
//   done based on the value of the havedare version ID from the TXCFG register.

long pcitbl[] =
{	0x812910EC,				// RTL 8129
	0x813110EC,
	0x813910EC,				// RTL 8139
	0x013910EC,				// RTL 8139B
	0x810110EC,				// RTL 8101E
	0x816810EC,				// RTL 8168SB
	0x12111113,				// Accton 5030
	0x13601500,				// Delta 8139
	0x13604033,				// Addtron 8139
	0x13001186				// D-Link DFE-530TX+
};

// Generate the table used to identify chips. Value is the expected value
//   for the hardware version id from the chip. Index is the chip type
//   minus 1.

long chiptbl[] =
{	0x60000000,				// RTT_8139        = 1
	0x70000000,				// RTT_8139A       = 2
	0x74000000,				// RTT_8139AG_C    = 3
	0x78000000,				// RTT_8139B_8130  = 4
	0x74400000,				// RTT_8139D_8100B = 5
	0x74C00000,				// RTT_8181        = 6
	0x74800000				// RTT_8139CP      = 7
};

// Generate table which maps from the chip type to a descriptive text string

char *descriptbl[] =
{	"RTL 8139",				// RTT_8139        = 1
	"RTL 8139A",			// RTT_8139A       = 2
	"RTL 8139AG/C",			// RTT_8139AG_C    = 3
	"RTL 8139B/8130",		// RTT_8139B_8130  = 4
	"RTL 8139D/8100B",		// RTT_8139D_8100B = 5
	"RTL 8181",				// RTT_8181        = 6
	"RTL 8139C+",			// RTT_8139CP      = 7
};

// Device characteristics table for ENRTA NIF devices

static CHARFNC chrgetboard;
static CHARFNC chrgetlink;
static CHARFNC chrsetlink;
static CHARFNC chrgettxflow;
static CHARFNC chrsettxflow;
static CHARFNC chrgetrxflow;
static CHARFNC chrsetrxflow;
static CHARFNC chrgetphyaddr;

static const CHARTBL  enrtadctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(CLASS   , TEXT, U,  8, &knlChrMsgClass  , sysChrDevGetClass,
		sysChrDevChkClass, 0),
  CTITEM(TYPE    , TEXT, U,  4, &knlChrMsgType   , sysChrDevGetULong,
		 0               , offsetof(DCB, typename)),
  CTITEM(BOARD   , STR , U, 32, &xosnetMsgBoard  , chrgetboard,
		0                , 0),
  CTITEM(PCIBUS  , DECV, U,  2, &xospciMsgBus    , xosnetGetIdb2,
		0                , offsetof(EIDB, enbus)),
  CTITEM(PCISLOT , DECV, U,  2, &xospciMsgSlot   , xosnetGetIdb2,
		0                , offsetof(EIDB, enslot)),
  CTITEM(PCIFUNC , DECV, U,  2, &xospciMsgFunc   , xosnetGetIdb2,
		0                , offsetof(EIDB, enfunc)),
  CTITEM(PCIVENID, HEXV, U,  2, &xospciMsgVen    , xosnetGetIdb2,
		0                , offsetof(EIDB, endevice)),
  CTITEM(PCIDEVID, HEXV, U,  2, &xospciMsgDev    , xosnetGetIdb2,
		0                , offsetof(EIDB, endevice) + 2),
  CTITEM(PCIREV  , HEXV, U,  1, &xospciMsgRev    , xosnetGetIdb1,
		0                , offsetof(EIDB, enrevsn)),
  CTITEM(IOREG   , HEXV, U,  4, &knlChrMsgIoReg  , xosnetGetIdb4,
		0                , offsetof(EIDB, bioreg)),
  CTITEM(MEMREG  , HEXV, U,  4, &knlChrMsgMemReg , xosnetGetIdb4,
		0                , offsetof(EIDB, enphyreg)),
  CTITEM(INT     , DECV, U,  1, &knlChrMsgInt    , xosnetGetIdb1,
		0                , offsetof(EIDB, intlvl)),
  CTITEM(BUFRSIZE, DECV, U,  4, &xosnetMsgBufrSz , xosnetGetIdb4,
		0                , offsetof(EIDB, enbufrsz)),
  CTITEM(LINK    , TEXT, U, 20, &xosnetMsgLink   , chrgetlink,
		chrsetlink       , 0),
  CTITEM(TXFLOW  , TEXT, U,  8, &xosnetMsgTxFlow , chrgettxflow,
		chrsettxflow     , 0),
  CTITEM(RXFLOW  , TEXT, U,  8, &xosnetMsgRxFlow , chrgetrxflow,
		chrsetrxflow     , 0),
  CTITEM(PHYADDR , HEXB, U,  6, &xosnetMsgPhyAddr, chrgetphyaddr,
		0                , 0),
  CTJUMP(xosnetComChars)
}};

// Characteristics table for addunit

static CHARFNC enrtaaubus;
static CHARFNC enrtaauslot;
static CHARFNC enrtaaufunc;
static CHARFNC enrtaaubufrsz;

CHARITEM enrtaautbl[] =
{ CTITEM(UNIT    , DECV, U,  1, 0, 0, sysChrIgnore , 0),
  CTITEM(TYPE    , TEXT, U,  4, 0, 0, sysChrIgnore , 0),
  CTITEM(BUS     , DECV, U,  4, 0, 0, enrtaaubus   , 0),
  CTITEM(SLOT    , DECV, U,  4, 0, 0, enrtaauslot  , 0),
  CTITEM(FUNC    , DECV, U,  4, 0, 0, enrtaaufunc  , 0),
  CTLAST(BUFRSIZE, DECV, U,  4, 0, 0, enrtaaubufrsz, 0),
};

// Driver function dispatch table

static long  XOSFNC setpkt(IDB *idb, NPB *npb, long etype);
static long  XOSFNC setecho(IDB *idb, NPB *npb);
static long  XOSFNC sendpkt(IDB *idb, NPB *npb, long etype);
static long  XOSFNC recvpkt(IDB *idb,NPB *npb, long len);
static long  XOSFNC getetype();

static void  XOSFNC sethwaddr(IDB *idb, NPB *npb, char addr[6]);
static void  XOSFNC setbcaddr(IDB *idb, NPB *npb);

static llong XOSFNC getsrcphy(IDB  *idb, NPB  *npb);

IFDISP enrtadisp =
{	setpkt,				// Set up hardware part of packet
	sethwaddr,			// Store hardware address in packet
	setbcaddr,			// Store broadcase hardware address in packet
	setecho,			// Set up packet for echo
	sendpkt,			// Send output packet
	getsrcphy			// Get source physical network address from packet
};

static long XOSFNC aufail(long rtn);

static ADDUNITFNC enrtaaddunit;


//************************************************************
// Function: initenrta - Initialization routine
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

#pragma code_seg ("x_ONCE");

static long XOSFNC initenrta(
	char **pctop,
	char **pdtop)
{
	long rtn;

	sysChrFixUp((CHARTBL *)&enrtadctbl);
	sysChrFixUp((CHARTBL *)&enrtaautbl);
	if ((rtn = sysIoDriverRegister("ERTA", enrtaaddunit, &xosnetNifCcb)) >= 0)
		*pctop = codetop;
	return (rtn);
}

#pragma code_seg ();


//************************************************************
// Function: enrtaaddunit - Add Ethernet interface unit
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// Define structure which contains local items for addunit that are referenced
//   by the characteristics set functions

typedef struct
{	long  bufrsz;		// Specified total input buffer size
	long  pcibus;
	long  pcislot;
	long  pcifunc;
	long  bmctl;
	char  txflow;
	char  rxflow;
	char  fill1[2];
} AUDATA;


long XOSFNC enrtaaddunit(
	long unit)
{
	AUDATA data;
	long  *pcitp;
	long   rtn;
	long  *lpnt;
	IDB   *idbp;
	EIDB  *idb;
	char  *irout;			// Interrupt routine offset
	long   ioreg;			// Base IO register number
	long   inum;			// Interrupt level
	long   bufrszx;
	union
	{ long clsitem;
	  struct
	  {	char revid;
	    char prog;
		char subclsc;
		char clsc;
	  };
	}      pci;
	long   pcidevitem;
	char   hvxmres;
	REGS  *regs;
	long   phyregs;
	PCIB  *pcib;
	long   ctype;

	char  *irng;
	long   irngphy;			// Physical address of the input ring buffer
	long   cnt;
	long   bits;

	hvxmres = 0;						// Clear memory pointers here in case
	irng = NULL;						//   we get an error before memory is
										//   allocated
	data.bufrsz = 64;
	data.pcibus = -1;
	data.pcislot = -1;
	data.pcifunc = -1;
	data.bmctl = R_BMCTL_AUTO;		// Store default mode (auto)
	data.txflow = R_MEDIASTS_TXFCE;	// Store default flow control status
	data.rxflow = R_MEDIASTS_RXFCE;
	if (knlTda.buffer2 == NULL)
		return (ER_CHARM);
	if ((rtn = sysChrValuesNew(FALSE, enrtaautbl, &data)) < 0)
		return (rtn);
	if ((data.pcislot | data.pcifunc | data.pcibus) == -1)
										// Did we get a PCI address?
		return (ER_CHARM);				// No - fail
	if ((rtn = xospciFindByAddr(&pcib, data.pcibus, data.pcislot,
			data.pcifunc)) < 0)
		return (rtn);
	if (pcib->clsc != 2)
		return (ER_PDTYP);
	pcitp = pcitbl;
	cnt = sizeof(pcitbl)/sizeof(long);
	do
	{
		if (pcib->devid == *pcitp++)
			break;
	} while (--cnt > 0);
	if (cnt <= 0)
		return (ER_PDTYP);

	if ((((long)pcib->addr0) & 0x01)	== 0) // IO address?
		return (ER_PDTYP);				// No - fail
	ioreg = pcib->addr0 & 0xFFFFFF00;	// Store IO address

	if (((long)pcib->addr1) & 0x01)		// Memory address?
		return (ER_PDTYP);				// No - fail
	phyregs = pcib->addr1 & 0xFFFFFF00;	// Store physical memory address

	pci.revid = pcib->revid;
	pcidevitem = pcib->devid;

	if (pcib->inum == 0)					// Must have an interrupt
		return (ER_PDTYP);
	inum = pcib->inum;

	sysMemGetXRes();
	if ((rtn = sysIoChkInt(pcib->inum | 0x100)) < 0)
		return (aufail(rtn));			// See if the interrupt is available
	if ((rtn = sysIoChkPorts(ioreg, ioreg + 255)) < 0)
		return (aufail(rtn));			// See if our IO registers are available

	if ((rtn = sysMemGetXSpace(256, (char **)&regs)) < 0)
		return (aufail(rtn));			// Get address space for our registers
										//   (This will get a whole page!)
	if ((rtn = sysMemMapPhyPages((char *)regs, 1, MT_SYS, 0x3F,
			phyregs & 0xFFFFF000, 0xFFFFFFFF)) < 0) // Map the registers
		return (aufail(rtn));

/// IS THIS RIGHT? Assumes GetXSpace rounds down to beginning of a page!

	*((char **)&regs) += (((long)phyregs) & 0xF00);

	bits = regs->R_txcfg & 0x7CC00000;	// Get the configuration bits we care
										//   about
	cnt = sizeof(chiptbl)/sizeof(long);	// Look for a match in our table
	lpnt = chiptbl;
	do
	{
		if (bits == *lpnt++)
			break;
	} while (--cnt > 0);
	ctype = (cnt > 0) ? (lpnt - chiptbl) : RTT_8139;
	if (regs->R_cmd & R_CMD_RESET)	// Is the reset bit set now?
		return (aufail(ER_PDTYP));		// Yes - something is very wrong!
	regs->R_cmd |= R_CMD_RESET;		// Reset the interface

	// Allocate memory for the input ring buffer - We must allocate a single
	//   large contiguous buffer.

	// NOTE: The Realtek datasheets all indicate that the input ring buffer
	//       is 16 bytes larger than the indicated power of 2. Thus instead
	//       of 65536 the size would be 65552. None of the chips we have
	//       seen behave this way. They all use the exact power of 2. Just
	//       to be safe, we allocate an extra page after the receive ring.
	//       If the chip does actually use the extra 16 bytes as part of the
	//       ring, this will cause a serious error which will reset the chip
	//       every 100 or so packets input (since the buffer wrapping logic
	//       will not work right), but at least it will not randomly write on
	//       someone else's memory! My suspicion is that it does not really
	//       "use" the extra 16 bytes as part of the ring but may sometimes
	//       modify them. If so, the current way this is handled is correct.
	//       I have never observed any locations past the end of the ring
	//       buffer being modified.

	if (data.bufrsz <= 8)
	{
		data.bufrsz = 8;
		bufrszx = 0;
	}
	else if (data.bufrsz <= 16)
	{
		data.bufrsz = 16;
		bufrszx = 1;
	}
	else if (data.bufrsz <= 32)
	{
		data.bufrsz = 32;
		bufrszx = 2;
	}
	else
	{
		data.bufrsz = 64;
		bufrszx = 3;
	}
	if ((rtn = sysMemGetLargeBuffer(data.bufrsz << 10, FALSE,
			&irngphy, (char **)&irng)) < 0)
		return (aufail(rtn));			// Get a large buffer

	if (regs->R_cmd & R_CMD_RESET)	 // Has the chip reset finished?
		return (aufail(ER_DEVER));

	regs->R_intsts = 0xFFFF;			// Reset all interrupt requests

	sysSchSDelay(5);

	if (regs->R_intsts & 0xFFFF)		// Did it work?
		return (aufail(ER_DEVER));		// No - fail

	// Make our IDB

	if ((rtn = xosnetMakeIdb(unit, sizeof(EIDB), &enrtadctbl, &idbp)) < 0)
		return (aufail(rtn));
	idb = (EIDB *)idbp;

	// Set up the IDB

	*(long *)idb->typname = 'ERTA';		// Store type name
	idb->iftype = IFT_MULTI;			// Store interface type
	idb->hwhdrsz = 14;					// Store hardware header size
	idb->hwpktsz = 1500;				// Store maximum packet size
	idb->hlen = 6;						// Store hardware address size
	idb->hwtype = 1;					// Store hardware type value for APR
	idb->ifdisp = &enrtadisp;
	idb->fdisp = (DFRKFNC *)enrtafork;	// Store address of fork routine
	idb->bioreg = ioreg;
	idb->enregs = regs;
	idb->enphyreg = phyregs;
	idb->enregs = regs;
	idb->enirngphy = irngphy;
	idb->enirng = irng;
	idb->enbufrsz = data.bufrsz;
	idb->enbufrszx = bufrszx;
	idb->enctype = ctype;
	idb->enirngend = data.bufrsz << 10;
	idb->enibsmask = idb->enirngend - 1;
	idb->enxmtmax = 4 - 1;
	idb->enflowctl = data.txflow | data.rxflow;
	idb->config = IC_SPEED_100|IC_FD|IC_AUTO;
	idb->hwaddr = *(llong *)regs->R_netaddr; // Store MAC address in the IDB
	idb->enbus = pcib->bus;
	idb->enslot = pcib->slot;
	idb->enfunc = pcib->func;
	idb->endevice = pcib->devid;
	idb->enrevsn = pcib->revid;
	if (eidbtail != NULL)				// Link into our list of IDBs
		eidbtail->next = (IDB *)idb;
	else
		eidbhead = idb;
	eidbtail = idb;

	// Set up the interrupt code for this unit

	extern char fix1[], fix2[], fix3[], fix4[], fix5[], fix6[], enrtaexit[];

	if ((rtn = sysMemGetXCode(4 * (long)&ENRTAINTSZ, (char **)&irout)) < 0)
		return (aufail(rtn));			// Get space for interrupt code

	sysLibMemCpyLong((long *)irout, (long *)&enrtaint, (long)&ENRTAINTSZ);
										// Copy the prototype code
	*(vushort **)(((long)fix1) + (long)irout) = &regs->R_intsts; // Do fix-up
	*(vushort **)(((long)fix2) + (long)irout) = &regs->R_intsts;
	*(vlong **)(((long)fix3) + (long)irout) = &idb->enintsts;
	*(vushort **)(((long)fix4) + (long)irout) = &regs->R_intmask;
	*(EIDB **)(((long)fix5) + (long)irout) = idb;
	*(long *)(((long)fix6) + irout) = ((long)sysIoReqFork) - ((long)fix6) -
			((long)irout) - 4;
	if (!haveoas)						// Set up our once-a-second routine the
	{									//   first time though here
		sysSchSetUpOAS(enrtaoas);
		haveoas = TRUE;
	}

	if (sysIoRegPorts(idb->name, ioreg, ioreg + 255) < 0)
		CRASH('CNRR');

	if (sysIoSetInt(idb->name, inum | 0x100, DT_TRAP+PL_0, irout,
			irout + (long)enrtaexit) < 0) // Set our interrupt vector
		CRASH('CNSI');

	sysMemGiveXRes();					// Give up the exec memory resource
	enrtareset(idb);					// Reset the interface
	sysIoEnableInt(inum);				// Enable our interrupt
	return (0);
}


//************************************************************
// Function: aufail - Called on addunit failure to do clean-up
// Returned: Error code (the argument value)
//************************************************************

static long XOSFNC aufail(
	long code)
{
	CRASH('FAIL');

	return (code);
}


//********************************************************
// Function: enrtaaubufrsz - Set the value of the BUFRSIZE
//							 addunit characteristic
// Returned: Number of bytes stored or a negative XOS
//           error code if error
//********************************************************

static long XOSFNC enrtaaubufrsz(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSGetULongV(val, &((AUDATA *)data)->bufrsz, cnt));
}


//***********************************************************************
// Function: enrtaaubus - set the value of the BUS addunit characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC enrtaaubus(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;

	if ((rtn = sysUSGetULongV(val, &((AUDATA *)data)->pcibus, cnt)) < 0)
		return (rtn);
	return ((((AUDATA *)data)->pcibus & 0xFFFFFF00) ? ER_CHARV : 0);
}


//*************************************************************************
// Function: enrtaauslot - set the value of the SLOT addunit characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//*************************************************************************

static long XOSFNC enrtaauslot(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;

	if ((rtn = sysUSGetULongV(val, &((AUDATA *)data)->pcislot, cnt)) < 0)
		return (rtn);
	return ((((AUDATA *)data)->pcislot & 0xFFFFFFE0) ? ER_CHARV : 0);
}


//*************************************************************************
// Function: enrtaaufunc - set the value of the FUNC addunit characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//*************************************************************************

static long XOSFNC enrtaaufunc(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;

	if ((rtn = sysUSGetULongV(val, &((AUDATA *)data)->pcifunc, cnt)) < 0)
		return (rtn);
	return ((((AUDATA *)data)->pcibus & 0xFFFFFF80) ? ER_CHARV : 0);
}


//***********************************************************************
// Function: chrgetboard: Get the value of the BOARD characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC chrgetboard(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysLibStrNMovZ(val, descriptbl[((EIDB *)((NETDCB *)
			knlTda.dcb)->nifidb)->enctype - 1], cnt));
}


//***********************************************************************
// Function: chrgetlink - Get the value of the LINK characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

// This returns a text string that indicates the current link status as
//   follows:
//		{Link|Nolink}-{Auto-|}{10|100|???}{FD|HD|}
//   If the link is not connected (Nolink displayed) and is in auto mode
//   (Auto displayed) the speed and duplex information is displayed as ???.

static long XOSFNC chrgetlink(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	REGS *regs;
	EIDB *idb;
	char *pnt;
	char  bufr[64];

	idb = (EIDB *)((NETDCB *)knlTda.dcb)->nifidb;
	regs = idb->enregs;

	pnt = bufr + sysLibStrMov(bufr, (regs->R_bmsts & R_BMSTS_LINK) ?
			"Link-" : "NoLink-");
	while (TRUE)
	{
		if (regs->R_bmctl & R_BMCTL_AUTO)
		{
			pnt += sysLibStrMov(pnt, "Auto-");
			if ((regs->R_bmsts & R_BMSTS_LINK) == 0)
			{
				sysLibStrMov(pnt, "???");
				break;
			}
		}
		pnt += sysLibStrMov(pnt, (regs->R_mediasts & R_MEDIASTS_SPEED10) ?
				"10" : "100");
		sysLibStrMov(pnt, (regs->R_bmctl & R_BMCTL_DUPLEX) ? "FD" : "HD");
		break;
	}
	return (sysLibStrMov(val, bufr));
}


//***********************************************************************
// Function: chrsetlink Set the value of the LINK characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

// This allows the link mode to be set to one of: "Auto", "10HD", "10FD",
//   "100HD", or "100FD".

// NOTE: Setting the mode to AUTO when connected to a hub/switch that does
//       not support negoiation will, in general, result in unpredictable
//       behavior.

static long XOSFNC chrsetlink(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	static CHRSTRVAL linktbl[] =
	{	{"AUTO" , IC_AUTO},
		{"10HD" , IC_SPEED_10},
		{"10FD" , IC_SPEED_10|IC_FD},
		{"100HD", IC_SPEED_100},
		{"100FD", IC_SPEED_100|IC_FD}
	};

	EIDB *idb;
	long  tvl;

	if ((tvl = sysChrGetValTbl(val, cnt, linktbl,
			sizeof(linktbl)/sizeof(CHRSTRVAL))) < 0)
		return (tvl);
	idb = (EIDB *)((NIFDCB *)knlTda.dcb)->nifidb;
	idb->config = (idb->config & ~(IC_AUTO|IC_FD|IC_SPEED)) | tvl;
	setlinkmode(idb);
	return (0);
}


#if 0

dcsetmode:
	MOVL	EBX, #modetbl
	MOVL	ECX, #MODETBLSZ
	CALL	knlGetDcVal2##		// Get new value
	JC	4_			// If error
	MOVL	ESI, dcb_netidb[EDI]
	MOVL	EDX, idb_enrtamapreg[ESI]
	TESTB	idb_enrtacattr = #CA_GIGABIT ;Is this a gigabit chip?
	JNE	6_

;Here if not a gigabit chip

	MOVL	idb_enrtabmctl = EAX
	MOVW	R_BMCTL[EDX], AX
4_:	RET

;Here if this is a gigibit chip

6_:	CRASH	GIGA


#endif


//***********************************************************************
// Function: chrgettxflow - Get the value of the TXFLOW characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC chrgettxflow(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	REGS *regs;
	EIDB *idb;
	char *str;

	idb = (EIDB *)((NETDCB *)knlTda.dcb)->nifidb;
	regs = idb->enregs;
	str = ((regs->R_mediasts & R_MEDIASTS_TXFCE) ? "Enabled" : "Disabled");
	return (sysLibStrNMov(val, str, cnt));
}


//***********************************************************************
// Function: chrgetrxflow Set the value of the RXFLOW characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC chrgetrxflow(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	REGS *regs;
	EIDB *idb;
	char *str;

	idb = (EIDB *)((NETDCB *)knlTda.dcb)->nifidb;
	regs = idb->enregs;
	str = ((regs->R_mediasts & R_MEDIASTS_RXFCE) ? "Enabled" : "Disabled");
	return (sysLibStrNMov(val, str, cnt));
}


//***********************************************************************
// Function: chrsettxflow Set the value of the TXFLOW characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC chrsettxflow(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{


	return (0);
}


#if 0

dcsettxflow:
	MOVL	EBX, #endistbl
	MOVL	ECX, #ENDISTBLSZ
	CALL	knlGetDcVal2##		// Get new value
	JC	4_			// If error
	MOVL	ESI, dcb_netidb[EDI]
	MOVL	EDX, idb_enrtamapreg[ESI]
	TESTB	idb_enrtacattr = #CA_GIGABIT ;Is this a gigabit chip?
	JNE	8_

;Here if not a gigabit chip

	MOVB	AH, R_MEDIASTS[EDX]
	ANDB	AH, #{~R_MEDIASTS_TXFCE&0FFh}
	ORB	AH, AL
	ANDB	idb_enrtaflowctl = #{~R_MEDIASTS_TXFCE&0FFh}
	ORB	idb_enrtaflowctl = AL
	MOVB	R_MEDIASTS[EDX], AH
4_:	RET

;Here if this is a gigabit chip

8_:	CRASH	GIGA


#endif


//***********************************************************************
// Function: chrsetrxflow - Set the value of the RXFLOW characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC chrsetrxflow(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{


	return (0);
}

#if 0

dcsetrxflow:
	MOVL	EBX, #endistbl
	MOVL	ECX, #ENDISTBLSZ
	CALL	knlGetDcVal2##		// Get new value
	JC	12_			// If error
	MOVL	ESI, dcb_netidb[EDI]
	MOVL	EDX, idb_enrtamapreg[ESI]
	TESTB	idb_enrtacattr = #CA_GIGABIT ;Is this a gigabit chip?
	JNE	10_

;Here if not a gigabit chip

10_:
	MOVB	AL, R_MEDIASTS[EDX]
	ANDB	AL, #~R_MEDIASTS_RXFCE
	ORB	AL, AH
	ANDB	idb_enrtaflowctl = #{~R_MEDIASTS_RXFCE&0FFh}
	ORB	idb_enrtaflowctl = AH
	MOVB	R_MEDIASTS[EDX], AL
	RET

;Here if this ia a gigabit chip

12_:	CRASH	GIGA

#endif


//***********************************************************************
// Function: chrgetphyaddr - Set the value of the PHYADDR characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC chrgetphyaddr(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, ((NETDCB *)knlTda.dcb)->nifidb->hwaddr, cnt));
}


//*************************************************************
// Function: getsrcphy - Driver function to get source physical
//                       network address from packet
// Returned: The hardware address
//*************************************************************

static llong XOSFNC getsrcphy(
	IDB  *idb,
	NPB  *npb)
{
	return (*((llong *)((ENPKT *)npb)->srchwaddr) & 0xFFFFFFFFFFFF);
}


//****************************************************
// Function: enrtareset - Reset the Ethernet interface
// Returned: Nothing
//****************************************************

static void XOSFNC enrtareset(
	EIDB *idb)
{
	volatile int rcnt;

	REGS  *regs;
	ENNPB *npb;

	regs = idb->enregs;
	regs->R_cmd = R_CMD_RESET;		// Reset the controller
	rcnt = 10000;
	while (regs->R_cmd & R_CMD_RESET && --rcnt > 0)
		;

	// First clean up anything that might be going on now

	regs->R_intmask = 0;				// Ensure no more interrupts (This is
	idb->enintsts = 0;					//   not really needed, but it does 
										//   not hurt.)
	while (idb->enocnt != 0)			// Have any pending output?
	{
		npb = idb->enorng[idb->enotaker];
		idb->enorng[idb->enotaker] = 0;
		xosnetFinXmit(ER_DEVER, (IDB *)idb, (NPB *)npb);
		idb->enotaker = (idb->enotaker + 1) & 0x03;
		idb->enocnt--;
	}

	// Here with all pending output handled

	idb->xmtavail = idb->enxmtmax;

	idb->enoputter = 0;					// Clear our ring pointers
	idb->enotaker = 0;
	idb->enitaker = 0;

	// Now set up the buffer rings

	regs->R_tsts[0] = R_TSD_OWN;		// Make sure driver owns the transmit
	sysSchSDelay(2);					//   descriptors (Reset should have
	regs->R_tsts[1] = R_TSD_OWN;		//   done this, this just makes sure!)
	sysSchSDelay(2);
	regs->R_tsts[2] = R_TSD_OWN;
	sysSchSDelay(2);
	regs->R_tsts[3] = R_TSD_OWN;
	sysSchSDelay(2);
	regs->R_rbaddr = idb->enirngphy;	// Set the receive buffer address

	regs->R_eepcfg = 0xC0;				// Enable writing config registers

	idb->xmtavail = 3;					// We can handle 3 output packets at
										//   once

	// Note: The Linux driver says you must enable Tx/Rx before setting
	//       transfer thresholds!

	regs->R_cmd = R_CMD_RXE|R_CMD_TXE;
	regs->R_rxcfg = (idb->enbufrszx << 11) | (RX_FIFO_THRESH << 13) |
			(RX_DMA_BURST << 8) | R_RXC_APM | R_RXC_AB;
	regs->R_txcfg = (TX_DMA_BURST << 8) | 0x03000000;
	regs->R_eepcfg = 0xC0;		// Enable configuration registers

	//// SET SOME CONFIG1 BITS HERE???

	regs->R_bmctl = idb->enbmctl;		// Set the mode
	regs->R_mediasts = idb->enflowctl;	// Set flow control enables
	regs->R_eepcfg = 0;					// Lock the configuration registers
	sysSchSDelay(2);
	regs->R_intmask = R_INT_SER|R_INT_RFO|R_INT_ULC|R_INT_RBO|
			R_INT_TER|R_INT_TOK|R_INT_RER|R_INT_ROK;
										// Enable interrupts
	idb->state = (regs->R_mediasts & R_MEDIASTS_LINKDWN) ? LNKS_DISCNCTD :
				LNKS_ESTAB;
	setlinkmode(idb);
}


//***********************************************
// Function: setlinkmode - Set hardware link mode
// Returned: Nothing
//***********************************************

static void setlinkmode(
	EIDB *idb)
{
	REGS *regs;

	regs = idb->enregs;
	regs->R_bmctl = R_BMCTL_SPEED|R_BMCTL_DUPLEX; // Assume 100MHz, full duplex
	if ((idb->config & IC_SPEED) == IC_SPEED_10)
		regs->R_bmctl &= ~R_BMCTL_SPEED; // If 10MHz
	if ((idb->config & IC_FD) == 0)
		regs->R_bmctl &= ~R_BMCTL_DUPLEX; // If half duplex
	if (idb->config & IC_AUTO)
		regs->R_bmctl |= (R_BMCTL_AUTO|R_BMCTL_RESTART); // If auto
}




//*************************************************************
// Function: setecho - Driver function to set up hardware level
//                     packet header for echo packet
// Returned: Offset of start of software part of packet if
//           normal or a negative XOS error code if error
//*************************************************************

/// ??????????????

long XOSFNC setecho(
	IDB *idb,
	NPB *npb)
{
#if 0

setecho:MOVL	EAX, en_srchwadr+0[EBX] ;Copy source to destination
	MOVL	en_dsthwadr+0[EBX], EAX
	MOVW	DX, en_srchwadr+4[EBX]
	MOVW	en_dsthwadr+4[EBX], DX
	JMP	4_

#endif

	return (0);
}


//********************************************************
// Function: setpkt - Driver function to set up hardware
//                    level packet header
// Returned: Offset of start of software part of packet if
//           normal or a negative XOS error code if error
//********************************************************

long XOSFNC setpkt(
	IDB *idb,
	NPB *npb,
	long etype)
{
	((ENNPB *)npb)->ethertype = convnetword(etype); // Store Ethertype value
	*(long *)(((ENNPB *)npb)->srchwaddr+0) = // Store our address in the
			*(long *)&((EIDB *)idb)->hwaddr; //   packet as the source
	*(ushort *)(((ENNPB *)npb)->srchwaddr + 4) = //   address
			*(ushort *)(((char *)&((EIDB *)idb)->hwaddr) + 4);
	npb->npofs = sizeof(ENPKT);			// Store offset of next level
	npb->tpofs = sizeof(ENPKT);
	npb->count = sizeof(ENPKT);			// Store initial packet size
	npb->outdone = NULL;				// Assume no output done function
	return (sizeof(ENNPB));
}


//************************************************************
// Function: sethwaddr - Driver function to store destination
//                       hardware address in a packet
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

void XOSFNC sethwaddr(
	IDB *idb,
	NPB *npb,
	char hwa[6])
{
	*(long *)((ENNPB *)npb)->dsthwaddr = *(long *)hwa;
	*(ushort *)(((ENNPB *)npb)->dsthwaddr + 4) = *(ushort *)(hwa + 4);
}


//************************************************************
// Function: setbcaddr - Driver function to store broadcast
//                       hardware address in a packet
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

void XOSFNC setbcaddr(
	IDB *idb,
	NPB *npb)
{
	*(long *)((ENNPB *)npb)->dsthwaddr = 0xFFFFFFFF;
	*(ushort *)(((ENNPB *)npb)->dsthwaddr + 4) = 0xFFFF;
}


//************************************************************
// Function: sendpkt - Driver function to send a packet
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// Must be called from fork level, does not wait for completion

// Without descriptors output packets must be contigous in memory, but since
//   all of the higher level routines always generate contiguous packets that
//   do not cross memory page boundries, this is not a problem.  It also
//   requires that the output buffers be long-aligned, which is also not a
//   probelm since all packets generated by the higher level routines are
//   long aligned. With descriptors these restrictions are relaxed considerably
//   so there is no problem there either

// xosnetSendPkt handles scheduling and queueing. We will not be called here
//   unless there is an output buffer slot available. All we have to do is
//   keep track of which slot to use next.

static long XOSFNC sendpkt(
	IDB *idb,
	NPB *npb,
	long etype)
{
	REGS *regs;
	int   length;
	int   oput;

	npb->sts |= NPS_XMITING;
	length = npb->count;
	if (length > 1514)					// Is the packet too big?
		length = 1514;					// Yes - just use as much as we can!
	((EIDB *)idb)->enocnt++;
	oput = ((EIDB *)idb)->enoputter;	// Make this easier to read and give
										//   the compiler a little help.
	((EIDB *)idb)->enorng[oput] = (ENNPB *)npb;
										// Store address of the packet in the
										//   pointer ring
	if (length < 60)					// Is the packet too short?
	{
		sysLibMemSet(npb->data + length, 0, 60 - length);
		length = 60;					// Yes - this code counts on the fact
	}									//   that we never allocate a packet
										//   buffer of less than 128 bytes,
										//   which always has room for a 60
										//   byte packet
	regs = ((EIDB *)idb)->enregs;
	regs->R_taddr[oput] = sysMemGetPhyAddr(((char *)&npb->data));
										// Store physical address of the packet
										//   in the output address register
	regs->R_tsts[oput] = length | (48 << 16);
	((EIDB *)idb)->enoputter = (oput + 1) & 0x03;
	return (0);
}


//********************************************************
// Function: enrtafork - Device fork function for ENRTADEV
// Returned: Nothing
//********************************************************

void XOSFNC enrtafork(
	EIDB *idb)
{
	REGS  *regs;
	BPKT  *bpkt;
	ENNPB *npb;
	char  *ppnt;
	long   intsts;
	long   pktsts;
	long   code;
	long   sts;
	int    length;
	int    len;
	int    amnt;
	int    taker;

	idb->cntint++;						// Count the interrupt

	regs = idb->enregs;
	intsts = idb->enintsts;

	// First check the ULC flag. This bit is set for both a transmit underrun
	//   (which should not be possible since we don't start output until the
	//   entire packet is in the FIFO) and link status chnage. It's cheap to
	//   update the link status so we do that whenever we see this set. The
	//   only 2 states we are are ESTAB and DISCNCTD.

	if (intsts & R_INT_ULC)
		idb->state = (regs->R_mediasts & R_MEDIASTS_LINKDWN) ? LNKS_DISCNCTD :
				LNKS_ESTAB;

	// Check for serious controller errors

	if (intsts & (R_INT_RFO|R_INT_RBO|R_INT_SER))
	{
		// Here with a device error interrupt - this includes FIFO
		//   overflow, receive ring buffer overflow, and "system error".
		//   We count the errors and reset the interface.

		if (intsts & R_INT_SER)
			idb->cntsyserr++;
		if (intsts & R_INT_RFO)
			idb->cntnoibfr++;
		if (intsts & R_INT_RBO)
			idb->cntrover++;
		enrtareset(idb);
	}

	// Check for input done (including input errors)

	if (intsts & (R_INT_ROK|R_INT_RER))
	{
		// Here if have input available

		idb->enintsts &= ~(R_INT_ROK|R_INT_RER);
		while (idb->enitaker != (regs->R_rbput & idb->enibsmask))
		{								// Is there anything in the input
										//   buffer?
			bpkt = (BPKT *)(idb->enirng + idb->enitaker);
			if ((length = bpkt->len - 4) == (0xFFF0 - 4))
				break;					// This means the chip is still storing
										//   the packet! (See note near top of
										//   this file.)
			while (TRUE)				// Fake loop to allow escape
			{
				if (length < 60)
				{
					idb->cntrunt++;
					break;
				}

				// Get the status bits in a non-volatile item so the
				//   compiler does not keep reloading it!

				pktsts = bpkt->sts;
				if ((bpkt->sts & R_RSR_ROK) == 0) // Good packet?
				{
					if (pktsts & R_RSR_ISE|R_RSR_CRC)
						idb->cntcrc++;
					else if (pktsts & R_RSR_RUNT)
						idb->cntrunt++;
					else if (pktsts & R_RSR_LONG)
						idb->cnttoobig++;
					else if (pktsts & R_RSR_FAE)
						idb->cntfa++;
					else
					{
						// Here if none of the expected packet status bits
						//   are set - call it a bad pointer and reset the
						//   interface!

						idb->cntbadpnt++;
						enrtareset(idb);	//// WRONG!!!!
					}
					break;
				}
				if (length > 1600)
				{
					idb->cnttoobig++;
					break;
				}
				idb->cntpktin++;		// OK - count the received packet
				idb->cntbytein += length;
				if (pktsts & (R_RSR_MAR|R_RSR_BAR))
				{						// Broadcast or multicast?
					if (pktsts & R_RSR_MAR) // Yes - Multicast?
						idb->cntmcpktin++; // Yes
					else
						idb->cntbcpktin++; // No - must be broadcast
				}
				if (xosnetMakeNpb((IDB *)idb, NULL, length,
						(NPB **)&npb) < 0) // Get a buffer for the packet
				{
					idb->cntnosbfr++;
					break;
				}
				npb->count = length; 	// Store length in the packet
				npb->npofs = sizeof(ENPKT);
				npb->tpofs = sizeof(ENPKT);
				ppnt = npb->data;
				len = length;
				amnt = idb->enirngend - (((char *)&bpkt->pkt) -
						idb->enirng);	// Get number of bytes to end of the
										//   ring buffer
				if (len > amnt)			// Did it wrap?
				{						// Yes - copy the first part
					sysLibMemCpyLong((long *)ppnt, (long *)&bpkt->pkt,
							amnt >> 2);
					len -= amnt;
					ppnt += amnt;
				}
				sysLibMemCpyLong((long *)ppnt, (long *)&bpkt->pkt,
						(len + 3) >> 2); // Copy the rest
				xosnetRecvPkt((IDB *)idb, convnetword(npb->ethertype),
						(NPB *)npb);
				break;					// Exit from the fake loop
			}							// End of fake loop

			// Advance the pointers

			length = (length + 11) & 0xFFFFFFFC; // Allow for the header and
												 //   CRC bytes, round up
			if ((length += idb->enitaker) >= idb->enirngend)
				length -= idb->enirngend; // Advance the pointer
			idb->enitaker = length;
			if ((length -= 0x10) < 0)
				length += idb->enirngend;
			regs->R_rbtake = length;	// Set the hardware taker pointer
										// End of loop for processing received
										//   packets without descriptors
		}
	}

	// Check for output done (including output errors)

	// NOTE: We treat the 4 pairs of output registers as a 4 item ring. The
	//       datasheet does not say how these should be used but this seems
	//       to work!

	if (intsts & (R_INT_ULC|R_INT_TOK|R_INT_TER))
	{
		// Here if have output complete

		idb->enintsts &= ~(R_INT_ULC|R_INT_TOK|R_INT_TER);
		while (idb->enocnt != 0)
		{
			taker = idb->enotaker;
			sts = regs->R_tsts[taker];
			if ((sts & R_TSD_OWN) == 0) // Has the buffer been output?
				break;					// No - false alarm!
			idb->enocnt--;				// Yes
			npb = idb->enorng[taker];	// Get address of the packet that was
										//   just output
			*(long *)&idb->enorng[taker] &= 0x80000000;
										// For debugging only!
			if (npb == NULL)			// Make sure we have a packet
				CRASH('NPKT');
			idb->enotaker = (taker + 1) & 0x03; // Advance the taker
			idb->cntnumcol += ((sts >> 24) & 0x0F);
			if ((sts & R_TSD_TOK) == 0)
			{
				if (sts & R_TSD_TUN)
					idb->cntxunder++;
				if (sts & R_TSD_OWC)
					idb->cntowcol++;
				if (sts & R_TSD_TABT)
					idb->cntjabber++;
				if (sts & R_TSD_CDH)
					idb->cnthrtbt++;
				if (sts & R_TSD_CRS)
					idb->cntcsense++;
				if ((sts & (R_TSD_TUN|R_TSD_OWC|R_TSD_TABT|
						R_TSD_CDH|R_TSD_CRS)) == 0)
					idb->cntxcol++;
				code = ER_DEVER;
			}
			else
				code = 0;
			xosnetFinXmit(code, (IDB *)idb, (NPB *)npb); // Process the pkt
		}

		// Here with all output done processing complete - see if we need
		//   to start output

		if (idb->xmtavail > 0 && (npb = (ENNPB *)idb->outhead) != NULL)
		{								// Do we need to do output?
			if ((idb->outhead = npb->sendnext) == NULL) // Yes - remove the pkt
				idb->outtail = NULL;					//   from the send list
			idb->xmtavail--;			// Reduce output availability
			npb->sendnext = NULL;		// Set link to 0 to indicate packet is
										//   being output
			npb->holdrt = NULL;			// Also make sure hold list back pointer
										//   is clear
			sendpkt((IDB *)idb, (NPB *)npb, npb->ethertype);
		}
	}
	regs->R_intmask = R_INT_SER|R_INT_RFO|R_INT_ULC|R_INT_RBO|
				R_INT_TER|R_INT_TOK|R_INT_RER|R_INT_ROK;
}										// Enable interrupts


//********************************************
// Function: enrtaoas - Once-a-second function
// Returned: Nothing
//********************************************

static void XOSFNC enrtaoas(void)
{



}

#if 0

	MOVL	ESI, enrtafidb		// Get offset of first IDB
4_:	TESTL	ESI, ESI		// More to check?
	JE	10_			// No
	CMPB	idb_outtimer = #0	// Yes - need to time output?
	JE	6_			// No
	DECB	idb_outtimer[ESI]	// Yes - has it timed out?
	JNE	6_			// No
	INCL	idb_cntxhung[ESI]	// Yes - count it
	TOFORK				// Raise to fork level

;;;;; THIS IS WRONG!!!!!

	MOVL	EAX, #ER_DEVER		// Report this as a device error
	STC
	CALL	xosnetFinXmit##		// Do output done processing
	FROMFORK			// Back to main program level
6_:	MOVL	ESI, idb_nextd[ESI]	// Advance to next IDB
	JMP	4_			// Continue

10_:	RET
.PAGE
;Function to write value to a PHY register
;	int phywrite(
;	    long iobase,	// IO register block address
;	    long value);	// 0(8) Register number(8) Value(16)
;  Value returned is 0 if OK or a negative XOS error code if error

phyw_iobase 12t
phyw_value  8t

phywrite:
	PUSHL	ESI
	MOVL	EDX, phyw_iobase[ESP]
	MOVL	EAX, phyw_value[ESP]
	ORL	EAX, #0x80000000
	MOVL	RTG_PHYAR[EDX], EAX
	MOVL	ESI, #1000t
4_:	MOVL	ECX, #20t
	CALL	knlSDelay##
	CLRL	EAX
	BTL	RTG_PHYAR[EDX], #31t
	JNC	8_
	LOOP	ECX, 4_
	MOVL	EAX, #ER_NORSP
8_:	POPL	ESI
	RET	8t

;Function to read value from a PHY register
;	int phyread(
;	    long iobase,	// IO register block address
;	    long regnum);	// Register number
;  Value returned is the 16-bit register value (bits 16-31 are 0) if OK or
;    a negative XOS error code if error

phyr_iobase 12t
phyr_regnum 8t

phyread:PUSHL	ESI
	MOVL	EDX, phyr_iobase[ESP]
	MOVL	EAX, phyr_regnum[ESP]
	SHLL	EAX, #16t
	MOVL	RTG_PHYAR[EDX], EAX
	MOVL	ESI, #1000t
10_:	MOVL	ECX, #20t
	CALL	knlSDelay##
	CLRL	EAX
	MOVL	EAX, RTG_PHYAR[EDX]
	ANDL	EAX, 0x8000FFFF
	BTCL	EAX, #31t
	JNC	14_
	LOOP	ECX, 10_
	MOVL	EAX, #ER_NORSP
14_:	POPL	ESI
	RET	8t
.PAGE

#endif
