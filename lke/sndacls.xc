//--------------------------------------------------------------------------
// sndacls - XOS class driver for the HDA audio interface
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
#include <xossnd.h>
#include "sndacls.h"

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0

// This driver supports interfaces based on the Intel "High Definition Audio"
//   specification. This intial version is pretty minimal//  it only supports
//   simple stereo output.

// It is implimented as a combined class and device driver since only a single
//   type of sound device is supported (or expected) on a system. Regardless,
//   the amount of common code for different type sound devices is small so
//   this does not really cost much. Also, since each sound class driver exits
//   cleanly upon loading if it does not find a sound device that is supports,
//   it is simple to just load different drivers until one "sticks".

// All supported sound devices use the same specilized API to do efficent DMA
//   stream IO directly to and from user space.

// This driver allows multiple opens on the SND device but only supports one
//   output and one input stream.

#define ourdcb ((SNDADCB *)knlTda.dcb)

static INITLKE initsnda;
static void initfail(SCB *scb);

#pragma data_seg(_HEAD);

	LKEHEAD(SNDACLS, initsnda, MAJVER, MINVER, EDITNUM, CLASS); 

#pragma data_seg();

static void XOSFNC relbuffer(void);
static void XOSFNC relstream(void);
static long XOSFNC setvolume(void);


long xxxintctl = offsetof(REG, intctl);
long xxxcorblbase = offsetof(REG, corblbase);
long xxxicoi = offsetof(REG, icoi);
long xxxsdreg = offsetof(REG, sdreg);
long xxxwallclka = offsetof(REG, wallclka);
long xxxsd0lpiba = offsetof(REG, sd0lpiba);

long xxxvreg = offsetof(SCB, vreg);

static DFRKFNC     sndafork;
static DEVCHKFNC   sndadevchk;
static long XOSFNC sndacdcb(void);
static long XOSFNC sndaopen(char *spec);
static long XOSFNC sndaopena(void);
static long XOSFNC sndainpblk(void);
static long XOSFNC sndaoutblk(void);
static long XOSFNC sndaspc(void);
static void XOSFNC sndaclose(void);

static long sndadcbnum;
static long sndadcbmax;
static long sndadcblmt = 10000;

// SVC dispatch table for SND class devices

static const QFDISP sndaqfdisp =
{	NULL,					// Mount
	sndacdcb,				// Clear DCB
	NULL,					// Open additional
	sndaopen,				// Open
	NULL,					// Find file
	NULL,					// Delete
	NULL,					// Rename
	sndainpblk,				// Input block
	sndaoutblk,				// Output block
	NULL,					// Reserved
	sndaspc,				// Special functions
	sndaclose,				// Close
	NULL,					// Get device label
	NULL,					// Commit data
	NULL,					// Get device info
	NULL					// Verify disk changed
};

// Class characteristics tables for the SND device class

static INFO(msgnumber , "Number of in use SND devices");
static INFO(msgmaximum, "Maximum number of in use SND devices");
static INFO(msglimit  , "Maximum number of SND devices allowed");

static CHARTBL sndacctbl =
{ CTBEGIN(sysChrValuesNew),
 {CTITEM(NUMBER  , DECV, U, 4, &msgnumber , sysChrGetULong, NULL        ,
		&sndadcbnum),
  CTITEM(MAXIMUMX, DECV, U, 4, &msgmaximum, sysChrGetULong, sysChrSetULong,
		&sndadcbmax),
  CTLAST(LIMIT   , DECV, U, 4, &msglimit  , sysChrGetULong, sysChrSetULong,
		&sndadcblmt)
 }
};

// Device parameter tables for SND class devices

static PARMGETFNC iopgetvect;
static PARMSETFNC iopsetvect;
static PARMGETFNC iopgetdata;
static PARMSETFNC iopsetdata;

PARMTBL2 genparams = {16,
{	PTITEM( ,  0, NULL             , NULL),				// 0x0000
	PTITEM(U,  4, sysIopGetFileOptn, sysIopSetFileOptn),// 0x0001
	PTITEM(L, 18, sysIopGetFileSpec, NULL),				// 0x0002
	PTITEM(U,  4, sysIopGetDevSts  , NULL),				// 0x0003
	PTITEM(U,  1, sysIopGetUnitNum , NULL),				// 0x0004
	PTITEM(U, 16, sysIopGetGlbId   , NULL),				// 0x0005
	PTITEM(U,  4, sysIopGetDelay   , sysIopSetDelay),	// 0x0006
	PTITEM(U,  4, sysIopGetTimeOut , sysIopSetTimeOut),	// 0x0007
	PTITEM(U,  1, sysIopGetInpSts  , NULL),				// 0x0008
	PTITEM(U,  1, sysIopGetOutSts  , NULL),				// 0x0009
	PTITEM(U,  1, sysIopGetInpQLmt , sysIopSetInpQLmt),	// 0x000A
	PTITEM(U,  1, sysIopGetOutQLmt , NULL),				// 0x000B
	PTITEM(U,  4, iopgetvect       , iopsetvect),		// 0x000C
	PTITEM( ,  0, NULL             , NULL),				// 0x000D
	PTITEM(U,  4, iopgetdata       , iopsetdata),		// 0x000E
	PTITEM(U,  4, sysIopGetNumOpen , NULL),				// 0x000F
	PTITEM(U,  1, sysIopGetBufrLmt , sysIopSetBufrLmt)	// 0x0010
}};

static PARMTBL1 sndaparams = {3,
{	&genparams,				// 0x00xx - Generic IO parameters
	&knlIoDfltFilParams,	// 0x01xx - File system IO parameters
	&knlIoDfltTrmParams		// 0x02xx - Terminal IO parameters
}};

static PARMGETFNC iopgetvolume;
static PARMSETFNC iopsetvolume;
static PARMGETFNC iopgetrate;
static PARMSETFNC iopsetrate;
static PARMGETFNC iopgetwidth;
static PARMSETFNC iopsetwidth;
static PARMGETFNC iopgetchnls;
static PARMSETFNC iopsetchnls;
static PARMGETFNC iopgetfifosize;
static PARMGETFNC iopgetmapreg;
static PARMSETFNC iopsetmapreg;

static PARMTBL2 sndadv0params = {7,
{	PTITEM(U, 12, sysIopGetClass, sysIopChkClass),	// 0x8000 Device class
	PTITEM(U,  4, iopgetvolume  , iopsetvolume),	// 0x8001 Volume
	PTITEM(U,  4, iopgetrate    , iopsetrate),		// 0x8002 Sample rate
	PTITEM(U,  4, iopgetwidth   , iopsetwidth),		// 0x8003 Sample width
	PTITEM(U,  4, iopgetchnls   , iopsetchnls),		// 0x8004 Number of channels
	PTITEM(U,  4, iopgetfifosize, NULL),			// 0x8005 Get FIFO size
	PTITEM(U,  4, iopgetmapreg	, iopsetmapreg)		// 0x8006 Map device regs
}};

static PARMTBL1 sndadvparams = {1,
{	&sndadv0params
}};

// Define the CCB for the SND class

static CCB sndaccb =
{	'CCB*',					// label
	NULL,					// Address of next CCB
	"SND",					// Name of this class
	0xFFFFFFFF,				// Name prefix part mask
	'SND0',					// Name prefix part value
	sndadevchk,				// Address of device check routine
	NULL,					// Address of class func disp table
	&sndacctbl				// Address of class char table
};


// SND audio interface (type HDAA) device characteristics table

static INFO(msgcodecven, "Codec vendor/device");
static INFO(msgcodecver, "Codec version");
static INFO(msgwidgets , "Widget paths");

static CHARFNC getscb4byte;
static CHARFNC getwidgets;
static CHARLEN getwidlen;

static CHARTBL sndadchartbl =
{ CTBEGIN(sysChrValuesNew),
 {CTITEM(CLASS   , TEXT , U,      8, &knlChrMsgClass   , sysChrDevGetClass,
		sysChrDevChkClass, 0),

  CTITEM(TYPE    , TEXT , U,      4, &knlChrMsgUnitType, sysChrDevGetULong,
		NULL             , offsetof(DCB, typename)),

  CTITEM(MEMREG  , HEXV , U,      4, &knlChrMsgMemReg  , getscb4byte,
		NULL             , offsetof(SCB, preg)),

  CTITEM(INT     , DECV , U,      4, &knlChrMsgInt     , getscb4byte,
		NULL             , offsetof(SCB, intnum)),

  CTITEM(CODECVEN, HEXV , U,      4, &msgcodecven      , getscb4byte,
		NULL             , offsetof(SCB, codecven)),

  CTITEM(CODECVER, HEXV , U,      4, &msgcodecver      , getscb4byte,
		NULL             , offsetof(SCB, codecver)),

  CTLAST(WIDGETS , DATAS, h, 0xFFFF, &msgwidgets       , getwidgets,
		NULL             , (long)getwidlen)
}};


//=============================================================
// Following are various static data tables needed for the HDAC
//=============================================================

// Table which gives CORB buffer size indexed by the REG_CORBSIZE register
//   value (this is also 1/2 the RIRB buffer size)

static long sizetbl[] = {8, 64, 1024};

// Table which defines sample rates

RTVAL ratetbl[] =
{	{  8000, SPCM_8  , 0x07},
	{ 11026, SPCM_11 , 0x42},
	{ 16000, SPCM_16 , 0x02},
	{ 22050, SPCM_22 , 0x42},
	{ 32000, SPCM_32 , 0x0A},
	{ 44100, SPCM_44 , 0x40},
	{ 48000, SPCM_48 , 0x00},
	{ 88200, SPCM_88 , 0x48},
	{ 96000, SPCM_96 , 0x08},
	{176400, SPCM_176, 0x58},
	{192000, SPCM_192, 0x18}
};


//================================================
// Following are once-only initialization routines
//   executed when the LKE is loaded
//================================================


//*********************************************************************
// Function: sndainit - Initializaton routine for the SNDA device
// Returned:  0 if found a device or a negative XOS error code if error
//            search, or a negative XOS error code if error.
//*********************************************************************

// This driver does not use the addunit function. The initialization routine
//   looks for a supported HDA interface (PCI class = 3, subclass = 4) and
//   initializes it as SND0 if it is found. If no supported device is found
//   loading of the LKE fails.

#pragma code_seg (x_ONCE);


long XOSFNC initsnda(
	char **pctop,
	char **pdtop)
{
	SCB   *scb;
	REG   *vreg;
	PCIB  *pcib;
	long   preg;
	long   rtn;
	long   cnt;
	long   outinx;
	char   mask;

	sysChrFixUp(&sndacctbl);
	sysChrFixUp(&sndadchartbl);

	// Find the HDA sound device by class

	pcib = NULL;
	if ((rtn = xospciFindByClass(&pcib, 4, 3, 0)) < 0)
		return (rtn);

	// Here if we have an HDA device

	if ((rtn = sysIoChkInt(pcib->intitem | 0x100)) < 0) // See if its interrupt
		return (rtn);									//   is available
	if ((rtn = sysMemGetXSpace(0x4000, (char **)&vreg)) < 0)
		return (rtn);					// Get 16KB of address space
	preg = pcib->addr0 & 0xFFFFFFF0;	// Map the registers
	if ((rtn = sysMemMapPhyPages((char *)vreg, 4, MT_SYS, 0x7B, preg,
			0xFFFFFFFF)) < 0)
		return (rtn);

	// Reset the controller if not already reset and then bring it out of
	//   reset

	if (vreg->gctl & 0x01)				// Is the controller reset now?
	{
		vreg->gctl = 0;					// No - try to reset it now
		sysSchTimeWait(100 * ST_MILLISEC); // Wait for a short time
		if (vreg->gctl & 0x01)			// Did it really reset?
			return (ER_DEVER);			// No!
	}
	vreg->gctl = 0x01;					// Terminate the reset
	sysSchTimeWait(100 * ST_MILLISEC);	// Wait for a short time

	if ((vreg->gctl & 0x01) == 0)		// Did it come out of reset?
		return (ER_DEVER);				// No!

	// Allocate and initialize our SCB

	if ((rtn = sysMemGetXMem(sizeof(SCB), (char **)&scb)) < 0)
		return (rtn);
	sysLibMemSetLong((long *)scb, 0, sizeof(SCB)/4); // Clear the SCB
	scb->label = 'SCB*';
	scb->fdisp = sndafork;
	scb->fnext = (void *)0xFFFFFFFF;
	*(long *)scb->name = 'SNDA';
	scb->intmask = 0x40000000;
	scb->vreg = vreg;
	scb->preg = preg;
	scb->intnum = pcib->intitem;

	scb->inpsdb.intbit = 0x01;			// Set up the input SDB
	scb->inpsdb.strminx = 0;
	scb->inpsdb.vsdreg = vreg->sdreg + 0;
	scb->inpsdb.stream = 0x20;
	outinx = vreg->gcap.numis;
	scb->outsdb.intbit = 0x01 << outinx; // Set up the output SDB
	scb->outsdb.strminx = outinx;
	scb->outsdb.vsdreg = vreg->sdreg + outinx;
	scb->outsdb.stream = 0x10;
	sndaintfix1 = scb;					// Fix up the interrupt code
	sndaintfix2 = offsetof(SCB, vreg);

	// Set up to access the codecs. First determine how big the RIRB should
	//   be (we use the largest supported size), allocate a buffer, and
	//   initialize the RIRB.

	if (vreg->rirbctl & 0x02)			// Is the DMA engine running?
	{
		initfail(scb);
		return (ER_DEVER);				// ??? - we just reset everything!
	}
	cnt = 3;							// Find the largest supported size
	mask = 0x40;
	do
	{
		if (vreg->rirbsize & mask)
			break;
		mask >>= 1;
	} while (-cnt > 0);
	if (cnt <= 0)
	{
		initfail(scb);
		return (ER_DEVER);				// If we didn't find a size!
	}
	cnt--;
	vreg->rirbsize = cnt;
	scb->rirbsize = sizetbl[cnt] << 1;
	if ((rtn = sysMemGetNcXmb(scb->rirbsize, (char **)&scb->vrirb)) < 0)
	{
		initfail(scb);
		return (rtn);
	}
	scb->prirb = (ulong)sysMemGetPhyAddr((void *)scb->vrirb);
	vreg->rirblbase = scb->prirb;		// Give the controller the physical
										//   address
	vreg->rirbwp = 0x8000;				// Clear the write pointer
	vreg->rirbintcnt = 1;				// Interrupt on each response
	vreg->rirbctl = 0x03;				// Enable RIRB DMA

	// Determine how big the CORB should be (we use the largest supported
	//   size), allocate a buffer, and initialize the CORB

	cnt = 3;							// Find the largest supported size
	mask = 0x40;
	do
	{
		if (vreg->corbsize & mask)
			break;
		mask >>= 1;
	} while (-cnt > 0);
	if (cnt <= 0)
	{
		initfail(scb);
		return (ER_DEVER);				// If we didn't find a size!
	}
	cnt--;
	vreg->corbsize = cnt;
	scb->corbsize = sizetbl[cnt];
	if ((rtn = sysMemGetNcXmb(scb->corbsize, (char **)&scb->vcorb)) < 0)
	{
		initfail(scb);
		return (rtn);
	}
	scb->pcorb = (ulong)sysMemGetPhyAddr(scb->vcorb);
	vreg->corblbase = scb->pcorb;		// Give the controller the physical
										//   address

	// Clear the CORB read pointer - There are two different descriptions of
	//   how to do this in the HDA documents. One says that bit 15 is simply
	//   set to clear the pointer and is always read as 0 (which is the same
	//   as the description for clearing the RIRB write pointer). The other
	//   says that bit 15 must be set to 1 and set back to 0 after verifying
	//   that it is one. I don't know if this is a simple error in the one
	//   document or if there are two (or more) different versions of the
	//   hardware. We do have one version (AMD SB700 chipset) which does make
	//   the bit visible but works if the bit is left set. Another version
	//   (Intel ICH7 (27D8)) also makes the bit visible and DOES NOT work
	//   unless it is cleared. Clearing the bit for the AMD chipset appears to
	//   do no harm. The following code attemps to handle all situations. It
	//   has been observed that when the bit is visible it becomes visible
	//   very quickly (one or two instruction times) after it is set.

	cnt = 100;							// Don't wait too long here
	vreg->corbrp = 0x8000;				// Set the bit to clear the read pointer
	while ((vreg->corbrp & 0x8000) == 0 && --cnt > 0)
		;
	vreg->corbrp = 0;					// Clear the bit (If the hardware
										//   doesn't need this it should do no
	cnt = 100;							//   harm - we hope!)
	while ((vreg->corbrp & 0x8000) && --cnt > 0)
		;
	vreg->corbwp = 0;					// Clear the write pointer
	vreg->corbctl = 0x03;				// Enable CORB DMA

	// Set up our interrupt

	if (sysIoSetInt(scb->name, scb->intnum | 0x100, DT_TRAP, sndaint,
			sndaintexit) < 0)
		CRASH('INAV');					// [Interrupt Not Available] - We
										//   already checked this!
	sysIoEnableInt(scb->intnum);		// Enable the interrupt
	vreg->intctl = 0xC0000000;			// Enable controller interrupts

	// We now have the controller initialized enough that we can talk to the
	//   codecs.

	scb->codecmsk = vreg->codecsts;		// Store the codec state change bits
										//   which indicate which codecs exist

	if ((rtn = setupwidgets(scb)) < 0)	// Set up the widgets for the audio
	{									//   function group
		initfail(scb);
		return (rtn);
	}

	// Here with everything set up - finally, install the SND class

	sndaccb.blkhead = sndaccb.blktail = scb;
	if ((rtn = sysIoNewClass(&sndaccb)) < 0)
	{
		initfail(scb);
		return (rtn);
	}
	*pctop = codetop;
	return (0);
}


//************************************************
// Function: initfail- Give up allocated memory on
//                     initializtion failure
// Returned: Nothing
//************************************************

// Must be called before returning an error after the SCB is allocated

static void initfail(
	SCB *scb)
{
	if (scb->vrirb != NULL)
		sysMemGiveXmb((void *)scb->vrirb);
	if (scb->vcorb != NULL)
		sysMemGiveXmb((void *)scb->vcorb);
	sysMemGiveXmb(scb);
}


#pragma code_seg ();


//=========================================
// End of once-only initialization routines
//=========================================


//========================================================
// Following are characteristics functions for SND devices
//========================================================


//*******************************************************
// Function: getscb4byte- Characteristics function to get
//                        4-byte value from the SCB
// Returned: Number of bytes returned if normal or a
//           negative XOS error code if error
//*******************************************************

static long XOSFNC getscb4byte(
	const CHARITEM *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return(sysUSPutULongV(val, *(long *)(((char *)ourdcb->sndascb) +
			item->data), cnt));
}


//*******************************************************************
// Function: getwidgets - Characteristics function to get widget data
// Returned: Number of bytes returned if normal or a negative XOS
//           error code if error
//*******************************************************************

long XOSFNC getwidgets(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	SCB *scb;

	scb = ourdcb->sndascb;
	if (cnt > scb->wdgdsz)
		cnt = scb->wdgdsz;
	return (sysLibMemCpy(val, (char *)scb->wdgdata, cnt));
}


//******************************************************
// Function: getwidlen - Characteristics length function
//                       to get widget data length
// Returned: Length of the widget characteristic
//******************************************************

long XOSFNC getwidlen(void)
{
	return (ourdcb->sndascb->wdgdsz);
}


//===============================================================
// Following are the common XOS functions that implement a device
//===============================================================


//*******************************************************************
// Function: sndadevchk - Device check routine for the SND device
// Returned:  0 if no match, 1 if found a match, 2 if need to restart
//            search, or a negative XOS error code if error.
//*******************************************************************

// We never need to restart the search for the device so we never return 2.

static long XOSFNC sndadevchk(
	char  name[16],			// Device name (16 bytes, 0 filled)
	char *path,				// Path specification (buffer must be at least
							//   FILESPCSIZE + 1 bytes long)
	long  rtndcb)			// TRUE if should return a DCB
{
	long rtn;

	if (name[4] != 0)
		return (0);
	if (rtndcb)							// Does he want a DCB?
	{
		if (sndadcbnum >= sndadcblmt)
			return (ER_TMDVC);
		if ((rtn = sysIoGetDymDcb(name, DS_NONSTD|DS_PHYS|DS_MAPPED|DS_MOUT|
				DS_MIN, sizeof(SNDADCB), &knlTda.dcb)) < 0 ||
				(rtn = sysIoLinkDcb(knlTda.dcb, &sndaccb)) < 0)
			return (rtn);
		if (++sndadcbnum > sndadcbmax)
			sndadcbmax = sndadcbnum;
		ourdcb->qfdisp = &sndaqfdisp;
		ourdcb->devchar = &sndadchartbl;
		*(llong *)ourdcb->typename = 'SNDA';
		ourdcb->sndascb = (SCB *)sndaccb.blkhead;
		ourdcb->sndacvolume = 0x3FFF3FFF;
		ourdcb->sndachnls = 2;			// Number of channels
		ourdcb->sndawidth = 16;			// Sample width
		ourdcb->sndawidthb = 2;
		ourdcb->sndarate = 48000;		// Sample rate
		ourdcb->sndafmt.l = 0x0011;		// Divide by 1, multipy by 1, 16-bit
	}									//   samples, 2 channels
	return (1);
}


//*************************************************************
// Function: sndaopen - Open routine for SND devices
// Returned: 0 if normal or a negative XOS error code if error
//*************************************************************

static long XOSFNC sndaopen(
	char *spec)
{
	long rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&sndaparams,
			&sndadvparams, NULL, 0)) < 0)
		knlTda.error = rtn;
	return (0);
}


//*************************************************************
// Function: sndaclose - Close routine for SND devices
// Returned: Nothing
//*************************************************************

static void XOSFNC sndaclose(void)
{
	relbuffer();					// Release the buffer if we have one (This
									//   will first stop and release the stream
}									//   if necessary.)


//*******************************************************
// Function: sndacdcb - Clear DCB routine for SND devices
// Returned: ER_BUSY
//*******************************************************

static long XOSFNC sndacdcb(void)
{
	sndadcbnum--;
	if (ourdcb->sndabuffer != NULL)		// Do we have a buffer?
		sysIoUnlockMem(&ourdcb->sndalocktbl); // Yes - unlock it
	sysIoGiveDymDcb(knlTda.dcb);
	return (0);
}


//==============================================================
// The read block and write block functions are not used for IO.
//   They can be used to get or set IO parameters.
//==============================================================


//***********************************************************
// Function: sndainpblk - Input block routine for SND devices
// Returned:  Amount input if normal or a negative XOS error
//			  code if error.
//***********************************************************

static long XOSFNC sndainpblk(void)
{
	long rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&sndaparams,
			&sndadvparams, NULL, 0)) < 0)
		knlTda.error = rtn;
	if (knlTda.count != 0)
		knlTda.error = ER_NOIN;
	return (0);
}


//************************************************************
// Function: sndaoutblk - Output block routine for SND devices
// Returned:  Amount input if normal or a negative XOS error
//			  code if error.
//************************************************************

static long XOSFNC sndaoutblk(void)
{
	long rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&sndaparams,
			&sndadvparams, NULL, 0)) < 0)
		knlTda.error = rtn;
	if (knlTda.count != 0)
		knlTda.error = ER_NOOUT;
	knlTda.status |= QSTS_DONE;
	return (0);
}


// The SND device does not do IO in the standard way. It impliements continuous
//   stream IO using the SPECIAL function to start and stop the stream.

//   A two segment buffer is used. The buffer length specified (in qab.count)
//   specifies the length of one segment. The second segment immediately follows
//   the first. The buffer address and size MUST both be an even mulitple of 128
//   (low order 7 bits must be 0 - this restriction is imposed by the hardware).
//   The buffer address specified (in qab.buffer1) must be a page address (low
//   12 bits must be 0). Since the memory pages containing the buffers will be
//   uncached, it is strongly suggested that no other use be made of pages
//   which contain all or part of a buffer.

//   The function value is used as follows:
//     Bit 0 - Set to start stream
//     Bit 1 - Set to stop stream
//     Bit 2 - Set to set up buffer (buffer1 and count must be set, buffer
//             pages are locked and made non-cachable)
//     Bit 3 - Set to give up buffer (pages are unlocked and made cachable,
//             memory is NOT give up)
//     Bit 4 - Set to set up output stream
//     Bit 5 - Set to set up input stream
//     Bit 6 - Set to give up stream

//   A buffer and a stream must be set up before doing starting a stream to
//   do IO. The buffer must be set up before or at the same time a stream is
//   set up. Both must be set up before or at the same time a stream is started.
//   Once a stream has been stopped (see below), the stream and the buffer may
//   be given up. Giving up the buffer also gives up the stream if it has not
//   already been given up.
  
//   "Set up" and "give up" bits cannot both be set at the same time. Buffers
//   and streams must be given up before new ones can be set up.

//   A device (handle) can be used for input or output. Changing IO direction
//   terminates and gives up the current stream before setting up a new stream.
//   The buffer1 and count arguments are used to specify the address and size
//   of the mapped buffer. This buffer is locked and is in use continuously once
//   a stream is allocated until the stream is given up. If the count value is
//   0 no change is made to the buffer address or size. If the count value is
//   positive this specifies a new buffer address and size (the buffer1 value
//   must contain the non-null buffer address). If the count value is negative
//   the current buffer is released. This also give up the stream.
//   The count value can only be non-0 when IO is not active.

//   Other parameters are specified as device depedent IO parameters as
//     follows:
//       IOPAR_SND_VOLUME = 0x8002 - Volume (default is 0x3FFF3FFF)
//       IOPAR_SND_RATE   = 0x8004 - Sample rate (default is 48000)
//       IOPAR_SND_WIDTH  = 0x8005 - Sample width (default is 16)
//       IOPAR_SND_CHNLS  = 0x8006 - Number of channels (default is 2)

//   These parameters can only be specified when a stream has been set up. The
//   VOLUME value takes effect immediately, even if the stream is active. The
//   other values do not take effect until the next time the stream is started.
//   The values of these parameters are all reset to there default values when
//   a stream is set up.

//   Once the SND device has been opened, the set up operations can be
//   combined or issued separately. (See comment below about initially
//   filling the buffer). 

//   The standard IO parameters IOPAR_SIGVECT1 and IOPAR_SIGDATA are used to
//   specify a vector for the signal that is issued at the end of each buffer
//   segment. This signal must be used since it is the only way to synchronize
//   filling buffer segments with the data transfer.

//   To start output, fill the entire buffer (both segments) with data and issue
//   a SPECIAL function with bit 0 set. A signal will be received when the first
//   segement has been completely output. That segment should be filled with new
//   data at that time. No additional SPECIAL function calls are made until
//   output is to be stopped except that a call can be made to change the volume.
//   On each signal received the next buffer segment should be filled.

//   The buffer should be set up before it is initally filled. This ensures that
//   all data will be committed to memory since setting up the buffer makes all
//   of the buffer pages non-cachable. (HDA devices are usually implemented as
//   PCIex devices which do not support buffer snooping so all memory accessed
//   by the controller must be non-cachable.)

//   Output is stopped by issueing a SPECIAL function call with bit 1 set in the
//   function value. No other bits may be set. Unfortunately, the HDA sound
//   controller is not capable of completely synchronizing stopping output with
//   the current position in the buffer. To cleanly stop output the following
//   proceedure must be used:
//     1. Pad the last buffer segment containing data to its end with silence.
//        If the data does not provide a transition to silence at its end this
//        should be added to avoid a sharp click at the end of the output.
//     2. When the next signal is received fill the next buffer segment with
//        silence and issue an svcIoOutBlockP call with function bit 1 set.
//     3. A final signal will be received with the second signal data item
//        set to -1. This indicates that output has been stopped.
//   Generally only a few samples will be output from the final buffer segment
//   before output stops but the exact number cannot be garenteed.

//   The current version of this driver ONLY supports CHNLS = 2. It does
//   support all widths and rates supported by the hardware. According to
//   the HDA spec the only rate guarenteed to be available is 48KHz.

//   The current version of this driver ONLY supports ONE output stream and
//   ONE input stream. Looking at the architecture of various HDA codecs, it
//   is clear that the intent is to use multiple output streams to support
//   additional output channels, not to provide independent output audio
//   streams. We currenly only support 2 channels. The intent does appear to
//   be to support multiple mixed input sources but this version's support
//   for input is minimal. We only support a single input source and that
//   has NOT been fully tested!!


//********************************************************************
// Function: sndaspc - Special device function routine for SND devices
// Returned:  0 if normal or a negative XOS error code if error
//********************************************************************

// Parameter values are taken from knlTda.dcb, knlTda.buffer1, knlTda.count,
//   and knlTda.option.

static long XOSFNC sndaspc(void)
{
	BD    *bdl;
	SCB   *scb;
	SDB   *sdb;
	SDREG *vsdreg;
	REG   *vreg;
	char  *addr;
	long   rtn;
	long   num;
	long   left;
	long   cnt;
	long   amnt;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&sndaparams,
			&sndadvparams, NULL, 0)) < 0)
	{
		knlTda.error = rtn;
		knlTda.status |= QSTS_DONE;
		return (0);
	}
	if (knlTda.option & SDF_SND_STOP)	// Want to stop the stream?
	{
		if ((ourdcb->sndasts & SSTS_ACTIVE) == 0) // Yes - active now?
		{
			knlTda.error = ER_NACT;		// No - error
			knlTda.status |= QSTS_DONE;
			return (0);
		}
		ourdcb->sndasts |= SSTS_STOPPING; // Yes - set stop request
		knlTda.status |= QSTS_DONE;
		return (0);
	}
	scb = ourdcb->sndascb;
	if (knlTda.option & (SDF_SND_INSTRM|SDF_SND_OUTSTRM|SDF_SND_SETBUFR|
			SDF_SND_START))				// Want to set up something or start?
	{
		if (knlTda.option & (SDF_SND_RELSTRM|SDF_SND_RELBUFR)) // Yes
		{								// Also trying to give up something?
			knlTda.error = ER_FUNC;		// Yes - that's not allowed!
			knlTda.status |= QSTS_DONE;
			return (0);
		}
		if (ourdcb->sndasts & SSTS_ACTIVE) // Have an active stream now?
		{
			knlTda.error = ER_ACT;
			knlTda.status |= QSTS_DONE;
			return (0);
		}
		if (knlTda.option & SDF_SND_SETBUFR) // Want to set up a buffer?
		{
			if (ourdcb->sndabuffer != NULL) // Yes - do we have a buffer?
			{
				knlTda.error = ER_HVBFR; // Yes - fail
				knlTda.status |= QSTS_DONE;
				return (0);
			}
			if ((knlTda.count & 0x07F) || knlTda.count > 0x40000 || 
					(((long)knlTda.buffer1) & 0x0FFF) != 0)
										// Buffer must an even multiple of 128B
										//   and smaller than 256KB and must
			{							//   start on a page boundry
				knlTda.error = ER_BFRER;
				knlTda.status |= QSTS_DONE;
				return (0);
			}

			// Here with a new buffer to set up. The buffer is always divided
			//   into two equal segments. The second segment starts immediately
			//   after the first segment. The knlTda.count value specifies the
			//   length of one segment. We must set up a BDL for the buffer.
			//   First determine the number of pointers needed. This will be
			//   equal to the number of pages in the buffer if each segment
			//   contains only whole pages or this number plus one otherwise.

			num = ((knlTda.count + 0x0FFF) >> 12) << 1;
			ourdcb->sndabdllen = num;	// Get number of buffer pages
			num <<= 4;					// Get bytes needed for the BDL (Each
										//   pointer is 16 bytes.)
			if (num < 128)				// Must be at least 128 bytes to ensure
				num = 128;				//   correct alignment
			if ((rtn - sysMemGetNcXmb(num, (char **)&bdl)) < 0)
			{							// Get an XMB for the BDL
				knlTda.error = rtn;
				knlTda.status |= QSTS_DONE;
				return (0);
			}

			// Here with an XMB allocated for the BDL - Lock the buffer pages
			//   and make them non-cachable.

			if ((rtn = sysIoLockMem(LCKB_NOCACHE, &ourdcb->sndalocktbl,
					knlTda.buffer1, knlTda.count << 1)) < 0)
			{
				sysMemGiveXmb((void *)bdl);
				knlTda.error = rtn;
				knlTda.status |= QSTS_DONE;
				return (0);
			}
			ourdcb->sndabdl = bdl;

			// Construct the BDL

			addr = knlTda.buffer1;
			cnt = 2;
			do
			{
				left = knlTda.count;
				do
				{
					if ((amnt = 0x1000 - (((long)addr) & 0x0FFF)) > left)
						amnt = left;
					bdl->paddr = (ulong)sysMemGetPhyAddr(addr);
					bdl->length = amnt;
					addr += amnt;
					left -= amnt;
					bdl->flags = (left == 0);
					bdl++;
				} while (left > 0);
			} while (--cnt > 0);

			// Here with the BDL set up

			ourdcb->sndabuffer =  knlTda.buffer1; // Save buffer address and
			ourdcb->sndaseglen = knlTda.count;	  //   size in the DCB
		}

		// Here with a buffer set up or if not setting up a buffer

		if (knlTda.option & (SDF_SND_INSTRM|SDF_SND_OUTSTRM))
		{								// Do we want to set setup a stream?
			// Here if want to set up a stream

			if (ourdcb->sndasdb != NULL) // Do we have a stream now?
			{
				knlTda.error = ER_HVCHN; // Yes - fail
				knlTda.status |= QSTS_DONE;
				return (0);
			}
			if (ourdcb->sndabuffer == NULL) // Do we have a buffer?
			{
				knlTda.error = ER_NOBFR; // No - fail
				knlTda.status |= QSTS_DONE;
				return (0);
			}
			sdb = (knlTda.option & SDF_SND_OUTSTRM) ? &scb->outsdb :
					&scb->inpsdb;
			if (sdb->dcb != NULL)		// Is the stream available?
			{
				knlTda.error = ER_CHNNA; // No - fail
				knlTda.status |= QSTS_DONE;
				return (0);
			}
			ourdcb->sndasdb = sdb;		// Grab the stream
			sdb->dcb = ourdcb;
		}

		// Here with a stream set up or if not setting up a stream now

		if (knlTda.option & SDF_SND_START) // Want to start now?
		{
			if (ourdcb->sndasdb == NULL) // Yes - do we have a stream?
			{
				knlTda.error = ER_NOCHN; // No - fail
				knlTda.status |= QSTS_DONE;
				return (0);
			}
			sdb = ourdcb->sndasdb;
			vreg = scb->vreg;
			vsdreg = sdb->vsdreg;

			// Reset the stream

			vsdreg->ctrl = 0;			// Make sure the run bit is clear

			vsdreg->ctrl = 0x01;		// Reset the stream

			cnt = 1000;					// Don't wait too long
			while ((vsdreg->ctrl & 0x01) == 0 && -- cnt > 0)
				;
			vsdreg->ctrl = 0;			// Remove reset
			if (cnt < 0)
			{
				knlTda.error = ER_DEVER;
				knlTda.status |= QSTS_DONE;
				return (0);
			}

			// Here with the stream reset

			vsdreg->bdlp = (ulong)sysMemGetPhyAddr((char *)ourdcb->sndabdl);
										// Give the physical address of the
										//   BDL to controller
			vsdreg->lvi =  ourdcb->sndabdllen - 1; // Set the last valid index
												   //   value
			vsdreg->cbl = ourdcb->sndaseglen << 1; // Set the cyclic buffer
												   //   length
			vsdreg->fmt = ourdcb->sndafmt.l; // Give stream format to the
										     //   controller
			vsdreg->ctrl = sdb->stream << 16; // Set the stream number
			if ((rtn = codeccmd(scb, sdb->cnode, CV_SETCF |
					ourdcb->sndafmt.l)) < 0) // Give stream format to the DAC
			{								 //   or ADC
				knlTda.error = rtn;
				knlTda.status |= QSTS_DONE;
				return (0);
			}
			ourdcb->sndasts |= SSTS_ACTIVE; // Indicate active
			if ((rtn = setvolume()) < 0) // Set the volume control
			{
				knlTda.error = rtn;
				knlTda.status |= QSTS_DONE;
				return (0);
			}
			scb->intmask |= sdb->intbit; // Make the channel's interrupt visible
			vreg->intctl |= sdb->intbit; // Enable the channel's interrupt
			vsdreg->ctrl |= 0x1E;		// Set RUN, enable all stream interrupts
		}
	}
	else
	{
		// Here if not setting up something or starting a transfer - see if
		//   we want to release something

		if (knlTda.option & SDF_SND_RELSTRM) // Want to release the stream?
			relstream();				// Yes
		if (knlTda.option & SDF_SND_RELBUFR) // Want to release the buffer?
			relbuffer();				// Yes
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}


//================================================
// Following are support functions for SND devices
//================================================


//********************************************************************
// Function: codeccmd - Send a command to a codec and get the response
// Returned: 0 if normal or a negative XOS error code if error
//********************************************************************

long XOSFNC codeccmd(
	SCB *scb,			// Address of the SCB
	long addr,			// Codec address (CAD in bits 15-8 and NID in bits 7-0)
	long func)			// Message (verb + command data)
{
	REG *vreg;
	long corbwp;

/// NEED TO WAIT FOR THE CODEC RESOURCE HERE !!!!

	vreg = scb->vreg;	
	corbwp = (vreg->corbwp + 1) & ((scb->corbsize - 1) >> 2); /// CHECK THIS!
	scb->vcorb[corbwp] = (addr << 20) | func;

	TOFORK;
	vreg->corbwp = corbwp;
	scb->rrspwtid = knlTda.addr;
	knlTda.rtnvalue0 = 0;
	sysSchSetUpToWait(-1, 2 * ST_SECOND, THDS_DW2);
	SCHEDULE();

	if (knlTda.rtnvalue0 < 0)
		return (knlTda.rtnvalue0);
	scb->rirbrsp = -1;					// Indicate no response seen

	while (scb->rirbrp != vreg->rirbwp)	// Do we have a response?
	{
		scb->rirbrp = (scb->rirbrp + 1) & ((scb->rirbsize - 1) >> 3);
										// Advance our read pointer
		scb->rirbrsp = scb->vrirb[scb->rirbrp].resp; // Get the response
	}
	return ((scb->rirbrsp == -1) ? ER_NORSP : 0);
}


//***************************************
// Function: relbuffer - Release a buffer
// Returned: Nothing
//***************************************

// The BDL is given up and the buffer pages are unlocked and made cachable.

static void XOSFNC relbuffer(void)
{
	relstream();						// Release the stream if we have one
	if (ourdcb->sndabuffer != NULL)
	{
		sysMemGiveXmb((char *)ourdcb->sndabdl);
		ourdcb->sndabdl = NULL;
		sysIoUnlockMem(&ourdcb->sndalocktbl);
	}
}


//*************************************
// Function: relstrm - Release a stream
// Returned: Nothing
//*************************************

//   If a mapped register pages has been created it is given up.

static void XOSFNC relstream(void)
{
	SCB   *scb;
	SDB   *sdb;
	REG   *vreg;
	SDREG *vsdreg;

	if (ourdcb->sndasdb != NULL)		// Do we have a stream?
	{
		if (ourdcb->sndaregmdb != NULL)	// Do we have a mapped register page?
		{
			sysMemChngMsect(0, ourdcb->sndaregmdb->base, 0); // Yes - give it up
			ourdcb->sndaregmdb = NULL;
		}
		if (ourdcb->sndasts & SSTS_ACTIVE) // Yes - is it active?
		{
			TOFORK;
			if (ourdcb->sndasts & SSTS_ACTIVE) // Yes - is it still active?
			{
				ourdcb->sndasts &= ~SSTS_ACTIVE; // Yes - but not any more

				scb = ourdcb->sndascb;
				vreg = scb->vreg;
				sdb = ourdcb->sndasdb;
				vsdreg = sdb->vsdreg;
				vsdreg->ctrl = 0;		// Clear the run bit and the interrupt
										//   enables
				vreg->intctl &= ~sdb->intbit;
				scb->intmask &= ~sdb->intbit;
			}
			FROMFORK;
			sysSchTimeWait(2 * TICKPERSP); // Wait for 1 or 2 scheduler ticks
		}
		if (ourdcb->sndasdb->dcb != ourdcb)
			CRASH('BSDB');
		ourdcb->sndasdb->dcb = NULL;
		ourdcb->sndasdb = NULL;
	}
}


//*******************************************************
// Function: setvolume - Set hardware's volume control
// Returned: 0 if normal or a negative XOS error if error
//*******************************************************

// This version only adjusts for the number of gain steps, not the size of
//   each step. A specified value of 0x7FFF gives maximum volume (whatever
//   that) is and a value of 0 gives minimum volume.

static long XOSFNC setvolume(void)
{
	SCB *scb;
	SDB *sdb;
	VC  *vpnt;
	long func;
	long rtn;
	long volume;

	scb = ourdcb->sndascb;
	if ((sdb = ourdcb->sndasdb) != NULL && ourdcb->sndasts & SSTS_ACTIVE)
	{
		vpnt = sdb->volume;
		func = CV_SETAGM | ((vpnt->agno & 0x80) ? 0x4000 : 0x8000);
		while (vpnt->anode.val != 0)
		{
			if ((volume = ((ourdcb->sndacvolume >> 16) * (vpnt->agnn + 1)) /
					0x8000) > vpnt->agnn)
				volume = vpnt->agnn;
			if ((rtn = codeccmd(scb, scb->codec | vpnt->anode.num,
					func | 0x2000 | volume | (vpnt->anode.inx << 8))) < 0)
				return (rtn);

			if ((volume = ((ourdcb->sndacvolume & 0x7FFF) * (vpnt->agnn + 1)) /
					0x8000) > vpnt->agnn)
				volume = vpnt->agnn;
			if ((rtn = codeccmd(scb, scb->codec | vpnt->anode.num,
					func | 0x1000 | volume | (vpnt->anode.inx << 8))) < 0)
				return (rtn);
			vpnt++;
		}
	}
	ourdcb->sndaavolume = ourdcb->sndacvolume;
	return (0);
}


//=====================================================
// Following are IO parameter functions for SND devices
//=====================================================


//********************************************************************
// Function: iopgetvect - Get function for IO parameter IOPAR_SIGVECT1
// Returned: Number of bytes stored if normal or a negative XOS error
//           code if error
//********************************************************************

static long XOSFNC iopgetvect(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return(sysUSPutULongV(val, ourdcb->sndavect, cnt));
}


//********************************************************************
// Function: iopsetvect - Set function for IO parameter IOPAR_SIGVECT1
// Returned: 0 if normal or a negative XOS error code if error
//********************************************************************

static long XOSFNC iopsetvect(
	PINDEX inx,
	char  *val,
	long   cnt,
	void  *data)
{
	long value;
	long rtn;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	ourdcb->sndavect = value;
	ourdcb->sndapid = knlPda.pid;
	return (0);
}


//*******************************************************************
// Function: iopgetdata - Get function for IO parameter IOPAR_SIGDATA
// Returned: Number of bytes stored if normal or a negative XOS error
//           code if error
//*******************************************************************

static long XOSFNC iopgetdata(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return(sysUSPutULongV(val, ourdcb->sndadata, cnt));
}


//*******************************************************************
// Function: iopsetdata - Set function for IO parameter IOPAR_SIGDATA
// Returned: 0 if normal or a negative XOS error code if error
//*******************************************************************

static long XOSFNC iopsetdata(
	PINDEX inx,
	char  *val,
	long   cnt,
	void  *data)
{
	long value;
	long rtn;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	ourdcb->sndadata = value;
	return (0);
}


//************************************************************************
// Function: iopgetvolume - Get function for IO parameter IOPAR_SND_VOLUME
// Returned: Number of bytes stored if normal or a negative XOS error
//           code if error
//************************************************************************

static long XOSFNC iopgetvolume(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return(sysUSPutULongV(val, ourdcb->sndacvolume, cnt));
}


//************************************************************************
// Function: iopsetvolume - Set function for IO parameter IOPAR_SND_VOLUME
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************************

static long XOSFNC iopsetvolume(
	PINDEX inx,
	char  *val,
	long   cnt,
	void  *data)
{
	long value;
	long rtn;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	ourdcb->sndacvolume = value;
	return (setvolume());
}


//********************************************************************
// Function: iopgetrate - Get function for IO parameter IOPAR_SND_RATE
// Returned: Number of bytes stored if normal or a negative XOS error
//           code if error
//********************************************************************

static long XOSFNC iopgetrate(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return(sysUSPutULongV(val, ourdcb->sndarate, cnt));
}


//********************************************************************
// Function: iopsetrate - Set function for IO parameter IOPAR_SND_RATE
// Returned: 0 if normal or a negative XOS error code if error
//********************************************************************

// This can be set while a stream is active but is not effective until the
//   next time the stream is started.

// If a value is specified that exactly matches one of the standard values
//   less than 48KHz that is not supported by the stream (Virtually all codecs
//   DO NOT support values less than 44.1KHz!), the corresponding 44.1KHz or
//   48KHz value is used. Otherwise, the value set is the closest value
//   greater than or equal to the value requested that is supported by the
//   stream or the maximum value supported by the stream if a greater value
//   than that is requested. The only value grananteeded to be available is
//   48KHz, although it has been observed that most codecs also support 44.1KHz
//   and also support at least one higher rate.

static long XOSFNC iopsetrate(
	PINDEX inx,
	char  *val,
	long   cnt,
	void  *data)
{
	SDB   *sdb;
	RTVAL *rtpnt;
	long   value;
	long   rtn;

	if (ourdcb->sndasdb == NULL)
		return (ER_NOCHN);
	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	sdb = ourdcb->sndasdb;
	if (value > sdb->maxrate)
		value = sdb->maxrate;
	rtpnt = ratetbl;
	cnt = sizeof(ratetbl)/sizeof(RTVAL);
	do
	{
		if (value == rtpnt->rate)		// Exact match?
		{
			if (rtpnt->rbit & sdb->supported)
				break;
			value = (rtpnt->rfact & 0x40) ? 44100 : 48000;
		}
		else if (value < rtpnt->rate)
			break;
		rtpnt++;
	} while (--cnt > 0);
	if (cnt <= 0)
		rtpnt--;

	// Here with the ratetbl entry for the rate to use

	ourdcb->sndafmt.rfact = rtpnt->rfact;
	ourdcb->sndarate = rtpnt->rate;
	return (0);
}


//**********************************************************************
// Function: iopgetwidth - Get function for IO parameter IOPAR_SND_WIDTH
// Returned: Number of bytes stored if normal or a negative XOS error
//           code if error
//**********************************************************************

static long XOSFNC iopgetwidth(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return(sysUSPutULongV(val, ourdcb->sndawidth, cnt));
}


//**********************************************************************
// Function: iopsetwidth - Set function for IO parameter IOPAR_SND_WIDTH
// Returned: 0 if normal or a negative XOS error code if error
//**********************************************************************

static long XOSFNC iopsetwidth(
	PINDEX inx,
	char  *val,
	long   cnt,
	void  *data)
{
	long value;
	long widthb;
	long rtn;
	long chk;
	char fmt;

	if (ourdcb->sndasdb == NULL)
		return (ER_NSTRM);
	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	switch (value)
	{
	 case 8:
		fmt = 0;
		chk = 0x00010000;
		widthb = 1;
		break;

	 case 16:
		fmt = 1;
		chk = 0x00020000;
		widthb = 2;
		break;

	 case 20:
		fmt = 2;
		chk = 0x00040000;
		goto comm;

	 case 24:
		fmt = 3;
		chk = 0x00080000;
		goto comm;

	 case 32:
		fmt = 4;
		chk = 0x00100000;
	 comm:
		widthb = 4;
		break;

	 default:
		return (ER_PARMV);
	}
	if ((ourdcb->sndasdb->supported & chk) == 0)
		return (ER_PARMV);
	ourdcb->sndawidthb = widthb;
	ourdcb->sndawidth = value;
	ourdcb->sndafmt.bps = fmt;
	return (0);
}


//**********************************************************************
// Function: iopgetchnls - Get function for IO parameter IOPAR_SND_CHNLS
// Returned: Number of bytes stored if normal or a negative XOS error
//           code if error
//**********************************************************************

static long XOSFNC iopgetchnls(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return(sysUSPutULongV(val, ourdcb->sndachnls, cnt));
}


//**********************************************************************
// Function: iopsetchnls - Set function for IO parameter IOPAR_SND_CHNLS
// Returned: 0 if normal or a negative XOS error code if error
//**********************************************************************

// This version only supports 2 channels.

static long XOSFNC iopsetchnls(
	PINDEX inx,
	char  *val,
	long   cnt,
	void  *data)
{
	long value;
	long rtn;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	if (value != 2)
		return (ER_PARMV);
	return (0);
}


//*********************************************************
// Function: iopgetfifosize - Get function for IO parameter
//                            IOPAR_SND_GETFIFOSZ
// Returned: Number of bytes stored if normal or a negative
//           XOS error code if error
//*********************************************************

static long XOSFNC iopgetfifosize(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	if (ourdcb->sndasdb == NULL)
		return (ER_NOCHN);
	return (sysUSPutULongV(val, ourdcb->sndasdb->vsdreg->fifosize, cnt));
}


//*********************************************************
// Function: iopgetmapreg - Get function for IO parameter
//                          IOPAR_SND_GETMAPREG
// Returned: Number of bytes stored if normal or a negative
//           XOS error code if error
//*********************************************************

// An ER_NOCHN error is returned if no stream is set up. The value returned
//   is the offset in the mapped register memory page for the buffer offset
//   value for the current stream.

static long XOSFNC iopgetmapreg(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	if (ourdcb->sndasdb == NULL)
		return (ER_NOCHN);
	return (sysUSPutULongV(val, ((char *)ourdcb->sndasdb->vsdreg) -
			((char *)ourdcb->sndascb->vreg) + offsetof(SDREG, pib), cnt));
}


//************************************************************
// Function: iopsetmapreg - Set function for IO parameter
//                          IOPAR_SND_GETMAPREG
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// The value used when setting this parameter is the address where the 1
//   page of device resgisters is mapped. This memory must be unallocated.
//   The memory is mapped read-only. When read, the value is the offset in
//   the register page of the current position in buffer value for the active
//   stream. The format of the value at this location depends on the type of
//   sound device. For the HDA controller it is a single 32-bit value which
//   specifies the current relative position in the entire buffer.

// This parameter is intended to provide a way for a user program to directly
//   monitor system overhead relative to the sound stream. No other use is
//   supported!

static long XOSFNC iopsetmapreg(
	PINDEX  inx,
	char   *val,
	long    cnt,
	void   *data)
{
	MDB  *mdb;
	char *value;
	long  rtn;

	if (ourdcb->sndasdb == NULL)
		return (ER_NOCHN);
	if ((rtn = sysUSGetULongV(val, (long *)&value, cnt)) < 0)
		return (rtn);
	if (value < (char *)0x1000 || value >= systembase)
		return (ER_PARMV);

	if ((mdb = ourdcb->sndaregmdb) != NULL && mdb->base != value)
	{									// Do we need to unmap registers?
		sysMemChngMsect(0, ourdcb->sndaregmdb->base, 0); // Yes
		ourdcb->sndaregmdb = NULL;
	}
	if ((mdb = ourdcb->sndaregmdb) == NULL && // Do we need to map registers?
			(rtn = sysMemMapPhys(value, PG_READ, 0x1000, ourdcb->sndascb->preg,
				-1, &ourdcb->sndaregmdb)) < 0)
		return (rtn);

///long XOSFNC sysMemMapPhys(char *base, long ptype, long size, char *paddr,
///		long imask, MDB **ppmdb);



	return (0);
}


//=========================================================
// Following are device fork level routines for SND devices
//=========================================================


//**********************************************************
// Function: sndafork - Device fork function for SND devices
// Returned: Nothing
//**********************************************************

void sndafork(
	DCB *blk)
{
	REG     *vreg;
	SDREG   *vsdreg;
	SCB     *scb;
	SDB     *sdb;
	SNDADCB *dcb;
	long     code;
	long     pib;
	long     intbits;
	long     inx;
	char     sdbits;
	char     rirbbits;

	scb = (SCB *)blk;
	vreg = scb->vreg;
	while ((intbits = vreg->intsts) & scb->intmask) // Have any interrupts?
	{
		if (intbits & 0x3FFFFFFF)		// Have a stream interrupt?
		{
			inx = 0;
			do
			{
				sdb = (inx == 0) ? &scb->outsdb : &scb->inpsdb;
				vsdreg = sdb->vsdreg;
				if (intbits & sdb->intbit)
				{
					sdbits = vsdreg->sts; // Get the stream's interrupt bits
					vsdreg->sts = sdbits; // Clear them
					code = (sdbits & 0x18) ?
						((sdbits & 0x08) ? ER_DOURN : ER_DEVER) : 0;

					if ((dcb = sdb->dcb) != NULL) // Do we have a DCB?
					{
						if (dcb->sndasts & SSTS_STOPPING)
						{
							pib = -1;
							dcb->sndasts &= ~(SSTS_STOPPING|SSTS_ACTIVE);
						}
						else
							pib = vsdreg->pib;
						if (dcb->sndasdb == sdb && dcb->sndavect != 0)
							sysSchReqSignal(dcb->sndavect, NULL, 0,
									dcb->sndapid, 3, dcb->sndadata, code, pib);
					}
					else
						pib = -1;
					if (pib == -1)		// Should we stop the stream?
					{
						// Here if need to stop the stream, either because of a
						//   stop request or because there is no DCB associated
						//   with the stream (which should not be possible!)

						vsdreg->ctrl = 0;
						vreg->intctl &= ~sdb->intbit;
						scb->intmask &= ~sdb->intbit;
					}
				}

			} while (++inx < 2);
		}

		// Here with all DMA stream interrupts (if any) handled

		if (intbits & 0x40000000)		// Have a general controller interrupt?
		{
			rirbbits = vreg->rirbsts;	// Get the RIRB interrupt bits
			if (rirbbits & 0x05)		// Have an RIRB interrupt?
			{
				vreg->rirbsts = rirbbits; // Yes - clear the request
				if (rirbbits & 0x04)	// Overrun error?
					CRASH('OVRE');		// [OVer Run Error] - This should never
										//   happen since we only send one
										//   request at a time!

				// Here if not an RIRB overrun, must be a response interrupt

				if (scb->rrspwtid != NULL) // Is a thread waiting for an RIRB
										   //   response?
				{						// Yes - wake him up
					sysIoResumeThread(scb->rrspwtid, 0x5555);
					scb->rrspwtid = NULL;
				}
				else					// No one waiting - must be an
										//   unsolicited response
					CRASH('USOL');		// WE DON'T HANDLE THIS YET!
			}

			// Here with an RIRB interrupt (if any) handled

			if (vreg->corbsts & 0x01)	// Have a CORB interrupt?
				CRASH('MEME');			// [MEM Error] - This is a serious
										//   hardware error!
		}
	}
	vreg->intctl |= 0x80000000;			// Finished - enable interrupts
}
