//--------------------------------------------------------------------------
// pcicls.xc
//
// Written by: John Goltz
//
// Edit History:
//
//--------------------------------------------------------------------------

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
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES//  LOSS
//   OF USE, DATA, OR PROFITS//  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <stddef.h>
#include <ctype.h>
#include <xos.h>
#include <xoslib.h>
#include <xoserr.h>
#include <xosx.h>
#include <xosxchar.h>
#include <xosxparm.h>
#include <xosxlke.h>
#include <xosxpci.h>

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0

static INITLKE initpci;

#pragma data_seg(_HEAD);

	LKEHEAD(PCICLS, initpci, MAJVER, MINVER, EDITNUM, CLASS); 

#pragma data_seg();

// This device class implements the interface to PCI bus controllers which
//   use configuratiaon mechanism #1. Mechanism #2 (which is obsolete, in
//   spite of the higher number) is NOT supported by this version.

// No devices are implimented. It's only purpose is to provide information
//   about and control of the PCI configuration via class characteristics and
//   a kernel mode call interface.

// Define IO ports - The type 1 and type 2 registers overlap. 32-bit accesses
//   access the type 1 registers, 8-bit accesses access the type 2 registers.

#define P_PCI1ADDR 0x0CF8	// Type 1 configuration address register (32-bit)
#define P_PCI1DATA 0x0CFC	// Type 1 configuration data register (32-bit)

static long pcicfgreg;

SINFO(msgdev   , "Device data");
SINFO(msgcfgreg, "Config reg value");

// Following text is provided for the use of various PCI devices for commonly
//   used values.

XINFO(xospciMsgVen  , "PCI vendor");
XINFO(xospciMsgDev  , "PCI device");
XINFO(xospciMsgRev  , "PCI revision");
XINFO(xospciMsgSSVen, "PCI sub-system vendor");
XINFO(xospciMsgSSDev, "PCI sub-system device");
XINFO(xospciMsgBus  , "PCI bus number");
XINFO(xospciMsgSlot , "PCI slot (device) number");
XINFO(xospciMsgFunc , "PCI function number");

// PCI bus interface (configuration type 1) class characteristics table

static CHARFNC getcfgreg;

static long XOSFNC setcfgreg(CHARITEM const *item, char *val, long cnt,
		void *data);
static long XOSFNC pcigetdev(CHARITEM const *item, char *val, long cnt,
		void *data);

static CHARTBL pcicctbl =
{ CTBEGIN(sysChrValuesNew),
 {CTLAST(CFGREG, HEXV, u, 4, &msgcfgreg, getcfgreg, setcfgreg, 0)
 }
};

static PCIB *lastitem = (PCIB *)&pcicctbl.items[0];

static CCB pciccb =
{	'CCB*',				// label   - 'CCB*'
	NULL,				// next    - Address of next CCB
	"PCI",				// name    - Name of this class
	0xFFFFFFFF,			// npfxmsk - Name prefix part mask
	'PCI0',				// npfxval - Name prefix part value
	NULL,				// dchk    - Address of device check routine
	NULL,				// fdsp    - Address of class func disp table
	&pcicctbl			// clschar - Address of class char table
};


typedef union
{	long l;
	struct
	{	long reg   : 8;
		long func  : 3;
		long slot  : 5;
		long bus   : 8;
		long       : 7;
		long enable: 1;
	};
} ADDR;

//============================
// Following is once-only code
//============================

static long XOSFNC readconfig(long addr);
static void XOSFNC writeconfig(long addr, long data);
static long XOSFNC chkdev(ADDR addr);

//************************************************************
// Function: initpci - Once-only initialization routine
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// The initialization routine does the best it can to find all PCI devices. It
//   never returns an error that would cause the LKE to be unloaded. This is
//   necessary since quite a few other LKEs call functions defined here and
//   would not load if this LKE is not present. It is assumed that this will
//   be one of the first LKEs loaded and that the basic routines for registering
//   the class and allocating memory should not fail. If one of these does fail,
//   the system is crashed.

// This routine does a complete scan of the PCI bus and creates a DEVbbddf
//   class characteristic for each device found.

#pragma code_seg ("x_ONCE");

static long XOSFNC initpci(
	char **pctop,
	char **pdtop)
{
	ADDR addr;
	long rtn;

	sysChrFixUp(&pcicctbl);

	if ((rtn = sysIoNewClass(&pciccb)) < 0)	// Create the PCI class
		CRASH('CNCC');					// [Can Not Create Class]

	// See if we have a type 1 PCI configuration interface. This is the only
	//   configuration interface supported by this version of XOS.

	OUTL(P_PCI1ADDR, 0x80000004);
	IOPAUSE;
	INL(P_PCI1DATA);
	IOPAUSE;

	if (INL(P_PCI1ADDR) == 0x80000004)
	{
		// Here when we have determined that we have a type 1 PCI configuration
		//   interface. Register the IO registers used to access this.

		if ((rtn = sysIoRegPorts(pciccb.name, P_PCI1ADDR, P_PCI1ADDR)) < 0)
			CRASH('CNRP');					// [Can Not Register Ports]

		// Scan all possible PCI devices and construct our database. We make the
		//   following assumtions about the PCI configuration:
		//   1) All devices implement function 0. If function 0 is not found,
		//        no additional function are checked.
		//   2) Multifunction devices set bit 7 in the header type register (0E)
		//        for function 0. If this bit is not set, no additional
		//		  functions are checked. If it is set, all possible (6)
		//		  additional functions are checked. (Additional functions
		//		  should be contiguous, but it's cheap to check them all!)

		addr.l = 0;
		do
		{
			if ((rtn = chkdev(addr)) < 0)
				break;
			addr.l += ((rtn == 2) ? 0x0100 : 0x0800);
		} while ((addr.l & 0xFF000000) == 0);
	}

	// Here with all PCI devices located or with error

	*pctop = codetop;
	return (0);
}


//****************************************************************
// Function: chkdev - Check for a PCI device and create a CHARITEM
//                    if a device is found
// Returned: 0 if there is no device at the address, 1 if a single
//           unit device was found, 2 if a multi-unit device was
//           found or a negative XOS  error code if error
//****************************************************************

// NOTE: This function is once-only

static long XOSFNC chkdev(
	ADDR addr)
{
	register PCIB *item;
	PCIB *item2;
	long *vpnt;
	long  devid;
	long  addrval;
	long  cmdval;
	long  rtn;
	long  cnt;
	long  bar;
	long  val;

	addr.l &= 0xFFFFFF00;
	if ((devid = readconfig(addr.l)) == 0xFFFFFFFF)
		return ((addr.l & 0x0700) ? 2 : 0);

	// Here if have a device at this address

	if ((rtn = sysMemGetXMem(sizeof(PCIB), (char **)&item2)) < 0)
		return (rtn);
	item = item2;
	item->label = 'CHRI';
	item->info = &msgdev;
	*(long *)item->name = 'DEV';

/// NOTE: OWXC 1.8.4 is treating ALL bit fields as signed!!

///	sysLibHex2Str(item->name + 3, addr.bus, 2);
///	sysLibHex2Str(item->name + 5, addr.slot, 2);
///	sysLibHex2Str(item->name + 7, addr.func, 1);

	sysLibHex2Str(item->name + 3, (addr.l >> 16) & 0xFF, 2);
	sysLibHex2Str(item->name + 5, (addr.l >> 11) & 0x1F, 2);
	sysLibHex2Str(item->name + 7, (addr.l >> 8) & 0x07, 1);

	item->rep = REP_DATAS;
	item->flags = 0;
	item->clength = sizeof(PCIB) - offsetof(PCIB, addr);
	item->getfunc = pcigetdev;
	item->setfunc = NULL;
	item->data = 0;
	item->addr = addr.l;
	item->devid = devid;
	item->clsitem = readconfig(addr.l + 0x08);

	item->regC = readconfig(addr.l + 0x0C);

	if ((item->regC & 0x7F0000) == 0) // Type 0 header?
	{
		item->inum = readconfig(addr.l + 0x3C);
		cnt = 6;
		bar = addr.l + 0x10;
		vpnt = &item->addr0;

		do
		{
			val = readconfig(bar);
			if (val & 0x01)
			{
				if ((val & 0xFFFE) == 0)
					val = 0;
			}
			else
			{
				if ((val & 0xFFFFFFF0) == 0)
					val = 0;
			}
			vpnt[0] = val;
			if (val == 0)
				vpnt[1] = 0;
			else
			{
				cmdval = readconfig(addr.l + 4); // Clear IO and memory
				CLI;							 //   enables
				writeconfig(addr.l + 4, cmdval & 0xFFFC);
				writeconfig(bar, 0xFFFFFFFF); // Write all 1s to the
											  //   address register!
				addrval = readconfig(bar); // Read it back
				writeconfig(bar, val);	// Restore the address reg value
				writeconfig(addr.l + 4, cmdval); // Restore the command reg
				STI;					// Interrupts are OK now

				// Here with the size value obtained and the address register
				//   restored. Now figure out what the value means.

				if (val & 0x01)			// IO address?
					vpnt[1] = ((~addrval | 0x0F) & 0xFFFF) + 1;
				else
				{
					vpnt[1] = (~addrval | 0x0F) + 1; // Store size
					if (val & 0x04)		// 64-bit address?
					{
						if (--cnt <= 0) // Yes - make sure not at end
							break;
						vpnt += 2;		// Advance to next BAR
						bar += 4;
						vpnt[0] = readconfig(bar); // Store the high parts
						vpnt[1] = 0xFFFFFFFF; // Set size to -1 as flag
					}
				}
			}
			vpnt += 2;
			bar += 4;
		} while (--cnt > 0);
	}

	lastitem->next = (CHARITEM *)item;	// Link in this item
	lastitem = item;

	return (((addr.l & 0x0700) || (item->regC & 0x800000)) ? 2 : 1);
}

//*********************************************************
// Function: readconfig - Read a PCI configuration register
// Returned: Value from register (no error indication)
//*********************************************************

static long XOSFNC readconfig(
	long addr)
{
	OUTL(P_PCI1ADDR, addr | 0x80000000);
	IOPAUSE;
	return (INL(P_PCI1DATA));
}

//***********************************************************
// Function: writeconfig - Write a PCI configuration register
// Returned: Nothing
//***********************************************************

static void XOSFNC writeconfig(
	long addr,	 		// PCI configuration address
	long data)			// Data to write
{
	OUTL(P_PCI1ADDR, addr | 0x80000000);
	IOPAUSE;
	OUTL(P_PCI1DATA, data);
}

#pragma code_seg ();					// End of once-only code


//====================================================
// Following are local class characteristics functions
//====================================================


//***********************************************************************
// Function: getcfgreg - Get the value of the CFGREG class characteristic
// Returned: 0 if normal or a negative XOS error code if error
//***********************************************************************

// This reads the value from the device. It does not use our data base.
//   Generally, this charateristic should only be used when investigating
//   unsupported devices.

static long XOSFNC getcfgreg(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	OUTL(P_PCI1ADDR, pcicfgreg | 0x80000000);
	IOPAUSE;
	return (sysUSPutULongV(val, INL(P_PCI1DATA), cnt));
}


//**********************************************************************
// Funcion: setcfgreg - Set the value of the CFGREG class characteristic
//                      when gettin the value of this characteristic
// Returned: 0 if normal or a negative XOS error code if error
//**********************************************************************

// This sets the configuration register address to read when getting the
//   value of this characteristic

static long XOSFNC setcfgreg(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSGetULongV(val, &pcicfgreg, cnt));
}


static long XOSFNC pcigetdev(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	if (cnt > (sizeof(PCIB) - offsetof(PCIB, addr)))
		cnt = sizeof(PCIB) - offsetof(PCIB, addr);
	return (sysLibMemCpy(val, (char *)&((PCIB *)item)->addr, cnt));
}


//======================================================================
// Following functions are exported for use by other parts of the kernel
//======================================================================


//*******************************************************************
// Function: xospciFindVendor - Find a PCI device given the device ID
// Returned: Value is the encoded configuration address if normal
//           (positive) or a negative XOS error code if error
//*******************************************************************

// On a normal return ppcib points to the PCIB found.

long XOSFNC __export xospciFindByVenDev(
	PCIB **ppcib,		// Address of pointer to receive address of PCIB found
	long   vendor,		// Vendor part of device ID
	long   device)		// Device part of device ID (-1 if don't care)
{
	PCIB *pcib;

	if ((pcib = *ppcib) == NULL)
		pcib = (PCIB *)pcicctbl.items[0].next;
	else
		pcib = (PCIB *)pcib->next;
	while (pcib != NULL)
	{
		if (pcib->ven == vendor && (device == -1 || pcib->dev == device))
		{
			*ppcib = pcib;
			return (pcib->addr);
		}
		pcib = (PCIB *)pcib->next;
	}
	return (ER_PDNAV);
}


//***********************************************************************
// Function xospciFindByClass - Find a PCI device data given class values
// Returned: Value is the encoded configuration address if normal
//           (positive) or a negative XOS error code if error
//***********************************************************************

long XOSFNC __export xospciFindByClass(
	PCIB **ppcib,		// Address of pointer to receive address of PCIB found
	long   cls,			// PCI device class
	long   subcls,		// PCI device sub-class (-1 if don't care)
	long   progif)		// PCI device programming interface (-1 if don't care)
{
	PCIB *pcib;

	if ((pcib = *ppcib) == NULL)
		pcib = (PCIB *)pcicctbl.items[0].next;
	else
		pcib = (PCIB *)pcib->next;
	while (pcib != NULL)
	{
		if (pcib->clsc == cls && (subcls == -1 || pcib->subclsc == subcls) &&
				(progif == -1 || pcib->prog == progif))
		{
			*ppcib = pcib;
			return (pcib->addr);
		}
		pcib = (PCIB *)pcib->next;
	}
	return (ER_PDNAV);
}


//***********************************************************************
// Function: xospciFindByAddr - Find PCI device data given device address
// Returned: Value is the encoded configuration address if normal
//           (positive) or a negative XOS error code if error
//***********************************************************************

long XOSFNC __export xospciFindByAddr(
	PCIB **ppcib,		// Address of pointer to receive address of PCIB found
	long   bus,			// Bus address
	long   slot,		// Slot address
	long   func)		// Function number
{
	PCIB *pcib;
	long addr;

	addr = (bus << 16) | (slot << 11) | (func << 8);
	pcib = (PCIB *)pcicctbl.items[0].next;
	while (pcib != NULL)
	{
		if (pcib->addr == addr)
		{
			*ppcib = pcib;
			return (pcib->addr);
		}
		pcib = (PCIB *)pcib->next;
	}
	return (ER_PDNAV);
}


//*************************************************************
// Function: xospciReadCfgReg - Read PCI configuration register
// Returned: Register value (there is no error indication)
//*************************************************************

long XOSFNC __export xospciReadCfgReg(
	PCIB  *pcib,
	long   reg)
{
	OUTL(P_PCI1ADDR, (pcib->addr + reg) | 0x80000000);
	IOPAUSE;
	return (INL(P_PCI1DATA));
}


//***************************************************************
// Function: xospciWriteCfgReg - Write PCI configuration register
// Returned: Nothing
//***************************************************************

void XOSFNC __export xospciWriteCfgReg(
	PCIB *pcib,
	long  reg,
	long  data)
{
	OUTL(P_PCI1ADDR, (pcib->addr + reg) | 0x80000000);
	IOPAUSE;
	OUTL(P_PCI1DATA, data);
}


//************************************************************************
// Function: xospciWriteCfgRegByte - Write PCI configuration register byte
// Returned: Nothing
//************************************************************************

void XOSFNC __export xospciWriteCfgRegByte(
	PCIB *pcib,
	long  reg,
	long  data)
{
	OUTL(P_PCI1ADDR, ((pcib->addr + reg) & 0xFFFFFFFC) | 0x80000000);
	IOPAUSE;
	OUTB(P_PCI1DATA + (reg & 0x03), (char)data);
}
