//============================================
// USBCTL.C
// Written by John Goltz
//============================================

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <xos.h>
#include <xosstr.h>
#include <xostime.h>
#include <errmsg.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xosvect.h>
#include <xosacct.h>
#include <xoserr.h>
#include <procarg.h>
#include <xosthreads.h>
#include <xosservert.h>
#include <xosusb.h>
///#include <usb.h>
///#include <usbctl.h>


#define MAJVER  1
#define MINVER  0
#define EDITNUM 0

// This program is the USB control symbiont.  It is responsable for all
//   USB device setup and removal.  The kernel USB class driver is fairly
//   simple and has no knowledge of device configuration.  All protocol
//   based operations are handled by this program, not by the class driver.

// While this program uses the standard server structure it does not use
//   the normal ADD command to add instances.  Upon startup, all USB
//   controllers in the system are found and an instance is created for
//   each controller.  A thread is created for each instance which serves
//   as the controller tread and also as the hub thread for the virtual root
//   hub associated with the controller.  An additional thread is created
//   for each hub that is found during operation.  All configuration
//   operations are done by the controller threads upon request from a
//   hub thread.  This implies that device configuration is single threaded
//   for each controller.  While initial startup would be a little faster
//   if each hub thread did configuration, doing it this way eliminates
//   the need for interlocks and should not be significantly slower,
//   especially after startup is complete.

// The USB class driver and all USB interface level drivers must be loaded
//   before this program is started.


time_s onesec = {XT_SECOND, 0};

struct
{   byte4_parm time;
    char       end;
} lrcinparms =
{  {PAR_SET|REP_HEXV, 4, IOPAR_TIMEOUT, 6}
};

uchar msgoutbufr[16];

long  unit;
long  address;
long  port;
long  ready;
long  cmdtdb;
long  usbhndl;
long  numunits;
char *cmdpntr[2];
///CDB  *cdbtbl[NUMDEVS];
///HDB  *hdbtbl[NUMHUBS];

///uchar __based(__segname("_DATA")) *iobufrpnt = iobufr;

char  prgname[] = "USBCTL";
char  srvname[] = "USBCTL";

struct
{   text8_parm class;
    uchar      end;
} usboparms =
{   {PAR_GET|REP_TEXT, 8, IOPAR_CLASS}
};

///SPECPARMS specparms =
///{	{PAR_SET|REP_TEXT, 4, IOPAR_CLASS, "USB"}
///};

static void  cmdstatus(char *cmd);
static int   fncstsaddr(arg_data *arg);
static int   fncstsport(arg_data *arg);
static int   fncunit(arg_data *arg);

char *sendresponse(uchar *bufr, uchar *base);

///static void putinlog(int instance, ulong pid, long code, char *fmt, ...);


arg_spec srvCmdWord[] =
{   {"STATUS"    , 0, NULL, srvFncCommand, (long)cmdstatus},
    {"STA"       , 0, NULL, srvFncCommand, (long)cmdstatus},
    {0}
};

arg_spec keywordsts[] =
{	{"UNIT"   , ASF_VALREQ|ASF_NVAL , NULL, fncunit, 0},
	{"UNI"    , ASF_VALREQ|ASF_NVAL , NULL, fncunit, 0},
	{"ADDRESS", ASF_VALREQ|ASF_NVAL , NULL, fncstsaddr, 0},
	{"ADD"    , ASF_VALREQ|ASF_NVAL , NULL, fncstsaddr, 0},
	{"PORT"   , ASF_VALREQ|ASF_NVAL , NULL, fncstsport, 0},
	{"POR"    , ASF_VALREQ|ASF_NVAL , NULL, fncstsport, 0},
	{0}
};


void mainalt(
    char *args)

{
	args = args;

	srvInitialize(args, (void *)NULL /* CMDTDB */, 0x2000, 40, 100 /* VECT_BASE */, MAJVER, MINVER,
	    EDITNUM);
}


void srvSetup1(void)

{
///	CDB *cdb;
	long rtn;
	char usbname[12];
	char bufr[256];


	srvFinishCmd();

	cmdtdb = (long)thdData;
	if ((rtn = setvector(3 /* VECT_STATUS */, 1, NULL /* statussignal */)) < 0)
	{
		srvLogSysLog(rtn, "Error setting status signal vector - Cannot "
				"continue");
		srvCmdErrorResp(rtn, "Error setting status signal vector - Cannot "
				"continue", NULL, srvMsgDst);
		exit(1);
	}
///	if ((rtn = svcMemChange(  ((__segment)cdbtbl):>iobufr, PG_READ|PG_WRITE, 0x1000)) < 0)
	{
		srvLogSysLog(rtn, "Error allocating IO buffer - Cannot continue");
		srvCmdErrorResp(rtn, "Error allocating IO buffer - Cannot continue",
				NULL, srvMsgDst);
		exit(1);
	}
	numunits = 0;
	while (numunits < 16)
	{
		sprintf(usbname, "_USB%d:***", numunits);
	    if ((usbhndl = svcIoOpen(XO_IN|XO_OUT, usbname, &usboparms)) < 0)
	    {
			if (usbhndl == ER_NSDEV)
				break;
			sprintf(bufr, "Cannot open USB device %s - Cannot continue",
					usbname);
			srvCmdErrorResp(usbhndl, bufr, NULL, srvMsgDst);
			exit(1);
    	}
	   	if (strcmp(usboparms.class.value, "USB") != 0)
   		{
			sprintf(bufr, "Device %s is not a USB device - Cannot continue",
					usbname);
			srvCmdErrorResp(0, bufr, NULL, srvMsgDst);
			exit(1);
    	}

	    // Create a root thread which handles one USB device.

///    	cdb = (CDB *)((numunits << 20) + CDBBASE + 0x200);
///	    if ((rtn = thdCtlCreate((long)cdb, 0x3200, sizeof(CDB), rootthread,
///				NULL, 0)) < 0)
    	{
			srvLogSysLog(rtn, "Error creating USB device thread - Cannot "
					"continue");
			srvCmdErrorResp(rtn, "Error creating USB device thread - Cannot "
					"continue", NULL, srvMsgDst);
			exit(1);
    	}
		ready = 0;
		while (ready == 0)
			thdCtlSuspend(NULL, -1);
		if (ready < 0)
		{
			srvLogSysLog(ready, "Cannot continue");
			srvCmdErrorResp(ready, "Cannot continue", NULL, srvMsgDst);
			exit(1);
		}
///		cdbtbl[numunits] = cdb;

		printf("### unit %d setup\n", numunits);

		numunits++;
	}
///	sprintf(bufr, STR_MT_FINALMSG"USBCTL: "ver" - %d USB device%s set up",
///			numunits, (unit == 1) ? "" : "s");
	srvLogSysLog(0, bufr + 9);
	srvCmdResponse(bufr, srvMsgDst);
}


void srvSetup2(void)

{

}

void srvSetupCmd(void)

{
    unit = -1;
}

void srvFinishCmd(void)

{

}

void srvMessage(
    char *msg,
    int   size)

{
    msg = msg;
    size = size;
}

//*******************************************
// Function: cmdadd - Process the ADD command
// Returned: Nothing
//*******************************************

/*
static void cmdadd(
    char *cmd)

{
    IDB *idb;
	long rtn;
	char bufr[320];

	if (!requireinst(cmd, keywordadd))
		return;
	if (insttbl[instance] != NULL)
	{
		srvCmdErrorResp(0, "Instance already exists", NULL, srvMsgDst);
		return;
	}
	if (devname[0] == 0)
	{
		srvCmdErrorResp(0, "TCP value not specified", NULL, srvMsgDst);
		return;
	}
	if (tcpport == 0)
	{
		srvCmdErrorResp(0, "PORT value not specified", NULL, srvMsgDst);
		return;
	}
	if ((idb = malloc(sizeof(IDB))) == NULL)
	{
		srvCmdErrorResp(-errno, "Cannot allocate memory for instance data",
				NULL, srvMsgDst);
		return;
	}
	strupr(devname);
	memset(idb, 0, sizeof(IDB));
	idb->parms.handle.desp = PAR_SET | REP_HEXV;
	idb->parms.handle.size = 4;
	idb->parms.handle.index = IOPAR_NETCONHNDL;
	idb->parms.timeout.desp = PAR_SET | REP_HEXV;
	idb->parms.timeout.size = 4;
	idb->parms.timeout.index = IOPAR_TIMEOUT;
	idb->parms.timeout.value =0xFFFFFFFF;
	idb->parms.ipaddr.desp = PAR_GET | REP_HEXV;
	idb->parms.ipaddr.size = 4;
	idb->parms.ipaddr.index = IOPAR_NETRMTNETAR;
	idb->parms.tcpport.desp = PAR_GET | REP_HEXV;
	idb->parms.tcpport.size = 4;
	idb->parms.tcpport.index = IOPAR_NETRMTPORTR;
	idb->instance = instance;
	idb->qab.qab_func = QFNC_OPEN;
	idb->qab.qab_vector = VECT_CONNECT;
	idb->qab.qab_option = O_IN | O_OUT;
	idb->qab.qab_buffer1 = idb->devname;
	idb->qab.qab_parm = &(idb->parms);
	idb->active = TRUE;
    tcpoparms.tcpport.value = tcpport;
    tcpoparms.conlimit.value = conlimit;
	if ((rtn = svcIoOpen(O_IN|O_OUT, devname, &tcpoparms)) < 0)
	{
		srvCmdErrorResp(rtn, "Cannot open TCP device %s", NULL, srvMsgDst,
				devname);
		free(idb);
		return;
	}
	if (strcmp(tcpoparms.class.value, "TCP") != 0)
	{
		srvCmdErrorResp(0, "Device %s is not a TCP device", NULL, srvMsgDst,
				devname);
		svcIoClose(idb->parms.handle.value, 0);
		free(idb);
		return;
	}
	idb->parms.handle.value = rtn;
    strcpy(idb->devname, devname);

    idb->tcpport = tcpport;
    idb->instance = instance;


	idbhead = idb;
	if ((rtn = svcIoQueue(&(idb->qab))) < 0 || (rtn = idb->qab.qab_error) < 0)
	{
		srvCmdErrorResp(rtn, "Error queuing initial TCP open for connections\n"
				"    Instacnce created but is not active", NULL, srvMsgDst);
		svcIoClose(idb->parms.handle.value, 0);
		idb->active = FALSE;
	}
	instcnt++;
	insttbl[instance] = idb;
	sprintf(bufr, STR_MT_INTRMDMSG"HOSTINIT: "ver" - Instance %d created\n"
			"          TCP device: %s, Port: %d, Program: %s", instance,
			devname, tcpport, progname);
	srvLogSysLog(0, bufr + 11);
	srvCmdResponse(bufr, srvMsgDst);
	srvLogSysLog(0, ver" - Startup is complete");
	srvCmdResponse(STR_MT_FINALMSG"HOSTINIT: "ver" - Startup is complete",
			srvMsgDst);
}
*/

//*************************************************
// Function: cmdstatus - Process the STATUS command
// Returned: Nothing
//*************************************************

static void cmdstatus(
    char *cmd)

{
	cmd = cmd;


    procarg(cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keywordsts, NULL,
		    srvCmdError, NULL, NULL);

/*
	char *bufr;
	CDB **tpnt;
	CDB  *cdb;
	DDB  *ddb;
	CFG  *cfg;
	IFC  *ifc;
	IFD  *ifd;
	EPT  *ept;
	int   cnt;
	char  text[32];
	char  bufr1[512];

	unit = -1;
	address = -1;
	port = -1;
    cmdpntr[0] = cmd;
    if (!procarg(cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keywordsts, NULL,
		    srvCmdError, NULL, NULL))
		return;

///	sprintf(bufr1, STR_MT_INTRMDMSG"USBCTL: "ver" - Unit %d", srvUnitNum);
    srvCmdResponse(bufr1, srvMsgDst);

	if (unit == -1 && address == -1)
	{
		srvCmdResponse(STR_MT_INTRMDMSG"Unit Type IRQ  IOReg Ports  Con sts  "
				"Act sts Con dev Act dev", srvMsgDst);
		cnt = numunits;
		tpnt = cdbtbl;
		while (--cnt >= 0)
		{
			cdb = *tpnt++;
			sprintf(bufr1, "%c%4d %4.4s %3d 0x%04.4X %5d %8.*b %8.*b %7d %7d",
					(cnt < 0 || *tpnt == NULL) ? MT_FINALMSG : MT_INTRMDMSG,
					cdb->unit, cdb->type, cdb->irq, cdb->ioreg, cdb->numports,
					cdb->numports, cdb->consts, cdb->numports, cdb->actsts,
					cdb->condevs, cdb->actdevs);
		    srvCmdResponse(bufr1, srvMsgDst);
		}
	}
	else if (unit != -1)
	{
		if (unit >= numunits)
		{
			sprintf(bufr1, STR_MT_FINALERR"? USB unit %d does not exist", unit);
		    srvCmdResponse(bufr1, srvMsgDst);
			return;
		}
		cdb = cdbtbl[unit];
		if (address != -1)
		{
			if (address < 1 || address > 127)
			{
				sprintf(bufr1, STR_MT_FINALERR"? USB address %d is not valid",
						address);
		    	srvCmdResponse(bufr1, srvMsgDst);
				return;
			}
			if ((ddb = cdb->devtbl[address]) == NULL)
			{
				sprintf(bufr1, STR_MT_FINALERR"? USB address %d is not in use "
						"on USB unit %d", address, unit);
		    	srvCmdResponse(bufr1, srvMsgDst);
				return;
			}

			bufr = bufr1;
			bufr[256] = 0;

			sprintf(bufr, STR_MT_INTRMDMSG"USB address %d on USB unit %d:",
					address, unit);
			bufr = sendresponse(bufr, bufr1);

/// ADD CODE HERE TO REPORT PORT

			sprintf(bufr, STR_MT_INTRMDMSG"Max packet size for end-point 0: %d",
					ddb->maxpktsz0);
			bufr = sendresponse(bufr, bufr1);

			if (ddb->manstr[0] != 0)
			{
				sprintf(bufr, STR_MT_INTRMDMSG"Manufacturer: %s", ddb->manstr);
				bufr = sendresponse(bufr, bufr1);
			}
			if (ddb->prostr[0] != 0)
			{
				sprintf(bufr, STR_MT_INTRMDMSG"Product: %s", ddb->prostr);
				bufr = sendresponse(bufr, bufr1);
			}
			if (ddb->serstr[0] != 0)
			{
				sprintf(bufr, STR_MT_INTRMDMSG"Serial Number: %s", ddb->serstr);
				bufr = sendresponse(bufr, bufr1);
			}
			cfg = ddb->firstcfg;
			while (cfg != NULL)
			{
				if (cfg == ddb->curcfg)
					strcpy(text, " (Current)");
				else
					text[0] = 0;
				sprintf(bufr, STR_MT_INTRMDMSG"Configuration %d%s:",
						cfg->value, text);
				bufr = sendresponse(bufr, bufr1);

				sprintf(bufr, STR_MT_INTRMDMSG"  Selection value: %d",
						cfg->value);
				bufr = sendresponse(bufr, bufr1);

				sprintf(bufr, STR_MT_INTRMDMSG"  Attributes: 0x%02.2X",
						cfg->attrib);
				bufr = sendresponse(bufr, bufr1);

				sprintf(bufr, STR_MT_INTRMDMSG"  Max power: %d",
						cfg->maxpower);
				bufr = sendresponse(bufr, bufr1);


				sprintf(bufr, STR_MT_INTRMDMSG"  Number of interfaces: %d",
						cfg->numifc);
				bufr = sendresponse(bufr, bufr1);

				if (cfg->desc[0] != 0)
				{
					sprintf(bufr, STR_MT_INTRMDMSG"  Description: %s",
						cfg->desc);
					bufr = sendresponse(bufr, bufr1);
				}
				ifc = cfg->firstifc;
				while (ifc != NULL)
				{
					sprintf(bufr, STR_MT_INTRMDMSG"  Interface %d:",
						ifc->number);
					bufr = sendresponse(bufr, bufr1);

					ifd = ifc->firstifd;
					while (ifd != NULL)
					{
						if (ifd == ifc->curifd)
							strcpy(text, " (Current)");
						else
							text[0] = 0;
						sprintf(bufr, STR_MT_INTRMDMSG"    Alternate %d%s:",
							ifd->altsel, text);
						bufr = sendresponse(bufr, bufr1);

						sprintf(bufr, STR_MT_INTRMDMSG"      Class: %d, "
								"Sub-class: %d, Protocol: %d", ifd->class,
								ifd->subclass, ifd->protocol);
						bufr = sendresponse(bufr, bufr1);

						sprintf(bufr, STR_MT_INTRMDMSG"      Number of "
							"end-points: %d", ifd->numept);
						bufr = sendresponse(bufr, bufr1);

						if (ifd->desc[0] != 0)
						{
							sprintf(bufr, STR_MT_INTRMDMSG"      "
									"Description: %s", cfg->desc);
							bufr = sendresponse(bufr, bufr1);
						}
						ept = ifd->firstept;
						while (ept != NULL)
						{
							sprintf(bufr, STR_MT_INTRMDMSG"      "
								"End-point %d (%s):", ept->number & 0x7F,
								(ept->number & 0x80) ? "Input" : "Output");
							bufr = sendresponse(bufr, bufr1);

							sprintf(bufr, STR_MT_INTRMDMSG"        "
								"Attributes: 0x%02.2X", ept->attrib);
							bufr = sendresponse(bufr, bufr1);


							sprintf(bufr, STR_MT_INTRMDMSG"        "
								"Max packet size: %d", ept->maxpktsz);
							bufr = sendresponse(bufr, bufr1);

							sprintf(bufr, STR_MT_INTRMDMSG"        "
								"Packet rate: %02.2X", ept->pktrate);
							bufr = sendresponse(bufr, bufr1);

							ept = ept->next;
						}
						ifd = ifd->next;
					}
					ifc = ifc->next;
				}

				cfg = cfg->next;
			}
			bufr = (bufr == bufr1) ? (bufr1 + 256) : bufr1;
			*bufr = MT_FINALMSG;
			srvCmdResponse(bufr, srvMsgDst);




		}

	}
*/
}


char *sendresponse(
	uchar *bufr,
	uchar *base)

{
	if (bufr == base)
	{
		if (bufr[256] != 0)
		   	srvCmdResponse(base + 256, srvMsgDst);
		return (base + 256);
	}
	else
	{
    	srvCmdResponse(base, srvMsgDst);
		return (base);
	}
}


//**********************************************************
// Function: fncunit - Process UNIT keyword for all commands
// Returned: TRUE if OK, FALSE if error
//**********************************************************

static int fncunit(
    arg_data *arg)

{
    unit = arg->val.n;
    return (TRUE);
}


//**********************************************************************
// Function: fncstsaddr - Process ADDRESS keyword for the STATUS command
// Returned: TRUE if OK, FALSE if error
//**********************************************************************

static int fncstsaddr(
    arg_data *arg)

{
    address = arg->val.n;
    return (TRUE);
}


//*******************************************************************
// Function: fncstsport - Process PORT keyword for the STATUS command
// Returned: TRUE if OK, FALSE if error
//*******************************************************************

static int fncstsport(
    arg_data *arg)

{
    port = arg->val.n;
    return (TRUE);
}


//*********************************************
// Function: getaddress - Assign a USB address
// Returend: USB address (positive) or negative
//				XOS error code
//*********************************************

/*
int getaddress(
	CDB *cdb)

{
	int addr;

	addr = 1;
	do
	{
		if (cdb->devtbl[addr] == NULL)
		{
			cdb->devtbl[addr] = (DDB *)-1;
			return (addr);
		}
	} while (++addr <= 127);
	return (ER_TMDVA);
}
*/

//*****************************************************
// Function: fail - Log message for general failure not
//		associated with a command
// Returned: Nothing
//*****************************************************

/*
void fail(
    CDB  *cdb,
    long  code,
    char *msg)

{
    char *pnt;
    char *logpnt;
    char  text[200];

    logpnt = text + sprintf(text, "Unit %d, inst %d, ", srvUnitNum,
			cdb->unit);
    pnt = strmov(logpnt, msg);
    srvLogSysLog(code, text);
}
*/

//*******************************************************
// Function: putinlog - Put a message into the system log
// Returend: Nothing
//*******************************************************

// Sends error response to the user and logs the error

/*
void putinlog(
	int   instance,
	ulong pid,
	long  code,
	char *fmt, ...)

{
	va_list pi;
	long    xoscode;
	char    text[200];
	char    text2[300];
	char    textmms[100];
	char    textxos[100];
	char    textpid[32];

    va_start(pi, fmt);

	if (pid != 0)
		sprintf(textpid, "(%d:%d.%d) ", instance, pid >> 16, (ushort)pid);
	else
		sprintf(textpid, "(%d) ", instance);
    vsprintf(text, fmt, &va_arg(pi, long));
	xoscode = mmserrmsg(code, strmov(textmms, "\n        "));
	if (xoscode != 0)
		svcSysErrMsg(xoscode, 0x03, strmov(textxos, "\n        "));
	svcSysLog(text2, sprintf(text2, "xxxxUSBCTL  %s%s%s%s", textpid, text,
			(textmms[9] != 0) ? textmms : "", (xoscode != 0) ? textxos : ""));
	if (srvReqName[0] == 0)
		printf("%c USBCTL: %s\n", (code < 0) ? '?' : '%', text2 + 12);
}
*/

