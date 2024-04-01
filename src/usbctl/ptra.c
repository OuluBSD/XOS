//==================================================
// ptra.c Functions dealing with type A USB printers
// Written by John Goltz
//==================================================

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

// The type A USB printer link driver supports unidirectional (protocol = 1)
//   and bidirectional (protocol = 2) printers for the printer device class
//   (class = 7, subclass = 1).

// This link driver links the standard XOS printer device class to USB
//   devices. It is a simple driver that cares nothing about the data
//   being sent to the printer. It does support reading "write-back" data
//   for bidirectional printers.

// The printer class uses a single set of device names beginning with PTR
//   followed by the unit number. To avoid confusion with legacy printers,
//   we assign device numbers starting with 50 (decimal). A future version
//   will support "named" units with names of the form PTR_somename so that
//   the printer name can be attached to a specific physical printer based
//   printer serial number instead of USB port or order of attachment.

#define FULLSPLPATHSZ 256

static CRIT splclscrit = {'CRIT'};
static CRIT ptrclscrit = {'CRIT'};
static CRIT usbptralnk = {'CRIT'};

static cchar classname[] = "PTR:";	// Class name
static cchar   splname[] = "SPL:";

static void setupname(char *name, char *prefix, char *text);


//***************************************************
// Function: ptrasetup - Set up a type A USB printer
// Returned: 0 if normal, negative XOS error if error
//***************************************************

long ptrasetup(
	CDB *cdb,
	DDB *ddb,
	IFD *ifd)

{
	_Packed struct
	{	TEXT8CHAR  type;
		BYTE4CHAR  unit;
		LNGSTRCHAR ptrcmd;
		BYTE4CHAR  gport;
		BYTE4CHAR  addr;
		BYTE4CHAR  spec;
		BYTE4CHAR  speed;
		BYTE4CHAR  ep0max;
		BYTE4CHAR  epin;
		BYTE4CHAR  inmax;
		BYTE4CHAR  epout;
		BYTE4CHAR  outmax;
		BYTE4CHAR  ven;
		BYTE4CHAR  dev;
		BYTE4CHAR  rel;
		LNGSTRCHAR usbmnftr;
		LNGSTRCHAR usbprod;
		LNGSTRCHAR usbsernm;
		LNGSTRCHAR ptrmnftr;
		LNGSTRCHAR ptrmodel;
		LNGSTRCHAR splname;
		uchar      end;
	} auchars =
	{	{PAR_SET|REP_TEXT, 8, "TYPE", "USBA"},
		{PAR_SET|REP_DECV, 4, "UNIT"},
		{PAR_SET|REP_STR , 0, "PTRCMD"},
		{PAR_SET|REP_DECV, 4, "USBGPORT"},
		{PAR_SET|REP_DECV, 4, "USBADDR"},
		{PAR_SET|REP_HEXV, 4, "USBSPEC"},
		{PAR_SET|REP_TEXT, 4, "USBSPEED"},
		{PAR_SET|REP_DECV, 4, "USBEP0MX"},
		{PAR_SET|REP_DECV, 4, "USBBIN"},
		{PAR_SET|REP_DECV, 4, "USBPKTMX"},
		{PAR_SET|REP_DECV, 4, "USBBOUT"},
		{PAR_SET|REP_DECV, 4, "USBPKTMX"},
		{PAR_SET|REP_HEXV, 4, "USBVEN"},
		{PAR_SET|REP_HEXV, 4, "USBDEV"},
		{PAR_SET|REP_HEXV, 4, "USBREL"},
		{PAR_SET|REP_STR , 0, "USBMNFTR"},
		{PAR_SET|REP_STR , 0, "USBPROD"},
		{PAR_SET|REP_STR , 0, "USBSERNM"},
		{PAR_SET|REP_STR , 0, "PTRMNFTR"},
		{PAR_SET|REP_STR , 0, "PTRMODEL"},
		{PAR_SET|REP_STR , 0, "SPLNAME"}
	};
	QAB auqab =
	{
	    QFNC_WAIT|QFNC_CLASSFUNC,	// func    - Function
	    0,							// status  - Returned status
	    0,							// error   - Error code
	    0,							// amount  - Amount
	    0,							// handle  - Device handle
	    0,							// vector  - Signal vector
	    {0},
	    CF_ADDUNIT,					// option  - Options or command
	    0,							// count   - Count
		(char *)classname,			// buffer1 - Pointer to file spec
	    NULL,						// buffer2 - Unused
	    NULL						// parm    - Pointer to parameter area
	};
	_Packed struct
	{	BYTE4CHAR  unit;
		LNGSTRCHAR splname;
		LNGSTRCHAR spldev;
		LNGSTRCHAR splpath;
		LNGSTRCHAR ptrname;
		BYTE4CHAR  seqnum;
		uchar      end;
	} splchars =
	{	{PAR_SET|REP_DECV, 4, "UNIT"},
		{PAR_SET|REP_STR , 0, "SPLNAME"},
		{PAR_SET|REP_STR , 0, "SPLDEV"},
		{PAR_SET|REP_STR , 0, "SPLPATH"},
		{PAR_SET|REP_STR , 0, "PTRNAME"},
		{PAR_SET|REP_DECV, 4, "SEQNUM"}
	};
	QAB splqab =
	{
	    QFNC_WAIT|QFNC_CLASSFUNC,	// func    - Function
	    0,							// status  - Returned status
	    0,							// error   - Error code
	    0,							// amount  - Amount
	    0,							// handle  - Device handle
	    0,							// vector  - Vector for interrupt
	    {0},
	    CF_ADDUNIT,					// option  - Options or command
	    0,							// count   - Count
		(char *)splname,			// buffer1 - Pointer to file spec
	    NULL,						// buffer2 - Unused
	    NULL						// parm    - Pointer to parameter area
	};
	EPT  *ept;
	char *pnt;
	char *spnt;
	char *dpnt;
	char *bgn;
	long  rtn;
	int   cnt;
	int   cmdlen;
	int   ptrcmdlen;
	int   mnftrlen;
	int   modellen;
	char  devid[600];
	char  ptrcmd[128];
	char  splname[32];
	char  mnftr[48];
	char  model[48];
	char  devname[16];

///	char  splpath[128];
///	char  fullsplpath[FULLSPLPATHSZ];

	char  chr;
	char  save;

	auqab.buffer2 = (char *)&auchars;
	splqab.buffer2 = (char *)&splchars;

	debugprint("in ptrasetup, lport=%d gport=0x%X", ddb->lport, ddb->gport);

	// The the printer ID string

	memset(devid, 0, sizeof(devid));

	// NOTE: The interface number is in the HIGH byte of wIndex, NOT the LOW
	//       byte as with other requests! The low byte is the alternate setting
	//       value, which we do not currently support.

	if ((rtn = dosetup(cdb, ddb, ddb->maxpktsz0, ddb->speed, ddb->addr, 0,
			0xA1, 0, ddb->curcfg->index, ifd->number << 8, (uchar *)devid,
			sizeof(devid) - 1)) < 0)
	{
		// NOTE: While this is an IF level error, we report it at the device
		//       level. Since the spec says that printers can only have one
		//       interface, this should be OK!

		deverror(ddb, rtn, "Error getting printer ID");
		return (FALSE);
	}
	mnftr[0] = 0;
	model[0] = 0;
	auchars.ptrmodel.strlen = 0;
	ptrcmdlen = 0;
	ptrcmd[0] = 0;
	strcpy(splname, "USB");	
	pnt = devid + 2;
	debugprint("DevID: %s", pnt);
	while (*pnt != 0)
	{
		bgn = pnt;
		while (*pnt != ';' && *pnt != 0)
			pnt++;
		save = *pnt;
		*pnt = 0;

		spnt = bgn;
		while (*spnt != ':' && *spnt != 0)
			spnt++;
		if (*spnt++ == ':')
		{
			cmdlen = spnt - bgn;

///			debugprint("%d === %s === %s", cmdlen, bgn, spnt);

			if ((cmdlen == 4 && strnicmp(bgn, "CMD", 3) == 0) ||
					(cmdlen == 12 &&  strnicmp(bgn, "COMMAND SET", 11) == 0))
			{
				cnt = 126;
				dpnt = ptrcmd;
				while (--cnt > 0 && (chr = *spnt++) != 0 && chr != ';')
				{
					if (chr != ' ')
						*dpnt++ = chr;
				}
				*dpnt = 0;
				ptrcmdlen = dpnt - ptrcmd;

				debugprint("PTRCMD: (%d) %s", ptrcmdlen, ptrcmd);
			}
			else if ((cmdlen == 4 && strnicmp(bgn, "MDL", 3) == 0) ||
					(cmdlen == 6 &&  strnicmp(bgn, "MODEL", 5) == 0))
			{
				modellen = strnmov(model, spnt, sizeof(model) - 1) - model;
				if (strnicmp(spnt, "DESKJET ", 8) == 0)
					setupname(splname, "DJ", bgn + 12);
				else if (strnicmp(spnt, "OFFICEJET ", 10) == 0)
					setupname(splname, "OJ", bgn + 14);
				else if (strnicmp(spnt, "LASERJET ", 9) == 0)
					setupname(splname, "LJ", bgn + 13);
				else if (strnicmp(spnt, "HL-", 3) == 0)
					setupname(splname, "HL", bgn + 7);
				else
					strcpy(splname, "PRINTER");
			}
			else if ((cmdlen == 4 && strnicmp(bgn, "MFG", 3) == 0) ||
					(cmdlen == 12 &&  strnicmp(bgn, "MANFACTURER", 11) == 0))
				mnftrlen = strnmov(mnftr, spnt, sizeof(mnftr) - 1) - mnftr;
		}
		debugprint("%s", bgn);
		if (save != 0)
			pnt++;
		else
			break;
	}
	debugprint("Spool name: %s", splname);

	auchars.epin.value = 0;
	auchars.epout.value = 0;
	ept = ifd->fept;
	while (ept != NULL && (auchars.epin.value == 0 || auchars.epout.value == 0))
	{
		if (ept->attrib == 0x02)
		{
			if ((ept->number & 0x80) && ept->number <= 0x8F)
			{
				if (auchars.epin.value == 0)
				{
					auchars.epin.value = ept->number & 0x0F;
					auchars.inmax.value = ept->maxpktsz;
				}
			}
			else if (ept->number <= 0x0F)
			{
				if (auchars.epout.value == 0)
				{
					auchars.epout.value = ept->number;
					auchars.outmax.value = ept->maxpktsz;
				}
			}
		}
		ept = ept->next;
	}
	if ((ifd->protocol != 1 && auchars.epin.value == 0) ||
			auchars.epout.value == 0)
	{
		logerror(0, "Invalid PRTA configuration: Required endpoint missing: "
				"Input:%d Output:%d", auchars.epin.value,
				auchars.epout.value);
		return (ER_USBBC);
	}

	debugprint("have type A printer at %d on USB%d:", ddb->addr, cdb->unit);
	debugprint("epin = %d, epout = %d", auchars.epin.value,
			auchars.epout.value);

	// Get here when it looks like a valid type A printer. Make sure the
	//   printer class driver is loaded

	critBegin(&ptrclscrit);
    if (!checklke("PTRCLS"))
    {
		debugprint("loading PTRCLS");

		if (lkeloadf(0x01, "PTRCLS.LKE", NULL) != 0)
		{
			critEnd(&ptrclscrit);
			logerror(0, "Could not load PTRCLS.LKE. The type A printer "
					"will not be available");
			return (ER_ERROR);
		}
    }
	critEnd(&ptrclscrit);

	// Load the client driver if its not already loaded

	critBegin(&usbptralnk);
    if (!checklke("USBPTRALNK"))
    {
		debugprint("loading USBPTRALNK");

		if (lkeloadf(0x01, "USBPTRALNK.LKE", NULL) != 0)
		{
			critEnd(&usbptralnk);
			logerror(0, "Could not load USBPTRALNK.LKE. The type A printer "
					"will not be available");
			return (ER_ERROR);
		}
    }
	critEnd(&usbptralnk);

	debugprint("USBPTRALNK is loaded");

	// Here with our driver loaded - Finish setting up the addunit arguement
	//   block

	//  First see if we can have another USB printer unit

	if ((auchars.unit.value = allocunitnumber("PTR")) < 0)
	{
		logerror(ER_TMDDV, "Can not create USB type A disk unit");
		return (ER_TMDDV);
	}
	strcpy(ifd->clss, "PTR");
	strcpy(ifd->name, "PTR");
	auchars.unit.value += 50;
	ifd->allocofs = 50;
	ifd->allocfact = 1;
	ifd->unit = auchars.unit.value;

	// OK - create the device

	auchars.ven.value = ddb->usbven;
	auchars.dev.value = ddb->usbdev;
	auchars.rel.value = ddb->usbrel;
	auchars.gport.value = ddb->gport;
	auchars.addr.value = ddb->addr;
	auchars.spec.value = ddb->usbspec;
	auchars.speed.value = (ddb->speed == USB_XFERSPEED_HIGH) ? 'H' :
			(ddb->speed == USB_XFERSPEED_FULL) ? 'F' : 'L';
	auchars.ep0max.value = ddb->maxpktsz0;
	auchars.usbmnftr.buffer = ddb->usbmnftr; // Manufacturer string
	auchars.usbmnftr.bfrlen = auchars.usbmnftr.strlen = strlen(ddb->usbmnftr);
	auchars.usbprod.buffer = ddb->usbprod; // Product (model) string
	auchars.usbprod.bfrlen = auchars.usbprod.strlen = strlen(ddb->usbprod);
	auchars.usbsernm.buffer = ddb->usbsernm; // Serial number string
	auchars.usbsernm.bfrlen = auchars.usbsernm.strlen = strlen(ddb->usbsernm);
	auchars.ptrmnftr.buffer = mnftr;
	auchars.ptrmnftr.bfrlen = auchars.ptrmnftr.strlen = mnftrlen;
	auchars.ptrmodel.buffer = model;
	auchars.ptrmodel.bfrlen = auchars.ptrmodel.strlen = modellen;
	auchars.ptrcmd.buffer = ptrcmd;
	auchars.ptrcmd.bfrlen = auchars.ptrcmd.strlen = ptrcmdlen;
	auchars.splname.buffer = splname;
	auchars.splname.bfrlen = auchars.splname.strlen = strlen(splname);

	debugprint("TYPE=%s UNIT=%d USBGPORT=0x%X USBADDR=%d USBSPEC=%04X "
			"USBSPEED=%s", auchars.type.value, auchars.unit.value,
			auchars.gport.value, auchars.addr.value, auchars.spec.value,
			(char *)&auchars.speed.value);
	debugprint("USBEP0MX=%d USBEPIN=%d USBPKTMX=%d USBEPOUT=%d USBPKTMX=%d",
			auchars.ep0max.value, auchars.epin.value, auchars.inmax.value,
			auchars.epout.value, auchars.outmax.value);
	debugprint("USBVEN=%04X USBDEV=%04X USBREL=%04X", auchars.ven.value,
			auchars.dev.value, auchars.rel.value);
	debugprint("USBMNFTR=%s", (char __near *)auchars.usbmnftr.buffer);
	debugprint("USBPROD=%s", (char __near *)auchars.usbprod.buffer);
	debugprint("USBSERNM=%s", (char __near *)auchars.usbsernm.buffer);
	debugprint("PTRMNFTR=%s", (char __near *)auchars.ptrmnftr.buffer);
	debugprint("PTRMODEL=%s", (char __near *)auchars.ptrmodel.buffer);
	debugprint("PTRCMD=%s", (char __near *)auchars.ptrcmd.buffer);
	debugprint("SPLNAME=%s", (char __near *)auchars.splname.buffer);

	if ((rtn = svcIoQueue(&auqab)) < 0 || (rtn = auqab.error) < 0)
	{
		logerror(rtn, "Error creating type A USB printer PTR%d:",
				auchars.unit.value);
		return (rtn);
	}
	sprintf(devname, "PTRA: PTR%d:", auchars.unit.value);
	if ((rtn = addtotext(&ifd->devlst, devname)) < 0 ||
			(rtn = addtotext(&ifd->devtxt, model)) < 0)
		return (rtn);
	logerror(0, "Printer (type A) created at %08X: %s %s", ddb->gport,
			ifd->devlst, ddb->devtxt);

	// Here with the USB printer device created. Now make sure the SPLCLS
	//   class driver is loaded. (The SPL device for this printer will be
	//   created by PTRSRV.)

	critBegin(&splclscrit);
    if (!checklke("SPLCLS"))
    {
		debugprint("loading SPLCLS");

		if (lkeloadf(0x01, "SPLCLS.LKE", NULL) != 0)
		{
			critEnd(&splclscrit);
			logerror(0, "Could not load SPLCLS.LKE. Spooling will not be set "
					"up for PTR%d: (%s)", auchars.unit.value, splname);
			return (0);
		}
    }
	critEnd(&splclscrit);

	debugprint("SPLCLS is loaded");

	// Create a PTRSRV instance to handle this printer. PTRSRV will add the
	//   corresponding SPL class device. (This version requires that PTRSRV
	//   is running. It must be started, probably in USTARTUP.BAT, before
	//   this code is executed.)

	// NOTE: We need to add code here to look up the printer's serial number
	//       in a printer definition file to see if a specific name has been
	//       assigned to it. This should also tell us if this should be the
	//       default printer (PTR:).

	rtn = sprintf(ptrcmd, STR_MT_SRVCMD"$ADD %s PTR%d:", splname,
			auchars.unit.value);
	if ((rtn = servercmd("PTRSRV", "SYS^PTRSRV^1", ptrcmd, rtn)) < 0)
	{
		logerror(rtn, "Could not create PTRSRV instance for PTR%d (%s) - "
				"Spooling will not be set up", auchars.unit.value, splname);
		return (0);
	}
	return (0);
}


//***********************************************************************
// Function: setupname - Construct printer spool name from the MDL string
// Returned: Nothing
//***********************************************************************

static void setupname(
	char *name,
	char *prefix,
	char *text)

{
	int  cnt;
	char chr;

	debugprint("in setupname: %s", text);

	name = strmov(name, prefix);		// Put in the prefix
	cnt = 6;
	while (--cnt >= 0 && (chr = *text++) != 0 && !isspace(chr))
		*name++ = chr;
	*name = 0;			
}
