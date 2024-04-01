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
#include <xosxip.h>


#define ourdcb ((IPDDCB *)knlTda.dcb)


//=====================================================================
// This file contains routines that provide general support for network
//   level access and that provide support for common transport level
//   operations.
//=====================================================================


//====================================================================
// Following functions are common device characteristics functions for
//   IPCLS devices and protocols
//====================================================================


//************************************************************
// Function: xosipGetTpdb4 - Process characteristic which gets
//                            a 4-byte value from the TPDB
// Returned: Number of byte stored if normal or a negative
//           XOS error code if error
//************************************************************

long XOSFNC __export xosipGetTpdb4(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULongV(val, *(long *)(((char *)ourdcb->tpdb) +
			item->data), cnt));
}


//************************************************************************
// Function: xosipGetTpdb4NBO - Process characteristic which gets a 4-byte
//                               network byte order value from the TPDB
// Returned: Number of byte stored if normal or a negative XOS error
//           code if error
//************************************************************************

long XOSFNC __export xosipGetTpdb4NBO(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULongV(val,
			convnetlong(*(long *)(((char *)ourdcb->tpdb) + item->data)),
			cnt));
}


//************************************************************
// Function: xosipSetTpdb4 - Process characteristic which sets
//                            a 4-byte value from the TPDB
// Returned: Number of byte stored if normal or a negative XOS
//           error code if error
//************************************************************

long XOSFNC __export xosipSetTpdb4(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) >= 0)
		*(long *)(((char *)ourdcb->tpdb) + item->data) =
				value;
	return (rtn);
}


//************************************************************************
// Function: xosipSetTpdb4NBO - Process characteristic which sets a 4-byte
//                               network byte order value from the TPDB
// Returned: Number of byte stored if normal or a negative XOS error
//           code if error
//************************************************************************

long XOSFNC __export xosipSetTpdb4NBO(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) >= 0)
		*(long *)(((char *)ourdcb->tpdb) + item->data) =
				convnetlong(value);
	return (rtn);
}


//******************************************************************
// Function: xosipGetIpDev - Process characteristic which return the
//                           name of the IP device used
// Returned: Number of byte stored if normal or a negative XOS error
//           code if error
//******************************************************************

long XOSFNC __export xosipGetIpDev(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysLibStrMov(val, ourdcb->npdb->name));
}


//================================================
// Following functions are common device parameter
//   functions for devices which use IP
//================================================


//*************************************************************************
// Function: xosIpGetRmtNetAS - Get value for IOPAR_NETRMTNETAS (0x0506)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter returns the current remote network address that will be
//   used when sending datagrams.

long XOSFNC __export xosipGetRmtNetAS(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutULongV(val, ourdcb->rmtnetas, cnt));
}


//**********************************************************************
// Function: xosipSetRmtNetAS - Set value for IOPAR_NETRMTNETAS (0x0506)
// Returned: 0 if normal or an XOS error code if error
//**********************************************************************

// This parameter sets the current remote network address that will be
//   used when sending datagrams.

long XOSFNC __export xosipSetRmtNetAS(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	void   *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) >= 0)
		ourdcb->rmtnetas = value;
	return (rtn);
}


//*************************************************************************
// Function: xosipGetRmtPortS - Get value for IOPAR_NETRMTPORTS (0x0508)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter returns the remote port number that is used for sending
//   datagrams. If used in the same IO operation that sends a datagram it
//   returns the value used when sending that datagram.

long XOSFNC __export xosipGetRmtPortS(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutULongV(val, ourdcb->rmtports, cnt));
}


//*****************************************************************
// Function: xosipSetRmtPortS - Set value for IOPAR_NETRMTPORTS (0x0508)
// Returned: 0 if normal or an XOS error code if error
//*****************************************************************

// This parameter sets the remote port number that is used for sending
//   datagrams. If used in the same IO operation that sends a datagram it
//   is used to send that datagram. This value is "sticky" and is used
//   until it is changed.

long XOSFNC __export xosipSetRmtPortS(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	void   *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	if (value <= 0 || value > 0xFFFF)
		return (ER_PARMV);
	ourdcb->rmtports = (ushort)value;
	return (0);
}


//*************************************************************************
// Function: xosipSetRmtName - Set value for IOPAR_NETRMTNAME (0x050A)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter is used to specify an ASCII destination address (DNS
//   or IP address) that will be used to send datagrams. It is converted
//   to an IP address which is set as the value controlled by the
//   IOPAR_NETNETAS parameter. The actual value specified here is NOT
//   retained and cannot be retrieved.

long XOSFNC __export xosipSetRmtName(
	PINDEX  inx,
	char   *val,
	long    cnt,
	void   *data)
{

	return (0);
}


//*************************************************************************
// Function: xosipGetLclNetA - Get value for IOPAR_NETLCLNETA (0x050E)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter returns the current local network address.

long XOSFNC __export xosipGetLclNetA(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutULongV(val, ((IPNPDB *)ourdcb->tpdb)->ipaddr, cnt));
}


//*************************************************************************
// Function: xosipGetMaxPktSz- Get value for IOPAR_NETPKTMAX (0x0512)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter returns the current maximum datagram size. Attempts
//   to send larger datagrams will fail.

long XOSFNC __export xosipGetMaxPktSz(
	PINDEX  inx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutULongV(val, ourdcb->maxpktsz, cnt));
}


//*************************************************************************
// Function: xosipSetMaxPktSz - Set value for IOPAR_NETPKTMAX (0x0512)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter sets the current maximum datagram size. Attempts to
//   send larger datagrams will fail.

long XOSFNC __export xosipSetMaxPktSz(
	PINDEX  inx,
	char   *val,
	long    cnt,
	void   *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	if (value <= 8)
		value = 8;
	else if (value > ourdcb->npdb->maxpktsz)
		value = ourdcb->npdb->maxpktsz;
	ourdcb->maxpktsz = (ushort)value;
	return (0);
}


//*************************************************************************
// Function: xosipGetInqLimit - Get value for IOPAR_NETINQLIMIT (0x0513)
// Returned: Number of bytes stored if normal or an XOS error code if error
//*************************************************************************

// This parameter returns the input datagram queue limit. If a datagram is
//   received when this many datagrams are queued, it is quietly discarded.

long XOSFNC __export xosipGetInqLimit(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutULongV(val, ourdcb->ipinqmax, cnt));
}


//*****************************************************************
// Function: xosipSetInqLimit - Set value for IOPAR_NETINQLIMIT (0x0513)
// Returned: 0 if normal or an XOS error code if error
//*****************************************************************

// This parameter sets the input datagram queue limit. If a datagram is
//   received when this many datagrams are queued, it is quietly discarded.

long XOSFNC __export xosipSetInqLimit(
	PINDEX  pinx,
	char   *val,
	long    cnt,
	void   *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	if (value <= 1)
		value = 1;
	else if ( value > 10)
		value = 10;
	ourdcb->maxpktsz = (ushort)value;
	return (0);
}


//====================================================================
// Following functions are general network level support routines that
//   are called by both the IPn: device and the protocol routines
//====================================================================


//*****************************************************
// Function: iprecvpkt - Process a received packet
// Returned: TRUE if packet accepted, FALSE if rejected
//*****************************************************

// The main function performed by this function is to reconstruct fragmented
//   packets. It also verifies header is valid.

// This function accepts (returns TRUE) for all packets. Any packet with an
//   unknown protocol destination is discarded.

long XOSFNC iprecvpkt(
	IPNPDB *npdb,
	NPB    *npb)
{
	IPPKT *ippkt;
	TPLB  *tplb;
	int    plen;
	int    hlen;
	int    iplen;

	if (npb->label != NPB_LABEL)		// Is this really a network buffer?
		CRASH('NNPB');
	ippkt = (IPPKT *)(npb->data + npb->npofs);
	if ((plen = npb->count - npb->npofs) < 20)
	{
		npdb->cntpsltmn++;
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	if ((ippkt->verlen & 0xF0) != 0x40)
	{
		npdb->cntbadhdr++;
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	hlen = (ippkt->verlen & 0x0F) << 2;
	if (hlen > plen)
	{
		npdb->cntbadhdr++;
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	if (xosipChkSum((char *)ippkt, hlen) != 0xFFFF)
	{									// Check the checksum (This is done in
		npdb->ipcntchksum++;			//   assembler since C does not do 1's
		xosnetGiveNpb(npb);				//   compliment addition very well!)
		return (TRUE);
	}
	if ((iplen = convnetword(ippkt->tlength)) < plen) // Make sure the packet is
	{												  //   long enough for the
		npdb->cntpsltdl++;							  //   data
		xosnetGiveNpb(npb);
		return (TRUE);
	}	
	if (ippkt->dstaddr.l != npdb->ipaddr && ippkt->dstaddr.l != 0 &&
			!((((ippkt->dstaddr.l^npdb->ipaddr) & ~npdb->ipsubnetmask) == 0) &&
			(ippkt->dstaddr.l & npdb->ipsubnetmask) == 0))
	{
		npdb->cntnodst++;				// If not for us
		xosnetGiveNpb(npb);
		return (TRUE);
	}

	// Here with a packet that is for us

	npb->tpofs += hlen;
	npb->count = iplen + npb->npofs;	// Discard any excess

	if (ippkt->fragment & CONVNETWORD(0x3FFF)) // Is this a fragmented packet?
	{
		// Here if we have part of a fragmented packet.  We first check our
		//   current fragment list to see if we are already receiving this
		//   packet.  If so, we check to see if this is a duplicate and, if
		//   not, link it to the list for the packet and check to see if the
		//   packet is now complete. If we are not receiving this packet, we
		//   start receiving it.

		// Format and management of the fragmented packet list:
		//   A list of fragmented packets is keep with head pointer
		//   NPDB.ipfraghead and link pointer NPB.next.  For each fragmented
		//   packet, a sub-list of fragments is keep with head pointer
		//   NPB.link in the first fragment and link pointer NPB.link in
		//   each additional fragment.  Note that all fragments are actually
		//   complete packets.  The list is ordered by fragment offset.  A
		//   timer is maintained using npb_fragtmr in the first fragment as
		//   the counter.  The timer is started when the first fragment for
		//   a packet is received.  If all fragments have not been received
		//   before it times out, the fragments that have been received are
		//   discarded.  The fragment offset, total IP packet length, IP
		//   source address, and the packet ID value are copied to fixed
		//   locations in the packet header for each fragment to make
		//   searching the fragment list quicker.  When all fragments for a
		//   packet have been received, the original packet is reconstructed
		//   and processed.

		CRASH('FRAG');

#if 0
;	c{ECX} = Total size of packet (including ALL headers) according to the
;		   IP total length
;	c{EDX} = Offset of start of IP header

havfrag:INCL	pdb_ipcntnumfrag[EDI]	;Count the fragment
	MOVZWL	EAX, ip_fragment[EDX]	;Get fragment flags and offset
	XCHGB	AL, AH			;Fix up the byte order
	SHLL	EAX, #3t		;Times 8 gives byte offset and moves
					;  the flag bits to the high 16 bits
	MOVL	npb_fragofs[EBX], EAX	;Store this
	ADDW	CX, AX			;Store data length through the end of
	MOVL	npb_count[EBX], ECX	;  this fragment
	MOVZWL	EAX, ip_ident[EDX]	;Store packet ID value in header
	MOVL	npb_fragid[EBX], EAX
	MOVL	ECX, ip_srcipaddr[EDX]	;Store source IP address
	MOVL	npb_fragipa[EBX], ECX
	PUSHL	ESI
	LEAL	ESI, pdb_ipfraghead[EDI] ;Now search the fragment list to see
4$:	MOVL	EDX, [ESI]		 ;  if we are now receiving this packet
	TESTL	EDX, EDX
	JE	8$.S
	CMPL	EAX, npb_fragid[EDX]
	JNE	6$.S
	CMPL	ECX, npb_fragipa[EDX]
	JE	10$.S			;Found it
6$:	LEAL	ESI, npb_next[EDX]	;Not this one - get next
	JMP	4$.S			;Continue if more to check

;Here if we are not now receiving this packet - add it to the fragment list

8$:	MOVL	npb_fragtmr[EBX], #TIMEOUT_FRAG
	CLRL	EAX
	MOVL	npb_fragnxt[EBX], EAX
	MOVL	npb_next[EBX], EAX
	MOVL	[ESI], EBX
	POPL	ESI			;That all for now (since this is the
	RET				;  first fragment, the packet can't be
					;  complete now!)

;Here if we are currently receiving this packet - First search through the
;  fragments we have and find the place for this one.  Note that duplicate
;  and/or overlapping fragments are possible since it is valid to retransmit
;  a packet without changing its ID and different copies of the same packet
;  may be fragmented differently.
;	c[EDX] = Offset of first fragment for packet
;	c[ESI] = Offset of pointer to first fragment

10$:	PUSHL	ESI			;Save offset of pointer to first
					;  fragment for packet
	MOVZWL	EAX, npb_fragofs[EBX]
	CMPW	AX, npb_fragofs[EDX]	;Will this be the new first fragment?
	JA	12$.S			;No
	MOVL	EAX, npb_fragtmr[EDX]	;Yes - copy the current timer value
	MOVL	npb_fragtmr[EBX], EAX
	MOVL	npb_fragnxt[EBX], EDX
	MOVL	EAX, [ESI]
	MOVL	npb_next[EBX], EAX
	MOVL	[ESI], EBX
	JMP	22$.S			;Now see if we have a complete packet

;Here if this fragment will not become the new first fragment for the packet.
;  Find where it goes in the list.

12$:	MOVL	ECX, npb_count[EBX]
	LEAL	ESI, npb_fragnxt[EDX]
14$:	MOVL	EDX, [ESI]
	TESTL	EDX, EDX
	JE	20$.S
	CMPW	AX, npb_fragofs[EDX]	;Does it go before this fragment?
	JBE	16$.S			;Yes
	CMPL	ECX, npb_count[EDX]	;No - is it completely included in this
					;  fragment?
	JBE	18$.S			;Yes - just quietly discard it!
	LEAL	ESI, npb_fragnxt[EDX]
	JMP	14$.S

;Here this fragment has an offset less than or equal to the fragment we are
;  checking.  Either it goes before this fragment, it replaces this fragment,
;  or it should be discarded.

16$:	JB	20$.S			;If it definately goes before
	CMPL	EAX, npb_count[EDX]	;Same offset - is this one larger?
	JBE	18$.S			;No - discard this fragment
	MOVL	EAX, npb_fragnxt[EDX]	;Yes - use this one instead
	MOVL	npb_fragnxt[EBX], EAX
	MOVL	[ESI], EBX
	MOVL	EBX, EDX		;Discard the one we had in the list

;Here to discard this fragment because it is completely contained within a
;  fragment we already have

18$:	POPL	EAX
	POPL	ESI
	CALL	xosnetGiveBufr
	RET

;Here to insert this fragment into the list before the fragment we are checking

20$:	MOVL	npb_fragnxt[EBX], EDX
	MOVL	[ESI], EBX

;Since we have added a fragment to the list, we must check and see if we have
;  a complete packet now.

22$:	POPL	ESI			;Get offset of pointer to first fragment
	MOVL	EBX, [ESI]
	CMPW	npb_fragofs[EBX], #0	;Is this really the first fragment?
	JNE	26$.S			;No - packet is not complete
	MOVL	ECX, npb_count[EBX]	;Get total length of first fragment
24$:	MOVL	EDX, npb_fragnxt[EBX]	;Get offset of next fragment
	TESTL	EDX, EDX
	JE	28$.S			;If no more
	MOVZBL	EAX, npb_tpofs[EBX]
	NEGL	EAX
	ADDL	EAX, npb_count[EBX]	;Are these fragments contiguous?
	CMPW	AX, npb_fragofs[EDX]
	JB	26$.S			;No - packet is not complete
	MOVL	EBX, EDX		;Yes - advance to next packet
	MOVZBL	EAX, npb_tpofs[EBX]	;Get length of everything after the IP
	NEGL	EAX			;  header
	ADDL	EAX, npb_count[EBX]
	ADDL	ECX, ECX
	JMP	24$.S

;Here if packet is not complete - just return quietly

26$:	POPL	ESI
	RET

;Here at end of fragment list if all of the fragments are contiguous.  See if
;  the last fragment is the top fragment

28$:	TESTB	npb_fragofs+2[EBX], #01h ;Is this the last fragment?
	JNE	26$.S			;No - packet is not complete yet
					;Yes - fall into fragcomp on next page
.PAGE
;Here with a complete fragmented packet - Now we have to put it back together.
;	c{ECX} = Length of complete packet
;	c{EBX} = Offset of last fragment

fragcomp:
	INCL	pdb_ipcntpktfrag[EDI]	;Count the fragmented packet
	MOVL	EDX, [ESI]		;Get offset of first fragment
	MOVL	EAX, npb_next[EDX]	;Remove it from the fragmented packet
	MOVL	[ESI], EAX		;  list
	MOVL	ECX, npb_count[EBX]	;Get total size of packet
	PUSHL	ECX
	CALL	xosnetGetBufr		;Get a network buffer for the
	POPL	ECX			;  complete packet
	JC	4$.S			;If error
	MOVL	npb_count[EBX], ECX
	MOVL	EAX, npb_lpofs[EDX]
	MOVL	npb_lpofs[EBX], EAX
	MOVL	ECX, npb_count[EDX]	;Get total length of the first fragment
	CLD
	PUSHL	EDI
	MOVL	ESI, EDX
	MOVL	EDI, EBX
2$:	PUSHL	DS
	POPL	ES
	RMOVSB	[EDI], [ESI]
	PUSHL	npb_fragnxt[EDX]
	XCHGL	EBX, EDX
	CLRL	EAX
	MOVL	npb_fragnxt[EBX], EAX
	CALL	xosnetGiveBufr
	MOVL	EBX, EDX
	POPL	EDX
	TESTL	EDX, EDX
	JE	10$.S
	MOVZBL	ECX, npb_tpofs[EDX]
	LEAL	ESI, [EDX+ECX]
	NEGL	ECX
	ADDL	ECX, npb_count[EDX]
	MOVZWL	EDI, npb_fragofs[EDX]
	SUBL	ECX, EDI
	MOVZBL	EAX, npb_tpofs[EBX]
	ADDL	EDI, EAX
	ADDL	EDI, EBX
	JMP	2$.S

;Here if error allocating a buffer for the reconstructed packet - This probably
;  means the system is in big trouble, but we try to handle it as best as we
;  can by simply discarding all of the collected fragments.

4$:	POPL	ESI
	MOVL	EBX, EDX
6$:	PUSHL	npb_fragnxt[EBX]	;Give up the buffers
	CALL	xosnetGiveBufr
	POPL	EBX
	TESTL	EBX, EBX
	JNE	6$.S
	RET

;Here with the reconstructed packet

10$:	MOVL	ECX, EDI
	POPL	EDI
	POPL	ESI
	SUBL	ECX, EBX		;Get length of the packet
	MOVZBL	EAX, npb_tpofs[EBX]	;Get length of IP part
	SUBL	ECX, EAX
	MOVB	AL, npb_npofs[EBX]	;Get offset of start of IP header
	LEAL	EDX, [EBX+EAX]
					;Fall into rcvrdy on next page
.PAGE

#endif

	}

	// Here with a complete packet to process

	npdb->cntpktin++;					// Count the packet
	npdb->cntbytein += npb->count;

	// Find a protocol to handle this packet

	tplb = npdb->firsttplb;
	while (tplb != NULL)
	{
		if (tplb->protocol == ippkt->protocol || tplb->protocol == -1)
		{
			if ((tplb->recvfnc)(tplb->blk, npb)) // Found one
				return (TRUE);			// If finished
		}
		tplb = tplb->next;
	}

	// If get there, no one is available to handle the packet or every one
	//   available to handle it has rejected it.

	npdb->cntnodst++;					// Count the unwanted packet
	xosnetGiveNpb(npb);					// Discard the packet
	return (TRUE);
}


//*******************************************
// Function: ipoas - IP once-a-second routine
// Returned: Nothing
//*******************************************

// This routine times out collection of fragmented packets.

void ipoas(void)
{
#if 0

ipsoas:	MOVL	EDI, xosipPdbHead
	TESTL	EDI, EDI
	JE	16$.S
4$:	TOFORK
	LEAL	ESI, pdb_ipfraghead[EDI]
	MOVL	EBX, [ESI]		;Get first packet
	TESTL	EBX, EBX
	JE	14$.S			;If none
6$:	DECL	npb_fragtmr[EBX]	;Reduce the timer count
	JNS	10$.S			;Go on if still some time left
	MOVL	EAX, npb_next[EBX]	;Timed out - remove from the fragmented
	MOVL	[ESI], EAX		;  packet list
8$:	INCL	pdb_ipcntdisfrag[EDI]	;Count the discarded fragment
	PUSHL	npb_fragnxt[EBX]	;Save offset of next fragment buffer
	MOVL	npb_fragnxt[EBX], #0	;Must clear this here so xosnetGiveBufr
					;  does not get confussed!
	CALL	xosnetGiveBufr	;Give up the fragment buffer
	POPL	EBX
	TESTL	EBX, EBX		;Have another fragment buffer?
	JNE	8$.S			;Yes - continue
	JMP	12$.S			;No

;Here if this partial packet has not timed out yet

10$:	LEAL	ESI, npb_next[EBX]	;Advance to next packet in list
12$:	MOVL	EBX, [ESI]
	TESTL	EBX, EBX
	JNE	6$.S			;Continue if more partial packets
14$:	FROMFORK			;No more for this PDB
	MOVL	EDI, pdb_nextd[EDI]	;Advance to next PDB
	TESTL	EDI, EDI
	JNE	4$.S			;Continue if more PDBs
16$:	JMP	routeoas#		;Finished here - go do routing stuff

#endif

	routeoas();							// Do routing once-a-second stuff
}

#if 0
	.SBTTL	ipabort - Subroutine to abort all IP output

;Subroutine to abort all IP output
;	c{EDI} = Offset of PDB
;	c{ESI} = Offset of SDB
;	CALL	ipabort

ipabort:CALL	icmpabortping#
	RET

#endif



//==============================================================
// Following functions are characteristics functions for various
//   protocol levels
//==============================================================






//=======================================================================
// Following functions manage ports for transport level protocols/devices
//=======================================================================

#if 0

	.SBTTL	xosipGetPort - Subroutine to assign transport level port

;Subroutine to assign a specified transport level port number - this routine
;  will fail if the specified port is in use and duplicates are not enabled
;	c{ECX} = Desired port number, bit 31 set if should increment number
;		   if not acceptable, bit 30 set if duplicate number OK (bit 30
;		   is tested first)
;	c{EDI} = Offset of DCB
;	CALL	xosipGetPort
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal
;	  c[CX] = Port number stored in dcb_ipslclport

xosipGetPort::
	CMPL	dcb_ipshashnext[EDI], #-1 ;In a hash list now?
	JNE	10$			;Yes - big problem!
	CMPW	dcb_ipslclport[EDI], CX ;Really changing it?
	JE	18$			;No - nothing needed here!
	MOVZWL	EAX, dcb_ipslclport[EDI] ;No
	PUSHL	EAX			;Save current port (if any)
	TESTL	EAX, EAX		;Have a current port?
	JE	2$			;No
	PUSHL	ECX			;Yes
	CALL	xosipGivePort		;Give up current port
	POPL	ECX
2$:	CMPL	dcb_ipsportnext[EDI], #-1
	JNE	8$
	MOVL	EBX, dcb_ipstpdb[EDI]	;Point to port list head pointer
	ADDL	EBX, #tpdb_fport
4$:	MOVL	EDX, [EBX]		;Get next DCB in port list
	TESTL	EDX, EDX
	JE	20$
	CMPW	dcb_ipslclport[EDX], CX ;Check port number
	JE	12$			;Same
	JA	20$			;OK - go insert it here
6$:	LEAL	EBX, dcb_ipsportnext[EDX] ;Not yet - advance to next DCB
	JMP	4$

8$:	CRASH	BDPL			;[BaD Port List]

10$:	CRASH	INHL			;[IN Hash List]

;Here with duplicate port number

12$:	BTL	ECX, #30t		;Is a duplicate number OK?
	JC	20$			;Yes - go insert our DCB into the list
	TESTL	ECX, ECX		;No - should we increment it?
	JNS	14$			;No - fail!
	INCW	CX			;Yes - bump the port number
	JNE	6$			;Continue if can
14$:	POPL	ECX			;Restore previous port number
	JREGZ	ECX, 16$		;If none
	CALL	xosipGetPort		;Use previous port (this should not
					;  fail!)
16$:	MOVL	EAX, #ER_NPRIU		;Get error code
	STC
18$:	RET

;Here with place to insert this DCB in the port list

20$:	POPL	EAX			;Discard old port number
	MOVW	dcb_ipslclport[EDI], CX ;Store port number
	MOVL	[EBX], EDI		;Insert this DCB into the port list
	MOVL	dcb_ipsportnext[EDI], EDX
	CLC
	RET
.PAGE
	.SBTTL	xosipGivePort - Subroutine to give up a transport level port number

;Subroutine to give a transport level port number
;	c{EDI} = Offset of DCB
;	CALL	xosipGivePort

xosipGivePort::
	CMPL	dcb_ipshashnext[EDI], #-1 ;In a hash list now?
	JNE	4$			;Yes - big problem!
	MOVL	EBX, dcb_ipstpdb[EDI]	;No - point to port list head pointer
	ADDL	EBX, #tpdb_fport
2$:	MOVL	EDX, [EBX]		;Get next DCB in port list
	TESTL	EDX, EDX
	JE	6$			;No!
	CMPL	EDX, EDI		;This one?
	JE	10$			;Yes
	LEAL	EBX, dcb_ipsportnext[EDX] ;No - advance to next
	JMP	2$			;Continue

4$:	CRASH	INHL			[IN Hash List]

6$:	CRASH	NIPL			;[Not In Port List]

;Here with our dcb found in the list

10$:	MOVL	EAX, dcb_ipsportnext[EDI] ;Remove DCB from the list
	MOVL	[EBX], EAX
	MOVW	dcb_ipslclport[EDI], #0 ;Clear the port number
	MOVL	dcb_ipsportnext[EDI], #-1
	RET				;Finished
.PAGE

#endif

//==================================================================
// Following functions are used to return device info for transfport
//   level protocols/devices
//==================================================================

#if 0

;Subroutine to store local port number for devinfo (stack frame must be set
;  up before calling)
;	c{ES:EDI} = Address of user buffer
;	CALL	xosipDevInfoLcl

$$$=!0				;Must match definitions in calling modules!
FRM info_left  , 4t
FRM info_amount, 4t
info_SIZE=!$$$

xosipDevInfoLcl:
	MOVL	EBX, #lcllbl		;Followed by "L="
	CALL	knlInfoStr
	MOVZWL	EAX, dcb_ipslclport[ESI] ;Get local port number
	CALL	knlInfoDec		;Store the port number
	CMPL	dcb_ipsrmtaddr[ESI], #0 ;Have a remote address?
	JE	4$.S			;No
	MOVL	EBX, #rmtlbl		;Yes
	CLRL	ECX
	CALL	knlInfoStr
	MOVL	EAX, dcb_ipsrmtaddr[ESI]
	MOVZWL	EDX, dcb_ipsrmtport[ESI]
	XCHGB	DL, DH
	CALL	xosipInfoAddr
4$:	DECL	info_left[EBP]		;Have room for a null?
	JS	6$.S			;No
	IFFAULT	knlRtnAdrErLv		;Yes - store one
	MOVB	ES:[EDI], #0
6$:	MOVL	EAX, info_amount[EBP]
	LEAVE
	RET

	DATA
	.MOD	4
lcllbl:	.ASCIZ	"L="
rmtlbl:	.ASCIZ	" R="

	CODE


;Subroutine to store network address into the information buffer
;	c{EAX}    = IP address
;	c{EDX}    = Port number
;	c{ES:EDI} = Buffer pointer
;	CALL	xosipInfoAddr

xosipInfoAddr::
	MOVL	ECX, #4t
	PUSHL	EDX
	PUSHL	EAX
14$:	MOVZBL	EAX, [ESP]
	RORL	[ESP], #8t
	CALL	knlInfoDec
	MOVB	AL, #'.'
	CALL	knlInfoChr
	LOOP	ECX, 14$
	POPL	EAX
	POPL	EAX			;Fall into knlInfoDec
	JMP	knlInfoDec

#endif

//=================================================================
// Following are miscelanious support routines for transfport level
//=================================================================


//============================================================================
//
// The scheme used here to set up packets to be sent is somewhat unusual.
//   Inorder to not have to copy data to make room to lower level packet
//   headers we set up these headers before any data is stored in the packet,
//   generally as soon as the packet buffer is allocated. This means the
//   packet buffer must be allocated big enough to allow room for all of the
//   lower level headers (since we require that packets be stored in a single
//   buffer). This only causes a problem with IP and transport level (TCP and
//   UDP packets) since there headers are variable length. Fortunately, header
//   options are generally not used and are realitively easy to account for in
//   advance.
//
// The general proceedure to output an IP packet is:
//   1) Call xosnetGetBuffer to allocate a packet buffer (NPB).
//   2) Call xosipSetPkt to set up the IP (network) and link level header.
//        This functions calls the interface driver's setpkt function to set
//        up the hardware level header.
//   3) Store the transport level header and data in the packet.
//   4) Set the outdone item in the NPB if necessary. If this is not set, the
//        NPB is given up after the packet is output.
//   5) Call xosipSendPkt to send the packet. This will strore the link level
//        hardware address in the packet (using ARP if necessary) and output
//        the packet.
//
//============================================================================


//**********************************************************
// Function: xosipSetPkt - Set up IP datagram packet header
// Returned: Offset (relative to NPB.data) of the first byte
//           beyond the IP header (positive) if normal or a
//           negative XOS error code if  error
//**********************************************************

// This function does not set the IP header length value since it is often
//   not known when this is called. It is set from the NPB.count value when
//   xosipSendPkt is called. The header checksum is also calculated and stored
//   by xosipSendPkt.

long XOSFNC __export xosipSetPkt(
	TPDB *tpdb,
	NPB  *npb,
	long  ipaddr)		// Destination IP address
{
	IPPKT  *pkt;
	IDB    *idb;
	IPNPDB *npdb;

	if (npb->label != NPB_LABEL)		// Is this really a network buffer?
		CRASH('NPDB');

	npdb = (IPNPDB *)tpdb->npdb;

	idb = npdb->idb;
	idb->ifdisp->setpkt(idb, npb, npdb->nplb.etype); // Set up the link level
													 //   header
	pkt = (IPPKT *)(npb->data + npb->npofs);
	*(long *)&pkt->verlen = 0x45;		// Store version, header length, and
	*(long *)&pkt->ident = 0;			// Store ident and frag data (both 0)

	*(long *)&pkt->t2l = (tpdb->tplb.protocol << 8) + 128;
										// Store time to live, IP protocol,
										//   and header checksum value (0)
	pkt->srcaddr.l = npdb->ipaddr;		// Store source IP address
	pkt->dstaddr.l = ipaddr;			// Store destination IP address
	npb->count += sizeof(IPPKT);		// Increase packet byte count
	npb->tpofs += sizeof(IPPKT);
	return (npb->tpofs);
}


//******************************************************************
// Function: xosipSendPkt - Finish setting IP header and send packet
// Returned: 0 if normal or a negative XOS error code if error
//******************************************************************

// In all cases, the packet is consumed, either it is queued for output or
//   it is given up.

long XOSFNC __export xosipSendPkt(
	IPNPDB *npdb,
	NPB    *npb)
{
	IPPKT *pkt;
	long   len;
	long   rtn;

	pkt = (IPPKT *)(npb->data + npb->npofs);
	len = npb->count - npb->npofs;
	pkt->tlength = convnetword(len);	// Store length of the IP packet
	pkt->hdrchksum = ~xosipChkSum((char *)pkt, (pkt->verlen & 0x0F) << 2);
	npdb->cntpktout++;					// Bump our counts
	npdb->cntbyteout += len;
	if ((rtn = routegetaddr(npdb, npb, FALSE)) < 0) // Get the hardware address
	{
		xosnetGiveNpb(npb);
		return (rtn);
	}
	if (rtn == 1)						// Send packet on the network?
		xosnetSendPkt(npdb->idb, npb);	// Yes
	else if (rtn == 2)					// Send to self?
	{

		CRASH('SELF');

	}
	return (0);
}


//*********************************************************************
// Function: xosipParseAddress - Parse IP address string
// Returned: Offset in string of character after the address (positive)
//           if normal or a negative XOS error code if error
//*********************************************************************

// Must be called from main program level. This function may initiate
//   network IO and wait for it to complete.//

// The address string has the following formats: ({} indicates optional items)
//     nn.nn.nn.nn{|port}
//     dddd.dddd{.dddd{...}}}{|port}
//   Where nn is a decimal between 0 and 255, dddd is a domain name element,
//   and port is a port number between 1 and 65535. The address string must be
//   terminated by the end of the string or by a double colon (::).

// If a URL is specified, it is looked up using DNS. The IP address is stored
//   in ipdstnetas in the DCB. The port number is stored in ipdstports in the
//   DCB.

// Differeniating between DNS names and non-delimited IP addresses is difficult
//   since DNS names can begin with a digit and in theory can contain only
//   digits. Fortunately, in practice, all top level domains are alpha, so
//   we should always see some alpha characters in a domain name.  We
//   differentiate between IP address and domain names by assuming anything
//   which contains both a period and a dash is a domain name.

// Access to the string is NOT protected from page faults.

long XOSFNC __export xosipParseAddress(
	char *str,				// Address of string
	long  portok)			// TRUE if port number allowed, FALSE if not
{
	char *pnt1;
	char *pnt2;
	char *pnta;
	long  rtn;
	long  port;
	long  ipaddr;
	long  value;
	int   cnt;
	char  chr;
	char  hvalpha;

	// First determine what we have by scanning the string looking for alpha
	//   characters and counting periods. If there are any alpha characters
	//   and 1 or more periods it is a URL.. If there are only digits and
	//   exactly 3 periods, it is an IP address. Otherwise it is invalid.
	//   It is also invalid if any non-alpha, non-digit, non-period, or
	//   non-vertical bar characters.

	pnt1 = str;
	cnt = 0;
	hvalpha = FALSE;
	port = 0;
	while ((chr = *pnt1) != 0 && chr != ':' && chr != '|')
	{
		pnt1++;
		if (chr == '.')
			cnt++;
		else if (isalpha(chr))
			hvalpha = TRUE;
		else if (!isdigit(chr))
			return (ER_NILAD);
	}
	if ((hvalpha && cnt < 1) || (!hvalpha && cnt != 3))
		return (ER_NILAD);
	if (chr == '|')						// Do we have a port number?
	{
		if (!portok)					// Is this allowed?
			return (ER_NILAD);			// No - fail
		if ((rtn = sysLibStr2Dec(pnt1, &port)) < 0)
			return (rtn);
		pnt1 += rtn;
		if ((pnt1[0] != 0 && !(pnt1[0] == ':' && pnt1[1] == ':')) ||
				port <= 0 || port > 0xFFFF)
			return (ER_NILAD);
	}
	if (pnt1[0] == ':')
		pnt1++;

	if (hvalpha)
	{
		// Here if have a URL

		return (ER_NNSNA);

	}
	else
	{
		// Here if have an IP address

		cnt = 4;
		pnta = (char *)&ipaddr;
		pnt2 = str;
		while (TRUE)
		{
			if ((rtn = sysLibStr2Dec(pnt2, &value)) < 0)
				return (rtn);
			if (value < 0 || value > 255)
				return (ER_NILAD);
			*pnta++ = (char)value;
			if (--cnt <= 0)
				break;
			pnt2 += rtn;
			if (pnt2[0] != '.')
				return (ER_NILAD);
			pnt2++;
		}
	}
	ourdcb->rmtnetas = ipaddr;
	if (portok)
		ourdcb->rmtports = port;
	return (pnt1 - str + 1);
}


#if 0

;Subroutine to store IP address string. The string stored can always be used
;  as input to xosipParseAddress. It has the format:
;	[nn.nn.nn.nn]|port
;  Where nn is a decimal between 0 and 255, and port is a port number between
;    1 and 65535.
;	c{ES:EBX} = Address of buffer
;	c{ECX}    = Length of buffer
;	c{EAX}    = IP address
;	c{EDX}    = Port number
;	CALL	xosipParseAddress
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal
;	c{ECX} = Length of string stored

$$$=!0
FRM sa_left, 4t
FRM sa_size, 4t
FRM sa_addr, 4t
FRM sa_port, 4t
sa_SIZE=!$$$

xosipStoreAddress:
	DECL	ECX
	JS	6$.S
	ENTER	sa_SIZE, 0
	MOVL	sa_left[EBP], ECX
	MOVL	sa_size[EBP], ECX
	MOVL	sa_addr[EBP], EAX
	MOVL	sa_port[EBP], EDX
	MOVB	AL, #'['
	CALL	storechr
	MOVZBL	EAX, sa_addr+0[EBP]
	CALL	storedec
	MOVB	AL, #'.'
	CALL	storechr
	MOVZBL	EAX, sa_addr+1[EBP]
	CALL	storedec
	MOVB	AL, #'.'
	CALL	storechr
	MOVZBL	EAX, sa_addr+2[EBP]
	CALL	storedec
	MOVB	AL, #'.'
	CALL	storechr
	MOVZBL	EAX, sa_addr+3[EBP]
	CALL	storedec
	MOVB	AL, #']'
	CALL	storechr
	MOVB	AL, #'|'
	CALL	storechr
	MOVL	EAX, sa_port[EBP]
	CALL	storechr
4$:	MOVL	ES:[EBX], #0
	MOVL	EAX, sa_size[EBP]
	SUBL	EAX, ECX
	LEAVE
6$:	RET


storedec:
	CLRL	EDX
	DIVL	lit10
	PUSHL	EDX
	ORL	EAX, EAX
	JE	8$.S
	CALL	storedec
8$:	POPL	EAX
	ANDB	AL, #0Fh
	ADDB	AL, #'0'
storechr:
	JREGZ	ECX, 4$
	DECL	ECX
	MOVB	ES:[EBX], AL
	RET	
.PAGE
	.SBTTL	ipsrcv - Subroutine to process received IP packet

2$:	POPL	ECX
	POPL	ESI
4$:	INCL	pdb_ipcntpsltmn[EDI]
	JMP	7$.S

6$:	POPL	ECX
7$:	INCL	pdb_ipcntbadhdr[EDI]
8$:	CALL	xosnetGiveBufr
	RET

10$:	POPL	ECX
	INCL	pdb_ipcntchksum[EDI]
	JMP	8$.S

#endif
