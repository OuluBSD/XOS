//==============================================================
// hidboota.c Functions dealing with type A USB HID boot devices
// Written by John Goltz
//==============================================================

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

// The type A USB boot device drivers support USB keyboards (class =3,
//   subclass = 1, protocol = 1) and USB mouse devices (class =3,
//   subclass = 1, protocol = 2) using the boot protocol.

// This link drivers link the standard XOS boot device classes to USB
//   devices.

// The keyboard class uses a single set of device names beginning with KBD
//   followed by the unit number. Even though there are currently no non-USB
//   keyboards (other than the actual console keyboard, which is a special
//   case) we still start the unit numbers at 50 to be consistent with the
//   printer class.

// The mouse class uses a single set of device names beginning with MOUSEP
//   followed by the unit number. To avoid conflicts with non-USB mouse
//   devices, we start the unit numbers at 50 to be consistent with the
//   other USB classes.

static CRIT lke1crit = {'CRIT'};
static CRIT lke2crit = {'CRIT'};

static char trmname[16] = "TRM0";


static int bootlke(DDB *ddb, IFD *ifd, char *lke);


//******************************************************
// Function: hidbootsetup - Set up a type A USB keyboard
// Returned: 0 if normal, negative XOS error if error
//******************************************************

long hidbootsetup(
	CDB  *cdb,
	DDB  *ddb,
	IFD  *ifd,
	char *classname,
	char *devbase,
	char *devtype,
	char *lke1,
	char *lke2)
{
	_Packed struct
	{	BYTE4CHAR  unit;
		TEXT8CHAR  type;
		LNGSTRCHAR trm;
		BYTE4CHAR  gport;
		BYTE4CHAR  addr;
		BYTE4CHAR  spec;
		BYTE4CHAR  speed;
		BYTE4CHAR  intrf;
		BYTE4CHAR  ep0max;
		BYTE4CHAR  epiin;
		BYTE4CHAR  iinmax;
		BYTE4CHAR  iinrate;
		BYTE4CHAR  usbven;
		BYTE4CHAR  usbdev;
		BYTE4CHAR  usbrel;
		LNGSTRCHAR usbmnftr;
		LNGSTRCHAR usbprod;
		LNGSTRCHAR usbsernm;
		uchar      end;
	} auchars =
	{	{PAR_SET|REP_DECV, 4, "UNIT"   , 0},
		{PAR_SET|REP_TEXT, 8, "TYPE"   , "KBUA"},
		{PAR_SET|REP_STR , 0, "DISPLAY", trmname, 20},
		{PAR_SET|REP_DECV, 4, "USBGPORT"},
		{PAR_SET|REP_DECV, 4, "USBADDR"},
		{PAR_SET|REP_HEXV, 4, "USBSPEC"},
		{PAR_SET|REP_TEXT, 4, "USBSPEED"},
		{PAR_SET|REP_DECV, 4, "USBINTRF"},
		{PAR_SET|REP_DECV, 4, "USBEP0MX"},
		{PAR_SET|REP_DECV, 4, "USBIIN"},
		{PAR_SET|REP_DECV, 4, "USBPKTMX"},
		{PAR_SET|REP_DECV, 4, "USBRATE"},
		{PAR_SET|REP_HEXV, 4, "USBVEN"},
		{PAR_SET|REP_HEXV, 4, "USBDEV"},
		{PAR_SET|REP_HEXV, 4, "USBREL"},
		{PAR_SET|REP_STR , 0, "USBMNFTR"},
		{PAR_SET|REP_STR , 0, "USBPROD"},
		{PAR_SET|REP_STR , 0, "USBSERNM"},
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
    	NULL,						// buffer1 - Pointer to file spec
    	NULL,						// buffer2 - Unused
    	NULL						// parm    - Pointer to parameter area
	};
	EPT  *ept;
	long  rtn;
	char  devid[600];
	char  devname[32];

	cdb = cdb;

	auqab.buffer2 = (char *)&auchars;

	debugprint("in hidbootsetup for %08X-%d", ddb->gport, ifd->number);

	memset(devid, 0, sizeof(devid));

	auchars.epiin.value = 0;
	ept = ifd->fept;
	while (ept != NULL && auchars.epiin.value == 0)
	{
		if (ept->attrib == 0x03)
		{
			if ((ept->number & 0x80) && ept->number <= 0x8F)
			{
				if (auchars.epiin.value == 0)
				{
					auchars.epiin.value = ept->number & 0x0F;
					auchars.iinmax.value = ept->maxpktsz;
					auchars.iinrate.value = ept->pktrate;
				}
			}
		}
		ept = ept->next;
	}
	if (auchars.epiin.value == 0)
	{
		logerror(ER_USBBC, "Invalid %s configuration: Required interrupt input "
				"endpoint missing", devtype);
		return (ER_USBBC);
	}

	debugprint("have type A %s boot device at %08X-%d", devtype, ddb->gport,
			ifd->number);
	debugprint("epintin = %d", auchars.epiin.value);

	// Get here when it looks like a valid type A boot device. Load the
	//   necessary drivers if they are not already loaded

	critBegin(&lke1crit);
	rtn = bootlke(ddb, ifd, lke1);
	critEnd(&lke1crit);
	if (rtn < 0)
		return (rtn);
	critBegin(&lke2crit);
	rtn = bootlke(ddb, ifd, lke2);
	critEnd(&lke2crit);
	if (rtn < 0)
		return (rtn);

	// Here with our driver loaded - Finish setting up the addunit arguement
	//   block

	//  First see if we can have another USB unit for the boot device

	if ((auchars.unit.value = allocunitnumber(devbase)) < 0)
	{
		logerror(ER_TMDDV, "Can not create additional USB type A %s boot "
				"device unit at %08X-%d", ddb->gport, ifd->number);
		return (ER_TMDDV);
	}
	strcpy(ifd->clss, classname);
	strcpy(ifd->name, devbase);
	auchars.unit.value += 50;
	ifd->allocofs = 50;
	ifd->allocfact = 1;
	ifd->unit = auchars.unit.value;

	// OK - Try to set idle mode but go on if can't (This should not have
	//   much effect.)

	if ((rtn = dosetup(cdb, ddb, 64, ddb->speed, ddb->addr, 0, SETUP_CLASS |
			SETUP_INTERFACE, TYPE_SET_IDLE, 0, ifd->number, NULL, 0)) < 0)
	{
		logerror(rtn, "Error setting idle mode for type A USB "
				"boot device %s%d:", devbase, auchars.unit.value);
	}

	// Try to set boot protocol but go on if can't (This could keep it from
	//   from working, but it might be OK!)

	if ((rtn = dosetup(cdb, ddb, 64, ddb->speed, ddb->addr, 0, SETUP_CLASS |
			SETUP_INTERFACE, TYPE_SET_PROTOCOL, 0, ifd->number, NULL, 0)) < 0)
	{
		logerror(rtn, "Error setting to use boot protocol for type A USB "
				"boot device %s%d:", devbase, auchars.unit.value);
	}

	// Create the device

	auqab.buffer1 = classname;
	*(long *)auchars.type.value = *(long *)devtype;
	auchars.gport.value = ddb->gport;
	auchars.addr.value = ddb->addr;
	auchars.spec.value = ddb->usbspec;
	auchars.speed.value = (ddb->speed == USB_XFERSPEED_HIGH) ? 'H' :
			(ddb->speed == USB_XFERSPEED_FULL) ? 'F' : 'L';
	auchars.intrf.value = ifd->number;
	auchars.ep0max.value = ddb->maxpktsz0;
	auchars.usbven.value = ddb->usbven;
	auchars.usbdev.value = ddb->usbdev;
	auchars.usbrel.value = ddb->usbrel;
	auchars.usbmnftr.buffer = ddb->usbmnftr; // Manufactor string
	auchars.usbmnftr.bfrlen = auchars.usbmnftr.strlen = strlen(ddb->usbmnftr);
	auchars.usbprod.buffer = ddb->usbprod; // Device string
	auchars.usbprod.bfrlen = auchars.usbprod.strlen = strlen(ddb->usbprod);
	auchars.usbsernm.buffer = ddb->usbsernm; // Serial number string
	auchars.usbsernm.bfrlen = auchars.usbsernm.strlen = strlen(ddb->usbsernm);

	debugprint("TYPE=%s UNIT=%d USBGPORT=0x%X USBADDR=%d USBSPEC=%04X "
			"USBSPEED=%.4s USBINTRF=%d", auchars.type.value, auchars.unit.value,
			auchars.gport.value, auchars.addr.value, auchars.spec.value,
			(char *)&auchars.speed.value, auchars.intrf.value);
	debugprint("USBMNFTR = %s", (char __near *)auchars.usbmnftr.buffer);
	debugprint("USBPROD = %s", (char __near *)auchars.usbprod.buffer);
	debugprint("USBSERNM = %s", (char __near *)auchars.usbsernm.buffer);
	debugprint("USBEP0MX=%d USBEPIN=%d USBPKTMX=%d",
			auchars.ep0max.value, auchars.epiin.value, auchars.iinmax.value);
	if ((rtn = svcIoQueue(&auqab)) < 0 || (rtn = auqab.error) < 0)
	{
		logerror(rtn, "Error creating type A USB boot device %s%d:", devbase,
				auchars.unit.value);
		return (rtn);
	}
	sprintf(devname, "HBTA: %s%d:", devbase, auchars.unit.value);
	if ((rtn = addtotext(&ifd->devlst, devname)) < 0)
		return (rtn);
	logerror(0, "%s boot device %s (type A) created at %08X-%d:\n%s %s",
			devtype, devname + 6, ddb->gport, ifd->number, ifd->devlst,
			ddb->devtxt);
	return (0);
}


static int bootlke(
	DDB  *ddb,
	IFD  *ifd,
	char *lke)
{
    if (!checklke(lke))
    {
		debugprint("loading %s", lke);

		strmov(strmov(lkename, lke), ".LKE");
		if (lkeloadf(0x01, lkename, NULL) != 0)
		{
			logerror(ER_USBBC, "Could not load LKE %s.\nThe type A boot "
					"device at %08X-%d will not be available", lke, ddb->gport,
					ifd->number);
			return (ER_USBBC);
		}
	}
	debugprint("%s is loaded", lke);
	return (0);
}
