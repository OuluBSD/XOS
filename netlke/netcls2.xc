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
#include <xoslib.h>
#include <xosx.h>
#include <xosxchar.h>
#include <xosxparm.h>
#include <xosxlke.h>
#include <xosnet.h>
#include <xosxnet.h>

#define ourdcb ((NIFDCB *)knlTda.dcb)

// This file contains data and functions which provide services for various
//   network devices. This includes low level functions from some of the
//   higher level protocols.

// Common info strings used by network devices

XINFO(xosnetMsgNetDev  , "Network interface device");
XINFO(xosnetMsgBadSize , "Discarded: Bad size for packet");
XINFO(xosnetMsgTooBig  , "Discarded: Packet is too big");
XINFO(xosnetMsgFixedCnt, "Times input byte count fixed");
XINFO(xosnetMsgBcPktIn , "Broadcast packets input");
XINFO(xosnetMsgCrcErr  , "Discarded: Input CRC error");
XINFO(xosnetMsgDev     , "Network interface device");
XINFO(xosnetMsgLnkDown , "Discarded: Link down");
XINFO(xosnetMsgPhyAddr , "Physcial network address");
XINFO(xosnetMsgNoSBfr  , "Discarded: No system buffer");
XINFO(xosnetMsgOvrErr  , "Discarded: Input overrun error");
XINFO(xosnetMsgRunt    , "Discarded: Runt packet"); 
XINFO(xosnetMsgNoDst   , "Discarded: No destination");
XINFO(xosnetMsgPktIn   , "Packets input");
XINFO(xosnetMsgPktOut  , "Packets output");
XINFO(xosnetMsgXHung   , "Hung output errors");
XINFO(xosnetMsgOAbort  , "Output aborts");
XINFO(xosnetMsgBadPnt  , "Discarded: Bad ring pointer");
XINFO(xosnetMsgNoIBfr  , "Discarded: No interface buffer");
XINFO(xosnetMsgFrmErr  , "Discarded: Framing error");
XINFO(xosnetMsgXCSenErr, "Output carrier lost errors");
XINFO(xosnetMsgXUndErr , "Output underrun errors");
XINFO(xosnetMsgXHtBtErr, "Output heartbeat errors");
XINFO(xosnetMsgXOWCErr , "Output out of window collisions");
XINFO(xosnetMsgXColErr , "Output collisions");
XINFO(xosnetMsgXXColErr, "Excessive output collisions");
XINFO(xosnetMsgBoard   , "Board type");
XINFO(xosnetMsgLink    , "Link status");
XINFO(xosnetMsgMode    , "Mode (speed and duplex)");
XINFO(xosnetMsgTxFlow  , "Transmit flow control");
XINFO(xosnetMsgRxFlow  , "Receive flow control");
XINFO(xosnetMsgNetAdr  , "Physical network address");
XINFO(xosnetMsgBufrSz  , "Buffer size (KB)");

XINFO(xosnetMsgEType   , "Ethertype value");
XINFO(xosnetMsgBadHdr  , "Discarded: Bad packet header");
XINFO(xosnetMsgPSLtMn  , "Discarded: Packet < minimum");
XINFO(xosnetMsgPSLtDL  , "Discarded: Packet < data length");
XINFO(xosnetMsgPSLtHL  , "Discarded: Packet < header length");
XINFO(xosnetMsgDLLtHL  , "Discarded: Data < header length");


// Common device characteristics table used by network hardware drivers

const CHARITEM __export xosnetComChars[] =
{ CTITEM(PKTIN   , DECV, U, 4, &xosnetMsgPktIn   , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntpktin)),
  CTITEM(BYTEIN  , DECV, U, 4, &knlChrMsgByteIn  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntbytein)),
  CTITEM(PKTOUT  , DECV, U, 4, &xosnetMsgPktOut  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntpktout)),
  CTITEM(BYTEOUT , DECV, U, 4, &knlChrMsgByteOut , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntbyteout)),
  CTITEM(BCPKTIN , DECV, U, 4, &xosnetMsgBcPktIn , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntbcpktin)),
  CTITEM(BADSIZE , DECV, U, 4, &xosnetMsgBadSize , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntbadsize)),
  CTITEM(TOOBIG  , DECV, U, 4, &xosnetMsgTooBig  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cnttoobig)),
  CTITEM(BADPNT  , DECV, U, 4, &xosnetMsgBadPnt  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntbadpnt)),
  CTITEM(NOSBFR  , DECV, U, 4, &xosnetMsgNoSBfr  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntnosbfr)),
  CTITEM(NOIBFR  , DECV, U, 4, &xosnetMsgNoIBfr  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntnoibfr)),
  CTITEM(IFRAME  , DECV, U, 4, &xosnetMsgFrmErr  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntfa)),
  CTITEM(ICRC    , DECV, U, 4, &xosnetMsgCrcErr  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntcrc)),
  CTITEM(IRUNT   , DECV, U, 4, &xosnetMsgRunt    , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntrunt)),
  CTITEM(NODST   , DECV, U, 4, &xosnetMsgNoDst   , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntnodst)),
  CTITEM(IOVRRN  , DECV, U, 4, &xosnetMsgOvrErr  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntrover)),

  CTITEM(FIXEDCNT, DECV, U, 4, &xosnetMsgFixedCnt, xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntfixedcnt)),


  CTITEM(OCSEN   , DECV, U, 4, &xosnetMsgXCSenErr, xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntcsense)),
  CTITEM(OUNDRN  , DECV, U, 4, &xosnetMsgXUndErr , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntxunder)),
  CTITEM(OHUNG   , DECV, U, 4, &xosnetMsgXHung   , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntxhung)),
  CTITEM(OHTBT   , DECV, U, 4, &xosnetMsgXHtBtErr, xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cnthrtbt)),
  CTITEM(OOWC    , DECV, U, 4, &xosnetMsgXOWCErr , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntowcol)),
  CTITEM(OCOL    , DECV, U, 4, &xosnetMsgXColErr , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntnumcol)),
  CTITEM(OXCOL   , DECV, U, 4, &xosnetMsgXXColErr, xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntjabber)),
  CTLAST(ABORT   , DECV, U, 4, &xosnetMsgOAbort  , xosnetGetIdb4, xosnetSetIdb4,
		offsetof(IDB, cntabort))
};


//===================================================================
// Following functions are addunit characteristic functions used when
//   adding protocol devices
//===================================================================


//**********************************************************************
// Function: xosnetAUUnit - Process UNIT characteristic when adding unit
// Returned: 0 if OK or a negative XOS error if value is invalid
//**********************************************************************

long XOSFNC __export xosnetAUUnit(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;

	if ((rtn = sysUSGetULongV(val, &((NETAUDATA *)data)->unit, cnt)) < 0)
		return (rtn);
	return ((((ulong)((NETAUDATA *)data)->unit) > 99) ? ER_CHARV : 0);
}


//**************************************************************************
// Function: xosnetAUNetDev - Process NETDEV characteristic when adding unit
// Returned: 0 if OK or a negative XOS error if value is invalid
//**************************************************************************

long XOSFNC __export xosnetAUNetDev(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	if (cnt > 16)
		return (ER_CHARS);
	sysLibMemSetLong(((NETAUDATA *)data)->dev.l, 0, 4);
	return (sysLibMemCpy(((NETAUDATA *)data)->dev.c, val, cnt));
}


//**************************************************************
// Function: xosnetAUEProtocol - Process PROTOCOL characteristic
//                               when adding unit
// Returned: 0 if OK or a negative XOS error if value is invalid
//**************************************************************

long XOSFNC __export xosnetAUProtocol(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;

	if ((rtn = sysUSGetULongV(val, &((NETAUDATA *)data)->protocol, cnt)) < 0)
		return (rtn);
	return ((((NETAUDATA *)data)->protocol < (item->data >> 16) ||
			((NETAUDATA *)data)->protocol > (item->data & 0xFFFFF)) ?
			ER_CHARV : 0);
}


//**************************************************************
// Function: xosnetAUPriority - Process PRIORITY characteristic
//                              when adding unit
// Returned: 0 if OK or a negative XOS error if value is invalid
//**************************************************************

long XOSFNC __export xosnetAUPriority(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;

	if ((rtn = sysUSGetULongV(val, &((NETAUDATA *)data)->priority, cnt)) < 0)
		return (rtn);
	return ((((ulong)((NETAUDATA *)data)->priority) > 0xFFFF) ? ER_CHARV : 0);
}


//========================================================================
// Following functions are commonly needed device characteristic functions
//   for network devices and protocols
//========================================================================


//*********************************************************
// Function: xosnetGetIdb4 - Process characteristic which
//                           gets 4-byte value from the IDB
// Returned: Number of byte stored if normal or a negative
//           XOS error code if error
//*********************************************************

long XOSFNC __export xosnetGetIdb4(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULongV(val,
			*(long *)(((char *)((NETDCB *)knlTda.dcb)->nifidb) + item->data),
			cnt));
}


//*********************************************************
// Function: xosnetSetIdb4 - Process characteristic which
//                           sets 2-byte value from the IDB
// Returned: Number of byte stored if normal or a negative
//           XOS error code if error
//*********************************************************

long XOSFNC __export xosnetSetIdb4(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) >= 0)
		*(long *)(((char *)((NETDCB *)knlTda.dcb)->nifidb) + item->data) =
				value;
	return (rtn);
}


//*********************************************************
// Function: xosnetGetIdb4 - Process characteristic which
//                           gets 2-byte value from the IDB
// Returned: Number of byte stored if normal or a negative
//           XOS error code if error
//*********************************************************

long XOSFNC __export xosnetGetIdb2(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULongV(val,
			*(ushort *)(((char *)((NETDCB *)knlTda.dcb)->nifidb) + item->data),
			cnt));
}


//*********************************************************
// Function: xosnetGetIdb1 - Process characteristic which
//                           gets 1-byte value from the IDB
// Returned: Number of byte stored if normal or a negative
//           XOS error code if error
//*********************************************************

long XOSFNC __export xosnetGetIdb1(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULongV(val, *(char *)(((char *)((NETDCB *)knlTda.dcb)->
			nifidb) + item->data), cnt));
}


//*************************************************************
// Function: xosnetGetNpdb4 - Process characteristic which gets
//                            a 4-byte value from the NPDB
// Returned: Number of byte stored if normal or a negative
//           XOS error code if error
//*************************************************************

long XOSFNC __export xosnetGetNpdb4(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULongV(val,
			*(long *)(((char *)((NETDCB *)knlTda.dcb)->npdb) + item->data),
			cnt));
}


//*************************************************************************
// Function: xosnetGetNpdb4NBO - Process characteristic which gets a 4-byte
//                               network byte order value from the NPDB
// Returned: Number of byte stored if normal or a negative XOS error
//           code if error
//*************************************************************************

long XOSFNC __export xosnetGetNpdb4NBO(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULongV(val,
			convnetlong(*(long *)(((char *)((NETDCB *)knlTda.dcb)->npdb) +
			item->data)), cnt));
}


//*************************************************************
// Function: xosnetSetNpdb4 - Process characteristic which sets
//                            a 4-byte value from the NPDB
// Returned: Number of byte stored if normal or a negative XOS
//           error code if error
//*************************************************************

long XOSFNC __export xosnetSetNpdb4(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) >= 0)
		*(long *)(((char *)((NETDCB *)knlTda.dcb)->npdb) + item->data) =
				value;
	return (rtn);
}


//*************************************************************************
// Function: xosnetSetNpdb4NBO - Process characteristic which sets a 4-byte
//                               network byte order value from the NPDB
// Returned: Number of byte stored if normal or a negative XOS error
//           code if error
//*************************************************************************

long XOSFNC __export xosnetSetNpdb4NBO(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) >= 0)
		*(long *)(((char *)((NETDCB *)knlTda.dcb)->npdb) + item->data) =
				convnetlong(value);
	return (rtn);
}



//*************************************************************************
// Function: xosnetGetNetDev - Process characteristic which return the name
//                             of the network interface  device used
// Returned: Number of byte stored if normal or a negative XOS error
//           code if error
//*************************************************************************

long XOSFNC __export xosnetGetNetDev(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysLibStrMov(val, ourdcb->nifidb->name));
}






//=========================================================================
// Following functions provide various services for various network devices
//=========================================================================


//***********************************************************************
// Function: xosnetMakeIdb - Creates a network interface data block (IDB)
// Returned: 0 if normal or a negative XOS error code if error
//***********************************************************************

// Must be called with the exec memory resource.

// IDBs are allocated in static exec memory. Once created, they cannot be
//   removed.

long XOSFNC __export xosnetMakeIdb(
	long           unit,	// Unit number
	long           size,	// Size of the IDB
	CHARTBL const *ctbl,	// Address of device characteristics table
	IDB          **pidb)	// Address of pointer to receive address of IDB
{
	IDB *idb;
	long rtn;

	if ((rtn = sysMemGetXMem(size, (char **)&idb)) < 0)
		return (rtn);

	idb->label = 'IDB*';
	idb->devchar = ctbl;
	idb->unit = unit;

///	idb->ssfdisp = &netsffork;

	idb->ssidb.next = (NPB *)-1;

	idb->fnext = (DCB *)-1;

	*(long *)idb->name = 'NIF';
	sysLibDec2Str(idb->name + 3, unit);
	idb->next = firstidb;
	firstidb = idb;
	*pidb = idb;
	return (0);
}


//************************************************************
// Function: xosnetMakeDcb - Create a network DCB
// Returned: 0 if normal or a negative xOS error code if error
//************************************************************

// On a normal return, the address of the DCB created is stored in
//   tda.dcb.

long XOSFNC __export xosnetMakeDcb(
	char   *name,			// Device name (16 characters)
	long    unit,			// Unit number
	long    size,			// Size for DCB
	CCB    *ccb,			// Address of the CCB
	IDB    *idb,			// Address of the IDB
	CHARTBL const *devchar,	// Address of device characteristics table
	QFDISP const  *qfdisp,	// Address of QFNC dispatch table
	long    desc)			// Description bits
{
	long rtn;

	if ((rtn = sysIoGetDymDcb(name, desc, size, &knlTda.dcb)) < 0)
		return (rtn);
	knlTda.dcb->punit = unit;
	sysIoLinkDcb(knlTda.dcb, ccb);
	*(llong *)ourdcb->typename = *(llong *)idb->typname; // Copy type name
	ourdcb->devchar = devchar;			// Store address of the device
										//   characteristics table
	ourdcb->qfdisp = qfdisp;			// Staore address of the dispatch table
	ourdcb->nifidb = idb;				// Store IDB address in the DCB
	return (0);
}


//*****************************************************************
// Function: xosnetAbort - Terminate all IO for a network interface
// Returned: Nothing
//*****************************************************************

void XOSFNC __export xosnetAbort(
	IDB *idb)
{
	NPB *npb;
///	NPB *next;

	npb = idb->outhead;
	idb->outhead = NULL;
	idb->outtail = NULL;

#if 0

	while (idb != NULL)
	{
		next = npb->sendnext;
		xosnetFinXmit(ER_ABORT, idb, npb);

		//// SHOULD THIS CALL A DEVICE LEVEL ABORT ROUTINE????

		npb = next;
	}
	sdb = idb->sdbhead;
	while (sdb != NULL)
	{
		sdb->disp->abort(sdb);
		sdb = sdb->nexti;
	}
#endif
}


//******************************************************************
// Function: xosnetWaitForPktOut - Wait until packet has been output
// Returned: 0 if normal or a negative XOS error code if error
//******************************************************************

// Upon return output is either complete or has been canceled, in either case,
//   the packet may be used freely, including being deallocated immediately -

// NOTE: An error return from this routine does NOT guarentee that the packet
//       was not output, but it probably was not output in this case.

long xosnetWaitForPktOut(
	IDB  *idb,
	NPB  *npb,
	llong timeout)
{
	long rtn;

	while (npb->sendnext != (NPB *)-1)	// Is output finished?
	{
///		npb->outdone = xosnetOutputIsDone;	// No - change done dispatch
		if ((rtn = xosnetOutputWait(timeout)) < 0) // Wait until output is done
			break;
	}
	if (rtn >= 0)						// Error?
		return (0);						// No - finished

	// Here if error while waiting

	if (npb->sendnext == (NPB *)-1)		// Is the packet idle now?
		return (rtn);					// Yes - just return the error

/// IS THIS RIGHT???

	if (npb->sts & NPS_XMITING)			// No - is it being output now?
	{
		while (npb->sendnext != (NPB *)-1) // Yes - must wait until interface
			xosnetOutputWait(-1);		   //   get done with it (which should
		return (rtn);					   //   happen soon since all output
	}									   //   has a hardware level time-out)

	// Here if have error and the packet has not been output yet - remove it
	//   from the output queue, the send self list, or the routing table hold
	//   list.

#if 0

4$:	PUSHL	EAX
	PUSHL	EDX
	TESTB	npb_sts[EBX], #NPS$SELF ;No - did we send it to ourselves?
	JE	14$			;No
	LEAL	EDX, idb_sfhead[ESI]	;Yes - find predecessor in send-self
					;  list
6$:	CMPL	[EDX], EBX		;This one?
	JE	8$			;Yes
	MOVL	EDX, [EDX]		;No - advance to next
	TESTL	EDX, EDX
	JE	12$
	ADDL	EDX, #npb_sendnext
	JMP	6$

;Here with predecessor in send self list

8$:	MOVL	EAX, npb_sendnext[EBX]	;Remove this packet from the list
	MOVL	[EDX], EAX
	TESTL	EAX, EAX		;Last packet in list?
	JNE	22$			;No - finished
	CMPL	idb_sfhead[ESI], #0	;Yes - was it the only one?
	JE	10$			;Yes
	LEAL	EAX, -npb_sendnext[EDX]	;No - point to start of new tail packet
10$:	MOVL	idb_sftail[ESI], EAX	;Update tail pointer for list
	JMP	22$

12$:	CRASH	BPSL			;[Bad Packet Send-self List]

;Here if not sending the packet to ourselves

14$:	CMPL	npb_holdrt[EBX], #0	;Is this packet being held by a routing
					;  table entry?
	JE	15$			;No
	CALL	unholdpkt		;Yes - remove it
	JMP	22$			;Continue

;Here if packet should be in the output list

15$:	LEAL	EDX, idb_outhead[ESI]	;Find predecessor in output list
16$:	CMPL	[EDX], EBX
	JE	18$
	MOVL	EDX, [EDX]
	TESTL	EDX, EDX
	JE	26$			;Not in list - serious error
	ADDL	EDX, #npb_sendnext
	JMP	16$

;Here with predecessor in packet output list

18$:	MOVL	EAX, npb_sendnext[EBX]	;Remove this packet from the list
	MOVL	[EDX], EAX
	TESTL	EAX, EAX		;Last packet in list?
	JNE	22$			;No - finished
	CMPL	idb_outhead[ESI], #0	;Yes - was it the only one?
	JE	20$			;Yes
	LEAL	EAX, -npb_sendnext[EDX]	;No - point to start of new tail packet
20$:	MOVL	idb_outtail[ESI], EAX	;Update tail pointer for list
22$:	MOVL	npb_sendnext[EBX], #-1
	POPL	EDX
	POPL	EAX			;Restore error code
24$:	STC				;Indicate error
	RET

26$:	CRASH	BPOL			;[Bad Packet Output List]

#endif

	return (rtn);
}

#if 0

	.SBTTL	unholdpkt - Subroutine to remove packet from routing hold list

;Subroutine to remove packet from a routing table hold list

unholdpkt:
	PUSHL	ECX
	MOVL	ECX, npb_holdrt[EBX]	;Get offset of routing table entry
	LEAL	EDX, rt_holdhead[ECX]	;Find predecessor in hold list
2$:	CMPL	[EDX], EBX
	JE	4$.S
	MOVL	EDX, [EDX]
	TESTL	EDX, EDX
	JE	10$.S
	ADDL	EDX, #npb_sendnext
	JMP	2$.S

;Here with predecessor in packet hold list

4$:	MOVL	EAX, npb_sendnext[EBX]	;Remove this packet from the list
	MOVL	[EDX], EAX
	TESTL	EAX, EAX		;Last packet in list?
	JNE	8$.S			;No - finished
	CMPL	rt_holdhead[ECX], #0	;Yes - was it the only one?
	JE	6$.S			;Yes
	LEAL	EAX, npb_sendnext[EDX]	;No - point to start of new tail packet
6$:	MOVL	rt_holdtail[ECX], EAX	;Update tail pointer for list
8$:	POPL	ECX
	RET

10$:	CRASH	BPHL			;[Bad Packet Hold List]

#endif

//************************************************************
// Function: xosnetSendPkt - Send packet without waiting
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// Caller may later call xosnetWaitForPktOut to wait until output is
//   finished - Must be called at fork level.

// NOTE: This is the lowest level externally available sendpkt routine.  It
//       directly calls the hardware driver to send the packet.  No packet
//       processing is done here. The main function performed by this routine
//       is to queue packets for transmission when the interface is busy.

long XOSFNC __export xosnetSendPkt(
	IDB *idb,
	NPB *npb)
{
	if (idb->label != 'IDB*')			// Make sure really have an IDB here
		CRASH('NIDB');
	if (npb->sendnext != (NPB *)-1)		// Is this packet on an output list now?
		CRASH('BPOL');					// [Bad Packet Output List]
	if (npb->sts & NPS_XMITING)			// Now outputing this packet?
		CRASH('SPK2');					// [Sending PacKet 2 times]
	if (idb->state < LNKS_CNCTD)		// No - connected or established?
		return (ER_NLKNA);				// No - fail
	if ((npb->sts & NPS_SELF) == 0)		// Yes - is it for us?
	{
		npb->sendnext = NULL;
		if ((idb->sts & (IS_RESET|IS_REXMIT)) == 0 // No - can we send it now?
				&& idb->xmtavail > 0)
		{
			idb->xmtavail--;			// Yes

			npb->holdrt = NULL;			// Also make sure hold list back
										//   pointer is clear
			return (idb->ifdisp->sendpkt(idb, npb, 999));
		}
		else
		{
			// Here if packet output is busy - queue this packet for output
			//   later

			if (idb->outtail != NULL)
				idb->outtail->sendnext = npb;
			else
				idb->outhead = npb;
			idb->outtail = npb;
		}
	}
	else
	{
		// Here if sending to self

		CRASH('SELF');
	}
	return (0);
}


//****************************************************
// Function: xosnetFinXmit - Do output done processing
// Returned: Nothing
//****************************************************

// This is called at device fork level by an interface driver when a
//   packet has been completely output.

void XOSFNC __export xosnetFinXmit(
	long code,
	IDB *idb,
	NPB *npb)
{
	idb->outtimer = 0;					// Stop the output timer
	npb->sts &= ~NPS_XMITING;
	idb->cntpktout++;					// Count the packet
	idb->cntbyteout += npb->count;		// Add in to byte count
	npb->sendnext = (NPB *)-1;			// Indicate not being output
	idb->xmtavail++;					// Increase ouput availablity
	if (npb->outdone != NULL)			// Have output done function?
		npb->outdone(code, idb, npb);	// Yes - do it (this may queue
	else								//   another request)
		xosnetGiveNpb(npb);				// No - give give up the packet buffer
}


//************************************************************
// Function: xosnetOutputWait - Wait until output is done
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

long XOSFNC __export xosnetOutputWait(
	llong timeout)
{
	long rtn;

	if (knlTda.frame == offsetof(DCB, outthrd))
	{
		ourdcb->nifsts1 |= NIFS1_OFWOUT;
		rtn = sysIoWait(timeout, THDS_DW2);
		ourdcb->nifsts1 &= ~NIFS1_OFWOUT;
		if (rtn < 0)
			return (rtn);
	}
	else
	{
		ourdcb->nifsts1 |= NIFS1_IFWOUT;
		rtn = sysIoWait(timeout, THDS_DW2);
		ourdcb->nifsts1 &= ~NIFS1_IFWOUT;
		if (rtn < 0)
			return (rtn);
	}
	return (0);
}


//**************************************************************
// Function: xosnetWaitForWin - Wait until output window is open
// Returned: 0 if normal or a negative XOS error code if error
//**************************************************************

long XOSFNC __export xosnetWaitForWin(
	llong timeout)
{
	long rtn;

	if (knlTda.frame == offsetof(DCB, outthrd))
	{
		ourdcb->nifsts1 |= NIFS1_OFWSND;
		rtn = sysIoWait(timeout, THDS_DW2);
		ourdcb->nifsts1 &= ~NIFS1_OFWSND;
		if (rtn < 0)
			return (rtn);
	}
	else
	{
		ourdcb->nifsts1 |= NIFS1_IFWSND;
		rtn = sysIoWait(timeout, THDS_DW2);
		ourdcb->nifsts1 &= ~NIFS1_IFWSND;
		if (rtn < 0)
			return (rtn);
	}
	return (0);
}


//************************************************************
// Function: xosnetWaitForRsp - Wait for a response
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

long XOSFNC __export xosnetWaitForRsp(
	llong timeout)
{
	long rtn;

	if (knlTda.frame == offsetof(DCB, outthrd))
	{
		ourdcb->nifsts1 |= NIFS1_OFWRSP;
		rtn = sysIoWait(timeout, THDS_DW2);
		ourdcb->nifsts1 &= ~NIFS1_OFWRSP;
		if (rtn < 0)
			return (rtn);
	}
	else
	{
		ourdcb->nifsts1 |= NIFS1_IFWRSP;
		rtn = sysIoWait(timeout, THDS_DW2);
		ourdcb->nifsts1 &= ~NIFS1_IFWRSP;
		if (rtn < 0)
			return (rtn);
	}
	return (0);
}


//************************************************************
// Function: xosnetOutDone - Called when output is complete
// Returned: Nothing
//************************************************************

void XOSFNC __export xosnetOutDone(
	NIFDCB *dcb,
	long    code)
{
	if (dcb->nifsts1 & NIFS1_OFWOUT)
	{
		dcb->nifsts1 &= ~NIFS1_OFWOUT;
		sysIoResumeOutput((DCB *)dcb, code);
	}
	if (dcb->nifsts1 & NIFS1_IFWOUT)
	{
		dcb->nifsts1 &= ~NIFS1_IFWOUT;
		sysIoResumeOutput((DCB *)dcb, code);
	}
}


//*************************************************************
// Function: xosnetSendDone - Called when output window is open
// Returned: Nothing
//*************************************************************

void XOSFNC __export xosnetSendDone(
	NIFDCB *dcb,
	long   code)
{
	if (dcb->nifsts1 & NIFS1_OFWSND)
	{
		dcb->nifsts1 &= ~NIFS1_OFWSND;
		sysIoResumeOutput((DCB *)dcb, code);
	}
	if (dcb->nifsts1 & NIFS1_IFWSND)
	{
		dcb->nifsts1 &= ~NIFS1_IFWSND;
		sysIoResumeOutput((DCB *)dcb, code);
	}
}


//*************************************************************
// Function: xosnetRspDone - Called when a response is received
// Returned: Nothing
//*************************************************************

void XOSFNC __export xosnetRspDone(
	NIFDCB *dcb,
	long   code)
{
	if (dcb->nifsts1 & NIFS1_OFWRSP)
	{
		dcb->nifsts1 &= ~NIFS1_OFWRSP;
		sysIoResumeOutput((DCB *)dcb, code);
	}
	if (dcb->nifsts1 & NIFS1_IFWRSP)
	{
		dcb->nifsts1 &= ~NIFS1_IFWRSP;
		sysIoResumeOutput((DCB *)dcb, code);
	}
}


//************************************************************
// Function; xosnetMakeNpb - Make a network packet buffer
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

long XOSFNC __export xosnetMakeNpb(
	IDB  *idb,			// Address of the IDB
	void *blk,			// Address of DCB, NPDB, or TPDB
	long  size,			// Data size needed
	NPB **pnpb)			// Address of pointer to receive address of buffer
{
	NPB *npb;
	NPB *npbp;
	long rtn;

	if ((rtn = sysMemGetXmb(sizeof(NPB) + idb->hwhdrsz + size,
			(char **)&npbp)) < 0)
		return (rtn);
	npb = npbp;
	npb->label = NPB_LABEL;
	npb->sts = 0;
	npb->sendnext = (NPB *)-1;
	npb->next = NULL;
	npb->outdone = NULL;
	npb->holdrt = NULL;
	npb->dcb = blk;
	*pnpb = npb;
	return (0);
}


//******************************************************************
// Function: xosnetGiveList - Give up list of network packet buffers
// Returned: Nothing
//******************************************************************

void XOSFNC __export xosnetGiveList(
	NPB *npb)
{
	NPB *next;

	while (npb != NULL)
	{
		next = npb->next;
		xosnetGiveNpb(npb);
		npb = next;
	}
}


//********************************************************
// Function: xosnetGiveNpb - Give up network packet buffer
// Returned: Nothing
//********************************************************

void XOSFNC __export xosnetGiveNpb(
	NPB *npb)
{
	REQUIREFORK;
	if (npb->label != NPB_LABEL)	 	// Is this really a network buffer?
		CRASH('NNPB');
	if (npb->sendnext == (NPB *)-1)		// Is this buffer being output?
		sysMemGiveXmb(npb);				// No - just give it up directly
	else
		INT3;
///		npb->outdone = xosnetGiveBuffer; // ??????
}


//****************************************************************
// Function: xosnetNullOutDone - Output done function that does
//                               nothing except give up the buffer
// Returned: Nothing
//****************************************************************

void XOSFNC __export xosnetNullOutDone(
	long code,
	IDB *idb,
	NPB *npb)
{
	xosnetGiveNpb(npb);
}


//***********************************************************************
// Function: xosnetCheckNetAddr - See if a network address was specified
// Returned: > 0 if a network address was found (value is the position
//           in the string of the character following the second colon),
//           0 if no network address was found, or a negative XOS error
//           code if error
//***********************************************************************

// A network address is of the form:
//		stringofchracters::
// Generally, the string will be of the form n.n.n.n (dotted decimal notation),
//   n-n-n-n ("dotted" hexidecimal notation), or name.name.name... (domain name),
//   although this routine does not care about this. It only checks for the ::.

// It is assumed that the str argument points to an exec address, there is no
//   protection from address errors.

long XOSFNC __export xosnetCheckNetAddr(
	char *str)
{
	char *pnt;
	char  chr;

	pnt = str;
	while ((chr = *pnt++) != 0)
	{
		if (chr == ':' && pnt[0] == ':')
		{
			if (pnt[1] == ':')
				return (ER_BDSPC);
			return (pnt - str + 1);			
		}
	}
	return (0);
}


//************************************************************
// Function: xosnetRecvPkt - Called when packet has been input
//                          by the interface driver
// Returned: Nothing
//************************************************************

// The list of NPLBs is ordered by priority. We scan the list and call each
//   function whose etype matches the packets or is default. If the function
//   accepts the packet (returns TRUE) we are finished. If not we continue
//   down the list.

void XOSFNC __export xosnetRecvPkt(
	IDB *idb,
	long etype,
	NPB *npb)
{
	NPLB *nplb;

	nplb = idb->firstnplb;
	while (nplb != NULL)
	{
		if (nplb->etype == etype || nplb->etype == -1)
		{
			if ((nplb->recvfnc)(nplb->blk, npb))
				return;
		}
		nplb = nplb->next;
	}

	// If get there, no one is available to handle the packet or every one
	//   available to handle it has rejected it.

	idb->cntnodst++;				// Count the unwanted packet
	xosnetGiveNpb(npb);				// Discard the packet
}


//******************************************************************
// Function: xosnetMakeNetwork - Create a new network level protocol
// Returned: 0 if normal or a negative XOS error code if error
//******************************************************************

// This function does most of the work for creating a network level protocol
//   device. It does NOT link it into any lists so the caller can set up
//   the protocol dependent part before calling xosnetLinkNetwork to do this.

// Returns with the exec memory resource on a normal return. It will normally
//   be given up by calling xosnetLinkNetwork.

long XOSFNC __export xosnetMakeNetwork(
	char      ifname[16],	// Device name for network interface device
							//   (no colon, zero filled)
	char     *bname,		// Base name
	long      unit,			// Unit number
	long      etype,
	long      hdrsz,		// Packet header size
	long      npdbsz,		// Size of the NPDB to create
	RECVFNC  *recvfnc,		// Address of the network level receive function
	NPDBDISP *ndisp,		// Address of the network level dispatch table
	CCB      *ccb,			// Address of the CCB
	NPDB    **pnpdb)		// Address of pointer to receive address of new TPDB
{
	IDB   *idb;
	NPDB  *npdb;
	long   rtn;
	char   name[20];

	// First find the corresponding IDB

	idb = firstidb;
	while (idb != NULL)
	{
		if (sysLibStrNICmp(ifname, idb->name, 16) == 0)
			break;
		idb = idb->next;
	}
	if (idb == NULL)
		return (ER_PDVNF);
	rtn = sysLibStrMov(name, bname);	// Construct the full name
	rtn += sysLibDec2Str(name + rtn, unit);
	sysLibMemSet(name + rtn, 0, 16 - rtn);

	// See if this network level protocol device already exists

	npdb = idb->fnpdb;
	while (npdb != NULL)
	{
		if (sysLibStrCmp(npdb->name, name) == 0)
			return (ER_DUADF);
	}

	// Unit does not now exist - Create a NPDB for it

	sysMemGetXRes();
	if ((rtn = sysMemGetXMem(npdbsz, (char **)pnpdb)) < 0)
	{
		sysMemGiveXRes();
		return (rtn);
	}
	npdb = *pnpdb;						// OW thinks pnpdb is aliased because
										//   we passed its address!
	npdb->label = 'NPDB';				// Store label
	npdb->maxpktsz = idb->hwpktsz - hdrsz; // Store maximum packet size
	npdb->hdrsz = hdrsz;				// Store header size
	npdb->idb = idb;					// Store offset of IDB in the PDB
	npdb->ccb = ccb;
	npdb->disp = ndisp;
	npdb->unit = unit;					// Store unit number
	sysLibStrMov(npdb->name, name);		// Put name into the PDB
	npdb->protolen = 4;					// Store length of our protocol address
	npdb->nplb.label = 'NPLB';			// Set up the imbedded NPLB
	npdb->nplb.etype = etype;
	npdb->nplb.blk = npdb;
	npdb->nplb.recvfnc = recvfnc;
	*pnpdb = npdb;
	return (0);
}


//******************************************************************
// Function: xosnetLinkNplb - Puts a new NPLB into its protocol list
// Returned: Nothing
//******************************************************************

// Must be called with the exec memory resource.

void XOSFNC __export xosnetLinkNplb(
	IDB  *idb,
	NPLB *nplb,
	long  priority)
{
	NPLB  *next;
	NPLB **pntr;

///	idb = npdb->idb;
///	npdb->next = idb->fnpdb;			// Put it in the interface list
///	idb->fnpdb = npdb;

	nplb->priority = priority;
	pntr = &idb->firstnplb;
	while ((next = *pntr) != NULL)
	{
		if (nplb->priority > next->priority)
			break;
		pntr = &next->next;
	}
	if ((nplb->next = next) != NULL)
		next->prev = &nplb->next;
	nplb->prev = pntr;
	*pntr = nplb;
}


//*****************************************************************
// Function: xosnetFindInterface - Find IDB for a network interface
// Returned: Address of the IDB or NULL if not found
//*****************************************************************

IDB XOSFNC __export *xosnetFindInterface(
	long unit)
{
	IDB *idb;

	idb = firstidb;
	while (idb != NULL)
	{
		if (unit == idb->unit)
			return (idb);
		idb = idb->next;
	}
	return (NULL);
}


//********************************************************************
// Function: xosnetMakeTransport - Create new transport level protocol
// Returned: 0 if normal or a negative XOS error code if error
//********************************************************************

// This function does most of the work to create a transport level protocol.

long XOSFNC __export xosnetMakeTransport(
	char      uname[16],	// Device name for underlying network protocol
							//   (no colon, zero filled)
	char     *bname,		// Base device name for this transport protocol
	long      unit,			// Unit number
	long      protocol,		// Protocol
	long      hdrsz,		// Packet header size
	long      tpdbsz,		// Size of the TPDB to create
	RECVFNC  *recvfnc,		// Address of the transport level receive function
	TPDBDISP *tdisp,		// Address of the transport level dispatch table
	CCB      *ccb,			// Address of CCB
	TPDB    **ptpdb)		// Address of pointer ot receive address of new TPDB
{
	NPDB *npdb;
	TPDB *tpdb;
	TPDB *xtpdb;
	long  rtn;
	char  name[16];

	sysLibMemSetLong((long *)name, 0, 4);
	sysLibStrNMov(name, bname, 16);
	if (sysIoFindDevByName(name, NULL, FALSE) != ER_NSDEV) // Does this device
														   //   exist?
		return (ER_DUADF);				// Yes - fail

	// Find the corresponding network level protocol (NPDB)

	npdb = xosnetFirstNpdb;
	while (npdb != NULL)
	{
		if (sysLibStrNICmp(npdb->name, uname, 16) == 0)
		{
			sysMemGetXRes();
			if ((rtn = sysMemGetXMem(tpdbsz, (char **)&xtpdb)) < 0)
			{							// Get memory for the TPDB
				sysMemGiveXRes();
				return (rtn);
			}
			tpdb = xtpdb;				// OW thinks ppdb is aliased because
										//   we passed it address!
			tpdb->label = 'TPDB';
			tpdb->maxpktsz = npdb->maxpktsz - hdrsz;
										// Store maximum packet size
			tpdb->hdrsz = hdrsz;		// Store header size
			tpdb->ccb = ccb;
			tpdb->disp = tdisp;
			tpdb->unit = unit;			// Store unit number
			sysLibStrMov(tpdb->name, name); // Put name into the PDB
			tpdb->npdb = (NPDB *)npdb;
			tpdb->idb = npdb->idb;
			tpdb->tplb.label = 'TPLB';	// Set up the imbedded TPLB
			tpdb->tplb.protocol = protocol;
			tpdb->tplb.blk = tpdb;
			tpdb->tplb.recvfnc = recvfnc;
			*ptpdb = tpdb;
			return (0);
		}
		npdb = npdb->nexts;
	}
	return (ER_PDVNF);
}


//******************************************************************
// Function: xosnetLinkTplb - Puts a new TPLB into its protocol list
// Returned: Nothing
//******************************************************************

// Must be called with the exec memory resource.

void XOSFNC __export xosnetLinkTplb(
	NPDB *npdb,
	TPLB *tplb,
	long  priority)
{
	TPLB  *next;
	TPLB **pntr;

///	idb = npdb->idb;
///	npdb->next = idb->fnpdb;			// Put it in the interface list
///	idb->fnpdb = npdb;

	pntr = &npdb->firsttplb;
	while ((next = *pntr) != NULL)
	{
		if (tplb->priority > next->priority)
			break;
		pntr = &next->next;
	}
	if ((tplb->next = next) != NULL)
		next->prev = &tplb->next;
	tplb->prev = pntr;
	*pntr = tplb;
}

