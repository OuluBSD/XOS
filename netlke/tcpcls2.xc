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


//==================================================
// This file contains routines for receiving packets
//==================================================


//*****************************************************
// Function: tcprecvpkt - Process a received TCP packet
// Returned: TRUE if packet accepted, FALSE if rejected
//*****************************************************

// This function accepts all packets and thus always returns TRUE.

// This is a VERY long function. It would be nice to break it up into multiple
//   functions (especially in the bits switch statement), but we really don't
//   want to introduce the overhead of passing lots of arguments or of haning
//   to reconstruct various pointers and counts in the functions called. If we
//   had local functions available this would be a good compromise, but we
//   don't!! We do call a function to handle SYNs, but that does not happen
//   very often compared to receiving a packet.

long XOSFNC tcprecvpkt(
	TCPTPDB *tpdb,
	NPB     *npb)
{
	NPB     *nnpb;
	TCPPKT  *npkt;
	IPNPDB  *npdb;
	IPPKT   *ippkt;
	IPPKT   *nippkt;
	TCPPKT  *pkt;
	TCPDDCB *dcb;
	TCPDDCB *pdcb;
	TCPDDCB *cdcb;
	char    *pnt;
	long     count;
	long     maxseg;
	long     hdlen;
	long     hinx;

	if (npb->label != NPB_LABEL)
		CRASH('NNPB');

	npdb = (IPNPDB *)tpdb->npdb;
	pkt = (TCPPKT *)(npb->data + npb->tpofs);
	count = (npb->count -= npb->tpofs);
	if (count < sizeof(TCPPKT))			// Is it big enough for the minimum
	{									//   header?
		tpdb->cntpsltmn++;
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	if ((hdlen = pkt->hdlen << 2) < 20)	// Get the length of the header, make
	{									//   sure not too small
		tpdb->cntbadhdr++;
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	if (hdlen < count)					// Is the packet long enough to contain
	{									//   the header
		tpdb->cntpsltmn++;
		xosnetGiveNpb(npb);
		return (TRUE);
	}
	npb->apofs = npb->tpofs + hdlen;	// Store offset of the data part
	npb->apcnt = count - hdlen;			// Store length of the data part

	ippkt = (IPPKT *)(npb->data + npb->npofs); // Get address of the IP header
	if (xosipChkSumPH(ippkt->srcaddr.l, ippkt->dstaddr.l,	  // Check the
			convnetlong((tpdb->tplb.protocol << 16) + count), //   checksum
			(char *)pkt, count) != 0xFFFF)
	{
		tpdb->cntchksum++;				// If bad checksum
		xosnetGiveNpb(npb);
		return (TRUE);
	}

	maxseg = 536;
	if ((pkt->code & TCPC_SYN) && (hdlen -= 5) > 0)	// Have any options? (We
	{												//   don't support any
		pnt = pkt->options;							//   non-SYN options!)
		do
		{
			switch (*pnt)
			{
			 case TCPO_END:
				hdlen = 0;
				break;

			 case TCPO_FILL:
				hdlen--;
				break;

			 case TCPO_MAXSEG:
				if (pnt[1] == 4 && hdlen >= 4)
				{
					maxseg = convnetword(*(short *)(pnt + 2));
					if (maxseg > (tpdb->maxpktsz - sizeof(TCPPKT)))
						maxseg = tpdb->maxpktsz - sizeof(TCPPKT);
				}
			 default:
				hdlen -= pnt[1];
				pnt += pnt[1];
				break;
			}
		} while (hdlen > 0);
		if (hdlen < 0)					// Bad option list?
		{
			tpdb->tcpcntbadopt++;		// Yes - Count the error and discard
			xosnetGiveNpb(npb);			//   the packet
		}
	}

	// Calculate the hash index - this is the XOR of all bytes in the remote
	//   and local port number and the remote IP address giving an 8-bit hash
	//   index value.

	hinx = pkt->srcport ^ pkt->dstport ^ ippkt->srcaddr.l;
	hinx ^= (hinx >> 16);
	hinx = (char)((hinx >> 8) ^ hinx);

	dcb = tpdb->tcpconhash[hinx];
	while (dcb != NULL)
	{
		if (ippkt->srcaddr.l == dcb->rmtnetas &&
				pkt->srcport == dcb->rmtports &&
				pkt->dstport == dcb->tcplclport)
			break;
		dcb = dcb->tcphashnext;
	}
	if (dcb == NULL)
	{
		// Here if there is no matching DCB

		if (pkt->code == TCPC_SYN)		// Is this a SYN?
		{
			// Here with SYN packet. We see if there's a DCB on the SYN wait
			//   list with a matching port.

			dcb = tpdb->tcpsynhead;
			pdcb = NULL;
			while (dcb != NULL)
			{
				if (pkt->dstport == dcb->rmtports)
					break;
				pdcb = dcb;
				dcb = dcb->tcpsynnext;
			}
			if (dcb == NULL)			// Did we find one?
			{

				// SEND A SYN|RST TO REFUSE THE CONNECTION!!



				tpdb->cntnodst++;
				xosnetGiveNpb(npb);
				return (TRUE);
			}

			// Here with a DCB that wants a SYN

			if (dcb->tcpconlimit == 0) // Is this a type 2 DCB?
			{
				if (pdcb == NULL)	// No - remove it from the list
					tpdb->tcpsynhead = dcb->tcpsynnext;
				else
					pdcb->tcpsynnext = dcb->tcpsynnext;
				if (dcb->tcpsynnext == NULL)
					tpdb->tcpsyntail = pdcb;

				if (dcb->tcprcvhead != NULL)
					CRASH('RPNN');
				dcb->tcprcvhead = npb;	// Give him the packet
				hinx = pkt->srcport ^ pkt->dstport ^ ippkt->srcaddr.l;
				hinx ^= (hinx >> 16);
				hinx = (char)((hinx >> 8) ^ hinx);
				dcb->tcphashnext = tpdb->tcpconhash[hinx];
				tpdb->tcpconhash[hinx] = dcb;
				if (dcb->outthrd != NULL) // Make him up if need to
					sysIoResumeThread(dcb->outthrd, 0);
				return (TRUE);
			}

			// Here if we have a DCB for a type 2 connect - If there are
			//   any DCB's Queued on this DCB we give the packet to one of
			//   them. (We must hash that DCB here to avoid a race if we get
			//   a retransmission of the SYN before this one is processed.)
			//   If not, we check to see if we already have a matching SYN
			//   packet queued to the DCB. If we do, we discard this one. If
			//   not we put the SYN packet on the DCB's packet list.

			if ((cdcb = dcb->tcpconhead) != NULL)
			{
				if ((dcb->tcpconhead = cdcb->tcpconnext) == NULL)
					dcb->tcpcontail = NULL;
				cdcb->tcprcvhead = npb;
				hinx = pkt->srcport ^ pkt->dstport ^ ippkt->srcaddr.l;
				hinx ^= (hinx >> 16);
				hinx = (char)((hinx >> 8) ^ hinx);
				dcb->tcphashnext = tpdb->tcpconhash[hinx];
				tpdb->tcpconhash[hinx] = dcb;
				if (cdcb->outthrd != NULL) // Make him up if need to
					sysIoResumeThread(cdcb->outthrd, 0);
				return (TRUE);
			}

			// Here if no DCB is waiting - Must queue the SYN packet

			if (dcb->tcpconcnt >= dcb->tcpconlimit)
			{
				xosnetGiveNpb(npb);
				return (TRUE);
			}
			dcb->tcpconcnt++;
			nnpb = dcb->tcprcvhead;
			while (nnpb != NULL)
			{
				nippkt = (IPPKT *)(nnpb->data + nnpb->npofs);
				npkt = (TCPPKT *)(nnpb->data + nnpb->tpofs);
				if (nippkt->srcaddr.l == ippkt->srcaddr.l &&
					npkt->srcport == pkt->srcport)
				{
					tpdb->cntnodst++;	// Count the discarded packet
					xosnetGiveNpb(npb);
					return (TRUE);
				}
				nnpb = nnpb->next;
			}

			// Here if this is not a duplicate

			if (dcb->tcprcvtail == NULL)
				dcb->tcprcvhead = npb;
			else
				dcb->tcprcvtail->next = NULL;
			dcb->tcprcvtail = npb;
			return (TRUE);
		}

		// Here with a non-SYN packet that we have no destination for.

		if (pkt->code == (TCPC_FIN|TCPC_ACK))
		{
			// Here with a FIN|ACK packet which does not have a matching DCB.
			//   We really should ignore this, but we respond to it by sending
			//   a FIN|ACK to keep Suns (and probably other Unix-like systems)
			//   from going through an extremely long FIN_WAIT_1 timeout!

			// SEND BACK A FIN|ACK

		}
		tpdb->cntnodst++;				// Count the discarded packet
		xosnetGiveNpb(npb);
		return (TRUE);
	}

	// With a DCB for the received packet

	tpdb->cntpktin++;
	tpdb->cntbytein += count;

	if (dcb->tcpstate < TCPS_ESTAB)		// Are we setting up a connection?
	{
		switch (dcb->tcpstate)			// Yes - dispatch on current socket
		{								//   state
		 case TCPS_SYNSENT:				// SYN sent

#if 0

;Here with packet after we have sent the initial SYN when establishing a
;  connection (TCPS_SYNSENT) - this should be a SYN|ACK

rcvsynsent:
	CMPL	dcb_tcprcvputb[EDI], #0	;Is the pointer free?
	JNE	ignore			;No - ignore the packet!
	MOVL	dcb_tcprcvputb[EDI], EBX ;Yes - save offset of packet data
	MOVL	dcb_tcprcvputp[EDI], EDX
	MOVZWL	EAX, tcp_window[EDX]	;Get window size
	XCHGB	AL, AH
	MOVL	dcb_tcpoutwina[EDI], EAX ;Store it
	MOVL	dcb_tcpoutwinm[EDI], EAX
	MOVL	EAX, rcv_maxseg[EBP]
	MOVL	EDX, dcb_netpdb[EDI]
	MOVL	EDX, pdb_ipmaxpktsz[EDX]
	SUBL	EDX, #20t
	CMPL	EAX, EDX		;Can we use this size?
	JLE	2$			;Yes
	MOVL	EAX, EDX		;No - just use what we can
2$:

	CMPL	EAX, #2000t
	JB	3$
	INT3
3$:

	MOVL	dcb_tcpoutmaxsg[EDI], EAX
	LEAVE
	CLRL	EAX
	MOVL	EDX, dcb_tcpfuncdisp[EDI]
	JMPIL	CS:tcpf_synsent[EDX]

#endif

			break;


		 case TCPS_SYNACKD:				// ACK sent for SYN|ACK

#if 0

;Here with packet after we have sent the ACK for the SYN|ACK - this is either
;  the first data packet for the TCP connection or is a retransmission of the
;  SYN|ACK (which means that our ACK was lost)

rcvsynackd:
	MOVL	EAX, tcp_seqnum[EDX]	;Is the sequence number right?
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	CMPL	dcb_tcprcvsnum[EDI], EAX
	JNE	4$			;No - ignore the packet!
	MOVB	AL, tcp_code[EDX]	;Yes
	ANDB	AL, #~TCPC$PSH
	CMPB	AL, #TCPC$SYN|TCPC$ACK	;Is this a SYN|ACK?
	JE	6$			;Yes
	MOVB	dcb_tcpstate[EDI], #TCPS_ESTAB ;No - update connection state
	JMP	rcvestab		;Continue

;Here with out of sequence packet

4$:	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the out of sequence packet
	INCL	tpdb_tcpcntoutseq[EAX]
ignore:	MOVL	EBX, rcv_pkt[EBP]	;Make sure have packet buffer offset
	CALL	xosnetGiveBufr##	;Give up the packet buffer
	RET

;Here with retransmission of the SYN|ACK - in this case we just ACK it again

6$:	CALL	xosnetGiveBufr##	;Finished with this packet
	JMP	tcpsendacknow#

#endif

			break;

		 case TCPS_SYNRCVD:				// SYN received

			// Here with the packet which should be the ACK for the SYN|ACK we
			//   just sent - there are several posibilities here:
			//   1. ACK with correct sequence and ACK numbers - This completes
			//      the connection sequence
			//   2. SYN which is a duplicate of the original we received - This
			//      must be a retransmission caused by the loss of our SYN|ACK
			//      - We send it again
			//   3. ACK with FIN indicated with correct sequence and ACK numbers
			//      - This is an aborted connection sequence - We return an
			//      ER_NCLST error
			//   4. All other cases - We ignore the packet and continue waiting

#if 0

rcvsynrcvd:
	MOVL	ECX, tcp_seqnum[EDX]
	XCHGB	CL, CH
	RORL	ECX, #16t
	XCHGB	CL, CH
	MOVB	AL, tcp_code[EDX]
	ANDB	AL, #~TCPC$PSH		;Ignore the PUSH bit here
	CMPB	AL, #TCPC$ACK		;Simple ACK?
	JE	8$			;Yes
	CMPB	AL, #TCPC$ACK|TCPC$FIN	;ACK with FIN?
	JE	8$			;Yes
	CMPB	AL, #TCPC$SYN		;SYN?
	JNE	2$			;No - ignore it
	INCL	ECX			;Yes - is it the same one?
	CMPL	ECX, dcb_tcprcvsnum[EDI]
	JE	xostcpSendSynAck#	;Yes - just resend the SYN|ACK
2$:	MOVL	EAX, dcb_ipstpdb[EDI]	;No - count the out of sequence packet
	INCL	tpdb_tcpcntoutseq[EAX]
	JMP	ignore		;Ignore the packet

;Here with ACK or ACK|FIN in response to our SYN|ACK

8$:	CMPL	ECX, dcb_tcprcvsnum[EDI] ;Is it the one we are expecting?
	JNE	2$			;No - ignore it
	MOVL	EAX, tcp_acknum[EDX]	;Maybe
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	DECL	EAX
	CMPL	EAX, dcb_tcpoutsnum[EDI]
	JNE	2$			;No
	INCL	dcb_tcpoutsnum[EDI]	;Yes - fix up the sequence numbers
	INCL	dcb_tcprcvanum[EDI]
	MOVB	dcb_tcpstate[EDI], #TCPS_ESTAB ;Set new connection state
	CMPL	dcb_tcpconbase[EDI], #0	;Is this a type 2 open?
	JE	10$			;No
	TESTB	tcp_code[EDX], #TCPC$FIN ;FIN indicated?
	JNE	16$			;Yes - fail	
	CLRL	EAX
	CALL	tcptype2ready		;Yes
	JMP	14$

;Here if type 1 open

10$:	CALL	xostcpTimerRmv#		;Stop the connection timer
	MOVL	ESI, dcb_tcpackwake[EDI] ;Stop the ACK timer if its running
	TESTL	ESI, ESI
	JE	12$
	CLRL	EAX
	MOVL	dcb_tcpackwake[EDI], EAX
	CALL	knlWakeRemove##
12$:	MOVL	EBX, rcv_pkt[EBP]
	MOVL	EDX, rcv_tcphdr[EBP]
	TESTB	tcp_code[EDX], #TCPC$FIN ;FIN indicated?
	JNE	20$			;Yes - fail	
	CLRL	EAX
	CALL	xosnetSndDone##
14$:	MOVZBL	EAX, tcp_offset[EDX]	;Get length of TCP header
	SHRL	EAX, #2
	ANDB	AL, #0FCh
	CMPL	rcv_tcplen[EBP], EAX
	JA	estab2			;Go handle data if have any
	JMP	xosnetGiveBufr##	;No data - discard the packet

;Here if FIN indicated - for type 2 open - fail

16$:	MOVB	dcb_tcpstate[EDI], #TCPS_ESTAB
	MOVL	EAX, #ER_NCLST
	JMP	tcptype2ready

;Here if FIN indicated - for type 1 open - fail

20$:

#endif
			break;
		}

	}
	else if (dcb->tcpstate <= TCPS_CLOSEWAIT) // Do we have a connection (at
	{										  //   to some extent)
		// Do preliminary packet processing - This processes acknowlegements
		//   and window size changes

#if 0


;	c{EBX} = c{rcv_pkt{EBP}}    = Offset of packet buffer
;	c{ECX} = c{rcv_tcphdr{EBP}} = Length of TCP part of the packet
;	c{EDX} = c{rcv_tcplen{EBP}} = Offset of TCP header
;	c{EDI} = Offset of DCB
;	CALL	procpkt
;	C:set = Cannot continue with packet (packet already given up)
;	C:clr = Can process data part of packet

procpkt:TESTB	tcp_code[EDX], #TCPC$SYN ;Is this a SYN packet?
	JNE	2$			;Yes - treat it like it is below the
	MOVL	EAX, tcp_seqnum[EDX]	;  receive window
	XCHGB	AL, AH			;No - get sequence number from packet
	RORL	EAX, #16t
	XCHGB	AL, AH
	CMPL	EAX, dcb_tcprcvsnum[EDI] ;Is this what we are expecting?
	JE	26$			;Yes
	JNS	8$			;No
2$:	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the out of window packet
	INCL	tpdb_tcpcntoutwin[EAX]
	CALL	tcpsendacknow#		;Below window (or SYN) - ACK it
4$:	MOVL	EBX, rcv_pkt[EBP]
6$:	CALL	xosnetGiveBufr##	;But otherwise ignore it
	STC
	RET

;Here for out of sequence packet which is in the window

8$:	MOVL	npb_seqnum[EBX], EAX
	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the out of sequence packet
	INCL	tpdb_tcpcntoutseq[EAX]
	CALL	procack			;Process the ACK in this packet
	MOVL	EBX, rcv_pkt[EBP]
	MOVL	EDX, rcv_tcphdr[EBP]
	MOVL	ECX, rcv_tcplen[EBP]
	MOVZBL	EAX, tcp_offset[EDX]	;Get length of TCP header
	SHRL	EAX, #2
	ANDL	EAX, #0x0FFFFFFFC
	CMPL	ECX, EAX		;Do we have any data?
	JLE	6$			;No - discard the packet
	CMPB	dcb_tcprcvoosc[EDI], #10t ;Have too many now?
	JAE	6$			;Yes - discard the packet
	SUBL	EDX, EBX		;No
	MOVB	npb_tpofs[EBX], DL	;Store offset of TCP (transport) part
	SUBL	ECX, EAX
	MOVL	npb_apcnt[EBX], ECX
	INCB	dcb_tcprcvoosc[EDI]	;Count this one
	MOVL	EDX, dcb_ipstpdb[EDI]	;Get offset of the TPDB
	INCL	tpdb_tcpcntoosnum[EDX] ;Bump count
	MOVL	EAX, tpdb_tcpcntoosnum[EDX] ;Is this a new maximum?
	CMPL	tpdb_tcpcntoosmax[EDX], EAX
	JAE	10$			;No
	MOVL	tpdb_tcpcntoosmax[EDX], EAX ;Yes - remember it
10$:	MOVL	EAX, npb_seqnum[EBX]	 ;Scan the out-of-sequence list to
	MOVL	EDX, dcb_tcprcvoosl[EDI] ;  find the place to put this packet
	TESTL	EDX, EDX
	JE	12$
	CMPL	EAX, npb_seqnum[EDX]	;Does this one go before the first one?
	JE	6$
	JA	14$			;No
12$:	MOVL	dcb_tcprcvoosl[EDI], EBX ;Yes - link it in
	MOVL	npb_next[EBX], EDX	
	JMP	20$			;Continue	

;Here if this packet does not go before the first packet in the out-of-sequence
;  list

14$:	MOVL	ECX, EDX		;Advance to next packet in list
	MOVL	EDX, npb_next[EDX]
	TESTL	EDX, EDX		;Have another?
	JE	16$			;No
	CMPL	EAX, npb_seqnum[EDX]	;Yes - does this one go here?
	JE	6$
	JA	14$			;No - continue
16$:	MOVL	npb_next[ECX], EBX	;Yes - link it in
	MOVL	npb_next[EBX], EDX
	MOVL	EBX, ECX
	CALL	chkmrg			;See if can merge with previous packet
	JC	24$			;Finished if could merge (in this case
					;  we know we cannot merge any more so
					;  there is no point in checking more)
18$:	MOVL	EBX, npb_next[EBX]	;Advance to next packet
	TESTL	EBX, EBX		;Finished if no next packet
	JE	22$
20$:	CALL	chkmrg			;See if can merge here
	JC	18$			;Continue checking if could merge
22$:	STC
24$:	RET

;Here with the expected packet

26$:	TESTB	tcp_code[EDX], #TCPC$RST ;Is this a reset?
	JNE	resetp			;Yes - go handle it
	TESTB	tcp_code[EDX], #TCPC$ACK ;No - does this packet contain an
					 ;  acknowledgement?
	JE	6$			;No - ignore this packet!
	CALL	procack			;Yes - process the ACK
	MOVL	EBX, rcv_pkt[EBP]	;Restore registers
	MOVL	EDX, rcv_tcphdr[EBP]
	MOVL	ECX, rcv_tcplen[EBP]
	CLC				;Indicate still have packet
	RET
.PAGE
2$:	CLC
	RET

;Here if have a reset packet

resetp:	CALL	xosnetGiveBufr##	;Finished with the packet
	CALL	xostcpGiveAll#		;Give up all buffers and other stuff
	CALL	xostcpHashClr#		;Remove DCB from the hash table
	MOVB	dcb_tcpstate[EDI], #TCPS_CLOSED ;Indicate idle
	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the reset
	INCL	tpdb_tcpcntrstrcvd[EAX]
	MOVL	EAX, #ER_NCLST
	CALL	knlResumeInput##	;Wake him up if he's waiting for
	MOVL	EAX, #ER_NCLST		;  anything
	CALL	knlResumeOutput##
	STC				;Indicate no packet to process
	RET				;Return



#endif

		if (dcb->tcpstate == TCPS_ESTAB) // Connection established?
		{
#if 0

;Here with packet when in the connected (TCPS_ESTAB) state

rcvestab:
	CALL	procpkt			;Do preliminary packet processing
	JC	ret006			;If can not continue with packet
	TESTB	tcp_code[EDX], #TCPC$FIN ;Is this a FIN?
	JNE	2$			;Yes
	CALL	estab0			;No - process the data
	JMP	4$			;Continue

;Here if have a FIN

2$:	CALL	estab0			;Process the packet first
	INCL	dcb_tcprcvsnum[EDI]	;Bump receive sequence number for FIN
	CALL	tcpsendacknow#		;Send ACK for the FIN
	CMPL	dcb_opencnt[EDI], #0	;Is this DCB in use?
	JE	8$			;No
	MOVB	dcb_tcpstate[EDI], #TCPS_CLOSEWAIT ;Yes - update state
	MOVL	EAX, dcb_tcpfuncdisp[EDI] ;Dispatch to routine for protocol
	CALLI	CS:tcpf_rcvfin[EAX]
4$:	MOVL	EBX, dcb_tcprcvoosl[EDI] ;Have any out-of-sequence packets?
	TESTL	EBX, EBX
	JE	10$			;No - finished now
	MOVL	EAX, npb_seqnum[EBX]	;Yes - is this the one we are expecting?
	CMPL	dcb_tcprcvsnum[EDI], EAX
	JNE	10$			;No - finished
	MOVL	EAX, dcb_ipstpdb[EDI]	;Yes - get offset of the TPDB
	DECL	tpdb_tcpcntoosnum[EAX]	;Reduce count
	MOVL	EAX, npb_next[EBX]	;Remove packet from the list
	MOVL	dcb_tcprcvoosl[EDI], EAX
	MOVL	rcv_pkt[EBP], EBX
	MOVZBL	EDX, npb_tpofs[EBX]
	MOVL	ECX, npb_count[EBX]
	SUBL	ECX, EDX
	MOVL	rcv_tcplen[EBP], ECX
	ADDL	EDX, EBX
	MOVL	rcv_tcphdr[EBP], EDX
	DECB	dcb_tcprcvoosc[EDI]
	JS	6$
	MOVZBL	EAX, dcb_tcpstate[EDI]	;Get connection state
	JMPIL	CS:rcvdsp[EAX*4]	;Dispatch on the state to process the
					;  packet

6$:	CRASH	OOSC			;[Out-Of-Sequence Count is bad]

;Common receive finish routine

tcprcvfin::
	MOVL	EAX, #ER_NCCLR		;Wake up anyone who might be waiting
	CALL	xosnetRspDone##
	MOVL	EAX, #ER_NCCLR
	JMP	xosnetSndDone##

;Here with FIN for DCB which is not in use - we must respond here

8$:	CALL	xostcpSendFin#		;Send a FIN
	JC	givedcb			;Error - just terminate it all now
	MOVB	dcb_tcpstate[EDI], #TCPS_LASTACK ;Update state
10$:	RET				;Finished if OK

;Subroutine to process the data in a packet

estab0:	MOVL	ECX, rcv_tcplen[EBP]
	MOVZBL	EAX, tcp_offset[EDX]	;Get length of TCP header
	SHRL	EAX, #2
	ANDL	EAX, #0x0FFFFFFFC
	SUBL	ECX, EAX		;Get length of data part
	JLE	ignore			;Finished with this packet if no data
estab2:	CMPL	ECX, dcb_tcprcvwinb[EDI] ;Can we take this much data now?
	JA	16$			;No
	MOVZBL	EDX, npb_apofs[EBX]
	ADDL	EDX, EBX
	MOVL	EAX, dcb_tcpfuncdisp[EDI] ;Continue
	JMPIL	CS:tcpf_estab[EAX]

;Here we cannot accept the entire packet - If it is a one-byte window probe
;  we send back an ACK which ACKs nothing, otherwise we quietly discard it

16$:	DECL	ECX			;Does it contain 1 byte of data?
	JE	18$			;Yes - this is a normal window probe
	MOVL	ECX, dcb_ipstpdb[EDI]	;No - count the flow control overrun
	INCL	tpdb_tcpcntflowovr[ECX]
18$:	CALL	xosnetGiveBufr##	;Give up the packet
	JMP	tcpsendackf#		;Send an ACK
.PAGE
;Standard input packet processing for ESTAB state for TCP
;	c{EBX} = Offset of packet bufrer
;	c{EDX} = Offset of start of data
;	c{ECX} = Size of data

tcpestab::
	CMPL	dcb_tcprcvputb[EDI], #0 ;Do we have any receive packets?
	JNE	2$			;Yes
	MOVL	dcb_tcprcvtakb[EDI], EBX ;No - just save this one
	MOVL	dcb_tcprcvtakp[EDI], EDX
	MOVL	dcb_tcprcvtakc[EDI], ECX
	MOVL	dcb_tcprcvcnt[EDI], ECX
	JMP	8$

;Here if have at least one receive packet buffered

2$:	CMPL	dcb_tcprcvavl[EDI], ECX	;Can we merge packets?
	JB	6$			;No - go link in this packet

	JMP	6$			;;;;;;;;;;;

	MOVL	EAX, dcb_ipstpdb[EDI]	;Yes - get offset of the TPDB
	INCL	tpdb_tcpcntmerge[EAX]	;Count the merged packet
	ADDL	dcb_tcprcvcnt[EDI], ECX ;Advance pointers and counts
	MOVL	EAX, dcb_tcprcvputb[EDI]

;;;;;;;qqqqqqqqqqqqqq

	ADDL	npb_count[EAX], ECX

	SUBL	dcb_tcprcvavl[EDI], ECX
	MOVL	EAX, dcb_tcprcvputp[EDI]
	ADDL	dcb_tcprcvputp[EDI], ECX
	PUSHL	EDI
	PUSHL	ESI
	PUSHL	ECX
	MOVL	EDI, EAX
	PUSHL	DS
	POPL	ES
	MOVL	ESI, EDX
	CLD
	CMPL	ECX, #8			;More than 8 bytes to store?
	JBE	4$			;No
	MOVL	EAX, ECX		;Yes - get destination long alligned
	MOVL	ECX, EDI
	NEGL	ECX
	ANDL	ECX, #0x03
	SUBL	EAX, ECX
	RMOVSB	[EDI], [ESI]
	MOVL	ECX, EAX
	SHRL	ECX, #2			;Copy longs
	RMOVSL	[EDI], [ESI]
	MOVL	ECX, EAX
	ANDL	ECX, #0x03
4$:	RMOVSB	[EDI], [ESI]		;Copy any bytes left over

;;;;;qqqqqq

	POPL	ECX
	POPL	ESI
	POPL	EDI
	CALL	xosnetGiveBufr##	;Give up the packet
	JMP	10$

;Here if cannot merge packets - link this packet to the list

$$$$ppp::
6$:	MOVL	EAX, dcb_tcprcvputb[EDI]
	MOVL	npb_next[EAX], EBX
	ADDL	dcb_tcprcvcnt[EDI], ECX

8$:	CLRL	EAX
	MOVL	npb_next[EBX], EAX
	MOVL	dcb_tcprcvputb[EDI], EBX

	ADDL	EDX, ECX
	MOVL	dcb_tcprcvputp[EDI], EDX

	MOVZBL	EAX, npb_sizex[EBX]	;Calculate amount left in packet
	MOVL	EAX, knlXmbSize##[EAX*4]
	LEAL	EAX, -npb_mSIZE[EAX+EBX]
	SUBL	EAX, EDX
	MOVL	dcb_tcprcvavl[EDI], EAX
10$:	ADDL	dcb_tcprcvsnum[EDI], ECX ;Bump receive sequence number
	SUBL	dcb_tcprcvwinb[EDI], ECX ;Reduce receive window size
	CALL	xostcpSendAck#		;Send ACK
	CLRL	EAX
	JMP	xosnetRspDone##



#endif
		}
		else if (dcb->tcpstate == TCPS_FINWAIT1 || // FIN sent, waiting for FIN
												   //   in reply and for ACK?
				dcb->tcpstate == TCPS_FINWAIT2)	// ACK for FIN received, still
		{										//   waiting for FIN?
#if 0

;Here with received data packet after we have sent a FIN - get here for both
;  FINWAIT1 and FINWAIT2 states

rcvfinwait:
	CALL	procpkt			;Do preliminary packet processing
	JC	2$			;If can not continue with packet
	MOVL	EAX, dcb_tcpouttake[EDI] ;Anything left unACKed?
	CMPL	EAX, dcb_tcpoutack[EDI]
	JNE	4$			;Yes
	MOVB	dcb_tcpstate[EDI], #TCPS_FINWAIT2 ;No - update state
4$:	MOVZBL	EAX, tcp_offset[EDX]	;Get length of TCP header
	SHRL	EAX, #2
	ANDB	AL, #0FCh
	SUBL	ECX, EAX		;Get length of data part
	PUSHL	tcp_code[EDX]		;Save code bits from the packet
	PUSHL	ECX
	CALL	xosnetGiveBufr##	;Finished with the packet
	POPL	ECX
	TESTB	[ESP], #TCPC$FIN	;Does it have FIN set?
	JE	6$			;No
	INCL	ECX			;Yes
6$:	TESTL	ECX, ECX		;Have any data bytes or FIN?
	JE	8$			;No - don't send an ACK
	ADDL	dcb_tcprcvsnum[EDI], ECX ;Bump receive sequence number
	CALL	tcpsendacknow#		;Send ACK immediately
8$:	POPL	EAX			;Restore code bits from the packet
	TESTB	AL, #TCPC$FIN		;Was this packet a FIN?
	JE	ret012			;No - finished here
	CMPB	dcb_tcpstate[EDI], #TCPS_FINWAIT1 ;Have we received the ACK?
	MOVB	dcb_tcpstate[EDI], #TCPS_CLOSING ;Assume we have not
	JE	ret012			;Right - just wait for the ACK
timew:	MOVB	dcb_tcpstate[EDI], #TCPS_TIMEWAIT ;Wrong - all finished
	CMPL	dcb_opencnt[EDI], #0	;Is anyone using this DCB?
	JNE	wakeall		;Yes
givedcb:MOVL	EAX, dcb_sdisp[EDI]	;No - give up the DCB
	JMPIL	CS:sd_cleardcb[EAX]

wakeall:CLRL	EAX
	CALL	knlResumeInput##	;Wake him up if he's waiting for
	CLRL	EAX			;  anything
	JMP	knlResumeOutput##

#endif



		}
		else if (dcb->tcpstate == TCPS_CLOSING) // Reply FIN received without
												//   ACK, waiting for ACK for
		{										//   the FIN?
#if 0

;Here with received data packet after we have received a reply FIN without an
;  ACK - this should be the ACK for the FIN we initially sent

rcvclosing:
	CALL	procpkt			;Do preliminary packet processing
	JC	ret012			;If can not continue with packet
	MOVL	EAX, dcb_tcpouttake[EDI] ;Anything left unACKed?
	CMPL	EAX, dcb_tcpoutack[EDI]
	JE	timew			;No - all finished now
ret012:	RET

#endif

		}
		else
		{
			// Here if FIN has been received, now waiting for process to close
			//   the TCP device
#if 0

;Here with packet after we have received a FIN but before we have sent a
;  FIN in reply

rcvclosewait:
	CALL	procpkt			;Do preliminary packet processing
	JC	ret006			;If can not continue with packet
	JMP	ignore			;Otherwise just ignore it

#endif

		}

	}
	else
	{
		// Here if we are cleaning up after a connection




		if (dcb->tcpstate == TCPS_LASTACK) // Reply FIN sent, waiting for final ACK
		{
#if 0

;Here with packet after we have sent a reply FIN and are waiting for the
;  last ACK

rcvlastack:
	TESTB	tcp_code[EDX], #TCPC$SYN ;Is this a SYN packet?
	JNE	8$			;Yes - this must mean we lost the final
					;  ACK and he's trying to set up a new
					;  connection - we treat this just like
					;  the final ACK but otherwise ignore
					;  it - he should retransmit the SYN in
					;  a few seconds and it should all work
					;  right then!
	MOVL	EAX, tcp_seqnum[EDX]	;No - get sequence number from packet
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	CMPL	EAX, dcb_tcprcvsnum[EDI] ;Is this what we are expecting?
	JNE	10$			;No - ignore it
	TESTB	tcp_code[EDX], #TCPC$ACK|TCPC$RST ;Is this an ACK or a reset?
	JE	12$			;No - ignore it
2$:	MOVB	dcb_tcpstate[EDI], #TCPS_CLOSED ;Yes - indicate idle
	CMPL	dcb_opencnt[EDI], #0	;Is this DCB in use?
	JE	4$			;No
	CALL	wakeall			;Yes - wake up anyone waiting
	CALL	xosnetGiveBufr##
	RET

;Here if DCB is not in use

4$:	PUSHL	EBX
	CALL	givedcb			;Free up the DCB
	POPL	EBX
	CALL	xosnetGiveBufr##	;Give up the packet buffer
ret006:	RET

;Here for SYN when expecting final ACK

8$:	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the out of sequence packet
	INCL	tpdb_tcpcntoutseq[EAX]
	JMP	2$

;Here for out of sequence packet

10$:	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the out of sequence packet
	INCL	tpdb_tcpcntoutseq[EAX]
	CALL	xosnetGiveBufr##	;Give up the packet buffer
	RET

12$:	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the packet with no ACK
	INCL	tpdb_tcpcntnoack[EAX]
	CALL	xosnetGiveBufr##	;Give up the packet buffer
	RET

#endif
		}
		else							// if TIMEWAIT
		{
			xosnetGiveNpb(npb);			// All finished - timing out port
			return (TRUE);				// Ignore the packet
		}
	}

	// When get here we have processed the input packet and still have a
	//   connection - Now handle possible changes to our send window.

#if 0

/// ????	ANDB	dcb_tcpsts1[EDI], #~{TCPS1$ACKED|TCPS1$OPENED}

	MOVZBL	EAX, dcb_tcpstate[EDI]	;Get connection state
	CALLI	CS:rcvdsp[EAX*4]	;Dispatch on the state
	LEAVE				;Give up the rcv_ frame
	TESTL	EDI, EDI		;Still have a DCB?
	JE	20$			;No - finished now
	TESTB	dcb_tcpsts1[EDI], #TCPS1$ACKED|TCPS1$OPENED
					;Yes - did we change the window or ACK
					;  anything?
	JE	20$			;No
					;Yes - fall into chksend on next page
.PAGE
;Here with ACK or window change

chksend:CALL	xostcpTimerRmv#		;Stop the retransmit timer
	ANDB	dcb_tcpsts2[EDI], #~TCPS2$REXMIT ;No longer retransmitting
	MOVB	dcb_tcpretrycnt[EDI], #0 ;Reset retransmit counter
	CMPL	dcb_tcpoutpkt+npb_nextsnd[EDI], #-1 ;Is our packet being
						    ;  output now?
	JNE	2$			;Yes
	CALL	xostcpCheckSend#	;No - start output if we need to
2$:	MOVL	EAX, dcb_tcpouttake[EDI] ;Have any unACKed data?
	CMPL	EAX, dcb_tcpoutack[EDI]
	JE	6$			;No
	CALL	tcprexmitreq#		;Yes - restart the timer
6$:	MOVL	EAX, dcb_tcpoutput[EDI]	;Is there any space in the output
	SUBL	EAX, dcb_tcpoutack[EDI]	;  buffer?
	JGE	8$
	ADDL	EAX, dcb_tcpoutsize[EDI]
8$:	JE	10$			;No
	CLRL	EAX			;Yes
	MOVL	ECX, dcb_tcpfuncdisp[EDI] ;Wake up application to send more
	JMPIL	CS:tcpf_needout[ECX]	  ;  data

10$:	RET

/////////////////////////////////////
//// END OF TCPRECVPKT
/////////////////////////////////////

#endif

	return (TRUE);
}

#if 0

;Here if need to resend a SYN-ACK for a type 2 passive open

;;;; TIMER FUNCTION

synacktimeout:
	MOVL	dcb_tcprxmtwake[EDI], #0
	DECB	dcb_tcpretrycnt[EDI]	;Should we send it again?
	JS	4$
	CALL	xostcpSendSynAck#	;Yes
	MOVL	EAX, #6000t		;Start a timer for retransmitting the
	MOVL	EBX, #synacktimeout	;  SYN-ACK
	CALL	xostcpTimerReq#
	JNC	ret004
4$:	MOVL	EAX, #ER_NORSP		;No - fail
	CMPL	dcb_tcpconbase[EDI], #0	;Is this a type 2 open?
	JE	xosnetSndDone##		;No - this will cause the application's
					;  open to fail which will let him know
					;  there was a problem. Other than
					;  to possibly log the error, the
					;  the application should not care.
	JMP	tcptype2ready		;Yes
.PAGE

;Subroutine to process the ACK in a received packet
;	CALL	procack

procack:

	CLRL	EAX
	CALL	putinxxx#

	MOVL	EAX, tcp_acknum[EDX]	;Get ACK number
	XCHGB	AL, AH			;Fix up the byte order
	RORL	EAX, #16t
	XCHGB	AL, AH	
	MOVL	ECX, dcb_tcpoutsnum[EDI] ;Get number unACKed
	SUBL	ECX, EAX
	JS	ackdone
	CMPL	EAX, dcb_tcprcvanum[EDI] ;Also ignore it if ACKing less than
	JS	ackdone			 ;  has already been ACKed
	CMPL	EAX, dcb_tcprcvanum[EDI] ;Are we really ACKing anything?
	JE	ackdone			;No - finished here
	MOVL	dcb_tcprcvanum[EDI], EAX ;Yes - update last ACK number
	ORB	dcb_tcpsts1[EDI], #TCPS1$ACKED
	MOVL	EDX, dcb_tcpoutsize[EDI]
	MOVL	EAX, dcb_tcpouttake[EDI] ;Update the ACK pointer
	SUBL	EAX, ECX
	JGE	4$
	ADDL	EAX, EDX
4$:	CMPL	EAX, EDX
	JL	5$
	CRASH	BAKP			;[Bad AcK Pointer]

5$:	MOVL	dcb_tcpoutack[EDI], EAX
	CMPL	EAX, dcb_tcpoutput[EDI]	;Have we ACKed everything?
	JNE	6$			;No
	TESTB	dcb_tcpsts2[EDI], #TCPS2$NEEDFIN ;Yes - was a FIN pending?
	JE	6$			;No
	ANDB	dcb_tcpsts2[EDI], #~TCPS2$NEEDFIN ;Yes - but not now
6$:	CMPL	dcb_tcpoutpush[EDI], #-1 ;Do we have a PUSH now?
	JE	caltime			;No
	MOVL	EDX, dcb_tcpoutput[EDI]	;Yes - get distance between the ACK
	SUBL	EDX, EAX		;  pointer and the putter pointer
	JGE	8$
	ADDL	EDX, dcb_tcpoutsize[EDI]
8$:	MOVL	EAX, dcb_tcpoutput[EDI]	 ;Get distance between the PUSH
	SUBL	EAX, dcb_tcpoutpush[EDI] ;  pointer and the putter pointer
	JGE	10$
	ADDL	EAX, dcb_tcpoutsize[EDI]
10$:	CMPL	EAX, EDX		;Has the ACK pointer gone past the
					;  PUSH pointer?
	JL	caltime		;No
	MOVL	dcb_tcpoutpush[EDI], #-1 ;Yes - clear the PUSH pointer
.PAGE
;Here to calculate round-trip time for a packet

caltime:TESTB	dcb_tcpsts2[EDI], #TCPS2$REXMIT ;Are we retransmitting
	JE	4$			;No

;Here if we are retransmitting - leave the average value unchanged but double
;  the interval value we are using (Kern algorithm)

	SHLL	dcb_tcprxmtint[EDI], #1
	CMPL	dcb_tcprxmtint[EDI], #TIMEOUT_TRNMAX/TICKPERSP ;Too big?
	JL	4$			;No
	MOVL	dcb_tcprxmtint[EDI], #TIMEOUT_TRNMAX/TICKPERSP ;Yes

;Here if not retransmitting - calculate the round-time normally using the
;  Jacobson algorithm

4$:	TESTB	dcb_tcpsts1[EDI], #TCPS1$TIMING ;Are we timing now?
	JE	ackdone			;No - go on
	MOVL	EAX, dcb_tcprcvanum[EDI] ;Yes - did we ACK the packet we are
	SUBL	EAX, dcb_tcprndsnum[EDI] ;  timing?
	JS	ackdone			;No
	ANDB	dcb_tcpsts1[EDI], #{~TCPS1$TIMING&0FFh} ;No longer timing
	MOVL	EAX, knlSchTtl##	;Get round-trip time for packet
	SUBL	EAX, dcb_tcprndbgn[EDI]
	MOVL	dcb_tcprndbgn[EDI], #0
	JG	6$			;Make sure its at least 1!
	MOVL	EAX, #1t
6$:
.IF NE $$TCPTIME
	MOVL	tcptimetemp, EAX
.ENDC

;Here with the round-trip time for the packet - now calculate the error value:
;	error = time - average
;  Note that the average (dcb_tcprxmtave) is scaled by 8 here.

	SHLL	EAX, #3t
	SUBL	EAX, dcb_tcprndave[EDI] ;Calculate error value

;Now update the average using the error value (note that we use an unbalanced
;  weighting factor - 1/8 if decreasing and 1/2 if increasing)

	JNS	10$			;If increasing, use t = 2, if decreasing
;;;;;;;	SARL	EAX, #2t		;  use t = 8
10$:	SARL	EAX, #1t
	ADDL	dcb_tcprndave[EDI], EAX ;Update average
	CMPL	dcb_tcprndave[EDI], #{TIMEOUT_TRNMAX/TICKPERSP}*12t ;Too big?
	JBE	12$			;No
	MOVL	dcb_tcprndave[EDI], #{TIMEOUT_TRNMAX/TICKPERSP}*12t ;Yes

;Now calculate the new variance (change in the error value).  Note that the
;  average variance value (dcb_tcprxmtvar) is scaled by 4 here.

12$:	TESTL	EAX, EAX		;Get magnitude of the error value
	JNS	14$
	NEGL	EAX
14$:	SHLL	EAX, #2t
	SUBL	EAX, dcb_tcprndvar[EDI]
	SARL	EAX, #2t
	ADDL	dcb_tcprndvar[EDI], EAX ;Update the average variance value
	CMPL	dcb_tcprndvar[EDI], #{TIMEOUT_TRNMAX/TICKPERSP}*4t ;Too big?
	JBE	16$			;No
	MOVL	dcb_tcprndvar[EDI], #{TIMEOUT_TRNMAX/TICKPERSP}*4t ;Yes

;Now calculate the retranmission time-out value.  This is
;	timeout = 2 * (average + variance) + 10
;  Note that the average is scaled by 8 and the variance is scaled by 4 here.
;  The extra additive term is used in place of a test for a minimum value.

16$:	MOVL	EAX, dcb_tcprndave[EDI]
	SHRL	EAX, #1
	CMPL	dcb_tcprndvar[EDI], EAX ;Is the variance too big?
	JBE	18$			;No - go on
	MOVL	dcb_tcprndvar[EDI], EAX ;Yes - reduce it
18$:	ADDL	EAX, dcb_tcprndvar[EDI]
	SHRL	EAX, #1
	ADDL	EAX, #10t
	CMPL	EAX, #TIMEOUT_TRNMAX/TICKPERSP ;Too big?
	JB	20$			;No
	MOVL	EAX, #TIMEOUT_TRNMAX/TICKPERSP ;Yes - just use the maximum
	MOVL	dcb_tcprndave[EDI], #{TIMEOUT_TRNMAX/TICKPERSP}*2
	MOVL	dcb_tcprndvar[EDI], #0
20$:	MOVL	dcb_tcprxmtint[EDI], EAX ;Update retransmition interval value

.IF NE $$TCPTIME
	PUSHL	EDX
	MOVL	EDX, tcptimepnt
	MOVL	EAX, knlSchTtl##
	MOVL	[EDX], EAX
	MOVL	EAX, tcptimetemp
	MOVW	4[EDX], AX
	MOVL	EAX, dcb_tcprxmtint[EDI]
	MOVW	6[EDX], AX
	MOVL	EAX, dcb_tcprndave[EDI]
	ADDL	EAX, EAX
	MOVW	8[EDX], AX
	MOVL	EAX, dcb_tcprndvar[EDI]
	SHLL	EAX, #2t
	MOVW	10t[EDX], AX
	MOVL	12t[EDX], #0
	ADDL	EDX, #16t
	CMPL	EDX, #tcptimeend
	JB	22$
	MOVL	EDX, #tcptimering
22$:	MOVL	tcptimepnt, EDX
	POPL	EDX
.ENDC
.PAGE
;Here when finished processing ACKs

ackdone:MOVL	EDX, rcv_tcphdr[EBP]	;Restore offset of the TCP header in
					;  the received packet
	MOVZWL	EAX, tcp_window[EDX]
	XCHGB	AL, AH			;Get window value from the packet
	CMPL	dcb_tcpoutwinm[EDI], EAX ;New maximum window size?
	JGE	4$			;No
	MOVL	dcb_tcpoutwinm[EDI], EAX ;Yes - remember it
4$:	MOVL	ECX, dcb_tcpouttake[EDI] ;Adjust for unACKed data
	SUBL	ECX, dcb_tcpoutack[EDI]
	JGE	6$
	ADDL	ECX, dcb_tcpoutsize[EDI]
6$:	SUBL	EAX, ECX
	CMPL	dcb_tcpoutwina[EDI], EAX ;Has the window changed?
	JE	ret020			;No
	ORB	dcb_tcpsts1[EDI], #TCPS1$OPENED ;Yes - indicate that
	MOVL	dcb_tcpoutwina[EDI], EAX ;Store updated window size

	MOVL	EAX, #1
	CLRL	ECX
	CALL	putinxxx#
	CLC

ret020:	RET				;Finished here
.PAGE
	.SBTTL	xostcpOpenWindow - Subroutine to open the receive window

;Subroutine to open the receive window
;	CALL	xostcpOpenWindow
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal
;	  EAX and ECX are preserved

xostcpOpenWindow:
	PUSHL	EAX
	MOVL	EAX, dcb_tcprcvwinb[EDI] ;Get amount window has changed since
	SUBL	EAX, dcb_tcprcvwina[EDI] ;  we last announced it
	ADDL	EAX, dcb_tcprcvsnum[EDI] ;Allow for pending but unsent ACKS
	SUBL	EAX, dcb_tcprcvtnum[EDI]
	JL	2$			;Always update if smaller (this should
					;  normally not be the case!)
	SHLL	EAX, #2			;Changed by more than 1/4?
	CMPL	EAX, dcb_tcprcvwinm[EDI]
	JB	8$			;No - don't do it yet!
2$:	PUSHL	ECX			;Yes
	MOVL	ECX, dcb_tcprcvoosl[EDI] ;Do we have any out-of-sequence
	TESTL	ECX, ECX		 ;  packets?
	JE	4$			;No
	MOVL	EAX, npb_seqnum[ECX]	 ;Yes - is the first one the next one
	CMPL	EAX, dcb_tcprcvsnum[EDI] ;  we want?
	JE	6$			;Yes - don't send window update now
4$:	CALL	tcpsendacknow#		;No - send window update
6$:	POPL	ECX
8$:	POPL	EAX
	CLC				;Always return with C clear
ret050:	RET
.PAGE
	.SBTTL	xostcpGetLong - Subroutine to get received data long

;Subroutine to get received data long - must be called in extended fork
;  context
;	c{EDI} = Offset of DCB
;	c{ESI} = Offset of SDB
;	CALL	xostcpGetLong
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal
;	  c{EAX} = Value

xostcpGetLong::
	PUSHL	#0			;Clear value
	CALL	xostcpGetByte		;Get high order byte
	JC	2$
	MOVB	3[ESP], AL		;Store it
	CALL	xostcpGetByte		;Get next byte
	MOVB	2[ESP], AL		;Store it
	JNC	4$			;Continue if OK
2$:	POPL	ECX			;If error
	RET

	.SBTTL	xostcpGetWord - Subroutine to get received data word

;Subroutine to get received data word - must be called in extended fork
;  context
;	c{EDI} = Offset of DCB
;	c{ESI} = Offset of SDB
;	CALL	xostcpGetWord
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal
;	  c{EAX} = Value

xostcpGetWord::
	PUSHL	#0			;Clear value
4$:	CALL	xostcpGetByte		;Get high order byte
	JC	2$
	MOVB	1[ESP], AL		;Store it
	CALL	xostcpGetByte		;Get low order byte
	JC	2$
	MOVB	[ESP], AL		;Store it
	POPL	EAX			;Get value
	RET				;Finished
.PAGE
	.SBTTL	xostcpGetByte - Subroutine to get received data byte

;Subroutine to get received data byte - must be called in extended fork
;  context
;	c{EDI} = Offset of DCB
;	c{ESI} = Offset of SDB
;	CALL	xostcpGetByte
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal
;	  c{EAX} = Data byte (zero extended)

xostcpGetByte::
	CMPL	dcb_tcprcvcnt[EDI], #0	;Have any data available now?
	JNE	4$			;Yes
2$:	CMPB	dcb_tcpstate[EDI], #TCPS_ESTAB ;No - still have a connection?
	JNE	14$			;No
3$:	CALL	tcprspwait		;Wait until have some data
	JC	xostcpFatal#		;If error
4$:	MOVL	EBX, dcb_tcprcvtakb[EDI] ;Point to receive buffer
	TESTL	EBX, EBX		;Really have a buffer?
	JE	12$			;No
	MOVL	EDX, dcb_tcprcvtakp[EDI] ;Yes - point to receive data
	MOVZBL	EAX, [EDX]		;Get a data byte
	INCL	dcb_tcprcvwinb[EDI]	;Open receive window
	DECL	dcb_tcprcvcnt[EDI]	;Reduce total amount available
	INCL	dcb_tcprcvtakp[EDI]	;Bump pointer
	DECL	dcb_tcprcvtakc[EDI]	;Reduce count for this packet
	JNE	6$			;If still something in this buffer

;Here if current buffer is now empty

tcpget2:PUSHL	npb_next[EBX]		;Give up this buffer
	CALL	xosnetGiveBufr##
	POPL	EBX
	MOVL	dcb_tcprcvtakb[EDI], EBX ;Update our pointer
	TESTL	EBX, EBX		;Have another buffer?
	JNE	10$			;Yes
	MOVL	dcb_tcprcvtakp[EDI], EBX ;No - clear pointers
	MOVL	dcb_tcprcvputb[EDI], EBX
	MOVL	dcb_tcprcvputp[EDI], EBX
	CMPL	dcb_tcprcvcnt[EDI], #0	;Make sure count is 0 (for debugging!)
	JNE	8$
6$:	RET

8$:	CRASH	BNRC			;[Bad Network Receive byte Count]

;Here if have another buffer

10$:	MOVZBL	EDX, npb_apofs[EBX]	;Get offset of start of data
	MOVL	ECX, npb_count[EBX]
	SUBL	ECX, EDX
	MOVL	dcb_tcprcvtakc[EDI], ECX
	ADDL	EDX, EBX
	MOVL	dcb_tcprcvtakp[EDI], EDX ;Update taker data pointer
	RET				;Finished

;Here if don't have a packet

12$:	CMPB	dcb_tcpstate[EDI], #TCPS_ESTAB ;Still have a connection?
	JE	2$			;Yes - continue trying
14$:	MOVL	EAX, #ER_NCLST		;No - report the lost connection
	CMPB	dcb_tcpstate[EDI], #TCPS_CLOSEWAIT
	JNE	16$
	MOVL	EAX, #ER_NCCLR
16$:	STC
	RET
.PAGE
	.SBTTL	xostcpGetBlk - Subroutine to get block of received data

;Subroutine to get block of received data - must be called in extended fork
;  context
;	c{EAX,EDX} = Time-out amount
;	c{ES:EBX}  = Address of buffer to receive data
;	c{ECX}     = Amount wanted
;	c{EDI}     = Offset of DCB
;	c{ESI}     = Offset of SDB
;	CALL	xostcpGetBlk
;	C:set = Error
;	  c{EAX} = Error code
;	  c{ECX} = Amount obtained
;	C:clr = Normal
;	  c{ECX} = Amount obtained

$$$=!0
FRM tgb_buffer , 8
FRM tgb_count  , 4
FRM tgb_amount , 4
FRM tgb_timeout, 8
tgb_SIZE=!$$$

xostcpGetBlk::
	ENTER	tgb_SIZE, 0
	MOVL	tgb_timeout+0[EBP], EAX
	MOVL	tgb_timeout+4[EBP], EDX
	MOVL	tgb_buffer+0[EBP], EBX
	MOVL	tgb_buffer+4[EBP], ES
	MOVL	tgb_count[EBP], ECX
	CLRL	EAX			;Clear amount transfered
	MOVL	tgb_amount[EBP], EAX
	CMPL	dcb_tcprcvcnt[EDI], #0	;Have any data available now?
	JNE	4$			;Yes
2$:	CMPB	dcb_tcpstate[EDI], #TCPS_ESTAB ;No - still have a connection?
	JNE	6$			;No
	MOVL	EAX, tgb_timeout+0[EBP]	;Yes
	MOVL	EDX, tgb_timeout+4[EBP]	;Yes
	CALL	xosnetRspWait##		;Wait for data
	JC	8$
4$:	MOVL	EBX, dcb_tcprcvtakb[EDI] ;Point to receive buffer
	TESTL	EBX, EBX		;Do we really have one?
	JNE	10$			;Yes
	CMPB	dcb_tcpstate[EDI], #TCPS_ESTAB ;No - still have a connection?
	JE	2$			;Yes - continue trying
6$:	MOVL	EAX, #ER_NCLST		;No - report the lost connection
	CMPB	dcb_tcpstate[EDI], #TCPS_CLOSEWAIT
	JNE	8$
	MOVL	EAX, #ER_NCCLR
8$:	CLRL	ECX
	LEAVE
	STC
	RET

;Here with a packet

10$:	MOVL	ECX, tgb_count[EBP]	;Yes - get amount wanted
	CMPL	ECX, dcb_tcprcvtakc[EDI] ;Do we have this much in this buffer?
	JBE	12$			;Yes
	MOVL	ECX, dcb_tcprcvtakc[EDI] ;No - get amount we do have here
12$:	PUSHL	EDI			;Save registers we need here
	PUSHL	ESI
	PUSHL	ECX			;Save amount to transfer
	MOVL	ESI, dcb_tcprcvtakp[EDI] ;Point to data in our buffer
	LESL	EDI, tgb_buffer[EBP]	;Point to user's buffer
	CLD
14$:	TESTL	EDI, #03		;Need byte first?
	JE	16$			;No
	MOVSB	[EDI], [ESI]		;Yes
	DECL	ECX
	JNE	14$
	JMP	18$

16$:	MOVL	EAX, ECX		;Remember low order count bits
	SHRL	ECX, #2			;Change to number of longs
	RMOVSL	[EDI], [ESI]		;Transfer most of the block
	MOVL	ECX, EAX		;Transfer any bytes left over
	ANDL	ECX, #3
	RMOVSB	[EDI], [ESI]
18$:	POPL	ECX			;Restore registers
	POPL	ESI
	POPL	EDI
	ADDL	tgb_buffer+0[EBP], ECX	;Bump user buffer address
	SUBL	tgb_count[EBP], ECX	;Reduce amount wanted
	ADDL	tgb_amount[EBP], ECX	;Increase amount transfered
	ADDL	dcb_tcprcvtakp[EDI], ECX ;Bump taker data pointer
	SUBL	dcb_tcprcvcnt[EDI], ECX ;Reduce total amount available
	ADDL	dcb_tcprcvwinb[EDI], ECX ;Open receive window
	SUBL	dcb_tcprcvtakc[EDI], ECX ;Reduce amount in this buffer
	JNE	22$			;If something left we must be finished
	CALL	tcpget2			;Buffer empty - give up the buffer
	CMPL	tgb_count[EBP], #0	;Do we want more?
	JE	22$			;No - finished
	CMPL	dcb_tcprcvcnt[EDI], #0	;Yes - have any more data available?
	JNE	4$			;Yes - go get it
22$:	MOVL	ECX, tgb_amount[EBP]	;No - get amount transfered
	LEAVE
	RET				;Finished
.PAGE
	.SBTTL	tcprspwait - Subroutine to wait for TCP response

;Subroutine to wait for TCP response - this routine attempts to use a time-out
;  value that is the minimum which will allow 5 or 6 retransmissions if
;  necesary with an absolute minimum value of 20 seconds.  This is hard to do
;  exactly without excessive calculation bacause of the non-linear nature of
;  the retransmission timing scheme.  We use the following formula (times in
;  seconds):
;    RTT  < 1:  TO = RTT * 20
;    RTT >= 1:  TO = 20 + (X - 1) * 5
;	c{EDI} = Offset of DCB
;	CALL	tcprspwait
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal

tcprspwait::
	MOVL	EAX, dcb_tcprxmtint[EDI] ;Get the retransmission interval (in
					 ;  scheduler periods)
	MULL	knlLitTICKPERSP##	;Change to system ticks
	CMPL	EAX, #TICKPERSEC	;Less than 1 second?
	JA	4$			;No
	IMULL	EAX, #20t		;Yes
	JMP	6$

;Here if retransmission time is greater than 1 second - use a smaller slope
;  so this time-out does not get to be too large

4$:	ADDL	EAX, #3t*TICKPERSEC
	IMULL	EAX, #5t
6$:	CMPL	EAX, #20t*TICKPERSEC
	JB	8$
	MOVL	EAX, #20t*TICKPERSEC
8$:	CLRL	EDX
	JMP	xosnetRspWait##
.PAGE
	.SBTTL	tcpoas - Once-a-second routine

tcpoas::MOVL	ESI, #tcptwdcb#-dcb_tcptwnext ;Get offset of head pointer
4$:	MOVL	EDI, dcb_tcptwnext[ESI]
	TESTL	EDI, EDI		;More to check?
	JE	10$			;No
	TOFORK
	DECB	dcb_tcptwcnt[EDI]	;Yes - is this one time out yet?
	JNE	5$			;No
	CMPB	dcb_tcpstate[EDI], #TCPS_TIMEWAIT ;In timed wait state now?
	JE	6$			;Yes
	MOVB	dcb_tcpstate[EDI], #TCPS_TIMEWAIT ;No - but it is now
	MOVB	dcb_tcptwcnt[EDI], #TIMECNT_TW
5$:	FROMFORK			;Back to main program level
	MOVL	ESI, EDI		;Advance to next DCB
	JMP	4$			;Continue

6$:	MOVL	EAX, dcb_tcptwnext[EDI]	;Remove from list
	MOVL	dcb_tcptwnext[ESI], EAX
	PUSHL	ESI
	CALL	tcpgdcb#		;Give up the DCB
	POPL	ESI
	FROMFORK
	JMP	4$

10$:	RET

#endif
