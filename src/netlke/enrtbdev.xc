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
#include <xosx.h>
#include <xosxlke.h>
#include <xosxchar.h>
#include <xosxparm.h>
#include <xosxpci.h>
#include <xosxnet.h>
#include <xosxen.h>

// 1.0.0 - 29-Jan-02
//		Initial version, created loosely from ENNEADRV 1.2.0
// 1.0.1 - 19-Jul-02
//		Changed transmit done to check TOK and TUN instead of OWN - seems to
//		fix problem with output hanging at 10MHz.
// 2.0.0 - TCP output rewrite; fixed problem when mapped registers not at 4KB
//		boundry.
// 2.0.1 - 21-Jan-03
//		Changed to make input ring buffer not cachable - fixed input race
//		problem.

#define MAJVER  3
#define MINVER  0
#define EDITNUM 0

extern char ENRTAINTSZ[];
extern char *enrtaint;

static char haveoas = FALSE;

typedef struct enrtaidb__ ENRTAIDB;

static INITLKE initenrta;

static OASFNC  enrtaoas;

static void XOSFNC enrtafork(ENRTAIDB *idb);
#pragma aux enrtafork parm [EDI];

static void XOSFNC enrtareset(ENRTAIDB *idb);

#pragma data_seg(_HEAD);
	LKEHEAD(ENRTADEV, initenrta, MAJVER, MINVER, EDITNUM, DEVICE); 
#pragma data_seg();

// This device driver supports the RealTek 8100 series chips, This includes
//   the 10/100 chips:
//     8129, 8139 (8100), and 8139C+
//   and the 10/100/1000 chips:
//     8169 (8110)

// These chips have differing sets of characteristics in common

//   Chip:              8129     8139    8139C+    8169
//                   +--------+--------+--------+--------+
//   Desciptor IO:   |        No       |       Yes       |
//                   +--------+--------+--------+--------+
//   Direct MMI acc: |   No   |       Yes       |   No   |
//                   +--------+--------+--------+--------+

// The 8139C+ supports up to 64 descriptors, the 8169 supports up to 1024
//   descriptors.

#define OUTDESCP 16				// Size of output descriptor ring when using
								//   descriptors (must be 16, 32, or 64)

// Define our type value for each supported chip type. These are internal
//   values. They are never visible outside of this driver.

#define RTT_8169          1
#define RTT_8169S_8110S_1 2
#define RTT_8169S_8110S_2 3
#define RTT_8169SB_8110SB 4
#define RTT_8118SC        5
#define RTT_8168B_8110B_1 6
#define RTT_8168B_8110B_2 7
#define RTT_8101E         8
#define RTT_8100E_1       9
#define RTT_8100E_2       10
#define RTT_8129          11
#define RTT_8139          12
#define RTT_8139A         13
#define RTT_8139AG        14
#define RTT_8139B         15
#define RTT_8130          16
#define RTT_8139C         17
#define RTT_8139CP        18
#define RTT_MAX           18

// Define structure which specifies the device registers

// The prefix is used as follows:
//   R_ - All chips
//   O_ - 8129 only
//   F_ - "Fast Ethernet" chips (8129, 8139, 8139C+)
//   A_ - "Fast Ethernet" chips except 8139C+ (8129, 8139)
//   C_ - 8139C only
//   D_ - Chips which use descriptors (8139C+, 8169)
//   G_ - "Gigabit Ethernet" chips (8169)

typedef volatile struct
{	char   R_netaddr[6];		// 00 Network address
	char   R_xxx[2];			// 06 Reserved
	char   R_multimap[8];		// 08 Multicast map

/// RTD_DTCCMD  =!0x10		;( 8) Dump tally counter command

	long   F_tsd0;				// 10 Transmit status of descriptor 0
	long   F_tsd1;				// 14 Transmit status of descriptor 1
	long   F_tsd2;				// 18 Transmit status of descriptor 2
	long   F_tsd3;				// 1C Transmit status of descriptor 3
	union
	{ struct
	  {	llong D_nptxd;			// 20 Normal priority transmit descriptor table
		llong D_hptxd;			// 28 High priority transmit descriptor table

	  };
	  struct
	  {	long F_tsad0;			// 20 Transmit start address of descriptor 0
		long F_tsad1;			// 24 Transmit start address of descriptor 1
		long F_tsad2;			// 28 Transmit start address of descriptor 2
		long F_tsad3;			// 2C Transmit start address of descriptor 3
	  };
	};
	long   F_rbaddr;			// 30 Receive buffer address
	ushort F_erbc;				// 34 Early receive byte count register
	char   R_ers;				// 36 Early receive status register
	char   R_cmd;				// 37 Command register
	union
	{ char   G_txreq;			// 38 Transmit request register
	  ushort F_rbtake;			// 38 Receive buffer taker pointer
	};
	ushort F_rbput;				// 3A Receive buffer putter pointer
	ushort R_intmask;			// 3C Interrupt mask register
	ushort R_intsts;			// 3E Interrupt status register
	long   R_txcfg;				// 40 Transmit configuration register
	long   R_rxcfg;				// 44 Receive configuration register
	long   R_timecnt;			// 48 Timer count register
	long   R_mpc;				// 4C Missed packet count register
	char   R_eepcfg;			// 50 EEPROM command register
	char   R_config0;			// 51 Configuration register 0
	char   R_config1;			// 52 Configuration register 1
	char   G_config2;			// 53 Configuration register 2
	union
	{ struct
	  { char G_config3;			// 54 Configuration register 3
		char G_config4;			// 55 Configuration register 4
		char G_config5;			// 56 Configuration register 5
	  };
	  struct
	  { long   F_timeint;		// 54 Timer interrupt register
		char   F_mediasts;		// 58 Media status register
		char   F_config3;		// 59 Configuration register 3
		char   F_config4;		// 5A Configuration register 4
		char   fill0[1];
		ushort R_multint;		// 5C Multiple interupt select
		char   F_pcirev;		// 5E PCI revision ID
		char   fill1[1];		// 5F
	  };
	};
	union
	{ struct
	  {	ushort A_tsad;			// 60 Transmit status of all descriptors
		ushort F_bmctl;			// 62 Basic mode control reg
		ushort F_bmsts;			// 64 Basic mode status reg
		ushort F_anadv;			// 66 Auto-negotiation advertisement reg
		ushort F_anlpar;		// 68 Auto-negotiation link partner reg
		ushort F_anexp;			// 6A Auto-negotiation expansion reg
		ushort F_discnt;		// 6C Discount count reg
		ushort F_fsccnt;		// 6E False carrier sense count reg
	  };
	  struct
	  {	long G_phyar;			// 60 PHY access register
		long G_tbicsr;			// 64 TBI control and status reg
		ushort G_tbianar;		// 68 TBI advertisement reg
		ushort G_tbilpar;		// 6A TBI link partner ability reg
		ushort G_physts;		// 6C PHY (GMII, MII, or TBI) status reg
	  };
	};
	ushort F_nwaytst;			// 70 N-way test register
	ushort F_rxercnt;			// 72 RX_ER count register
	ushort F_cscfg;				// 74 CS configuration register
	char   fill2[2];			// 76
	ulong  F_phy1parm;			// 78 PHY parameter 1 register
	long   F_twister;			// 7C Twister parameter register
	char   F_phy2parm;			// 80 PHY parameter 2 register
	char   fill3[1];
	ushort F_tdokla;			// 82 Low address of a TX descriptor
	union
	{ struct
	  {	char F_pmcrc[8];		// 84 Power management CRCs
		char F_pmwake0[8];		// 8C Power management wakeup frame 0
		char F_pmwake1[8];		// 94 Power management wakeup frame 1
		char F_pmwake2[8];		// 9C Power management wakeup frame 2
		char F_pmwake3[8];		// A4 Power management wakeup frame 3
		char F_pmwake4[8];		// AC Power management wakeup frame 4
		char F_pmwake5[8];		// B4 Power management wakeup frame 5
		char F_pmwake6[8];		// BC Power management wakeup frame 6
		char F_pmwake7[8];		// C4 Power management wakeup frame 7
		char F_pmlsb[8];		// CC Power management LSBs of mask
	  };
	  struct
	  {	char G_pmwake0[8];		// 84 Power management wakeup frame 0
		char G_pmwake1[8];		// 8C Power management wakeup frame 1
		char G_pmwake2[16];		// 94 Power management wakeup frame 2
		char G_pmwake3[16];		// A4 Power management wakeup frame 3
		char G_pmwake4[16];		// B4 Power management wakeup frame 4
		char G_pmcrc[5];		// C4 Power management CRCs for wakeup frames
	  };
	};
	long   F_flash;				// D4 Flash memory read/write register
	char   F_config5;			// D8 Configuration register 5
	char   C_txreq;				// D9 Transmit request register
	ushort G_rxmxpkt;			// DA Maximum received packet size
	char   fill4[4];			// DC
	ushort D_cpcmd;				// E0 C+ command register
	ushort G_undoc;				// E2 ???
	llong  D_rxd;				// E4 Address of receive descriptor table
	char   fill6[4];			// E8
	char   D_etth;				// EC Early transmit threshold
	char   fill7[3];			// ED
	long   R_cbfne;				// F0 Cardbus function event register
	long   R_cbevm;				// F4 Cardbus event mask register
	long   R_cbpss;				// F8 Cardbus present state register
	long   R_cbfre;				// FC Cardbus force event register
} ENRTA;

// Define bits for ENRTA.R_cmd

#define RTR_CMD_RESET 0x10		// Reset
#define RTR_CMD_RXE   0x08		// Receiver enable
#define RTR_CMD_TXE   0x04		// Transfer enable
#define RTR_CMD_BFRE  0x01		// Receive buffer empty

// Define bits in the transmit descriptor status registers

#define RTF_TSD_CRS    0x80000000 // Carrier sense lost
#define RTF_TSD_TABT   0x40000000 // Transmit abort
#define RTF_TSD_OWC    0x20000000 // Out of window collision
#define RTF_TSD_CDH    0x10000000 // CD heart beat
#define RTF_TSD_NCC    0x0F000000 // Number of collisions count
#define RTF_TSD_ERTXTH 0x003F0000 // Early transmit threshold
#define RTF_TSD_TOK    0x00008000 // Transmit OK
#define RTF_TSD_TUN    0x00004000 // Transmit FIFO underrun
#define RTF_TSD_OWN    0x00002000 // Driver owns descriptor
#define RTF_TSD_SIZE   0x00001FFF // Buffer size

// Define bits for ENRTA.R_intmask and ENRTA.R_intsts

#define RTR_INT_SER 0x8000		// System error interrupt
#define RTR_INT_TO  0x4000		// Time-out interrupt
#define RTR_INT_CLC 0x2000		// Cable length change interrupt
#define RTR_INT_RFO 0x0040		// Receive FIFO overflow interrupt
#define RTR_INT_ULC 0x0020		// Packet underrun/link change interrupt
#define RTR_INT_RBO 0x0010		// Receive buffer overflow interrupt
#define RTR_INT_TER 0x0008		// Transmit error interrupt
#define RTR_INT_TOK 0x0004		// Transmit OK interrupt
#define RTR_INT_RER 0x0002		// Receive error interrupt
#define RTR_INT_ROK 0x0001		// Receive OK interrupt

// Define bits for ENRTA.R_rxcfg

#define RTR_RXC_ERTH     0x0F0000000 // Early Rx threshold bits
#define RTR_RXC_MULERINT 0x000020000 // Mutilple early interrupt select
#define RTR_RXC_RER8     0x000010000 // ????
#define RTR_RXC_RXFTH    0x00000E000 // Receive FIFO threshold
#define RTR_RXC_RBLEN    0x000001800 // Receive buffer length
#define RTR_RXC_MXDMA    0x000000700 // Maximum DMA burst size
#define RTR_RXC_WRAP     0x000000080 // Disable packet wrap in buffer
#define RTR_RXC_EEPTYPE  0x000000040 // EEPROM type
#define RTR_RXC_AER      0x000000020 // Accept error packets
#define RTR_RXC_AR       0x000000010 // Accept runt packets
#define RTR_RXC_AB       0x000000008 // Accept broadcast packets
#define RTR_RXC_AM       0x000000004 // Accept multicast packets
#define RTR_RXC_APM      0x000000002 // Accept physical address match packets
#define RTR_RXC_AAP      0x000000001 // Accept all packets (promiscuous mode)

// Define bits for ENRTA.F_mediasts

#define RTF_MEDIASTS_TXFCE   0x80 // Tx flow control enable
#define RTF_MEDIASTS_RXFCE   0x40 // Rx flow control enable
#define RTF_MEDIASTS_AUXSTS  0x10 // Aux. power present status
#define RTF_MEDIASTS_SPEED10 0x08 // Speed is 10MHz
#define RTF_MEDIASTS_LINKDWN 0x04 // Link is down
#define RTF_MEDIASTS_TXPF    0x02 // Pause packet sent (input is paused)
#define RTF_MEDIASTS_RXPF    0x01 // Pause packet received (output is paused)

// Define bits for ENRTA.F_bmctl

#define RTF_BMCTL_RESET   0x8000 // Reset PHY
#define RTF_BMCTL_SPEED   0x2000 // Speed set (1 = 100MHz, 0 = 10MHz)
#define RTF_BMCTL_AUTO    0x1000 // Auto negotiation enable
#define RTF_BMCTL_RESTART 0x0200 // Restart auto-negotiation
#define RTF_BMCTL_DUPLEX  0x0100 // Duplex mode (1 = FD, 0 = HD)

// Define bits for ENRTA.F_bmsts

#define RTF_BMSTS_AUTOCMP 0x0020 // Auto-negotiation is complete
#define RTF_BMSTS_RMTFLT  0x0010 // Remote fault
#define RTF_BMSTS_LINK    0x0004 // Link status
#define RTF_BMSTS_JABBER  0x0002 // Jabber condition detected

// Define bits for ENRTA.G_tbicsr

#define RTG_TBICSR_RESET  0x80000000 // Reset
#define RTG_TBICSR_LB     0x40000000 // Loop-back enable
#define RTG_TBICSR_AUTO   0x20000000 // Auto-negotiation enable
#define RTG_TBICSR_RAUTO  0x10000000 // Restart auto-negotiation
#define RTG_TBICSR_LINKOK 0x02000000 // Link connected
#define RTG_TBICSR_COMP   0x01000000 // Auto-negotiation complete

// Define bits in ENRTA.G_physts

#define RTG_PHYSTS_TBIEN 0x80	// TBI enable
#define RTG_PHYSTS_TXFC  0x40	// Transmit flow control enabled
#define RTG_PHYSTS_RXFC  0x20	// Receive flow control enabled
#define RTG_PHYSTS_1000  0x10	// 1000MBS
#define RTG_PHYSTS_100   0x08	// 100MBS
#define RTG_PHYSTS_10    0x04	// 10MBS
#define RTG_PHYSTS_LINK  0x02	// Link status
#define RTG_PHYSTS_FULL  0x01	// Full duplex

// Define bits in ENRTA.D_cpcmd

#define RTD_CPCMD_MSBFEN 0x0200	// Most signficant byte first enable
#define RTD_CPCMD_VLANEN 0x0040	// Receive VLAN detagging enable
#define RTD_CPCMD_RCSEN  0x0020	// Receive checksum offload enable
#define RTD_CPCMD_DACEN  0x0010	// PCI dual address cycle enable
#define RTD_CPCMD_MRWEN  0x0008	// PCI multiple read/write enable

// Define bits in ENRTA.G_txreq and ENRTA.C_txreg

#define RTX_TXREQ_HPQ  0x80		// Start high priority queue
#define RTX_TXREQ_NPQ  0x40		// Start normal priority queue
#define RTX_TXREQ_FSWI 0x01		// Forced software interrupt

// Define the Gigabit PHY registers (these are accessed through RTG_PHYAR)

#define GPHY_CTR   0			// Control register
#define GPHY_STS   1			// Status register
#define GPHY_ID1   2			// ID register 1
#define GPHY_ID2   3			// ID register 2
#define GPHY_AUTO  4			// Auto-negociate register
#define GPHY_GBCTR 9			// Gigabit control register

// Define bits in GPHY_CTR

#define PHY_CTR_RESET  0x8000	// Reset
#define PHY_CTR_LB     0x4000	// Loop-back enable
#define PHY_CTR_SPDLSB 0x2000	// Speed selection LSB
#define PHY_CTR_AUTOEN 0x1000	// Auto-negotiation enable
#define PHY_CTR_PWRDWN 0x0800	// Power down
#define PHY_CTR_ISOL   0x0400	// Isolate
#define PHY_CTR_RAUTO  0x0200	// Restart auto-negotiation
#define PHY_CTR_FULL   0x0100	// Full duplex
#define PHY_CTR_SPDMSB 0x0040	// Speed selection MSB

// Define bits in GPHY_STS

#define GPHY_STS_100T4  0x8000	// 100base-T4
#define GPHY_STS_100XF  0x4000	// 100base-X full duplex
#define GPHY_STS_100XH  0x2000	// 100base-X half duplex
#define GPHY_STS_10F    0x1000	// 10MBS full duplex
#define GPHY_STS_10H    0x0800	// 10MBS half duplex
#define GPHY_STS_100T2F 0x0400	// 100base-T2 full duplex
#define GPHY_STS_100T2H 0x0200	// 100base-T2 half duplex
#define GPHY_STS_XSTS   0x0100	// Extended status available
#define GPHY_STS_PRESUP 0x0040	// Preamble cuppression capable
#define GPHY_STS_COMP   0x0020	// Auot-negotiation complete
#define GPHY_STS_RMTFLT 0x0010	// Remote fault
#define GPHY_STS_AUTO   0x0008	// Auto-negotiation capability
#define GPHY_STS_OK     0x0004	// Link connected
#define GPHY_STS_JAB    0x0002	// Jabber detect
#define GPHY_STS_XCAP   0x0001	// Extended capability

// Define bits in GPHY_AUTO 

#define PHY_AUTO_100FULL 0x0100
#define PHY_AUTO_100HALF 0x0080
#define PHY_AUTO_10FULL  0x0040
#define PHY_AUTO_10HALF  0x0020

// Define bits in GPHY_GBCTR

#define PHY_GBCTR_ASPAUSE  0x0800
#define PHY_GBCTR_PAUSE    0x0400
#define PHY_GBCTR_1000FULL 0x0200
#define PHY_GBCTR_1000HALF 0x0100


#define TX_DMA_BURST   4
#define RX_DMA_BURST   4
#define RX_FIFO_THRESH 7


// Define structure for the input and output descriptors

typedef volatile struct
{	long  sts;
	long  vlan;
	long  bufr;
	long  bufrhi;
} XFRDSP;

// Define structure for packets in the input ring buffer (not using descriptors)

typedef volatile _Packed struct
{	ushort sts;
	ushort len;
	ENPKT  pkt;
} BPKT;

// Define bits in the receive status register in the received packet header

#define RTR_RSR_MAR  0x8000		// Multicase address received
#define RTR_RSR_PAM  0x4000		// Physical address received
#define RTR_RSR_BAR  0x2000		// Broadcast address received
#define RTR_RSR_ISE  0x0020		// Invalid symbol error
#define RTR_RSR_RUNT 0x0010		// Runt packet
#define RTR_RSR_LONG 0x0008		// Long packet
#define RTR_RSR_CRC  0x0004		// CRC error in packet
#define RTR_RSR_FAE  0x0002		// Frame alignment error
#define RTR_RSR_ROK  0x0001		// Receive OK

// Define bits in XFRDSP.sts for input descriptor mode

#define ID_STS_OWN  0x80000000	// Buffer owned by controller if set
#define ID_STS_EOR  0x40000000	// Eof of ring
#define ID_STS_FS   0x20000000	// First segment
#define ID_STS_LS   0x10000000	// Last segment
#define ID_STS_MAR  0x08000000	// Multicast address received
#define ID_STS_PAM  0x04000000	// Physical address matched
#define ID_STS_BAR  0x02000000	// Broadcast address received
#define ID_STS_BOVF 0x01000000	// Buffer overflow (Its unclear what this bit
								//   means since it is in a descriptor but
								//   seems to mean that no descriptors are
								//   available!)
#define ID_STS_FOVF 0x00800000	// FIFO overflow (This bit appears to be always
								//   set and does not indicate an error!)
#define ID_STS_RWT  0x00400000	// Receive watchdog timer expired (Have no
								//   idea what this means!)
#define ID_STS_RES  0x00200000	// Receive error summary
#define ID_STS_RUNT 0x00100000	// Runt packet
#define ID_STS_CRC  0x00080000	// CRC error
#define ID_STS_PID  0x00060000	// Protocol ID:
				  				//   0 = Non-IP
								//   1 = TCP/IP
								//   2 = UDP/IP
								//   3 = IP
#define ID_STS_IPF  0x00010000	// IP checksum failure
#define ID_STS_UDPR 0x00008000	// UDP checksum faiiure
#define ID_STS_TCPF 0x00004000	// TCP checksum failure
#define ID_STS_LEN  0x00003FFF	// Received packet length

// Define bits in XFRDSP.sts for output descriptor mode

#define OD_STS_OWN   0x80000000	// Buffer owned by controller if set
#define OD_STS_EOR   0x40000000	// Eof of ring
#define OD_STS_FS    0x20000000	// First segment
#define OD_STS_LS    0x10000000	// Last segment
#define OD_STS_LRG   0x08000000	// Large send (cmd)
#define OD_STS_FUNR  0x02000000	// Fifo underrun (status)
#define OD_STS_TES   0x00800000	// Transmit error summary (status)
#define OD_STS_OWC   0x00400000	// Out of window collision (status)
#define OD_STS_LNKF  0x00200000	// Link failure (status)
#define OD_STS_EXC   0x00100000	// Excessive collisions (status)
#define OD_STS_CCNT  0x000F0000	// Collision count
#define OD_STS_IPCS  0x00040000	// Calculate IP checksum (cmd)
#define OD_STS_TCPCS 0x00020000	// Calculate TCP checksum (cmd)
#define OD_STS_UDPCS 0x00010000	// Calculate UDP checksum (cmd)
#define OD_STS_LEN   0x0000FFFF	// Buffer length

#define MAJV    3
#define MINV    0
#define EDITNUM 0

// Define structure for the IDB for the ENRTA interface

typedef struct enrtaidb__
{	IDB;						// Common part of the IDB
	char    enrtaintreq;		// Interrupt request number
	char    enrtactype;			// Chip type
	char    enrtaflowctl;
	char    enrtacattr;			// Chip attribute bits
	char    enrtabufrszx;		// Non-descriptor receive buffer size index
	char    enrtaxmtmax;		// Maximum number of buffered output packets
	char    fill1[2];
	ushort  enrtabus;			// PCI bus bus number
	ushort  enrtaslot;			// PCI bus slot number
	ushort  enrtafunc;			// PCI bus function number
	char    enrtarevsn;			// PCI version ID
	char    fill2[1];
	long    enrtadevice;		// PCI vendor ID and device ID
	PCIB   *enrtapcib;			// Address of the PCIB (needed for calls to
								//   xospciReadCfgReg and xospciWriteCfgReg)
	ENRTA  *enrtaregs;			// Base address of the memory mapped registers
	vchar  *enrtartxrreg;		// Address of the RTX_TXREQ register (its
								//   different on the 8139C+ and the gigabit
								//   chips)
	long    enrtaintsts;		// Used to transfer interrupt status to fork
								//   level
	long    enrtabmctl;			// Current value for RTF_BMCTL
	union
	{ XFRDSP *enrtaiphyrng;		// Address of beginning of the physical input
								//   header ring if using descriptors
	  char   *enrtaiphybfr;		// Address of beginning of the physical input
	};							//   ring buffer if not using descriptors
	ENPKT **enrtaivirrng;		// Address of beginning of the input virtual
								//   header ring if using descriptors (not used
								//   if not using descriptors)
	long    enrtairngend;		// Byte offset of end of the input ring
	long    enrtaiphyrngphy;	// Physical address of the physical input
								//   descriptor ring if using descriptors or
								//   physical address of the input ring buffer
	long    enrtaitaker;		// Input buffer taker pointer (byte offset into
								//   the virtual input header ring if using
								//   descriptors or into data ring buffer
	long    enrtaphyreg;
	long    enrtabufrsz;		// Total receive buffer size in KB
	long    enrtaocnt;			// Output buffer count
	union
	{ XFRDSP *enrtaophyrng;		// Address of beginning of the physical input
								//   descriptor ring if using descriptors
	  vlong  *enrtaophybfr;		// Address of the RTF_TSAD0 device reg if not
	};							//   using descriptors
	vlong  *enrtaophysts;		// Address of the RTF_TSD0 device reg if not
								//   using descriptors
	ENNPB **enrtaovirrng;		// Address of beginning of the virtual ouput
								//   header ring
	long    enrtaorngend;		// Byte offset of end of the output ring
	long    enrtaophyrngphy;	// Physical address of physical descriptor ring
								//   if using descriptors or physcial address
								//   of first output buffer
	long    enrtaoputter;		// Output buffer ring putter pointer (byte
								//   offset in the virutal output header ring)
	long    enrtaotaker;
	long   *enrtacnts;			// Address of counts XMB
	long    enrtacntspa;
};

ENRTAIDB *enrtaidbhead;
ENRTAIDB *enrtaidbtail;

// Define values for the chip attribute table bytes stored in idb_enrtacattr.
//   These are initialized from attrtbl when a unit is added.

#define CA_DESCP   0x80			// Chip uses descriptors
#define CA_GIGABIT 0x40			// Chip supports 1000MBS speed - This also
								//   implies support for large descriptor
								//   tables (up to 1024 descriptors each) and
								//   for 64 bit physical address.
#define CA_8169S   0x20			// Chip is an 8169S
#define CA_AUTO    0x01			// Auto-negotiation is enabled


NAMETBL2 modetbl[] =
{	{"AUTO"  , RTF_BMCTL_AUTO},
	{"10HD"  , 0},
	{"10FD"  , RTF_BMCTL_DUPLEX},
	{"100HD" , RTF_BMCTL_SPEED},
	{"100FD" , RTF_BMCTL_SPEED|RTF_BMCTL_DUPLEX},
	{"1000FD", 0}
};

NAMETBL2 endistbl[] =
{	{"ENABLED" , (RTF_MEDIASTS_RXFCE << 8) + RTF_MEDIASTS_TXFCE},
	{"DISABLED", 0}
};


ENRTAIDB *enrtafidb;		// Address of first IDB for this type of interface
ENRTAIDB *enrtalidb;		// Address of last IDB for this type of interface

long badpcnt;


// Generate the PCI device id table. This table is only used to determine if
//   the device is a valid type. The determination of the device sub-type is
//   done based on the value of the havedare version ID from the TXCFG register.

long pcitbl[] =
{	0x812910EC,				// RTL 8129
	0x813910EC,				// RTL 8139
	0x816910EC,				// RTL 8169/RTL8169SB
	0x816710EC,				// RTL 8169SC
	0x816810EC,				// RTL 8168B
	0x810110EC,				// RTL 8101E
	0x816810EC,				// RTL 8168SB
	0x12111113,				// Accton 5030
	0x13601500,				// Delta 8139
	0x13604033,				// Addtron 8139
	0x13001186				// D-Link DFE-530TX+
};

	0x89760BDA	????
	0x89770BDA	????


// Generate the table used to identify chips. Value is the expected value
//   for the hardware version id from the chip. Index is the chip type
//   minus 1.

long chiptbl[] =
{	0x00000000,				// RTT_8169          = 1  (Method 1)
	0x00800000,				// RTT_8169S_8110S_1 = 2  (Method 2)
	0x04000000,				// RTT_8169S_9110S_2 = 3  (Method 3)
	0x10000000,				// RTT_8169SB_8110SB = 4  (Method 4)
	0x18000000,				// RTT_8110SC        = 5  (Method 5)
	0x30000000,				// RTT_8168B_8111B_1 = 6  (Method 11)
	0x38000000,				// RTT_8168B_8111B_2 = 7  (Method 12)
	0x34000000,				// RTT_8101E         = 8  (Method 13)
	0x30800000,				// RTT_8100E_1       = 9  (Method 14)
	0x38800000,				// RTT_8100E_2       = 10 (Method 15)
	0x60000000,				// RTT_8129          = 11
	0x70000000,				// RTT_8139          = 12
	0x70000000,				// RTT_8139A         = 13
	0x70800000,				// RTT_8139AG        = 14
	0x78000000,				// RTT_8139B         = 15
	0x7C000000,				// RTT_8130          = 16
	0x74000000,				// RTT_8139C         = 17
	0x74800000				// RTT_8139CP        = 18
};

// Generate table which maps from the chip type to a descriptive text string

char *descriptbl[] =
{	"RTL 8169 (Gigabit)",			// RTT_8169          = 1
	"RTL 8169S/8110S-1 (Gigabit)",	// RTT_8169S_8110S_1 = 2
	"RTL 8169S/8110S-2 (Gigabit)",	// RTT_8169S_8110S_2 = 3
	"RTL 8169SB (Gigabit)",			// RTT_8169SB_8110SB = 4
	"RTL 8169SC (Gigabit)",			// RTT_8118SC        = 5
	"RTL 8168B-1 (Gigabit)",		// RTT_8168B_8110B_1 = 6
	"RTL 8168B-2 (Gigabit)",		// RTT_8168B_8110B_2 = 7
	"RTL 8110B-1 (Gigabit)",		// RTT_8101E         = 8
	"RTL 8100E-1 (Fast/PCIX)",		// RTT_8100E_1       = 9
	"RTL 8100E-2 (FAST/PCIX)",		// RTT_8100E_2       = 10
	"RTL 8129 (Fast)",				// RTT_8129          = 11
	"RTL 8139 (Fast)",				// RTT_8139          = 12
	"RTL 8139A (Fast)",				// RTT_8139A         = 13
	"RTL 8139AG (Fast)",			// RTT_8139AG        = 14
	"RTL 8139B (Fast)",				// RTT_8139B         = 15
	"RTL 8130 (Fast)",				// RTT_8130          = 16
	"RTL 8139C (Fast)",				// RTT_8139C         = 17
	"RTL 8139CP (Fast)"				// RTT_8139CP        = 18
};

/// "RTL 8110B-2 (Gigabit);"
/// "RTL 8101E (Fast/PCIX)"


// Chip attribute table - Indexed by the chip type. This value is copied into
//   idb_enrtacattr when a unit is added.

char attrtbl[] =
{	CA_AUTO|CA_DESCP|CA_GIGABIT,			// RTT_8169          = 1
	CA_AUTO|CA_DESCP|CA_GIGABIT|CA_8169S,	// RTT_8169S_8110S_1 = 2
	CA_AUTO|CA_DESCP|CA_GIGABIT|CA_8169S,	// RTT_8169S_8110S_2 = 3
	CA_AUTO|CA_DESCP|CA_GIGABIT,			// RTT_8169SB_8110SB = 4
	CA_AUTO|CA_DESCP|CA_GIGABIT,			// RTT_8118SC        = 5
	CA_AUTO|CA_DESCP|CA_GIGABIT,			// RTT_8168B_8110B_1 = 6
	CA_AUTO|CA_DESCP|CA_GIGABIT,			// RTT_8168B_8110B_2 = 7
	CA_AUTO|CA_DESCP|CA_GIGABIT,			// RTT_8101E         = 8
	CA_AUTO|CA_DESCP|CA_GIGABIT,			// RTT_8100E_1       = 9
	CA_AUTO|CA_DESCP|CA_GIGABIT,			// RTT_8100E_2       = 9
	CA_AUTO,								// RTT_8129          = 11
	CA_AUTO,								// RTT_8139          = 12
	CA_AUTO,								// RTT_8139A         = 13
	CA_AUTO,								// RTT_8139AG        = 14
	CA_AUTO,								// RTT_8139B         = 15
	CA_AUTO,								// RTT_8130          = 16
	CA_AUTO,								// RTT_8139C         = 17
	CA_AUTO|CA_DESCP						// RTT_8139CP        = 18
};

// Input input ring size table - A positive value indicates that descriptors
//   are used and specifies the number of descriptors in the input ring. A
//   negative indicates that descriptors are not used and specifies the length
//   of the input ring buffer.

long inrngtbl[] =
{	128,					// RTT_8169          = 1
	128,					// RTT_8169S_8110S_1 = 2
	128,					// RTT_8169S_8110S_2 = 3
	128,					// RTT_8169SB_8110SB = 4
	128,					// RTT_8118SC        = 5
	128,					// RTT_8168B_8110B_1 = 6
	128,					// RTT_8168B_8110B_2 = 7
	 64,					// RTT_8101E         = 8
	 64,					// RTT_8100E_1       = 9
	 64,					// RTT_8100E_2       = 9
	-64,					// RTT_8129          = 11
	-64,					// RTT_8139          = 12
	-64,					// RTT_8139A         = 13
	-64,					// RTT_8139AG        = 14
	-64,					// RTT_8139B         = 15
	-64,					// RTT_8130          = 16
	-64,					// RTT_8139C         = 17
	 64						// RTT_8139CP        = 18
};


// Device characteristics table for ENRTA NET devices

static CHARFNC chrgetboard;
static CHARFNC chrgetlink;
static CHARFNC chrgetmode;
static CHARFNC chrsetmode;
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
		0                , offsetof(ENRTAIDB, enrtabus)),
  CTITEM(PCISLOT , DECV, U,  2, &xospciMsgSlot   , xosnetGetIdb2,
		0                , offsetof(ENRTAIDB, enrtaslot)),
  CTITEM(PCIFUNC , DECV, U,  2, &xospciMsgFunc   , xosnetGetIdb2,
		0                , offsetof(ENRTAIDB, enrtafunc)),
  CTITEM(PCIVENID, HEXV, U,  2, &xospciMsgVen    , xosnetGetIdb2,
		0                , offsetof(ENRTAIDB, enrtadevice)),
  CTITEM(PCIDEVID, HEXV, U,  2, &xospciMsgDev    , xosnetGetIdb2,
		0                , offsetof(ENRTAIDB, enrtadevice) + 2),
  CTITEM(PCIREV  , HEXV, U,  1, &xospciMsgRev    , xosnetGetIdb1,
		0                , offsetof(ENRTAIDB, enrtarevsn)),
  CTITEM(IOREG   , HEXV, U,  4, &knlChrMsgIoReg  , xosnetGetIdb4,
		0                , offsetof(ENRTAIDB, bioreg)),
  CTITEM(MEMREG  , HEXV, U,  4, &knlChrMsgMemReg , xosnetGetIdb4,
		0                , offsetof(ENRTAIDB, enrtaphyreg)),
  CTITEM(INT     , DECV, U,  1, &knlChrMsgInt    , xosnetGetIdb1,
		0                , offsetof(ENRTAIDB, intlvl)),
  CTITEM(BUFRSIZE, DECV, U,  4, &xosnetMsgBufrSz , xosnetGetIdb4,
		0                , offsetof(ENRTAIDB, enrtabufrsz)),
  CTITEM(LINK    , TEXT, U, 12, &xosnetMsgLink   , chrgetlink,
		0                , 0),
  CTITEM(MODE    , TEXT, U, 12, &xosnetMsgMode   , chrgetmode,
		chrsetmode       , 0),
  CTITEM(TXFLOW  , TEXT, U, 12, &xosnetMsgTxFlow , chrgettxflow,
		chrsettxflow     , 0),
  CTITEM(RXFLOW  , TEXT, U, 12, &xosnetMsgRxFlow , chrgetrxflow,
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


// Protocol translation table which converts from standard internal protocol
//   values to device protocol values

long etypetbl[] =
{	-1,					//           = 80000000 - Illegal
	0xBA81,				// PROT_SSL1 = 80000001 - SSL 1st Ethernet protocol
	0xBB81,				// PROT_SSL2 = 80000002 - SSL 2nd Ethernet protocol
	0xBC81,				// PROT_SSL3 = 80000003 - SSL 3rd Ethernet protocol
	0xBD81,				// PROT_SSL4 = 80000004 - SSL 4th Ethernet protocol
	0xBE81,				// PROT_SSL5 = 80000005 - SSL 5th Ethernet protocol
	0xBF81,				// PROT_SSL6 = 80000006 - SSL 6th Ethernet protocol
	0xC081, 			// PROT_SSL7 = 80000007 - SSL 7th Ethernet protocol
	0xC181,				// PROT_SSL8 = 80000008 - SSL 8th Ethernet protocol
	-1,					//           = 80000009 - Illegal
	-1,					// 	         = 8000000A - Illegal
	-1,					//           = 8000000B - Illegal
	-1,					//           = 8000000C - Illegal
	-1,					//           = 8000000D - Illegal
	-1,					//           = 8000000E - Illegal
	-1,					//           = 8000000F - Illegal
	0x0008,				// PROT_IP   = 80000010 - Internet IP protocol
	0x0608,				// PROT_ARP  = 80000011 - Internet ARP protocol
	0x3580				// PROT_RARP = 80000012 - Internet RARP protocol
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
	if ((rtn = sysIoDriverRegister("ERTA", enrtaaddunit, &xosnetCcb)) >= 0)
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
	AUDATA    data;
	long     *pcitp;
	long      rtn;
	long      amnt;
	long     *lpnt;
	ENPKT   **vrngpnt;
	XFRDSP   *prngpnt;
	char     *bpnt;
	long      bphy;
	long      left;
	IDB      *idbp;
	ENRTAIDB *idb;
	char     *irout;		// Interrupt routine offset
	long      ioreg;		// Base IO register number
	long      inum;			// Interrupt level
	long      bufrszx;
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
	char   config;			// Value for idb_config
	char   hvxmres;
	char   cattr;
	ENRTA *regs;
	long   phyregs;
	PCIB  *pcib;
	long   ctype;
	ENPKT **ivirrng;		// Virtual address of input virtural header ring
							//   (not used if not using descriptors)
	union
	{ XFRDSP *iphyrng;		// Virtual address of input physical header ring
	  char   *iphybfr;
	}       p;
	long    iphyrngphy;		// Physical address of the physical input
							//   descriptor ring if using descriptors or
							//   physical address of the input ring buffer
	ENNPB **ovirrng;		// Address of output virtual header ring
	XFRDSP *ophyrng;		// Address of output physical header ring
	vlong  *ophysts;
	long    ophyrngphy;		// Physical address of physical descriptor ring
							//   if using descriptors or physcial address of
							//   first output buffer ?????
	long  *cnts;			// Address of the counts buffer
	long   cnt;
	long   bits;

	hvxmres = 0;
	p.iphyrng = 0;						// Clear memory pointers here in case we
	ivirrng = 0;						//   get an error before memory is
	ophyrng = 0;						//   allocated
	cnts = 0;
	config = 0;
	data.bufrsz = 64;
	data.pcibus = -1;
	data.pcislot = -1;
	data.pcifunc = -1;
	data.bmctl = RTF_BMCTL_AUTO;		// Store default mode (auto)
	data.txflow = RTF_MEDIASTS_TXFCE;	// Store default flow control status
	data.rxflow = RTF_MEDIASTS_RXFCE;
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

	bits = regs->R_txcfg & 0x7C800000;	// Get the configuration bits we care
										//   about
	cnt = sizeof(chiptbl)/sizeof(long);	// Look for a match in our table
	lpnt = chiptbl;
	do
	{
		if (bits == *lpnt++)
			break;
	} while (--cnt > 0);				// If not found, assume plain 8139 or
										//   some kind of descriptor based chip
	ctype = (cnt > 0) ? (lpnt - chiptbl) : ((bits & 0x40000000) ?
			RTT_8139 : RTT_8169);
	cattr = attrtbl[ctype - 1];
	if (regs->R_cmd & RTR_CMD_RESET)	// Is the reset bit set now?
		return (aufail(ER_PDTYP));		// Yes - something is very wrong!
	regs->R_cmd |= RTR_CMD_RESET;		// Reset the interface

	// Allocate memory for the buffer rings

	if (inrngtbl[ctype - 1] >= 0)
	{
		// Here if chip uses descriptors - We maintain a virtual buffer header
		//   ring for both the input and output ring. These rings simply
		//   consist of the virtual addresses of each buffer.

		// We allocate buffers as follows:

		//   Description                         Size              Location
		//   Output physical buffer header ring  up to 1024 bytes  XMB
		//   Output virtual buffer header ring   up to 256 bytes   Static memory
		//   Input physical buffer header ring   up to 4096 bytes  XMB
		//   Input virtual buffer header ring    up to 1024 bytes  Static memory
		//   Input buffers                       up to 256 KB      XMB
		//   Counts buffer                       64 bytes          XMB

		// The physical buffer header rings must start on a 256 byte boundry.

		// The output ring size is set to between 16 and 64 at assembly time.
		//   The input buffer size can be specified when the unit is added. We
		//   always use 1KB input buffers. Inorder to allow us to use an XMB
		//   for the input descriptor ring we restrict the maximum number of
		//   input buffers to 256, allowing up to a total of 256KB of total
		//   input buffer space. (Hopefully this is enough!) The total input
		//   buffer size can be specified in the range of 16KB to 512KB (except
		//   for the 8139E which has a maximum total buffer size of 64KB since
		//   it only supports 64 descriptors). The input ring size is the
		//   buffer size (in bytes) divided by 64.

		// The physical buffer descriptor rings are defined in the RealTek
		//   documentation (such as it is). The virtual buffer descriptor rings
		//   simply contain the virtual address of each buffer in the ring. All
		//   input buffers are allocated here and are a fixed size (1024 bytes).
		//   To minimize fragmentation we use the smallest number of XMBs we
		//   can, combining as many buffers as possible in each XMB (up to 64).
		//   No output buffers are allocated since output is done direct from
		//   the packet buffers.

		data.bufrsz = (data.bufrsz == 0) ? 256 : (data.bufrsz < 8) ? 8 :
				(data.bufrsz < 16) ? 16 : (data.bufrsz < 32) ? 32 :
				(data.bufrsz < 64) ? 64 : (data.bufrsz < 128) ? 128 : 256;
		rtn = (cattr & CA_GIGABIT) ? 256 : 64;
		if (data.bufrsz > rtn)
			data.bufrsz = rtn;

		amnt = OUTDESCP + data.bufrsz;
		if ((rtn = sysMemGetXMem(amnt, (char **)&ovirrng)) < 0)
			return (aufail(rtn));

		ivirrng = (ENPKT **)(ovirrng + OUTDESCP);
		sysLibMemSet((char *)ovirrng, 0, amnt);

		// Get memory for the output physical descriptor ring

		if ((rtn = sysMemGetXmb(OUTDESCP * 16, (char **)&ophyrng)) < 0)
			return (aufail(rtn));

		ophyrngphy = sysMemGetPhyAddr((void *)ophyrng);

		// Get memory for the input physical descriptor ring

		if ((rtn = sysMemGetXmb(data.bufrsz * 16, (char **)&p.iphyrng)) < 0)
			return (aufail(rtn));
		iphyrngphy = sysMemGetPhyAddr((void *)p.iphyrng);

		// Get XMBs for the input buffers and set up the header rings

		vrngpnt = (ENPKT **)ivirrng;
		prngpnt = p.iphyrng;
		left = data.bufrsz;
		do
		{
			if ((amnt = left) > 64)
				amnt = 64;
			if ((rtn = sysMemGetNcXmb(amnt << 10, &bpnt)) < 0)
				return (aufail(rtn));
			bphy = sysMemGetPhyAddr(bpnt);

			// Here with an XMB - Divide it up into 1024 byte buffers. Note
			//   that we do not set the OWN bit here. It will be set when
			//   input is reset.

			*vrngpnt++ = (ENPKT *)bpnt;	// Store virtual address in the virtual										//   ring
			prngpnt->sts = 1024;		// Store buffer size, clear status bits
			prngpnt->vlan = 0;
			prngpnt->bufr = bphy;		// Store physical address in the
			prngpnt->bufrhi = 0;		//   physical ring
			prngpnt++;
			bpnt += 1024 * 64;
			bphy += 1024 * 64;
		} while ((left -= amnt) > 0);
	}
	else
	{
		// Here if chip does not use descriptors - In this case we allocate
		//   a single large contiguous buffer used as follows:

		//   Description                     Size
		//   Input ring buffer               8192, 16384, 32768, or 65536 bytes
		//   Unused                          256 bytes
		//   Ouput buffers (output virtual   16 bytes
		//     header ring)

		// The input ring buffer is first, followed by an unused area followed
		//   by the output virtual descriptor ring. The unused area provides a
		//   "nice" address for the ring to make debugging a little easier.
		//   The default size for the input ring buffer is 65536. The other
		//   sizes can be specified when the unit is added if desired.

		// NOTE: The Realtek datasheets all indicate that the input ring buffer
		//       is 16 bytes larger than the indicated power of 2. Thus instead
		//       of 65536 the size would be 65552. None of the chips we have
		//       seen behave this way. They all use the exact power of 2. Just
		//       to be safe, we allocate an extra page after the receive ring
		//       and leave the first 256 bytes of this page unused just in
		//       case the chip decides to modify some of them. If the chip
		//       does actually use the extra 16 bytes as part of the ring, this
		//       will cause a serious error which will reset the chip every
		//       100 or so packets input (since the buffer wrapping logic will
		//       not work right), but at least it will not randomly write on
		//       someone else's memory!
		//       My suspicion is that it does not really "use" the extra 16
		//       bytes as part of the ring but may sometimes modify them. If
		//       so, the current way this is handled is correct.

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
		if (data.bufrsz <= 32)
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
				&iphyrngphy, (char **)&p.iphyrng)) < 0)
			return (aufail(rtn));			// Get a large buffer

///	PUSHL	ESI
///	PUSHL	#0x99
///	PUSHL	#11000
///	CALL	sysMemSet##

/// FIX THIS !!!!!

///		ovirrng = iphyrng + (data.bufrsz << 10) + 256;
		ophyrng = (XFRDSP *)&regs->F_tsad0; // Get the address of the 1st output
											//   physical "descriptor" ring,
											//   which in this case points to
											//   the chip's first output
											//   buffer address register.
		ophysts = &regs->F_tsd0;		// Get the address of the 1st output
	}									//   status register

	// Get an XMB for the counts buffer

	if ((rtn = sysMemGetNcXmb(-XMBX_64, (char **)&cnts)) < 0)
		return (aufail(rtn));

	// Here with the buffers allocated and set up.

	if (regs->R_cmd & RTR_CMD_RESET)	 // Has the chip reset finished?
		return (aufail(ER_DEVER));

	regs->R_intsts = 0xFFFF;			// Reset all interrupt requests

	sysSchSDelay(5);

	if (regs->R_intsts & 0xFFFF)		// Did it work?
		return (aufail(ER_DEVER));		// No - fail

	// Make our IDB

	if ((rtn = xosnetMakeIdb(unit, sizeof(ENRTAIDB), &enrtadctbl, &idbp)) < 0)
		return (aufail(rtn));
	idb = (ENRTAIDB *)idbp;

	// Set up the IDB

	*(long *)idb->typname = 'ERTA';		// Store type name
	idb->iftype = IFT_MULTI;			// Store interface type
	idb->hwhdrsz = 14;					// Store hardware header size
	idb->hwpktsz = 1500;				// Store maximum packet size
	idb->hlen = 6;						// Store hardware address size
	idb->hwtype = 0x0100;				// Store hardware type value for APR

///	idb->cntoffset = offsetof(qqq, en_count); // Store offset of count field

	idb->ifdisp = &enrtadisp;
	idb->state = LNKS_ESTAB;
	idb->fdisp = (DFRKFNC *)enrtafork;	// Store address of fork routine
	idb->bioreg = ioreg;
	idb->enrtaregs = regs;
	idb->enrtartxrreg = (ctype == RTT_8139CP) ? &regs->C_txreq : &regs->G_txreq;
	idb->enrtaphyreg = phyregs;
	idb->enrtaregs = regs;
	idb->enrtaophyrng = ophyrng;
	idb->enrtaophysts = ophysts;
	idb->enrtaovirrng = (ENNPB **)ovirrng;
	idb->enrtaophyrngphy = ophyrngphy;
	idb->enrtaiphyrng = p.iphyrng;
	idb->enrtaivirrng = ivirrng;
	idb->enrtaiphyrngphy = iphyrngphy;
	if ((idb->enrtacnts = cnts) != NULL)
		idb->enrtacntspa = sysMemGetPhyAddr(cnts);
	idb->enrtabufrsz = data.bufrsz;
	idb->enrtabufrszx = bufrszx;
	idb->enrtactype = ctype;
	idb->enrtacattr = attrtbl[ctype - 1];
	if (cattr & CA_DESCP)
	{
		idb->enrtairngend = OUTDESCP * 4;
		idb->enrtaxmtmax = OUTDESCP - 1;
	}
	else
	{
		idb->enrtairngend = data.bufrsz << 10;
		idb->enrtaxmtmax = 4 - 1;
	}
	idb->enrtaflowctl = data.txflow | data.rxflow;
	idb->hwaddr = *(llong *)regs->R_netaddr; // Store MAC address in the IDB
	idb->enrtabus = pcib->bus;
	idb->enrtaslot = pcib->slot;
	idb->enrtafunc = pcib->func;
	idb->enrtadevice = pcib->devid;
	idb->enrtarevsn = pcib->revid;

	if (enrtaidbtail != NULL)			// Link into our list of IDBs
		enrtaidbtail->next = (IDB *)idb;
	else
		enrtaidbhead = idb;
	enrtaidbtail = idb;

	// Set up the interrupt code for this unit

	extern char fix1[], fix2[], fix3[], fix4[], fix5[], fix6[], enrtaexit[];

	if ((rtn = sysMemGetXCode(4 * (long)&ENRTAINTSZ, (char **)&irout)) < 0)
		return (aufail(rtn));			// Get space for interrupt code

	INT3;

	sysLibMemCpyLong((long *)irout, (long *)&enrtaint, (long)&ENRTAINTSZ);
										// Copy the prototype code
	*(vushort **)(((long)fix1) + (long)irout) = &regs->R_intsts; // Do fix-up
	*(vushort **)(((long)fix2) + (long)irout) = &regs->R_intsts;
	*(vlong **)(((long)fix3) + (long)irout) = &idb->enrtaintsts;
	*(vushort **)(((long)fix4) + (long)irout) = &regs->R_intmask;
	*(ENRTAIDB **)(((long)fix5) + (long)irout) = idb;
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
// Function: enrtaaubufrsz - set the value of the BUFRSIZE
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

#if 0

// PHY initialization data for 8169SC (method 4)

ushort initm4[] =
{	0x1F, 0x0002,
	0x01, 0x90D0,
	0x1F, 0x0000,
	0xFFFF, 0xFFFF

// PHY initialization data for 8169S/8110S

ushort initm23[] =
{	0x1F, 0x0001,
	0x06, 0x006E,
	0x08, 0x0708,
	0x15, 0x4000,
	0x18, 0x65C7,

	0x1F, 0x0001,
	0x03, 0x00A1,
	0x02, 0x0008,
	0x01, 0x0120,
	0x00, 0x1000,
	0x04, 0x0800,
	0x04, 0x0000,

	0x03, 0xFF41,
	0x02, 0xDF60,
	0x01, 0x0140,
	0x00, 0x0077,
	0x04, 0x7800,
	0x04, 0x7000,

	0x03, 0x802F,
	0x02, 0x4F02,
	0x01, 0x0409,
	0x00, 0xF0F9,
	0x04, 0x9800,
	0x04, 0x9000,

	0x03, 0xDF01,
	0x02, 0xDF20,
	0x01, 0xFF95,
	0x00, 0xBA00,
	0x04, 0xA800,
	0x04, 0xA000,

	0x03, 0xFF41,
	0x02, 0xDF20,
	0x01, 0x0140,
	0x00, 0x00bb,
	0x04, 0xB800,
	0x04, 0xB000,

	0x03, 0xDF41,
	0x02, 0xDC60,
	0x01, 0x6340,
	0x00, 0x007d,
	0x04, 0xD800,
	0x04, 0xD000,

	0x03, 0xDF01,
	0x02, 0xDF20,
	0x01, 0x100a,
	0x00, 0xA0FF,
	0x04, 0xF800,
	0x04, 0xF000,

	0x1F, 0x0000,
	0x0B, 0x0000,
	0x00, 0x9200,
	0xFFFF, 0xFFFF
}; 

ushort *phytbl[] =
{	0,					// RTT_8169          = 1
	initm23,			// RTT_8169S_8110S_1 = 2
	initm23,			// RTT_8169S_8110S_2 = 3
	initm4,				// RTT_8169SB_8110SB = 4
	0,					// RTT_8118SC        = 5
	0,					// RTT_8168B_8110B_1 = 6
	0,					// RTT_8168B_8110B_2 = 7
	0,					// RTT_8101E         = 8
	0,					// RTT_8100E_1       = 9
	0,					// RTT_8100E_2       = 9
	0,					// RTT_8129          = 11
	0,					// RTT_8139          = 12
	0,					// RTT_8139A         = 13
	0,					// RTT_8139AG        = 14
	0,					// RTT_8139B         = 15
	0,					// RTT_8130          = 16
	0,					// RTT_8139C         = 17
	0					// RTT_8139CP        = 18
};

// Function to initialize the PHY - this is magic taken from the Linux driver!

phyi_ctype 16t
phyi_reg   12t

phyinit:PUSHL	ESI
	ENTER	0, 0
	MOVL	EAX, phyi_ctype[EBP]
	MOVL	ESI, phytbl[EAX*4]
	TESTL	ESI, EBX
	JE	8_
4_:	PUSHL	phyi_reg[EBP]
	PUSHL	[ESI]
	CALL	phywrite
	TESTL	EAX, EAX
	JS	8_
	ADDL	ESI, #4
	CMPL	 = #-1
	JNE	4_
	CLRL	EAX
8_:	LEAVE
	POPL	ESI
	RET	8t
.PAGE





enrtaausetmode:
	MOVL	EBX, #modetbl
	MOVL	ECX, #MODETBLSZ
	CALL	knlGetDcVal2##		// Get new value
	JC	4_			// If error
	MOVL	data->bmctl[EBP], EAX
4_:	RET


#endif

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
	return (sysLibStrNMovZ(val, descriptbl[((ENRTAIDB *)((NETDCB *)
			knlTda.dcb)->ifidb)->enrtactype - 1], cnt));
}


//***********************************************************************
// Function: chrgetlink - Get the value of the LINK characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC chrgetlink(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	ENRTA    *regs;
	ENRTAIDB *idb;
	char     *str;

	idb = (ENRTAIDB *)((NETDCB *)knlTda.dcb)->ifidb;
	regs = idb->enrtaregs;
	str = (idb->enrtacattr & CA_GIGABIT) ?	// Is this a gigabit chip?
			((regs->G_physts & RTG_PHYSTS_LINK) ? "Link" : "NoLink") :
			((regs->F_bmsts & RTF_BMSTS_LINK) ? "Link" : "NoLink");
	return (sysLibStrNMovZ(val, str, cnt));
}


//***********************************************************************
// Function: chrgetmode Get the value of the MODE characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC chrgetmode(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	ENRTA    *regs;
	ENRTAIDB *idb;
	char     *str;
	int       mbits;
	int       cbits;
	int       sbits;

	idb = (ENRTAIDB *)((NETDCB *)knlTda.dcb)->ifidb;
	regs = idb->enrtaregs;
	if (idb->enrtacattr & CA_GIGABIT) 	// Is this a gigabit chip?
		mbits = regs->G_physts;			// Yes - get the PHY status bits
	else								// No - map the status and control bits
	{									//   to match the gigabit chip
		cbits = regs->F_bmctl;			// Get the current control bits
		sbits = regs->F_bmsts;			// And status bits
		mbits = ((cbits&RTF_BMCTL_AUTO) && (sbits&RTF_BMSTS_AUTOCMP) == 0) ?
				0 : (cbits & RTF_BMCTL_SPEED) ? RTG_PHYSTS_100 : RTG_PHYSTS_10;
		if (cbits & RTF_BMSTS_LINK)
			mbits |= RTG_PHYSTS_LINK;
	}
	if (idb->enrtacattr == CA_AUTO)		// Auto-negoiation enabled?
	{
		if ((mbits & RTG_PHYSTS_LINK) == 0) // Yes is the link up?
			str = "Auto-NoLink";
		else
		{
			if ((mbits & (RTG_PHYSTS_10|RTG_PHYSTS_100|RTG_PHYSTS_1000)) == 0)
				str = "Auto-???";
			else
			{
				if (mbits & RTG_PHYSTS_1000)
					str = "Auto-1000FD";
				else if (mbits & RTG_PHYSTS_100)
					str = (mbits & RTG_PHYSTS_FULL)	? "Auto-100FD" :
							"Auto-100HD";
				else
					str = (mbits & RTG_PHYSTS_FULL)	? "Auto-10FD" : "Auto-10HD";
			}
		}
	}
	else
	{
		if (mbits & RTG_PHYSTS_1000)
			str = "1000FD";
		else if (mbits & RTG_PHYSTS_100)
			str = (mbits & RTG_PHYSTS_FULL)	? "100FD" : "100HD";
		else
			str = (mbits & RTG_PHYSTS_FULL)	? "10FD" : "10HD";
	}
	return (sysLibStrNMovZ(val, str, cnt));
}


//***********************************************************************
// Function: chrsetmode Set the value of the MODE characteristic
// Returned: Number of bytes stored or a negative XOS error code if error
//***********************************************************************

static long XOSFNC chrsetmode(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{


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
	MOVW	RTF_BMCTL[EDX], AX
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
	ENRTA    *regs;
	ENRTAIDB *idb;
	char     *str;

	idb = (ENRTAIDB *)((NETDCB *)knlTda.dcb)->ifidb;
	regs = idb->enrtaregs;
	str = (idb->enrtacattr & CA_GIGABIT) ?	// Is this a gigabit chip?
			((regs->G_physts & RTG_PHYSTS_TXFC) ? "Enabled" : "Disabled") :
			((regs->F_mediasts & RTF_MEDIASTS_TXFCE) ? "Enabled" : "Disabled");
	return (sysLibStrNMovZ(val, str, cnt));
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
	ENRTA    *regs;
	ENRTAIDB *idb;
	char     *str;

	idb = (ENRTAIDB *)((NETDCB *)knlTda.dcb)->ifidb;
	regs = idb->enrtaregs;
	str = (idb->enrtacattr & CA_GIGABIT) ?	// Is this a gigabit chip?
			((regs->G_physts & RTG_PHYSTS_RXFC) ? "Enabled" : "Disabled") :
			((regs->F_mediasts & RTF_MEDIASTS_RXFCE) ? "Enabled" : "Disabled");
	return (sysLibStrNMovZ(val, str, cnt));
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

	MOVB	AH, RTF_MEDIASTS[EDX]
	ANDB	AH, #{~RTF_MEDIASTS_TXFCE&0FFh}
	ORB	AH, AL
	ANDB	idb_enrtaflowctl = #{~RTF_MEDIASTS_TXFCE&0FFh}
	ORB	idb_enrtaflowctl = AL
	MOVB	RTF_MEDIASTS[EDX], AH
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
	MOVB	AL, RTF_MEDIASTS[EDX]
	ANDB	AL, #~RTF_MEDIASTS_RXFCE
	ORB	AL, AH
	ANDB	idb_enrtaflowctl = #{~RTF_MEDIASTS_RXFCE&0FFh}
	ORB	idb_enrtaflowctl = AH
	MOVB	RTF_MEDIASTS[EDX], AL
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
	return (sysUSPutULLongV(val, ((NETDCB *)knlTda.dcb)->ifidb->hwaddr, cnt));
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
	return (*((llong *)((ENNPB *)npb)->srchwaddr) & 0xFFFFFFFFFFFF);
}


//****************************************************
// Function: enrtareset - Reset the Ethernet interface
// Returned: Nothing
//****************************************************

static void XOSFNC enrtareset(
	ENRTAIDB *idb)
{
	volatile int rcnt;

	ENRTA  *regs;
	XFRDSP *dpnt;
	NPB    *npb;
	int     cnt;

	regs = idb->enrtaregs;
	regs->R_cmd = RTR_CMD_RESET;		// Reset the controller
	rcnt = 10000;
	while (regs->R_cmd & RTR_CMD_RESET && --rcnt > 0)
		;

	// First clean up anything that might be going on now

	regs->R_intmask = 0;				// Ensure no more interrupts (This is
	idb->enrtaintsts = 0;				//   not really needed, but it does 
										//   not hurt.)
	while (idb->enrtaocnt != 0)			// Have any pending output?
	{
		npb = (NPB *)idb->enrtaovirrng[idb->enrtaotaker];
		idb->enrtaovirrng[idb->enrtaotaker] = 0;
		xosnetFinXmit(ER_DEVER, (IDB *)idb, npb);
		if (++(idb->enrtaotaker) >= idb->enrtaorngend)
			idb->enrtaotaker = 0;
		idb->enrtaocnt--;
	}

	// Here with all pending output handled

	idb->xmtavail = idb->enrtaxmtmax;

	idb->enrtaoputter = 0;				// Clear our ring pointers
	idb->enrtaotaker = 0;
	idb->enrtaitaker = 0;

	// Now set up the buffer rings

	if (idb->enrtacattr & CA_DESCP)		// Using descriptors?
	{
		cnt = idb->enrtabufrsz;			// Yes - initialize the input header
		dpnt = idb->enrtaiphyrng;		//   ring
		do
		{
			dpnt->sts = ID_STS_OWN + 1024;
			dpnt->vlan = 0;
			dpnt++;
		} while (--cnt > 0);
		(dpnt - 1)->sts = ID_STS_OWN + ID_STS_EOR + 1024;

		cnt = 9999;						// Initialize the output header ring
		dpnt = idb->enrtaophyrng;
		do
		{
			dpnt->sts = 0;
			dpnt->vlan = 0;
			dpnt->bufr = 0;
			dpnt->bufrhi = 0;
			dpnt++;
		} while (--cnt > 0);
		(dpnt - 1)->sts = OD_STS_EOR;

		regs->D_rxd = idb->enrtaiphyrngphy;   // Give the header ring addresses
		regs->D_nptxd = idb->enrtaophyrngphy; //   to the controller
	}
	else
	{
		// Here if not using descriptors

		regs->F_tsd0 = RTF_TSD_OWN;		// Make sure driver owns the transmit
		sysSchSDelay(2);				//   descriptors (Reset should have
		regs->F_tsd1 = RTF_TSD_OWN;		//   done this, this just makes sure!)
		sysSchSDelay(2);
		regs->F_tsd2 = RTF_TSD_OWN;
		sysSchSDelay(2);
		regs->F_tsd3 = RTF_TSD_OWN;
		sysSchSDelay(2);
		regs->F_rbaddr = idb->enrtaiphyrngphy; // Set the receive buffer address
	}

	// Now set up the hardware - there are 4 cases:
	//     Non-descriptor based 100MBS chips
	//     Descriptor based 100MBS chips (8139C+)
	//     8169/8169S(B) gigabit chips
	//     Other gibabit chips
	// There are significant differences due to the use or non-use of
	//   descriptors and to the type of PHY used by the gigabit chips. There
	//   is some duplication of code in the 4 cases but it has been left
	//   that way to make it easier to follow.

	regs->R_eepcfg = 0xC0;				// Enable writing config registers
	if (idb->enrtacattr & CA_GIGABIT)	// Is this a gigabit chip?
	{
		// Here to reset a gigabit chip

		if (idb->enrtactype < RTT_8118SC) // Is this an 8169/8169S(B)?
		{
			// Here to reset an 8169 or 8169S(B)

			regs->R_cmd = RTR_CMD_RXE|RTR_CMD_TXE;

			regs->D_etth = 0x3F;		// No early transmit
			regs->D_cpcmd = regs->D_cpcmd | RTD_CPCMD_MRWEN;
										// Set up the DMA configuration

			xospciWriteCfgReg(idb->enrtapcib, 0x0C,
					(xospciReadCfgReg(idb->enrtapcib, 0x0C) & 0xFFFFFF00) |
					0x0C);

			regs->G_rxmxpkt = 1522;		// Set maximum packet size (Is this
										//   right?)
			regs->R_rxcfg = (regs->R_rxcfg & 0xFE7E1880) | 0x0000E70A;
										// Should look at the DMA thresholds!!
			regs->R_txcfg = 0x03000700;	// Set DMA burst size and interframe gap
			regs->D_cpcmd = regs->D_cpcmd; // More magic!
			regs->D_cpcmd = (idb->enrtacattr & CA_8169S) ? 0x4008 : 0x0008;
			regs->G_undoc = 0;			// Undocumented register!!
			regs->R_eepcfg = 0;			// Lock the configuration registers
			sysSchSDelay(2);
			regs->R_multint = regs->R_multint & 0xF000;
										// No early interrupt (Don't know why
										//   the high 4 bits are preserved!)
		}
		else
		{
			// Here to reset a gigabit chip other than an 8169 or 8169S(B)

			CRASH('GIGA');

			// Note: The Linux driver says you must enable Tx/Rx before setting
			//       transfer thresholds!

			regs->R_cmd = RTR_CMD_RXE|RTR_CMD_TXE;
///			regs->R_rxcfg = (idb->enrtabufrsz << qq) | (RX_FIFO_THRESH << 13) |
///					(RX_DMA_BURST << 8) | RTR_RXC_APM|RTR_RXC_AB;
			regs->R_txcfg = (TX_DMA_BURST << 8) | 0x03000000;
			regs->F_rbaddr = idb->enrtaiphyrngphy; // Set receive ring buffer
												   //   address
			regs->F_bmctl = idb->enrtabmctl; // Set the mode
			regs->F_mediasts = idb->enrtaflowctl; // Set flow control enables
		}
	}
	else
	{
		// Here to reset other than a gigabit chip

		if (idb->enrtacattr & CA_DESCP)	// Using descriptors?
		{
			// Here if using descriptors (only the 8139C+)

			CRASH('CPLS');

		}
		else
		{
			// Here if not using descriptors

			idb->xmtavail = 3;			// We can handle 3 output packets at
										//   once

			// Note: The Linux driver says you must enable Tx/Rx before setting
			//       transfer thresholds!

			regs->R_cmd = RTR_CMD_RXE|RTR_CMD_TXE;
			regs->R_rxcfg = (idb->enrtabufrszx << 11) | (RX_FIFO_THRESH << 13) |
					(RX_DMA_BURST << 8) | RTR_RXC_APM | RTR_RXC_AB;
			regs->R_txcfg = (TX_DMA_BURST << 8) | 0x03000000;
			regs->R_eepcfg = 0xC0;		// Enable configuration registers

			//// SET SOME CONFIG1 BITS HERE???

			regs->F_bmctl = idb->enrtabmctl; // Set the mode
			regs->F_mediasts = idb->enrtaflowctl; // Set flow control enables
			regs->R_eepcfg = 0;			// Lock the configuration registers
			sysSchSDelay(2);
		}
	}
	regs->R_intmask = RTR_INT_SER|RTR_INT_RFO|RTR_INT_ULC|RTR_INT_RBO|
			RTR_INT_TER|RTR_INT_TOK|RTR_INT_RER|RTR_INT_ROK;
										// Enable interrupts
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
	*(long *)(((ENNPB *)npb)->srchwaddr+0) =	 // Store our address in the
			*(long *)&((ENRTAIDB *)idb)->hwaddr; //   packet as the source
	*(ushort *)(((ENNPB *)npb)->srchwaddr + 4) = //   address
			*(ushort *)(((char *)&((ENRTAIDB *)idb)->hwaddr) + 4);
	npb->npofs = sizeof(PKT);			// Store offset of next level
	npb->count = sizeof(PKT);			// Store initial packet size
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
	int length;
	int oput;

	npb->sts |= NPS_XMITING;
	length = npb->count;
	if (length > 1514)					// Is the packet too big?
		length = 1514;					// Yes - just use as much as we can!

	((ENRTAIDB *)idb)->enrtaocnt++;

	oput = ((ENRTAIDB *)idb)->enrtaoputter; // Make this easier to read and give
											//   the compiler a little help.
	((ENRTAIDB *)idb)->enrtaovirrng[oput] = (ENNPB *)npb;
										// Store address of the packet in the
										//   virtual ring
	if (((ENRTAIDB *)idb)->enrtacattr & CA_DESCP) // Using descriptors?
	{
		// Here if using descriptors - We do not need to pad short packets
		//   since the chip does this.

		((ENRTAIDB *)idb)->enrtaophyrng[oput].bufr = 
				sysMemGetPhyAddr(((char *)npb) + sizeof(ENNPB));
										// Store physical address of the packet
										//   in the physical ring
		((ENRTAIDB *)idb)->enrtaophyrng[oput].sts =
				(((ENRTAIDB *)idb)->enrtaophyrng[oput].sts & OD_STS_EOR) |
				(OD_STS_OWN | OD_STS_FS | OD_STS_LS) | length;
										// Store length and set the OWN bit
										//   (Must preserve the EOR bit.) - This
										//   will make the packet visible to the
										//   chip if output is active.
		*((ENRTAIDB *)idb)->enrtartxrreg = RTX_TXREQ_NPQ;
	}									// Start output if it's not active
	else
	{
		// Here if not using dscriptors

		if (length < 60)				// Is the packet too short?
		{
			sysLibMemSet(((char *)npb) + sizeof(NPB) + length, 0, length - 60);
			length = 60;				// Yes - this code counts on the fact
										//   that we never allocate a packet
										//   buffer of less than 128 bytes,
										//   which always has room for a 60
		}								//   byte packet

		((ENRTAIDB *)idb)->enrtaophybfr[oput] = 
				sysMemGetPhyAddr(((char *)npb) + sizeof(ENNPB));
										// Store physical address of the packet
										//   in the output buffer register
		((ENRTAIDB *)idb)->enrtaophysts[oput] = length | 0x0300000;
	}
	if (++(((ENRTAIDB *)idb)->enrtaoputter) >= ((ENRTAIDB *)idb)->enrtaorngend)
		((ENRTAIDB *)idb)->enrtaoputter = 0; // Advance the putter
	return (0);
}


//********************************************************
// Function: enrtafork - Device fork function for ENRTADEV
// Returned: Nothing
//********************************************************

void XOSFNC enrtafork(
	ENRTAIDB *idb)
{
	ENRTA  *regs;
	BPKT   *bpkt;
	XFRDSP *fdesc;
	XFRDSP *ldesc;
	ENNPB  *npb;
	char   *ppnt;
	long    intsts;
	long    pktsts;
	long    sts;
	long    code;
	int     findx;
	int     lindx;
	int     length;
	int     clen;
	int     amnt;
	int     taker;

	idb->cntint++;						// Count the interrupt

	regs = idb->enrtaregs;
	intsts = idb->enrtaintsts;
	if (intsts & (RTR_INT_RFO|RTR_INT_RBO|RTR_INT_SER))
	{
		// Here with a device error interrupt - this includes FIFO
		//   overflow, receive ring buffer overflow, and "system error".
		//   We count the errors and reset the interface.

		if (intsts & RTR_INT_SER)
			idb->cntsyserr++;
		if (intsts & RTR_INT_RFO)
			idb->cntnoibfr++;
		if (intsts & RTR_INT_RBO)
			idb->cntrover++;
		enrtareset(idb);
	}
	if (intsts & (RTR_INT_ROK|RTR_INT_RER))
	{
		// Here if have input available

		idb->enrtaintsts &= ~(RTR_INT_ROK|RTR_INT_RER);

		if (idb->enrtacattr & CA_DESCP)	// Using descriptors?
		{
			// Here with received packet if using descriptors - We first
			//   make sure we have a complete packet and find its end.

			while (fdesc->sts & ID_STS_OWN) // Is this buffer full?
			{
				findx = lindx = idb->enrtaitaker;
				fdesc = ldesc = &idb->enrtaiphyrng[findx];

				while (TRUE)			// Fake loop
				{
					if ((fdesc->sts & ID_STS_FS) == 0) // Start of a packet?
					{
						idb->cntbadpnt++;
						break;
					}
					if ((ldesc->sts & ID_STS_LS) == 0) // End of the packet?
					{								// No - try next one
						if (++lindx >= idb->enrtairngend)
							lindx = 0;
						ldesc = &idb->enrtaiphyrng[lindx];
						if ((ldesc->sts & (ID_STS_OWN & ID_STS_LS)) == 0)
						{
							idb->cntbadpnt++;
							break;
						}
					}

					// When get here, findx is the taker value for the first
					//   buffer for the packet and fdescp points to the buffer's
					//   descriptor. lindx is the taker value for the last
					//   buffer for the packet and ldescp points to the buffer's
					//   descriptor.

					if (fdesc->sts & (ID_STS_RES|ID_STS_RUNT|ID_STS_CRC))
					{
						// Here if there is an error on the packet

						if (fdesc->sts & ID_STS_CRC)
							idb->cntcrc++;
						if (fdesc->sts & ID_STS_RUNT)
							idb->cntrunt++;
						break;
					}
					if ((length = fdesc->sts & ID_STS_LEN) > 1600)
					{
						idb->cntbadpnt++;
						break;
					}

					// Here with a good input packet

					length -= 4;			// Discard the CRC bytes
					if (length > 1024)		// Does it fit in one buffer?
					{
						if (findx == lindx) // No - do we have two buffers?
						{
							idb->cntbadpnt++;
							break;
						}
					}
					if (xosnetGetBuffer((IDB *)idb, NULL, length,
							(NPB **)&npb) < 0) // Get a buffer for the packet
					{
						idb->cntnosbfr++;
						break;
					}
					ppnt = ((char *)npb) + sizeof(ENNPB);
					clen = (length + 3) >> 2;
					if ((amnt = clen) > 256)
						amnt = 256;
					sysLibMemCpyLong((long *)ppnt, // Copy the first buffer
							(long *)idb->enrtaivirrng[findx], amnt);
					if ((clen -= amnt) > 0)		// Do we have a second buffer?
						sysLibMemCpyLong((long *)ppnt + 1024, // Yes
								(long *)idb->enrtaivirrng[lindx], clen);
					npb->count = length;
					npb->npofs = sizeof(PKT);
					xosnetRecvPkt((IDB *)idb, convnetword(npb->ethertype),
							(NPB *)npb);
					break;				// Exit from the fake loop
				}

				// Here when finished with a packet - advance the ring

				fdesc->sts = (fdesc->sts & ID_STS_EOR) | 1024;
										// Free up the first buffer (Must
										//   preserve the EOR bit.)
				if (lindx != findx)		// Did we use 2 descriptors?
										// Yes - free up the second one
					ldesc->sts = (ldesc->sts & ID_STS_EOR) | 1024;
				if (++lindx >= idb->enrtairngend) // Advance to next descriptor
					lindx = 0;
				idb->enrtaitaker = lindx;
			}							// End of loop processing input
		}
		else
		{
			// Here with input if not using descriptors

			while (idb->enrtaitaker != regs->F_rbput) // Is there anything in
			{										  //   the buffer?
				while (TRUE)			// Fake loop to allow escape
				{
					bpkt = (BPKT *)(idb->enrtaiphybfr + idb->enrtaitaker);
					if ((length = bpkt->len - 4) < 60)
					{
						idb->cntrunt++;
						break;
					}

					// Get the status bits in a non-volatile item so the
					//   compiler does not keep reloading it!

					pktsts = bpkt->sts;
					if ((bpkt->sts & RTR_RSR_ROK) == 0) // Good packet?
					{
						if (pktsts & RTR_RSR_ISE|RTR_RSR_CRC)
							idb->cntcrc++;
						else if (pktsts & RTR_RSR_RUNT)
							idb->cntrunt++;
						else if (pktsts & RTR_RSR_LONG)
							idb->cnttoobig++;
						else if (pktsts & RTR_RSR_FAE)
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
					idb->cntpktin++;	// OK - count the received packet
					idb->cntbytein += length;

					if (pktsts & (RTR_RSR_MAR|RTR_RSR_BAR))
					{					// Broadcast or multicast?
						if (pktsts & RTR_RSR_MAR) // Yes - Multicast?
							idb->cntmcpktin++; // Yes
						else
							idb->cntbcpktin++; // No - must be broadcast
					}
					if (xosnetGetBuffer((IDB *)idb, NULL, length,
							(NPB **)&npb) < 0) // Get a buffer for the packet
					{
						idb->cntnosbfr++;
						break;
					}
					npb->count = length; 	// Store length in the packet
					npb->npofs = sizeof(ENPKT);
					ppnt = ((char *)npb) + sizeof(NPB);
					amnt = idb->enrtairngend - (((char *)&bpkt->pkt) -
							idb->enrtaiphybfr); // Get number of bytes to end
												//   of the ring buffer
					if (length > amnt)		// Did it wrap?
					{						// Yes - copy the first part
						sysLibMemCpyLong((long *)ppnt, (long *)&bpkt->pkt,
								amnt >> 2);
						length -= amnt;
						ppnt += amnt;
					}
					sysLibMemCpyLong((long *)ppnt, (long *)&bpkt->pkt,
							length >> 2);	// Copy the rest
					xosnetRecvPkt((IDB *)idb, convnetword(npb->ethertype),
							(NPB *)npb);
					break;				// Exit from the fake loop
				}						// End of fake loop

				// Advance the pointers

				length = (length + 11) & 0xFFFFFFFC; // Allow for the header and
													 //   CRC bytes, round up
				if ((length += idb->enrtaitaker) >= idb->enrtairngend)
					length -= idb->enrtairngend; // Advance the pointer
				idb->enrtaitaker = length;
				if ((length -= 0x10) < 0)
					length += idb->enrtairngend;
				regs->F_rbtake = length; // Set the hardware taker pointer
			}							// End of loop for processing received
										//   packets without descriptors
		}
	}
	if (intsts & (RTR_INT_ULC|RTR_INT_TOK|RTR_INT_TER))
	{
		// Here if have output complete

		idb->enrtaintsts &= ~(RTR_INT_ULC|RTR_INT_TOK|RTR_INT_TER);
		if (idb->enrtacattr & CA_DESCP)	// Using descriptors?
		{
			// Here for transmit done if using descriptors

			while (idb->enrtaocnt != 0)
			{
				taker = idb->enrtaotaker;
				sts = idb->enrtaophyrng[taker].sts;
				if (sts & OD_STS_OWN)	// Has the buffer been output?
					break;				// No - false alarm!
				idb->enrtaocnt--;		// Yes
				idb->enrtaophyrng[taker].sts = sts & OD_STS_EOR;
										// Free up the descriptor (must
										//   preserve the EOR bit)
				npb = idb->enrtaovirrng[taker];
										// Get address of the packet that was
										//   just output
				*(long *)&idb->enrtaovirrng[taker] &= 0x80000000;
										// For debugging only!
				if (npb == NULL)		// Make sure we have a packet
					CRASH('NPKT');
				if (++taker > idb->enrtaorngend) // Advance the taker
					taker = 0;
				idb->enrtaotaker = taker;
				idb->cntnumcol += ((sts >> 16) & 0x0F);
				if (sts & (OD_STS_FUNR|OD_STS_TES|OD_STS_OWC|OD_STS_LNKF|
						OD_STS_EXC))
				{
					if (sts & OD_STS_FUNR)
						idb->cntxunder++;
					if (sts & OD_STS_OWC)
						idb->cntowcol++;
///					if (sts & OD_STS_LNKF)
///
					if (sts & OD_STS_EXC)
						idb->cntxcol++;
					code = ER_DEVER;
				}
				else
					code = 0;
				xosnetFinXmit(code, (IDB *)idb, (NPB *)npb); // Process the pkt
			}
		}
		else
		{
			// Here for transmit done if not using descriptors

			while (idb->enrtaocnt != 0)
			{
				taker = idb->enrtaotaker;
				sts = idb->enrtaophysts[taker];

				if ((sts & RTF_TSD_OWN) == 0) // Has the buffer been output?
					break;				// No - false alarm!
				idb->enrtaocnt--;		// Yes
				idb->enrtaophyrng[taker].sts = sts & OD_STS_EOR;

				npb = idb->enrtaovirrng[taker];
										// Get address of the packet that was
										//   just output
				*(long *)&idb->enrtaovirrng[taker] &= 0x80000000;
										// For debugging only!
				if (npb == NULL)		// Make sure we have a packet
					CRASH('NPKT');
				if (++taker > idb->enrtaorngend) // Advance the taker
					taker = 0;
				idb->enrtaotaker = taker;

				idb->cntnumcol += ((sts >> 24) & 0x0F);

				if ((sts & RTF_TSD_TOK) == 0)
				{
					if (sts & RTF_TSD_TUN)
						idb->cntxunder++;
					if (sts & RTF_TSD_OWC)
						idb->cntowcol++;
					if (sts & RTF_TSD_TABT)
						idb->cntjabber++;
					if (sts & RTF_TSD_CDH)
						idb->cnthrtbt++;
					if (sts & RTF_TSD_CRS)
						idb->cntcsense++;
					if ((sts & (RTF_TSD_TUN|RTF_TSD_OWC|RTF_TSD_TABT|
							RTF_TSD_CDH|RTF_TSD_CRS)) == 0)
						idb->cntxcol++;
					code = ER_DEVER;
				}
				else
					code = 0;
				xosnetFinXmit(code, (IDB *)idb, (NPB *)npb); // Process the pkt
			}
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
	regs->R_intmask = RTR_INT_SER|RTR_INT_RFO|RTR_INT_ULC|RTR_INT_RBO|
				RTR_INT_TER|RTR_INT_TOK|RTR_INT_RER|RTR_INT_ROK;
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
