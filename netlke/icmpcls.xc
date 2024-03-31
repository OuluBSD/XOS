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
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES//  LOSS
//   OF USE, DATA, OR PROFITS//  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

// This device class implements the Internet ICMP protocol. This is a transport
//   level protocol layered under IP. Its primary purpose is to provide
//   various support capabilities for IP but can also be used indepedently
//   in a few instances. The ICMPn: device can be used to send and receive
//   ECHO and TIMESTAMP messages. There are no higher level protocols supported
//   by ICMP.

#include <stddef.h>
#include <ctype.h>
#include <xos.h>
#include <xoslib.h>
#include <xoserr.h>
#include <xosx.h>
#include <xosxchar.h>
#include <xosxparm.h>
#include <xosxlke.h>
#include <xosnet.h>
#include <xosxnet.h>
#include <xosxip.h>
#include <xosxicmp.h>

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0

static INITLKE  initicmp;
static OUTDNFNC outblkdone;

static long XOSFNC icmprecvpkt(ICMPTPDB *tpdb, NPB *npb);
static long XOSFNC icmpsendpkt(ICMPTPDB *tpdb, ICMPDDCB *dcb, long dstaddr,
		long type, long h2, char *data, long len, OUTDNFNC *outdn);


// Define structure used to save information about parameter request

typedef struct
{	char *ipapnt;		// Address for storing remote IP address
	long  ipacnt;		// Size for remode IP address
} IOPDATA;


#define ourdcb ((ICMPDDCB *)knlTda.dcb)

#pragma data_seg(_HEAD);
	LKEHEAD(ICMPCLS, initicmp, MAJVER, MINVER, EDITNUM, CLASS); 
#pragma data_seg();

static long icmpdcbnum;			// Number of in use ICMP DCBs
static long icmpdcbmax;			// Maximum in use ICMP DCBs
static long icmpdcblmt = 10000;	// Maximum number of ICMP DCBs allowed

ICMPTPDB *icmpfirsttpdb;

// Transport level dispatch table for ICMP

static TPDBDISP icmpdisp =
{	NULL,				// Link up
	NULL,				// Link down
///	icmpabort			// Abort output
///	rtgetaddr,			// getaddr function for protocol
};

//**************************************************
// Device IO parameter tables for ICMP class devices
//**************************************************

static PARMGETFNC getrmtnetar;

// Network IO parameter table (0x05xx)

static PARMTBL2 udpnetparams = {19,
{ PTITEM( ,  0, NULL, NULL),			//                   = 0x0500
  PTITEM( ,  0, NULL, NULL),			//                   = 0x0501
  PTITEM(U,  4, NULL, NULL),			//                   = 0x0502
  PTITEM(U,  4, NULL, NULL),			// IOPAR_NETLCLPORT  = 0x0503
  PTITEM(U,  8, NULL, NULL),			// IOPAR_NETRMTHWAS  = 0x0504
  PTITEM(U,  8, NULL, NULL),			// IOPAR_NETRMTHWAR  = 0x0505
  PTITEM(U,  4, xosipGetRmtNetAS, xosipSetRmtNetAS),
										// IOPAR_NETRMTNETAS = 0x0506
  PTITEM(U,  4, getrmtnetar, NULL),		// IOPAR_NETRMTNETAR = 0x0507
  PTITEM(U,  4, NULL, NULL),			// IOPAR_NETRMTPORTS = 0x0508
  PTITEM(U,  4, NULL, NULL),			// IOPAR_NETRMTPORTR = 0x0509
  PTITEM(L, 32, NULL, xosipSetRmtName),	// IOPAR_NETRMTNAME  = 0x050A
  PTITEM(U,  4, NULL, NULL),			// IOPAR_NETSMODE    = 0x050B
  PTITEM(U,  4, NULL, NULL),			// IOPAR_NETCMODE    = 0x050C
  PTITEM( ,  0, NULL, NULL),		    // IOPAR_NETRCVWIN   = 0x050D
  PTITEM(U,  4, xosipGetLclNetA, NULL),	// IOPAR_NETLCLNETA  = 0x050E
  PTITEM(U,  4, NULL, NULL),			// IOPAR_NETKATIME   = 0x050F
  PTITEM( ,  0, NULL, NULL),			// IOPAR_NETCONLIMIT = 0x0510
  PTITEM(U,  4, NULL, NULL),			// IOPAR_NETCONHNDL  = 0x0511
  PTITEM(U,  4, xosipGetMaxPktSz, xosipSetMaxPktSz),
										// IOPAR_NETPKTMAX   = 0x0512
  PTITEM(U,  4, xosipGetInqLimit, xosipSetInqLimit)
										// IOPAR_NETINQLIMIT = 0x0513
}};

// Top level table

static PARMTBL1 icmpparams = {6,
{	&knlIoStdGenParams,		// 0x00xx - Generic IO parameters
	&knlIoDfltFilParams,	// 0x01xx - File system IO parameters
	&knlIoDfltTrmParams,	// 0x02xx - Terminal IO parameters
	NULL,					// 0x03xx - Disk IO parameters
	NULL,					// 0x04xx - Tape IO parameters
	&udpnetparams			// 0x05xx - Network IO parameters
}};



//*********************************************
// Device dispatch table for ICMP class devices
//*********************************************

static long XOSFNC icmpgivedcb(void);
static long XOSFNC icmpopen(char *spec);
static long XOSFNC icmpoutblk(void);
static long XOSFNC icmpinpblk(void);

static const QFDISP icmpqfdisp =
{	NULL,				// Mount
	icmpgivedcb,		// Clear DCB
	NULL,				// Open additional
	icmpopen,			// Open device/file
	NULL,				// Device parameters
	NULL,				// Delete file
	NULL,				// Rename file
	icmpinpblk,			// Input block
	icmpoutblk,			// Output block
	NULL,				// Get input/output status
	NULL,				// Special device function
	sysIoCloseFin,		// Close file
	NULL,				// Get device label
	NULL,				// Commit data to disk
	NULL,				// Get device info
	NULL				// Verify changed disk
};


// Class characteristics tables for the ICMP device class

SINFO(msgnumber , "Number of in use ICMP devices");
SINFO(msgmaximum, "Maximum number of in use ICMP devices");
SINFO(msglimit  , "Maximum number of ICMP devices allowed");

static const CHARTBL icmpcctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(NUMBER  , DECV, U, 4, &msgnumber , sysChrGetULong, NULL        ,
		&icmpdcbnum),
  CTITEM(MAXIMUMX, DECV, U, 4, &msgmaximum, sysChrGetULong, sysChrSetULong,
		&icmpdcbmax),
  CTLAST(LIMIT   , DECV, U, 4, &msglimit  , sysChrGetULong, sysChrSetULong,
		&icmpdcblmt)
}};

// Class function dispatch table for ICMP class devices

static long XOSFNC icmpaddunit(void); 

static CFDISP icmpcls = { 3,
{	icmpaddunit,			// CF_ADDUNIT =  8 - Add unit
	NULL,				// CF_PUNITS  =  9 - Get information about
						//                     physical units
	NULL				// CF_AUNITS  = 10 - Get information about active
}};


static const CHARTBL icmpdctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(CLASS   , TEXT, U,  8, &knlChrMsgClass  , sysChrDevGetClass,
		sysChrDevChkClass, 0),

  CTITEM(IPDEV   , TEXT, U, 20, &xosipMsgIpDev  , xosipGetIpDev,
		NULL             , 0),

  CTITEM(PROTOCOL, DECV, U,  4, &xosipMsgIpProt , xosipGetTpdb4,
		NULL             , offsetof(ICMPTPDB, tplb.protocol)),

  CTITEM(PKTIN   , DECV, U,  4, &xosnetMsgPktIn  , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(ICMPTPDB, cntpktin)),

  CTITEM(BYTEIN  , DECV, U,  4, &knlChrMsgByteIn , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(ICMPTPDB, cntbytein)),

  CTITEM(PKTOUT  , DECV, U,  4, &xosnetMsgPktOut , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(ICMPTPDB, cntpktout)),

  CTITEM(BYTEOUT , DECV, U,  4, &knlChrMsgByteOut, xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(ICMPTPDB, cntbyteout)),

///  CTITEM(BADADDR , DECV, U,  4, &msgipbadaddr    , xosipGetTpdb4,
///		xosipSetTpdb4   , offsetof(ICMPTPDB, ipcntbadaddr)),

  CTITEM(BADHDR  , DECV, U,  4, &xosnetMsgBadHdr , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(ICMPTPDB, cntbadhdr)),

  CTITEM(PSLTMN  , DECV, U,  4, &xosnetMsgPSLtMn , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(ICMPTPDB, cntpsltmn)),

  CTLAST(PSLTDL  , DECV, U,  4, &xosnetMsgPSLtDL , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(ICMPTPDB, cntpsltdl)),

///  CTLAST(DLLTHL  , DECV, U,  4, &xosnetMsgDLLtHL , xosipGetTpdb4,
///		xosipSetTpdb4   , offsetof(ICMPTPDB, ipcntdllthl))
}};


//*************
// The ICMP CCB
//*************

static DEVCHKFNC icmpdevchk;

CCB icmpccb =
{	'CCB*',
	0,					// next    - Address of next CCB
	"ICMP",				// name    - Name of this class
	0xFFFFFFFF,			// npfxmsk - Name prefix part mask
	'ICMP',				// npfxval - Name prefix part value
	icmpdevchk,			// devchk  - Address of device check function
	&icmpcls,			// fdsp    - Address of class func disp table
	&icmpcctbl			// clschar - Address of class char table
};


static const CHARITEM icmpaublk[] =
{	CTITEM(UNIT    , DECV, U, 4, NULL, NULL, xosnetAUUnit    , 0),
	CTITEM(NETDEV  , DECV, U, 4, NULL, NULL, xosnetAUNetDev  , 0),
	CTITEM(PROTOCOL, DECV, U, 4, NULL, NULL, xosnetAUProtocol, (1<<16)+0xFF),
	CTLAST(PRIORITY, DECV, U, 4, NULL, NULL, xosnetAUPriority, 0)
};


//***********************************************************
// Function: initcimp - Initialization routine
// Returned: 0 if normal or a negative XOS error code if erro
//***********************************************************

#pragma code_seg ("x_ONCE");

static long XOSFNC initicmp(
	char **pctop,
	char **pdtop)
{
	long rtn;

	sysChrFixUp((CHARTBL *)&icmpcctbl);
	sysChrFixUp((CHARTBL *)&icmpdctbl);
	sysChrFixUp((CHARTBL *)&icmpaublk);
	if ((rtn = sysIoNewClass(&icmpccb)) >= 0)
		*pctop = codetop;
	return (rtn);
}

#pragma code_seg ();


//************************************************************
// Function: icmpaddunit - Implements the CL_ADDUNIT function
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC icmpaddunit(void)
{
	ICMPTPDB *tpdb;
	NETAUDATA data;
	long      rtn;

	if (knlTda.buffer2 == NULL)
		return (ER_CHARM);
	data.dev.l[0] = -1;
	data.unit = -1;
	data.protocol = -1;
	data.priority = 0x7FFF;
	if ((rtn = sysChrValuesNew(TRUE, icmpaublk, &data)) < 0)
		return (rtn);
	if ((data.unit | data.dev.l[0] | data.protocol) == -1)
		return (ER_CHARM);
	if ((rtn = xosnetMakeTransport(data.dev.c, "ICMP", data.unit,
			data.protocol, offsetof(ICMPPKT, data), sizeof(ICMPTPDB),
			(RECVFNC *)icmprecvpkt, &icmpdisp, &icmpccb, (TPDB **)&tpdb)) < 0)
		return (rtn);
	tpdb->devdesc = DS_DUPLEX|DS_QIN|DS_QOUT|DS_DINDFT|DS_DOUTDFT|DS_DIN|
			DS_DOUT;
	xosnetLinkTplb(tpdb->npdb, &tpdb->tplb, data.priority);
	tpdb->nextp = (TPDB *)icmpfirsttpdb;
	icmpfirsttpdb = (ICMPTPDB *)tpdb;
	tpdb->nexts = xosnetFirstTpdb;
	xosnetFirstTpdb = (TPDB *)tpdb;
	sysMemGiveXRes();
	knlTda.amount = 1;
	knlTda.status |= QSTS_DONE;
	knlTda.dcb = NULL;
	return (0);
}

//=====================
// End of addunit stuff
//=====================


//==================================================================
// Following are the standard functions that implement an XOS device
//==================================================================


//********************************************************************
// Function: icmpdevchk - Device check function for ICMP class devices
// Returned: 0 if no match, 1 if found a match, 2 if need to restart
//           search, or a negative XOS error code if error.
//********************************************************************

static long XOSFNC icmpdevchk(
	char  name[16],		// Device name (16 bytes, 0 filled)
	char *path,			// Path specification
	long  rtndcb)
{
	ICMPTPDB *tpdb;
	long      rtn;
	int       unit;

	if (isdigit(name[4]))
	{
		if (name[5] == 0)
			unit = name[4] & 0x0F;
		else if (isdigit(name[5]) && name[6] == 0)
			unit = (name[4] & 0x0F) * 10 + (name[5] & 0x0F);
		else
			return (0);
		tpdb = icmpfirsttpdb;
		while (tpdb != NULL)
		{
			if (unit == tpdb->unit)
			{
				if ((rtn = xosnetMakeDcb(name, unit, sizeof(ICMPDDCB), &icmpccb,
						tpdb->idb, &icmpdctbl, &icmpqfdisp,
						tpdb->devdesc)) < 0)
					return (rtn);
				ourdcb->icmpinqmax = 10;
				ourdcb->npdb = (NPDB *)tpdb->npdb;
				ourdcb->tpdb = (TPDB *)tpdb;
				return (1);
			}
			tpdb = (ICMPTPDB *)tpdb->nextp;
		}
		return (ER_NSDEV);
	}
	return (0);
}


//************************************************************
// Function: icmpopen - Handle the sd_open entry - Open device
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC icmpopen(
	char *spec)
{
	long  rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&icmpparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		return (rtn);

	return ((spec[0] != 0) ? xosipParseAddress(spec, FALSE) : 0);
}


//***************************************************************
// Function: icmpgivedcb - Handle the sd-cdcb entry - Give up DCB
// Returned: 0 if normal or a negative XOS error code if error
//***************************************************************

static long XOSFNC icmpgivedcb(void)
{
	icmpdcbnum--;
	sysIoGiveDymDcb(knlTda.dcb);
	knlTda.dcb = NULL;
	return (0);
}


//*****************************************************************
// Function: icmpoutblk - Handle the sd_outblk entry - Output block
// Returned: 0 if normal or a negative XOS error code if error
//*****************************************************************

// Only PING (echo request) packets can be output. No retries are done. Both
//   the ID and sequence number fields in the packet are used by the system
//   to associate packets with DCBs. If it is necessary to associate replies
//   with individual packets sent, the data provided must include the necessary
//   ID sequences. A destination IP address must be or have been specified.

// If QSTS_WAIT is not set, this function returns immediately after the
//   packet is queued to be output. No indication is provided when output
//   is complete. If QSTS_WAIT is set, the function does not return until
//   the packet has been output. If doing direct IO this wait is not
//   interruptable.


static long XOSFNC icmpoutblk(void)
{
	long rtn;

	knlTda.timeout = -1;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&icmpparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		knlTda.error = rtn;
	else if (ourdcb->rmtnetas == 0)
		knlTda.error = ER_NILAD;
	else if (knlTda.count > 0)
	{
		if (knlTda.count > 1400)		// Make sure it's not too big!
			knlTda.count = 1400;
		ourdcb->outpndcnt++;
		if ((rtn = icmpsendpkt((ICMPTPDB *)ourdcb->tpdb, ourdcb,
				ourdcb->rmtnetas, ICMPT_ECHOREQ, ourdcb->id, knlTda.buffer1,
				knlTda.count, outblkdone)) < 0)
			knlTda.error = rtn;
		if (knlTda.func & QFNC_WAIT)
		{
			TOFORK;
			while (ourdcb->outpndcnt > 0)
			{
				if ((rtn = sysIoWait(knlTda.timeout, THDS_DW2)) < 0)
				{
					knlTda.error = rtn;
					knlTda.status |= QSTS_DONE;
					return (0);
				}
				TOFORK;
			}
			FROMFORK;
		}
		knlTda.amount = knlTda.count;
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}


//************************************************************
// Function: outblkdone - Output done function for ICMP output
// Returned: Nothing
//************************************************************

static void XOSFNC outblkdone(
	long code,
	IDB *idb,
	NPB *npb)
{
	ICMPDDCB *dcb;

	dcb = (ICMPDDCB *)npb->dcb;
	dcb->outpndcnt--;
	if (dcb->outthrd != NULL && dcb->outthrd->state == THDS_DW2)
		sysIoResumeThread(dcb->outthrd, code);
	xosnetGiveNpb(npb);
}


//****************************************************************
// Function: icmpinpblk - Handle the sd_outblk entry - Input block
// Returned: 0 if normal or a negative XOS error code if error
//****************************************************************

// Only PING response (echo response) packets received in response to an echo
//   request output by the same device can be input.

static long XOSFNC icmpinpblk(void)
{
	NPB     *npb;
	ICMPPKT *pkt;
	long     rtn;
	long     len;

	knlTda.timeout = -1;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&icmpparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		knlTda.error = rtn;
	else if (knlTda.count > 0)
	{
		while (TRUE)
		{
			TOFORK;
			if ((npb = ourdcb->icmprcvhead) != NULL)
				break;
			if ((rtn = sysIoWait(knlTda.timeout, THDS_DW2)) < 0)
			{
				knlTda.error = rtn;
				knlTda.status |= QSTS_DONE;
				return (0);
			}
		}
		if ((ourdcb->icmprcvhead = npb->next) == NULL)
			ourdcb->icmprcvtail = NULL;
		FROMFORK;						// Don't need fork level now that the
										//   packet is off the list
		pkt = (ICMPPKT *)(npb->data + npb->tpofs);
		len = npb->count - npb->tpofs - sizeof(ICMPPKT);
		if (len > knlTda.count)
			len = knlTda.count;
		rtn = sysLibMemCpy(knlTda.buffer1, pkt->data, len);
		TOFORK;
		xosnetGiveNpb(npb);
		FROMFORK;
		if (rtn < 0)
			knlTda.error = rtn;
		else
			knlTda.amount = len;
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}


//*************************************************************************
// Function: getrmtnetar - Get value for IOPAR_NETRMTNETAR (0x0507)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter returns the remote network address for the most recently
//   received datagram. If used in the same IO operation that reads a
//   datagram it returns the address for that datagram.

static long XOSFNC getrmtnetar(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	if (data == NULL)					// Need to defer this?
		return (sysUSPutULongV(val, ourdcb->icmpnetar, cnt));
										// No - give him the value now
	((IOPDATA *)data)->ipapnt = val;	// Yes - remember where the value goes
	((IOPDATA *)data)->ipacnt = cnt;
	return (0);
}


//**********************************************************************
// Function: icmprecvpkt - Process received packet for the ICMP protocol
// Returned: TRUE if packet accepted, FALSE if rejected
//**********************************************************************

// NOTE: Accepted packets include invalid packets that are discarded. This
//       function accepts all packets and always returns TRUE.

static long XOSFNC icmprecvpkt(
	ICMPTPDB *tpdb,
	NPB      *npb)
{
	ICMPPKT  *pkt;
	ICMPDDCB *dcb;
	long      len;

	if (npb->label != NPB_LABEL)		// Is this really a network buffer?
		CRASH('NNPB');
	pkt = (ICMPPKT *)(npb->data + npb->tpofs);
	tpdb->cntpktin++;
	tpdb->cntbytein += npb->count;
	len = npb->count - npb->tpofs;
	if (len < sizeof(ICMPPKT))			// Is it big enough for the header?
	{
		tpdb->cntpsltmn++;				// No - count it and discard it
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	if (xosipChkSum((char *)pkt, len) != 0xFFFF) // Check the checksum
	{
		tpdb->cntchksum++;				// If bad checksum
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	switch (pkt->type)
	{
	 case ICMPT_ECHOREPLY:				// Echo reply
		if ((dcb = (ICMPDDCB *)sysIoGetDcbFromId(pkt->h2)) != NULL &&
				dcb->ccb == &icmpccb)
		{
			if (dcb->icmpinqcnt >= dcb->icmpinqmax) // Have too many queued
			{										//   packets?
				dcb->icmpcntinqfull++;	// Yes
				break;
			}
			dcb->icmpinqcnt++;			// No - count it
			npb->next = NULL;
			if (dcb->icmprcvtail != NULL) // Link it to the receive list
				dcb->icmprcvtail->next = npb;
			else
				dcb->icmprcvhead = npb;
			dcb->icmprcvtail = npb;
			if (dcb->inpthrd != NULL)	// Is input active?
				sysIoResumeThread(dcb->inpthrd, 0); // Yes - wake him up
			return (TRUE);
		}
		break;

	 case ICMPT_DSTUNR:   				// Destination unreachable
		break;

	 case ICMPT_SRCQUENCH:				// Source quench
		break;

	 case ICMPT_REDIRECT: 				// Redirect
		break;

	 case ICMPT_ECHOREQ:  				// Echo request
		icmpsendpkt(tpdb, NULL, ((IPPKT *)(npb->data + npb->npofs))->srcaddr.l,
				ICMPT_ECHOREPLY	+ (pkt->code << 8), pkt->h2, pkt->data,
				len - sizeof(ICMPPKT), NULL);
		break;
	}
	xosnetGiveNpb(npb);
	return (TRUE);
}


//*********************************************************
// Function: icmpsendpkt - Generate and send an ICMP packet
// Returned:
//*********************************************************

static long XOSFNC icmpsendpkt(
	ICMPTPDB *tpdb,
	ICMPDDCB *dcb,
	long      dstaddr,		// Destination IP address
	long      type,			// Type + code * 0x100
	long      h2,			// Second long for header
	char     *data,
	long      len,
	OUTDNFNC *outdn)
{
	NPB     *npb;
	NPB     *npbp;
	IDB     *idb;
	IPNPDB  *npdb;
	ICMPPKT *pkt;
	long     rtn;

	idb = tpdb->idb;
	npdb = (IPNPDB *)tpdb->npdb;
	if ((rtn = xosnetMakeNpb(idb, dcb, npdb->hdrsz + sizeof(ICMPPKT) + len,
			&npbp)) < 0)
		return (rtn);
	npb = npbp;
	xosipSetPkt((TPDB *)tpdb, npb, dstaddr);
	pkt = (ICMPPKT *)(npb->data + npb->tpofs);
	*(long *)&pkt->type = type;			// Store type and code, clear checksum
	*(long *)&pkt->h2 = h2;				// Store second header long
	if (len > 0)
		sysLibMemCpy(pkt->data, data, len);	// Store the data
	npb->count += (sizeof(ICMPPKT) + len);
	pkt->chksum = ~xosipChkSum((char *)pkt, len + sizeof(ICMPPKT));
	npb->outdone = outdn;
	xosipSendPkt(npdb, npb);
	return (0);
}
