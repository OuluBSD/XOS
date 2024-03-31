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

// This class implements the TCP transport level protocol. It supports the
//   full protocol with the following exceptions:
//     Out of order packets are discarded
//     Urgent data is not handled correctly
//     The TIMEWAIT state is not implemented - ports are made idle as soon
//       as a connection is cleared

// 2 types of passive opens are supported:

// Type 1: The TCP device is opened specifying * as the network address (for
//         example, TCP0:*::. The open does not complete until there is an
//         incomming connection on the port.  Only one such passive open is
//         allowed on a port, thus this is not practical to use when multiple
//         requests are expected. In is intended to be used when establishing
//         simple peer-to-peer connections.

// Type 2: The TCP device is opened specifying a non-zero value for the
//         IOPAR_NETCONLIMIT IO parameter.  This specifies the maximum
//         number of incomming connections which will queued on the port.
//         This open completes immediately. The device handle returned cannot
//         be used for IO.  Another open is done specifing the handle returned
//         by the first open as the value of the IOPAR_NETCONHNDL IO parameter.
//         This open does not complete until there is an incoming connection.
//         Up to the number of incoming connections specified in the first
//         open will be queued by the TCP driver allowing overlapped processing
//         of incoming connections. Two DCBs and one IO frame are tied up when
//         doing this. This is basically equivilant to the Berkley Sockets
//         method of accepting connections.

// An input buffer is allocated for each connected device big enough to hold
//   the entire advertized input window (maximum input window size is 64KB).
//   Data is read into the buffer as packets are received, including out of
//   order (but in window) packets. (????)

// An output buffer is allocated for each connected device big enough to hold
//   the advertized output window up to 16KB. This allows for a reasonable
//   amount of overlapped processing without using an excessive amount of
//   memory. Data is not put into packets until it is to be transmitted.
//   Transmitted packets are not saved for retransmission but are rebuilt
//   from the output buffer when retransmission is necessary. This implies
//   that retransmitted packets may be different from the originals if data
//   has been added to the output buffer. Since packets are not saved until
//   they are acknowledged, it is impossible to calculate round trip times
//   for individual packets. Instead, a timer is started when a packet is
//   send. When an acknowledgement is received for all of the data in that
//   packet the round trip time for that data is calculated. While this
//   timer is active, no other round trip timings are done. Once the packet
//   being timed is acknowledged, the next packet sent will be timed.


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
#include <xosxtcp.h>

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0


static INITLKE inittcp;

static long XOSFNC tcprecvpkt(TCPTPDB *tpdb, NPB *npb);

#pragma data_seg(_HEAD);
	LKEHEAD(TCPCLS, inittcp, MAJVER, MINVER, EDITNUM, CLASS); 
#pragma data_seg();

static long tcpdcbnum;			// Number of in use TCP DCBs
static long tcpdcbmax;			// Maximum in use TCP DCBs
static long tcpdcblmt = 10000;	// Maximum number of TCP DCBs allowed

TCPTPDB *tcpfirsttpdb;

// Transport level dispatch table for TCP

static TPDBDISP tcpdisp =
{	NULL,				// Link up
	NULL,				// Link down
///	tcpabort			// Abort output
///	rtgetaddr,			// getaddr function for protocol
};

// Define macro used to get the name of our DCB as a TCP DCB

#define ourdcb ((TCPDDCB *)knlTda.dcb)

// Define structure used to save information about parameter request

typedef struct
{	char *ipapnt;		// Address for storing remote IP address
	long  ipacnt;		// Size for remode IP address
	char *portpnt;		// Address for storing remote TCP port number
	long  portcnt;		// Size for remote TCP port number
} IOPDATA;


//*************************************************
// Device IO parameter tables for TCP class devices
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

static PARMTBL2 tcpnetparams = {19,
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

static PARMTBL1 tcpparams = {6,
{	&knlIoStdGenParams,		// 0x00xx - Generic IO parameters
	&knlIoDfltFilParams,	// 0x01xx - File system IO parameters
	&knlIoDfltTrmParams,	// 0x02xx - Terminal IO parameters
	NULL,					// 0x03xx - Disk IO parameters
	NULL,					// 0x04xx - Tape IO parameters
	&tcpnetparams			// 0x05xx - Network IO parameters
}};


//********************************************
// Device dispatch table for TCP class devices
//********************************************

static long XOSFNC tcpgivedcb(void);
static long XOSFNC tcpopen(char *spec);
static long XOSFNC tcpclose(void);
static long XOSFNC tcpinpblk(void);
static long XOSFNC tcpoutblk(void);

static const QFDISP tcpfdisp =
{	NULL,				// Mount
	tcpgivedcb,			// Clear DCB
	NULL,				// Open additional
	tcpopen,			// Open device/file
	NULL,				// Device parameters
	NULL,				// Delete file
	NULL,				// Rename file
	tcpinpblk,			// Input block
	tcpoutblk,			// Output block
	NULL,				// Get input/output status
	NULL,				// Special device function
	tcpclose,			// Close
	NULL,				// Get device label
	NULL,				// Commit data to disk
	NULL,				// Get device info
	NULL				// Verify changed disk
};


// Class characteristics tables for the TCP device class

SINFO(msgnumber , "Number of in use TCP devices");
SINFO(msgmaximum, "Maximum number of in use TCP devices");
SINFO(msglimit  , "Maximum number of TCP devices allowed");

static const CHARTBL tcpcctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(NUMBER  , DECV, U, 4, &msgnumber , sysChrGetULong, NULL        ,
		&tcpdcbnum),
  CTITEM(MAXIMUMX, DECV, U, 4, &msgmaximum, sysChrGetULong, sysChrSetULong,
		&tcpdcbmax),
  CTLAST(LIMIT   , DECV, U, 4, &msglimit  , sysChrGetULong, sysChrSetULong,
		&tcpdcblmt)
}};

// Class function dispatch table for TCP class devices

static long XOSFNC tcpaddunit(void); 

static CFDISP tcpcls = { 3,
{	tcpaddunit,			// CF_ADDUNIT =  8 - Add unit
	NULL,				// CF_PUNITS  =  9 - Get information about
						//                     physical units
	NULL				// CF_AUNITS  = 10 - Get information about active
}};


static const CHARTBL tcpdctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(CLASS   , TEXT, U,  8, &knlChrMsgClass  , sysChrDevGetClass,
		sysChrDevChkClass, 0),

  CTITEM(IPDEV   , TEXT, U, 20, &xosipMsgIpDev  , xosipGetIpDev,
		NULL             , 0),

  CTITEM(PROTOCOL, DECV, U,  4, &xosipMsgIpProt , xosipGetTpdb4,
		NULL             , offsetof(TCPTPDB, tplb.protocol)),

  CTITEM(PKTIN   , DECV, U,  4, &xosnetMsgPktIn  , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(TCPTPDB, cntpktin)),

  CTITEM(BYTEIN  , DECV, U,  4, &knlChrMsgByteIn , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(TCPTPDB, cntbytein)),

  CTITEM(PKTOUT  , DECV, U,  4, &xosnetMsgPktOut , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(TCPTPDB, cntpktout)),

  CTITEM(BYTEOUT , DECV, U,  4, &knlChrMsgByteOut, xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(TCPTPDB, cntbyteout)),

///  CTITEM(BADADDR , DECV, U,  4, &msgipbadaddr    , xosipGetTpdb4,
///		xosipSetTpdb4   , offsetof(TCPTPDB, ipcntbadaddr)),

  CTITEM(BADHDR  , DECV, U,  4, &xosnetMsgBadHdr , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(TCPTPDB, cntbadhdr)),

  CTITEM(PSLTMN  , DECV, U,  4, &xosnetMsgPSLtMn , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(TCPTPDB, cntpsltmn)),

  CTLAST(PSLTDL  , DECV, U,  4, &xosnetMsgPSLtDL , xosipGetTpdb4,
		xosipSetTpdb4   , offsetof(TCPTPDB, cntpsltdl)),

///  CTLAST(DLLTHL  , DECV, U,  4, &xosnetMsgDLLtHL , xosipGetTpdb4,
///		xosipSetTpdb4   , offsetof(TCPTPDB, ipcntdllthl))
}};


//************
// The TCP CCB
//************

static DEVCHKFNC tcpdevchk;

CCB tcpccb =
{	'CCB*',
	0,					// next    - Address of next CCB
	"TCP",				// name    - Name of this class
	0xFFFFFF,			// npfxmsk - Name prefix part mask
	'TCP',				// npfxval - Name prefix part value
	tcpdevchk,			// devchk  - Address of device check function
	&tcpcls,			// fdsp    - Address of class func disp table
	&tcpcctbl			// clschar - Address of class char table
};


static void tcpgiveport(void);


static CHARFNC tcpauunit;
static CHARFNC tcpauipunit;
static CHARFNC tcpauprotocol;
static CHARFNC tcpaupriority;


static const CHARITEM tcpaublk[] =
{	CTITEM(UNIT    , DECV, U, 4, NULL, NULL, xosnetAUUnit    , 0),
	CTITEM(NETDEV  , DECV, U, 4, NULL, NULL, xosnetAUNetDev  , 0),
	CTITEM(PROTOCOL, DECV, U, 4, NULL, NULL, xosnetAUProtocol,
			(1540<<16)+0xFFFF),
	CTLAST(PRIORITY, DECV, U, 4, NULL, NULL, xosnetAUPriority, 0)
};


//************************************************************
// Function: inittcp - Initialization routine
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

#pragma code_seg ("x_ONCE");

static long XOSFNC inittcp(
	char **pctop,
	char **pdtop)
{
	long rtn;

	sysChrFixUp((CHARTBL *)&tcpcctbl);
	sysChrFixUp((CHARTBL *)&tcpdctbl);
	sysChrFixUp((CHARTBL *)&tcpaublk);
	if ((rtn = sysIoNewClass(&tcpccb)) >= 0)
		*pctop = codetop;
	return (rtn);
}

#pragma code_seg ();


//************************************************************
// Function: tcpaddunit - Implements the CL_ADDUNIT function
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC tcpaddunit(void)
{
	TCPTPDB  *tpdb;
	NETAUDATA data;
	long      rtn;

	if (knlTda.buffer2 == NULL)
		return (ER_CHARM);
	data.dev.l[0] = -1;
	data.unit = -1;
	data.protocol = -1;
	data.priority = 0x7FFF;
	if ((rtn = sysChrValuesNew(TRUE, tcpaublk, &data)) < 0)
		return (rtn);
	if ((data.unit | data.dev.l[0] | data.protocol) == -1)
		return (ER_CHARM);
	if ((rtn = xosnetMakeTransport(data.dev.c,  "TCP", data.unit,
			data.protocol, offsetof(TCPPKT, options), sizeof(TCPTPDB),
			(RECVFNC *)tcprecvpkt, &tcpdisp, &tcpccb, (TPDB **)&tpdb)) < 0)
		return (rtn);
	tpdb->devdesc = DS_DUPLEX|DS_QIN|DS_QOUT|DS_DINDFT|DS_DOUTDFT|DS_DIN|
			DS_DOUT;
	xosnetLinkTplb(tpdb->npdb, &tpdb->tplb, data.priority);
	tpdb->nextp = (TPDB *)tcpfirsttpdb;
	tcpfirsttpdb = (TCPTPDB *)tpdb;
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
// Function: tcpdevchk - Device check function for TCP class devices
// Returned: 0 if no match, 1 if found a match, 2 if need to restart
//           search, or a negative XOS error code if error.
//******************************************************************

static long XOSFNC tcpdevchk(
	char  name[16],		// Device name (16 bytes, 0 filled)
	char *path,			// Path specification
	long  rtndcb)
{
	TCPTPDB *tpdb;
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
		tpdb = tcpfirsttpdb;
		while (tpdb != NULL)
		{
			if (unit == tpdb->unit)
			{
				if ((rtn = xosnetMakeDcb(name, unit, sizeof(TCPDDCB), &tcpccb,
						tpdb->idb, &tcpdctbl, &tcpfdisp,
						tpdb->devdesc)) < 0)
					return (rtn);
				ourdcb->npdb = (NPDB *)tpdb->npdb;
				ourdcb->tpdb = (TPDB *)tpdb;
				ourdcb->outpndmax = 10;
				return (1);
			}
			tpdb = (TCPTPDB *)tpdb->nextp;
		}
		return (ER_NSDEV);
	}
	return (0);
}



#IF 0

	.SBTTL	sd_devinfo - Get device information

;Here for the get device information entry
;	c{ECX}    = Length of user buffer
;	c{ES:EDI} = Address of user buffer
;	c{ESI}    = Offset of DCB
;	CALL	tcpdevinfo

$$$=!0				;Must match definitions in IPSCLS!
FRM info_left  , 4t
FRM info_amount, 4t
info_SIZE=!$$$

xostcpDevInfo:
	ENTER	info_SIZE, 0
	MOVL	info_left[EBP], ECX
	CLRL	EAX
	MOVL	info_amount[EBP], EAX
	MOVZBL	EDX, dcb_tcpstate[ESI]	;Get TCP state
	CMPB	DL, #TCPS_LISTEN	;Is it TCPS_LISTEN?
	JNE	4$			;No
	CMPW	dcb_tcpconlimit[ESI], #0 ;Yes - have a connection queue?
	JE	4$			;No
	MOVB	DL, #12t		;Yes - fix up the state name
4$:	LEAL	EBX, CS:infoname[EDX*4]	;Get offset of name string
	MOVL	ECX, #4			;Its 4 characters long
	CALL	knlInfoStr##		;Store the name
	MOVB	AL, #' '
	CALL	knlInfoChr##
	JMP	xosipsDevInfoLcl##

	DATA
	.MOD	4
infoname:
	.ASCII	"CLSD"		;TCPS_CLOSED    = 0  - Idle
	.ASCII	"LSTN"		;TCPS_LISTEN    = 1  - Waiting for SYN
	.ASCII	"SYNS"		;TCPS_SYNSENT   = 2  - SYN sent
	.ASCII	"SYNA"		;TCPS_SYNACKD   = 3  - ACK sent for SYN|ACK
	.ASCII	"SYNR"		;TCPS_SYNRCVD   = 4  - SYN received
	.ASCII	"ESTB"		;TCPS_ESTAB     = 5  - Connection established
	.ASCII	"FIN1"		;TCPS_FINWAIT1  = 6  - FIN sent, waiting for FIN
				;			 in reply and for ACK
	.ASCII	"FIN2"		;TCPS_FINWAIT2  = 7  - ACK for FIN received,
				;			 still waiting for FIN
	.ASCII	"CLSG"		;TCPS_CLOSING   = 8  - Reply FIN received
				;			 without ACK, waiting
				;			 for ACK for the FIN
	.ASCII	"CLSW"		;TCPS_CLOSEWAIT = 9  - FIN received, waiting
				;			 for process to close
				;			 TCP device
	.ASCII	"LACK"		;TCPS_LASTACK   = 10 - Reply FIN sent, waiting
				;			 for final ACK
	.ASCII	"TMWT"		;TCPS_TIMEWAIT  = 11 - All finished - timing out
				;			 port
	.ASCII	"LSTQ"
	CODE
.PAGE

#endif


#typedef struct
{

} PDATA;


//************************************************************
// Function: tcpopen - Handle the open entry - Open device
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC tcpopen(
	char *spec)
{
	PDATA data;

	if (knlTda.cmd & (XO_REPEAT|XO_REQFILE|XO_ODF|XO_FAILEX|XO_CREATE|
			XO_TRUNCA|XO_TRUNCW|XO_APPEND|XO_UNQNAME|XO_CONTIG|XO_FAPPEND))
		return (ER_IFDEV);

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&ipparams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		return (rtn);

	if ((spec[0] != 0) && (rtn = xosipParseAddress(spec, FALSE)) < 0)
		return (rtn);

	if (knlTda.qfunc == QFNC_DEVPARM)
		knlTda.cmd &= ~(XO_IN|XO_OUT);
	if ((knlTda.cmd & (XO_IN|XO_OUT) == 0)
		return (0);

	// Here if want to establish a network connection. There are 3
	//   possibilities:
	//   1) Passive (in-coming) type 1 connection
	//   2) Passive (in-coming) type 2 connection
	//   3) Active (out-going) connection

	if (ourdcb->rmtneta != 0)			// Was a remote IP address specified?
	{
		// Here with a remove IP address - This is an active (out-going) open.

		if ((rtn = xostcpConnect()) < 0)
			return (rtn);

	}
	else if (ourdcb->tcpconhndl != 0)	// Was a connection handle specified?
	{
		// Here with a connection handle - This is the second part of a
		//   type 2 passive open.



	}
	else
	{
		// Here to start a passive open

		if (ourdcb->tcplclport == 0)	// A local port must be specified
			return (ER_NLPNS);
		if (ourdcb->tcpconlimit == 0)	// Was a connection limit specified?
		{
			if ((rtn = waitforconnect()) < 0) // No - wait for a connection
				return (rtn);
		}
	}

	// Here with the connection complete - Return the remote address if he
	//   asked for it. (0 is returned if starting type 2 passive connection.)




	return (0);
}



;Here for the devparm function or if physical or raw IO requested

tcpdevparm:
	ANDB	SS:xffCmd##, #~{O$IN|O$OUT} ;No IO allowed
	JMP	opendn			;Thats all
.PAGE
;Here for type 2 passive open - We first find the indicated DCB.  If there
;  is a pending SYN sequence queued, we use it.  If not, we wait until there
;  is one. Note that only one DCB can be waiting for a connection using a
;  base DCB.

tcpopentype2:
	MOVZWL	EAX, opn_conhndl[EBP]
	PUSHL	EDI
	CALL	knlXfGetDcb##
	MOVL	EDX, EDI
	POPL	EDI
	JC	4$
	CMPL	dcb_ccb[EDX], #tcpccb	;Is this a TCP DCB?
	JNE	4$			;No
	MOVB	AL, dcb_punit[EDI]	;Yes - is it the same unit?
	CMPB	AL, dcb_punit[EDX]
	JNE	4$			;No
	CMPB	dcb_tcpstate[EDX], #TCPS_LISTEN
	JNE	4$
	CMPW	dcb_tcpconlimit[EDX], #0
	JE	4$
	CMPL	dcb_tcpconbase[EDX], #0
	JE	8$
4$:	MOVL	EAX, #ER_PARMV
	STC
6$:	RET

;Here with a valid device for type 2 incoming connections

8$:	MOVZWL	ECX, dcb_ipslclport[EDX]
	CMPW	CX, dcb_ipslclport[EDI]
	JE	10$
	BTSL	ECX, #30t
	PUSHL	EDX
	CALL	xosipsGetPort##
	POPL	EDX
	JC	6$
10$:	CMPL	dcb_tcpconwait[EDX], #0	;Is a connection available now?
	JNE	14$			;Yes
	MOVB	dcb_tcpstate[EDI], #TCPS_LISTEN ;Update the connection state
	MOVL	dcb_tcpconbase[EDX], EDI
	MOVL	dcb_tcpconbase[EDI], EDX
	PUSHL	EDX
	MOVL	EAX, SS:xffTimeOut##
	MOVL	EDX, SS:xffTimeOut##+4
	CALL	xosnetSndWait##		;Wait for a while
	POPL	EDX
	JNC	10$
	RET

;Here with an incoming connection to use

14$:	CLRL	EAX			;Unlink from the base DCB for this port
	MOVL	dcb_tcpconbase[EDX], EAX
	MOVL	dcb_tcpconbase[EDI], EAX
	MOVL	EBX, dcb_tcpconwait[EDX]
	MOVL	ECX, dcb_tcpconnext[EBX]
	MOVL	dcb_tcpconwait[EDX], ECX
	MOVL	dcb_tcpconnext[EBX], EAX
	MOVL	dcb_tcpconbase[EBX], EAX
	PUSHL	EDI
	MOVL	EDI, EBX
	MOVL	ESI, dcb_tcpackwake[EDI] ;Clear the ACK timer if its running
	TESTL	ESI, ESI
	JE	16$
	MOVL	dcb_tcpackwake[EDI], #0
	CALL	knlWakeRemove##
16$:	MOVL	EAX, dcb_tcpprmerr[EDI] ;Get error code
	TESTL	EAX, EAX
	JNS	18$			;Go on if no error
	PUSHL	EAX
	CALL	xostcpGiveDcb		;Give up the temporary DCB
	POPL	EAX			;Restore error code
	POPL	EDI			;Restore DCB offset
	STC
	RET

;Here if no error on the open

18$:	PUSHL	EDI
	LEAL	ESI, dcb_tcpcopybgn[EDI]
	MOVL	EBX, 4[ESP]
	MOVL	EAX, dcb_ipsrmtaddr[EDI]
	MOVL	dcb_ipsrmtaddr[EBX], EAX
	MOVW	AX, dcb_ipsrmtport[EDI]
	MOVW	dcb_ipsrmtport[EBX], AX
	LEAL	EDI, dcb_tcpcopybgn[EBX]
	MOVL	ECX, #{dcb_tcpcopyend-dcb_tcpcopybgn}/4
	CLD
	PUSHL	DS
	POPL	ES
	RMOVSL	[EDI], [ESI]
	MOVL	EDI, [ESP]
	ADDL	EDI, #dcb_tcpcopybgn
	MOVL	ECX, #{dcb_tcpcopyend-dcb_tcpcopybgn}/4
	CLRL	EAX
	RSTOSL	[EDI]
	POPL	EDI
	MOVZWL	EAX, dcb_ipslclport[EDI]
	PUSHL	EAX
	CALL	xostcpHashClr#		;Remove DCB from the hash table
	CALL	xosipsGivePort##	;Give up its local port
	CALL	xostcpGiveDcb		;Give up the DCB
	POPL	ECX
	BTSL	ECX, #30t
	POPL	EDI
	CALL	xosipsGetPort##
	CALL	tcpcalhash#
	CALL	xostcpHashSet#		;Put DCB into the hash table
	CALL	xostcpSendAck#		;Send an ACK if we need to
	JMP	opendn
.PAGE
;Here for type 1 passive open to wait for an in-coming connection

tcpopenpas:
	MOVL	dcb_tcprndave[EDI], #{TIMEOUT_TRNBGN/TICKPERSP}<3t
	MOVL	dcb_tcprndvar[EDI], #0
	MOVL	dcb_tcprxmtint[EDI], #TIMEOUT_TRNBGN/TICKPERSP+4
	CALL	xostcpPutSynWait	;Put this DCB in the Syn wait list
	MOVL	EAX, SS:xffTimeOut##	;Get time-out value
	MOVL	EDX, SS:xffTimeOut##+4
	CALL	xosnetRspWait##		;Wait for a SYN
	JC	8$			;If error
	CALL	xostcpRmvSynWait	;Remove DCB from the SYN wait list
tcpopenin:
	CALL	xostcpTimerRmv#		;Stop the connect timer
	CALL	xostcpAcceptSyn		;Accept the connection
	MOVB	SS:xffCount##, #4-1	;Retry this 4 times
2$:	CALL	xostcpSendSynAck
	JC	6$
	MOVL	EAX, dcb_tcprcvwinb[EDI]
	MOVL	dcb_tcprcvwina[EDI], EAX
	MOVL	dcb_tcprcvwinm[EDI], EAX
	MOVL	dcb_tcprndave[EDI], #{TIMEOUT_TRNBGN/TICKPERSP}<3t
	MOVL	dcb_tcprndvar[EDI], #0
	MOVL	dcb_tcprxmtint[EDI], #TIMEOUT_TRNBGN/TICKPERSP+4
	MOVL	EAX, #TIMEOUT_CON	;Get time-out value
	CLRL	EAX
	CALL	xosnetSndWait##		;Wait for the ACK
	JNC	opendn			;If OK
	CMPL	EAX, #ER_NORSP		;Did this time-out?
	JNE	opnerr			;No
	DECB	SS:xffCount##		;Yes - should we try again?
	JNS	2$			;Yes
	MOVB	AL, #ER_NCLST		;No - get right error code
opnerr:	PUSHL	EAX			;Save error code
	CALL	xostcpHashClr#		;Remove DCB from the hash table
	MOVB	dcb_tcpstate[EDI], #TCPS_CLOSED
4$:	POPL	EAX
	STC
6$:	RET

;Here with error while we are in the SYN wait list

8$:	PUSHL	EAX
	CALL	xostcpRmvSynWait
	JMP	4$
.PAGE


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

#if 0

xostcpGiveDcb:
	CMPL	dcb_tcpackwake[EDI], #0
	JE	6$
	PUSHL	ESI
	MOVL	ESI, dcb_tcpackwake[EDI]
	MOVL	dcb_tcpackwake[EDI], #0
	CALL	knlWakeRemove##
	POPL	ESI
6$:	CMPW	dcb_tcpconlimit[EDI], #0 ;Is this a base DCB?
	JE	20$			;No
	PUSHL	EDI			;Yes
10$:	CMPL	dcb_tcpconpend[EDI], #0 ;Give up all pending DCBs
	JE	12$
	MOVL	EDI, dcb_tcpconpend[EDI]
	CALL	xostcpGiveDcb
	MOVL	EDI, [ESP]
	JMP	10$

12$:	CMPL	dcb_tcpconwait[EDI], #0 ;Give up all waiting DCBs
	JE	14$
	MOVL	EDI, dcb_tcpconwait[EDI]
	CALL	xostcpGiveDcb
	MOVL	EDI, [ESP]
	JMP	12$

14$:	CMPL	dcb_tcpconbase[EDI], #0	;Is there a DCB waiting for
					;  connections?
	JE	16$			;No
	MOVL	EDX, dcb_tcpconbase[EDI] ;Yes - unlink it from this DCB
	CLRL	EAX
	MOVL	dcb_tcpconbase[EDI], EAX
	MOVL	dcb_tcpconbase[EDX], EAX
	MOVL	EDI, EDX
	MOVL	EAX, #ER_ABORT		;Terminate it
	MOVL	EDX, dcb_tcpfuncdisp[EDI]
	CALL	xosnetSndDone##
16$:	POPL	EDI
	JMP	38$

;Here if this is not a base DCB

20$:	CMPL	dcb_tcpconbase[EDI], #0	;Is this DCB involved in a type 2
					;  open?
	JE	38$			;No - go on
	MOVL	EAX, dcb_tcpconbase[EDI] ;Yes - is this the real DCB which is
	CMPL	EDI, dcb_tcpconbase[EAX]
	JNE	22$			;No
	MOVL	dcb_tcpconbase[EAX], #0	;Yes
	JMP	36$

;Here if this is a temporary type 2 connection DCB

22$:	ADDL	EAX, #dcb_tcpconpend	;First look for it in the pending list
24$:	CMPL	[EAX], #0
	JE	26$
	CMPL	[EAX], EDI
	JE	34$
	MOVL	EAX, [EAX]
	ADDL	EAX, #dcb_tcpconnext
	JMP	24$

26$:	MOVL	EAX, dcb_tcpconbase[EDI] ;Then look for it in the waiting list
	ADDL	EAX, #dcb_tcpconwait
28$:	CMPL	[EAX], #0
	JE	30$
	CMPL	[EAX], EDI
	JE	34$
	MOVL	EAX, [EAX]
	ADDL	EAX, #dcb_tcpconnext
	JMP	28$

30$:	CRASH	BCNL			;[Bad CoNnection List]

34$:	MOVL	EDX, dcb_tcpconnext[EDI]
	MOVL	[EAX], EDX
36$:	MOVL	dcb_tcpconbase[EDI], #0

;Here with the type 2 open stuff cleaned up if necessary

38$:	CMPB	dcb_tcpstate[EDI], #TCPS_LISTEN
	JNE	40$
	CMPL	dcb_ipshashnext[EDI], #-1
	JE	40$
	CALL	xostcpRmvSynWait
40$:
;;;;;	CMPB	dcb_tcpstate[EDI], #TCPS_CLOSED ;Is it idle now?
;;;;;	JNE	46$			;No
tcpgdcb::
	MOVB	dcb_tcpstate[EDI], #0	;Yes
42$:	CALL	xostcpHashClr#		;Remove DCB from the hash table
	CALL	xostcpGiveAll#		;Make sure no buffers allocated
	CMPW	dcb_ipslclport[EDI], #0	;Have a local port number?
	JE	44$			;No
	CALL	xosipsGivePort##	;Yes - give it up
44$:	MOVL	ESI, dcb_tcpoutbufr[EDI] ;Do we have an output buffer?
	TESTL	ESI, ESI
	JE	45$			;No
	MOVL	EDX, dcb_tcpoutsize[EDI] ;Yes - give it up
	CALL	knlGetIndex##
	CALL	knlGiveXmb##
45$:	JMP	xosipsGiveDcb##		;Give up the DCB

;Here if DCB is not idle

46$:	MOVL	EAX, tcptwdcb		;Link to the timed wait list
	MOVL	dcb_tcptwnext[EDI], EAX
	MOVL	tcptwdcb, EDI
	MOVB	dcb_tcptwcnt[EDI], #TIMECNT_TW
	RET				;Thats all for now (will get to tcpgdcb
					;  when this timer goes off)


#endif

}




#if 0


	.SBTTL	tcpioplclneta - Here for the IOPAR_NETLCLNETA parameter

	DPARMHDR GET, DECV
tcpioplclneta:
	MOVL	EAX, dcb_netpdb[EDI]
	MOVL	EAX, pdb_ipaddr[EAX]
	JMP	knlStrParm4##

	.SBTTL	tcpioplclport - Here for the IOPAR_NETLCLPORT parameter for open

;Here for the IOPAR_NETLCLPORT parameter - Set or get local port number for open

	DPARMHDR  BOTH, DECV
tcpioplclport:
	TESTB	AH, #PAR$GET		;Want value returned?
	JE	10$			;No
2$:	MOVZWL	EAX, dcb_ipslclport[EDI] ;Yes - return current value
	JMP	knlStrParm4##

	.SBTTL	tcpxplclport - Here for the IOPAR_LCLPORT parameter for transfer

;Here for the IOPAR_LCLPORT parameter - Get local port number for transfer

	DPARMHDR  GET, DECV
tcpxplclport:
	JMP	2$

	.SBTTL	tcpioprmtports - Here for the IOPAR_RMTPORS parameter for open

;Here for the IOPAR_RMTPORTS parameter - Get or set remote port for open

	DPARMHDR  BOTH, DECV
tcpioprmtports:
	TESTB	AH, #PAR$GET		;Want value returned?
	JE	10$			;No
4$:	MOVZWL	EAX, dcb_ipsrmtport[EDI] ;Yes - return current value
6$:	XCHGB	AL, AH			;Fix up the byte order
	JMP	knlStrParm4##

	.SBTTL	tcpioprmtportr - Here for the IOPAR_RMTPORTR parameter

;Here for the IOPAR_RMTPORTR parameter - Get remote port number

	DPARMHDR  GET, DECV
tcpioprmtportr:
	JMP	4$

	.SBTTL	tcpioprmtnetas - Here for the IOPAR_RMTNETAS parameter for open

;Here for the IOPAR_RMTNETAS parameter - Get remote network address for open

	DPARMHDR  BOTH, DECV
tcpioprmtnetas:
	TESTB	AH, #PAR$GET		;Want value returned?
	JE	10$			;No
8$:	MOVL	EAX, dcb_ipsrmtaddr[EDI] ;Yes - return current value
	JMP	knlStrParm4##

10$:	RET

	.SBTTL	tcpioprmtnetar - Here for the IOPAR_RMTNETAR parameter

;Here for the IOPAR_RMTNETAR parameter - Get remote network address

	DPARMHDR  GET, DECV
tcpioprmtnetar:
	JMP	8$
.PAGE
;Here for the IOPAR_NETSMODE parameter - Set network mode bits

	DPARMHDR  BOTH, DECV
tcpioprmsmode:
	TESTB	AH, #PAR$SET		;Want to change value?
	JE	4$			;No
	PUSHL	EAX			;Yes
	CALL	knlGetParm##		;Get bits to set
	JC	6$
	ORL	dcb_netmode[EDI], EAX ;Set bits
	JMP	2$

;Here for the IOPAR_NETCMODE parameter - Clear network mode bits

	DPARMHDR  BOTH, DECV
tcpioprmcmode:
	TESTB	AH, #PAR$SET		;Want to change value?
	JE	4$			;No
	PUSHL	EAX			;Yes
	CALL	knlGetParm##		;Get bits to clear
	JC	6$
	NOTL	EAX			;Clear bits
	ANDL	dcb_netmode[EDI], EAX
2$:	POPL	EAX
4$:	TESTB	AH, #PAR$GET		;Want value returned?
	JE	8$			;No
	MOVL	EAX, dcb_netmode[EDI]	;Yes - get value
	JMP	knlStrParm4##		;Give it to him

6$:	POPL	ECX			;Fix up the stack
8$:	RET
.PAGE
	.SBTTL	xostcpConnect - Subroutine to establish TCP connection for open

;Subroutine to establish TCP connection for open - must be called in extended
;  fork context
;	c{EAX} = Receive byte window size
;	c(CL)  = Send packet window size
;	c{EDX} = Offset of name buffer (on stack)
;	c{EDI} = Offset of DCB
;	CALL	xostcpConnect
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal
;	  c{EDX} = Offset of next part of name

;A network address may be specified after the device name.  It must be
;  terminated with a double colon as follows:
;	TCP0:147.23.45.67.34::
;  The fifth byte is optional, if included, specifies the value to be used
;  for the low order byte of the local network address.

$$$=!0
FRM con_frame1 , 4
FRM con_frame2 , 4
FRM con_name   , 4	;Offset of name buffer
FRM con_retry  , 4	;Retry counter
FRM con_bfr    , 4	;Offset of packet buffer used to send SYNC
con_SIZE=!$$$

xostcpConnect:
	ENTER	con_SIZE, 2
	MOVL	dcb_tcprcvwinb[EDI], EAX ;Store our receive byte window size
	MOVL	dcb_tcprcvwinm[EDI], EAX
	MOVL	ESI, dcb_netidb[EDI]	;Get offset of our IDB
	BTL	SS:xffHvValue1##, #IOV1%RMTNETAS ;Have an address parameter?
	JNC	4$			;No	
	CALL	xosnetChkNetAddr##	;Yes - skip possible address string
	JNE	2$			;If none
	LEAL	EDX, 1[ECX]
2$:	MOVL	EBX, con_frame1[EBP]	;Yes - get its value
	MOVL	EBX, SS:opn_rmtnetas[EBX]
	JMP	gethva1			;Continue

;Here if address parameter (IOPAR_RMTNETAS) not specified

4$:	CALL	xosnetChkNetAddr##	;Was a network address given?
	JNE	getpds			;No - fail!

;Here if a network address was given

	CMPB	SS:[EDX], #'A'		;Is the first character alpha?
	JB	getipa			;No - go collect Internet address
	CALL	xosipsDnsGetIpA##	;Yes - translate domain name
	JNC	gethva2			;Go on if OK
	LEAVE				;Error
	RET
.PAGE
getipa:	CLRL	EBX
	MOVL	ECX, #4			;Remote IP address may contain up to 4
2$:	CALL	getval			;  fields
	RORL	EBX, #8
	CMPB	AL, #':'		;End of remote IP address?
	JE	10$			;Yes
	CMPB	AL, #'.'		;No - is it the separator?
	JNE	getpds			;No - fail
	LOOP	ECX, 2$			;Yes - continue
getpds:	MOVL	EAX, #ER_NILAD
6$:	STC
	LEAVE
	RET

;Here with internet address
;	c{EBX} = IP address

10$:	INCL	EDX			;Skip the second colon
gethva1:CMPL	EBX, #0			;Is entire address 0?
	JE	getpds			;Yes - fail!
gethva2:MOVL	con_name[EBP], EDX	;Save offset of name buffer
	MOVL	EDX, dcb_netpdb[EDI]	;Get offset of the PDB
	TESTL	EBX, pdb_ipsubnetmask[EDX] ;Did he specify the network part?
	JNE	12$			;Yes
	MOVL	EAX, pdb_ipaddr[EDX]	;No - use our network address
	ANDL	EAX, pdb_ipsubnetmask[EDX]
	ORL	EBX, EAX
12$:	MOVL	dcb_ipsrmtaddr[EDI], EBX ;Store IP address
	ANDB	dcb_ipssts1[EDI], #~IPSS1$RROUTE ;Clear removed route flag
14$:	CALL	knlGetSysHRDtTm##	;Get current fractional days
	MOVL	ECX, dcb_ipstpdb[EDI]
	ADDL	EAX, tpdb_tcpseqoffset[ECX] ;Add in our offset
	MOVL	dcb_tcpoutsnum[EDI], EAX ;Set as our initial sequence number
	MOVL	dcb_tcprcvanum[EDI], EAX
	CLRL	EAX			;Set acknowledgement number to 0
	MOVL	dcb_tcprcvsnum[EDI], EAX
	CALL	tcpcalhash#		;Calculate hash index
	CALL	xostcpHashSet#		;Put DCB into the TCP hash table
	MOVL	ECX, #60t
	CALL	xosnetGetBufr##		;Get a network buffer
	JC	tcpconf			;If no buffer available
	MOVL	con_bfr[EBP], EBX	;OK
	CALL	tcpsetpkt#		;Set up the TCP packet header
	JC	16$
	MOVZBL	ECX, npb_tpofs[EBX]
	MOVL	EAX, dcb_tcpoutsnum[EDI]
	MOVL	npb_seqnum[EBX], EAX
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	MOVL	tcp_seqnum[EBX+ECX], EAX
	ORB	tcp_code[EBX+ECX], #TCPC$SYN ;Set the SYN bit
	MOVL	EAX, dcb_tcprcvwinb[EDI]
	MOVL	dcb_tcprcvwina[EDI], EAX
	XCHGB	AL, AH
	MOVW	tcp_window[EBX+ECX], AX
	ADDB	tcp_offset[EBX+ECX], #0x20 ;We will store 8 option bytes
	MOVL	EAX, dcb_ipstpdb[EDI]	;Store the maximum segment size option
	MOVL	EAX, tpdb_tcpmaxseg[EAX]
	XCHGB	AL, AH
	SHLL	EAX, #16t
	MOVW	AX, #0x0402
	MOVL	[EDX], EAX
	MOVL	4[EDX], #0
	ADDL	npb_count[EBX], #8
	MOVL	dcb_tcprndave[EDI], #{TIMEOUT_TRNBGN/TICKPERSP}<3t
	MOVL	dcb_tcprndvar[EDI], #0
	MOVL	dcb_tcprxmtint[EDI], #TIMEOUT_TRNBGN/TICKPERSP+4
	MOVB	dcb_tcpstate[EDI], #TCPS_SYNSENT ;Set connection state
	MOVB	con_retry[EBP], #RETRY_CON ;Initialize retry counter
	CALL	tcpchksum#
	CALL	xosipsSendSetup##	;Finish setting up the IP header
tcpcon2:MOVL	ESI, dcb_netsdb[EDI]	;Make sure have SDB offset
	MOVL	EAX, #xosnetOutDone##	;Output the packet
	CALL	xossnpSendPkt##
	JNC	15$
	TESTL	EAX, EAX
	JNE	tcpcone
	INT3

;;;	JC	tcpcone

15$:

	MOVL	ESI, dcb_netidb[EDI]
	MOVL	EAX, #-1		;Wait here so will see possible ARP
	MOVL	EDX, EAX		;  errors
	CALL	xosnetWaitPkt##

16$:
	JNC	tcpcon4
	TESTL	EAX, EAX
	JNE	tcpcone
	INT3

;;;	JC	tcpcone			;If error

tcpcon4:CMPL	dcb_tcprcvputb[EDI], #0	;Do we have a response yet?
	JNE	tcpcon6			;Yes (unusual but not impossible!)
	MOVL	EAX, #TIMEOUT_TRNBGN	;No - wait for the response
	CLRL	EDX
	CALL	xosnetRspWait##
	JNC	tcpcon6			;If normal
	MOVL	EBX, con_bfr[EBP]	;Restore pointer to our packet
	CMPL	EAX, #ER_NORSP		;Time-out error?

	JE	17$
	TESTL	EAX, EAX
	JNE	tcpcone
	INT3

;;;	JNE	tcpcone			;No - fail now

17$:	DECB	con_retry[EBP]		;Should we do it again?
	JS	18$			;No
	MOVL	EAX, dcb_ipstpdb[EDI]	;Yes
	INCL	tpdb_tcpcntrexmit[EAX]	;Count the retransmission
	MOVL	EBX, con_bfr[EBP]
	JMP	tcpcon2			;Continue

;Here if have tried enough

18$:	TESTB	npb_sts[EBX], #NPS$USEDARP ;Did we use ARP this time?
	JNE	20$			;Yes - fail
	TESTB	dcb_ipssts1[EDI], #IPSS1$RROUTE ;Just to be safe, have we done
						  ;  this before here?
	JNE	20$			;Yes - fail!
	ORB	dcb_ipssts1[EDI], #IPSS1$RROUTE ;No - remember we have now
	MOVL	EAX, dcb_ipsrmtaddr[EDI] ;Remove this entry from our routing
	PUSHL	EDI			 ;  table
	MOVL	ESI, dcb_netsdb[EDI]	;Make sure have SDB offset
	MOVL	EDI, dcb_netpdb[EDI]
	MOVB	DL, #0
	CALL	xosipsRtRmvOne##
	POPL	EDI
	MOVL	EBX, con_bfr[EBP]
	JC	20$			;Forget it if error here
	CALL	xosnetGiveBufr##	;OK - Give up the buffer
	CALL	xostcpHashClr#		;Remove DCB from the hash table
	JMP	14$			;Go try again

;Here if cannot find destination node

20$:	MOVL	EAX, #ER_NSNOD		;Get error code
tcpcone:CALL	xosnetGiveBufr##	;Give up our buffer
tcpconf:MOVB	dcb_tcpstate[EDI], #TCPS_CLOSED ;Indicate idle
	CALL	xostcpGiveAll#		;Make sure don't have any buffers
	PUSHL	EAX
	CALL	xostcpHashClr#		;Remove DCB from the hash table
	CLRL	EAX
	MOVL	dcb_ipsrmtaddr[EDI], EAX ;Clear remote IP address
	POPL	EAX
	STC				;Indicate error
	LEAVE
	RET
.PAGE
;Here when get a reply to our initial SYN packet - the reply must have both
;  the SYN and ACK bits set (and no others) - if we get a reply that has just
;  the ACK bit set, then the other side must have a half-open connection - we
;  send a reset packet followed by another SYN packet - if we get just a SYN,
;  then it must have crossed his SYN - we send an ACK - if we get a SYN with
;  a FIN, the other side has rejected the connection - we reply with a FIN|ACK

tcpcon6:MOVL	EBX, dcb_tcprcvputb[EDI] ;Restore pointer to packet
	CLRL	EAX
	MOVL	dcb_tcprcvputb[EDI], EAX
	MOVL	EDX, dcb_tcprcvputp[EDI]
	MOVB	AL, tcp_code[EDX]
	ANDB	AL, #~TCPC$PSH
	CMPB	AL, #TCPC$SYN		;SYN only?
	JE	16$			;Yes
	TESTB	AL, #TCPC$ACK		;No - have ACK bit?
	JE	2$			;No - ignore it
	CMPB	al, #TCPC$ACK		;Yes - ACK bit only?
	JE	8$			;Yes - means a half open connection!
	MOVL	EAX, tcp_acknum[EDX]	;No - is this the one we want?
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	DECL	EAX
	CMPL	dcb_tcpoutsnum[EDI], EAX
	JNE	6$			;No - ignore it
	MOVB	AL, tcp_code[EDX]
	ANDB	AL, #~TCPC$PSH
	CMPB	AL, #TCPC$SYN|TCPC$ACK	;SYN + ACK?
	JE	18$			;Yes - go on
	TESTB	AL, #TCPC$RST|TCPC$FIN	;No - RST or FIN bits?
	JNE	10$			;Yes
2$:	MOVL	ECX, dcb_ipstpdb[EDI]	;No
	INCL	tpdb_tcpcntbadhdr[ECX]	;Count this
4$:	CALL	xosnetGiveBufr##	;And ignore the packet!
	JMP	tcpcon4			;Continue waiting for a response

;Here if ACK number is wrong

6$:	MOVL	ECX, dcb_ipstpdb[EDI]
	INCL	tpdb_tcpcntoutseq[ECX]	;Count the out of sequence packet
	JMP	4$			;Ignore the packet

;Here if have ACK only - this means the other side has a half open connection

8$:	PUSHL	dcb_tcpoutsnum[EDI]
	MOVL	EAX, tcp_acknum[EDX]
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	MOVL	dcb_tcpoutsnum[EDI], EAX
	CALL	xostcpSendReset#
	POPL	dcb_tcpoutsnum[EDI]

	JNC	9$
	TESTL	EAX, EAX
	JNE	tcpcone
	INT3


;;;	JC	tcpcone


9$:	MOVL	EBX, con_bfr[EBP]
	JMP	tcpcon2			;Go send the SYN again

;Here if have RESET or FIN with ACK

10$:	MOVL	EAX, #ER_NCRFS		;Get error code
12$:	CALL	xosnetGiveBufr##
14$:	MOVL	EBX, con_bfr[EBP]

	TESTL	EAX, EAX
	JNE	tcpcone
	INT3

;;;	JMP	tcpcone

;Here with SYN as response to our initial SYN

16$:	JMP	4$			;DO THIS SOON!!!

;Here with SYN|ACK as response to our initial SYN

18$:	MOVL	EAX, tcp_seqnum[EDX]	;Initialize the remote sequence number
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	INCL	EAX
	MOVL	dcb_tcprcvsnum[EDI], EAX
	DECL	EAX			 ;Set the sent number to 1 less so we
	MOVL	dcb_tcprcvtnum[EDI], EAX ;  will always send the first ACK
	MOVZWL	EAX, tcp_window[EDX]	;Store window value
	XCHGB	AL, AH
	MOVL	dcb_tcpoutwina[EDI], EAX
	MOVL	dcb_tcpoutwinm[EDI], EAX
	CALL	xosnetGiveBufr##	;Finished with this packet
	MOVL	EBX, con_bfr[EBP]	;Also give up our output packet
	CALL	xosnetGiveBufr##
	MOVB	dcb_tcpstate[EDI], #TCPS_ESTAB ;Update connection state
	INCL	dcb_tcpoutsnum[EDI]	;Fix up the send sequence number to
	INCL	dcb_tcprcvanum[EDI]	;Also fix the ACK number
	CALL	tcpsendacknow#		;Send an ACK
	CLRL	EAX			;Indicate success
	MOVL	EDX, con_name[EBP]	;Get pointer to rest of name
	LEAVE
	RET
.PAGE	
;Subroutine to get value of next network address field
;	c{EDX} = Offset of address string
;	CALL	getval
;	c(AL)  = Stopper character
;	c(BL)  = Value
;	c{EDX} = Offset of next character in address string

getval:	MOVB	AL, #0
2$:	MOVB	BL, SS:[EDX]		;Get character
	INCL	EDX			;Bump pointer
	CMPB	BL, #'0'		;Is it a digit?
	JB	4$			;No
	CMPB	BL, #'9'
	JA	4$
	ANDB	BL, #0Fh		;Yes - get value
	MULB	lit10			;And add into value for field
	ADDB	AL, BL
	JMP	2$

;Here if next character is not a digit

4$:	XCHGB	AL, BL			;Get things in right registers
	RET				;Return
.PAGE
	.SBTTL	sd_close - Close device

;Here for the close device entry
;	c{ECX} = Close option bits
;	c{EDI} = Offset of DCB
;	c{ESI} = Offset of device table entry
;	CALL	tcpclose
;	C:set = Error or not ready
;	  c{EAX} = Error code (0 means not ready)
;	C:clr = Normal

tcpclose:
	PUSHL	ESI
	CALL	sysIoCloseLock##	;Lock the QAB
	MOVB	iorb_queue[ESI], #dcb_outframe
	MOVL	iorb_routine[ESI], #tcpclose1
	JMP	knlXfQueue##		;Switch to extended fork context

;Here in extended fork context

tcpclose1:
	CMPL	SS:xffParm##, #0	;Have any device parameters?
	JE	2$			;No
	MOVL	EBX, #tcpioparms	;Yes - process parameters
	MOVL	ECX, #knlComDPParms##
	LFSL	EDX, SS:xffParm##
	CALL	knlProcDevParm##
	JC	4$
2$:	CLRL	EAX
4$:	CMPL	dcb_opencnt[EDI], #1	;Final close?
	JNE	8$			;No
	PUSHL	EAX			;Yes - save the current error code
	MOVL	ESI, dcb_netidb[EDI]	;Get offset of our IDB
	CALL	xostcpClear		;Terminate our connection
	JNC	6$
	MOVL	[ESP], EAX
6$:	POPL	EAX
8$:	JMP	knlXfCloseFin##		;Go finish up
.PAGE

#endif



//************************************************************
// Function: xostcpPutSynWait - Put DCB into the SYN wait list
// Returned: Nothing
//************************************************************

void XOSFNC __export xostcpPutSynWait(void)
{
	TCPTPDB *tpdb;

	if (ourdcb->tcpsynnext != (TCPDDCB *)-1)
		CRASH('ISWL');
	tpdb = ourdcb->tpdb;
	if ((ourdcb->tcpsynnext = tpdb->tcpsyntail)) == NULL)
		tpdb->tcpsynhead = ourdcb;
	else
		tpdb->tcpsyntail->tcpsynnext = ourdcb;
	tpdb->tcpsyntail = ourdcb;
	ourdcb->tcpstate = TCPS_LISTEN;
}


//***************************************************************
// Function: xostcpRmvSynWait - Remove DCB from the SYN wait list
// Returned: Nothing
//***************************************************************

void XOSFNC __export xostcpRmvSynWait(void)
{
	TCPTPDB *tpdb;
	TCPDDCB *dcb;
	TCPDDCB *prev;

	if (ourdcb->tcpsynnext == ((TCPDDCB *)-1) ||
			ourdcb->tcpstate != TCPS_LISTEN)
		CRASH('BTSS');

	tpdb = ourdcb->tpdb;

	dcb = tpdb->tcpsynhead;
	prev = NULL;

	do
	{
		if (dcb == ourdcb)
		{
			if (prev == NULL)
				tpdb->tcpsynhead = dcb->tcpsynnext;
			else
				prev->next = dcb->tcpsynnext;
			if (dcb->tcpsynnext == NULL)
				tpdb->tcpsyntail = prev;
			dcb->tcpsynnext = (TCPDDCB *)-1;
			return;				
		}
		prev = dcb;
	} while ((dcb = dcb->tcpsynnext) != NULL);
	CRASH('BSWL');
}

#if 0

	.SBTTL	xostcpAcceptSyn - Subroutine to accept connection when SYN received

;Subroutine to accept connection when SYN received on DCB in the TCPS_LISTEN
;  state
;	c{EDI} = Offset of DCB
;	CALL	xostcpAcceptSyn

xostcpAcceptSyn::
	MOVL	EBX, dcb_tcprcvputb[EDI] ;Point to IP header in packet
tcpacceptsyn::
	MOVB	dcb_tcpstate[EDI], #TCPS_SYNRCVD ;Update connection state
	CLRL	EAX
	MOVL	dcb_tcprcvputb[EDI], EAX
	MOVZBL	EDX, npb_npofs[EBX]	;Point to IP header in packet
	MOVL	EAX, ip_srcipaddr[EBX+EDX] ;Get the remote IP address
	MOVL	dcb_ipsrmtaddr[EDI], EAX
	MOVZBL	EDX, npb_tpofs[EBX]	;Point to TCP header in packet
	MOVW	AX, [EBX+EDX]		;Get the remote port number
	MOVW	dcb_ipsrmtport[EDI], AX
	MOVL	EAX, tcp_seqnum[EBX+EDX] ;Get packet sequence number
	XCHGB	AL, AH			;Fix up the byte order
	RORL	EAX, #16t
	XCHGB	AL, AH
	MOVL	dcb_tcprcvtnum[EDI], EAX
	INCL	EAX			;Plus one
	MOVL	dcb_tcprcvsnum[EDI], EAX ;Store as initial receive number
	MOVL	dcb_tcprcvwinb[EDI], #16000t ;Initialize receive window amount
	MOVL	dcb_tcprcvwinm[EDI], #16000t
	MOVZWL	EAX, tcp_window[EBX+EDX] ;Initialize send window amount
	XCHGB	AL, AH
	MOVL	dcb_tcpoutwina[EDI], EAX
	MOVL	dcb_tcpoutwinm[EDI], EAX
	CALL	xosnetGiveBufr##	;Give up the received packet
	CALL	knlGetSysHRDtTm##	;Get current fractional days
	MOVL	ECX, dcb_ipstpdb[EDI]
	ADDL	EAX, tpdb_tcpseqoffset[ECX] ;Add in our offset
	MOVL	dcb_tcpoutsnum[EDI], EAX ;Set as our initial sequence number
	MOVL	dcb_tcprcvanum[EDI], EAX
	CALL	tcpcalhash#
	CALL	xostcpHashSet#		;Put DCB into the hash table
	CLC
	RET
.PAGE
	.SBTTL	xostcpSendSynAck - Subroutine to send SYN|ACK packet

;Subroutine to send SYN|ACK packet
;	c{EDI} = Offset of DCB
;	CALL	xostcpSendSynAck

xostcpSendSynAck::
	MOVL	ECX, #60t
	CALL	xosnetGetBufr##		;Get a buffer
	JC	4$			;If can't get one
	CALL	tcpsetpkt#		;Set up the packet header
	JC	6$
	MOVL	npb_dcb[EBX], EDI
	MOVZBL	ECX, npb_tpofs[EBX]
	MOVL	EAX, dcb_tcpoutsnum[EDI]
	MOVL	npb_seqnum[EBX], EAX
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	MOVL	tcp_seqnum[EBX+ECX], EAX
	MOVB	tcp_code[EBX+ECX], #TCPC$SYN|TCPC$ACK
	ADDB	tcp_offset[EBX+ECX], #0x20 ;We will store 8 option bytes
	MOVL	EAX, dcb_ipstpdb[EDI]	;Store the maximum segment size option
	MOVL	EAX, tpdb_tcpmaxseg[EAX]
	XCHGB	AL, AH
	SHLL	EAX, #16t
	MOVW	AX, #0x0402
	MOVL	[EDX], EAX
	MOVL	4[EDX], #0
	ADDL	npb_count[EBX], #8
	CALL	tcpsetack#		;Set up acknowlegement stuff in packet
	CALL	xosipsSendSetup##	;Set up final IP header stuff
	MOVL	EAX, #xosnetGiveBufr##	;Send the packet and then give up the
	CALL	xossnpSendPkt##		;  buffer (but don't wait)
	CLC
4$:	RET

6$:	CALL	xosnetGiveBufr##
	STC
	RET
.PAGE
	.SBTTL	xostcpClear - Subroutine to clear TCP connection

;Subroutine to clear TCP connection - must be called in extended fork
;  context
;	c{EDI} = Offset of DCB
;	c{ESI} = Offset of IDB
;	CALL	xostcpClear
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal

xostcpClear:
	MOVZBL	EAX, dcb_tcpstate[EDI]	;Get connection state
	JMPIL	CS:clrdsp[EAX*4]	;Dispatch on the state

	.MOD	4
clrdsp:	.LONG	clridle		;TCPS_CLOSED    = 0  - Idle
	.LONG	clrlisten	;TCPS_LISTEN    = 1  - Waiting for SYN
	.LONG	clrestab	;TCPS_SYNSENT   = 2  - SYN sent
	.LONG	clrestab	;TCPS_SYNACKD   = 3  - ACK sent for SYN|ACK
	.LONG	clrestab	;TCPS_SYNRCVD   = 4  - SYN received
	.LONG	clrestab	;TCPS_ESTAB     = 5  - Connection established
	.LONG	clridle		;TCPS_FINWAIT1  = 6  - FIN sent, waiting for FIN
				;			 in reply and for ACK
	.LONG	clridle		;TCPS_FINWAIT2  = 7  - ACK for FIN received,
				;			 still waiting for FIN
	.LONG	clridle		;TCPS_CLOSING   = 8  - Reply FIN received
				;			 without ACK, waiting
				;			 for ACK for the FIN
	.LONG	clrclosewait	;TCPS_CLOSEWAIT = 9  - FIN received, waiting
				;			 for process to close
				;			 TCP device
	.LONG	clridle		;TCPS_LASTACK   = 10 - Reply FIN sent, waiting
				;			 for final ACK
	.LONG	clridle		;TCPS_TIMEWAIT  = 11 - All finished - timing out
				;			 port

.PAGE
;Here when clearing if already idle or quiet

clridle:CALL	xostcpGiveAll#		;Make sure don't have any buffers
	CLRL	EAX
	RET

;Here when clearing if waiting for SYN

clrlisten:
	CMPL	dcb_tcpconbase[EDI], #0
	JNE	clridle
4$:	MOVL 	EAX, dcb_tcpconnext[EDI]
	TESTL	EAX, EAX
	JE	clridle

;Here if this is a SYN queue DCB with at least one DCB waiting for a
;  connection - we must wake up all DCB's that are waiting

	MOVL	EAX, dcb_tcpconnext[EDI] ;Remove him from the list
	MOVL	EDX, dcb_tcpconnext[EAX]
	MOVL	dcb_tcpconnext[EDI], EDX
	CLRL	EDX
	MOVL	dcb_tcpconnext[EAX], EDX
	MOVL	dcb_tcpconbase[EAX], EDX
	PUSHL	EDI
	MOVL	EDI, EAX
	MOVL	EAX, #ER_ABORT
	CALL	knlResumeOutput##	;Wake him up
	POPL	EDI
	JMP	4$

;Here when clearing if have already received a FIN - in this case we send the
;  reply FIN and then wait for the final ACK

clrclosewait:
	CALL	xostcpSendFin#		;Send a FIN
	JC	10$
	MOVB	dcb_tcpstate[EDI], #TCPS_LASTACK ;Update state
	CALL	tcprspwait#		;Wait for the final ACK
10$:	RET

;Here when clearing if a connection is established - we send a FIN and then
;  wait for the reply FIN

clrestab:				       ;Is the application level
	TESTB	dcb_tcpsts2[EDI], #TCPS2$FATAL ;  protocol alive?
	JE	14$			;Yes
	CALL	xostcpGiveAll#		;No - make sure no buffers allocated
14$:	CALL	xostcpSendFin#		;Send a FIN
	JC	18$
	MOVB	dcb_tcpstate[EDI], #TCPS_FINWAIT1 ;Update state
16$:	CALL	tcprspwait#		;Wait for a response
	JC	18$			;If error
	CMPB	dcb_tcpstate[EDI], #TCPS_CLOSING ;OK - did we get a FIN?
	JE	20$			;Yes
	CMPB	dcb_tcpstate[EDI], #TCPS_TIMEWAIT ;Maybe
	JNE	16$			;No - continue waiting
	JMP	20$			;Yes

;Here if error while sending a FIN or waiting for his FIN

18$:	MOVB	dcb_tcpstate[EDI], #TCPS_TIMEWAIT ;Indicate almost idle
	CMPL	EAX, #ER_NORSP		;Time-out error?
	JNE	20$			;No
	MOVL	EAX, #ER_NCLST		;Yes - get right error code
20$:	PUSHL	EAX			;Save error code
	CALL	xostcpGiveAll#		;Make sure don't have any buffers
	POPL	EAX
	BTL	EAX, #31t		;Set C if error
ret020:	RET
.PAGE
	.SBTTL	tcplost - Subroutine to indicate that connection is lost

;Subroutine to indicate that a TCP connection has been lost
;	c{EDI} = Offset of DCB
;	CALL	tcplost
;  c{EAX} is respected

tcplost::
	CMPL	dcb_ipsrmtaddr[EDI], #0	;Do we have a remote address now?
	JE	4$			;No
	PUSHL	EAX			;Yes
	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the lost connection
	INCL	tpdb_tcpcntclost[EAX]
	CALL	xostcpHashClr#		;Remove DCB from its hash list
	CLRL	EAX
	MOVL	dcb_ipsrmtaddr[EDI], EAX ;Clear remote IP address
	POPL	EAX
4$:	ORB	dcb_tcpsts2[EDI], #TCPS2$FATAL ;Indicate application level is
					       ;  dead
	CMPL	dcb_opencnt[EDI], #0	;Is the DCB in use?
	JE	6$			;No
	MOVL	EAX, #ER_NCLST		;Yes - wake him up if he's waiting for
	CALL	knlResumeInput##	;  anything
	MOVL	EAX, #ER_NCLST
	JMP	knlResumeOutput##

;Here if DCB is not in use - give it up

6$:	MOVL	EAX, dcb_sdisp[EDI]	;Give up the DCB
	JMPIL	sd_cleardcb[EAX]
.PAGE
	.SBTTL	sd_inpblock - Input block

;Here for the input block entry
;	c{ESI} = Offset of IORB
;	c{EDI} = Offset of DCB
;	CALL	tcpinpblk
;	c{EAX} = Error code
;	c{EBX} = Status bits
;	c{ECX} = Amount transfered

$$$=!0
FRM inp_amount , 4
inp_SIZE=!$$$

tcpinpblk:
	PUSHL	ESI
	CALL	sysIoQabLock##		;Lock the QAB
	TESTL	EAX, EAX
	JS	2$			;If error
	PUSHL	ESI
	CALL	sysIoReadLock##		;Lock all buffer pages
	TESTL	EAX, EAX
	JS	2$			;If error
	MOVB	iorb_queue[ESI], #dcb_inpframe
	MOVL	iorb_routine[ESI], #tcpinpblk1
	JMP	knlXfQueue##		;Switch to extended fork context

2$:	TOFORK
	RET

;Here in extended fork context

tcpinpblk1:
	ENTER	inp_SIZE, 0
	CMPB	dcb_tcpstate[EDI], #TCPS_ESTAB ;Do we have a connection?
	JE	3$			;Yes
	CMPB	dcb_tcpstate[EDI], #TCPS_CLOSEWAIT ;Maybe
	JNE	10$			;No - fail
3$:	MOVL	SS:xffTimeOut##, #TIMEOUT_AP ;Store default time-out value
	MOVL	SS:xffTimeOut##+4, #0
	MOVL	inp_amount[EBP], #0	;Clear total amount input
	CMPL	SS:xffParm##, #0	;Have any device parameters?
	JE	4$			;No
	MOVL	EBX, #tcpioparms	;Yes - process parameters
	MOVL	ECX, #knlComDPParms##
	LFSL	EDX, SS:xffParm##
	CALL	knlProcDevParm##
	JC	12$
4$:	MOVL	ESI, dcb_netidb[EDI]	;Get offset of our IDB
	MOVL	ECX, SS:xffCount##	;Get count
	JREGZ	ECX, 6$
	LESL	EBX, SS:xffBuffer1##	;Get address of buffer
	MOVL	EAX, SS:xffTimeOut##
	MOVL	EDX, SS:xffTimeOut##+4
	CALL	xostcpGetBlk#
	JC	8$
	BTL	SS:xffCmd##, #O%PARTIAL	;Can we accept partial input?
	JC	6$			;Yes
	ADDL	inp_amount[EBP], ECX	;No - add into total amount
	ADDL	SS:xffBuffer1##, ECX	;Bump buffer offset
	SUBL	SS:xffCount##, ECX	;Reduce amount wanted
	JG	4$			;If need more
	MOVL	ECX, inp_amount[EBP]	;Finished - get total amount
6$:	CLRL	EAX
	CALL	xostcpOpenWindow#	;Open sender's window if need to
8$:	LEAVE
	MOVL	EBX, #QSTS$DONE
	RET

;Here if don't have a connection

10$:	MOVL	EAX, #ER_NCLST
12$:	CLRL	ECX
	JMP	8$
.PAGE
	.SBTTL	sd_outstring - Output string

;Here for the output string entry
;	c{ESI} = Offset of IORB
;	c{EDI} = Offset of DCB
;	CALL	tcpoutstr
;	c{EAX} = Error code
;	c{EBX} = Status bits
;	c{ECX} = Amount transfered

tcpoutstr:
	PUSHL	ESI
	CALL	sysIoQabLock##		;Lock the QAB
	TESTL	EAX, EAX
	JS	2$			;If error
	PUSHL	ESI
	CALL	sysIoStringLock##	;Lock the string pages
	TESTL	EAX, EAX
	JNS	4$			;OK - go on
2$:	TOFORK				;If error
	RET

	.SBTTL	sd_outblock - Output block

;Here for the output block entry
;	c{ESI} = Offset of IORB
;	c{EDI} = Offset of DCB
;	CALL	tcpoutblk
;	c{EAX} = Error code
;	c{EBX} = Status bits
;	c{ECX} = Amount transfered

;TCP output is somewhat complex, with silly window syndrome avoidance and
;  packet coalescence (Nagel althgorithm) to worry about.  Since pushes are
;  optional, we may have some unsent data buffered when we get here, but it
;  will always be less than half the "maximum window size".  (This is a somewhat
;  problematical value since it is not directly provided by the protocol.  We
;  keep track of the largest window size reported and use this value.)  When
;  finished here, we always do a push (even if its not requested) if the amount
;  buffered is half the maximum window size or greater.  We do not store data
;  unless the window is open some amount, and we store data in chunks no
;  larger than half the maximum window size.  This means we may over-commit
;  up to half the maximum window size.  An output block is not split across
;  packets unless absolutely necessary.  If there is a packet buffered and
;  the entire output block can be added to the packet, this is done (even if
;  this means allocating additional chained buffer chunks).  If the output
;  block is too big to fit in a single packet by itself, as much as possible
;  is added to the current packet.  If it will fix in a maximum size packet,
;  the current buffered data is pushed and a new packet is allocated.

;Note that the output code uses a somewhat unusual scheme to avoid the silly
;  window syndrome in addition to this.  If the next packet to be output will
;  not completely fit in the window, nothing is output and a window probe
;  timer is started.  The first time it goes off, as much of the packet as
;  will fit in the window is output, completely closing the window.

$$$=!0
FRM out_amount , 4
out_SIZE=!$$$

tcpoutblk:
	PUSHL	ESI
	CALL	sysIoQabLock##		;Lock the QAB
	TESTL	EAX, EAX
	JS	2$			;If error
	PUSHL	ESI
	CALL	sysIoWriteLock##	;Lock all buffer pages
	TESTL	EAX, EAX
	JS	2$			;If error
4$:	MOVB	iorb_queue[ESI], #dcb_outframe
	MOVL	iorb_routine[ESI], #tcpoutblk1
	JMP	knlXfQueue##		;Switch to extended fork context

;Here in extended fork context

tcpoutblk1:
	ENTER	out_SIZE, 0
	CLRL	EAX
	MOVL	out_amount[EBP], EAX
	MOVL	SS:xffTimeOut##, #TIMEOUT_AP ;Store default time-out value
	MOVL	SS:xffTimeOut##+4, #0
	CMPL	SS:xffParm##, #0	;Have any device parameters?
	JE	6$			;No
	MOVL	EBX, #tcpioparms	;Yes - process parameters
	MOVL	ECX, #knlComDPParms##
	LFSL	EDX, SS:xffParm##
	CALL	knlProcDevParm##
	JC	16$
	MOVL	ESI, dcb_netidb[EDI]	;Get offset of our IDB
6$:	MOVL	ECX, SS:xffCount##	;Get count
	TESTL	ECX, ECX
	JE	14$			;If nothing to output
	MOVL	EAX, SS:xffTimeOut##	;Get time-out value
	MOVL	EDX, SS:xffTimeOut##+4
	LESL	EBX, SS:xffBuffer1##	;Get address of buffer
	PUSHL	ECX
	CALL	xostcpPutBlk#		;Store the data in the output buffer
	POPL	ECX
	JC	16$			;If error
	ADDL	SS:xffBuffer1##, ECX	;Bump pointer
	ADDL	out_amount[EBP], ECX	;Increase amount done
	SUBL	SS:xffCount##, ECX	;Reduce amount to do
	JG	6$			;Continue if more to do
14$:	CLRL	EAX			;Finished
16$:	MOVL	ECX, out_amount[EBP]	;Get amount output
	LEAVE
	MOVL	EBX, #QSTS$DONE
	RET
.PAGE
	DATA

tcpccb::       .LONG 'CCB*'	;TCP CCB
	       .BLKB ccb_SIZE-4
tcptwdcb::     .LONG 0		;Offset of first timed wait DCB
xostcpTpdbHead:.LONG 0		;Offset of first TCP TPDB
temp:	       .BLKB 12t	;Buffer for destination message names
lit10:	       .LONG 10t
tcpxxx:: .LONG 0

	END
