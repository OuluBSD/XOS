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

#include <stddef.h>
#include <ctype.h>
#include <xos.h>
#include <xoserr.h>
#include <xosx.h>
#include <xosxchar.h>
#include <xosxparm.h>
#include <xosxlke.h>
#include <xoslib.h>
#include <xosnet.h>
#include <xosxnet.h>
#include <xosxarp.h>
#include <xosxip.h>

// This class driver implements the Internet Protocol Suite.  It directly
//   implements the IP device class and provides services used by the
//   routines that implement the higher level protocols (UDP, TCP, RCP,
//   etc.).  This class driver uses services provided by the NETCLS driver
//   and must be loaded after this driver.
// 
// Each of the transport level protocols (UDP, TCP, and RCP) have a TPDB
//   (Transport Protocol Data Block) which provides the base data for
//   implementing the transport protocol.  Each application level protocol
//   supported in the kernel for each transport protocol has a DPDB (Device
//   Protocol Data Block).  Note that the associated transport level devices
//   (UDPn, TCPn, and RCPn) are really implemented as transport devices, so
//   each of them has a DPDB.
// 
//                            +-----------------+       Interface
//                            |  Interface IDB  |       Level
//                            +-----------------+
//                              |             |
//             +------------------------+  +---------+  Network
//             +         IP PDB         |--| ARP PDB |  Level
//             +------------------------+  +---------+
//               |         |          |
//      +----------+  +----------+  +----------+        Transport
//      | ICMP TDB |  | UDP TPDB |  | TCP TPDB |        Level
//      +----------+  +----------+  +----------+
//                                       |
//                                  +----------+        Application
//                                  | XFP APDB |        Level
//                                  +----------+

// Also, DCBs (devices) can be created immediately below each of the enities
//   shown here. Except for ARP and ICMP, all can do IO at their level. All
//   can be used to obtain statistics for the enitity they are under.

// Each protocol level implements a multiplexing scheme to support multiple
//   independent data streams at that level. At the interface level the
//   network protocol type serves to identify streams. At the network level
//   the transport protocol type serves to identify streams and at the
//   transport level the port number identifies a stream. Currently the device
//   level simply provides access to a stream defined by the transport level
//   and does not do any additional multiplexing, although this certainly
//   could be implimented if needed.

// While ARP is specified and implimented as a seperate network level protocol
//   it interacts heavily with IP. Its only function is to provide address
//   resolution for IP.

// This version of XOS implements a one-to-one relationship between a stack's
//   low level data stream and a physical network interface, that is, each
//   higher level device (TCPn:, etc.) is associated with exactly one physical
//   network interface (NETn:) which has the same unit number. Multiple
//   interfaces are supported, but each one implements an independent network
//   with its own protocol stack. A future version MAY impliment multiple
//   connections to a network.

// At the interface level, only Ethernet DIX (Blue Book) packets are supported.

// A device is provided at each layer. (NETn:, IPn:, etc.). If no protocol
//   value is specified for such a device, that device can be used to obtain
//   global statistics (using device characteristis) for the next lower
//   layer. In this case, no IO can be performed by the device. If a protocol
//   and priority is specified, the device and be used to do IO directly into
//   the lower layer. Thus if an eithertype value is specified for a NETn:
//   device, IO is done using raw packets with the specified eithertype. Such
//   a device can also specify that it wants to receive all input not accepted
//   by a higher priority device or protocol. Not all such devices support IO.
//   In partictular, ARPn: devices do not support IO.

// For example:
//   Device Without protocol/port            With protocol/port
//   NETn:  Reports about a single if        Raw network IO using the ethertype
//   IPn:   Reports about IP on a single if  Raw IP IO using the spec.protocol
//   TCPn:  Reports about TCP on a single if Raw TCP IO using the spec. port
//   UDPn:  Reports about UDP on a single if Raw UDP IO using the spec. port

// 1.0.3 - 16-Apr-90
//   Fixed bug in setting up ARP packets - target hardware field was not
//   being zeroed
// 1.0.4 - 29-Aug-90
//   Upgraded to XOS v1.5
// 1.0.5 - 25-Jan-91
//   Upgraded to XOS v1.6
// 1.0.6 - 29-Aub-91
//   Many fixes to TCP protocol handling, converted XFP and TNP routines
//   into separately loaded LKEs
// 1.0.7 - 7-Sep-91
//   More fixes to TCP and UDP protocol handling, added initial code for
//   Domain Name client (not tested yet), added loop-back code (not tested
//   yet)
// 1.0.8 - 9-Oct-91
//   Fixed problem in tcpputblk in IPSTCP which destroyed the buffer pool
//   in some cases when storing less than 3 bytes
// 1.0.9 - 27-Oct-91 (Penang)
//   Fixed problem with clearing wake requests in IPSTCP
// 1.0.10 - 29-Oct-91 (Penang)
//   Added address cache purge capability (purges address automatically
//   when TCP connect fails, can also purge with RTPURGE device
//   characteristic for IPSn and UDPn)
// 1.0.11 - 4-Nov-91 (Penang)
//   Fixed rexmit in IPSTCP to check for packet being output before
//   rexmitting a packet
// 1.0.12 - 18-Nov-91
//   Changed IPSTCP to always check that the DCB is in the list before
//   removing a DCB from the IP hash list
// 1.0.13 - 25-Nov-91
//   Finished send-self routines, significent work on the DNS support
// 1.0.14 - 13-Apr-92
//   Numerious changes for new interface drivers, fixed some bugs in the
//   ARP cache routines
// 1.0.15 - 18-Jun-92
//   Fixed ipsetpkt to get IDB offset from the DCB (in some cases ESI was
//   not set up here)
// 1.0.16 - 11-Jul-92
//   Expanded the ARP address cache to be a full IP routing table, added
//   host support for ICMP message, added new module (IPSROUTE) to contain
//   all routing routines, changed all address cache references to routing
//   table references, changed ACxxxx characteristics to RTxxxx
// 1.0.17 - 24-Oct-92
//   Many fixes, fixed retransmission problems, changed seq. number
//   generation to keep values in order on new connections
// 1.0.18 - 5-Jan-93
//   Many fixes, latest was to fix bug at setrtremove, was incrementing
//   count in wrong data block
// 1.1.1 - 7-Feb-95
//   Fixed problem in IPSROUTE, would remove LPENDING routing table entry 
//   with held packets, causing crash when packet were finally given up.
// 1.1.2 - 6-Aug-95
//   Fixed problem with getting and setting count values for characteristics.
// 1.1.4 - 14-Jun-95
//   Fixed problem with removing DRTnADDR characteristics when changing the
//   IP address.
// 1.1.5 - 19-Aug-02
//   Changed how npb_count value is used in received packets. Fixed problem
//   in ipsoas with npb_link when calling xosnetGiveBufr.
// 1.1.6 - 23-Aug-02
//   Fixed problem with fork level race in ipsoas.

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0

//=========================================================================
// This file mostly contains routines that support the network level device
//   (IPn:). This device allows direct access to the network at the IP
//   level for a single protocol type. and provides access to network
//   statistics for the IP level for a network interface.
//=========================================================================

static INITLKE initip;

#pragma data_seg(_HEAD);
	LKEHEAD(IPCLS, initip, MAJVER, MINVER, EDITNUM, CLASS); 
#pragma data_seg();

IPNPDB *ipfirstnpdb;

// Protocol level dispatch table for IP

///static long XOSFNC ipabort(IPNPDB *npdb);

static NPDBDISP ipdisp =
{	NULL,				// Link up
	NULL,				// Link down
///	ipabort				// Abort output
///	rtgetaddr,			// getaddr function for protocol
};

static long ipdcbnum;
static long ipdcbmax;
static long ipdcblmt;

//************************************************
// Device IO parameter tables for IP class devices
//************************************************

static PARMGETFNC getprotocol;
static PARMSETFNC setprotocol;

// Network IO parameter table (0x05xx)

static PARMTBL2 ipnetparams = {3,
{	PTITEM(U, 4, NULL      , NULL),			//                   = 0x0500
	PTITEM(U, 4, NULL      , NULL),			// IOPAR_NETSUBMASK  = 0x0501
	PTITEM(U, 4, getprotocol, setprotocol)	// IOPAR_NETPROTOCOL = 0x0502
}};

// Top level table

static PARMTBL1 ipparams = {6,
{	&knlIoStdGenParams,		// 0x00xx - Generic IO parameters
	&knlIoDfltFilParams,	// 0x01xx - File system IO parameters
	&knlIoDfltTrmParams,	// 0x02xx - Terminal IO parameters
	NULL,					// 0x03xx - Disk IO parameters
	NULL,					// 0x04xx - Tape IO parameters
	&ipnetparams			// 0x05xx - Network IO parameters
}};


//*******************************************
// Device dispatch table for IP class devices
//*******************************************

static long XOSFNC ipgivedcb(void);
static long XOSFNC ipopen(char *spec);
static long XOSFNC ipinpblk(void);
static long XOSFNC ipoutblk(void);

static const QFDISP ipqfdisp =
{	NULL,				// Mount
	ipgivedcb,			// Clear DCB
	NULL,				// Open additional
	ipopen,				// Open device/file
	NULL,				// Device parameters
	NULL,				// Delete file
	NULL,				// Rename file
	ipinpblk,			// Input block
	ipoutblk,			// Output block
	NULL,				// Get input/output status
	NULL,				// Special device function
	sysIoCloseFin,		// Close file
	NULL,				// Get device label
	NULL,				// Commit data to disk
	NULL,				// Get device info
	NULL				// Verify changed disk
};


// Class characteristics tables for the IP device class

SINFO(msgnumber , "Number of in use IP devices");
SINFO(msgmaximum, "Maximum number of in use IP devices");
SINFO(msglimit  , "Maximum number of IP devices allowed");

static const CHARTBL ipcctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(NUMBER  , DECV, U, 4, &msgnumber , sysChrGetULong, NULL        ,
		&ipdcbnum),
  CTITEM(MAXIMUMX, DECV, U, 4, &msgmaximum, sysChrGetULong, sysChrSetULong,
		&ipdcbmax),
  CTLAST(LIMIT   , DECV, U, 4, &msglimit  , sysChrGetULong, sysChrSetULong,
		&ipdcblmt)
}};


// Class function dispatch table for IP class devices

static long XOSFNC ipaddunit(void); 

static CFDISP ipcls = { 3,
{	ipaddunit,			// CF_ADDUNIT =  8 - Add unit
	NULL,				// CF_PUNITS  =  9 - Get information about
						//                     physical units
	NULL				// CF_AUNITS  = 10 - Get information about active
}};

SINFO(msgarpdev       , "ARP device");
SINFO(msgetype        , "Ethertype value");
SINFO(msgipaddr       , "IP address");
SINFO(msgrtraddr1     , "First router IP address");
SINFO(msgrtraddr2     , "Second router IP address");
SINFO(msgrtsize       , "IP routing table size");
SINFO(msgrtuse        , "IP routing table usage");
SINFO(msgrtremove     , "IP routing table remove request");
SINFO(msgrtdata       , "IP routing table data");
SINFO(msgsubmask      , "IP subnet address mask");

SINFO(msgpktfrag      , "Number of fragmented packets input");
SINFO(msgnumfrag      , "Number of fragments input");
SINFO(msgdisfrag      , "Number of discarded fragments");
SINFO(msgipchksum     , "Discarded: Bad IP header checksum");
SINFO(msgipnodst      , "Discarded: No destination for IP");
SINFO(msgipbadaddr    , "Discarded: Incorrect IP address");

XINFO(xosipMsgIpProt  , "IP protocol value");
XINFO(xosipMsgIpDev   , "IP device");
XINFO(xosipMsgRetry1  , "First retransmission threshold");
XINFO(xosipMsgRetry2  , "Second retransmission threshold");
XINFO(xosipMsgNameSrvr, "Domain Name Server on this system");
XINFO(xosipMsgChkSum  , "Discarded: Bad data checksum");
XINFO(xosipMsgNoDst   , "Discarded: No destination for port");
XINFO(xosipMsgIBLXcd  , "Discarded: Input buffer limit exceeded");


static CHARRST charreset;
static CHARFNC getarpdev;
static CHARFNC setrtsize;
static CHARFNC setrtremove;
static CHARFNC setchksumh;
static CHARFNC setetype;
/* static */ CHARFNC getyesno;
/* static */ CHARFNC setyesno;

static const CHARTBL ipdctbl = {CTBEGIN(charreset),
{ CTITEM(CLASS   , TEXT, U,  8, &knlChrMsgClass  , sysChrDevGetClass,
		sysChrDevChkClass, 0),

  CTITEM(NETDEV  , TEXT, U, 20, &xosnetMsgNetDev , xosnetGetNetDev,
		NULL             , 0),

  CTITEM(ETYPE   , HEXV, U,  4, &msgetype        , xosnetGetNpdb4,
		NULL             , offsetof(IPNPDB, nplb.etype)),

  CTITEM(APRDEV  , TEXT, U, 20, &msgarpdev       , getarpdev,
		NULL             , 0),

  CTITEM(IPADDR  , DECB, U,  4, &msgipaddr       , xosnetGetNpdb4,
		xosnetSetNpdb4 , offsetof(IPNPDB, ipaddr)),

  CTITEM(SUBMASK , DECB, U,  4, &msgsubmask      , xosnetGetNpdb4,
		xosnetSetNpdb4 , offsetof(IPNPDB, ipsubnetmask)),

  CTITEM(RTSIZE  , DECV, U,  4, &msgrtsize       , xosnetGetNpdb4,
		setrtsize        , offsetof(IPNPDB, iprtmax)),

  CTITEM(RTUSE   , DECV, U,  4, &msgrtuse        , xosnetGetNpdb4 ,
		NULL             , offsetof(IPNPDB, iprtinuse)),

  CTITEM(RTREMOVE, DECV, U,  4, &msgrtremove  , xosnetGetNpdb4 ,
		setrtremove      , offsetof(IPNPDB, iprtpurge)),

  CTITEM(RTRADDR1, DECB, U,  4, &msgrtraddr1     , xosnetGetNpdb4,
		xosnetSetNpdb4 , offsetof(IPNPDB, iprtraddr1)),

  CTITEM(RTRADDR2, DECB, U,  4, &msgrtraddr2     , xosnetGetNpdb4,
		xosnetSetNpdb4 , offsetof(IPNPDB, iprtraddr2)),

  CTITEM(PKTIN   , DECV, U,  4, &xosnetMsgPktIn  , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, cntpktin)),

  CTITEM(BYTEIN  , DECV, U,  4, &knlChrMsgByteIn , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, cntbytein)),

  CTITEM(PKTOUT  , DECV, U,  4, &xosnetMsgPktOut , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, cntpktout)),

  CTITEM(BYTEOUT , DECV, U,  4, &knlChrMsgByteOut, xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, cntbyteout)),

  CTITEM(PKTFRAG , DECV, U,  4, &msgpktfrag      , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, ipcntpktfrag)),

  CTITEM(NUMFRAG , DECV, U,  4, &msgnumfrag      , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, ipcntnumfrag)),

  CTITEM(DISFRAG , DECV, U,  4, &msgdisfrag      , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, ipcntdisfrag)),

  CTITEM(CHKSUM  , DECV, U,  4, &msgipchksum     , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, ipcntchksum)),

  CTITEM(NODST   , DECV, U,  4, &msgipnodst      , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, cntnodst)),

  CTITEM(BADADDR , DECV, U,  4, &msgipbadaddr    , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, ipcntbadaddr)),

  CTITEM(BADHDR  , DECV, U,  4, &xosnetMsgBadHdr , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, cntbadhdr)),

  CTITEM(PSLTMN  , DECV, U,  4, &xosnetMsgPSLtMn , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, cntpsltmn)),

  CTITEM(PSLTDL  , DECV, U,  4, &xosnetMsgPSLtDL , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, cntpsltdl)),

  CTITEM(DLLTHL  , DECV, U,  4, &xosnetMsgDLLtHL , xosnetGetNpdb4 ,
		xosnetSetNpdb4 , offsetof(IPNPDB, ipcntdllthl)),

  CTLAST(RTDATA  , DATAS, u, sizeof(IPROUTE), &msgrtdata, getrtdata,
		NULL             , offsetof(IPNPDB, iprtmax))
}};


//***********
// The IP CCB
//***********

static DEVCHKFNC ipdevchk;

static ARPRECV hndlarppkt;

CCB ipccb =
{	'CCB*',
	0,					// next    - Address of next CCB
	"IP",				// name    - Name of this class
	0xFFFF,				// npfxmsk - Name prefix part mask
	'IP',				// npfxval - Name prefix part value
	ipdevchk,			// devchk  - Address of device check function
	&ipcls,				// fdsp    - Address of class func disp table
	&ipcctbl			// clschar - Address of class char table
};

static CHARFNC ipauarpdev;

static const CHARITEM ipaublk[] =
{	CTITEM(UNIT    , DECV, U, 4, NULL, NULL, xosnetAUUnit    , 0),
	CTITEM(IFDEV   , DECV, U, 4, NULL, NULL, xosnetAUNetDev  , 0),
	CTITEM(ARPDEV  , DECV, U, 4, NULL, NULL, ipauarpdev      , 0),
	CTITEM(ETYPE   , DECV, U, 4, NULL, NULL, xosnetAUProtocol,
			(1520<<16)+0xFFFF),
	CTLAST(PRIORITY, DECV, U, 4, NULL, NULL, xosnetAUPriority, 0)
};


//***********************************************************
// Function: initip - Initialization routine
// Returned: 0 if normal or a negative XOS error code if erro
//***********************************************************

#pragma code_seg ("x_ONCE");

static long XOSFNC initip(
	char **pctop,
	char **pdtop)
{
	long rtn;

	sysChrFixUp((CHARTBL *)&ipcctbl);
	sysChrFixUp((CHARTBL *)&ipdctbl);
	sysChrFixUp((CHARTBL *)&ipaublk);
	if ((rtn = sysIoNewClass(&ipccb)) >= 0)
	{
		sysSchSetUpOAS(ipoas);
		*pctop = codetop;
	}
	return (rtn);
}

#pragma code_seg ();


//************************************************************
// Function: ipaddunit - Implements the CL_ADDUNIT function
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC ipdevrecvpkt(IPDCB *dcb, NPB *npb);

// For this device the add unit function creates a IP level device which
//   communicates using the NETn: device with the same unit number. For
//   this device to be fully functional, corresponding (same unit number)
//   ARP and ICMP devices must also be created.

typedef struct
{	NETAUDATA;
	union
	{ char c[16];
	  long l[4];
	}   arpdev;
} AUDATA;

static long XOSFNC ipaddunit(void)
{
	IPNPDB  *npdb;
	ARPNPDB *arpnpdb;
	AUDATA   data;
	long     rtn;

	if (knlTda.buffer2 == NULL)
		return (ER_CHARM);
	data.dev.l[0] = -1;
	data.arpdev.l[0] = -1;
	data.unit = -1;
	data.protocol = -1;
	data.priority = 0x7FFF;
	if ((rtn = sysChrValuesNew(TRUE, ipaublk, &data)) < 0)
		return (rtn);
	if ((data.unit | data.dev.l[0] | data.arpdev.l[0] | data.protocol) == -1)
		return (ER_CHARM);
	if ((arpnpdb = xosarpChkClient(data.arpdev.c, data.protocol)) == NULL)
		return (ER_NSDEV);
	if ((rtn = xosnetMakeNetwork(data.dev.c,  "IP", data.unit, data.protocol,
			offsetof(IPPKT, options), sizeof(IPNPDB), (RECVFNC *)iprecvpkt,
			&ipdisp, &ipccb, (NPDB **)&npdb)) < 0)
		return (rtn);
	npdb->iprtseqnum = 1;
	npdb->iparpdb.label = 'ARPD';
	npdb->iparpdb.etype = data.protocol;
	npdb->iparpdb.hwlen = npdb->idb->hlen;
	npdb->iparpdb.plen = 4;
	npdb->iparpdb.paddr = (char *)&npdb->ipaddr;
	npdb->iparpdb.hwamask = 0xFFFFFFFFFFFF;
	npdb->iparpdb.npdb = (NPDB *)npdb;
	npdb->iparpdb.recvfunc = hndlarppkt;
	if (xosarpRegClient(arpnpdb, &npdb->iparpdb) < 0)
		CRASH('ARPF');

	npdb->iprtmax = 20;

/// SET UP ROUTING STUFF HERE!

	xosnetLinkNplb(npdb->idb, &npdb->nplb, data.priority);
	npdb->nextp = (NPDB *)ipfirstnpdb;
	ipfirstnpdb = npdb;
	npdb->nexts = xosnetFirstNpdb;
	xosnetFirstNpdb = (NPDB *)npdb;
	knlTda.amount = 1;
	knlTda.status |= QSTS_DONE;
	knlTda.dcb = NULL;
	return (0);
}


//**********************************************************************
// Function: ipauarpdev - Process ARPDEV characteristic when adding unit
// Returned: 0 if OK or a negative XOS error if value is invalid
//**********************************************************************


static long XOSFNC ipauarpdev(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	if (cnt > 16)
		return (ER_CHARS);
	sysLibMemSetLong(((AUDATA *)data)->arpdev.l, 0, 4);
	return (sysLibMemCpy(((AUDATA *)data)->arpdev.c, val, cnt));
}


//=====================
// End of addunit stuff
//=====================


//=========================================================
// Following functions are characteristics functions unique
//   to the IPn: devices
//=========================================================


//**************************************************
// Function: charreset - QFNC_DEVCHAR reset function
// Returned: Return value from sysChrValuesNew
//**************************************************

static long XOSFNC charreset(
	long            badnm,
	const CHARITEM *first,
	void           *data)
{
	((IPNPDB *)((IPDCB *)knlTda.dcb)->npdb)->iprtpurge = 0;
										// Clear routing table purge status
	return (sysChrValuesNew(badnm, first, data));
}


//***********************************************************
// Function: setrtsize - Set the RTSIZE device characteristic
// Returned: 0 if OK or a negative XOS error code if error
//***********************************************************

static long XOSFNC setrtsize(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	if (value < 10)
		value = 10;
	((IPNPDB *)((IPDCB *)knlTda.dcb)->npdb)->iprtmax = value;
	return (0);
}


//***************************************************************
// Function: setrtremove - Set the RTREMOVE device characteristic
// Returned: 0 if OK or a negative XOS error code if error
//***************************************************************

// This is really a request to remove an entry from the IP routing table. If
//   a value of 0 is specified all entries are removed.

static long XOSFNC setrtremove(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long ipaddr;
	long rtn;

	if ((rtn = sysUSGetULongV(val, &ipaddr, cnt)) < 0) // Get the IP address
		return (rtn);
	return (routermvip((IPNPDB *)((IPDCB *)knlTda.dcb)->npdb, ipaddr));
}


//********************************************************
// Function: setyesno - Set value of characteristic with
//                      value of YES or NO
// Returned: 0 if OK or a negative XOS error code if error
//********************************************************

/* static */ long XOSFNC setyesno(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	IPNPDB *npdb;
	long    rtn;

	if ((rtn = sysChrGetYesNo(val, cnt)) < 0)
		return (rtn);

	npdb = (IPNPDB *)((IPDCB *)knlTda.dcb)->npdb;

	if (rtn)
		npdb->ipsts1 |= (long)item->data;
	else
		npdb->ipsts1 &= ~(long)item->data;
	return (0);
}


//********************************************************
// Function: getyesno - Get value of characteristic with
//                      value of YES or NO
// Returned: 0 if OK or a negative XOS error code if error
//********************************************************

/* static */ long XOSFNC getyesno(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	union
	{ char c[4];
	  long v;
	} value;

	value.v = (((IPNPDB *)((IPDCB *)knlTda.dcb)->npdb)->ipsts1 |
			(long)item->data) ? 'Yes' : 'No';
	return (sysLibStrNMov(val, value.c, cnt));
}


//*************************************************************
// Function: getarpdev - Get value of the ARPDEV characteristic
// Returned: Number of bytes returned  if OK or a negative XOS
//           error code if error
//*************************************************************

static long XOSFNC getarpdev(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysLibStrNMov(val, ((IPNPDB *)((IPDCB *)knlTda.dcb)->npdb)
			->iparpdb.arpnpdb->name, cnt));
}


//==================================================================
// Following are the standard functions that implement an XOS device
//==================================================================


//******************************************************************
// Function: ipdevchk - Device check function for IP class devices
// Returned: 0 if no match, 1 if found a match, 2 if need to restart
//				search, or a negative XOS error code if error.
//******************************************************************

static long XOSFNC ipdevchk(
	char  name[16],		// Device name (16 bytes, 0 filled)
	char *path,			// Path specification
	long  rtndcb)
{
	IPNPDB *npdb;
	long    rtn;
	int     unit;

	if (isdigit(name[2]))
	{
		if (name[3] == 0)
			unit = name[2] & 0x0F;
		else if (isdigit(name[3]) && name[4] == 0)
			unit = (name[2] & 0x0F) * 10 + (name[3] & 0x0F);
		else
			return (0);
		npdb = ipfirstnpdb;
		while (npdb != NULL)
		{
			if (unit == npdb->unit)
			{
				if ((rtn = xosnetMakeDcb(name, unit, sizeof(IPDDCB),
						&ipccb, (IDB *)npdb->idb, &ipdctbl, &ipqfdisp,
						npdb->ipdesc)) < 0)
					return (rtn);
				((IPDDCB *)knlTda.dcb)->npdb = (NPDB *)npdb;
				((IPDDCB *)knlTda.dcb)->iptplb.label = 'NPLB';
				((IPDDCB *)knlTda.dcb)->iptplb.blk = knlTda.dcb;
				((IPDDCB *)knlTda.dcb)->iptplb.recvfnc =
						(RECVFNC *)ipdevrecvpkt;
				return (1);
			}
			npdb = (IPNPDB *)npdb->nextp;
		}
		return (ER_NSDEV);
	}
	return (0);
}


//************************************************************
// Function: ipopen - Handle the open entry - Open device
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC ipopen(
	char *spec)
{
	long  rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&ipparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		return (rtn);
	return (0);
}


//************************************************************
// Function: ipgivedcb - Handle the cdcb entry - Give up DCB
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC ipgivedcb(void)
{



	return (0);
}


//*************************************************************
// Function: ipinpblk - Handle the inpblock entry - input block
// Returned: Queued IO return
//*************************************************************

static long XOSFNC ipinpblk(void)
{
	long  rtn;

	knlTda.timeout = -1;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&ipparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		knlTda.error = rtn;
	else
	{
		if (knlTda.count > 0)
		{
			knlTda.error = ER_NIYT;
		}
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}


//**************************************************************
// Function: ipoutblk - Handle the outblock entry - output block
// Returned: Queued IO return
//**************************************************************

static long XOSFNC ipoutblk(void)
{
	long  rtn;

	knlTda.timeout = -1;
	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&ipparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		knlTda.error = rtn;
	else
	{
		if (knlTda.count > 0)
		{
			knlTda.error = ER_NIYT;
		}
	}
	knlTda.status |= QSTS_DONE;
	return (0);
}


//===================================================================
// Following are device parameter functions unique to the IPn: device
//===================================================================


//************************************************************
// Function: setprotocol - Set the protocol value
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

long XOSFNC setprotocol(
	PINDEX inx,
	char  *val,
	long   cnt,
	void  *data)
{
	long  rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	if (value <= 0 || value > 0xFF)
		return (ER_PARMV);
	((IPDDCB *)knlTda.dcb)->iptplb.protocol = value;
	return (0);
}

//************************************************************
// Function: getprotocol - Get the protocol value
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

long XOSFNC getprotocol(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutULongV(val, ((IPDDCB *)knlTda.dcb)->iptplb.protocol, cnt));
}


//====================================================
// Following are support functions for the IPn: device
//====================================================


//****************************************************
// Function: hndlarpreq - Handle a received ARP packet
// Returned: Nothing
//****************************************************

static void XOSFNC hndlarppkt(
	NPDB *npdb,
	long  oper,
	llong srchwaddr,
	char *srcpaddr,
	char *tarpaddr)
{
	if (((IPNPDB *)npdb)->ipaddr == *(long *)tarpaddr)
	{
		routeupdate((IPNPDB *)npdb, *(long *)srcpaddr, srchwaddr, TRUE);
		if (oper == 1)
			xosarpSendPkt(&((IPNPDB *)npdb)->iparpdb, 0x02, (char *)&srchwaddr,
					srcpaddr);
	}
	else
		routeupdate((IPNPDB *)npdb, *(long *)srcpaddr, srchwaddr, FALSE);
}


static long XOSFNC ipdevrecvpkt(
	IPDCB  *dcb,
	NPB    *npb)
{




	return (0);
}
