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

// This device class implements the Internet ARP protocol. It is used by IP
//   to optain local hardware addresses. It is intended to also be available
//   for other protocol families, although none others currently use it.  The
//   ARPn: device implemented by this class driver is only useful for getting
//   statistics about ARP protocol usage using device characteristics. It
//   cannot do IO. There are no higher level protocols supported by ARP.

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
#include <xosxarp.h>

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0

static INITLKE initarp;

static long XOSFNC arprecvpkt(ARPNPDB *npdb, NPB *npb);

// Define offsets in the data part of an ARP packet. These are variable
//   based on the address widths.

#define HWSRC (0)
#define PSRC  (arpdb->hwlen)
#define HWTAR (arpdb->hwlen + arpdb->plen)
#define PTAR  (2 * arpdb->hwlen + arpdb->plen)

#pragma data_seg(_HEAD);
	LKEHEAD(ARPCLS, initarp, MAJVER, MINVER, EDITNUM, CLASS); 
#pragma data_seg();

static long arpdcbnum;			// Number of in use network DCBs
static long arpdcbmax;			// Maximum in use network DCBs
static long arpdcblmt = 10000;	// Maximum number of network DCBs allowed

ARPNPDB *arpfirstnpdb;

// Network protocol level dispatch table for ARP

static NPDBDISP arpdisp =
{	NULL,				// Link up
	NULL,				// Link down
///	arpabort			// Abort output
///	rtgetaddr,			// getaddr function for protocol
};


//********************************************
// Device dispatch table for ARP class devices
//********************************************

static long XOSFNC arpgivedcb(void);
static long XOSFNC arpopen(char *spec);

static const QFDISP arpqfdisp =
{	NULL,				// Mount
	arpgivedcb,			// Clear DCB
	NULL,				// Open additional
	arpopen,			// Open device/file
	NULL,				// Device parameters
	NULL,				// Delete file
	NULL,				// Rename file
	NULL,				// Input block
	NULL,				// Output block
	NULL,				// Get input/output status
	NULL,				// Special device function
	sysIoCloseFin,		// Close file
	NULL,				// Get device label
	NULL,				// Commit data to disk
	NULL,				// Get device info
	NULL				// Verify changed disk
};


// Class characteristics tables for the ARP device class

SINFO(msgnumber , "Number of in use ARP devices");
SINFO(msgmaximum, "Maximum number of in use ARP devices");
SINFO(msglimit  , "Maximum number of ARP devices allowed");

static const CHARTBL arpcctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(NUMBER  , DECV, U, 4, &msgnumber , sysChrGetULong, NULL        ,
		&arpdcbnum),
  CTITEM(MAXIMUMX, DECV, U, 4, &msgmaximum, sysChrGetULong, sysChrSetULong,
		&arpdcbmax),
  CTLAST(LIMIT   , DECV, U, 4, &msglimit  , sysChrGetULong, sysChrSetULong,
		&arpdcblmt)
}};


SINFO(msgbadprot, "Discarded: Unknown target protocol");

// Class function dispatch table for ARP class devices

static long XOSFNC arpaddunit(void); 

static CFDISP arpcls = { 3,
{	arpaddunit,			// CF_ADDUNIT =  8 - Add unit
	NULL,				// CF_PUNITS  =  9 - Get information about
						//                     physical units
	NULL				// CF_AUNITS  = 10 - Get information about active
}};


static const CHARTBL arpdctbl = {CTBEGIN(sysChrValuesNew),
{ CTITEM(CLASS   , TEXT, U,  8, &knlChrMsgClass  , sysChrDevGetClass,
		sysChrDevChkClass, 0),

  CTITEM(NETDEV  , TEXT, U,  8, &xosnetMsgNetDev , xosnetGetNetDev,
		NULL             , 0),

  CTITEM(ETYPE   , HEXV, U,  4, &xosnetMsgEType  , xosnetGetNpdb4,
		NULL             , offsetof(ARPNPDB, nplb.etype)),

  CTITEM(PKTIN   , DECV, U,  4, &xosnetMsgPktIn  , xosnetGetNpdb4,
		xosnetSetNpdb4   , offsetof(ARPNPDB, cntpktin)),

  CTITEM(BYTEIN  , DECV, U,  4, &knlChrMsgByteIn , xosnetGetNpdb4,
		xosnetSetNpdb4   , offsetof(ARPNPDB, cntbytein)),

  CTITEM(PKTOUT  , DECV, U,  4, &xosnetMsgPktOut , xosnetGetNpdb4,
		xosnetSetNpdb4   , offsetof(ARPNPDB, cntpktout)),

  CTITEM(BYTEOUT , DECV, U,  4, &knlChrMsgByteOut, xosnetGetNpdb4,
		xosnetSetNpdb4   , offsetof(ARPNPDB, cntbyteout)),

  CTITEM(NODST   , DECV, U,  4, &msgbadprot      , xosnetGetNpdb4,
		xosnetSetNpdb4   , offsetof(ARPNPDB, cntnodst)),

///  CTITEM(BADADDR , DECV, U,  4, &msgipbadaddr    , xosnetGetNpdb4,
///		xosnetSetNpdb4   , offsetof(ARPNPDB, ipcntbadaddr)),

  CTITEM(BADHDR  , DECV, U,  4, &xosnetMsgBadHdr , xosnetGetNpdb4,
		xosnetSetNpdb4   , offsetof(ARPNPDB, cntbadhdr)),

  CTITEM(PSLTMN  , DECV, U,  4, &xosnetMsgPSLtMn , xosnetGetNpdb4,
		xosnetSetNpdb4   , offsetof(ARPNPDB, cntpsltmn)),

  CTLAST(PSLTDL  , DECV, U,  4, &xosnetMsgPSLtDL , xosnetGetNpdb4,
		xosnetSetNpdb4   , offsetof(ARPNPDB, cntpsltdl)),

///  CTLAST(DLLTHL  , DECV, U,  4, &xosnetMsgDLLtHL , xosnetGetNpdb4,
///		xosnetSetNpdb4   , offsetof(ARPNPDB, ipcntdllthl))
}};


//************
// The ARP CCB
//************

static DEVCHKFNC arpdevchk;

CCB arpccb =
{	'CCB*',
	0,					// next    - Address of next CCB
	"ARP",				// name    - Name of this class
	0xFFFFFF,			// npfxmsk - Name prefix part mask
	'ARP',				// npfxval - Name prefix part value
	arpdevchk,			// devchk  - Address of device check function
	&arpcls,			// fdsp    - Address of class func disp table
	&arpcctbl			// clschar - Address of class char table
};


static const CHARITEM arpaublk[] =
{	CTITEM(UNIT    , DECV, U, 4, NULL, NULL, xosnetAUUnit    , 0),
	CTITEM(IFDEV   , DECV, U, 4, NULL, NULL, xosnetAUNetDev  , 0),
	CTITEM(ETYPE   , DECV, U, 4, NULL, NULL, xosnetAUProtocol,
			(1540<<16)+0xFFFF),
	CTLAST(PRIORITY, DECV, U, 4, NULL, NULL, xosnetAUPriority, 0)
};


//***********************************************************
// Function: initarp - Initialization routine
// Returned: 0 if normal or a negative XOS error code if erro
//***********************************************************

#pragma code_seg ("x_ONCE");

static long XOSFNC initarp(
	char **pctop,
	char **pdtop)
{
	long rtn;

	sysChrFixUp((CHARTBL *)&arpcctbl);
	sysChrFixUp((CHARTBL *)&arpdctbl);
	sysChrFixUp((CHARTBL *)&arpaublk);
	if ((rtn = sysIoNewClass(&arpccb)) >= 0)
		*pctop = codetop;
	return (rtn);
}

#pragma code_seg ();


//************************************************************
// Function: arpaddunit - Implements the CL_ADDUNIT function
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

typedef struct
{	long  unit;
	long  priority;
	long  etype;
	long  ifunit;
} AUDATA;

static long XOSFNC arpaddunit(void)
{
	ARPNPDB  *npdb;
	NETAUDATA data;
	long      rtn;

	if (knlTda.buffer2 == NULL)
		return (ER_CHARM);
	data.dev.l[0] = -1;
	data.unit = -1;
	data.protocol = -1;
	data.priority = 0x7FFF;
	if ((rtn = sysChrValuesNew(TRUE, arpaublk, &data)) < 0)
		return (rtn);
	if ((data.unit | data.dev.l[0] | data.protocol) == -1)
		return (ER_CHARM);
	if ((rtn = xosnetMakeNetwork(data.dev.c,  "ARP", data.unit, data.protocol,
			offsetof(ARPPKT, data), sizeof(ARPNPDB), (RECVFNC *)arprecvpkt,
			&arpdisp, &arpccb, (NPDB **)&npdb)) < 0)
		return (rtn);
	xosnetLinkNplb(npdb->idb, &npdb->nplb, data.priority);
	npdb->nextp = (NPDB *)arpfirstnpdb;
	arpfirstnpdb = (ARPNPDB *)npdb;
	npdb->nexts = xosnetFirstNpdb;
	xosnetFirstNpdb = (NPDB *)npdb;
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
// Function: arpdevchk - Device check function for ARP class devices
// Returned: 0 if no match, 1 if found a match, 2 if need to restart
//           search, or a negative XOS error code if error.
//******************************************************************

static long XOSFNC arpdevchk(
	char  name[16],		// Device name (16 bytes, 0 filled)
	char *path,			// Path specification
	long  rtndcb)
{
	ARPNPDB *npdb;
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
		npdb = arpfirstnpdb;
		while (npdb != NULL)
		{
			if (unit == npdb->unit)
			{
				if ((rtn = xosnetMakeDcb(name, unit, sizeof(ARPDCB), &arpccb,
						(IDB *)npdb->idb, &arpdctbl, &arpqfdisp,
						npdb->arpdesc)) < 0)
					return (rtn);
				((ARPDCB *)knlTda.dcb)->npdb = (NPDB *)npdb;
				return (1);
			}
			npdb = (ARPNPDB *)npdb->nextp;
		}
		return (ER_NSDEV);
	}
	return (0);
}


//************************************************************
// Function: arpopen - Handle the open entry - Open device
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC arpopen(
	char *spec)
{
	long  rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&knlIoMinParams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		return (rtn);
	return (0);
}


//************************************************************
// Function: arpgivedcb - Handle the cdcb entry - Give up DCB
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

static long XOSFNC arpgivedcb(void)
{
	arpdcbnum--;
	sysIoGiveDymDcb(knlTda.dcb);
	knlTda.dcb = NULL;
	return (0);
}


//********************************************************************
// Function: arprecvpkt - Process received packet for the ARP protocol
// Returned: TRUE if packet accepted, FALSE if rejected
//********************************************************************

// NOTE: Accepted packets include invalid packets that are discarded. This
//       function accepts all packets and always returns TRUE.

long XOSFNC arprecvpkt(
	ARPNPDB *npdb,
	NPB     *npb)
{
	ARPPKT *arppkt;
	ARPDB  *arpdb;
	IDB    *idb;
	long    etype;
	int     ttllen;

	if (npb->label != NPB_LABEL)		// Is this really a network buffer?
		CRASH('NNPB');
	arppkt = (ARPPKT *)(npb->data + npb->npofs);
	idb = npdb->idb;
	npdb->cntpktin++;
	npdb->cntbytein += npb->count;
	if (npb->count < sizeof(ARPPKT))	// Is it big enough for the header?
	{
		npdb->cntpsltmn++;				// No - count it and discard it
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	ttllen = arppkt->hwlen + arppkt->plen; // Yes - size of each address pair
	if (npb->count < (2 * ttllen))		// Is it big enough for the data?
	{
		npdb->cntpsltmn++;				// No - count it and discard it
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	if (convnetword(arppkt->hwtype) != npdb->idb->hwtype ||
			arppkt->hwlen == 0 ||		// Make sure hardware type and address
			arppkt->hwlen > 8 ||		//   length and protocol address length
			arppkt->plen == 0 ||		//   and the operations code are valid
			arppkt->plen > 16 ||
			(arppkt->oper != CONVNETWORD(1) && arppkt->oper != CONVNETWORD(2)))
	{
		npdb->cntbadhdr++;
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	etype = convnetword(arppkt->prot);
	arpdb = npdb->firstarpdb;
	while (arpdb != NULL)
	{
		if (arpdb->etype == etype)
		{
			arpdb->recvfunc(arpdb->npdb, convnetword(arppkt->oper),
					*(llong *)(arppkt->data + HWSRC) & arpdb->hwamask,
					arppkt->data + PSRC, arppkt->data + PTAR);
			xosnetGiveNpb(npb);
			return (TRUE);
		}
		arpdb = arpdb->next;
	}
	npdb->cntnodst++;
	xosnetGiveNpb(npb);
	return (TRUE);
}


//************************************************************
// Function: xosarpChkClient - Check for valid ARP client
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// Caller must have filled in the prot item in the ARPDB before calling this.

// Only one client is allowed for a protocol (prot) value. The main purpose
//   of this function is to check that a particular protocol value can be
//   registered.

// NOTE: Currently the only ARP client implimented is IP but this is designed
//       to support multiple client with different IP protocol values.

ARPNPDB * XOSFNC __export xosarpChkClient(
	char arpname[16],		// ARP unit number
	long etype)				// Client network level protocol (etype)
{
	ARPNPDB *npdb;
	ARPDB   *apnt;

	npdb = arpfirstnpdb;
	while (npdb != NULL)
	{
		if (sysLibStrNICmp(arpname, npdb->name, 16) == 0)
		{
			// Here with the request ARP unit

			apnt = npdb->firstarpdb;
			while (apnt != NULL)
			{
				if (apnt->etype == etype)
					return (NULL);
				apnt = apnt->next;
			}
			return (npdb);				// Not registered - give good return
		}
		npdb = (ARPNPDB *)npdb->nextp;
	}
	return (NULL);
}


//************************************************************
// Function: xosarpRegClient - Register an ARP client
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// Caller must have filled in the npdb, prot, hwlen, plen, reqfunc, and
//   rspfunc items in the ARPDB before calling this. This functions fills
//   in arpnpdb before returning. Once this function returns the client
//   must not modify the ARPDB in any way.

// Caller must call xosarpChkClient first to obtain the address of the ARP
//   NPDB and to see if any protocol is already registered. This call will
//   fail if the requested ARP device does not exist or if a client is
//   already registered with it for the specified protocol.

// NOTE: Currently the only ARP client implimented is IP but this is designed
//       to support multiple client with different IP protocol values.

long XOSFNC __export xosarpRegClient(
	ARPNPDB *npdb,	 		// ARP unit number
	ARPDB   *arpdb)			// Address of client's ARPDB
{
	arpdb->arpnpdb = npdb;
	arpdb->next = npdb->firstarpdb; // Add it to our list
	npdb->firstarpdb = arpdb;
	return (0);					// That's all
}


//**********************************************************
// Function: xosarpSendPkt - Generate and send an ARP packet
// Returned:
//**********************************************************

long XOSFNC __export xosarpSendPkt(
	ARPDB *arpdb,
	long   oper,			// 1 = Request, 2 = Reply, bit 31 set to force
							//    broadcast if target hardware address given
	char  *hwaddr,			// Target hardware address (NULL for broadcast)
	char  *paddr)			// Target protocol address
{
	ARPNPDB *arpnpdb;
	NPB     *npb;
	NPB     *npbp;
	IDB     *idb;
	ARPPKT  *pkt;
	long     rtn;

	arpnpdb = arpdb->arpnpdb;
	idb = arpnpdb->idb;
	if ((rtn = xosnetMakeNpb(idb, arpnpdb, sizeof(ARPPKT) +
			2 * (arpdb->plen + arpdb->hwlen), &npbp)) < 0)
		return (rtn);
	npb = npbp;
	idb->ifdisp->setpkt(idb, npb, arpnpdb->nplb.etype);
	if ((oper & 0x80000000) || hwaddr == NULL)
		idb->ifdisp->setbcaddr(idb, npb);
	else
		idb->ifdisp->sethwaddr(idb, npb, hwaddr);
	pkt = (ARPPKT *)(npb->data + npb->npofs);
	pkt->hwtype = CONVNETWORD(1);
	pkt->prot = convnetword(arpdb->etype);
	pkt->hwlen = arpdb->hwlen;
	pkt->plen = arpdb->plen;
	pkt->oper = convnetword(oper);
	sysLibMemCpy(pkt->data + HWSRC, (char *)&idb->hwaddr, arpdb->hwlen);
	sysLibMemCpy(pkt->data + PSRC, arpdb->paddr, arpdb->plen);
	if (hwaddr == NULL)
		sysLibMemSet(pkt->data + HWTAR, 0xFF, arpdb->hwlen);
	else
		sysLibMemCpy(pkt->data + HWTAR, hwaddr, arpdb->hwlen);
	sysLibMemCpy(pkt->data + PTAR,  paddr, arpdb->hwlen);
	npb->count += (sizeof(ARPPKT) + 2 * (arpdb->hwlen + arpdb->plen));
	xosnetSendPkt(idb, npb);
	return (0);
}
