//===============================================
// dska.c Functions dealing with type A USB disks
// Written by John Goltz
//===============================================

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

// Type A USB disks use the bulk-transfer only protocol (protocol = 0x50)
//   with either the RCB (subclass = 1), CD/DVD (subclass = 2) or the SCSI
//   (subclass = 6) command set.  This module ensures that the USBDSKALNK
//   USB disk client is loaded and uses the addunit class function to
//   connect a disk unit to the USB device.

typedef _Packed struct
{	TEXT8CHAR  type;
	BYTE4CHAR  unit;
	BYTE4CHAR  baseunit;
	BYTE4CHAR  logunit;
	uchar      end;
} AUCHARSA;


static CRIT dskclscrit = {'CRIT'};
static CRIT scsisupcrit = {'CRIT'};
static CRIT usbdskalnkcrit = {'CRIT'};

static char classname[] = "DISK:";	// Class name

static long adddskunit(CDB *cdb, DDB *ddb, IFD *ifd, void *auchars, int unit,
		int lun);
static long loaddsklke(char *name);


//***************************************************
// Function: dskasetup - Set up a type A USB disk
// Returned: 0 if normal, negative XOS error if error
//***************************************************

long dskasetup(
	CDB *cdb,
	DDB *ddb,
	IFD *ifd)
{
	_Packed struct
	{	TEXT8CHAR  type;
		BYTE4CHAR  unit;
		BYTE4CHAR  gport;
		BYTE4CHAR  addr;
		BYTE4CHAR  usbspec;
		BYTE4CHAR  speed;
		BYTE4CHAR  ep0max;
		BYTE4CHAR  epin;
		BYTE4CHAR  inmax;
		BYTE4CHAR  epout;
		BYTE4CHAR  outmax;
		BYTE4CHAR  usbven;
		BYTE4CHAR  usbdev;
		BYTE4CHAR  usbrel;
		LNGSTRCHAR usbmnftr;
		LNGSTRCHAR usbprod;
		LNGSTRCHAR usbsernm;
		uchar      end;
	} auchars0 =
	{	{PAR_SET|REP_TEXT, 8, "TYPE", "UDKA"},
		{PAR_SET|REP_DECV, 4, "UNIT"},
		{PAR_SET|REP_DECV, 4, "USBGPORT"},
		{PAR_SET|REP_DECV, 4, "USBADDR"},
		{PAR_SET|REP_DECV, 4, "USBSPEC"},
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
	};
	AUCHARSA aucharsa =
	{	{PAR_SET|REP_TEXT, 8, "TYPE", "UDKA"},
		{PAR_SET|REP_DECV, 4, "UNIT"},
		{PAR_SET|REP_DECV, 4, "BASEUNIT"},
		{PAR_SET|REP_DECV, 4, "LOGUNIT"},
	};
	_Packed struct
	{	BYTE4CHAR numluns;
		uchar      end;
	} lunchars =
	{	{PAR_GET|REP_DECV, 4, "NUMLUNS"}
	};

	EPT *ept;
	long rtn;
	long hndl;
	int  cnt;

	debugprint("in dskasetup, lport=%d gport=0x%X", ddb->lport, ddb->gport);
	auchars0.epin.value = 0;
	auchars0.epout.value = 0;
	ept = ddb->curcfg->fifd->fept;
	while (ept != NULL && (auchars0.epin.value == 0 ||
			auchars0.epout.value == 0))
	{
		if (ept->attrib == 0x02)
		{
			if ((ept->number & 0x80) && ept->number <= 0x8F)
			{
				if (auchars0.epin.value == 0)
				{
					auchars0.epin.value = ept->number & 0x0F;
					auchars0.inmax.value = ept->maxpktsz;
				}

			}
			else if (ept->number <= 0x07)
			{
				if (auchars0.epout.value == 0)
				{
					auchars0.epout.value = ept->number;
					auchars0.outmax.value = ept->maxpktsz;
				}
			}
		}
		ept = ept->next;
	}
	if (auchars0.epin.value == 0 || auchars0.epout.value == 0)
	{
		logerror(0, "Invalid DSKA configuration: No bulk transfer endpoint(s) "
				"specified");
		return (ER_USBBC);
	}

	debugprint("have type A disk at %d on USB%d:", ddb->addr, cdb->unit);
	debugprint("epin = %d, epout = %d", auchars0.epin.value,
			auchars0.epout.value);

	// Get here when it looks like a valid type A disk device. Load the disk
	//   link driver if its not already loaded. First we have to make sure the
	//   SCSI support functions are loaded (these are used by the disk link
	//   driver)

	critBegin(&dskclscrit);
	rtn = loaddsklke("DSKCLS");
	critEnd(&dskclscrit);
	if (rtn < 0)
		return (rtn);
	critBegin(&scsisupcrit);
	rtn = loaddsklke("SCSISUP");
	critEnd(&scsisupcrit);
	if (rtn < 0)
		return (rtn);
	critBegin(&usbdskalnkcrit);
	rtn = loaddsklke("USBDSKALNK");
	critEnd(&usbdskalnkcrit);
	if (rtn < 0)
		return (rtn);

	// Here with our drivers loaded - Finish setting up the addunit arguement
	//   block

	//  First see if we can have another USB disk unit

	if ((auchars0.unit.value = 8 * allocunitnumber("D") + 8) < 0)
	{
		logerror(ER_TMDDV, "Can not create USB type A disk unit");
		return (ER_TMDDV);
	}

	debugprint("allocunitnumber returned %d", auchars0.unit.value);

	strcpy(ifd->clss, "DISK");
	strcpy(ifd->name, "D");

	ifd->allocofs = 8;
	ifd->allocfact = 8;

	ifd->unit = auchars0.unit.value;

	// OK - create the device

	auchars0.gport.value = ddb->gport;
	auchars0.addr.value = ddb->addr;
	auchars0.speed.value = (ddb->speed == USB_XFERSPEED_HIGH) ? 'H' :
			(ddb->speed == USB_XFERSPEED_FULL) ? 'F' : 'L';
	auchars0.ep0max.value = ddb->maxpktsz0;
	auchars0.usbspec.value = ddb->usbspec;
	auchars0.usbven.value = ddb->usbven;
	auchars0.usbdev.value = ddb->usbdev;
	auchars0.usbrel.value = ddb->usbrel;

	auchars0.usbmnftr.buffer = ddb->usbmnftr; // Manufacturer string
	auchars0.usbmnftr.bfrlen = auchars0.usbmnftr.strlen = strlen(ddb->usbmnftr);
	auchars0.usbprod.buffer = ddb->usbprod; // Product (model) string
	auchars0.usbprod.bfrlen = auchars0.usbprod.strlen = strlen(ddb->usbprod);
	auchars0.usbsernm.buffer = ddb->usbsernm; // Serial number string
	auchars0.usbsernm.bfrlen = auchars0.usbsernm.strlen = strlen(ddb->usbsernm);

	debugprint("TYPE=%s UNIT=%d USBGPORT=0x%X USBPSEC=%04X USBADDR=%d "
			"USBSPEED=%.4s", auchars0.type.value, auchars0.unit.value,
			auchars0.gport.value, auchars0.usbspec.value, auchars0.addr.value,
			(char *)&auchars0.speed.value);
	debugprint("USBMNFTR = %s", (char __near *)auchars0.usbmnftr.buffer);
	debugprint("USBPROD = %s", (char __near *)auchars0.usbprod.buffer);
	debugprint("USBSERNM = %s", (char __near *)auchars0.usbsernm.buffer);
	debugprint("USBEP0MX=%d USBEPIN=%d USBPKTMX=%d USBEPOUT=%d USBPKTMX=%d",
			auchars0.ep0max.value, auchars0.epin.value, auchars0.inmax.value,
			auchars0.epout.value, auchars0.outmax.value);
	debugprint("USBVEN=%04X USBDEV=%04X USBREL=%04X", auchars0.usbven.value,
			auchars0.usbdev.value, auchars0.usbrel.value);

	// Create the disk unit for LUN 0 (which always exists)

	if ((hndl = adddskunit(cdb, ddb, ifd, &auchars0, auchars0.unit.value,
			0)) < 0)
		return (hndl);

	// Here with the LUN 0 unit created - if we could open it mounted try
	//   opening it without mounting so we can get the number of LUNs.

	if (hndl == 0 && (hndl = svcIoOpen(XO_PHYS|XO_NOMOUNT, ifd->devlst,
			NULL)) < 0)
	{
		logerror(rtn, "Error getting number of logical units on newly "
				"created USB disk %s", ifd->devlst);
		return (rtn);
	}
	rtn = svcIoDevChar(hndl, (char *)&lunchars); // Get the number of LUNs
	svcIoClose(hndl, 0);
	if (rtn < 0)
	{
		logerror(rtn, "Error getting number of logical units on newly "
				"created USB disk %s", ifd->devlst);
		return (rtn);
	}

	debugprint("Number of LUNs: %d", lunchars.numluns.value);

	aucharsa.baseunit.value = aucharsa.unit.value = auchars0.unit.value;
///	auqab.buffer2 = (char *)&aucharsa;
	aucharsa.logunit.value = 0;
	cnt = lunchars.numluns.value;
	while (--cnt > 0)
	{
		aucharsa.unit.value++;
		aucharsa.logunit.value++;
		if ((rtn = adddskunit(cdb, ddb, ifd, &aucharsa, aucharsa.unit.value,
				aucharsa.logunit.value)) < 0)
			return (rtn);
	}
	if (ifd->desc != NULL)
		addtotext(&ifd->devtxt, ifd->desc);

	debugprint("DSKA setup finished");

	return (0);
}


static long adddskunit(
	CDB  *cdb,
	DDB  *ddb,
	IFD  *ifd,
	void *auchars,
	int   unit,
	int   lun)
{
	_Packed struct
	{	TEXT8CHAR unittype;
		uchar     end;
	} unitchars =
	{	{PAR_GET|REP_TEXT, 8, "UNITTYPE"}
	};
	QAB auqab =
	{
	    QFNC_WAIT|QFNC_CLASSFUNC,// func    - Function
	    0,						 // status  - Returned status
	    0,						 // error   - Error code
	    0,						 // amount  - Amount
	    0,						 // handle  - Device handle
	    0,						 // vector  - Vector for interrupt
	    {0},
		CF_ADDUNIT,				 // option  - Options or command
		0,						 // count   - Count
		classname,				 // buffer1 - Pointer to file spec
		NULL,					 // buffer2 - Unused
		NULL					 // parm    - Pointer to parameter area
	};

	long  rtn;
	long  hndl;
	int   cnt;
	int   retry;
	char  name[32];

	debugprint("Creating LUN %d", lun);

	auqab.buffer2 = auchars;
	retry = 5;
	while ((rtn = svcIoQueue(&auqab)) < 0 || (rtn = auqab.error) < 0)
	{
		logerror(rtn, "Error creating type A USB disk U%d: (LUN %d) at %08X",
				unit, lun, ddb->gport);

		if ((cnt = cdb->fncresetport((HDB *)cdb, ddb->lport)) < 0)
										// Reset the port (which resets the
										//   device)
			logerror(cnt, "Error resetting type A USB disk");
		return (rtn);
	}
	logerror(0, "Disk (type A) created at %08X: D%d (LUN %d) %s", ddb->gport,
			unit, lun, ddb->devtxt);
	sprintf(name, "%sD%d:", (ifd->devlst == NULL) ? "DSKA: " : "", unit);
	if ((rtn = addtotext(&ifd->devlst, name)) < 0)
		return (rtn);
	sprintf(name, "D%d:", unit);
	if ((hndl = svcIoOpen(XO_PHYS, name, NULL)) < 0)
	{
		if ((hndl | 0xFFFFF000) != ER_NTRDY)
			logerror(hndl, "Error accessing newly created USB disk %s", name);
		hndl = 0;
	}
	else
	{
		unitchars.unittype.value[0] = 0;
		svcIoDevChar(hndl, (char *)&unitchars); // Get the unit type

		debugprint("Unit type: %s", unitchars.unittype.value);

		if (*(long *)(unitchars.unittype.value) == 'CDRM')
			loaddsklke("CDRACLS");
		else
			loaddsklke("FFSCLS");
	}
	return (hndl);
}


static long loaddsklke(
	char *name)
{
    if (!checklke(name))
    {
		debugprint("loading LKE %s", name);

		strmov(strnmov(lkename, name, 58), ".LKE");
		if (lkeloadf(0x01, lkename, NULL) != 0)
		{
			logerror(0, "Could not load %s.LKE. The type A disk will "
					"not be available", name);
			return (ER_LKENA);
		}
		debugprint("LKE %s is loaded", name);
    }
	else
		debugprint("LKE %s is already loaded", name);
	return (0);
}
