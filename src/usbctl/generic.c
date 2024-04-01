//=====================================================
// generic.c Functions dealing with generic USB devices
// Written by John Goltz
//=====================================================

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

static CRIT genericcrit = {'CRIT'};

static void badconfig(DDB *ddb, IFD *ifd, DRV *drv, char *msg);
static void setauchar(BYTE4CHAR *pnt, char *name, int epnum, int pktsz);


//*****************************************************
// Function: genericsetup - Set up a generic USB device
// Returned: 0 if normal, negative XOS error if error
//*****************************************************

long genericsetup(
	CDB *cdb,
	DDB *ddb,
	IFD *ifd,
	DRV *drv)

{
	_Packed struct 
	{	TEXT8CHAR  type;
		BYTE4CHAR  unit;
		LNGSTRCHAR name;
		BYTE4CHAR  gport;
		BYTE4CHAR  addr;
		BYTE4CHAR  usbspec;
		BYTE4CHAR  speed;
		BYTE4CHAR  ep0max;
		BYTE4CHAR  usbven;
		BYTE4CHAR  usbdev;
		BYTE4CHAR  usbrel;
		LNGSTRCHAR usbmnftr;
		LNGSTRCHAR usbprod;
		LNGSTRCHAR usbsernm;
		BYTE4CHAR  endpnt;
		uchar      more[512 - 8 + 4];
	} auchars =
	{	{PAR_SET|REP_TEXT, 8, "TYPE"},
		{PAR_SET|REP_DECV, 4, "UNIT"},
		{PAR_SET|REP_STR , 0, "NAME"},
		{PAR_SET|REP_DECV, 4, "USBGPORT"},
		{PAR_SET|REP_DECV, 4, "USBADDR"},
		{PAR_SET|REP_DECV, 4, "USBSPEC"},
		{PAR_SET|REP_TEXT, 4, "USBSPEED"},
		{PAR_SET|REP_DECV, 4, "USBPKTMX"},
		{PAR_SET|REP_HEXV, 4, "USBVEN"},
		{PAR_SET|REP_HEXV, 4, "USBDEV"},
		{PAR_SET|REP_HEXV, 4, "USBREL"},
		{PAR_SET|REP_STR , 0, "USBMNFTR"},
		{PAR_SET|REP_STR , 0, "USBPROD"},
		{PAR_SET|REP_STR , 0, "USBSERNM"}
	};

	char classname[36];

	QAB auqab =
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
	    NULL,						// buffer1 - Pointer to file spec
		NULL,						// buffer2 - Unused
	    NULL						// parm    - Pointer to parameter area
	};
	EPT  *ept;
	char *name;
	BYTE4CHAR
	     *aupnt;
	long  rtn;
	int   inx;
	int   binum;
	int   bonum;
	int   iinum;
	int   ionum;
	char  devname[32];

	cdb = cdb;

	auqab.buffer1 = classname;
	auqab.buffer2 = (char *)&auchars;

///	debugprint("in genericsetup, lport=%d gport=0x%X", ddb->lport, ddb->gport);

	// Determine what endpoints are available

	binum = bonum = iinum = ionum = 0;
	ept = ddb->curcfg->fifd->fept;
	aupnt = &auchars.endpnt;
	while (ept != NULL)
	{
		if (ept->attrib == 0x02)
		{
			if (ept->number & 0x80)
			{
				if (ept->number <= 0x87 && ++binum <= drv->bimax)
				{
					setauchar(aupnt, "USBBIN", ept->number & 0x0F,
							ept->maxpktsz);
					aupnt += 2;
				}
			}
			else
			{
				if (ept->number <= 0x07 && ++bonum <= drv->bomax)
				{
					setauchar(aupnt, "USBBOUT", ept->number & 0x0F,
							ept->maxpktsz);
					aupnt += 2;
				}
			}
		}
		else if (ept->attrib == 0x03)
		{
			if (ept->number & 0x80)
			{
				if (ept->number <= 0x87 && ++iinum <= drv->iimax)
				{
					setauchar(aupnt, "USBIIN", ept->number & 0x0F,
							ept->maxpktsz);
					setauchar(aupnt + 2, "USBIIINV", ept->pktrate, -1);
					aupnt += 3;
				}
			}
			else
			{
				if (ept->number <= 0x07 && ++ionum <= drv->iomax)
				{
					setauchar(aupnt, "USBIOUT", ept->number & 0x0F,
							ept->maxpktsz);
					setauchar(aupnt + 2, "USBIOINV", ept->pktrate, -1);
					aupnt += 3;
				}
			}
		}
		ept = ept->next;
	}
	if (binum < drv->bimin)
	{
		badconfig(ddb, ifd, drv, "Too few bulk transfer input endpoints");
		return (ER_USBBC);
	}
	if (bonum < drv->bomin)
	{
		badconfig(ddb, ifd, drv, "Too few bulk transfer output endpoints");
		return (ER_USBBC);
	}
	if (iinum < drv->iimin)
	{
		badconfig(ddb, ifd, drv, "Too few interrupt input endpoints");
		return (ER_USBBC);
	}
	if (ionum < drv->iomin)
	{
		badconfig(ddb, ifd, drv, "Too few interrupt output endpoints");
		return (ER_USBBC);
	}
	aupnt->desp = 0;

	// Get here when we have determined we have the required endpoints. Now
	//   load the required drivers if they are not alreay loaded

	inx = 0;
	while (inx < 4 && drv->drvr[inx][0] != 0)
	{
		critBegin(&genericcrit);
	    if (!checklke(drv->drvr[inx]))
    	{
			debugprint("loading %s", drv->drvr[inx]);

			sprintf(lkename, "%s.LKE", drv->drvr[inx]);
			if (lkeloadf(0x01, lkename, NULL) != 0)
			{
				critEnd(&genericcrit);
				logerror(0, "Could not load %s.LKE. The %s device will not "
						"be available", drv->drvr[inx], drv->clss);
				return (ER_ERROR);
			}
	    }
		critEnd(&genericcrit);
		debugprint("%s is loaded", drv->drvr[inx]);
		inx++;
	}

	// Here with all required drivers loaded - Finish setting up the addunit arguement
	//   block

	name = (drv->name[0] != 0) ? drv->name : "BULK";
	auchars.name.buffer = name;
	auchars.name.bfrlen = auchars.name.strlen = strlen(name);

	// Get another device unit if we can

	if ((auchars.unit.value = allocunitnumber(name)) < 0)
	{
		logerror(ER_TMDDV, "Too many units for device name %s", name);
		return (ER_TMDDV);
	}
	strcpy(ifd->clss, drv->clss);
	strcpy(ifd->name, name);
	ifd->allocofs = 0;
	ifd->allocfact = 1;
	ifd->unit = auchars.unit.value;

	// OK - create the device

	auchars.gport.value = ddb->gport;
	auchars.addr.value = ddb->addr;
	auchars.speed.value = (ddb->speed == USB_XFERSPEED_HIGH) ? 'H' :
			(ddb->speed == USB_XFERSPEED_FULL) ? 'F' : 'L';
	auchars.ep0max.value = ddb->maxpktsz0;
	auchars.usbspec.value = ddb->usbspec;
	auchars.usbven.value = ddb->usbven;
	auchars.usbdev.value = ddb->usbdev;
	auchars.usbrel.value = ddb->usbrel;

	auchars.usbmnftr.buffer = ddb->usbmnftr; // Manufacturer string
	auchars.usbmnftr.bfrlen = auchars.usbmnftr.strlen = strlen(ddb->usbmnftr);
	auchars.usbprod.buffer = ddb->usbprod; // Product (model) string
	auchars.usbprod.bfrlen = auchars.usbprod.strlen = strlen(ddb->usbprod);
	auchars.usbsernm.buffer = ddb->usbsernm; // Serial number string
	auchars.usbsernm.bfrlen = auchars.usbsernm.strlen = strlen(ddb->usbsernm);

	debugprint("TYPE=%s UNIT=%d USBGPORT=0x%X USBPSEC=%04X USBADDR=%d "
			"USBSPEED=%.4s", auchars.type.value, auchars.unit.value,
			auchars.gport.value, auchars.usbspec.value, auchars.addr.value,
			(char *)&auchars.speed.value);
	debugprint("USBMNFTR = %s", (char __near *)auchars.usbmnftr.buffer);
	debugprint("USBPROD = %s", (char __near *)auchars.usbprod.buffer);
	debugprint("USBSERNM = %s", (char __near *)auchars.usbsernm.buffer);
	debugprint("USBEP0MX=%d", auchars.ep0max.value);
	debugprint("USBVEN=%04X USBDEV=%04X USBREL=%04X", auchars.usbven.value,
			auchars.usbdev.value, auchars.usbrel.value);

	// Create the device unit

	sprintf(classname, "%s:", drv->clss);
	strncpy(auchars.type.value, drv->dtype, 8);

///	BREAK();

	auqab.buffer2 = (char *)&auchars;
	if ((rtn = svcIoQueue(&auqab)) < 0 || (rtn = auqab.error) < 0)
	{
		giveunitnumber(drv->name, auchars.unit.value);
		logerror(rtn, "Error creating unit %d for class %s on USB global "
				"port %08X", auchars.unit.value,  drv->clss,
				auchars.gport.value);
		return (rtn);
	}
	sprintf(devname, "GENA: %s%d:", name, auchars.unit.value);
	if ((rtn = addtotext(&ifd->devlst, devname)) < 0)
		return (rtn);
	logerror(0, "Generic bulk transfer device created at %08X: %s %s",
			ddb->gport, devname + 6, ddb->devtxt);
	return (0);
}


static void badconfig(
	DDB *ddb,
	IFD *ifd,
	DRV *drv,
	char *msg)
{
	int  len;
	char text[200];

	len = sprintf(text, "Invalid %s configuration: %s", drv->clss, msg);
	addtotext(&ifd->devlst, text);
	sprintf(text + len, " at %08X-%d", ddb->gport, ifd->number);
	logerror(ER_USBBC, text);
}


static void setauchar(
	BYTE4CHAR *pnt,
	char      *name,
	int        epnum,
	int        pktsz)

{
	char *cp;
	int   cnt;
	char  chr;

	debugprint("Addunit char: %s %d %d", name, epnum, pktsz);

	pnt->desp = PAR_SET|REP_DECV;
	pnt->size = 4;
	cp = pnt->name;
	cnt = 8;
	do
	{
		chr = *name++;
		*cp++ = chr;
	} while (--cnt > 0 && chr != 0);
	while (--cnt >= 0)
		*cp++ = 0;
	pnt->value = epnum;
	if (pktsz > 0)
	{
		pnt++;
		pnt->desp = PAR_SET|REP_DECV;
		pnt->size = 4;
		memcpy(pnt->name, "USBPKTMX", 8);
		pnt->value = pktsz;
	}
}
