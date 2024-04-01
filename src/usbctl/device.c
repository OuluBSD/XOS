//============================================
// DEVICE.C
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

#define MAXSTRLEN 64

// The data blocks created here mostly correspond one-to-one to the USB
//   descriptors as follows:
//                               +-------------+
//                               | Device Desc |               DDB
//                               +-------------+
//                                      |
//                          +-----------+-----------+
//                          |                       |
//                   +-------------+         +-------------+
//                   | Config Desc |         | Config Desc |   CFG
//                   +-------------+         +-------------+
//                          |                       |
//             +------------+------------+        .....
//             |                         |
//     +----------------+        +----------------+
//     | Interface Desc |        | Interface Desc |            IFD
//     +----------------+        +----------------+
//             |                         |
//       +-----+-----+             +-----+-----+
//       |           |             |           |  
//  +---------+ +---------+   +---------+ +---------+
//  | EP Desc | | EP Desc |   | EP Desc | | EP Desc |          EPT
//  +---------+ +---------+   +---------+ +---------+

// A unit of an XOS device that impliments a partictular USB device usually
//   corresponds to an Interface in a device. Different interfaces in a device
//   may also impliment completely different devices. In some cases a single
//   interface may impliment multiple device units through the use of higher
//   level multiplexing protocols, notibly the disk block-transfer only
//   protocol which impliments SCSI style logical units.

// At the kernel level, device drivers do not know anything about interfaces
//   as such. They simply impliment the use of specified end points to
//   transfer data for the device. The end points used are specified here
//   based on the descriptors read from the device.

// The database described has nothing to do with the operation of the USB
//   devices, other than initially assigning end points to the various XOS
//   devices which support various USB devices. The data is kept only to
//   allow detailed reporting of the overall USB configuration at any time.
//   Once an XOS USB device driver is installed and unit added, it has no
//   further contact with this program and will continue to operate normally
//   even if this program terminates. It should be noted, however, that this
//   version of USBCTL is NOT designed to be termianted and restarted. Doing
//   so will most likely result in failures mounting newly connected devices
//   because of name conflicts.

// These data structures are only modified by the thread which controls the
//   root port the device is connected to. They are also read by the base
//   thread when processing a status request. Adding items is alwaysdone in
//   a safe order and is not interlocked. Exclusive access is obtained when
//   removing items and when the structure is being accessed by the base
//   thread. 

// A seperate but related set of data structures is used to manage hubs.
//   Each root port has CDB (Controller Data Block) which contains a table
//   pointing to DCBs for all devices (including hubs) directly connected
//   to the controller. Each controller also has a DCB which is mostly a
//   dummy but makes the structure uniform. Each hub also has an HDB (Hub
//   Data Block). This is similiar to a CDB and shares a common initial part.
//   Each HDB contains a table of DDBs for each device directly connected to
//   it. This may be nested to 7 levels.


static char *getstring(CDB *cdb, DDB *ddb, int lport, int pktsz0, int addr,
		int speed, int index);
static char *iovalue(long value);


//*****************************************************
// Function: setupdevice - Set up a USB device for use
// Returned: Nothing
//*****************************************************

// This function attempts to enumerate and initialize a USB device. If it is
//   successfull it generates the full configuration tree described above.
//   It usually generates at least a DDB. If an error occures and the device
//   is unusable, the "text" item in the DDB reflects this. The one exception
//   to this is that if an LS or FS device is found on an HS interface (error
//   returned when enabling the port is ER_PDTYP) nothing is allocated.

void setupdevice(
	HDB *hdb,					// HDB for hub or CDB for controller device
								//   is connected to
	long lport,					// Local port number
	int  speed)					// Device speed
{
	union
	{	uchar   *c;
		DEVDESC *dev;
		CFGDESC *cfg;
		IFDESC  *ifd;
		EPDESC  *epd;
		HIDDESC *hid;
	}        cpnt;
	uchar   *dpnt;
	uchar   *cdesc;
	CDB     *cdb;
	DDB     *ddb;
	DDB     *hddb;
	CFG     *cfg;
	IFD     *ifd;
	EPT     *ept;
	CFG    **cfgpnt;
	IFD    **ifdpnt;
	IFD    **ifdaltpnt;
	EPT    **eptpnt;
	HIDLIST *dlpnt;
	DRV      drv;
	long     rtn;
	int      len;
	int      cnt;
	int      hcnt;
	int      type;
	int      num;
	int      size;
	int      cfgcnt;
	int      ifccnt;
	int      eptcnt;
	int      address;
	int      index;
	int      cfglen;
	int      pktsz0;
	int      prevnumber;
	int      prevaltsel;
	union
	{	char    c[128];
		DEVDESC d;
	}        iobufr;

	hddb = hdb->ddb;
	cdb = hddb->rootcdb;

	debugprint("in setupdevice, resetting port for %08X", (lport << 4) +
			cdb->unit);

	if ((rtn = (hdb->fncresetport)(hdb, lport)) < 0)
	{
		if (rtn == ER_USBNH)
		{
			debugprint("Not a HS device at %08X", cdb->unit + (lport << 4));
			cdb->portsts[lport] = 0; 	// Make it visible to the FS/LS
		}								//   controller
		else
			logerror(rtn, "Error resetting port at %08X", (lport << 4) +
					cdb->unit);
		return;
	}

	debugprint("port reset complete for %08x:", (lport << 4) + cdb->unit);

	if ((rtn = makeddb(hdb, lport, &ddb)) < 0)
	{
		logerror(rtn, "Error creating DDB for %08X", (lport << 4) + cdb->unit);
		return;
	}
	ddb->rootcdb = cdb;

///	fbrCtlWait(XT_SECOND * 5);

	// Assign an address to the device

	sprintf(iobufr.c, "*USB%d*", cdb->unit);
	if ((address = allocunitnumber(iobufr.c)) < 0)
	{
		logerror(ER_USBTA, "No unit number available for %08X", ddb->gport);
		return;
	}
	address++;
	ddb->addr = address;

	debugprint("using USB address %d for %08X", address, ddb->gport);

	if ((rtn = dosetup(cdb, ddb, 8, speed, 0, 0, SETUP_STANDARD | SETUP_DEVICE,
			TYPE_SET_ADDRESS, address, 0, NULL, 0)) < 0)
	{
		deverror(ddb, rtn, "Error setting USB device address");
		return;
	}

	debugprint("address set for %08X", ddb->gport);

	// Read the first 8 bytes of the device descriptor (we can do this
	//   independent of the EP0 packet size and it gives us the EP0 packet
	//   size for what follows

	if ((rtn = dosetup(cdb, ddb, 8, speed, address, 0, SETUP_INPUT |
			SETUP_STANDARD | SETUP_DEVICE, TYPE_GET_DESC, DESC_DEVICE << 8,
			0, iobufr.c, 8)) < 0)
	{
		deverror(ddb, rtn, "Error reading initial part of the device "
				"descriptor");
		return;
	}

	debugprint("have first 8 bytes of device descriptor for %08X",
			ddb->gport);
	dumpit(iobufr.c, 8);

	if (iobufr.c[1] != 1)
	{
		logerror(ER_USBDF, "Invalid device descriptor for %08X", ddb->gport);
		deverror(ddb, rtn, "Invalid device descriptor");
		return;
	}
	pktsz0 = iobufr.c[7];
	if (pktsz0 != 8 && pktsz0 != 16 && pktsz0 != 32 && pktsz0 != 64)
	{
		logerror(ER_USBDF, "Invalid EP 0 packet size for %08X", ddb->gport);
		deverror(ddb, rtn, "Invalid IP 0 packet size");
		return;
	}
	debugprint("EP0 packet size is %d for %08X", pktsz0, ddb->gport);

	// Read the entire device descriptor

	if ((rtn = dosetup(cdb, ddb, pktsz0, speed, address, 0, SETUP_INPUT |
			SETUP_STANDARD | SETUP_DEVICE, TYPE_GET_DESC, DESC_DEVICE << 8,
			0, iobufr.c, sizeof(DEVDESC))) < 0)
	{
		deverror(ddb, rtn, "Error reading device descriptor");
		return;
	}

	debugprint("have entire device descriptor (%d) for %08X", sizeof(DEVDESC),
			ddb->gport);
	dumpit(iobufr.c, sizeof(DEVDESC));

	// Store the basic device information

	ddb->speed = speed;
	ddb->maxpktsz0 = pktsz0;
	ddb->usbspec = iobufr.d.usbspec;
	ddb->usbven = iobufr.d.vendorid;
	ddb->usbdev = iobufr.d.deviceid;
	ddb->usbrel = iobufr.d.devrel;
	ddb->class = iobufr.d.class;
	ddb->subclass = iobufr.d.subclass;
	ddb->protocol = iobufr.d.protocol;
	ddb->numcfgs = cfgcnt = iobufr.d.numcfgs;
	ddb->fcfg = NULL;
	ddb->usbmnftr = getstring(cdb, ddb, lport, pktsz0, address, speed,
			iobufr.d.manufacturer);
	ddb->usbprod = getstring(cdb, ddb, lport, pktsz0, address, speed,
			iobufr.d.product);
	ddb->usbsernm = getstring(cdb, ddb, lport, pktsz0, address, speed,
			iobufr.d.serialnum);
	cfgpnt = &ddb->fcfg;

	debugprint("dev: Class:0x%02X Subclass:0x%02X Protocol:0x%02X Configs:%d",
			ddb->class, ddb->subclass, ddb->protocol, cfgcnt);

	index = 0;
	while (--cfgcnt >= 0)
	{
		// Get the total length of the configuration descriptors

		if ((rtn = dosetup(cdb, ddb, pktsz0, speed, address, 0, SETUP_INPUT |
				SETUP_STANDARD | SETUP_DEVICE, TYPE_GET_DESC,
				DESC_CONFIG << 8 + index, 0,  iobufr.c, 4)) < 0)
			return;

		dumpit(iobufr.c, 4);
		cfglen = *(ushort *)(iobufr.c + 2);
		if ((cdesc = (char *)usbmalloc(cfglen + 4)) == NULL)
		{
			logerror(ER_USBDF, "Error allocating memory for configuration "
					"descriptors for %08X", ddb->gport);
			deverror(ddb, ER_USBDF, "Error allocating memory for configuration "
					"descriptors");
			cleanddb(ddb);
			return;
		}

		debugprint("config descriptor length is %d for %08X", cfglen,
				ddb->gport);

		// Read the raw configuration descriptors

		if ((rtn = dosetup(cdb, ddb, pktsz0, speed, address, 0, SETUP_INPUT |
				SETUP_STANDARD | SETUP_DEVICE, TYPE_GET_DESC, DESC_CONFIG << 8,
				0, cdesc, cfglen)) < 0)
		{
			deverror(ddb, rtn, "Error reading configuration descriptors");
			return;
		}

		debugprint("have config descriptors for %08X", cfglen, ddb->gport);
		dumpit(cdesc, cfglen);

		// Parse the descriptors and construct our basic data that
		//   describes this device

		// First byte is the total length of the descriptor, second byte
		//   is the descriptor type

		cpnt.c = cdesc;
		cnt = cfglen;
		cfg = NULL;
		while (cnt >= 2)
		{
			len = cpnt.c[0];
			type = cpnt.c[1];
			switch (type)
			{
			 case DESC_CONFIG:
				if (cfg != NULL || len < sizeof(CFGDESC))
				{
					logerror(ER_USBDF, "Configuration descriptor is too short "
							"for %08X", ddb->gport);
					deverror(ddb, ER_USBDF, "Configuration descriptor is too "
							"short");
					free(cdesc);
					cleanddb(ddb);
					return;
				}
				if ((cfg = (CFG *)usbmalloc(sizeof(CFG))) == NULL)
				{
					logerror(-errno, "Error allocating memory for CFG for %08X",
							ddb->gport);
					deverror(ddb, -errno, "Error allocating memory for CFG");
					free(cdesc);
					cleanddb(ddb);
					return;
				}
				cfg->value = cpnt.cfg->cfgvalue;
				cfg->index = index;
				cfg->attrib = cpnt.cfg->attrib;
				cfg->maxpower = cpnt.cfg->maxpower * 2;
				cfg->numttlifd = ifccnt = cpnt.cfg->numifs;
				cfg->desc = getstring(cdb, ddb, lport, pktsz0, address, speed,
						cpnt.cfg->descindex);
				cfg->fifd = NULL;
				cfg->next = NULL;
				*cfgpnt = cfg;
				cfgpnt = &cfg->next;
				ifdpnt = &cfg->fifd;
				eptpnt = NULL;
				prevnumber = -1;
				prevaltsel = -1;

				debugprint("CFG: Value: %d", cfg->value);
				debugprint("CFG: # IFs: %d", cfg->numttlifd);

				break;

			 case DESC_INTERFACE:
				if (cfg == NULL || len < sizeof(IFDESC))
				{						// Must follow the CONFIG descriptor
					logerror(ER_USBDF, "Interface descriptor is out of order "
							"for %08X", ddb->gport);
					deverror(ddb, rtn, "Interface descriptor is out of order");
					free(cdesc);
					cleanddb(ddb);
					return;
				}

				// Verify that the interface number and alternate setting
				//   values are valid

				if ((cpnt.ifd->number == prevnumber &&
							cpnt.ifd->altsel <= prevaltsel) ||
						(cpnt.ifd->number != prevnumber  &&
							cpnt.ifd->altsel != 0) ||
						cpnt.ifd->number < prevnumber)
				{
					logerror(ER_USBDF, "Invalid interface descriptor for %08X",
							ddb->gport);
					deverror(ddb, ER_USBDF, "Invalid interface descriptor");
					free(cdesc);
					cleanddb(ddb);
					return;
				}

				// Now create an IFD - Note that although we allocate and
				//   partly link in alternate IFDs, this version has no
				//   provision for using anything but the default (first)
				//   alternate interface.

				if ((ifd = (IFD *)usbmalloc(sizeof(IFD))) == NULL)
				{
					logerror(-errno, "Error allocating memory for IFD for %08X",
							ddb->gport);
					deverror(ddb, -errno, "Error allocating memory for IFD");
					free(cdesc);
					cleanddb(ddb);
					return;
				}
				memset(ifd, 0, sizeof(IFD));
				ifd->label = 'IFD*';
				ifd->number = cpnt.ifd->number;
				ifd->seqnum = ifseqnum++;
				ifd->numept = eptcnt = cpnt.ifd->numept;
				ifd->altsel = cpnt.ifd->altsel;
				ifd->class = cpnt.ifd->class;
				ifd->subclass = cpnt.ifd->subclass;
				ifd->protocol = cpnt.ifd->protocol;
				ifd->desc = getstring(cdb, ddb, lport, pktsz0, address, speed,
						cpnt.ifd->descindex);
				eptpnt = &ifd->fept;

				if (ifd->number != prevnumber) // Is this an alternate IF?
				{
					*ifdpnt = ifd;		// No
					ifdpnt = &ifd->next;
				}
				else
					*ifdaltpnt = ifd;
				ifdaltpnt = &ifd->nexta;
				prevnumber = ifd->number;
				prevaltsel = ifd->altsel;

				debugprint("IF: number: %d select: %d class: 0x%02X subclass: "
						"0x%02X protocol: 0x%02X", ifd->number, ifd->altsel,
						ifd->class, ifd->subclass, ifd->protocol);
				debugprint("     # EPs: %d", ifd->numept);

				break;

			 case DESC_ENDPOINT:
				if (eptpnt == NULL || len < sizeof(EPDESC))
				{						// Must follow the INTERFACE descriptor
					logerror(ER_USBDF, "Endpoint descriptor is out of order "
							"for %08X", ddb->gport);
					deverror(ddb, ER_USBDF, "Endpoint descriptor is out of "
							"order");
					free(cdesc);
					cleanddb(ddb);
					return;
				}
				if ((ept = (EPT *)usbmalloc(sizeof(EPT))) == NULL)
				{
					logerror(-errno, "Error allocating memory for EPT for %08X",
							ddb->gport);
					free(cdesc);
					cleanddb(ddb);
					return;
				}
				ept->number = cpnt.epd->number;
				ept->attrib = cpnt.epd->attrib;
				ept->maxpktsz = cpnt.epd->maxpktsz;
				ept->pktrate = cpnt.epd->pktrate;
				ept->next = NULL;
				*eptpnt = ept;
				eptpnt = &ept->next;

				{
					static char *eptbl[] =
					{	"Control",
						"Control",
						"Isoc out",
						"Isoc In",
						"Bulk out",
						"Bulk in",
						"Int out",
						"Int in"
					};

					rtn = (ept->attrib & 0x03) << 1;
					if (ept->number & 0x80)
						rtn++;
					debugprint("EP: %2d %s Maxpkt: %d Interval: %d",
							ept->number & 0x0F, eptbl[rtn], ept->maxpktsz,
							ept->pktrate);
				}
				break;

			 case DESC_HID:
				if (ifd == NULL || ifd->class != CLASS_HID ||
						ifd->hiddesc != NULL)
					break;
				num = cpnt.hid->numdesc;
				if (num < 1 || len < (6 + 3 * num))
					break;				// Ignore this if too short
				size = 0;				// See how much space we need
				hcnt = num;
				dlpnt = cpnt.hid->desclist;
				while (--hcnt >= 0)
				{
					size += ((dlpnt->length + 7) & ~0x03);
					dlpnt++;
				}
				if ((dpnt = (uchar *)usbmalloc(size)) == NULL)
				{
					logerror(ER_USBDF, "Error allocating memory for HID "
							"report descriptors for %08X", ddb->gport);
					deverror(ddb, ER_USBDF, "Error allocating memory for HID "
							"report descriptors");
					free(cdesc);
					cleanddb(ddb);
					return;
				}
				hcnt = num;
				ifd->hiddesc = dpnt;
				dlpnt = cpnt.hid->desclist;
				while (--hcnt >= 0)
				{
					((ushort *)dpnt)[0] = dlpnt->type;
					((ushort *)dpnt)[1] = dlpnt->length;

					debugprint("reading HID descriptor");

					if ((rtn = dosetup(cdb, ddb, pktsz0, speed, address, 0,
							SETUP_INPUT | SETUP_STANDARD | SETUP_INTERFACE,
							TYPE_GET_DESC, dlpnt->type << 8, ifd->number,
							dpnt + 4, dlpnt->length)) < 0)
					{
						deverror(ddb, rtn, "Error reading HID descriptor");
						free(cdesc);
						cleanddb(ddb);
						return;
					}

					dumpit(dpnt + 4, dlpnt->length);

					if (dlpnt->type == 0x22)
					{
						if (debugflg)
						{
							debugprint("HID device");
							parsehid(dpnt +4, dlpnt->length);
						}
					}
					dpnt += (dlpnt->length + 4);
				}
				break;
			}
			cpnt.c += len;
			cnt -= len;
		}
		usbfree(cdesc);
		index++;
	}

	if (ddb->fcfg == NULL)
	{
		logerror(ER_USBBC, "No configuration descriptor returned for 0x08X",
				ddb->gport);
		deverror(ddb, rtn, "No configuration descriptor returned");
		return;
	}

	// Make the default configuration the current configuration

	ddb->curcfg = ddb->fcfg;
	ddb->fifd = ddb->curcfg->fifd;

	debugprint("cfg value is %d for %08X", ddb->curcfg->value, ddb->gport);

	if ((rtn = dosetup(cdb, ddb, 64, speed, address, 0, SETUP_OUTPUT |
			SETUP_STANDARD | SETUP_DEVICE, TYPE_SET_CONFIG,
			ddb->curcfg->value, 0, NULL,
			0)) < 0)
	{
		deverror(ddb, rtn, "Error setting active configuration");
		return;
	}
	snprintf(iobufr.c, sizeof(iobufr), "%s/%s/%s", ddb->usbprod, ddb->usbmnftr,
			ddb->usbsernm);
	addtotext(&ddb->devtxt, iobufr.c);
	debugprint("configuration set for %08X", ddb->gport);
	debugprint("USB: %04X VID: %04X DID: %04X REL: %04X", ddb->usbspec,
			ddb->usbven, ddb->usbdev, ddb->usbrel);
	debugprint("LPORT: %d GPORT: %X", ddb->lport, ddb->gport);

	// See if we have a set up defined for the VID/PID pair

	if (!findvendor(ddb->usbven, ddb->usbdev, &drv))
		drv.func[0] = 0;
	else
		debugprint("Found ven/dev: %s %s %s", drv.func, drv.clss, drv.drvr);

	// Try to create a device for each interface

	ifd = ddb->curcfg->fifd;
	while (ifd != NULL)
	{
		rtn = setupinterface(ddb, ifd, &drv);
		ifd = ifd->next;				// Advance to next interface
	}
}



long setupinterface(
	DDB *ddb,
	IFD *ifd,
	DRV *drv)
{
	long rtn;
	char text[128];

	// The 2.0 spec is somewhat vague about the case were the class/subclass
	//   protocol is specified in both the device descriptor and the interface
	//   descriptor. We assume that if the class value in the device descriptor
	//   is not 0, than all three values are to be taken from the device
	//   descriptor. If the class value in the device descriptor is 0, we take
	//   all three values from the interface descriptor.

	if (ifd->class == 0)
	{
		ifd->class = ddb->class;
		ifd->subclass = ddb->class;
		ifd->protocol = ddb->protocol;
	}

	// If we found a set up for the VID/PAR pair, use it

	if (drv->func[0] != 0)
	{
		if (strcmp(drv->func, "GENERIC") != 0)
		{
			logerror(ER_USBBC, "Unsupported set up function %s, device with "
					"VID/PID %04X/%04X will not be used", drv->func,
					ddb->usbven, ddb->usbdev, ddb->gport +
					(ddb->lport << ddb->portshift));
			removexosdevice(ddb->owner.hdb, ddb->lport);
			return (ER_USBBC);
		}
		else
			rtn = genericsetup(ddb->owner.cdb, ddb, ifd, drv);
	}
	else
	{
		// Here if no set up routine is associated with the VID/PID pair. Now
		//   see if there is a setup routine for the class for the device or
		//   for each interface (if no class is specified for the device).
		//   Only the default configuration is looked at here and only the
		//   default definition for each interface is looked at if no class
		//   is specified for the device.  

		switch (ifd->class)
		{
		 case 3:						// HID
			switch (ifd->subclass)
			{
			 case 0:					// Generic HID device
				goto notsup;

			 case 1:					// Boot interface
				switch (ifd->protocol)
				{
				 case 1:				// Keyboard
					rtn = hidbootsetup(ddb->owner.cdb, ddb, ifd, "KBD:", "KBD",
							"KBUA", "KBDCLS", "USBKBDALNK");
					break;

				 case 2:				// Mouse
					rtn = hidbootsetup(ddb->owner.cdb, ddb, ifd, "MOUSE:",
							"MOUSEP", "MSUA", "MOUSECLS", "USBMSEALNK");
					break;

				 default:
					goto notsup;
				}
				break;
			}
			break;

		 case 7:						// Printer
			if (ifd->subclass == 1)
			{
				switch (ifd->protocol)
				{
				 case 0x01:				// Unidirectional
				 case 0x02:				// Bidirectional
				 case 0x03:				// 1284.4 bidirectional
					rtn = ptrasetup(ddb->owner.cdb, ddb, ifd);
					break;

				 default:
					goto notsup;
				}
			}
			break;

		 case 8:						// Mass storage device
			switch (ifd->protocol)
			{
			 case 0x00:					// CBI with command completion interrupt
			 case 0x01:					// CBI without command completion
										//   interrupt
				goto notsup;

			 case 0x50:					// Bulk-only transport
				switch (ifd->subclass)
				{
				 case 0x01:				// RBC
				 case 0x06:				// SCSI
					rtn = dskasetup(ddb->owner.cdb, ddb, ifd);
					break;

				 case 0x02:				// ATAPI
					goto notsup;

				 case 0x03:				// QIC-157 (tape)
					goto notsup;

				 case 0x04:				// UFI (usually floppy disk)
					goto notsup;

				 case 0x05:				// SFF-8070i (usually floppy disk)
					goto notsup;
					}
				break;

			 default:
				goto notsup;
			}
			break;

		 case 9:						// Hub device
///			rtn = hubsetup(ddb->owner.cdb, ddb, ifd);
			goto notsup;

		 default:
		 notsup:
			sprintf(text, "Unsupported device %02X %02X %02X (%04X %04X %04X)",
					ifd->class, ifd->subclass, ifd->protocol,  ddb->usbven,
							ddb->usbdev, ddb->usbrel);
			addtotext(&ifd->devlst, text);
			sprintf(text, "Unsupported device %02X %02X %02X at %08X-%d",
					ifd->class, ifd->subclass, ifd->protocol, ddb->gport,
					ifd->number);
			logerror(ER_USBBC, "%s: %s", text, ddb->devtxt);
			rtn = ER_USBBC;
			break;
		}
	}
	ifd->error = rtn;
	return (rtn);
}


//************************************************************
// Function: makeddb - Make a DDB
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

long makeddb(
	HDB  *hdb,
	long  lport,
	DDB **pddb)
{
	DDB *ddb;

	if ((ddb = (DDB *)usbmalloc(sizeof(DDB))) == NULL)
		return (-errno);
	memset(ddb, 0, sizeof(DDB));
	ddb->label = 'DDB*';
	ddb->seqnum = dvseqnum++;
	ddb->owner.hdb = hdb;
	ddb->lport = lport;
	ddb->gport = hdb->ddb->gport + (lport << hdb->ddb->portshift);
	hdb->ddbtbl[lport] = ddb;
	*pddb = ddb;
	return (0);
}


//*****************************************************
// Function: removexosdevice - Remove an XOS device and
//				any child devices from the system
// Returned: 
//*****************************************************

void removexosdevice(
	HDB *hdb,
	long lport)

{
	DDB  *hddb;
	DDB  *devddb;
	DDB  *childddb;
	CDB  *rootcdb;
	HDB  *ourhdb;
	long  gport;
	long  cport;
	long  rtn;

	hddb = hdb->ddb;
	rootcdb = hddb->rootcdb;

	if ((devddb = hdb->ddbtbl[lport]) == NULL)
		return;							// Nothing to do here if no device!
	gport = devddb->gport;

	// First remove all child devices, if any

	if ((ourhdb = devddb->hdb) != NULL) // Is this a HUB?
	{
		// Here if this is a hub - Remove all child devices

		cport = 1;
		do
		{
			if ((childddb = ourhdb->ddbtbl[cport]) != NULL)
				removexosdevice(ourhdb, cport);
		} while (++cport <= 15);
	}
	hdb->fncdisableport(hdb, lport);	// Disable the port the device is
										//   connected to

	// Tell the client (if any) that this device is gone (This really should
	//   not return an error except possibly ER_USBCN is there is no client.
	//   If it does we just log it.

	if ((rtn = svcIoSpecial(rootcdb->usbhndl, USB_SDF_DISCONNECT, NULL,
			devddb->gport, (char *)&specparms)) < 0 && rtn != ER_USBCN)
		logerror(rtn, "Error removing USB client for global port %08X",
				devddb->gport);
}


//****************************************************
// Function: deleteusbdevice - Delete a USB device and
//				any child devices from our database
// Returned: 
//****************************************************

void deleteusbdevice(
	HDB *hdb,
	long lport)
{
	DDB  *hddb;
	DDB  *devddb;
	DDB  *childddb;
	CDB  *rootcdb;
	HDB  *ourhdb;
	IFD  *ifd;
	long  gport;
	long  cport;
	char  bufr[16];

	if ((devddb = hdb->ddbtbl[lport]) == NULL)
		return;							// Nothing to to here if no device
	gport = devddb->gport;
	hddb = hdb->ddb;
	rootcdb = hddb->rootcdb;

	// First delete all child devices, if any

	if ((ourhdb = devddb->hdb) != NULL) // Is this a HUB?
	{
		// Here if this is a hub - Remove all child devices

		cport = 1;
		do
		{
			if ((childddb = ourhdb->ddbtbl[cport]) != NULL)
				deleteusbdevice(ourhdb, cport);
		} while (++cport <= 15);
		devddb->owner.hdb = NULL;			// Delete our HDB
		usbfree(ourhdb);
	}

	debugprint("Deleting device at %08X", devddb->gport);

	sprintf(bufr, "*USB%d*", rootcdb->unit); // Give up the device's USB address
	giveunitnumber(bufr, devddb->addr - 1);
	ifd = devddb->fifd;
	while (ifd != NULL)
	{
		if (ifd->allocfact != 0)
			giveunitnumber(ifd->name, (ifd->unit - ifd->allocofs) /
					ifd->allocfact);
		ifd = ifd->next;		
	}
	cleanddb(devddb);					// Delete all the data blocks pointed
										//   to by our DDB
	hdb->ddbtbl[lport] = NULL;
	usbfree(devddb);					// Delete our DDB
}


//****************************************************
// Function: getstring - Get string from a USB device
// Returned: Address of string (allocated with malloc)
//****************************************************

static char *getstring(
	CDB  *cdb,
	DDB  *ddb,
	int   lport,
	int   pktsz0,
	int   addr,
	int   speed,
	int   index)
{
	char *dpnt;
	char *spnt;
	char *epnt;
	char *str;
	long  rtn;
	int   len;
	char  buffer[MAXSTRLEN * 2 + 2];

	if (index == 0)
		return ("");

	// Get the length of the string

	if ((rtn = dosetup(cdb, ddb, pktsz0, speed, addr, 0, SETUP_INPUT |
			SETUP_STANDARD | SETUP_DEVICE, TYPE_GET_DESC,
			(DESC_STRING << 8) + index, 0x0409, (uchar *)buffer, 2)) < 0)
	{
		debugprint("error reading string descriptor length (%d)", rtn);
		return ("");
	}
	if ((len = (uchar)(buffer[0])) > (MAXSTRLEN * 2 + 2))
		len = MAXSTRLEN * 2 + 2;

	// Get the descriptor

///	debugprint("reading string desc %d", index);

	if ((rtn = dosetup(cdb, ddb, pktsz0, speed, addr, 0, SETUP_INPUT |
			SETUP_STANDARD | SETUP_DEVICE, TYPE_GET_DESC,
			(DESC_STRING << 8) + index, 0x0409, (uchar *)buffer, len)) < 0)
	{
		logerror(rtn, "Error reading string descriptor for %08X",
				cdb->ddb->gport);
		return ("");
	}

///	debugprint("string len = %d", len);

	// Discard any trailing spaces

	len = len/2;
	spnt = buffer + 2;
	epnt = buffer;
	while (--len > 0)
	{
		if (*(ushort *)spnt != ' ')
			epnt = spnt;
		spnt += 2;
	}
	if ((len = (epnt - buffer)/2) <= 1 || *(ushort *)(buffer + 2) == 0)
		return ("");					// If null string

	// Allocate a memory block and copy the string

	str = dpnt = (char *)usbmalloc(len + 1);
	spnt = buffer + 2;
	while (--len >= 0)
	{
		*dpnt++ = (spnt[1] == 0) ? spnt[0] : '?';
		spnt += 2;
	}
	*dpnt = 0;

	debugprint("have str desc %d for %08X: (%d) |%s|", index, cdb->ddb->gport +
			(lport << cdb->ddb->portshift), len, str);

	return (str);
}


//************************************************************
// Function: addtotext - Add text to device or interface text
// Returned: 0 if noraml or a negative XOS error code if error
//************************************************************

long addtotext(
	char **pnt,
	char  *txt)
{
	char *lpnt;
	int   len;
	char  devlst[300];

	if (*pnt != NULL)
	{
		lpnt = strmov(devlst, *pnt);
		usbfree(*pnt);
		*lpnt++ = ',';
		*lpnt++ = ' ';
	}
	else
		lpnt = devlst;
	len = (strmov(lpnt, txt) - devlst) + 1;
	if ((*pnt = (char *)usbmalloc(len)) == NULL)
		return (-errno);
	strmov(*pnt, devlst);
	return (0);
}


//**************************************************
// Function: dosetup - Do a control (setup) transfer
//				transacton to a USB device
// Returned: Number of bytes transfered if positive,
//				XOS error code if negative
//**************************************************

long dosetup(
	CDB   *cdb,
	DDB   *ddb,
	int    size,
	int    speed,
	int    addr,
	int    endpnt,
	int    type,
	int    request,
	int    value,
	int    index,
	uchar *buffer,
	int    length)
{
	_Packed struct
	{	TEXT8PARM class;
		BYTE8PARM setup;
		BYTE4PARM xfersize;
		BYTE1PARM xfertype;
		BYTE1PARM xferspeed;
		BYTE1PARM address;
		BYTE1PARM endpnt;
		uchar     end;
	} ioparms =
	{	{PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "USB"},
		{PAR_SET|REP_HEXV, 8, IOPAR_USB_SETUP},
		{PAR_SET|REP_HEXV, 4, IOPAR_USB_PKTSIZE},
		{PAR_SET|REP_HEXV, 1, IOPAR_USB_XFERTYPE, USB_XFERTYPE_CNTRL},
		{PAR_SET|REP_HEXV, 1, IOPAR_USB_XFERSPEED},
		{PAR_SET|REP_DECV, 1, IOPAR_USB_ADDRESS},
		{PAR_SET|REP_DECV, 1, IOPAR_USB_ENDPNT}
	};

	int  retry;
	long rtn;
///	long rtn2;

	debugprint("dosetup: %02.2X %02.2X %04.4X %04.4X %d -- GP: %08X A:%d "
			"EP:%d SP:%d", type, request, value, index,
			length, ddb->gport, addr, endpnt, speed);

	ioparms.address.value = addr;
	ioparms.endpnt.value = endpnt;
	ioparms.xfersize.value = size;
	ioparms.xferspeed.value = speed;
	((SETUP *)(&ioparms.setup.value))->reqtype = type;
	((SETUP *)(&ioparms.setup.value))->request = request;
	((SETUP *)(&ioparms.setup.value))->value = value;
	((SETUP *)(&ioparms.setup.value))->index = index;
	((SETUP *)(&ioparms.setup.value))->length = length;
	retry = 4;
	do
	{
		rtn = (type & SETUP_INPUT) ?
				svcIoInBlockP(cdb->usbhndl, (char *)buffer, length,
				(char *)&ioparms) : svcIoOutBlockP(cdb->usbhndl, (char *)buffer,
				length, (char *)&ioparms);
		if (rtn >= 0)
			return (rtn);
		logerror(rtn, "Error executing SETUP transfer for %08X, retry = %d",
				ddb->gport, retry - 1);
		svcSchSuspend(NULL, ST_MILLISEC * 40);
	} while (--retry > 0);
	logerror(0, "Too many SETUP retries for %08X - giving up", ddb->gport);
	return (rtn);
}


void cleanddb(
	DDB *ddb)
{
	CFG *cfg;
	IFD *ifd;
	EPT *ept;

	if (ddb->usbmnftr != NULL && ddb->usbmnftr[0] != 0)
		usbfree(ddb->usbmnftr);
	if (ddb->usbprod != NULL && ddb->usbprod[0] != 0)
		usbfree(ddb->usbprod);
	if (ddb->usbsernm != NULL && ddb->usbsernm[0] != 0)
		usbfree(ddb->usbsernm);
	if (ddb->devtxt != NULL && ddb->devtxt != 0)
		usbfree(ddb->devtxt);
	while ((cfg = ddb->fcfg) != NULL)
	{
		if (cfg->desc != NULL && cfg->desc[0] != 0)
			usbfree(cfg->desc);
		while ((ifd = cfg->fifd) != NULL)
		{
			if (ifd->desc != NULL && ifd->desc[0] != 0)
				usbfree(ifd->desc);
			if (ifd->hiddesc != NULL)
				usbfree(ifd->hiddesc);
			if (ifd->devlst != NULL && ifd->devlst != 0)
				usbfree(ifd->devlst);
			if (ifd->devtxt != NULL && ifd->devtxt != 0)
				usbfree(ifd->devtxt);
			while ((ept = ifd->fept) != NULL)
			{
				ifd->fept = ept->next;
				usbfree(ept);
			}
			cfg->fifd = ifd->next;
			usbfree(ifd);
		}
		ddb->fcfg = cfg->next;
		usbfree(cfg);
	}
}


void dumpit(
	uchar *data,
	int    size)
{
	uchar *npnt;
	uchar *apnt;
	char  *prefix;
	int    cnt;
	int    offset;
	uchar  nbuf[52];
	uchar  abuf[20];
	uchar  chr;

	offset = 0;
	cnt = 16;
	npnt = nbuf;
	apnt = abuf;
	prefix = "";
	while (--size >= 0)
	{
		chr = *data++;
		npnt += sprintf((char *)npnt, "%02.2X ", chr);
		if (chr < ' ')
			chr = '.';
		*apnt++ = chr;
		if (--cnt <= 0)
		{
			*apnt = 0;
			nbuf[47] = 0;
			debugprint("%s%03.3X>%-47s|%s|", prefix, offset, nbuf, abuf);
			prefix = "|";
			offset += 16;
			cnt = 16;
			npnt = nbuf;
			apnt = abuf;
		}
	}
	if (cnt != 16)
	{
		*apnt = 0;
		nbuf[47] = 0;
		debugprint("%s%03.3X>%-47s|%s|", prefix, offset, nbuf, abuf);
	}
}


void deverror(
	DDB  *ddb,
	long  code,
	char *fmt, ...)

{
	va_list pi;
	int     len;
	char    text[200];

    va_start(pi, fmt);
    len = vsnprintf(text, sizeof(text), fmt, pi);

	// Store error message in the DDB
	
	ddb->error = code;
	if (ddb->devtxt != NULL)
	{
		usbfree(ddb->devtxt);
		ddb->devtxt = NULL;
	}
	if ((ddb->devtxt = (char *)usbmalloc(len + 3)) != NULL)
	{
		ddb->devtxt[0] = '?';
		ddb->devtxt[1] = ' ';
		memcpy(ddb->devtxt + 2, text, len);
		(ddb->devtxt + 2)[len] = 0;
	}
}
