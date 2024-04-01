//==================================
// hub.c Functions dealing with hubs
// Written by John Goltz
//==================================

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

// Hub devices are managed completely by this program. No additional driver(s)
//   are loaded.

//*******************************************
// Function: hubthread - Main thread function
//		for the thread that manages a hub
// Returned: Never returns
//*******************************************

// This thread initializes a hub for operation and then does interrupt
//   input to poll hub status chagnes.  Once initialization is complete
//   (which is interlocked with the root thread) interrupt input is the
//   only IO done by this thread.  All IO required to handle status
//   changes is done by the root thread.

void hubthread(
	DDB *ddb,
	int  hubnum)

{
	static _Packed struct
	{	TEXT4PARM class;
		BYTE1PARM xfertype;
		uchar     end;
	} inparm =
	{	{PAR_SET|REP_TEXT, 4, IOPAR_CLASS, "USB"},
		{PAR_SET|REP_HEXV, 1, IOPAR_USB_XFERTYPE, USB_XFERTYPE_INT}
	};

	_Packed struct
	{	TEXT4PARM class;
		BYTE1PARM addr;
		BYTE1PARM endpnt;
		BYTE1PARM pktsize;
		BYTE2PARM pktrate;
///		BYTE1PARM inpsize;
		uchar     end;
	} sdfparms =
	{	{PAR_SET|REP_TEXT, 4, IOPAR_CLASS, "USB"},
		{PAR_SET|REP_DECV, 1, IOPAR_USB_ADDRESS},
		{PAR_SET|REP_DECV, 1, IOPAR_USB_ENDPNT},
		{PAR_SET|REP_DECV, 1, IOPAR_USB_PKTSIZE},
		{PAR_SET|REP_DECV, 2, IOPAR_USB_PKTRATE},
///		{PAR_SET|REP_DECV, 1, IOPAR_USB_INPSIZE}
	};
	CDB *cdb;
	HDB *hdb;
	EPT *ept;
	long rtn;
	int  inx;
	char usbname[16];
	char intbufr[16];
	char descbufr[32];

	cdb = NULL;


	hubnum = hubnum;

///	hdb = (HDB *)fbrData;

///	hdb->hubnum = hubnum;

	hdb = NULL;	//////

	hdb->fncresetport = hubresetport;
///	cdb = ddb->cdb;

	// According to the USB spec, hubs can only have one configuration
	//   which must have only one interface which must have only one
	//   end-point which must be an interrupt input end-point.  We
	//   assume this is true.

	ept = ddb->curcfg->fifd->fept;

	if (ept->attrib != 3)
	{
		char text[128];

		sprintf(text, "Hub end-point is not an interrupt end-point for "
				"hub ??? - hub will not be used");
///		srvLogSysLog(ER_FMTER, text);
///		cdb->ready = ER_FMTER;
///		fbrCtlWake((long)cdb);
		hubterminate(hdb);
	}
	hdb->intpktrate = ept->pktrate;
	hdb->intmaxpktsz = ept->maxpktsz;
	hdb->intendpnt = ept->number;

	// Get the hub class descriptor

//// THE ADDRESS IS PROBABLY WRONG HERE!!!!

	if ((rtn = dosetup(cdb, cdb->ddb, 8, ddb->addr, FALSE, 0, 0, SETUP_INPUT |
			SETUP_CLASS | SETUP_DEVICE, TYPE_GET_DESC, 0x29 << 8,
			(uchar *)descbufr, sizeof(HUBDESC))) < 0)
	{
		char text[128];

		sprintf(text, "Error getting hub descriptor for hub ??? - hub will "
					"not be used");
///		srvLogSysLog(rtn, text);
		cdb->ready = rtn;
///		fbrCtlWake((long)cdb);
		hubterminate(hdb);
	}
	hdb->numports = ((HUBDESC *)descbufr)->numports;
	if (hdb->numports < 1 || hdb->numports > 31 || ((HUBDESC *)descbufr)->length <
			(offsetof(HUBDESC, data) + ((hdb->numports + 8)/8 * 2)) ||
			((HUBDESC *)descbufr)->desctype != 0x29)
	{
		char text[128];

		sprintf(text, "Incorrect format for hub descriptor for hub ??? - "
					"hub will be used");
///		srvLogSysLog(ER_FMTER, text);
		cdb->ready = ER_FMTER;
///		fbrCtlWake((long)cdb);
		hubterminate(hdb);
	}
	rtn = ((HUBDESC *)descbufr)->attrib;
	hdb->attrib = rtn;
	rtn = ((HUBDESC *)descbufr)->on2good;
	hdb->on2good = rtn;
	rtn = ((HUBDESC *)descbufr)->contrcurrent;
	hdb->contrcurrent = rtn;

	rtn = (hdb->numports + 8) / 8;

	debugprint("expect %d bytes", rtn);

	inx = 0;
	do
	{
		hdb->removable[inx] = ((HUBDESC *)descbufr)->data[inx];
		hdb->portpower[inx] = ((HUBDESC *)descbufr)->data[inx + rtn];
	} while (++inx < rtn);

	// Open a device to use to poll the hub for status changes

	sprintf(usbname, "_USB%d:", cdb->unit);
    if ((hdb->usbhndl = svcIoOpen(XO_IN|XO_OUT, usbname, NULL)) < 0)
    {
		char text[128];

		sprintf(text, "Cannot open USB device %s for hub ??? - hub will "
					"be used", usbname);
///		srvLogSysLog(rtn, text);
		cdb->ready = rtn;
///		fbrCtlWake((long)cdb);
		hubterminate(hdb);
	}

	// Configure the hub

	if ((rtn = dosetup(cdb, cdb->ddb, 8, ddb->addr, FALSE, 0, SETUP_OUTPUT |
			SETUP_DEVICE | SETUP_STANDARD, TYPE_SET_CONFIG,
			ddb->curcfg->value, 0, NULL, 0)) < 0)
	{
		char text[128];

		sprintf(text, "Error configuring hub ??? - hub will not be used");
///		srvLogSysLog(rtn, text);
		cdb->ready = rtn;
///		fbrCtlWake((long)cdb);
		hubterminate(hdb);
	}

	// Turn on power to the hubs ports

	if ((hdb->attrib & 0x02) == 0)		// Do we need to turn on power?
	{
		if ((hdb->attrib & 0x01) == 0)	// Yes - is it ganged?
		{								// Yes
			if ((rtn = dosetup(cdb, cdb->ddb, 8, ddb->addr, FALSE, 0,
					SETUP_CLASS | SETUP_DEVICE, TYPE_SET_FEATURE, 8, 0,
					NULL, 0)) < 0)
			{
				char text[128];

				sprintf(text, "Error setting port power for hub ??? - hub "
				"will not be used");
///				srvLogSysLog(rtn, text);
				cdb->ready = rtn;
///				fbrCtlWake((long)cdb);
				hubterminate(hdb);
			}
		}
		else							// If not ganged
		{
			inx = 1;
			do
			{

				debugprint("turning on power to port %d", inx);

				if ((rtn = dosetup(cdb, cdb->ddb, 8, ddb->addr, FALSE, 0,
						SETUP_CLASS | SETUP_OTHER, TYPE_SET_FEATURE, 8, inx,
						NULL, 0)) < 0)
				{
					char text[128];

					sprintf(text, "Error setting port power for hub ??? - "
							"hub will not be used");
///					srvLogSysLog(rtn, text);
					cdb->ready = rtn;
///					fbrCtlWake((long)cdb);
					hubterminate(hdb);
				}
			} while (++inx <= hdb->numports);
		}
	}

	// Start interrupt input

	sdfparms.addr.value = ddb->addr;
	sdfparms.endpnt.value = hdb->intendpnt;
	sdfparms.pktsize.value = hdb->intmaxpktsz;
	sdfparms.pktrate.value = hdb->intpktrate;;
///	sdfparms.inpsize.value = hdb->intmaxpktsz;

/// (hdb->numports + 8)/8;

///	if ((rtn = svcIoSpecial(hdb->usbhndl, SDF_USB_BGNINTINPUT, NULL, 10,
///			&sdfparms)) < 0)
	{
		char text[128];

		sprintf(text, "Error starting interrupt input for hub ??? - "
				"hub will not be used");
///		srvLogSysLog(rtn, text);
		cdb->ready = rtn;
///		fbrCtlWake((long)cdb);
		hubterminate(hdb);
	}

	// Here with all ports powered - release the root thread and start
	//   polling

	cdb->ready = 1;
///	fbrCtlWake((long)cdb);


	while (TRUE)
	{
		if ((rtn = svcIoInBlockP(hdb->usbhndl, intbufr, sizeof(intbufr),
				(char *)&inparm)) < 0)
		{
			char text[128];

			sprintf(text, "Error getting interrupt input for hub ??? - "
					"hub will not be used");
///			srvLogSysLog(rtn, text);
			cdb->ready = rtn;
///			fbrCtlWake((long)cdb);
			hubterminate(hdb);
		}

		debugprint("have interrupt input: %d bytes: %02.2X %02.2X", rtn,
				intbufr[0], intbufr[1]);

/*
				if ((req = (REQ *)usbmalloc(sizeof(REQ))) == NULL)
				{
					sprintf(bufr, "Error allocating memory for REQ block "
							"for USB device USB%d - FATAL ERROR", cdb->unit);
					srvLogSysLog(-errno, bufr);
					exit(1);
				}
				req->hdb = hdb;
				req->func = (conbits & 0x01) ? REQ_CONNECT : REQ_REMOVE;
				req->port = port;
				req->next = NULL;
				if (cdb->reqtail != NULL)
					cdb->reqtail->next = req;
				else
				{
					cdb->reqhead = req;
					fbrCtlWake((long)cdb);
				}
				cdb->reqtail = req;

*/
	}

}


//***************************************************
// Function: hubsetup - Set up a hub class device
// Returned: 0 if normal, negative XOS error if error
//***************************************************

// This function is only called by the root thread.  It is called whenever
//   a hub connects.

/*
long hubsetup(
	CDB *cdb,
	DDB *ddb)

{
	HDB *hdb;
	long rtn;
	long args[2];
	int  hubnum;
	char text[128];


	hubnum = 1;
	do
	{
///		if (cdb->hdbtbl[hubnum] == NULL)
		{
			hdb =  (HDB *)((hubnum << 20) + HDBBASE + 0x200);
			cdb->hdbtbl[hubnum] = (HDB *)-1;
			args[0] = (long)ddb;
			args[1] = hubnum;
		    if ((rtn = fbrCtlCreate((long)hdb, 0x3200, sizeof(HDB), hubthread,
				args, 2)) < 0)
	    	{
				sprintf(text, "Error creating hub thread - hub ??? will "
						"not be used");
				srvLogSysLog(rtn, text);
				return (rtn);
			}
			cdb->ready = 0;
			while (cdb->ready == 0)
				fbrCtlSuspendT(NULL, -1);
			return (0);
		}
	} while (++hubnum <= NUMHUBS);
	sprintf(text, "Too many hubs - hub ??? will not be used");
	srvLogSysLog(ER_USBTH, text);
	return (ER_USBTH);
}
*/

//***************************************************
// Function: hubresetport - Reset a hub port
// Returned: 0 if normal, negative XOS error if error
//***************************************************

// This function is only called by the root thread

long hubresetport(
	HDB *hdb,
	int  port)

{
	hdb = hdb;
	port = port;

	return (0);
}


void hubterminate(
	HDB *hdb)

{
	hdb = hdb;

	debugprint("hubterminate called!");
	exit (1);
}
