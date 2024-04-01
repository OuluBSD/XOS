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
#include <xoserr.h>
#include <xosx.h>
#include <xosxchar.h>
#include <xosxparm.h>
#include <xosxlke.h>
#include <xosxnet.h>

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0

// The Internet protocol stack consists of a number of LKEs which must be
//   loaded in a specific order as follows:
//		NETCLS		Must be loaded first.
//		ENxxxDRV	Interface drivers - Must be loaded after NETCLS and before
//					  any Internet units are added that use that driver. It's
//					  always OK (and generally easier) to load all drivers at
//					  this point. Note that any drivers that do not find any
//					  matching hardware do not "stick".
//		ARPCLS		Must be loaded before IPCLS.
//		IPCLS		Must be loaded before any transport level classes that
//					  use IP.
//		ICMPCLS		Should be loaded immediately after IPCLS since it provides
//					  necessary services for IPCLS.
//		TCPCLS		TCPCLS and UDPCLS can be loaded in either order. Each must
//		UDPCLS		  be loaded before and deviced which use it is added.
//		XFPCLS		Must be loaded after TCPCLS.

// It is generally recommended to load all of the above LKEs before any
//   network devices are created, although it is possible to create devices
//   as soon as all of the necessary LKEs have been loaded. It is difficult
//   to come up with a situation where this is a desirable thing to do.

// There is also an order dependence when adding network units. Generally,
//   when the unit number for an underlying unit must be specified when
//   adding a unit, that underlying unit must have been added previously.
//   Specifically:
//		The NIFn: unit used must be added before an ARPn: unit is added.
//		The ARPn: unit must be added before an IPn: unit is added.
//		The IPn: unit must be added before an ICMPn:, TCPn:, or UDPn: unit
//		  is added.
//		The TCPn: unit must be added before an XFPn: unit is added.
//   Obviously, the corresponding LKE must be loaded before a unit is added.

// It would be somewhat easier in general to automatically add interface units
//   automatically when an interface device driver is loaded. Unfortunately,
//   this would not allow control of the assignment of unit numbers to the
//   interfaces. This is absolutely necessary when a machine has interfaces
//   to more than one network.


static INITLKE initnet;

#pragma data_seg(_HEAD);
	LKEHEAD(NETCLS, initnet, MAJVER, MINVER, EDITNUM, CLASS); 
#pragma data_seg();

///long knlComDPParms;

long nifdcbnum;			// Number of in use network interface DCBs
long nifdcbmax;			// Maximum in use network interface DCBs
long nifdcblmt = 10000;	// Maximum number of network interface DCBs allowed

IDB  *firstidb;
NPDB * __export xosnetFirstNpdb;
TPDB * __export xosnetFirstTpdb;

static PARMGETFNC nifgtctlmode;
static PARMSETFNC nifstctlmode;
static PARMGETFNC nifgtprotocol;
static PARMSETFNC nifstprotocol;

static long XOSFNC nifaddunit(void);

static DEVCHKFNC   nifdevchk;

static long XOSFNC nifinpblk(void);
static long XOSFNC nifoutblk(void);

static CHARFNC nautype;
static CHARFNC nauunit;

// Class function dispatch table for NIF class devices

static CFDISP nifcls = { 3,
{	nifaddunit,			// CF_ADDUNIT =  8 - Add unit
	NULL,				// CF_PUNITS  =  9 - Get information about
						//                     physical units
	NULL				// CF_AUNITS  = 10 - Get information about active
}};

// Class characteristics tables for the NIF device class

static INFO(msgnumber , "Number of in use NIF devices");
static INFO(msgmaximum, "Maximum number of in use NIF devices");
static INFO(msglimit  , "Maximum number of NIF devices allowed");

static const CHARTBL nifcctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(NUMBER  , DECV, U, 4, &msgnumber , sysChrGetULong, NULL        ,
		&nifdcbnum),
  CTITEM(MAXIMUMX, DECV, U, 4, &msgmaximum, sysChrGetULong, sysChrSetULong,
		&nifdcbmax),
  CTLAST(LIMIT   , DECV, U, 4, &msglimit  , sysChrGetULong, sysChrSetULong,
		&nifdcblmt)
}};


//*************************************************
// Device IO parameter tables for NET class devices
//*************************************************

// Network IO parameter table (0x05xx)

static PARMTBL2 nifnetparams = {3,
{	PTITEM(U, 4, NULL         , NULL),			//                   = 0x0500
	PTITEM(U, 4, NULL         , NULL),			// IOPAR_NETSUBMASK  = 0x0501
	PTITEM(U, 4, nifgtprotocol, nifstprotocol)	// IOPAR_NETPROTOCOL = 0x0502
}};

// Top level table

static PARMTBL1 nifparams = {6,
{	&knlIoStdGenParams,		// 0x00xx - Generic IO parameters
	&knlIoDfltFilParams,	// 0x01xx - File system IO parameters
	&knlIoDfltTrmParams,	// 0x02xx - Terminal IO parameters
	NULL,					// 0x03xx - Disk IO parameters
	NULL,					// 0x04xx - Tape IO parameters
	&nifnetparams			// 0x05xx - Network IO parameters
}};

//********************************************************
// Device dependent parameter table for NIF: IO operations
//********************************************************

static PARMTBL2 nifddparams2 = {2,
{	PTITEM(U, 12, sysIopGetClass, sysIopChkClass), // 0x8000 Device class
	PTITEM(U, 4 , nifgtctlmode  , nifstctlmode)	   // IOPAR_NET_CTLMODE = 0x8001
}};

static PARMTBL1 nifddparams = {1,
{	&nifddparams2,			// 0x80xx
}};

static long XOSFNC nifgivedcb(void);

// Device dispatch table for NIF class devices

static const QFDISP nifqfdisp =
{	NULL,				// sd_mount    =  0 - Mount
	nifgivedcb,			// sd_cleardcb =  4 - Clear DCB
	NULL,				// sd_opena    =  8 - Open additional
	sysIoMinOpen,		// sd_open1    = 12 - Open device/file
	NULL,				// sd_findfile = 16 - Device parameters
	NULL,				// sd_delete   = 20 - Delete file
	NULL,				// sd_rename   = 24 - Rename file
	nifinpblk,			// sd_inblock  = 28 - Input block
	nifoutblk,			// sd_outblock = 32 - Output block
	NULL,				// sd_getiosts = 36 - Get input/output status
	NULL,				// sd_special  = 40 - Special device function
	sysIoCloseFin,		// sd_close    = 44 - Close file
	NULL,				// sd_label    = 48 - Get device label
	NULL,				// sd_commit   = 52 - Commit data to disk
	NULL,				// sd_devinfo  = 56 - Get device info
	NULL				// sd_vfychn   = 60 - Verify changed disk
};


//************
// The NIF CCB
//************

CCB __export xosnetNifCcb =
{	'*BCC',
	0,					// next    - Address of next CCB
	"NIF",				// name    - Name of this class
	0xFFFFFF,			// npfxmsk - Name prefix part mask
	'NIF',				// npfxval - Name prefix part value
	nifdevchk,			// devchk  - Address of device check routine
	&nifcls,			// fdsp    - Address of class func disp table
	&nifcctbl			// clschar - Address of class char table
};


static const CHARITEM nifaublk[] =
{	CTITEM(UNIT, DECV, U, 1, NULL, NULL, nauunit, 0),
	CTLAST(TYPE, TEXT, U, 4, NULL, NULL, nautype, 0)
};


//***********************************************************
// Function: initnet - Initialization routine
// Returned: 0 if normal or a negative XOS error code if erro
//***********************************************************

#pragma code_seg ("x_ONCE");

static long XOSFNC initnet(
	char **pctop,
	char **pdtop)
{
	long rtn;

	sysChrFixUp((CHARTBL *)&nifcctbl);
	sysChrFixUp((CHARTBL *)&nifaublk);
	if ((rtn = sysIoNewClass(&xosnetNifCcb)) >= 0)
		*pctop = codetop;
	return (rtn);
}

#pragma code_seg ();


//==================================================
// Following functions are specific to NIFn: devices
//==================================================


//************************************************************
// Function: nifaddunit - Implements the CL_ADDUNIT function
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// For this device the add unit function sets up a single network interface.
//   It does this by calling the apropriate network device driver.

// Description block for addunit characteristics

typedef struct
{	long  unit;
	llong type;
} NAUDATA;

static long XOSFNC nifaddunit(void)
{
	IDB    *idb;
	NAUDATA data;
	long    rtn;

	if (knlTda.buffer2 == NULL)
		return (ER_CHARM);
	data.unit = -1;
	data.type = -1;
	if ((rtn = sysChrValuesNew(TRUE, nifaublk, &data)) < 0)
		return (rtn);
	if (data.unit == -1 || data.type == -1)
		return (ER_CHARM);
	idb = firstidb;						// Is the unit defined now?
	while (idb != NULL)
	{
		if (data.unit == idb->unit)
			return (ER_DUADF);			// Yes - fail
		idb = idb->next;
	}									// No - try to add the unit
	if ((rtn = sysIoDriverAddUnit((char *)&data.type, &xosnetNifCcb,
			data.unit)) < 0)
		return (rtn);
	knlTda.amount = rtn;
	knlTda.status |= QSTS_DONE;
	knlTda.dcb = NULL;
	return (0);
}


//*****************************************************************
// Function: nautype - Process TYPE characteristic when adding unit
// Returned: 0
//*****************************************************************

static long XOSFNC nautype(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSGetULLongV(val, &((NAUDATA *)data)->type, cnt));
}


//*****************************************************************
// Function: nauunit - Process UNIT characteristic when adding unit
// Returned: 0
//*****************************************************************

static long XOSFNC nauunit(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	if (value > 99)
		return (ER_CHARS);
	((NAUDATA *)data)->unit = value;
	return (0);
}



//******************************************************************
// Function: nifdevchk - Device check function for NIF devices
// Returned: 0 if no match, 1 if found a match, 2 if need to restart
//				search, or a negative XOS error code if error.
//******************************************************************

static long nifdevchk(
	char  name[16],		// Device name (16 bytes, 0 filled)
	char *path,			// Path specification
	long  rtndcb)
{
	IDB *idb;
	long rtn;
	int  unit;

	if (isdigit(name[3]))
	{
		if (name[4] == 0)
			unit = name[3] & 0x0F;
		else if (isdigit(name[4]) && name[5] == 0)
			unit = (name[3] & 0x0F) + 10 + (name[4] & 0x0F);
		else
			return (0);
		idb = firstidb;
		while (idb != NULL)
		{
			if (unit == idb->unit)
			{
				if ((rtn = xosnetMakeDcb(name, unit, sizeof(NETDCB),
						&xosnetNifCcb, (IDB *)idb, idb->devchar, &nifqfdisp,
						idb->nifdesc)) < 0)
					return (rtn);
				return (1);
			}
			idb = idb->next;
		}
		return (ER_NSDEV);
	}
	return (0);
}


//**********************************************************
// Function: nifgivedcb - Clear DCB routine for NETn: device
// Returned: ER_BUSY
//**********************************************************

static long XOSFNC nifgivedcb(void)
{
	nifdcbnum--;
	sysIoGiveDymDcb(knlTda.dcb);
	knlTda.dcb = NULL;
	return (0);
}


long XOSFNC nifgtprotocol(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (0);
}


long XOSFNC nifstprotocol(
	PINDEX  inx,
	char   *val,
	long    cnt,
	void   *data)
{
	return (0);
}

#if 0
.PAGE
	.SBTTL	netiopctlmode - Process IOPAR_NET_CTLMODE device parameter

;Here to process the IOPAR_NET_CTLMODE device parameter - Set or clear link
;  control mode.  Link control mode allows a serial network port to be used
;  as a normal async port for communicating with a modem.  A non-zero parameter
;  value enables link control mode, a zero value disables it and resumes normal
;  link operation.  Link control mode is exclusive (only one DCB can have link
;  control mode set at a time) and is cleared automatically when the DCB is
;  closed.  When link control mode is set, all normal network IO is suspended
;  and normal input and output calls can be used to talk to a connected modem.
;  Synchronous links switch to async operation when link control mode is
;  enabled.  If bit 1 is set, DTR is forced low (this forces some modems into
;  command mode).

	DPARMHDR  SET, DECV, 1
netiopctlmode:
	CALL	knlGetParm##		;Get parameter value
	JC	ret002.S		;If error
	MOVL	ESI, dcb_netidb[EDI]	;OK - get IDB offset
	CMPL	EAX, #0
	JE	4$.S
	MOVL	EAX, #LNKCTL_SET
4$:	MOVL	EDX, EAX
	MOVL	ECX, idb_disp[ESI]
	JMPIL	CS:ndf_linkctl[ECX]	;Dispatch to the link level driver

#endif

long XOSFNC nifgtctlmode(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (0);
}

long XOSFNC nifstctlmode(
	PINDEX  inx,
	char   *val,
	long    cnt,
	void   *data)
{
	return (0);
}

#if 0
.PAGE
	.SBTTL	netiopspmodem - Process IOPAR_SPMODEM device parameter

;Here to process the IOPAR_SPMODEM device parameter - Set or clear modem
;  control bits

	DPARMHDR  BOTH, DECV, 2
netiopmodem:
	MOVL	ESI, dcb_netidb[EDI]
	CMPL	idb_lnkctldcb[ESI], EDI ;In modem control mode?
	JNE	10$.S			;No - fail
	TESTB	AH, #PAR$SET		;Yes - want to change the values?
	JE	4$.S			;No
	PUSHL	EAX			;Yes
	CALL	knlGetParm##		;Get parameter value
	JC	ret002.S		;If error
	MOVL	EDX, #LNKCTL_SETMC
	MOVL	ECX, idb_disp[ESI]
	CALLI	CS:ndf_linkctl[ECX]	;Dispatch to the link level driver
	JC	6$.S
	POPL	EAX
4$:	TESTB	AH, #PAR$GET		;Want value returned?
	JE	ret002.S
	MOVL	EDX, #LNKCTL_GETMC

	MOVL	ECX, idb_disp[ESI]
	CALLI	CS:ndf_linkctl[ECX]	;Dispatch to the link level driver
	JMP	knlStrParm4##

6$:	POPL	EDX
ret002:	RET

;Here if not in link control mode - report as illegal parameter index

10$:	MOVL	EAX, #ER_PARMI
	STC
	RET

	.SBTTL	xosnetIopSMode - Process IOPAR_NETSMODE device parameter

;Here to process the IOPAR_NETSMODE device parameter - Set network mode bits

	DPARMHDR  BOTH, HEXV, 4
xosnetIopSMode:
	TESTB	AH, #PAR$SET		;Want to change the value?
	JE	14$.S			;No
	PUSHL	EAX			;Yes
	CALL	knlGetParm##		;Get parameter value
	JC	6$.S
	ORL	dcb_netmode[EDI], EAX	;Set the bits
	JMP	12$.S

	.SBTTL	xosnetIopCMode - Process IOPAR_NETCMODE device parameter

;Here to process the IOPAR_NETCMODE device parameter - Clear network mode bits

	DPARMHDR  BOTH, HEXV, 4
xosnetIopCMode:
	TESTB	AH, #PAR$SET		;Want to change the value?
	JE	14$.S			;No
	PUSHL	EAX			;Yes
	CALL	knlGetParm##		;Get parameter value
	JC	6$.S
	NOTL	EAX
	ANDL	dcb_netmode[EDI], EAX	;Clear the bits
12$:	POPL	EAX
14$:	TESTB	AH, #PAR$GET		;Want value returned?
	JE	ret002.S		;No
	MOVL	EAX, dcb_netmode[EDI]	;Yes - get value
	JMPIL	knlStrParm4##


#endif

//*****************************************************************
// Function: nifinpblk - Handle the sd_inpblock entry - input block
// Returned: Queued IO return
//*****************************************************************

static long XOSFNC nifinpblk(void)
{
///	IDB  *idb;
	NPB  *pkt;
	long  rtn;

	knlTda.timeout = -1;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&nifparams,
			&nifddparams, NULL, 0)) < 0)
		return (rtn);
	if (knlTda.count > 0)
	{
///		idb = ((NETDCB *)knlTda.dcb)->netidb;
///		if (idb->lnkctldcb != NULL)
///		{
///			if (idb->lnkctldcb != knlTda.dcb)
///				return (ER_BUSY);
///			return (idb->disp->linkctl(idb, LNKCTL_INPUT));
///		}

		if ((rtn = sysIoWait(knlTda.timeout, THDS_DW2)) < 0) // Wait for data
			return (rtn);

		pkt = ((NIFDDCB *)knlTda.dcb)->ndvrcvpkt;
		((NIFDDCB *)knlTda.dcb)->ndvrcvpkt = NULL;

		// COPY DATA HERE!!
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}


//******************************************************************
// Function: nifoutblk - Handle the sd_outblock entry - output block
// Returned: Queued IO return
//******************************************************************

static long XOSFNC nifoutblk(void)
{
///	IDB *idb;
	long rtn;

	knlTda.timeout = -1;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&nifparams,
			&nifddparams, NULL, 0)) < 0)
		return (rtn);
	if (knlTda.count > 0)
	{
///		idb = ((NETDCB *)knlTda.dcb)->netidb;
///		if (idb->lnkctldcb != NULL)
///		{
///			if (idb->lnkctldcb != knlTda.dcb)
///				return (ER_BUSY);
///			return (idb->disp->linkctl(idb, LNKCTL_OUTPUT));
///		}

		// COPY DATA HERE!!
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}
