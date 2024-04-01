//--------------------------------------------------------------------------*
// SHOWPCI.C
// Command to display PCI device configuration
//
// Written by: John R. Goltz
//
//--------------------------------------------------------------------------*

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

// Command format:
//   SHOWPCI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xostime.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrtn.h>
#include <xoserr.h>
#include <progarg.h>
#include <proghelp.h>
#include <xoserrmsg.h>
#include <xosstr.h>
#include <xcmalloc.h>
#include "show.h"


typedef _Packed struct
{	uchar type;
	uchar length;
	char  name[8];
	union
	{	struct
		{	long lownum;
			long hinum;
		};
		struct
		{	char *pnt;
			short size;
			short amount;
		};
        time_s dtv;
        char   ss[8];
    };
} DCHAR;


typedef struct
{	ulong  addr;
	ushort venid;
	ushort devid;
	ulong  class;
	ulong  regC;
	uchar  irq;
	uchar  intline;
	ushort xxx;
	ulong  addr0;
	ulong  size0;
	ulong  addr1;
	ulong  size1;
	ulong  addr2;
	ulong  size2;
	ulong  addr3;
	ulong  size3;
	ulong  addr4;
	ulong  size4;
	ulong  addr5;
	ulong  size5;
} PCIVAL;

typedef union
{	char  *c;
	DCHAR *d;
} DPNT;

QAB charqab =
{	QFNC_WAIT|QFNC_CLASSFUNC,
	0,							// status  - Returned status
	0,							// error   - Error code
	0,							// amount  - Amount
	0,							// handle  - Device handle
	0,							// vector  - Vector for interrupt
	{0},
	CF_SIZE,					// option  - Options or command
	0,							// count   - Count
	"PCI:",						// buffer1 - Pointer to file spec
	NULL,						// buffer2 - Unused
	NULL						// parm    - Pointer to parameter area
};

extern int (*_sprintfpnt)(int, char **);

///static struct parmbfr *firstparm;
///static struct parmbfr *lastparm = (struct parmbfr *)(&firstparm);

int pciinfo(void)
{
	long    rtn;
	DPNT    cpnts;
	DPNT    cpntd;
	PCIVAL *vpnt;
	ulong  *apnt;
	char   *tpnt;
	int     ccval;
	int     scval;
	int     pival;
	int     rev;
	int     length;
	int     type;
	int     bus;
	int     dev;
	int     fnc;
	int     acnt;
	char    ccstr[16];
	char    scstr[16];
	char    pistr[16];
	char    mstr[6][32];
	char    intstr[8];

	++validcount;

	if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.error) < 0)



		femsg2(prgname, "Error getting PCI0: device characteristics", rtn,
				NULL);


	charqab.count = (ushort)charqab.amount;
	charqab.buffer2 = (char *)getspace(charqab.count + 16);
	charqab.option = CF_ALL;
	if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.error) < 0)
		femsg2(prgname, "Error getting PCI0: device characteristics", rtn,
				NULL);

	// Now we have a complete list of the device characteristics for the
	//   PCI0 device.  We now remove everything except for the DEVbbddn
	//   characteristics

	cpnts.c = cpntd.c = (char near *)charqab.buffer2;
	while (--charqab.amount >= 0)
	{
		type = cpnts.d->type & 0x0F;
		length = (type >= 14) ? sizeof(DCHAR) : (cpnts.d->length + 10);
		if (strncmp(cpnts.d->name, "DEV", 3) == 0)
		{
			memcpy(cpntd.c, cpnts.c, length);
			cpntd.d->type |= PAR_GET;
			cpntd.d->pnt = (char *)malloc(cpntd.d->size);
			cpntd.c += length;
		}
		cpnts.c += length;
	}
	*cpntd.c = 0;
	charqab.option = CF_VALUES;
	if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.error) < 0)
		femsg2(prgname, "Error getting PCI0: device characteristics", rtn,
				NULL);
	cpnts.c = (char near *)charqab.buffer2;
	fputs("  Bus Dev Fnc Ven  DvID Class      Sub-class   Iface Rv IR "
			"Address  Size\n", stdout);
	while (cpnts.d->type != 0)
	{
		vpnt = (PCIVAL *)(cpnts.d->pnt);
		bus = (vpnt->addr >> 16) & 0xFF;
		dev = (vpnt->addr >> 11) & 0x1F;
		fnc = (vpnt->addr >> 8) & 0x07;
		ccval = vpnt->class >> 24;
		scval = (uchar)(vpnt->class >> 16);
		pival = (uchar)(vpnt->class >> 8);
		rev = (uchar)vpnt->class;

		switch (ccval)
		{
		 case 0:						// Unspecified
			ccstr[0] = 0;
			switch (scval)
			{
			 case 0:
				scstr[0] = 0;
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 1:
				strcpy(scstr, "VGA");
				if (pival == 1)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
			}
			break;

		 case 1:						// Mass storage controller
			strcpy(ccstr, "Storage");
			switch (scval)
			{
			 case 0:					// SCSI
				strcpy(scstr, "SCSI");
				goto picom1;

			 case 1:					// IDE
				strcpy(scstr, "IDE");
				sprintf(pistr, "%02X", pival);
				break;

			 case 2:					// Floppy disk
				strcpy(scstr, "Floppy");
				goto picom1;

			 case 3:					// IPI
				strcpy(scstr, "IPI");
				goto picom1;

			 case 4:					// RAID
				strcpy(scstr, "RAID");
				goto picom1;

			 case 5:
				strcpy(scstr, "ATA");
				goto picom1;

			 case 6:
				strcpy(scstr, "AHCI");
			 picom1:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 2:						// Network controller
			strcpy(ccstr, "Network");
			switch (scval)
			{
			 case 0:					// Ethernet
				strcpy(scstr, "Ethernet");
				goto picom2;

			 case 1:					// Token ring
				strcpy(scstr, "TokenRing");
				goto picom2;

			 case 2:					// FDDI
				strcpy(scstr, "FDDI");
				goto picom2;

			 case 3:					// ATM
				strcpy(scstr, "ATM");
				goto picom2;

			 case 4:					// ISDN
				strcpy(scstr, "ISDN");
			 picom2:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 3:						// Display controller
			strcpy(ccstr, "Display");
			switch (scval)
			{
			 case 0:					// PC
				strcpy(scstr, "PC");

				if (pival == 0)
					strcpy(pistr, "VGA");
				else if (pival == 1)
					strcpy(pistr, "8514");
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 1:					// XGA
				strcpy(scstr, "XGA");
				goto picom3;

			 case 2:					// 3D
				strcpy(scstr, "3D");
			 picom3:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 4:						// Multimedia device
			strcpy(ccstr, "Multimedia");
			switch (scval)
			{
			 case 0:					// Video
				strcpy(scstr, "Video");
				goto picom4;

			 case 1:					// Audio
				strcpy(scstr, "Audio");
				goto picom4;

			 case 2:					// Telephony
				strcpy(scstr, "Telephony");
			 picom4:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 3:					// Audio
				strcpy(scstr, "HDAudio");
				goto picom4;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 5:						// Memory controller
			strcpy(ccstr, "Memory");
			switch (scval)
			{
			 case 0:					// RAM
				strcpy(scstr, "RAM");
				goto picom5;

			 case 1:					// Flash
				strcpy(scstr, "Flash");
			 picom5:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 6:						// Bridge device
			strcpy(ccstr, "Bridge");
			switch (scval)
			{
			 case 0:					// Host/PCI
				strcpy(scstr, "Host/PCI");
				goto picom6;

			 case 1:					// PCI/ISA
				strcpy(scstr, "PCI/ISA");
				goto picom6;

			 case 2:					// PCI/EISA
				strcpy(scstr, "PCI/EISA");
				goto picom6;

			 case 3:					// PCI/MicroChannel
				strcpy(scstr, "PCI/MicroCh");
				goto picom6;

			 case 4:					// PCI/PCI
				strcpy(scstr, "PCI/PCI");
				if (pival == 0)
					pistr[0] = 0;
				else if (pival == 1)
					strcpy(pistr, "SbDcd");
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 5:					// PCI/PCMCIA
				strcpy(scstr, "PCI/PCMCIA");
				goto picom6;

			 case 6:					// PCI/NuBus
				strcpy(scstr, "PCI/NuBus");
				goto picom6;

			 case 7:					// PCI/CardBus
				strcpy(scstr, "PCI/CardBus");
			 picom6:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 8:					// RACEway
				strcpy(scstr, "RACEway");
				sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 7:						// Simple connumications controller
			strcpy(ccstr, "Comm");
			switch (scval)
			{
			 case 0:					// PC com ports
				strcpy(scstr, "PCcom");
				switch (pival)
				{
				 case 0:				// Generic XT compatible
					strcpy(pistr, "Genrc");
					break;

				 case 1:				// 16450
					strcpy(pistr, "16450");
					break;

				 case 2:				// 16550
					strcpy(pistr, "16550");
					break;

				 case 3:				// 16650
					strcpy(pistr, "16650");
					break;

				 case 4:				// 16750
					strcpy(pistr, "16750");
					break;

				 case 5:				// 16850
					strcpy(pistr, "16850");
					break;

				 case 6:				// 16950
					strcpy(pistr, "16950");
					break;

				 default:
					sprintf(pistr, "%02X", pival);
					break;
				}
				break;

			 case 1:					// Parallel port
				strcpy(scstr, "Parallel");
				switch (pival)
				{
				 case 0:				// Generic PC parallel port
					strcpy(pistr, "Genrc");
					break;

				 case 1:				// Bi-directional
					strcpy(pistr, "BiDir");
					break;

				 case 2:				// ECP 1.X
					strcpy(pistr, "ECP 1");
					break;

				 case 3:				// IEEE 1284 controller
					strcpy(pistr, "1284c");
					break;

				 case 0xFE:				// IEEE 1284 target
					strcpy(pistr, "1284t");
					break;

				 default:
					sprintf(pistr, "%02X", pival);
					break;
				}
				break;

			 case 2:					// Multiport serial controller
				strcpy(scstr, "Multiport");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 3:					// Modem
				strcpy(scstr, "Modem");
				switch (pival)
				{
				 case 0:				// Generic modem
					strcpy(pistr, "Genrc");
					break;

				 case 1:				// Hayes/16450
					strcpy(pistr, "H-450");
					break;

				 case 2:				// Hayes/16550
					strcpy(pistr, "H-550");
					break;

				 case 3:				// Hayes/16650
					strcpy(pistr, "H-650");
					break;

				 case 0xFE:				// Hayes/16750
					strcpy(pistr, "H-750");
					break;

				 default:
					sprintf(pistr, "%02X", pival);
					break;
				}
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 8:						// Base system peripherals
			strcpy(ccstr, "System");
			switch (scval)
			{
			 case 0:					// Interrupt controller
				strcpy(scstr, "PIC");
				switch (pival)
				{
				 case 0:				// Generic
					strcpy(pistr, "Genrc");
					break;

				 case 1:				// ISA
					strcpy(pistr, "ISA");
					break;

				 case 2:				// EISA
					strcpy(pistr, "EISA");
					break;

				 case 10:				// IO APIC
					strcpy(pistr, "APIC");
					break;

				 case 20:				// IO(x) APIC
					strcpy(pistr, "xAPIC");
					break;

				 default:
					sprintf(pistr, "%02X", pival);
					break;
				}
				break;

			 case 1:					// DMA controller
				strcpy(scstr, "DMA");
				switch (pival)
				{
				 case 0:				// Generic DMA controller
					strcpy(pistr, "Genrc");
					break;

				 case 1:				// ISA
					strcpy(pistr, "ISA");
					break;

				 case 2:				// EISA
					strcpy(pistr, "EISA");
					break;

				 default:
					sprintf(pistr, "%02X", pival);
					break;
				}
				break;

			 case 2:					// 8237 timer
				strcpy(scstr, "Timer");
				switch (pival)
				{
				 case 0:				// Generic timer
					strcpy(pistr, "Genrc");
					break;

				 case 1:				// ISA
					strcpy(pistr, "ISA");
					break;

				 case 2:				// EISA
					strcpy(pistr, "EISA");
					break;

				 default:
					sprintf(pistr, "%02X", pival);
					break;
				}
				break;

			 case 3:					// RTC
				strcpy(scstr, "RTC");
				switch (pival)
				{
				 case 0:				// Generic RTC
					strcpy(pistr, "Genrc");
					break;

				 case 1:				// ISA
					strcpy(pistr, "ISA");
					break;

				 default:
					sprintf(pistr, "%02X", pival);
					break;
				}
				break;

			 case 4:					// PCI Hot-Plug controller
				strcpy(scstr, "HotPlug");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 9:						// Input device
			strcpy(ccstr, "Input");
			switch (scval)
			{
			 case 0:					// Keyboard
				strcpy(scstr, "Keyboard");
				goto picom9;

			 case 1:					// Digitizer
				strcpy(scstr, "Digitizer");
				goto picom9;

			 case 2:					// Mouse
				strcpy(scstr, "Mouse");
				goto picom9;

			 case 3:					// Scanner
				strcpy(scstr, "Scanner");
			 picom9:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 4:					// Gameport
				strcpy(scstr, "GamePort");
				if (pival == 0)
					pistr[0] = 0;
				else if (pival == 16)
					strcpy(pistr, "Legcy");
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 10:						// Docking station
			strcpy(ccstr, "Docking");
			switch (scval)
			{
			 case 0:					// Generic
				strcpy(scstr, "Generic");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 4:					// Gameport
				strcpy(scstr, "GamePort");
				if (pival == 0)
					pistr[0] = 0;
				else if (pival == 16)
					strcpy(pistr, "Legcy");
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 11:						// Processor
			strcpy(ccstr, "Processor");
			switch (scval)
			{
			 case 0:					// 80386
				strcpy(scstr, "80386");
				goto picom11;

			 case 1:					// 80486
				strcpy(scstr, "80486");
				goto picom11;

			 case 2:					// Pentinum
				strcpy(scstr, "Pentium");
				goto picom11;

			 case 16:					// Alpha
				strcpy(scstr, "Alpha");
				goto picom11;

			 case 32:					// Power PC
				strcpy(scstr, "Power PC");
				goto picom11;

			 case 48:					// MIPS
				strcpy(scstr, "MIPS");
			 picom11:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 12:						// Serial bus controller
			strcpy(ccstr, "SerialBus");

			switch (scval)
			{
			 case 0:					// FireWire (IEEE 1394)
				strcpy(scstr, "FireWire");
				switch (pival)
				{
				 case 0:				// Not specified
					pistr[0] = 0;
					break;

				 case 32:				// 1394 OpenHCI spec
					strcpy(pistr, "OHCI");
					break;

				 default:
					sprintf(pistr, "%02X", pival);
					break;
				}
				break;

			 case 1:					// ACCESS bus
				strcpy(scstr, "ACCESS");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 2:					// SSA (Serial Storage Architecture)
				strcpy(scstr, "SSA");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 3:					// USB
				strcpy(scstr, "USB");
				switch (pival)
				{
				 case 0x00:				// UHC spec
					strcpy(pistr, "UHC");
					break;

				 case 0x10:				// OHC spec
					strcpy(pistr, "OHC");
					break;

				 case 0x20:				// EHC spec
					strcpy(pistr, "EHC");
					break;

				 case 0x30:				// XHC spec
					strcpy(pistr, "XHC");
					break;

				 case 128:
					pistr[0] = 0;
					break;

				 default:
					sprintf(pistr, "%02X", pival);
					break;
				}
				break;

			 case 4:					// Fibre channel
				strcpy(scstr, "FIBRE");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 case 5:					// SMBus (System Management Bus)
				strcpy(scstr, "SMBus");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 13:						// Wireless controller
			strcpy(ccstr, "Wireless");
			switch (scval)
			{
			 case 0:					// iRDA compatible
				strcpy(scstr, "iRDA");
				goto picom13;

			 case 1:					// Consumer IR controller
				strcpy(scstr, "ConsumerIR");
				goto picom13;

			 case 16:					// RF controller
				strcpy(scstr, "RF");
			 picom13:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 14:						// Intelligent IO controller
			strcpy(ccstr, "Intelligent");
			switch (scval)
			{
			 case 0:					// I2O
				strcpy(scstr, "I2O");

			 default:
				sprintf(scstr, "%d", scval);
				break;
			}
			sprintf(pistr, "%02X", pival);
			break;

		 case 15:						// Satellite communications controller
			strcpy(ccstr, "Satellte");
			switch (scval)
			{
			 case 1:					// TV
				strcpy(scstr, "TV");
				goto picom15;

			 case 2:					// Audio
				strcpy(scstr, "Audio");
				goto picom15;

			 case 3:					// Voice
				strcpy(scstr, "Voice");
				goto picom15;

			 case 4:					// Data
				strcpy(scstr, "Data");
				goto picom15;
			 picom15:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 16:						// Encryption/decryption controller
			strcpy(ccstr, "Encryption");
			switch (scval)
			{
			 case 0:					// Data
				strcpy(scstr, "Data");
				goto picom16;

			 case 1:					// Entertainment
				strcpy(scstr, "Entrtnmnt");
			 picom16:
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 case 17:						// Data Acquisition and singal
										//   processing controller
			strcpy(ccstr, "DataAcq");
			switch (scval)
			{
			 case 0:					// DPIO
				strcpy(scstr, "DPIO");
				if (pival == 0)
					pistr[0] = 0;
				else
					sprintf(pistr, "%02X", pival);
				break;

			 default:
				sprintf(scstr, "%d", scval);
				sprintf(pistr, "%02X", pival);
				break;
			}
			break;

		 default:
			sprintf(ccstr, "%d", ccval);
			sprintf(scstr, "%d", scval);
			sprintf(pistr, "%02X", pival);
			break;
		}
		apnt = &vpnt->addr0;
		acnt = 0;
		tpnt = mstr[0];
		mstr[0][0] = 0;
		mstr[1][0] = 0;
		mstr[2][0] = 0;
		mstr[3][0] = 0;
		mstr[4][0] = 0;
		mstr[5][0] = 0;
		do
		{
			if (*apnt != 0)
			{
				if (*apnt & 0x01)
					sprintf(tpnt, " IO: %04X %,d", apnt[0] & 0xFFFE, apnt[1]);
				else
					sprintf(tpnt, " %08X %,d", apnt[0], apnt[1]);
				tpnt += 32;
			}
			apnt += 2;
		} while (++acnt < 5);
		if (vpnt->irq == 0)
			intstr[0] = 0;
		else if (vpnt->irq == 0xFF)
			strcpy(intstr, "?");
		else
			sprintf(intstr, "%d", vpnt->irq);

		printf("  %-3d %-3d %-3d %04X %04X %-10s %-11s %-5s %02X %-2s%s\n",
				bus, dev, fnc, vpnt->venid, vpnt->devid, ccstr, scstr, pistr,
				rev, intstr, mstr[0]);

		tpnt = mstr[1];
		acnt = 5;
		while (--acnt >= 0 && *tpnt != 0)
		{
			printf("%58s%s\n", "", tpnt);
			tpnt += 32;
		}
		cpnts.c += sizeof(DCHAR);
	}
	return (1);
}
