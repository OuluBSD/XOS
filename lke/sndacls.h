
typedef struct scb__ SCB;
typedef struct sdb__ SDB;

// Define structure for each stream descriptor register block

typedef volatile struct
{	union
	{ long ctrl;			// 0x00 Stream descriptor control (3 bytes!)
	  struct
	  {	char fill[3];
		char sts;			// 0x03 Stream descriptor status (Bits are RO or
	  };					//        RW1C so it's OK to write 0s when writing
	};						//        to ctrl as a long!)
	long   pib;				// 0x04	SD link position in buffer
	long   cbl;				// 0x08	SD cyclic buffer length
	ushort lvi;				// 0x0C	SD last valid index
	char   fill1[2];
	ushort fifosize;		// 0x10	SD FIFO size
	ushort fmt;				// 0x12	SD format
	char   fill2[4];
	llong  bdlp;			// 0x18	SD buffer descriptor list pointer
} SDREG;

// Define structure for the memory mapped register block

typedef volatile struct
{	struct
	{ ushort a64   : 1;
	  ushort numdos: 2;
	  ushort numbs : 5;
	  ushort numis : 4;
	  ushort numos : 4;
	}      gcap;			// 0x0000 Global capabilities
	char   vmin;			// 0x0002 Minor version
	char   vmaj;			// 0x0003 Major version
	ushort outpay;			// 0x0004 Output payload capability
	ushort inpay;			// 0x0006 Input payload capability
	long   gctl;			// 0x0008 Global control
	ushort codecinten;		// 0x000C Codec status change interrupt enable
	ushort codecsts;		// 0x000E Codec status change
	ushort gsts;			// 0x0010 Global status
	char   fill1[6];
	ushort outstrmpay;		// 0x0018 Output stream payload capability
	ushort instrmpay;		// 0x001A Input stream payload capability
	char   fill2[4];
	long   intctl;			// 0x0020 Interrupt control
	long   intsts;			// 0x0024 Interrupt status
	char   fill3[8];
	long   wallclk;			// 0x0030 Wall clock counter
	char   fill4[4];
	long   ssync;			// 0x0038 Stream synchronication
	char   fill5[4];
	long   corblbase;		// 0x0040 CORB lower base address
	long   corbubase;		// 0x0044 CORB upper base address
	ushort corbwp;			// 0x0048 CORB write pointer
	ushort corbrp;			// 0x004A CORB read pointer
	char   corbctl;			// 0x004C CORB control
	char   corbsts;			// 0x004D CORB status
	char   corbsize;  		// 0x004E CORB size
	char   fill6[1];
	long   rirblbase;		// 0x0050 RIRB lower base address
	long   rirbubase;		// 0x0054 RIRB upper base address
	ushort rirbwp;			// 0x0058 RIRB write pointer
	ushort rirbintcnt;		// 0x005A RIRB interrupt count
	char   rirbctl;			// 0x005C RIRB control
	char   rirbsts;			// 0x005D RIRB status
	char   rirbsize;		// 0x005E RIRB size
	char   fill7[1];
	long   icoi;			// 0x0060 Immediate command output interface
	long   icii;			// 0x0064 Immediate command input interface
	ushort icis;			// 0x0068 Immediate command status
	char   fill8[6];
	long   dpiblbase;		// 0x0070 DMA position buffer lower base
	long   dpibubase;		// 0x0074 DMA position buffer upper base
	char   fill9[8];
	SDREG  sdreg[1];		// 0x0080 Start of the SD array
	char   fill10[8080];
	long   wallclka;		// 0x2030 Wall clock alias
	char   fill11[80];
	long   sd0lpiba[1];		// 0x2084 Start of link position in current buffer
} REG;						//        array

// Define structure for a buffer descripter

typedef volatile struct
{	llong  paddr;			// Physical address
	long   length;			// Length
	long   flags;
} BD;

// Define structure for the stream format value (This value is used to
//   set the format for the DMA stream and for the converter (DAC and ADC)
//   codec.

typedef struct
{	ushort chnls: 4;		// Number of channels
	ushort bps  : 3;		// Bits per sample
	ushort      : 1;
	ushort rfact: 7;		// Sample base rate multiple/divide factors
	ushort type : 1;		// Stream type (0 = PCM)
} SFS;

// Define offsets in our DCB

typedef struct
{	DCB;
	SCB  *sndascb;			// Address of the SCB
	SDB  *sndasdb;			// Address of the SDB
	char *sndabuffer;		// Address of the user's buffer
	long  sndaseglen;		// Length of one buffer segment
	MDB  *sndaregmdb;		// Address of MDB for msect which maps registers
	long  sndavect;			// Signal vector
	long  sndadata;			// Signal data
	long  sndapid;			// Signal PID
	MLTBL sndalocktbl; 		// Buffer lock data
	long  sndawidth;		// Sample width (bits)
	long  sndachnls;		// Number of channels
	long  sndarate;			// Sample rate (samples per second)
	ulong sndaavolume;		// Active volume
	ulong sndacvolume;		// Current volume
	char  sndasts;			// Status bits
	char  sndabdllen;		// Length of the buffer descriptor list
	char  sndawidthb;		// Actual sample width in bytes
	char  fill1[1];
	union
	{ long l;
	  SFS;
	}     sndafmt;			// Stream format value
	BD   *sndabdl;			// Address of the buffer descriptor list
} SNDADCB;

// Define values for dcb_sndasts

#define SSTS_STOPPING 0x80	// Stream is being stopped
#define SSTS_ACTIVE   0x40	// Stream is active

// Define structure for path block items

typedef struct
{	char   node;
	char   dev : 4;
	char   inx : 4;
	char   type: 4;
	char   pos : 4;
	char   end : 1;
	char   voli: 1;
	char   volo: 1;
	char   eapd: 1;
	char   colr: 4;
} WDB;

// Define structure for volume control information

typedef struct
{	char agno;				// Amplifier gain offset (+ 0x80 if input amp)
	char agnn;				// Amplifier gain number of steps
	char agns;				// Amplifier gain step size
	char fill[1];
	union
	{ ushort val;
	  struct
	  {	ushort num: 12;		// Amplifier node number
		ushort inx: 4;		// Amplifier node input index
	  };
	}    anode;
} VC;

typedef volatile struct
{	long  resp;
	struct
	{ ulong codec: 4;
	  ulong unsol: 1;
	};
} RIRB;

// Define structure for the stream data blocks (SDB)

typedef struct sdb__
{	SNDADCB *dcb;			// Address of DCB using this stream
	long     intbit;		// Interrupt bit for this stream
	long     strminx;		// Stream index
	SDREG   *vsdreg;		// Address of this stream's register block
	long     supported;		// Supported converter sample sizes and rates
	long     maxrate;		// Maximum supported rate
	long     cnode;			// Converter (DAC or ADC) node address (includes
							//   codec address)
	long     stream;		// Stream number (in bits 7:4)
	VC       volume[8];		// Volume control nodes for this stream
};

// Define offsets in our SCB (Sound Controller data Block)

struct scb__
{	long     label;			// Label (SCB*)
							// WARNING: Following 2 items MUST match the
							//          corresponding items in a DCB
	DFRKFNC *fdisp;			// Address of fork routine
	DCB     *fnext;			// Address of next block in fork request list
	SCB     *next;			// Address of next SCB (always 0 for now)
	char     name[16];		// Device name
	long     srate;			// Supported sample rates
	long     swidth;		// Supported sample widths
	long     codecven;		// Codec vendor parameter
	long     codecver;		// Codec version parameter
	long     codec;			// Codec address
	long     preg;			// Physical address of the register block
	REG     *vreg;			// Virtual address of the register block
	long     intnum;		// Interrupt request number
	long     config;
	ushort   codecmsk;		// Codec available bits
	char     fill1[2];
	long     corbsize;		// Size of the CORB buffer
	long     rirbsize;		// Size of the RIRB buffer
	long     rirbrp;		// RIRB read pointer
	long     rirbrsp;		// Response (set by codeccmd)
	long    *vcorb;			// Virtual address of the CORB
	long     pcorb;			// Physical address of the CORB
	RIRB    *vrirb;			// Virtual address of the RIRB
	long     prirb;			// Physical address of the RIRB
	WDB     *wdgdata;
	long     wdgdsz;
	VC       boost[8];
	long     intmask;		// Bit mask for all interrupts
	long     dmainmsk;		// Bit mask for valid input DMA streams
	long     dmaoutmsk;		// Bit mask for valid output DMA streams
	TDA     *rrspwtid;		// Address of TDA for thread which is waiting for
	SDB      outsdb;		// Output SDB
	SDB      inpsdb;		// Input SDB
};

// Define connection list sizes for each widget type

#define PINSMAX  16
#define MIXERMAX 16
#define SELMAX   16
#define DACMAX   8
#define ADCMAX   8

typedef union
{	long l;
	struct
	{ ulong stereo : 1;
	  ulong inamp  : 1;
	  ulong outamp : 1;
	  ulong ampor  : 1;
	  ulong fmtor  : 1;
	  ulong stripe : 1;
	  ulong proc   : 1;
	  ulong unsol  : 1;
	  ulong clist  : 1;
	  ulong digital: 1;
	  ulong powctl : 1;
	  ulong lrswap : 1;
	  ulong        : 4;
	  ulong delay  : 4;
	  ulong type   : 4;
	};
} WCAP;

// Define offsets in the widget data structure

typedef struct
{	long   label;
	ushort node;			// Node number
	char   type;			// Widget type
	char   cllen;			// Connect list length

	char   iagno;			// Input amplifier gain offset
	char   iagnn;			// Input amplifier gain number of steps
	char   iagns;			// Input amplifier gain step size
	char   fill1[1];

	char   oagno;			// Output amplifier gain offset
	char   oagnn;			// Output amplifier gain number of steps
	char   oagns;			// Output amplifier gain step size
	char   fill2[1];

	long   pincap;			// Pin capabilities

	char   source;			// TRUE if node is a data source
	char   fill3[3];

	ushort path;			// Input selector for path through this node
	char   volinp;			// Using input amplifier as volume control
	char   volout;			// Using output amplifier as volume control

	WCAP   wcap;

	union
	{ long supported;		// Supported PCM formats (DAC and ADC only)
	  union
	  {	long l;
		struct
		{ ulong seq  : 4;
		  ulong assoc: 4;
		  ulong misc : 4;
		  ulong color: 4;
		  ulong ctype: 4;
		  ulong dev  : 4;
		  ulong loc  : 6;
		  ulong epc  : 2;
		};
	  }    pcd;				// Pin configuration default (pins only)
	};
	ushort clist[];			// Connection list (always 16-bit entires, always
} WDG;						//   individual entries)

// Define structure for a ratetbl entry

typedef _Packed struct
{	long rate;				// Rate value
	long rbit;				// Rate bit
	char rfact;				// Value for the rfact in the format register
} RTVAL;

// Define codec node types

#define CWT_DAC   0x0		// Audio output (DAC)
#define CWT_ADC   0x1		// Audio input (ADC)
#define CWT_MIXER 0x2		// Audio mixer
#define CWT_SEL   0x3		// Audio selector
#define CWT_PIN   0x4		// Pin complex (jack)
#define CWT_POWER 0x5		// Power control
#define CWT_VOL   0x6		// Volume knob
#define CWT_BEEP  0x7		// Beep generator
#define CWT_VEN   0xF		// Vendor defined

// Define bits in the supported PCM size, rates codec parameter value

#define SPCM_32BIT 0x00100000 // 32-bit formats are supported
#define SPCM_24BIT 0x00080000 // 24-bit formats are supported
#define SPCM_20BIT 0x00040000 // 20-bit formats are supported
#define SPCM_16BIT 0x00020000 // 16-bit formats are supported
#define SPCM_8BIT  0x00010000 // 8-bit formats are supported
#define SPCM_384   0x00000800 // 384KB rate is supported
#define SPCM_192   0x00000400 // 192KB rate is supported
#define SPCM_176   0x00000200 // 176.4KB rate is supported
#define SPCM_96    0x00000100 // 96KB rate is supported
#define SPCM_88    0x00000080 // 88.2KB rate is supported
#define SPCM_48    0x00000040 // 48KB rate is supported
#define SPCM_44    0x00000020 // 44.1KB rate is supported
#define SPCM_32    0x00000010 // 32KB rate is supported
#define SPCM_22    0x00000008 // 22.05KB rate is supported
#define SPCM_16    0x00000004 // 16KB rate is supported
#define SPCM_11    0x00000002 // 11.025 rate is supported
#define SPCM_8     0x00000001 // 8KB rate is supported

// Define connected device types (pin configuration default)

#define PDEV_LO     0x0		// Line out
#define PDEV_SPKR   0x1		// Speaker
#define PDEV_HPO    0x2		// Headphone out
#define PDEV_CD     0x3		// CD
#define PDEV_SPDIFO 0x4		// SPDIF output
#define PDEV_DIGO   0x5		// Other digital output
#define PDEV_MDML   0x6		// Modem line
#define PDEV_MDMPE  0x7		// Modem phone
#define PDEV_LI     0x8		// Line in
#define PDEV_AUX    0x9		// Aux
#define PDEV_MIC    0xA		// Microphone in
#define PDEV_TPHNY  0xB		// Telephony
#define PDEV_SPDIFI 0xC		// SPDIF input
#define PDEV_DIGI   0xD		// Other digital input
#define PDEV_OTHER  0xF		// Other

// Define codec command verbs

#define CV_SETCF    0x20000	// Set converter format
#define CV_SETAGM   0x30000	// Set amplifier gain/mute
#define CV_SETPC    0x40000	// Set processing coefficient
#define CV_SETCX    0x50000	// Set coefficient index
#define CV_SETCS    0x70100	// Set connection select
#define CV_SETPS    0x70300	// Set processing state
#define CV_SETSDI   0x70400	// Set input converter SDI
#define CV_SETPWR   0x70500	// Set power state
#define CV_SETCSC   0x70600	// Set converter stream and channel
#define CV_SETPW    0x70700	// Set pin widget
#define CV_SETUNSOL 0x70800	// Set unsolicited response state
#define CV_SETPINS  0x70900	// Set pin state
#define CV_SETBEEP  0x70A00	// Set beep
#define CV_SETPINC  0x70C00	// Set pin configuration
#define CV_RESET    0x7FF00	// Reset
#define CV_GETCF    0xA0000	// Get converter format
#define CV_GETAGM   0xB0000	// Get amplifier gain/mute
#define CV_GETPC    0xC0000	// Get processing coefficient
#define CV_GETCX    0xD0000	// Get coefficient index
#define CV_GETPARAM 0xF0000	// Get parameter value
#define CV_GETCS    0xF0100	// Get connection select
#define CV_GETCLE   0xF0200	// Get connection list entry
#define CV_GETPS    0xF0300	// Get processing state
#define CV_GETSDI   0xF0400	// Get input converter SDI
#define CV_GETPWR   0xF0500	// Get power state
#define CV_GETSC    0xF0600	// Get stream and channel
#define CV_GETPW    0xF0700	// Get pin widgt
#define CV_GETUNSOL 0xF0800	// Get unsolicited response state
#define CV_GETPINS  0xF0900	// Get pin state
#define CV_GETPINC  0xF0C00	// Get pin configuration
#define CV_GETBEEP  0xF0A00	// Get beep
#define CV_GETPIND  0xF1C00	// Get pin defaults

// Define codec parameters

#define CP_VENID 0x00		// Vendor ID
#define CP_REVID 0x02		// Revision ID
#define CP_SNCNT 0x04		// Subordinate node count
#define CP_FGT   0x05		// Function group type
#define CP_AFGC  0x08		// Audio function group capabilities
#define CP_AWCAP 0x09		// Audio widget capabilities
#define CP_SPCM  0x0A		// Supported PCM sample size and rates
#define CP_SSF   0x0B		// Supported stream formats
#define CP_PINC  0x0C		// Widget pin capabilities
#define CP_IAMPC 0x0D		// Input ampilifier capabilities
#define CP_CLL   0x0E		// Connection list length
#define CP_SPS   0x0F		// Supported power states
#define CP_PROCC 0x10		// Processing capabilities
#define CP_GPIOC 0x11		// GPIO pin count
#define CP_OAMPC 0x12		// Output ampilifier capabilities
#define CP_VOLC  0x13		// Volume knob capabilities

// Define bits for the value of the CP_AWCAP parameter

#if 0
#define AWC_TYPE   0x00F00000 // Widget type
#define AWC_DELAY  0x000F0000 // Delay
#define AWC_LRSEAP 0x00000800 // Left-right swap present
#define AWC_POWCTL 0x00000400 // Power control present
#define AWC_DIG    0x00000200 // Digital widget
#define AWC_CONLST 0x00000100 // Has connect list
#define AWC_UNSOL  0x00000080 // Supports unsolicited responses
#define AWC_PROC   0x00000040 // Widget has processing capability
#define AWC_STRIP  0x00000020 // Supports stripping
#define AWC_FMTOR  0x00000010 // Format override
#define AWC_APOR   0x00000008 // Amplifier parameter override
#define AWC_OAP    0x00000004 // Output amplifier present
#define AWC_IAP    0x00000002 // Input amplifier present
#define AWC_STEREO 0x00000001 // Stereo widget
#endif


extern SCB  *sndaintfix1;
extern RTVAL ratetbl[];
extern char  sndaintfix2;
extern char  sndaint[];
extern char  sndaintexit[];

long XOSFNC codeccmd(SCB *scb, long addr, long func);
long XOSFNC getmaxrate(long supported);
long XOSFNC setupwidgets(SCB *scb);
