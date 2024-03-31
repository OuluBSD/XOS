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

// This device class implements the Internet UDP protocol. This is a transport
//   level protocol layered over IP. It is a datagram protocol that does not
//   provide realible delivery. It main use is to transfer user data using
//   UDPn: devices. Currently, no higher level protocols which use UDP are
//   implemented in exec mode.

// Individual UDPn: devices are selected to receive input based on their local
//   port number. Lookup of the local port number is done using a simple hash
//   scheme using the low order 8 bits of the port number as a hash index with
//   a 256 entry hash table. Each hash table entry is the head pointer for a
//   list of all devices using local ports with the same hash index.

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
#include <xosxudp.h>

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0

static INITLKE initudp;

static long XOSFNC udprecvpkt(UDPTPDB *tpdb, NPB *npb);

#pragma data_seg(_HEAD);
	LKEHEAD(UDPCLS, initudp, MAJVER, MINVER, EDITNUM, CLASS); 
#pragma data_seg();

static long udpdcbnum;			// Number of in use UDP DCBs
static long udpdcbmax;			// Maximum in use UDP DCBs
static long udpdcblmt = 10000;	// Maximum number of UDP DCBs allowed

UDPTPDB *udpfirsttpdb;

// Transport level dispatch table for UDP

static TPDBDISP udpdisp =
{	NULL,				// Link up
	NULL,				// Link down
///	udpabort			// Abort output
///	rtgetaddr,			// getaddr function for protocol
};

// Define macro used to get the name of our DCB as a UDP DCB

#define ourdcb ((UDPDDCB *)knlTda.dcb)

// Define structure used to save information about parameter request

typedef struct
{	char *ipapnt;		// Address for storing remote IP address
	long  ipacnt;		// Size for remode IP address
	char *portpnt;		// Address for storing remote UDP port number
	long  portcnt;		// Size for remote UDP port number
} IOPDATA;


//*************************************************
// Device IO parameter tables for UDP class devices
//*************************************************

static PARMGETFNC getlclport;
static PARMSETFNC setlclport;
static PARMGETFNC getrmtnetar;

static PARMGETFNC getrmtports;
static PARMSETFNC setrmtports;

static PARMGETFNC getrmtportr;
static PARMGETFNC getmode;
static PARMSETFNC setmode;
static PARMSETFNC clrmode;
static PARMGETFNC getlclneta;
static PARMGETFNC getmaxpktsz;
static PARMSETFNC setmaxpktsz;
static PARMGETFNC getinqlimit;
static PARMSETFNC setinqlimit;

// Network IO parameter table (0x05xx)

static PARMTBL2 udpnetparams = {19,
{ PTITEM( ,  0, NULL, NULL),				//                   = 0x0500
  PTITEM( ,  0, NULL, NULL),				//                   = 0x0501
  PTITEM(U,  4, NULL, NULL),				//                   = 0x0502
  PTITEM(U,  4, getlclport , setlclport),	// IOPAR_NETLCLPORT  = 0x0503
  PTITEM(U,  8, NULL, NULL),				// IOPAR_NETRMTHWAS  = 0x0504
  PTITEM(U,  8, NULL, NULL),				// IOPAR_NETRMTHWAR  = 0x0505
  PTITEM(U,  4, xosipGetRmtNetAS, xosipSetRmtNetAS),
											// IOPAR_NETRMTNETAS = 0x0506
  PTITEM(U,  4, getrmtnetar, NULL),			// IOPAR_NETRMTNETAR = 0x0507
  PTITEM(U,  4, xosipGetRmtPortS, xosipSetRmtPortS),
											// IOPAR_NETRMTPORTS = 0x0508
  PTITEM(U,  4, getrmtportr, NULL),			// IOPAR_NETRMTPORTR = 0x0509
  PTITEM(L, 32, NULL, xosipSetRmtName),		// IOPAR_NETRMTNAME  = 0x050A
  PTITEM(U,  4, getmode, setmode),			// IOPAR_NETSMODE    = 0x050B
  PTITEM(U,  4, getmode, clrmode),			// IOPAR_NETCMODE    = 0x050C
  PTITEM( ,  0, NULL, NULL),		    	// IOPAR_NETRCVWIN   = 0x050D
  PTITEM(U,  4, xosipGetLclNetA , NULL),	// IOPAR_NETLCLNETA  = 0x050E
  PTITEM(U,  4, NULL, NULL),				// IOPAR_NETKATIME   = 0x050F
  PTITEM( ,  0, NULL, NULL),				// IOPAR_NETCONLIMIT = 0x0510
  PTITEM(U,  4, NULL, NULL),				// IOPAR_NETCONHNDL  = 0x0511
  PTITEM(U,  4, xosipGetMaxPktSz, xosipSetMaxPktSz),
											// IOPAR_NETPKTMAX   = 0x0512
  PTITEM(U,  4, xosipGetInqLimit, xosipSetInqLimit)
}};											// IOPAR_NETINQLIMIT = 0x0513

// Top level table

static PARMTBL1 udpparams = {6,
{	&knlIoStdGenParams,		// 0x00xx - Generic IO parameters
	&knlIoDfltFilParams,	// 0x01xx - File system IO parameters
	&knlIoDfltTrmParams,	// 0x02xx - Terminal IO parameters
	NULL,					// 0x03xx - Disk IO parameters
	NULL,					// 0x04xx - Tape IO parameters
	&udpnetparams			// 0x05xx - Network IO parameters
}};


//********************************************
// Device dispatch table for UDP class devices
//********************************************

static long XOSFNC udpgivedcb(void);
static long XOSFNC udpopen(char *spec);
static long XOSFNC udpclose(void);
static long XOSFNC udpinpblk(void);
static long XOSFNC udpoutblk(void);

static const QFDISP udpfdisp =
{	NULL,				// Mount
	udpgivedcb,			// Clear DCB
	NULL,				// Open additional
	udpopen,			// Open device/file
	NULL,				// Device parameters
	NULL,				// Delete file
	NULL,				// Rename file
	udpinpblk,			// Input block
	udpoutblk,			// Output block
	NULL,				// Get input/output status
	NULL,				// Special device function
	udpclose,			// Close
	NULL,				// Get device label
	NULL,				// Commit data to disk
	NULL,				// Get device info
	NULL				// Verify changed disk
};


// Class characteristics tables for the UDP device class

SINFO(msgnumber , "Number of in use UDP devices");
SINFO(msgmaximum, "Maximum number of in use UDP devices");
SINFO(msglimit  , "Maximum number of UDP devices allowed");

static const CHARTBL udpcctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(NUMBER  , DECV, U, 4, &msgnumber , sysChrGetULong, NULL        ,
		&udpdcbnum),
  CTITEM(MAXIMUMX, DECV, U, 4, &msgmaximum, sysChrGetULong, sysChrSetULong,
		&udpdcbmax),
  CTLAST(LIMIT   , DECV, U, 4, &msglimit  , sysChrGetULong, sysChrSetULong,
		&udpdcblmt)
}};

// Class function dispatch table for UDP class devices

static long XOSFNC udpaddunit(void); 

static CFDISP udpcls = { 3,
{	udpaddunit,			// CF_ADDUNIT =  8 - Add unit
	NULL,				// CF_PUNITS  =  9 - Get information about
						//                     physical units
	NULL				// CF_AUNITS  = 10 - Get information about active
}};


static const CHARTBL udpdctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(CLASS   , TEXT, U,  8, &knlChrMsgClass  , sysChrDevGetClass,
		sysChrDevChkClass, 0),

  CTITEM(IPDEV   , TEXT, U, 20, &xosipMsgIpDev  , xosnetGetNetDev,
		NULL             , 0),

  CTITEM(PROTOCOL, DECV, U,  4, &xosipMsgIpProt , xosipGetTpdb4,
		NULL             , offsetof(UDPTPDB, tplb.protocol)),

  CTITEM(PKTIN   , DECV, U,  4, &xosnetMsgPktIn  , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(UDPTPDB, cntpktin)),

  CTITEM(BYTEIN  , DECV, U,  4, &knlChrMsgByteIn , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(UDPTPDB, cntbytein)),

  CTITEM(PKTOUT  , DECV, U,  4, &xosnetMsgPktOut , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(UDPTPDB, cntpktout)),

  CTITEM(BYTEOUT , DECV, U,  4, &knlChrMsgByteOut, xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(UDPTPDB, cntbyteout)),

///  CTITEM(BADADDR , DECV, U,  4, &msgipbadaddr    , xosipGetTpdb4,
///		xosipSetTpdb4   , offsetof(UDPTPDB, ipcntbadaddr)),

  CTITEM(BADHDR  , DECV, U,  4, &xosnetMsgBadHdr , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(UDPTPDB, cntbadhdr)),

  CTITEM(PSLTMN  , DECV, U,  4, &xosnetMsgPSLtMn , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(UDPTPDB, cntpsltmn)),

  CTLAST(PSLTDL  , DECV, U,  4, &xosnetMsgPSLtDL , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(UDPTPDB, cntpsltdl)),

///  CTLAST(DLLTHL  , DECV, U,  4, &xosnetMsgDLLtHL , xosipGetTpdb4,
///		xosipSetTpdb4   , offsetof(UDPTPDB, ipcntdllthl))
}};


//************
// The UDP CCB
//************

static DEVCHKFNC udpdevchk;

CCB udpccb =
{	'CCB*',
	0,					// next    - Address of next CCB
	"UDP",				// name    - Name of this class
	0xFFFFFF,			// npfxmsk - Name prefix part mask
	'UDP',				// npfxval - Name prefix part value
	udpdevchk,			// devchk  - Address of device check function
	&udpcls,			// fdsp    - Address of class func disp table
	&udpcctbl			// clschar - Address of class char table
};


static void udpgiveport(void);


static const CHARITEM udpaublk[] =
{	CTITEM(UNIT    , DECV, U, 4, NULL, NULL, xosnetAUUnit    , 0),
	CTITEM(NETDEV  , DECV, U, 4, NULL, NULL, xosnetAUNetDev  , 0),
	CTITEM(PROTOCOL, DECV, U, 4, NULL, NULL, xosnetAUProtocol, (1<<16)+0xFF),
	CTLAST(PRIORITY, DECV, U, 4, NULL, NULL, xosnetAUPriority, 0)
};


//************************************************************
// Function: initudp - Initialization routine
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

#pragma code_seg ("x_ONCE");

static long XOSFNC initudp(
	char **pctop,
	char **pdtop)
{
	long rtn;

	sysChrFixUp((CHARTBL *)&udpcctbl);
	sysChrFixUp((CHARTBL *)&udpdctbl);
	sysChrFixUp((CHARTBL *)&udpaublk);
	if ((rtn = sysIoNewClass(&udpccb)) >= 0)
		*pctop = codetop;
	return (rtn);
}

#pragma code_seg ();


//************************************************************
// Function: udpaddunit - Implements the CL_ADDUNIT function
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC udpaddunit(void)
{
	UDPTPDB  *tpdb;
	NETAUDATA data;
	long     rtn;

	if (knlTda.buffer2 == NULL)
		return (ER_CHARM);
	data.dev.l[0] = -1;
	data.unit = -1;
	data.protocol = -1;
	data.priority = 0x7FFF;
	if ((rtn = sysChrValuesNew(TRUE, udpaublk, &data)) < 0)
		return (rtn);
	if ((data.unit | data.dev.l[0] | data.protocol) == -1)
		return (ER_CHARM);
	if ((rtn = xosnetMakeTransport(data.dev.c,  "UDP", data.unit,
			data.protocol, offsetof(UDPPKT, data), sizeof(UDPTPDB),
			(RECVFNC *)udprecvpkt, &udpdisp, &udpccb, (TPDB **)&tpdb)) < 0)
		return (rtn);
	tpdb->devdesc = DS_DUPLEX|DS_QIN|DS_QOUT|DS_DINDFT|DS_DOUTDFT|DS_DIN|
			DS_DOUT;
	xosnetLinkTplb(tpdb->npdb, &tpdb->tplb, data.priority);
	tpdb->nextp = (TPDB *)udpfirsttpdb;
	udpfirsttpdb = (UDPTPDB *)tpdb;
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


//******************************************************************
// Function: udpdevchk - Device check function for UDP class devices
// Returned: 0 if no match, 1 if found a match, 2 if need to restart
//           search, or a negative XOS error code if error.
//******************************************************************

static long XOSFNC udpdevchk(
	char  name[16],		// Device name (16 bytes, 0 filled)
	char *path,			// Path specification
	long  rtndcb)
{
	UDPTPDB *tpdb;
	long     rtn;
	int      unit;

	if (isdigit(name[3]))
	{
		if (name[4] == 0)
			unit = name[3] & 0x0F;
		else if (isdigit(name[4]) && name[5] == 0)
			unit = (name[3] & 0x0F) * 10 + (name[4] & 0x0F);
		else
			return (0);
		tpdb = udpfirsttpdb;
		while (tpdb != NULL)
		{
			if (unit == tpdb->unit)
			{
				if ((rtn = xosnetMakeDcb(name, unit, sizeof(UDPDDCB), &udpccb,
						tpdb->idb, &udpdctbl, &udpfdisp,
						tpdb->devdesc)) < 0)
					return (rtn);
				ourdcb->npdb = (NPDB *)tpdb->npdb;
				ourdcb->tpdb = (TPDB *)tpdb;
				ourdcb->outpndmax = 10;
				ourdcb->udpinqmax = 10;
				return (1);
			}
			tpdb = (UDPTPDB *)tpdb->nextp;
		}
		return (ER_NSDEV);
	}
	return (0);
}


//************************************************************
// Function: udpopen - Handle the open entry - Open device
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC udpopen(
	char *spec)
{
	long  rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&udpparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		return (rtn);
	return (0);
}


//**************************************************************
// Function: udpclose - Handle the sd_close entry - Close device
// Returned: 0 if normal or a negative XOS error code if error
//**************************************************************

static long XOSFNC udpclose(void)
{
	if (ourdcb->udplclport != 0)		// Do we have a local port now?
		udpgiveport();					// Yes - give it up
	return (sysIoCloseFin());
}


//**************************************************************
// Function: udpgivedcb - Handle the sd_cdcb entry - Give up DCB
// Returned: 0 if normal or a negative XOS error code if error
//**************************************************************

static long XOSFNC udpgivedcb(void)
{
	udpdcbnum--;
	sysIoGiveDymDcb(knlTda.dcb);
	knlTda.dcb = NULL;
	return (0);
}


//*****************************************************************
// Function: udpinpblk - Handle the sd_inpblock entry - Input block
// Returned: Queued IO return
//*****************************************************************

static long XOSFNC udpinpblk(void)
{
	NPB    *npb;
	UDPPKT *pkt;
	long    rtn;
	long    len;

	knlTda.timeout = -1;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&udpparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		knlTda.error = rtn;
	else if (knlTda.count > 0)
	{
		while (TRUE)
		{
			TOFORK;
			if ((npb = ourdcb->udprcvhead) != NULL)
				break;
			if ((rtn = sysIoWait(knlTda.timeout, THDS_DW2)) < 0)
			{
				knlTda.error = rtn;
				knlTda.status |= QSTS_DONE;
				return (0);
			}
		}
		if ((ourdcb->udprcvhead = npb->next) == NULL)
			ourdcb->udprcvtail = NULL;
		FROMFORK;						// Don't need fork level now that the
										//   packet is off the list
		pkt = (UDPPKT *)(npb->data + npb->tpofs);
		len = npb->count - npb->tpofs - sizeof(UDPPKT);
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


//******************************************************************
// Function: udpoutblk - Handle the sd_outblock entry - Output block
// Returned: Queued IO return
//******************************************************************

static long XOSFNC udpoutblk(void)
{
///	IDB     *idb;
	NPB     *npb;
	NPB     *npbp;
	UDPTPDB *tpdb;
	UDPPKT  *pkt;
	long     rtn;
	long     len;

	knlTda.timeout = -1;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&udpparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		knlTda.error = rtn;
	else while (knlTda.count > 0)
	{
		if (ourdcb->udplclport == 0)	// Do we have a local port?
		{
			knlTda.error = ER_NLPNS;	// No - fail
			break;
		}
		if (ourdcb->rmtnetas == 0 ||	// Do we a remote port and address?
				ourdcb->rmtports == 0)
		{
			knlTda.error = ER_NILAD;	// No - fail
			break;
		}
		if (ourdcb->outpndcnt >= ourdcb->outpndmax)
		{							 	// Do we have too many output packets
										//   pending?

			CRASH('qqqq');

		}
		if ((rtn = xosnetMakeNpb(ourdcb->nifidb, ourdcb, knlTda.count +
				ourdcb->npdb->hdrsz + sizeof(UDPPKT), &npbp)) < 0)
		{									// Get a packet buffer (assume no
			knlTda.error = rtn;				//   IP options)
			break;
		}
		npb = npbp;
		tpdb = (UDPTPDB *)ourdcb->tpdb;
		pkt = (UDPPKT *)(npb->data + xosipSetPkt((TPDB *)tpdb, npb,
				ourdcb->rmtnetas));	// Set up the IP header
		pkt->srcport = convnetword(ourdcb->udplclport); // Set up the UDP header
		pkt->dstport = convnetword(ourdcb->rmtports);
		*(long *)&pkt->length = convnetword(sizeof(UDPPKT) + knlTda.count);
										// This also sets the checksum to 0);
		if ((rtn = sysLibMemCpy(pkt->data, knlTda.buffer1, knlTda.count)) < 0)
		{
			sysMemGiveXmb(pkt);
			knlTda.error = rtn;
			break;
		}
		len = knlTda.count + sizeof(UDPPKT);
		npb->count += len;
		pkt->chksum = ~xosipChkSumPH(((IPNPDB *)ourdcb->npdb)->ipaddr,
				ourdcb->rmtnetas, convnetlong((tpdb->tplb.protocol << 16) +
				len), (char *)pkt, len);
		TOFORK;
		xosipSendPkt((IPNPDB *)ourdcb->npdb, npb); // Send the packet
		FROMFORK;
		knlTda.amount = knlTda.count;
		break;
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}


//=========================================
// Following are device parameter functions
//=========================================


//*************************************************************************
// Function: getlclport - Get value for IOPAR_NETLCLPORT (0x0503)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter returns the current local port value.

static long XOSFNC getlclport(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutULongV(val, ourdcb->udplclport, cnt));
}


//***************************************************************
// Function: setlclport - Set value for IOPAR_NETLCLPORT (0x0503)
// Returned: 0 if normal or an XOS error code if error
//***************************************************************

// This parameter sets the value for the local port. The port value is 16
//   bits. Bits 30 and 31 are flags as follows:
//		Bit 31: Allow duplicate (not supported in this version)
//		Bit 30: Increment value if port is assigned
// Remaining bits are reserved and must be 0

// NOTE: This version does NOT support filtering received datagrams by
//       remote port or address.

static long XOSFNC setlclport(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	void   *data)
{
	UDPTPDB  *tpdb;
	UDPDDCB  *dcbpnt;
	UDPDDCB **prev;
	long      rtn;
	long      value;
	long      inx;
	long      cont;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	if (value & 0xDFFF0000)				// Valid value?
		return (ER_PARMV);				// No - fail
	if (ourdcb->udplclport != 0)		// Do we have a local port now?
	{
		if (ourdcb->udplclport != (ushort)value)
			return (0);					// Nothing to do here If no change
		udpgiveport();					// Give up the current local port
	}
	if ((ushort)value == 0)				// Want to clear it?
		return (0);						// Yes - finished
	cont = value & 0x40000000;
	value &= 0xFFFF;
	inx = (char)value;
	tpdb = (UDPTPDB *)ourdcb->tpdb;
	while (TRUE)
	{
		TOFORK;
		prev = &tpdb->udpporthash[inx];
		while((dcbpnt = *prev) != NULL && dcbpnt->udplclport > value)
			prev = &dcbpnt->udphashnext;
		if (dcbpnt == NULL || dcbpnt->udplclport != value)
		{
			// Here if this port is available - use it

			ourdcb->udplclport = (ushort)value;
			ourdcb->udphashnext = dcbpnt;
			*prev = ourdcb;
			FROMFORK;
			return (0);
		}
		FROMFORK;
		if (!cont || ++value > 0xFFFF)	// Port is in use - should we search?
			return (ER_NPRIU);			// No or too big - fail
	}
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
		return (sysUSPutULongV(val, ourdcb->udprmtipar, cnt));
										// No - give him the value now
	((IOPDATA *)data)->ipapnt = val;	// Yes - remember where the value goes
	((IOPDATA *)data)->ipacnt = cnt;
	return (0);
}


//*************************************************************************
// Function: getrmtportar - Get value for IOPAR_NETRMTPORTR (0x0509)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter returns the remote port number for the most recently
//   received datagram. If used in the same IO operation that reads a
//   datagram  it returns the port number for that datagram.

static long XOSFNC getrmtportr(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	if (data == NULL)					// Need to defer this?
		return (sysUSPutULongV(val, ourdcb->udprmtportr, cnt));
										// No - give him the value now
	((IOPDATA *)data)->portpnt = val;	// Yes - remember where the value goes
	((IOPDATA *)data)->portcnt = cnt;
	return (0);
}


//*************************************************************************
// Function: getmode - Get value for IOPAR_NETxMODE (0x050B/0x050C)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// These parameters return the current network mode bits.

static long XOSFNC getmode(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{

	return (0);
}


//*************************************************************************
// Function: setmode - Set value for IOPAR_NETSMODE (0x050B)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter sets the specified network mode bits. Bits not specified
//   in the value are not changed.

static long XOSFNC setmode(
	PINDEX  inx,
	char   *val,
	long    cnt,
	void   *data)
{

	return (0);
}


//*************************************************************************
// Function: clrmode - Get value for IOPAR_NETCMODE (0x050C)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter clears the specified network mode bits. Bits not specified
//   in the value are not changed.

static long XOSFNC clrmode(
	PINDEX  inx,
	char   *val,
	long    cnt,
	void   *data)
{

	return (0);
}


//********************************************************************
// Function: udprecvpkt - Process received packet for the UDP protocol
// Returned: TRUE if packet accepted, FALSE if rejected
//********************************************************************

// NOTE: Accepted packets include invalid packets that are discarded. This
//       function accepts all packets and always returns TRUE.

static long XOSFNC udprecvpkt(
	UDPTPDB *tpdb,
	NPB      *npb)
{
	UDPPKT  *pkt;
	IPPKT   *ippkt;
	UDPDDCB *dcb;
	long     len;
	long     inx;
	long     port;

	if (npb->label != NPB_LABEL)		// Is this really a network buffer?
		CRASH('NNPB');
	pkt = (UDPPKT *)(npb->data + npb->tpofs);
	tpdb->cntpktin++;
	tpdb->cntbytein += npb->count;
	len = npb->count - npb->tpofs;
	if (len < sizeof(UDPPKT))			// Is it big enough for the header?
	{
		tpdb->cntpsltmn++;				// No - count it and discard it
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	ippkt = (IPPKT *)(npb->data + npb->npofs); // Get address of the IP header
	if (xosipChkSumPH(ippkt->srcaddr.l, ippkt->dstaddr.l,   // Check the
			convnetlong((tpdb->tplb.protocol << 16) + len), //   checksum
			(char *)pkt, len) != 0xFFFF)
	{
		tpdb->cntchksum++;				// If bad checksum
		xosnetGiveNpb(npb);
		return (TRUE);
	}

	// Here with a valid UDP packet

	INT3;

	port = convnetword(pkt->dstport);
	inx = (char)pkt->dstport;
	dcb = tpdb->udpporthash[(int)((char)port)];
	while (dcb != NULL)
	{
		if (dcb->udplclport == port)
		{
			if (dcb->udpinqcnt >= dcb->udpinqmax)
			{
				tpdb->udpcntinqfull++;
				dcb->udpcntinqfull++;
				xosnetGiveNpb(npb);
				return (TRUE);
			}
			dcb->udpinqcnt++;
			if ((npb->next = dcb->udprcvtail) != NULL)
				dcb->udprcvtail->next = npb;
			else
			{
				dcb->udprcvhead = npb;
				if (dcb->inpthrd != NULL) // Is he waiting for input?
					sysIoResumeThread(dcb->inpthrd, 0); // Yes - wake him up
			}
			dcb->udprcvtail = npb;
			return (TRUE);
		}
		dcb = dcb->udphashnext;
	}
	tpdb->cntnodst++;
	xosnetGiveNpb(npb);
	return (TRUE);
}


//*************************************************
// Function: udpgiveport - Give up a UDP local port
// Returned: Nothing
//*************************************************

static void udpgiveport(void)
{
	UDPTPDB  *tpdb;
	UDPDDCB  *dcbpnt;
	UDPDDCB **prev;

	tpdb = (UDPTPDB *)ourdcb->tpdb;
	TOFORK;
	prev = &tpdb->udpporthash[(int)((char)ourdcb->udplclport)];
	while((dcbpnt = *prev) != NULL && dcbpnt != ourdcb)
		prev = &dcbpnt->udphashnext;
	*prev = ourdcb->udphashnext;		// Remove us from the hash list
	ourdcb->udplclport = 0;				// No local port now
	FROMFORK;
}
