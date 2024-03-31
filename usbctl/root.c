//============================================
// ROOT.C Functions which deal with a root hub
// Written by John Goltz
//============================================

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

#include "usbctl.h"


typedef _Packed struct
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    long  intEIP;
    long  intCS;
    long  intEFR;
    short intcnt;
    short intnum;
    CDB  *cdb;
	long  lport;
	long  portsts;
} STSDATA;

typedef _Packed struct
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    long  intEIP;
    long  intCS;
    long  intEFR;
    short intcnt;
    short intnum;
	long  tmrhndl;
	short unit;
	short lport;
} TMRDATA;

typedef _Packed struct
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    long  intEIP;
    long  intCS;
    long  intEFR;
    short intcnt;
    short intnum;
	CDB  *cdb;
	long  gport;
	long  dvseqnum;
	long  ifnum;
	long  ifseqnum;
} RETRYDATA;

static void getdevinfo(CDB *cdb);
static void retrysignal(RETRYDATA retry);
static void setstsvect(CDB *cdb);
static void statussignal(STSDATA sd);
static void timersignal(TMRDATA sd);


//*****************************************************
// Function: rootthread - Main thread function for the
//		thread that manages a root hub (USB controller)
// Returned: Never returns
//*****************************************************

long rootthread(
	long unitnum,
	long usbhndl,
	long vector)
{
	CDB *cdb;
	DDB *ddb;
	long rtn;
	char bufr[64];

	debugprint("In root thread for USB%d:", unitnum);
	if ((cdb = (CDB *)usbmalloc(((sizeof(CDB) + 3) & 0xFFFFFFFC) +
			sizeof(DDB))) == NULL)
	{
		sfwResponse(-errno, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
				"Cannot allocate memory interface data blocks");
		svcSchTerminate(1, -1);
	}
	memset(cdb, 0, ((sizeof(CDB) + 3) & 0xFFFFFFFC) + ((sizeof(DDB) + 3) &
			0xFFFFFFFC));
	ddb = (DDB *)(((char *)cdb) + ((sizeof(CDB) + 3) & 0xFFFFFFFC));

	// Set up the CDB for the device

	cdb->label = 'CDB*';
	cdb->usbhndl = usbhndl;
	cdb->ddb = roottbl[unitnum] = ddb;
	cdb->unit = unitnum;
	cdb->vector = vector;
	cdb->fncresetport = rootresetport;
	cdb->fncdisableport = rootdisableport;

	// Set up a (mostly dummy) DDB for the controller

	ddb->label = 'DDB*';
	ddb->lport = unitnum;
	ddb->gport = unitnum;
	ddb->portshift = 4;
	ddb->rootcdb = ddb->cdb = cdb;

	getdevinfo(cdb);					// Get device information

	rtn = sprintf(bufr, "USB%d: %.4s host controller", cdb->unit, cdb->type);

	if ((ddb->devtxt = (char *)usbmalloc(rtn + 1)) == NULL)
	{
		sfwResponse(-errno, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
				"Cannot allocate memory for device text");
		svcSchTerminate(1, -1);
	}
	strmov(ddb->devtxt, bufr);

	// Set up the status signal

	if ((rtn = setvector(cdb->vector, 4, statussignal)) < 0)
	{
		sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
				"Error setting status signal vector - Cannot continue");
		svcSchTerminate(1, -1);
	}
	setstsvect(cdb);					// Give the status vector to the device
	if ((rtn = svcIoSpecial(cdb->usbhndl, USB_SDF_STATUS, NULL, 0,
			(char *)&specparms)) < 0)
	{
		sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
				"Error requesting status for USB device USB%d: - Cannot "
				"continue", cdb->unit);
		svcSchTerminate(1, -1);
	}

	// Set the debounce timer vector

	if ((rtn = setvector(cdb->vector + 1, 4, timersignal)) < 0)
	{
		sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
				"Error setting debounce timer signal vector - Cannot "
				"continue");
		svcSchTerminate(1, -1);
	}

	// Set the retry signal vector

	if ((rtn = setvector(cdb->vector + 2, 4, retrysignal)) < 0)
	{
		sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
				"Error setting retry signal vector - Cannot continue");
		svcSchTerminate(1, -1);
	}

	debugprint("Retry vector set to %d for USB%d:", cdb->vector + 2, cdb->unit);

	sprintf(bufr, "Controller USB%d: (%.4s) initialized", cdb->unit, cdb->type);
	debugprint(bufr);

	sfwResponse(0, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_PREFIX, bufr);

	// Let the base thread continue

	atomic_dec(startcnt);
	svcSchWake(0);

	// After this the set up function is not active. Now all we need to do
	//   is wait for signals.

	svcSchSetLevel(0);
	while (TRUE)
	{
		svcSchSuspend(NULL, -1);
	}
}


//*******************************************************************
// Function: getdevinfo - Get information about the controller device
// Returned: Nothng (Does not return if error)
//*******************************************************************

// This is a seperate function so we don't fill up the stack permanently
//   with our private copy the the device characteristics.

static void getdevinfo(
	CDB *cdb)
{
	static cchar debugtxt[] = "Active debug device";

	DDB *ddb;
	long rtn;
	_Packed struct
	{   TEXT4CHAR type;
		BYTE4CHAR numports;
		BYTE4CHAR irq;
		BYTE4CHAR ioreg;
	    uchar     end;
	} infochar =
	{	{PAR_GET|REP_TEXT, 4, "TYPE"},
		{PAR_GET|REP_DECV, 4, "NUMPORTS"},
		{PAR_GET|REP_DECV, 4, "INT"},
		{PAR_GET|REP_HEXV, 4, "IOREG"}
	};
	_Packed struct
	{	BYTE1CHAR dbport;
		uchar     end;
	} debugchar =
	{	{PAR_GET|REP_DECV, 1, "DBPORT"}
	};

	if ((rtn = svcIoDevChar(cdb->usbhndl, (char *)&infochar)) < 0)
	{
		sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
				"Error getting device information for USB%d: - Cannot "
				"continue", cdb->unit);
		svcSchTerminate(1, -1);
	}
	memcpy(cdb->type, infochar.type.value, 4);
	cdb->numports = infochar.numports.value;
	cdb->irq = infochar.irq.value;
	cdb->ioreg = infochar.ioreg.value;
	if (strncmp(infochar.type.value, "EHCI", 4) == 0 &&
			(rtn = svcIoDevChar(cdb->usbhndl, (char *)&debugchar)) >= 0)
	{
		if ((cdb->debugport = debugchar.dbport.value) != 0)
		{
			debugprint("Debugport = %d", cdb->debugport);
			if (makeddb((HDB *)cdb, cdb->debugport, &ddb) >= 0)
			{
				if ((ddb->devtxt = (char *)usbmalloc(sizeof(debugtxt))) == NULL)
				{
					sfwResponse(-errno, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|
							SFWRSP_PREFIX, "Cannot allocate memory for device "
							"text");
					svcSchTerminate(1, -1);
				}
				strcpy(ddb->devtxt, debugtxt);
			}
		}
	}
}


//********************************************************
// Function: setstsvect - Give status vector to the device
// Returned: Nothing (Does not return if error)
//********************************************************

// This is a seperate function so we don't fill up the stack permanently
//   with our private copy the the device parameters.

static void setstsvect(
	CDB *cdb)
{
	long rtn;
	_Packed struct
	{	BYTE1PARM vect;
		BYTE4PARM data;
		TEXT8PARM class;
		BYTE4PARM xfersize;
		BYTE1PARM xfertype;
		BYTE1PARM xferspeed;
		uchar     end;
	} intparms =
	{	{PAR_SET|REP_HEXV, 1, IOPAR_SIGVECT1},
		{PAR_SET|REP_HEXV, 4, IOPAR_SIGDATA},
		{PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "USB"},
		{PAR_SET|REP_HEXV, 4, IOPAR_USB_PKTSIZE, 8},
		{PAR_SET|REP_HEXV, 1, IOPAR_USB_XFERTYPE, USB_XFERTYPE_CNTRL},
		{PAR_SET|REP_HEXV, 1, IOPAR_USB_XFERSPEED, USB_XFERSPEED_FULL}
	};

	intparms.vect.value = cdb->vector;
	intparms.data.value = (long)cdb;
	if ((rtn = svcIoInBlockP(cdb->usbhndl, NULL, 0, (char *)&intparms)) < 0)
	{
		sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
				"Error setting up status signal for USB%d: - Cannot "
				"continue", cdb->unit);
		svcSchTerminate(1, -1);
	}
}


//*****************************************
// Function: statussignal - Signal function
//				for the port status signal
// Returned: Nothng
//*****************************************

static void statussignal(
	STSDATA sd)
{
	DDB *ddb;
	DDB *devddb;
	long rtn;

	atomic_inc(numactive);
	atomic_inc(ttlactive);
	ddb = sd.cdb->ddb;

	debugprint("** status signal for port %08X: sts: new=%04X old:%08X",
			sd.cdb->ddb->gport +
			(sd.lport << sd.cdb->ddb->portshift), sd.portsts,
			sd.cdb->portsts[sd.lport], sd.cdb->resetting);

	if ((sd.cdb->portsts[sd.lport] & USBPS_RST) && (sd.portsts & USBPS_PCS))
	{
		debugprint("-- redundent signal during reset discarded");
		atomic_dec(numactive);
		return;
	}

	if (!(sd.cdb->started) && (sd.portsts & USBPS_PCS) != 0)
	{
		debugprint("Starting device USB%d:", sd.cdb->unit);

		if ((rtn = svcIoSpecial(sd.cdb->usbhndl, USB_SDF_START, NULL, 0,
				(char *)&specparms)) < 0 && rtn != ER_ACT)
		{
			logerror(rtn, "Error starting USB device USB%d - This "
					"USB interface will not be used", sd.cdb->unit);
			sd.cdb->error = rtn;
			svcIoClose(sd.cdb->usbhndl, 0);
			atomic_dec(numactive);
			return;
		}
		sd.cdb->started = TRUE;
		svcSchSuspend(NULL, ST_MILLISEC * 50);

		debugprint("Unit %d started", sd.cdb->unit);
	}
	if (((sd.cdb->portsts[sd.lport] ^ sd.portsts) & USBPS_PCS) == 0)
	{									// Has the connect status changed?
		debugprint("Connect status HAS NOT changed for %08X", (sd.lport << 4) +
				sd.cdb->unit);			// No!
		atomic_dec(numactive);
		return;
	}
	if (sd.portsts & USBPS_PCS)			// Connected now?
	{
		// Here if a device is connected now - start the debounce timer

		if (sd.cdb->portsts[sd.lport] & USBPS_DBN) // Are we debouncing this?
		{
			debugprint("debounce is active for %08X, connection change "
					"ignored", sd.cdb->unit + (sd.lport << 4));
			atomic_dec(numactive);
			return;
		}

		// If not debouncing it - start debouncing

		debugprint("Connect status asserted for %08X - starting debounce",
				(sd.lport << 4) + sd.cdb->unit);

		sd.cdb->dbntmr = svcSchSetAlarm(sd.cdb->vector + 1, (sd.lport << 16) +
				sd.cdb->unit, ST_MILLISEC * 140);
		sd.portsts |= USBPS_DBN;
	}
	else
	{
		// Here if no device is connected now

		if (sd.cdb->portsts[sd.lport] & USBPS_DBN)
		{								// Are we debouncing this port?
			debugprint("Connect status cleared for %08X - aborting debounce",
					(sd.lport << 4) + sd.cdb->unit);
			svcSchClrAlarm(sd.cdb->dbntmr);
		}
		else
		{
			debugprint("Connect status cleared for %08X - removing device",
					(sd.lport << 4) + sd.cdb->unit);
			devddb = sd.cdb->ddbtbl[sd.lport];
			logerror(0, "Device disconnected from %08X", ddb->gport +
					(sd.lport << 4));
			removexosdevice((HDB *)sd.cdb, sd.lport); // Remove the device and
													  //   child devices (if
													  //   hub) from the system 

			debugprint("Device removed at %08X", ddb->gport + (sd.lport << 4));

			critBegin(&cfgcrit);
			deleteusbdevice((HDB *)sd.cdb, sd.lport); // Delete the device and
			critEnd(&cfgcrit);						  //   any child devices

			debugprint("Device deleted at %08X", ddb->gport + (sd.lport << 4));
		}
	}
	sd.cdb->portsts[sd.lport] = sd.portsts;
	atomic_dec(numactive);
}


//********************************************
// Function: timersignal - Signal function for
//				the debounce timer signal
// Returned: Nothng
//********************************************

static void timersignal(
	TMRDATA sd)
{
	CDB *cdb;

	atomic_inc(numactive);
	atomic_inc(ttlactive);
	cdb = roottbl[sd.unit]->cdb;
	sd.lport &= 0xFFFF;

	debugprint("** timer signal for %08X", (sd.lport << 4) + sd.unit);

	if ((cdb->portsts[sd.lport] & USBPS_DBN) == 0)
	{
		logerror(0, "Internal error for %08X; debounce status not set in "
				"timersignal for %08X, portsts = %08x", (sd.lport << 4) +
				 cdb->unit, cdb->portsts[sd.lport]);
		atomic_dec(numactive);
		return;
	}
	if ((cdb->portsts[sd.lport] & USBPS_PCS) == 0)
	{
		debugprint("in timersignal for %8X: Not connected, portsts = %08X",
				cdb->portsts[sd.lport]);
		atomic_dec(numactive);
		return;
	}
	cdb->portsts[sd.lport] &= ~USBPS_DBN;
	setupdevice((HDB *)cdb, sd.lport, (*(long *)cdb->type == 'EHCI') ?
			USB_XFERSPEED_HIGH : (cdb->portsts[sd.lport] & USBPS_LSD) ?
			USB_XFERSPEED_LOW : USB_XFERSPEED_FULL);
	atomic_dec(numactive);
}


//********************************************
// Function: retrysignal - Signal function for
//				the retry signal
// Returned: Nothng
//********************************************

static void retrysignal(
	RETRYDATA retry)
{
	HDB  *hdb;
	DRV   drv;
	DDB  *ddb;
	DDB **dtpnt;
	IFD  *ifd;
	long  rtn;
	long  inx;
	long  pval;
	char  text[32];

	atomic_inc(numactive);
	atomic_inc(ttlactive);

	if (debugflg)
	{
		if (retry.ifnum == -1)
			sprintf(text, "%08X", retry.gport);
		else
			sprintf(text, "%08X-%d", retry.gport, retry.ifnum);
		debugprint("Have retry signal for %s", text);
	}

	// First we must find the device. We cannot just pass the address of the
	//   DDB in the interrupt data since we may delete the device before we
	//   see this signal! We use the global port number and follow the DCB
	//   tables. We then check the device sequence number to make sure it's
	//   really the same device. If we discover it has been removed or removed
	//   and replaced, we do nothing. Note that we do NOT need to interlock
	//   this since only this thread will ever remove any thing from the
	//   configuration tables we are looking at.

	dtpnt = retry.cdb->ddbtbl;
	pval = retry.gport;

	debugprint("RETRY: gport=%X\n", pval);

	while (TRUE)
	{
		pval >>= 4;

		debugprint("RETRY: next gport=%X\n", pval);

		if ((inx = pval & 0xFF) == 0 || (ddb = dtpnt[inx]) == NULL ||
				(hdb = ddb->hdb) == NULL)
			break;
		dtpnt = hdb->ddbtbl;

		debugprint("RETRY: HDB=%X Dtable=%X\n", hdb, dtpnt);
	}

	if (pval == 0 || (pval & 0xFFFFFFF0) != 0 || ddb == NULL || 
			ddb->seqnum != retry.dvseqnum)
	{		
		debugprint("Retry: Could not find device %08X %d/%d", retry.gport,
				(ddb == NULL) ? -1 : ddb->seqnum, retry.dvseqnum);
		atomic_dec(numactive);
		return;
	}
	hdb = ddb->owner.hdb;

	debugprint("RETRY: gport=%X HDB=%X DDB=%X DDB.HDB=%X\n", pval, hdb, ddb,
			ddb->owner.hdb);

	// Here with the DDB for the device to retry

	if (retry.ifnum == -1)				// Device level retry request?
	{
		// Here for a device level retry - This is relatively easy, we
		//   simply remove the device (which is probably not useful anyway,
		//   and reset the port. Once the reset is done, it looks like the
		//   device was just connected any everything should happen normally.

		critBegin(&cfgcrit);
		deleteusbdevice(hdb, pval);
		critEnd(&cfgcrit);

		hdb->portsts[pval] = 0;
		if ((rtn = (hdb->fncresetport)(hdb, pval)) < 0)
		{
			if (rtn == ER_USBNH)
				debugprint("Retrying: not a HS device at %08X", ddb->gport);
			else
				debugprint("Retyring: Error resetting port");
		}
	}
	else
	{
		// Here for a interface level retry - This usually happens because
		//   the necessary configuration info was not available initially
		//   at system start-up. We remove any unnecessary allocated memory
		//   and try to set up the interface again. We DO NOT reset the port
		//   since we may have other interfaces correctly set up and we do
		//   not want to distrub them.

		ifd = ddb->fifd;				// Find the requested interface
		do
		{
			if (ifd->number == retry.ifnum)
				break;
		} while ((ifd = ifd->next) != NULL);
		if (ifd == NULL || ifd->seqnum != retry.ifseqnum)
		{
			debugprint("Retry: Could not find interface %08X-%d", retry.gport,
					retry.ifnum);
			atomic_dec(numactive);
			return;
		}

		// See if we have a set up defined for the VID/PID pair

		if (!findvendor(ddb->usbven, ddb->usbdev, &drv))
			drv.func[0] = 0;
		else
			debugprint("Retrying: Found ven/dev: %s %s %s", drv.func, drv.clss,
					drv.drvr);

		// Get rid of any data allocated for the interface

		critBegin(&cfgcrit);
		if (ifd->devlst != NULL)
		{
			usbfree(ifd->devlst);
			ifd->devlst = NULL;
		}
		if (ifd->devtxt != NULL)
		{
			usbfree(ifd->devtxt);
			ifd->devtxt = NULL;
		}
		critEnd(&cfgcrit);

		// Try to create a device for the interface

		setupinterface(ddb, ifd, &drv);
	}
	atomic_dec(numactive);
}

int seqval = 1;

//***************************************************
// Function: rootresetport - Reset a root hub port
// Returned: 0 if normal, negative XOS error if error
//***************************************************

long rootresetport(
	HDB *hdb,
	int  lport)
{
	CDB *cdb;
	long rtn;
	long portmask;
	long seqnum;

	seqnum = seqval++;
	cdb = (CDB *)hdb;
	portmask = 1 << lport;
	cdb->resetting |= portmask;

	cdb->portsts[lport] |= USBPS_RST;

	debugprint("in rootresetport: (%d) resetting port %08X", seqnum,
			(lport << 4) + cdb->unit);

	if ((rtn = svcIoSpecial(cdb->usbhndl, USB_SDF_RESETPORT + (lport << 8),
			NULL, 0, (char *)&specparms)) < 0)
	{
		debugprint("in rootresetport: (%d) port reset for %08X failed: %d",
				seqnum, (lport << 4) + cdb->unit, rtn);
		return (rtn);
	}

	debugprint("in rootresetport: (%d) port reset for %08X worked", seqnum,
			(lport << 4) + cdb->unit);
	svcSchSuspend(NULL, ST_MILLISEC * 40);
	cdb->resetting &= ~portmask;
	debugprint("in rootresetport: (%d) Enabling port %08X:", seqnum,
			(lport << 4) + cdb->unit);

	if ((rtn = svcIoSpecial(cdb->usbhndl, USB_SDF_ENABLEPORT + (lport << 8),
			NULL, 0, (char *)&specparms)) < 0)
	{
		debugprint("in rootresetport: port enable failed: %d", rtn);

		cdb->portsts[lport] &= ~(USBPS_RST|USBPS_PSS|USBPS_PES|USBPS_PCS);
		return (rtn);
	}

	debugprint("in rootresetport: port enable worked");

	cdb->portsts[lport] &= ~USBPS_RST;
	debugprint("in rootresetport: Requesting status for USB%d:", cdb->unit);
	if ((rtn = svcIoSpecial(cdb->usbhndl, USB_SDF_STATUS, NULL, 0,
			(char *)&specparms)) < 0)
		return (rtn);
	return (0);
}


long rootdisableport(
	HDB *hdb,
	int  lport)
{
	hdb = hdb;
	lport = lport;
	return (0);
}
