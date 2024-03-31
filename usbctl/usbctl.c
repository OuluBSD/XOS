//============================================
// USBCTL.C
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


// This program is the USB control symbiont.  It is responsable for all
//   USB device setup and removal.  The kernel USB class driver is fairly
//   simple and has no knowledge of device configuration.  All protocol
//   based operations are handled by this program, not by the class driver.

// While this program uses the standard thread based server structure it does
//   not use the normal ADD command to add instances.  Upon startup, all USB
//   controllers in the system are found and an instance is created for
//   each controller.  A thread is created for each instance which serves
//   as the controller tread and also as the hub thread for the virtual root
//   hub associated with the controller.  An additional thread is created
//   for each hub that is found during operation.  All configuration operations
//   are done by the controller threads upon request from a hub thread.  This
//   implies that device configuration is single threaded for each controller
//   While initial startup would be a little faster if each hub thread did
//   configuration, doing it this way eliminates the need for interlocks and
//   should not be significantly slower, especially after startup is complete.

// The USB class driver and all USB interface level drivers must be loaded
//   before this program is started.

// This program uses an internal data structure that mirrors the physical
//   topology of our USB devices. The CDB (controller data block) and HDB
//   (hub data block) "inheirt" from a common parent, although, since this
//   is written in C, we have to fake it. The two strutures have a common
//   initial part and are, in some cases, used interchangably. Also, each
//   USB device (including the host controllers) are defined in a DDB (device
//   data block). Each device which supports child devices also has a CDB
//   (host controller) or an HDB (hub). At the root of the tree is the contbl
//   which is an array of pointers to DDBs indexed by USB controller unit
//   number. There is one DDB for each USB host controller. Each of these
//   DDBs has an associated CDB. Each CDB contains a devtbl which is an array
//   of DDBs indexed by port number. Each of these DDBs that represents a hub
//   also has an associated HDB which contains a devtbl. This can continue
//   for up to 8 levels. This includes the host controllers as the first
//   two levels so there can be up to 6 levels of hubs connected. This allows
//   a complete description of the position of any device in this tree with a
//   32-bit value with each level specified by a 4-bit nibble. For example,
//   if a port value is 0x5312 this would represent a device connected to
//   port 5 on a hub which is connected to port 3 on a hub which is connected
//   to port 1 on host controller USB2:. A device directly connected to port
//   2 on controller USB3: would be represented as 0x23.

// This program does NOT directly keep track of the XOS devices associated
//   with each USB device other than to keep a list of XOS device units in
//   the devlst element in each IFD. The kernel level USB driver DOES keep
//   track of this. The only time we really care about this is when a USB
//   device is removed and all of the associated XOS devices must be destroyed.
//   This done by sending a request to the driver to destroy all devices
//   associated with a given global port.

// Memory usage:
//   0x00010000 - Start of code
//   0x00200000 - Start of virtual part of main stack
//   0x00220000 - Start of physical part of main stack
//   0x00220200 - Store of allocated data and heap
//   0x0x380000 - Store of the thread stack area - Each thread has a 32K area.
//                  The first page is not allocated. The remaining 28K is
//                  initially allocated virtually.

time_s onesec = {ST_SECOND};

int srvmajver  = 4;
int srvminver  = 2;
int srveditnum = 1;

DDB     *roottbl[16];

long     dskanumber;
long     ptranumber;
long     kbdanumber;
long     unitnum;
long     unit;
long     address;
long     port;
long     ready;
long     usbhndl;
long     ipmhndl = -1;
long     dvseqnum;
long     ifseqnum;
int      indent;
long     startcnt;
long     numactive;			// Number of root thread actions
long     ttlactive;			// Number of root threads now active
long     vector = 48;		// Next vector to use (Because vectors are
							//   associated with threads, each thread must
							//   have its own set of vectors. Each root
							//   thread uses 2 vectors.)
long     thrdstk = 0x380000;// Next thread stack area to use

CRIT     cfgcrit = {'CRIT'};
CRIT     vencrit = {'CRIT'};

NAMEDEF *namehead;
char    *cmdpntr[2];

_Packed struct
{   BYTE4PARM time;
    char      end;
} lrcinparms =
{  {PAR_SET|REP_HEXV, 4, IOPAR_TIMEOUT, 6}
};

char     msgoutbufr[16];
char     lkename[64];

static void putinlog(int instance, ulong pid, long code, char *fmt, ...);


SFWCMDTBL srvcmdtbl[] =
{   {"STATUS", cmdstatus},
    {"STA"   , cmdstatus},
	{"LOG"   , cmdlog},
	{"RETRY" , cmdretry},
	{"RET"   , cmdretry},
	{"ACT"   , cmdactive},
    {NULL}
};

_Packed struct
{	TEXT8PARM class;
	uchar     end;
} usboparms =
{	{PAR_GET|REP_TEXT, 8, IOPAR_CLASS}
};

SPECPARMS specparms =
{	{PAR_SET|REP_TEXT, 4, IOPAR_CLASS, "USB"}
};

char  prgname[] = "USBCTL";

char *debugdefer;
long  debugsize;

FILE *debugfile;
char  debugname[128];
uchar debugflg;
uchar debugtrm;


void mainalt(
    char *args)
{
	sfwThreadInit((HUMA *)args);
}


int srvsetup(void)
{
	time_s dt;
	char   *pnt;
	long    targs[3];
	long    rtn;
	int     unitcnt;
	char    usbname[12];
	char    bufr[256];

	debugtrm = debugflg = (sfwReqName[0] == 0);
	if (sfwLogFile != NULL)
	{
		debugflg = TRUE;
		if (*(short *)sfwLogFile == '*')
		{
			debugdefer = debugbase;
			debugsize = 0;
		}
		else
		{
			if (strlen(sfwLogFile) > 124)
			{
				sfwResponse(ER_BDSPC, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|
						SFWRSP_PREFIX, "Debug log file name is too long - "
						"Cannot continue");
				exit(1);
			}
			if ((debugfile = fopen(sfwLogFile, "a+")) == NULL)
			{
				sfwResponse(-errno, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|
						SFWRSP_PREFIX, "Error opening debug file - Cannot "
						"continue");
				exit(1);
			}
			strcpy(debugname, sfwLogFile);
		}
		svcSysDateTime(T_GTSYSDTTM, &dt);
		sdt2str(bufr + 128, "%H:%m:%s on %z%D-%3n-%y", (time_sz *)&dt);

		rtn = sprintf(bufr, "*   USBCTL %d.%d.%d starting at %s   *\n",
				srvmajver, srvminver, srveditnum, bufr + 128) - 1;
		pnt = bufr + 129;
		bufr[128] = '|';
		do
		{
			*pnt++ = '*';
		} while (--rtn > 0);
		debugprint("| ");
		debugprint(bufr + 128);
		debugprint(bufr);
		debugprint(bufr + 128);
		debugprint("| ");
		if (debugdefer != NULL)
			debugprint("***** Start of deferred log entries\n");
	}
	unitnum = 0;
	unitcnt = 0;
	while (++unitnum <= 15)
	{
		sprintf(usbname, "_USB%d:***", unitnum);
	    if ((usbhndl = svcIoOpen(XO_IN|XO_OUT, usbname, &usboparms)) < 0)
	    {
			if (usbhndl == ER_NSDEV)
				break;
			sfwResponse(usbhndl, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|
					SFWRSP_PREFIX, "Cannot open USB device %s", usbname);
			continue;
		}
		if (strcmp(usboparms.class.value, "USB") != 0)
			continue;

	    // Create a root thread to handle this USB controller.

		debugprint("Creating root thread for USB%d:", unitnum);

		atomic_inc(startcnt);

		if ((rtn = svcMemChange((char *)(thrdstk + 0x1000),
				PG_VIRTUAL|PG_READ|PG_WRITE, 0x7000)) < 0)
		{
			sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
					"Error allocating memory for the thread stack for USB%d: - "
					"Cannot continue", unitnum);
			exit(1);
		}
		targs[0] = unitnum;
		targs[1] = usbhndl;
		targs[2] = vector;
		vector += 3;
		thrdstk += 0x8000;
		if ((rtn = svcSchThread(0, (long *)thrdstk, 0, rootthread, targs,
				3)) < 0)
		{
			sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX,
					"Error creating USB device thread for USB%d: - Cannot "
					"continue", unitnum);
			exit(1);
		}
		unitcnt++;
	}
	while (startcnt)					// Wait until all controller finish
		svcSchSuspend(&startcnt, -1);	//   their set up
	if (unitcnt == 0)					// Did we find any controllers?
	{									// No - we might as well terminate
										//   since we have nothing to do!
		debugprint("No USB controllers were found - terminating");
		sfwResponse(0x80000000, SFWRSP_SEND|SFWRSP_LOG|SFWRSP_FINAL|
				SFWRSP_PREFIX, "No USB controllers were found - terminating");
		exit(0);
	}
	debugmem();
	sprintf(bufr, "%d USB controller%s initialized", unitcnt, (unitcnt == 1) ?
			" was" : "s were");
	debugprint(bufr);
	sfwResponse(0, SFWRSP_LOG|SFWRSP_SEND|SFWRSP_FINAL|SFWRSP_PREFIX, bufr);
	return (TRUE);
}


void srvmessage(
    char *msg,
    int   size)
{
    (void)msg;
    (void)size;
}


//**********************************************************
// Function: fncunit - Process UNIT keyword for all commands
// Returned: TRUE if OK, FALSE if error
//**********************************************************

int fncunit(
    arg_data *arg)
{
    unit = arg->val.n;
    return (TRUE);
}


void logerror(
	long  code,
	char *fmt, ...)
{
	va_list pi;
	char   *prefix;
	char    text1[200];
	char    text2[300];
	char    textxos[100];

    va_start(pi, fmt);
    vsprintf(text1, fmt, pi);
	textxos[0] = 0;
	prefix = "";
	if (code < 0)
	{
		prefix = "? ";
		svcSysErrMsg(code, 0x03, strmov(textxos, "\n  "));
	}
	svcSysLog(text2, sprintf(text2, "xxxxUSBCTL_1%s%s%s", prefix, text1,
			textxos));
	if (debugflg)
		debugprint("%s%s%s", prefix, text1, textxos);
}



void debugprint(
	char *fmt, ...)
{
	time_s  dt;
	va_list pi;
	char   *pnt;
	char   *bgn;
	char    text[200];
	char    time[32];
	char    chr;

	if (debugflg)
	{
	    va_start(pi, fmt);
    	vsnprintf(text, sizeof(text), fmt, pi);
		pnt = text;
		if (*pnt == '|')
			pnt++;
		critBegin(&sfwTrmCrit);
		do
		{
			bgn = pnt;
			while ((chr = *pnt) != 0 && chr != '\n' && chr != '\r')
				pnt++;
			*pnt = 0;
			if (bgn != pnt)
			{
				if (bgn == text)
				{
					svcSysDateTime(T_GTHRDTTM, &dt);
					sdt2str(time, "%m:%s.%3f", (time_sz *)&dt);
				}
				else
					time[0] = 0;
				if (debugtrm)
					printf("%9s %s\n", time, bgn);
				if (debugfile != 0 || debugdefer != NULL)
				{
					if (debugfile != NULL)
						fprintf(debugfile, "%9s %s\n", time, bgn);
					else
					{
						if ((debugdefer - debugbase + 400) > debugsize)
						{
							if ((debugsize = svcMemChange(debugbase,
									PG_READ|PG_WRITE, debugsize + 0x1000)) < 0)
							{
								debugdefer = NULL;
								svcMemChange(debugbase, 0, 0);
							}
						}
						debugdefer += sprintf(debugdefer, "%9s %s\n", time,
								bgn);
					}
				}
			}
			pnt++;
		} while (chr != 0);
		if (debugfile != NULL && fflush(debugfile) < 0)
			logerror(-errno, "Error committing log file data");
		critEnd(&sfwTrmCrit);
	}
}


//*********************************************************************
// Function: allocunitnumber - Allocate a unit number for a device name
// Returned: Allocated number
//*********************************************************************

int allocunitnumber(
	char *name)

{
	NAMEDEF *pnt;
	long     mask;
	int      number;

	pnt = namehead;						// Find the NAMEDEF for the name
	while (pnt != NULL)
	{
		if (strncmp(name, pnt->name, 32) == 0)
			break;
		pnt = pnt->next;
	}
	if (pnt == NULL)					// If didn't find it, create one
	{
		pnt = (NAMEDEF *)malloc(sizeof(NAMEDEF));
		strncpy(pnt->name, name, 32);
		pnt->mask = 0;
		pnt->next = namehead;
		namehead = pnt;
	}
	mask = 0x01;
	number = 0;
	while ((pnt->mask & mask) != 0)		// Find a free unit number
	{
		mask <<= 1;
		number++;
	}
	if (mask == 0)
	{
		debugprint("in allocunitnumber: %s, FAIL", name);

		return (-1);
	}
	pnt->mask |= mask;

	debugprint("in allocunitnumber: %s, %d", name, number);

	return (number);
}


void giveunitnumber(
	char *name,
	int   number)

{
	NAMEDEF *pnt;

	pnt = namehead;
	while (pnt != NULL)
	{
		if (strncmp(name, pnt->name, 32) == 0)
		{
			pnt->mask &= (~(1 << number));
			return;
		}
		pnt = pnt->next;
	}
}


//***************************************************
// Function: checklke - Determine if an LKE is loaded
// Returned: TRUE if loaded, FALSE if  not
//***************************************************

int checklke(
	char *name)

{
    lkeinfo_data *lkelist;
    lkeinfo_data *lkepnt;
	long          size;
    long          rtn;
	
    while (TRUE)
    {
        if ((size = svcSysGetInfo(GSI_LKE, 0, NULL, 0)) < 0)
										// See how much room we need
            return (FALSE);
        size += (5 * sizeof(lkeinfo_data));
        if ((lkelist = (lkeinfo_data *)usbmalloc(size)) == NULL)
										// Get space for the data
			return (FALSE);
        if ((rtn=svcSysGetInfo(GSI_LKE, 0, lkelist, rtn)) < 0) // Get the data
		{
		    usbfree(lkelist);
            return (FALSE);
		}
        if (rtn > size == 0)			// Was it truncated?
			break;						// No
        usbfree(lkelist);				// Yes - give up the block and
    }									//   try again
    lkepnt = lkelist;
    while ((rtn -= sizeof(lkeinfo_data)) >= 0)
    {
		if (stricmp(lkepnt->name, name) == 0)
		{
		    usbfree(lkelist);
			return (TRUE);
		}
		lkepnt++;
    }
    usbfree(lkelist);
	return (FALSE);
}


//****************************************************************
// Function: servercmd - Send command to a server and get response
// Returned: Message type from final message (positive) if OK,
//				negative XOS error code if error (error message
//				has been logged)
//****************************************************************

long servercmd(
	char *symname,
	char *msgdst,
	char *msgtext,
	int   msglen)

{
	_Packed struct
	{	BYTE4PARM  timeout;
		LNGSTRPARM dstname;
		char       end;
	} outparm =
	{	{PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT    , 5 * ST_SECOND},
		{PAR_SET|REP_STR , 0, IOPAR_MSGRMTADDRS, NULL}
	};
	_Packed struct
	{	BYTE4PARM timeout;
		char      end;
	} inpparm =
	{	{PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT, 5 * ST_SECOND}
	};
	long rtn;
	char mbufr[128];

	debugprint("server cmd: %d [%s] %s\n", msgtext[0], msgdst, msgtext + 1);

	outparm.dstname.buffer = msgdst;
	outparm.dstname.bfrlen = outparm.dstname.strlen = strlen(msgdst);
	if (ipmhndl <= 0)
	{
		if ((ipmhndl = svcIoOpen(XO_IN|XO_OUT, "IPM:", NULL)) < 0)
		{
			logerror(ipmhndl, "Error opening IPM: to send request to %s",
					symname);
			return (ipmhndl);
		}
	}
	if ((rtn = svcIoOutBlockP(ipmhndl, msgtext, msglen, (char *)&outparm)) < 0)
	{
		if (rtn == ER_NTDEF)
			logerror(0, "Symbiont %s is not running", symname);
		else
			logerror(rtn, "Error sending request to symbiont %s", symname);
		return (rtn);
	}
    do
    {
        if ((rtn = svcIoInBlockP(ipmhndl, mbufr, sizeof(mbufr),
				(char *)&inpparm)) < 0)
		{
			logerror(rtn, "Error reading request from symbiont %s", symname);
            return (rtn);
		}
        if (mbufr[0] > 7)				// Valid message type?
		{
			logerror(ER_ISRSP, "Invalid message type %d received from %s",
					mbufr[0], symname);
			return (ER_ISRSP);
		}
		if ((mbufr[0] & 0x03) == 0x03)
		{
			mbufr[rtn] = 0;
			logerror(0, "Error reported by symbiont %s\n%s", symname,
					mbufr + 1);
		}
		else if (debugflg)
			debugprint("Response (%d) from %s", mbufr[0], symname, mbufr + 1);
	} while (mbufr[0] < MT_FINALMSG);
	return (mbufr[0]);
}


void procargerror(
	char *msg1,
	char *msg2)
{
	char text[300];

	if (msg2 != NULL)
	{
		snprintf(text, sizeof(text), "%s\n%s", msg1, msg2);
		sfwResponse(0, SFWRSP_SEND|SFWRSP_FINAL, text);
	}
	else
		sfwResponse(0, SFWRSP_SEND|SFWRSP_FINAL, msg1);
}



void message(
	int   level,
	char *text)
{
	level = level;

	logerror(0, "Error loading LKE %s:\n%s", lkename, text);
}
