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

#include <xos.h>
#include <xosx.h>
///#include <xossvc.h>
#include <xoslib.h>
#include <xoserr.h>
#include <ctype.h>
#include <stddef.h>

// This driver implements the XOS interprocess message system as a class driver
//   using the queued IO input and output datagram functions

// Opening the device IPM: with O$IN set allows the process to receive messages
//   sent to its process ID by doing input.  Opening the device IPM:foobaz
//   allows the process to receive messages sent to the special IPM address
//   "foobaz".  If O$OUT is set, the process can send messages by doing output.
//   The source of the message will be the processes process ID if no name was
//   used when the device was opened or the else the name that was used. Only
//   one IPM device without a name may be opened by a process.

#define MSGMAX 1024			// Maximum length of a PDA message

typedef struct ipmdcb__ IPMDCB;
typedef struct ipmmsg__ IPMMSG;

long XOSFNC ipminit(void);

static DEVCHKFNC   ipmdevchk;
static long XOSFNC ipmcleardcb(void);
static long XOSFNC ipmopena(void);
static long XOSFNC ipmopen(char *spec);
static long XOSFNC ipminpblk(void);
static long XOSFNC ipmoutblk(void);
static void XOSFNC ipmclose(void);
static long XOSFNC ipminfo(void);

static long XOSFNC finddst(char *name, long len, IPMDCB **pdcb);


static PARMGETFNC lclgetaddr;
static PARMSETFNC lclsetaddr;
static PARMGETFNC rmtgetpid;
static PARMGETFNC rmtgetaddrs;
static PARMSETFNC rmtsetaddrs;
static PARMGETFNC rmtgetaddrr;

long    ipmdcbnum;
long    ipmdcbmax;
long    ipmdcblmt = 10000;
IPMDCB *ipmhead;			// Head pointer for IPM name list

// Define structure for the IPM DCB

struct ipmdcb__
{	DCB;
	IPMMSG *ipmhead;		// Message queue head pointer
	IPMMSG *ipmtail;		// Message queue tail pointer
	IPMDCB *ipmnext;		// Address of next named IPM DCB
	IPMDCB *ipmprev;		// Address of previous named IPM DCB
	char    ipmsts;
	char    ipmourlen;		// Length of our IPM name
	char    ipmourname[34];	// Our IPM name
	PDA    *ipmpda;
	char    fill[1];
	char    ipmdstlen;		// Length of destination name
	char    ipmdstname[34];	// Destination name
};

// Define bits for dcb_ipmsts

#define IPM_MSGWAIT 0x40	// Waiting for message to receive

// Define structure for an IPM message block

struct ipmmsg__
{	long    label;			// Label = 'IPM*'
	IPMMSG *next;			// Address of next IPMMSG
	long    pid;			// PID of sender of message
	char    offset;			// Offset from src of start of message data
	char    fill;
	ushort  length;			// Length of message data
	char    src[];			// Source name
};

// SVC dispatch table for IPM class devices

static const QFDISP ipmqfdisp =
{	NULL,					// Mount
	ipmcleardcb,			// Clear DCB
	ipmopena,				// Open additional
	ipmopen,				// Open
	NULL,					// Find file
	NULL,					// Delete
	NULL,					// Rename
	ipminpblk,				// Input block
	ipmoutblk,				// Output block
	0,						// Reserved

	0,						// Special functions
	ipmclose,				// Close
	0,						// Get device label
	0,						// Commit data
	ipminfo,				// Get device info
	0						// Verify disk changed
};

// Class characteristics tables for the IPM device class

static INFO(msgnumber , "Number of in use IPM devices");
static INFO(msgmaximum, "Maximum number of in use IPM devices");
static INFO(msglimit  , "Maximum number of IPM devices allowed");

static CHARTBL ipmdctbl =
{ CTBEGIN(sysChrValuesNew),
 {CTITEM(NUMBER  , DECV, U, 4, &msgnumber , sysChrGetULong, NULL        ,
		&ipmdcbnum),
  CTITEM(MAXIMUMX, DECV, U, 4, &msgmaximum, sysChrGetULong, sysChrSetULong,
		&ipmdcbmax),
  CTLAST(LIMIT   , DECV, U, 4, &msglimit  , sysChrGetULong, sysChrSetULong,
		&ipmdcblmt)
 }
};

// Device parameter tables for IPM class devices

static PARMTBL2 ipmipmparams = {2,
{	PTITEM( , 0, NULL     , NULL),		// 0x0600
	PTITEM(U, 4, rmtgetpid, NULL),		// 0x0601 - IOPAR_IPMRMTPID
}};

static PARMTBL2 ipmdgparams = {4,
{	PTITEM( ,  0, NULL       , NULL),			// 0x0700
	PTITEM(L, 34, lclgetaddr , NULL),			// 0x0701 - DGLCLADDR
	PTITEM(L, 34, rmtgetaddrs, rmtsetaddrs),	// 0x0702 -	DGRMTADDRS
	PTITEM(L, 34, rmtgetaddrr, NULL)			// 0x0703 - DGRMTADDRR
}};

static PARMTBL1 ipmparams = {8,
{	&knlIoStdGenParams,		// 0x00xx - Generic IO parameters
	&knlIoDfltFilParams,	// 0x01xx - File system IO parameters
	&knlIoDfltTrmParams,	// 0x02xx - Terminal IO parameters
	NULL,					// 0x03xx - Disk IO parameters
	NULL,					// 0x04xx - Tape IO parameters
	NULL,					// 0x05xx - Network IO parameters
	&ipmipmparams,			// 0x06xx - IPM IO parameters
	&ipmdgparams			// 0x07xx - Datagram IO parameters
}};

// Define the CCB for the IPM class

CCB ipmccb =
{	'CCB*',					// label
	NULL,					// Address of next CCB
	"IPM",					// Name of this class
	0xFFFFFFFF,				// Name prefix part mask
	'IPM',					// Name prefix part value
	ipmdevchk,				// Address of device check routine
	NULL,					// Address of class func disp table
	&ipmdctbl				// Address of class char table
};


// Define structure used for transfer parameter values

typedef struct
{	long   *rmtpidvp;		// Address for remote PID value
	char   *rmtarbfr;		// Address of remote address (receive) buffer
	ushort *rmtarszp;		// Address of remote address (receive) length value
	int     rmtarlen;		// Length of remote address (receive) buffer
} XFRDATA;


//*******************************************************************
// Function: ipminit - Initializaton routine for the IPM device
// Returned:  0 if no match, 1 if found a match, 2 if need to restart
//            search, or a negative XOS error code if error.
//*******************************************************************

#pragma code_seg (o_CODE, o_CODE);

long XOSFNC ipminit(void)
{
	long rtn;

	sysChrFixUp(&ipmdctbl);
	if ((rtn = sysIoNewClass(&ipmccb)) < 0)
		CRASH('CIMC');
	return (0);
}

#pragma code_seg ();

#pragma data_seg (_TBL, o_CODE);
long XOSFNC (*ipmpnt)(void) = ipminit;
#pragma data_seg ();


//*******************************************************************
// Function: ipmdevchk - Device check routine for the NULL device
// Returned:  0 if no match, 1 if found a match, 2 if need to restart
//            search, or a negative XOS error code if error.
//*******************************************************************

// Since the device name is only 3 characters, the only check for a matching
//   name needed has already been done. Thus we never return 0 here. We also
//   never need to restart so we never return 2 either.

static long XOSFNC ipmdevchk(
	char  name[16],			// Device name (16 bytes, 0 filled)
	char *path,				// Path specification (buffer must be at least
							//   FILESPCSIZE + 1 bytes long)
	long  rtndcb)			// TRUE if should return a DCB
{
	DCB *dcb;
	long rtn;

	if (ipmdcbnum >= ipmdcblmt)
		return (ER_TMDVC);
	if ((rtn = sysIoGetDymDcb(name, DS_DUPLEX|DS_QIN|DS_DOUT, sizeof(IPMDCB),
			&dcb)) < 0 || (rtn = sysIoLinkDcb(dcb, &ipmccb)) < 0)
		return (rtn);
	if (++ipmdcbnum > ipmdcbmax)
		ipmdcbmax = ipmdcbnum;
	dcb->qfdisp = &ipmqfdisp;
	dcb->devchar = &ipmdctbl;
	knlTda.dcb = dcb;
	return (1);
}


typedef long XOSFNC PARMFNC(PINDEX inx, char *val, long cnt, ushort *cpnt,
		void *data);


//*****************************************************************
// Function: rmtgetpid - Get the IOPAR_IPMRMTPID (0x0601) parameter
//*****************************************************************

static long XOSFNC rmtgetpid(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	((XFRDATA *)data)->rmtpidvp = (long *)val;
	return (0);
}


//******************************************************************
// Function: lclgetaddr - Get the IOPAR_DGLCLADDR (0x0701) parameter
//******************************************************************

static long XOSFNC lclgetaddr(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysLibMemCpy(val, ((IPMDCB *)knlTda.dcb)->ipmourname,
			((IPMDCB *)knlTda.dcb)->ipmourlen));
}


//********************************************************************
// Function: rmtgetaddrs - Get the IOPAR_DGRMTADDRS (0x0702) parameter
//********************************************************************

static long XOSFNC rmtgetaddrs(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	long rtn;

	if ((rtn = sysLibStrMov(val, ((IPMDCB *)knlTda.dcb)->ipmdstname)) < 0)
		rtn = sysUSPutShort((short *)cpnt, rtn);
	return (rtn);
}


//********************************************************************
// Function: rmtsetaddrs - Set the IOPAR_DGRMTADDRS (0x0702) parameter
//********************************************************************

static long XOSFNC rmtsetaddrs(
	PINDEX  inx,
	char   *val,
	long    cnt,
	void   *data)
{
	long rtn;

	if ((rtn = sysLibStrNLen(val, cnt)) < 0)
		return (rtn);
	if (rtn > 32)
		return (ER_PARMS);
	if ((rtn = sysLibMemCpy(((IPMDCB *)knlTda.dcb)->ipmdstname, val,
			rtn + 1)) < 0)
	{
		((IPMDCB *)knlTda.dcb)->ipmdstname[0] = 0;
		return (rtn);
	}
	return (0);
}


//********************************************************************
// Function: rmtgetaddrr - Get the IOPAR_DGRMTADDRR (0x0703) parameter
//********************************************************************

static long XOSFNC rmtgetaddrr(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	((XFRDATA *)data)->rmtarbfr = val;
	((XFRDATA *)data)->rmtarlen = cnt;
	((XFRDATA *)data)->rmtarszp = cpnt;
	return (0);
}


//**********************************************************//
// Function: ipmcleardcb - Clear DCB routine for IPM device //
// Returned: ER_BUSY                                        //
//**********************************************************//

static long XOSFNC ipmcleardcb(void)
{
	ipmdcbnum--;
	sysIoGiveDymDcb(knlTda.dcb);
	return (0);
}


//************************************************************
// Function: ipmopena - Open additional routine for IPM device
// Returned: ER_BUSY
//************************************************************

// Since an IPM device is closely coupled with the process that creates it,
//   it makes no sense to let it be associated with a different process.

static long XOSFNC ipmopena(void)
{
	return (ER_BUSY);
}


//*************************************************************
// Function: ipmopen - Open routine for IPM device
// Returned:  0 if normal or a negative XOS error code if error
//*************************************************************

static long XOSFNC ipmopen(
	char *spec)
{
	IPMDCB *ipmdcb;
	IPMDCB *junk;
	char   *pnt;
	long    rtn;
	long    len;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&ipmparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		return (rtn);
	ipmdcb = (IPMDCB *)knlTda.dcb;
	ipmdcb->ipmpda = knlPda.addr;
	if (spec[0] == 0)					// Was a name specified?
	{									// No - store our PID as the IPM name
		pnt = ipmdcb->ipmourname;
		pnt += sysLibDec2Str(pnt, knlPda.pid >> 16);
		*pnt++ = '.';
		pnt += sysLibDec2Str(pnt, knlPda.pid & 0xFFFF);
		ipmdcb->ipmourlen = pnt - ipmdcb->ipmourname;
	}
	else								// If a name was given, use it
	{
		if ((len = sysLibStrLen(spec)) < 0)
			return (len);
		if (len > 32)
			return (ER_BDSPC);
		if ((rtn = sysLibStrNMovX(ipmdcb->ipmourname, spec, 32)) < 0)
			return (rtn);
		ipmdcb->ipmourlen = len;
	}
	if ((rtn = finddst(ipmdcb->ipmourname, ipmdcb->ipmourlen, &junk)) !=
			ER_NTDEF)
		return (ER_ALDEF);
	if ((ipmdcb->ipmnext = ipmhead) != NULL)
		ipmhead->ipmprev = ipmdcb;
	ipmhead = ipmdcb;
	knlTda.amount = 1;
	return (0);
}


//**********************************************************
// Function: ipminpblk - Input block routine for IPM device
// Returned:  Amount input if normal or a negative XOS error
//			  code if error.
//**********************************************************

static long XOSFNC ipminpblk(void)
{
	IPMMSG *msg;
	long    rtn;
	long    len;
	XFRDATA xdata;

	xdata.rmtpidvp = NULL;
	xdata.rmtarbfr = NULL;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&ipmparams,
			&knlIoMinDDParams, &xdata, 0)) < 0)
		return (rtn);
	if (knlTda.count > 0)
	{
		if ((knlTda.cmd & XO_IN) == 0)
		{
			knlTda.error = ER_NOIN;
			knlTda.status |= QSTS_DONE;
			return (0);
		}
		TOFORK;
		while ((msg = ((IPMDCB *)knlTda.dcb)->ipmhead) == NULL)
		{
			((IPMDCB *)knlTda.dcb)->ipmsts |= IPM_MSGWAIT;
			if ((rtn = sysIoWait(knlTda.timeout, THDS_DW2)) < 0)
			{
				knlTda.error = rtn;
				knlTda.status |= QSTS_DONE;
				return (0);
			}
			TOFORK;
		}

		// Here with a message available

		if (knlTda.buffer1 != NULL &&	// Do we really want the message now?
				(((IPMDCB *)knlTda.dcb)->ipmhead = msg->next) == NULL)
			((IPMDCB *)knlTda.dcb)->ipmtail = NULL;
		FROMFORK;
		if (xdata.rmtpidvp != NULL)		// Does he want the remote PID?
			sysUSPutLong(xdata.rmtpidvp, msg->pid);
		if (xdata.rmtarbfr != NULL)		// Do we have a buffer for the source
		{								//   string?
			if (msg->src[0] == 0xFF)
				len = 5;
			else if ((len = sysLibStrLen(msg->src)) > xdata.rmtarlen)
				len = xdata.rmtarlen;
			sysLibMemCpy(xdata.rmtarbfr, msg->src, len + 1);
			sysUSPutShort((short *)xdata.rmtarszp, len);
		}
		if (knlTda.buffer1 != NULL)
		{
			if ((len = msg->length) > knlTda.count) // Is his buffer big enough?
			{
				len = knlTda.count;			// No - just give him what we can
				knlTda.status |= QSTS_TRUNC;//   and indicate truncation
			}
			rtn = sysLibMemCpy(knlTda.buffer1, msg->src + msg->offset,
					len);
			sysMemGiveXmb(msg);
			if (rtn < 0)
				knlTda.error = rtn;
			else
				knlTda.amount = len;
		}
		else
			knlTda.amount = len;
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}


//***********************************************************
// Function: ipmoutblk - Output block routine for IPM device
// Returned:  Amount output if normal or a negative XOS error
//			  code if error.
//***********************************************************

static long XOSFNC ipmoutblk(void)
{
	long    rtn;
	XFRDATA xdata;

	xdata.rmtpidvp = NULL;
	xdata.rmtarbfr = NULL;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&ipmparams,
			&knlIoMinDDParams, &xdata, 0)) < 0)
	{
		knlTda.error = rtn;
		knlTda.status |= QSTS_DONE;
		return (0);
	}

	if (knlTda.count > 0)
	{
		if ((knlTda.cmd & XO_OUT) == 0)
		{
			knlTda.error = ER_NOOUT;
			knlTda.status |= QSTS_DONE;
			return (0);
		}
		if (knlTda.count > MSGMAX)
		{
			knlTda.error = ER_ICDEV;
			knlTda.status |= QSTS_DONE;
			return (0);
		}

		TOFORK;
		rtn = sysIpmSendMsg(((IPMDCB *)knlTda.dcb)->ipmdstname,
				((IPMDCB *)knlTda.dcb)->ipmourname, knlPda.pid,
				knlTda.buffer1, knlTda.count);
		FROMFORK;
		if (rtn < 0)
		{
			knlTda.error = rtn;
			knlTda.status |= QSTS_DONE;
			return (0);
		}


		if (xdata.rmtpidvp != NULL)		// OK - does he want the remote PID?
			sysUSPutLong(xdata.rmtpidvp, rtn);

		knlTda.amount = knlTda.count;	// Return length of the message
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}


//*************************************************************
// Function: ipmclose - Close routine for IPM device
// Returned:  0 if normal or a negative XOS error code if error
//*************************************************************

static void XOSFNC ipmclose(void)
{
	IPMMSG *next;
	long    rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&ipmparams,
			&knlIoMinDDParams, NULL, 0)) < 0 && knlTda.error >= 0)
		knlTda.error = rtn;

	// Remove the DCB from the IPM list

	if (((IPMDCB *)knlTda.dcb)->ipmprev != NULL)
		((IPMDCB *)knlTda.dcb)->ipmprev->ipmnext =
				((IPMDCB *)knlTda.dcb)->ipmnext;
	else
		ipmhead = ((IPMDCB *)knlTda.dcb)->ipmnext;
	if (((IPMDCB *)knlTda.dcb)->ipmnext != NULL)
		((IPMDCB *)knlTda.dcb)->ipmnext->ipmprev =
				((IPMDCB *)knlTda.dcb)->ipmprev;

	// Give up any messages queued to this DCB

	while (((IPMDCB *)knlTda.dcb)->ipmhead != NULL)
	{
		next = ((IPMDCB *)knlTda.dcb)->ipmhead->next;
		sysMemGiveXmb(((IPMDCB *)knlTda.dcb)->ipmhead);
		((IPMDCB *)knlTda.dcb)->ipmhead = next;
	}
}


//*******************************************************
// Function: ipminfo - Device info routine for IPM device
// Returned:  Length of string stored if normal or a
//			  negative XOS error code if error
//*******************************************************

static long XOSFNC ipminfo(void)
{
	return (sysIoInfoSimple((((IPMDCB *)knlTda.dcb)->ipmourname == NULL) ?
			"No name" : ((IPMDCB *)knlTda.dcb)->ipmourname));
}


//************************************************************************
// Function: sysIpmSendLogInMsg - Send login message to the login symbiont
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************************

long XOSFNC sysIpmSendLogInMsg(
	char *trmname,			// Terminal name
	long  detect,			// Auto rate detect type
	long  data,				// Character + reason
							//   0x00xx = Data
							//   0x01xx = Carrier detect
							//   0x02xx = Ring indicate
							//   0x20xx = X.25 network connection
							//   0x30xx = TCP network connection
							//   0x31xx = RCP network connection
	char *srcstr,			// Address of source sring (without prefix)
	char *dststr)			// Address of destination string
{
	int  mlen;
	char msg[24];
	char dstbfr[40];

	msg[0] = MT_TERMDATA;				// Store message type
	msg[1] = (char)(data >> 8);			// Store reason
	msg[2] = (char)data;				// Store data byte
	msg[3] = (char)detect;				// Store auto-rate detect type
	mlen = sysLibStrNMov(msg + 4, trmname, 16) + 4; // Store device name
	msg[20] = 0;
	*(long *)dstbfr = 'SYS^';			// Store prefix destination string
	sysLibStrNMov(dstbfr + 4, dststr, 32);
	dstbfr[36] = 0;
	return (sysIpmSendMsg(dstbfr, srcstr, 0, msg, mlen));
}


//**************************************************
// Function: sysIpmSendMsg - Send message to process
// Returned: Remote PID (positive) if normal or a
//			 negative XOS error code if error
//**************************************************

// Must be called at fork level

long XOSFNC sysIpmSendMsg(
	char *dststr,			// Address of destination string
	char *srcstr,			// Address of source string
	long  srcpid,			// Source PID
	char *msgbfr,			// Address of message buffer
	long  msglen)			// Length of message
{
	IPMDCB *dstdcb;
	IPMMSG *msg;
	long    rtn;
	int     offset;
	int     srclen;
	int     dstlen;

	REQUIREFORK;
	dstlen = (dststr[0] == 0xFF) ? 5 :	// Get length of the destination string
			sysLibStrLen(dststr);
	if ((rtn = finddst(dststr, dstlen, &dstdcb)) < 0) // Find destination DCB
		return (rtn);
	srclen = (srcstr[0] == 0xFF) ? 5 :	// Get length of the source string
			sysLibStrLen(srcstr);
	offset = (srclen + 4) & 0xFFFFFFFC;
	if ((rtn = sysMemGetXmb(offsetof(IPMMSG, src) + offset + ((msglen + 3) &
			0xFFFFFFFC), (char **)&msg)) < 0) // Allocat an XMB for the message
		return (rtn);
	msg->label = 'IPM*';				// Store the header stuff
	msg->pid = srcpid;
	msg->offset = offset;
	msg->length = msglen;
	if ((rtn = sysLibMemCpy(msg->src, srcstr, srclen + 1)) < 0)
	{									// Store the source string
		sysMemGiveXmb(msg);
		return (rtn);
	}
	if ((rtn = sysLibMemCpy(msg->src + offset, msgbfr, msglen)) < 0)
	{									// Store the message text
		sysMemGiveXmb(msg);
		return (rtn);
	}
	msg->next = NULL;					// Link the message to the end of the
	if (dstdcb->ipmtail == NULL)		//   destination message list
		dstdcb->ipmhead = msg;
	else
		dstdcb->ipmtail->next = msg;
	dstdcb->ipmtail = msg;

	if (dstdcb->ipmsts & IPM_MSGWAIT)	// Is he waiting for a message?
		sysIoResumeInput((DCB *)dstdcb, 0); // Yes - wake him up
	return (dstdcb->ipmpda->pid);
}


static long XOSFNC finddst(
	char    *name,
	long     len,
	IPMDCB **pdcb)
{
	IPMDCB *ipmdcb;

	ipmdcb = ipmhead;					// Point to first IPM DCB

	while (ipmdcb != NULL)
	{
		if (len == ipmdcb->ipmourlen && sysLibStrNICmp(name,
				ipmdcb->ipmourname, 32) == 0)
		{
			*pdcb = ipmdcb;
			return (0);
		}
		ipmdcb = ipmdcb->ipmnext;
	}
	return (ER_NTDEF);
}
