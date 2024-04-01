	.TITLE	tcpcls3 - TCP routines for XOS (part 2)

;++++
; This software is in the public domain.  It may be freely copied and used
; for whatever purpose you see fit, including commerical uses.  Anyone
; modifying this software may claim ownership of the modifications, but not
; the complete derived code.  It would be appreciated if the authors were
; told what this software is being used for, but this is not a requirement.

;   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
;   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
;   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
;   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
;   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
;   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
;   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
;   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
;   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
;   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;----

	.PROC	80486
	.INCLUD	XOSINC:\XMAC\XOS.PAR
	.INCLUD	XOSINC:\XMAC\XOSX.PAR
	.INCLUD	XOSINC:\XMAC\XOSTIME.PAR
	.INCLUD	XOSINC:\XMAC\XOSERR.PAR
	.INCLUD	XOSINC:\XMAC\XOSNET.PAR
	.INCLUD	XOSINC:\XMAC\XOSXNET.PAR
	.INCLUD	XOSINC:\XMAC\XOSXIPS.PAR
	.INCLUD	XOSINC:\XMAC\XOSXTCP.PAR
	.INCLUD	XOSINC:\XMAC\XOSLKE.PAR
	.INCLUD	XOSINC:\XMAC\XOSXLKE.PAR

	LKECONT

;This file contains routines for receiving packets

	.EXPORT	xostcpWaitWin
	.EXPORT	xostcpFatal
	.EXPORT	xostcpGiveAll
	.EXPORT	xostcpHashClr
	.EXPORT	xostcpHashSet
	.EXPORT	xostcpPutBlk
	.EXPORT	xostcpSendAck
	.EXPORT	xostcpSendFin
	.EXPORT	xostcpSendReset
	.EXPORT xostcpTimerReq
	.EXPORT	xostcpTimerRmv
	.EXPORT	xostcpPush

	.SBTTL	xostcpSendReset - Subroutine to send a RST packet

;Subroutine to send a RST packet

xostcpSendReset::
	MOVL	ECX, #60t
	CALL	xosnetGetBufr##		;Get a network buffer
	JC	2$			;Forget it if can't get one
	CALL	tcpsetpkt		;Set up the packet header
	JC	6$
	MOVZBL	ECX, npb_tpofs[EBX]
	MOVL	EAX, dcb_tcpoutsnum[EDI]
	MOVL	npb_seqnum[EBX], EAX
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	MOVL	tcp_seqnum[EBX+ECX], EAX
	CLRL	EAX
	MOVL	tcp_acknum[EBX+ECX], EAX
	MOVW	tcp_window[EBX+ECX], AX
	ORB	tcp_code[EBX+ECX], #TCPC$RST ;Make this a reset packet
	CALL	tcpchksum		;Calculate the checksum
	CALL	xosipsSendSetup##
	MOVL	ESI, dcb_netsdb[EDI]
	MOVL	EAX, #xosnetGiveBufr##
	CALL	xossnpSendPkt##		;Send the packet and then give it up
	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the reset
	INCL	tpdb_tcpcntrstsent[EAX]
	CLC
2$:	RET

6$:	CALL	xosnetGiveBufr##
	STC
	RET
.PAGE
	.SBTTL	xostcpSendFin - Subroutine to send a FIN packet

;Subroutine to send a FIN packet

xostcpSendFin::				;Must be called in extended fork context
	ORB	dcb_tcpsts2[EDI], #TCPS2$NEEDFIN ;Request a FIN
	BTZL	dcb_netmode[EDI], #NMTCP%CONSEND ;Disable Nagel
	BTSL	dcb_netmode[EDI], #NMTCP%PUSH ;Push it
	MOVL	EBX, #dummy
	PUSHL	CS
	POPL	ES
	MOVL	ECX, #1
	JMP	tcpputblk2

dummy:	.BYTE	0
.PAGE
	.SBTTL	xostcpSendAck - Subroutine to send ACK packet

;Subroutine to send ACK packet - This subroutine inplements delayed ACKs. It
;  will delay at most 1 ACK.
;	CALL	xostcpSendAck

xostcpSendAck::
	MOVL	EAX, dcb_tcprcvsnum[EDI] ;Do we really need an ACK now?
	CMPL	dcb_tcprcvtnum[EDI], EAX
	JE	2$			;No
tcpsendackf::				;Yes
	CMPL	dcb_tcpackwake[EDI], #0	;Is the ACK timer running now?
	JNE	10$			;Yes - must send the ACK now
	MOVL	EBX, #tcpacktimeout	;No - start the timer
	MOVL	EAX, #TIMEOUT_ACK/TICKPERSP
	PUSHL	ESI
	CALL	knlWakeRequest##
	JC	4$			;If error starting timer
	MOVL	dcb_tcpackwake[EDI], ESI ;OK - finished for now
	POPL	ESI
2$:	RET

;Here if can't enter timer request - just send the ACK now

4$:	POPL	ESI
	JMP	tcpsendacknow

;Here when the ACK timer runs out

tcpacktimeout:
	CMPL	ESI, dcb_tcpackwake[EDI]
	JNE	6$
	CLRL	EAX
	MOVL	dcb_tcpackwake[EDI], EAX
	CALL	tcpsendacknow
	CLC
	RET

6$:	CRASH	BWKB

;Here if currently delaying an ACK - stop the timer and send an ACK now

10$:	MOVL	ESI, dcb_tcpackwake[EDI] ;Stop the timer
	MOVL	dcb_tcpackwake[EDI], #0
	CALL	knlWakeRemove##

;Here to send an ACK now

tcpsendacknow::
	MOVL	ECX, #60t
	CALL	xosnetGetBufr##		;Get a network buffer
	JC	2$			;Forget it if can't get one
	CALL	tcpsetpkt		;Set up the packet header
	JNC	18$
	CALL	xosnetGiveBufr##
	RET

18$:	MOVZBL	ECX, npb_tpofs[EBX]
	MOVL	EAX, dcb_tcpoutsnum[EDI]
	MOVL	npb_seqnum[EBX], EAX
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	MOVL	tcp_seqnum[EBX+ECX], EAX
	CALL	tcpsetack		;Store ACK number and window size in
	CALL	xosipsSendSetup##	;  packet
	MOVL	ESI, dcb_netsdb[EDI]
	MOVL	EAX, #xosnetGiveBufr##
	CALL	xossnpSendPkt##		;Send the packet and then give it up
	RET
.PAGE
	.SBTTL	tcpsetpkt - Subroutine to set up the TCP packet header

;Subroutine to set up the TCP (and lower) level packet headers
;	c{EBX} = Offset of packet buffer
;	CALL	tcpsetpkt
;	C:set = Error code
;	  c{EAX} = Error code
;	C:clr = Normal
;	  c(AL)  = Value stored in npb_apofs
;	  c{EDX} = Offset of first byte after TCP header
;  EBX is respected

tcpsetpkt::
	CLRL	EAX			;Yes
	MOVL	EDX, #IPP_TCP<8t	;Get transport protocol type
	CALL	xosipsSetPkt##		;Set up IP (and lower) level headers
	JC	4$			;If error
	MOVL	EAX, dcb_ipslclport[EDI] ;Get port numbers
	XCHGB	AL, AH
	MOVL	tcp_srcport[EDX], EAX	;Store in packet
	CLRL	EAX
	MOVL	tcp_acknum[EDX], EAX	;Clear remaining fields
	MOVL	tcp_checksum[EDX], EAX
	MOVL	npb_apcnt[EBX], EAX
	MOVB	AL, #50h		;Set offset field
	MOVL	tcp_offset[EDX], EAX
	MOVB	AL, npb_tpofs[EBX]
	ADDB	AL, #tcp_options
	MOVB	npb_apofs[EBX], AL
	ADDL	npb_count[EBX], #tcp_options ;Update byte counts
	ADDL	EDX, #tcp_options	;Point to start of data area (assuming
4$:	RET				;  no options)
.PAGE
	.SBTTL	xostcpHashSet - Subroutine to place DCB in the TCP hash table

;Subroutine to place DCB in the IP hash table
;	c{EBX} = Hash index
;	c{EDI} = Offset of DCB
;	CALL	xostcpHashSet

xostcpHashSet::
	CMPL	dcb_ipshashnext[EDI], #-1
	JNE	4$
	MOVL	EAX, dcb_ipstpdb[EDI]
	MOVL	EAX, tpdb_tcprcvhash[EAX]
	PUSHL	[EAX+EBX*4]
	POPL	dcb_ipshashnext[EDI]
	MOVL	[EAX+EBX*4], EDI
	RET

4$:	CRASH	IPHE			;[IP Hash Error]
.PAGE
	.SBTTL	xostcpHashClr - Subroutine to remove DCB from the TCP hash table

;Subroutine to remove DCB from the TCP hash table
;	c{EDI} = Offset of DCB
;	CALL	xostcpHashClr

xostcpHashClr:
	CMPL	dcb_ipshashnext[EDI], #-1 ;Still in the hash table?
	JE	ret026			;No - nothing needed here
	CALL	tcpcalhash		;Yes - remove DCB from the hash table
	MOVL	EAX, dcb_ipstpdb[EDI]
	MOVL	EAX, tpdb_tcprcvhash[EAX]	 ;Get address of hash table
	LEAL	EBX, -dcb_ipshashnext[EAX+EBX*4] ;  entry minus off of link
						 ;  in the DCB
2$:	CMPL	dcb_ipshashnext[EBX], EDI ;Is it the next one?
	JE	6$			;Yes
	MOVL	EBX, dcb_ipshashnext[EBX] ;No - advance to next
	TESTL	EBX, EBX		;Have another one to check?
	JNE	2$			;Yes - continue
	CRASH	BNHL			;[Bad Network Hash List]

4$:	CRASH	NIHL			;[Not In Hash List]

;Here with our DCB

6$:	PUSHL	dcb_ipshashnext[EDI]	;Remove from list
	POPL	dcb_ipshashnext[EBX]
	MOVL	dcb_ipshashnext[EDI], #-1
	RET

	.SBTTL	tcpcalhash - Subroutine to calculate TCP hash index from DCB

;Subroutine to calculate TCP hash index from DCB
;	c{EDI} = Offset of DCB
;	CALL	tcpcalhash
;	c{EBX} = Hash index

tcpcalhash::
	PUSHL	ESI
	CLRL	EBX
	MOVL	ECX, #8			;Number of bytes to use
	LEAL	ESI, dcb_ipsrmtaddr[EDI] ;First byte to use
	CLD
10$:	LODSB	[ESI]
	ROLB	BL, #1
	XORB	BL, AL
	LOOP	ECX, 10$
	POPL	ESI
ret026:	RET
.PAGE
	.SBTTL	xostcpChkWin - Subroutine to wait until window is open

;Subroutine to wait until window is open
;	c{EAX,EDX} = Time-out value (system ticks)
;	c{EDI} = Offset of DCB
;	CALL	xostcpWaitWin
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal

;NOTE:	We always allow one more byte than indicated by the window so we
;	will have somthing to send as a window probe.

xostcpWaitWin::
	CMPB	dcb_tcpstate[EDI], #TCPS_ESTAB ;Do we have a connection?
	JE	2$
	CMPB	dcb_tcpstate[EDI], #TCPS_CLOSEWAIT ;Maybe
	JNE	4$			;No - fail
2$:	CMPL	dcb_tcpoutwina[EDI], #0 ;Yes - have any space now?
	JGE	8$			;Yes
	PUSHL	EDX			;No - save time-out value
	PUSHL	EAX
	CMPL	dcb_tcpoutpkt+npb_nextsnd[EDI], #-1
	JNE	3$
	CALL	xostcpCheckSend
3$:
	MOVL	EAX, [ESP]
	MOVL	EDX, 4[ESP]
	CALL	xosnetSndWait##		;Wait for a while
	JC	10$			;If error
	POPL	EAX			;OK - restore time-out value
	POPL	EDX
	JMP	xostcpWaitWin		;Go check again

;Here if do not have a connection

4$:	MOVL	EAX, #ER_NCLST		;Assume have had fatal error
	TESTB	dcb_tcpsts2[EDI], #TCPS2$FATAL ;Right?
	JNE	6$			;Yes
	MOVB	AL, #ER_NCCLR&0FFh	;No - return right error code
6$:	STC
8$:	RET

;Here if error while waiting

10$:	ADDL	ESP, #8			;Discard time-out value
	JMP	xostcpFatal
.PAGE
	.SBTTL	xostcpPutBlk - Subroutine to store output data into a TCP packet

;Subroutine to store output data into the TCP output ring buffer - must be
;  called in extended fork context
;	c{ES:EBX} = Address of buffer containing data
;	c{ECX}    = Amount to output
;	c{EDI}    = Offset of DCB
;	c{ESI}    = Offset of SDB
;	CALL	xostcpPutBlk
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal

$$$=!0
FRM ts_buffer, 8t
FRM ts_count , 4t		;Amount to store in current segment of buffer
FRM ts_left  , 4t		;Total amount to store
ts_SIZE=!$$$


xostcpPutBlk::
	TESTB	dcb_tcpsts2[EDI], #TCPS2$SENTFIN|TCPS2$NEEDFIN
	JE	tcpputblk2
	MOVL	EAX, #ER_NOOUT
	STC
	RET

tcpputblk2:
	ENTER	ts_SIZE, 0
	MOVL	ts_buffer+0[EBP], EBX	;Store buffer address
	MOVL	ts_buffer+4[EBP], ES
	MOVL	ts_left[EBP], ECX	;Store amount to store

;;;	CMPL	ECX, #3000t
;;;	JL	1$
;;;	INT3
;;;1$:

	CMPL	dcb_tcpoutbufr[EDI], #0	;Do we have an output buffer now?
	JNE	tcpputblk4		;Yes - go on
	MOVL	EDX, dcb_tcpoutwinm[EDI] ;No - get the maximum window size
	SUBL	EDX, #100t
	CMPL	EDX, #2048t
	JGE	102$
	MOVL	EDX, #2048t
102$:	CMPL	EDX, #32768t
	JLE	104$
	MOVL	EDX, #32768t
104$:	CALL	knlGetIndex##
	PUSHL	ECX
	CALL	knlGetXmb##
	POPL	ECX
	JC	2$			;If error
	MOVL	dcb_tcpoutbufr[EDI], ESI
	MOVL	EAX, #64t
	SHLL	EAX, CL
	SUBL	EAX, #4
	MOVL	dcb_tcpoutsize[EDI], EAX
	MOVL	dcb_tcpoutpush[EDI], #-1
tcpputblk4:
	CMPL	ts_left[EBP], #0	;Finished?
	JG	6$			;No
2$:	LEAVE				;Yes
	RET

3$:	MOVL	EAX, #ER_NCLST
	LEAVE
	RET

4$:	MOVL	ECX, ts_left[EBP]
6$:	CMPB	dcb_tcpstate[EDI], #TCPS_ESTAB ;No - do we have a connection?
	JE	8$
	CMPB	dcb_tcpstate[EDI], #TCPS_CLOSEWAIT ;Maybe
	JNE	3$			;No - fail
8$:	MOVL	ECX, dcb_tcpoutsize[EDI]
	MOVL	EAX, dcb_tcpoutput[EDI] ;Get number of bytes available
	SUBL	EAX, dcb_tcpoutack[EDI]
	JGE	10$
	ADDL	EAX, ECX
10$:	DECL	ECX
	SUBL	ECX, EAX
	JG	16$			;If have some space available

;Here if no space is currently available in the output buffer

	CALL	xostcpCheckSend		;Start output if need to
	JC	14$
	MOVL	EAX, SS:xffTimeOut##
	MOVL	EDX, SS:xffTimeOut##+4
	CALL	xosnetSndWait##		;Wait for a while
	JNC	4$
14$:	LEAVE
	JMP	xostcpFatal

;Here if some space (but maybe not enough) is available

16$:	CMPL	ECX, ts_left[EBP]
	JLE	18$
	MOVL	ECX, ts_left[EBP]
18$:	MOVL	ts_count[EBP], ECX
	SUBL	ts_left[EBP], ECX
20$:	MOVL	ECX, ts_count[EBP]
	MOVL	EAX, dcb_tcpoutsize[EDI] ;Determine space available before end
	SUBL	EAX, dcb_tcpoutput[EDI]	 ;  of the output buffer
	CMPL	ECX, EAX		;Is it enought?
	JLE	22$			;Yes
	MOVL	ECX, EAX		;No - just copy as much as we can this
					;  time
22$:
	MOVL	EAX, #3
	CALL	putinxxx

	SUBL	ts_count[EBP], ECX	;Reduce amount left
	PUSHL	EDI
	PUSHL	ESI
	LFSL	ESI, ts_buffer[EBP]
	ADDL	ts_buffer[EBP], ECX
	MOVL	EAX, dcb_tcpoutput[EDI]
	MOVL	EDX, EAX
	ADDL	EAX, ECX
	CMPL	EAX, dcb_tcpoutsize[EDI]
	JL	24$
	CLRL	EAX
24$:	MOVL	dcb_tcpoutput[EDI], EAX
	MOVL	EDI, dcb_tcpoutbufr[EDI]
	ADDL	EDI, EDX
	PUSHL	DS
	POPL	ES
	CLD
	CMPL	ECX, #8			;More than 8 bytes to store?
	JBE	26$			;No
	MOVL	EAX, ECX		;Yes - get destination to a long
	MOVL	ECX, EDI		;  boundry
	NEGL	ECX
	ANDL	ECX, #03h
	SUBL	EAX, ECX
	RMOVSB	[EDI], FS:[ESI]
	MOVL	ECX, EAX
	SHRL	ECX, #2t		;Change to number of longs (round down)
	RMOVSL	[EDI], FS:[ESI]		;Copy most of it
	MOVL	ECX, EAX		;Copy any bytes left over
	ANDL	ECX, #3
26$:	RMOVSB	[EDI], FS:[ESI]
	POPL	ESI
	POPL	EDI

	MOVL	EAX, #13h
	MOVL	ECX, ts_count[EBP]
	CALL	putinxxx

	CMPL	ts_count[EBP], #0	;Are we finished with this amount?
	JG	20$			;No - continue
	CMPL	ts_left[EBP], #0	;Did we fill the buffer?
	JG	tcpputblk4		;Yes - continue
	BTL	dcb_netmode[EDI], #NMTCP%PUSH ;Should we push this data?
	JNC	28$			;No
	MOVL	EAX, dcb_tcpoutput[EDI]	;Yes - update the push pointer
	MOVL	dcb_tcpoutpush[EDI], EAX
	CMPL	dcb_tcpoutpkt+npb_nextsnd[EDI], #-1
	CLC
	JNE	28$
	CALL	xostcpCheckSend
28$:	LEAVE
ret030:	RET
.PAGE

xostcpPush::
	MOVL	EAX, dcb_tcpoutput[EDI]	;Update the push pointer
	MOVL	dcb_tcpoutpush[EDI], EAX
	CMPL	dcb_tcpoutpkt+npb_nextsnd[EDI], #-1
	JE	xostcpCheckSend
ret3clc:CLC
	RET
.PAGE
;Subroutine to check if should send data - This routines implements the Nagle
;  slow start algorithm and the send-side SWS avoidance algorithm.  The
;  caller must verify that no output is in progress before calling this
;  routine.
;	c{EDI} = Offset of DCB
;	CALL	xostcpCheckSend
;	C:set = Error
;	  c{EAX} = XOS error code
;	C:clr = Normal

;NOTE:	Sending a FIN is handled somewhat strangely here.  To send a FIN,
;	1 byte is stored in the buffer and the TCPS2$NEEDFIN status bit is
;	set.  Once this bit is set, no additional data can be stored into
;	the ring buffer.  When the last data in the ring buffer is to be
;	sent and the TCPS2$NEEDFIN bit is set the following is done:
;	  If there is more than 1 byte to send, everything but the last
;	    byte is packetized and sent normally.
;	  If there is exactly 1 byte to send, a 0 length packet with the
;	    FIN bit set is sent instead.
;	This allows the normal transmission and retransmission logic to
;	handle FINs without any additional special checks.

xostcpCheckSend::
	CMPL	dcb_tcpoutpkt+npb_nextsnd[EDI], #-1
					;Are we doing output now?
	JNE	ret3clc			;Yes - do nothing now

	MOVL	EAX, #4
	CALL	putinxxx

	PUSHL	#0			;Indicate not pushed
	MOVL	ECX, dcb_tcpoutput[EDI]	;Get number of bytes available
	SUBL	ECX, dcb_tcpouttake[EDI]
	JE	10$			;If nothing to do here
	JGE	2$
	ADDL	ECX, dcb_tcpoutsize[EDI] ;If wrapped
2$:	CMPL	ECX, dcb_tcpoutwina[EDI] ;Is this more than he wants
	JLE	4$			;No
	MOVL	ECX, dcb_tcpoutwina[EDI] ;Yes - only send as much as he wants
4$:
	MOVL	EAX, #5
	CALL	putinxxx

	TESTL	ECX, ECX		;Have something we can send?
	JLE	10$			;No
	CMPL	dcb_tcpoutpush[EDI], #-1 ;Has something been pushed?
	JE	8$			;No
	MOVL	EAX, dcb_tcpoutput[EDI] ;Maybe
	SUBL	EAX, dcb_tcpoutpush[EDI]
	JGE	6$
	ADDL	EAX, dcb_tcpoutsize[EDI]
6$:	CMPL	EAX, ECX
	JGE	8$			;No
	MOVB	[ESP], #TCPC$PSH	;Yes
8$:	TESTB	dcb_tcpsts2[EDI], #TCPS2$FORCE
	JNE	22$
	CMPL	ECX, dcb_tcpoutmaxsg[EDI] ;Yes - can we output a full packet?
	JGE	22$			;Yes - always send it

;;;;	CMPB	[ESP], #0		;No - has it been pushed?
;;;;	JE	10$			;No - send nothing now !!! TIMER ???

	BTL	dcb_netmode[EDI], #NMTCP%CONSEND ;Yes - are we using Negal?
	JNC	chksws			;No
	MOVL	EAX, dcb_tcpouttake[EDI] ;Yes - is anything unACKed?
	CMPL	EAX, dcb_tcpoutack[EDI]
	JE	chksws			;No - continue
10$:	ADDL	ESP, #4t		;Yes - no output now!
	CMPL	dcb_tcpoutwina[EDI], #0	;Is the window closed
	JG	14$			;If some window is available
	CALL	tcprexmitreq		;Window is closed - start the
14$:	CLC				;  retransmit timer for a window probe
	RET

;Here if error setting up the packet

seterr:	ADDL	ESP, #4t
	STC
	RET

;When get here, Negal has said we can send data - now check for SWS

chksws:	MOVL	EAX, dcb_tcpoutwinm[EDI] ;Get 1/4 of the maximum window offered
	SHRL	EAX, #2t
	MOVL	EDX, ECX
	CMPL	dcb_tcpoutwina[EDI], #0
	JL	20$
	ADDL	EDX, dcb_tcpoutwina[EDI]
20$:	CMPL	EDX, EAX		;Is the available window this large?
	JL	10$			;No - don't send now
22$:	TESTB	dcb_tcpsts2[EDI], #TCPS2$REXMIT ;Are we retransmitting now?
	JNE	10$			;Yes - can't send it now
	CMPL	ECX, dcb_tcpoutmaxsg[EDI] ;No - have more than the maximum
					  ;  packet
					;  size?
	JLE	sendpkt			;No
	MOVL	ECX, dcb_tcpoutmaxsg[EDI] ;Yes - only send a maximum packet
sendpkt:

	CMPL	ECX, #2000t
	JB	24$
	INT3
24$:

	SUBL	dcb_tcpoutwina[EDI], ECX ;Reduce the available window

	MOVL	EAX, dcb_tcprcvsnum[EDI] ;Do we need an ACK now?
	CMPL	dcb_tcprcvtnum[EDI], EAX
	JE	25$			;No
	CMPL	ECX, #2			;Yes - sending 2 bytes?
	JNE	25$			;No
	PUSHL	ECX
	PUSHL	EDX
	CALL	tcpsendacknow
	POPL	EDX
	POPL	ECX
25$:

	LEAL	EBX, dcb_tcpoutpkt[EDI]	;Point to our output packet buffer
	PUSHL	ECX
	CALL	tcpsetpkt		;Set up the packet header
	POPL	ECX
	JC	seterr			;If error
	ADDL	npb_count[EBX], ECX
	MOVL	npb_apcnt[EBX], ECX
	MOVL	ESI, dcb_tcpoutbufr[EDI]
	ADDL	ESI, dcb_tcpouttake[EDI]
	PUSHL	DS
	POPL	ES
	CLD
	PUSHL	ECX
	PUSHL	EDI
	MOVL	EAX, dcb_tcpoutsize[EDI] ;Is the data wrapped?
	SUBL	EAX, dcb_tcpouttake[EDI]
	MOVL	EDI, EDX
	CMPL	EAX, ECX
	JGE	nowrap			;No
wrapped:PUSHL	ECX
	SUBL	[ESP], EAX
	MOVL	ECX, EAX
	SHRL	ECX, #2
	RMOVSL	[EDI], [ESI]
	ANDL	EAX, #03h
	JE	26$
	MOVL	ECX, EAX
	RMOVSB	[EDI], [ESI]
26$:	POPL	ECX
	MOVL	EAX, [ESP]
	MOVL	ESI, dcb_tcpoutbufr[EAX]

;Here with data copied to end of the output ring buffer - it's now possible
;  that our output is not byte aligned, so we fix that now if necessary

28$:	TESTL	EDI, #03h
	JE	nowrap
	MOVSB	[EDI], [ESI]
	JMP	28$

;Here if the data in the output ring buffer is not wrapped - Note that we just
;  round up to a whole number of longs here.  The may cause us to access up
;  to 3 bytes beyond the end of the packet in the output ring. This is OK,
;  since we don't use the last 4 bytes of the ring buffer.

nowrap:
	CMPL	ECX, #2000t
	JB	30$
	INT3
30$:

	ADDL	ECX, #3
	SHRL	ECX, #2
	RMOVSL	[EDI], [ESI]
	POPL	EDI
	POPL	ECX
	MOVL	EAX, dcb_tcpoutsnum[EDI]
	MOVL	dcb_tcpoutpkt+npb_seqnum[EDI], EAX
	MOVZBL	EDX, npb_tpofs[EBX]
	XCHGB	AL, AH
	RORL	EAX, #16t
	XCHGB	AL, AH
	MOVL	tcp_seqnum[EBX+EDX], EAX
	ADDL	dcb_tcpoutsnum[EDI], ECX ;Bump send sequence number
	MOVL	EAX, dcb_tcpouttake[EDI] ;Advance the output ring buffer
	ADDL	EAX, ECX		 ;  taker pointer
	CMPL	EAX, dcb_tcpoutsize[EDI]
	JL	32$
	SUBL	EAX, dcb_tcpoutsize[EDI]
32$:	MOVL	dcb_tcpouttake[EDI], EAX ;Update the taker pointer
	MOVZBL	EDX, npb_tpofs[EBX]	;Empty
	CMPL	EAX, dcb_tcpoutput[EDI]	;Is the ring buffer "empty" now?
	JNE	sendrdy			;No - go on
	TESTB	dcb_tcpsts2[EDI], #TCPS2$NEEDFIN ;Yes - do we need a FIN?
	JE	sendrdy			;No
	DECL	npb_count[EBX]		;Yes - reduce the length of this
	DECL	npb_apcnt[EBX]		;  packet by 1
	JNE	beforefin		;If not empty now just send it
	ORB	tcp_code[EBX+EDX], #TCPC$FIN ;Empty - make this a FIN packet
	ORB	dcb_tcpsts2[EDI], #TCPS2$SENTFIN ;Indicate FIN has been sent
	MOVB	[ESP], #0		;Don't set PSH with FIN
	JMP	sendrdy

;Here if we have a packet to send before a FIN packet - back up the taker
;  pointer one byte

beforefin:
	DECL	dcb_tcpoutsnum[EDI]	;Reduce the send sequence number
	DECL	dcb_tcpouttake[EDI]	;Reduce the taker pointer
	JNS	sendrdy
	MOVL	EAX, dcb_tcpoutsize[EDI]
	DECL	EAX
	MOVL	dcb_tcpouttake[EDI], EAX
sendrdy:POPL	EAX
	ORB	tcp_code[EBX+EDX], AL	;Set the PSH bit if need to
	CALL	tcpsetack		;Set the ACK number and calculate the
					;  TCP checksum
	CALL	xosipsSendSetup##	;Set up IP part of packet header
	MOVL	ESI, dcb_netsdb[EDI]	;Get SDB offset
	MOVL	ECX, npb_count[EBX]

	MOVL	EAX, #6
	CALL	putinxxx

.IF NE $$TCPTIME
	PUSHL	EAX
	MOVL	EDX, tcptimepnt#
	MOVL	EAX, knlSchTtl##
	MOVL	[EDX], EAX
	MOVL	4[EDX], #0
	MOVL	8[EDX], #0
	MOVL	EAX, npb_apcnt[EBX]
	MOVL	12t[EDX], EAX
	ADDL	EDX, #16t
	CMPL	EDX, #tcptimeend#
	JB	38$
	MOVL	EDX, #tcptimering#
38$:	MOVL	tcptimepnt#, EDX
	POPL	EAX
.ENDC

	TESTB	dcb_tcpsts1[EDI], #TCPS1$TIMING ;Are we timing now?
	JNE	40$			;Yes - go on
	MOVL	EAX, knlSchTtl##	;No - time this packet
	MOVL	dcb_tcprndbgn[EDI], EAX
	MOVL	EAX, dcb_tcpoutsnum[EDI]
	MOVL	dcb_tcprndsnum[EDI], EAX
	ORB	dcb_tcpsts1[EDI], #TCPS1$TIMING ;Indicate timing now
40$:	MOVL	EAX, #tcpsenddone
	CALL	xossnpSendPkt##		;Output the packet and return
	JC	44$
	CMPL	dcb_tcprxmtwake[EDI], #0
	JE	tcprexmitreq
44$:	RET

;Here when output of a data packet is complete

tcpsenddone:
	CALL	xostcpCheckSend
	RET
.PAGE
;Here if have timeout before receiving an ACK for a packet we have
;  sent - We send a single packet from the output ring buffer.  If the
;  window is closed (possible if the other end has reduced his window
;  from the right) we send a 1-byte window probe packet. Normal output
;  is resumed as soon as this packet is ACKed.

2$:	CLC
	RET

tcprexmit:
	MOVL	dcb_tcprxmtwake[EDI], #0
	CMPL	dcb_tcpoutpkt+npb_nextsnd[EDI], #-1 ;Are we still outputting
						    ;  our packet?
	JNE	tcprexmitreq		;Yes - just restart the timer
	MOVL	ESI, dcb_netsdb[EDI]	;Get offset of the SDB
	MOVL	EAX, dcb_tcpouttake[EDI] ;Get distance between the taker
	SUBL	EAX, dcb_tcpoutack[EDI]	 ;  and ACK pointers
	JGE	4$
	ADDL	EAX, dcb_tcpoutsize[EDI]
4$:	JNE	8$			;Go on if something unACKed
	CMPL	dcb_tcpoutwina[EDI], #0	;Is the window closed?
	JG	xostcpCheckSend		;No - this is very strange - just try
					;  to start output again
	MOVL	EDX, dcb_tcpoutput[EDI]	;Yes - is anything unsent?
	CMPL	EDX, dcb_tcpouttake[EDI]
	JE	2$			;No - nothing to probe with now

;Here if we need a window probe - In this case we adjust our various numbers
;  to make it look like the first unsent byte has been sent but not ACKed so
;  we will retransmit it using the normal retransmit logic.  After the packet
;  has been constructed, we unadjust these values.  Note that this means that
;  we may get an ACK for a supposedly unsent byte.  The procack code allows
;  for this.

	INCL	dcb_tcpoutsnum[EDI]
	INCL	EAX
	MOVL	EDX, dcb_tcpouttake[EDI]
	INCL	EDX
	CMPL	EDX, dcb_tcpoutsize[EDI]
	JL	6$
	CLRL	EDX
6$:	MOVL	dcb_tcpouttake[EDI], EDX
	INCL	dcb_tcpoutwina[EDI]
	PUSHL	#1
	JMP	10$

;Check number of retries

8$:	PUSHL	#0
10$:	INCB	dcb_tcpretrycnt[EDI]	;Increment the retry count
	MOVB	DL, dcb_tcpretrycnt[EDI] ;Get retry number
	CMPB	DL, dcb_tcpretry1[EDI]	;At first threshold?
	JG	12$
	JNE	14$			;No - go on
	PUSHL	EAX
	MOVL	EAX, dcb_ipsrmtaddr[EDI] ;Yes - remove entry from the routing
	PUSHL	EDI			 ;  table to force us to re-do whatever
	MOVL	EDI, dcb_netpdb[EDI]	 ;  we do to get the hardware address
	PUSHL	EBX
	CALL	xosipsRtRmvOne##
	POPL	EBX
	POPL	EDI
	POPL	EAX
	JMP	14$

;Here if past the first threshold - fail if we do not have an address for the
;  packet

12$:	PUSHL	EAX
	MOVL	EAX, dcb_ipsrmtaddr[EDI]
	PUSHL	EDI
	MOVL	ESI, dcb_netsdb[EDI]
	MOVL	EDI, dcb_netpdb[EDI]
	PUSHL	EBX
	CALL	xosipsRtChkAddr##
	POPL	EBX
	POPL	EDI
	POPL	EAX
	JC	30$
14$:	MOVL	EDX, dcb_tcprndave[EDI] ;Double the retransmit interval
	MOVL	dcb_tcprndvar[EDI], EDX
	SHLL	dcb_tcprxmtint[EDI], #1
	SHLL	dcb_tcprndave[EDI], #1
	MOVL	dcb_tcprndvar[EDI], #0
	CMPL	dcb_tcprxmtint[EDI], #TIMEOUT_TRNMAX/TICKPERSP ;Too big?
	JB	16$			;No
	MOVL	dcb_tcprxmtint[EDI], #TIMEOUT_TRNMAX/TICKPERSP    ;Yes - set it
	MOVL	dcb_tcprndave[EDI], #{TIMEOUT_TRNMAX/TICKPERSP}*2 ;  to the max
16$:
.IF NE $$TCPTIME
	PUSHL	EAX
	MOVL	EDX, tcptimepnt#
	MOVL	EAX, knlSchTtl##
	MOVL	[EDX], EAX
	MOVW	4[EDX], #-1
	MOVL	EAX, dcb_tcprxmtint[EDI]
	MOVW	6[EDX], AX
	MOVL	EAX, dcb_tcprndave[EDI]
	ADDL	EAX, EAX
	MOVW	8t[EDX], AX
	MOVL	EAX, dcb_tcprndvar[EDI]
	SHLL	EAX, #2t
	MOVW	10t[EDX], AX
	MOVL	12t[EDX], #0
	ADDL	EDX, #16t
	CMPL	EDX, #tcptimeend#
	JB	18$
	MOVL	EDX, #tcptimering#
18$:	MOVL	tcptimepnt#, EDX
	POPL	EAX
.ENDC
	MOVB	DL, dcb_tcpretrycnt[EDI]
	CMPB	[ESP], #0		;Are we probing the window?
	JNE	20$			;Yes
	CMPB	dcb_tcpretry2[EDI], #0	;No - want infinite retry?
	JNE	22$			;No
20$:	CMPB	DL, #255t		;Yes - at maximum value?
	JB	22$			;No
	DECB	dcb_tcpretrycnt[EDI]	;Yes - keep it there
	JMP	24$

;Here if should check for too many retransmits

22$:	CMPB	DL, dcb_tcpretry2[EDI]	;At second threshold?
	JAE	30$			;Yes - its time to fail!
24$:	SUBL	dcb_tcpoutsnum[EDI], EAX ;Adjust the send sequence number
	ADDL	dcb_tcpoutwina[EDI], EAX ;And the available output window
	MOVL	EAX, dcb_tcpoutack[EDI]	 ;Back up the taker pointer to the ACK
	MOVL	dcb_tcpouttake[EDI], EAX ;  pointer
	ANDB	dcb_tcpsts2[EDI], #~TCPS2$REXMIT
	ORB	dcb_tcpsts2[EDI], #TCPS2$FORCE
	MOVL	EAX, dcb_ipstpdb[EDI]	;Count the retransmission
	INCL	tpdb_tcpcntrexmit[EAX]
	CALL	xostcpCheckSend		;Send a packet
	CALL	tcprexmitreq		;Restart the timer (SHOULD CHANGE THIS
					;  TO REUSE THE SAME WAKE BLOCK!!!)
	ANDB	dcb_tcpsts2[EDI], #~TCPS2$FORCE
	POPL	EAX
	CMPB	AL, #0			;Was this a window probe?
	JE	28$			;No
	DECL	dcb_tcpoutsnum[EDI]
	DECL	dcb_tcpoutwina[EDI]
	DECL	dcb_tcpouttake[EDI]
	JNS	28$
	MOVL	EAX, dcb_tcpoutsize[EDI]
	DECL	EAX
	MOVL	dcb_tcpouttake[EDI], EAX
	CLC
	RET

;Here if not sending a window probe

28$:	ORB	dcb_tcpsts2[EDI], #TCPS2$REXMIT ;Indicate retransmitting
	CLC
	RET

;Here if have retransmitted a packet too many times

30$:	ADDL	ESP, #4t
	MOVL	EAX, #ER_NCLST		;Get error code
	CALL	xostcpGiveAll		;Give up all buffers
	CALL	tcplost#		;Indicate connection lost
	CLC
	RET
.PAGE
	.SBTTL	tcprexmitreq - Subroutine to enter retransmit wake request for TCP

;Subroutine to enter retransmit wake request for TCP
;	CALL	tcprexmitreq
;	C:set = Error
;	  c{EAX} = Error code
;	C:clr = Normal

tcprexmitreq::
	MOVL	EAX, dcb_tcprxmtint[EDI] ;Get the retransmit interval
	MOVL	EBX, #tcprexmit
xostcpTimerReq:
	PUSHL	ESI
	CMPL	dcb_tcprxmtwake[EDI], #0 ;Have a timer going now?
	JE	4$			;No
	MOVL	ESI, dcb_tcprxmtwake[EDI] ;Yes - clear it first
	CALL	knlWakeRemove##
4$:	CALL	knlWakeRequest##	;Enter the wake request
	MOVL	dcb_tcprxmtwake[EDI], ESI ;Store offset of the timer block (this
	POPL	ESI			  ;  will be 0 if an error occured)
	RET				;Finished

	.SBTTL	xostcpTimerRmv - Subroutine to remove retransmit wake request for TCP

;Subroutine to remove retransmit wake request for TCP
;	CALL	xostcpTimerRmv

xostcpTimerRmv::
	CMPL	dcb_tcprxmtwake[EDI], #0 ;Have a timer going now?
	JE	ret040			;No
	PUSHL	ESI			;Yes - kill it
	MOVL	ESI, dcb_tcprxmtwake[EDI]
	CALL	knlWakeRemove##
	CLRL	ESI
	MOVL	dcb_tcprxmtwake[EDI], ESI
	POPL	ESI
ret040:	RET
.PAGE
	.SBTTL	tcpsetack - Subroutine to set up ACK number and window value

;Subroutine to set up ACK number and window value in TCP header and to
;  calculate and store the checksum for the TCP packet
;	c{EBX} = Offset of packet
;	c{EDI} = Offset of DCB
;	CALL	tcpsetack

tcpsetack::
	CMPL	dcb_tcpackwake[EDI], #0	;Is the ACK timer running now?
	JE	2$			;No
	PUSHL	ESI			;Yes - stop it
	MOVL	ESI, dcb_tcpackwake[EDI]
	CLRL	EAX
	MOVL	dcb_tcpackwake[EDI], EAX
	CALL	knlWakeRemove##
	POPL	ESI
2$:	MOVZBL	ECX, npb_tpofs[EBX]
	MOVL	EAX, dcb_tcprcvsnum[EDI] ;Get receive sequence number
	MOVL	dcb_tcprcvtnum[EDI], EAX ;Remember we have sent an ACK for it
	XCHGB	AL, AH			;Change to network byte order
	RORL	EAX, #16t
	XCHGB	AL, AH
	MOVL	tcp_acknum[EBX+ECX], EAX ;Store in the packet
	MOVL	EAX, dcb_tcprcvwinb[EDI] ;Get window size
	MOVL	dcb_tcprcvwina[EDI], EAX ;Remember we have sent this value
	XCHGB	AL, AH			;Change to network byte order
	MOVW	tcp_window[EBX+ECX], AX ;Store in the packet
	ORB	tcp_code[EBX+ECX], #TCPC$ACK
tcpchksum::
	MOVL	EDX, dcb_ipstpdb[EDI]	;Get offset of the TPDB
	INCL	tpdb_tcpcntpktout[EDX]	;Bump output counts
	MOVL	ECX, npb_count[EBX]	;Get total length of packet
	MOVZBL	EAX, npb_tpofs[EBX]	;Get offset of start of TCP part
	SUBL	ECX, EAX		;Calculate length of TCP part
	ADDL	tpdb_tcpcntbyteout[EDX], ECX
	MOVL	EAX, dcb_netpdb[EDI]	;Need to calculate checksum value?
tcpchksum2::
	TESTB	pdb_ipchksumh[EAX], #PCSH$GEN
	JE	ret040			;No - nothing more needed here
	PUSHL	ESI			;The first 2 longs for the pseudo-header
					;  are found in the IP header
	MOVZBL	ESI, npb_npofs[EBX]	   ;Get offset of the IP header source
	LEAL	ESI, ip_srcipaddr[ESI+EBX] ;  IP address field
	CLD
	LODSL	[ESI]			;Get first 32 bits
	ADDL	EAX, [ESI]		;Add in next 32 bits (destination IP
	ADCL	EAX, #0			;  address)
	MOVZBL	ESI, npb_tpofs[EBX]	;Point to the TCP header
	ADDL	ESI, EBX
	MOVW	tcp_checksum[ESI], #0	;Make sure checksum field is 0
	MOVL	EDX, npb_count[EBX]	;Get total length of TCP part (including
	MOVZBL	ECX, npb_tpofs[EBX]	;  the TCP header)
	SUBL	EDX, ECX
	MOVL	ECX, EDX		;Copy it for use below as a count
	XCHGB	DL, DH
	SHLL	EDX, #16t		;Position it for the checksum
	MOVB	DH, #IPP_TCP		;Include the protocol value
	ADDL	EAX, EDX		;Add into checksum value
	ADCL	EAX, #0
	CALL	xosipsChkSumFin##	;Calculate checksum for data part of
	NOTL	EAX			;  the packet
	MOVZBL	ECX, npb_tpofs[EBX]	;Store final checksum value
	MOVW	tcp_checksum[EBX+ECX], AX
	POPL	ESI
	RET
.PAGE
	.SBTTL	xostcpFatal - Subroutine to indicate have had fatal application error

;Subroutine to indicate have had fatal application level protocol error
;	CALL	xostcpFatal
;	C:set always
;  EAX is preserved

xostcpFatal:
	PUSHL	EAX
	ORB	dcb_tcpsts2[EDI], #TCPS2$FATAL ;Indicate fatal error
	CALL	xostcpGiveAll		;Give up all buffers we have
	CALL	xostcpSendReset
	CALL	xostcpHashClr		;Remove for the input hash list
	MOVB	dcb_tcpstate[EDI], #TCPS_CLOSED ;Indicate idle
4$:	POPL	EAX
	STC
	RET
.PAGE
	.SBTTL	xostcpGiveAll - Subroutine to give up all packet buffers

;Subroutine to give up all packet buffers
;	c{EDI} = Offset of DCB
;	CALL	xostcpGiveAll
;  c{EAX} is respected

xostcpGiveAll::
	PUSHL	EAX
	CALL	xostcpTimerRmv		;Stop retransmit timer if its running
	CMPL	dcb_tcpackwake[EDI], #0 ;Is the ACK timer running?
	JE	2$			;No
	PUSHL	ESI			;Yes - stop it
	MOVL	ESI, dcb_tcpackwake[EDI]
	CLRL	EAX
	MOVL	dcb_tcpackwake[EDI], EAX
	CALL	knlWakeRemove##
	POPL	ESI
2$:	POPL	EAX
giveal2:MOVL	EBX, dcb_tcprcvoosl[EDI] ;Do we have any out-of-sequence
	TESTL	EBX, EBX		 ;  packets?
	JE	6$			;No
	CLRL	ECX			;Yes
	MOVL	dcb_tcprcvoosl[EDI], ECX
	CALL	xosnetGiveList##
6$:	MOVL	EBX, dcb_tcprcvtakb[EDI] ;Do we have any receive packets?
	CLRL	ECX
	TESTL	EBX, EBX
	JE	xostcpClearSnd		;No
	MOVL	dcb_tcprcvtakb[EDI], ECX ;Yes
	MOVL	dcb_tcprcvtakp[EDI], ECX
	MOVL	dcb_tcprcvtakc[EDI], ECX
	MOVL	dcb_tcprcvavl[EDI], ECX
	MOVL	dcb_tcprcvputb[EDI], ECX
	MOVL	dcb_tcprcvputp[EDI], ECX
	MOVL	dcb_tcprcvcnt[EDI], ECX
	CALL	xosnetGiveList##
xostcpClearSnd:
	MOVL	dcb_tcpouttake[EDI], ECX
	MOVL	dcb_tcpoutput[EDI], ECX
	MOVL	dcb_tcpoutack[EDI], ECX
	MOVL	dcb_tcpoutpush[EDI], ECX
	ANDB	dcb_tcpsts2[EDI], #~{TCPS2$NEEDFIN|TCPS2$SENTFIN}
10$:	RET
.PAGE

;	c[AX] = Function
;		  1 = about to send
;		  2 = ACK done
;		  3 = put block
;		  4 = check send (before check)
;		  5 = check send (after check)
;	c[CX] = Amount

;Data stored:
;  0  2  Function
;  2  2  dcb_tcpoutput
;  4  2  dcb_tcpouttake
;  6  2  dcb_tcpoutpush
;  8  2  dcb_tcpoutwina
; 10  2  unacked byte count
; 12  2  output buffer byte count
; 14  2  amount

putinxxx::
;;;	CMPW	dcb_ipslclport[EDI], #7571t
;;;	JNE	8$
;;;	CMPL	dcb_ipsrmtaddr[EDI], #0900050Ah
;;;	JNE	8$
	PUSHL	EDX
	MOVL	EDX, xxxpnt
	CMPL	EDX, #xxxend
	JL	2$
	MOVL	EDX, #xxxrng
	MOVL	xxxpnt, EDX
2$:	MOVW	[EDX], AX
	MOVL	EAX, dcb_tcpoutput[EDI]
	MOVW	2[EDX], AX
	MOVL	EAX, dcb_tcpouttake[EDI]
	MOVW	4[EDX], AX
	MOVL	EAX, dcb_tcpoutpush[EDI]
	MOVW	6[EDX], AX
	MOVL	EAX, dcb_tcpoutwina[EDI]
	MOVW	8[EDX], AX
	MOVL	EAX, dcb_tcpoutput[EDI]
	SUBL	EAX, dcb_tcpoutack[EDI]
	JGE	4$
	ADDL	EAX, dcb_tcpoutsize[EDI]
4$:	MOVW	10t[EDX], AX
	MOVL	EAX, dcb_tcpoutput[EDI]
	SUBL	EAX, dcb_tcpouttake[EDI]
	JGE	6$
	ADDL	EAX, dcb_tcpoutsize[EDI]
6$:	MOVW	12t[EDX], AX
	MOVW	14t[EDX], CX
	ADDL	xxxpnt, #16t
	POPL	EDX
8$:	RET

	DATA

xxxrng::.BLKW	512t*8
xxxend::.LONG	0FFFFFFFFh
xxxpnt::.LONG	xxxrng

xdatatop::

	CODE

xcodetop::

	LKEEND
