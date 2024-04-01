	.TITLE	MOUSE

	.PROC	80386
	.INCLUD	XOSINC:\XMAC\XOS.PAR
	.INCLUD XOSINC:\XMAC\XOSTIME.PAR
	.INCLUD XOSINC:\XMAC\XOSERR.PAR
	.INCLUD	XOSINC:\XMAC\XOSTRM.PAR
	.INCLUD XOSINC:\XMAC\CLIBX.PAR
	.INCLUD	XOSINC:\XMAC\XOSTHREADS.PAR
	.INCLUD XWS.PAR
	.INCLUD XWSX.PAR
	.INCLUD	GDP.PAR

	.STDSEGS

	.PSECT	_TEXT_p

	.ENTRY	xwsInitMouse
xwsInitMouse::
	PUSHL	#mousetdb		;Create the mouse thread
	PUSHL	#0F000h-tdb_SIZE
	PUSHL	#0.B
	PUSHL	#mousethread
	PUSHL	#0.B
	PUSHL	#0.B
	CALLF	thdCtlCreate#
	TESTL	EAX, EAX
10$:	JS	xwserror#
	RET
.PAGE
	.SBTTL	mousethread - Main thread function for the mouse thread

;Main thread function for the mouse thread

$$$=!0
FRM mt_pressed , 4t
FRM mt_released, 4t
FRM mt_curstate, 4t
FRM mt_buttons , 4t
FRM mt_handle  , 4t
FRM mt_buffer  , 16t
mt_SIZE=!$$$

mousethread:
	MOVL	EBP, ESP
	SUBL	ESP, #mt_SIZE.B
	PUSHL	xws_TrmHndl#		;Get name of our mouse device
	PUSHL	DS
	PUSHL	#mousechars
	CALLF	svcIoDevChar##
	TESTL	EAX, EAX
	JS	3$.S
	MOVL	EBX, #mname-1		;Put a colon after the name
2$:	INCL	EBX
	CMPB	[EBX], #0
	JNE	2$.S
	MOVW	[EBX], #':'
	PUSHL	#O$IN.B			;Open the mouse device
	PUSHL	DS
	PUSHL	#mname
	PUSHL	#0.B
	PUSHL	#0.B
	CALLF	svcIoOpen##
	TESTL	EAX, EAX
3$:	JS	xwserror#		;If error
	MOVL	mt_handle.B[EBP], EAX
	CLRL	EAX
	MOVL	mt_buttons.B[EBP], EAX
mouseloop:
	PUSHL	mt_handle.B[EBP]
	PUSHL	DS
	LEAL	EAX, mt_buffer.B[EBP]
	PUSHL	EAX
	PUSHL	#16t.B
	CALLF	thdIoInBlock#
	TESTL	EAX, EAX
	JS	xwserror#
	MOVL	EAX, mt_buffer+0.B[EBP]	;Get button states
	MOVL	ECX, EAX
	MOVL	EDX, EAX
	XORL	EAX, mt_buttons.B[EBP]	;Get changed buttons
	MOVL	mt_buttons.B[EBP], EDX
	MOVL	EDX, EAX
	ANDL	EAX, ECX		;Get only buttons just pressed
	MOVL	mt_pressed.B[EBP], EAX
	NOTL	ECX			;Get only buttons just released
	ANDL	EDX, ECX
	MOVL	mt_released.B[EBP], EDX
	MOVL	ECX, mt_buffer+4.B[EBP]	;Get X movement
	MOVL	EDX, mt_buffer+8.B[EBP]	;Get Y movement
	MOVL	EAX, ECX		;Have any movement?
	ORL	EAX, EDX
	JE	nomove			;No
	ADDL	xwscursorx#, ECX	;Yes
	CMPL	xwscursorx#, #0.B	;Make sure cursor stays on the screen
	JGE	4$.S
	MOVL	xwscursorx#, #0
4$:	MOVL	EAX, xws_ScreenDRW#+drw_width
	CMPL	xwscursorx#, EAX
	JLE	6$.S
	MOVL	xwscursorx#, EAX
6$:	ADDL	xwscursory#, EDX
	CMPL	xwscursory#, #0.B
	JGE	8$.S
	MOVL	xwscursory#, #0
8$:	MOVL	EAX, xws_ScreenDRW+drw_height
	CMPL	xwscursory#, EAX
	JLE	findwin.S
	MOVL	xwscursory#, EAX
findwin:CALL	xwsfindcursorwindow#	;Find the window for the cursor
	MOVL	mt_curstate.B[EBP], EAX
	MOVZBL	ECX, win_basmoving.B[EDI] ;Now moving or resizing the window?
	TESTL	ECX, ECX
	JE	movnone
	MOVL	EBX, win_basparent.B[EDI]
	TESTL	EBX, EBX
	JE	16$.S
	MOVL	EAX, win_basscnxpos.B[EBX]
	ADDL	EAX, win_basxleft.B[EBX]
	CMPL	xwscursorx#, EAX
	JGE	10$.S
	MOVL	xwscursorx#, EAX
	JMP	12$.S

10$:	ADDL	EAX, win_basxclient.B[EBX]
	DECL	EAX
	CMPL	xwscursorx#, EAX
	JLE	12$.S
	MOVL	xwscursorx#, EAX
12$:	MOVL	EAX, win_basscnypos.B[EBX]
	ADDL	EAX, win_basytop.B[EBX]
	CMPL	xwscursory#, EAX
	JGE	14$.S
	MOVL	xwscursory#, EAX
	JMP	16$.S

14$:	ADDL	EAX, win_basyclient.B[EBX]
	DECL	EAX
	CMPL	xwscursory#, EAX
	JLE	16$.S
	MOVL	xwscursory#, EAX
16$:	JMPIL	movdsp-4[ECX*4]

	.PSECT	_DATA_p
movdsp:	.LONG	moving		;CS_MOVE   = 1
	.LONG	resizetop	;CS_SIZET  = 2
	.LONG	resizeleft	;CS_SIZEL  = 3
	.LONG	resizebottom	;CS_SIZEB  = 4
	.LONG	resizeright	;CS_SIZER  = 5
	.LONG	resizetl	;CS_SIZETL = 6
	.LONG	resizetr	;CS_SIZETR = 7
	.LONG	resizebl	;CS_SIZEBL = 8
	.LONG	resizebr	;CS_SIZEBR = 9

	.PSECT	_TEXT_p
.PAGE
;Here if now moving the window

moving:	MOVL	EAX, xwscursorx#	;Get the new target position
	SUBL	EAX, win_basmvxofs.B[EDI]
	MOVL	EDX, xwscursory#
	SUBL	EDX, win_basmvyofs.B[EDI]
	CMPL	EAX, win_basscnxpos.B[EDI] ;Different from current position?
	JNE	2$.S			;Yes
	CMPL	EDX, win_basscnypos.B[EDI] ;Maybe
	JE	6$.S			;No - nothing to do here
2$:	MOVL	ECX, win_basparent.B[EDI] ;Here if need to move the window now
	PUSHL	EDI
	SUBL	EAX, win_basscnxpos.B[ECX]
	PUSHL	EAX
	SUBL	EDX, win_basscnypos.B[ECX]
	PUSHL	EDX
	PUSHL	win_baswidth.B[EDI]
	PUSHL	win_basheight.B[EDI]
4$:	CALL	xwsWinMove#
6$:	MOVZBL	EAX, win_basmoving.B[EDI]
	JMP	movnone

;Here if resizeing the window using the top border

resizetop:
	MOVL	EAX, xwscursory#	;Get the new target top position
	SUBL	EAX, win_basmvyofs.B[EDI]
	SUBL	EAX, win_basscnypos.B[EDI] ;Different from current position?
	JE	6$.S			;No - nothing to do here
	MOVL	EDX, EAX
	ADDL	EAX, win_basypos.B[EDI]
	NEGL	EDX
	ADDL	EDX, win_basheight.B[EDI]
	MOVL	ECX, win_basytop.B[EDI]
	ADDL	ECX, win_basybottom.B[EDI]
	ADDL	ECX, #2t.B
	SUBL	ECX, EDX
	JLE	8$.S
	SUBL	EAX, ECX
	ADDL	EDX, ECX
8$:	PUSHL	EDI
	PUSHL	win_basxpos.B[EDI]
	PUSHL	EAX
	PUSHL	win_baswidth.B[EDI]
	PUSHL	EDX
	JMP	4$.S

;Here if resizeing the window using the left border

resizeleft:
	MOVL	EAX, xwscursorx#	;Get the new target left position
	SUBL	EAX, win_basmvxofs.B[EDI]
	SUBL	EAX, win_basscnxpos.B[EDI] ;Different from current position?
	JE	6$.S			;No - nothing to do here
	MOVL	EDX, EAX
	ADDL	EAX, win_basxpos.B[EDI]
	NEGL	EDX
	ADDL	EDX, win_baswidth.B[EDI]
	MOVL	ECX, win_basxleft.B[EDI]
	ADDL	ECX, win_basxright.B[EDI]
	ADDL	ECX, #2t.B
	SUBL	ECX, EDX
	JLE	10$.S
	SUBL	EAX, ECX
	ADDL	EDX, ECX
10$:	PUSHL	EDI
	PUSHL	EAX
	PUSHL	win_basypos.B[EDI]
	PUSHL	EDX
	PUSHL	win_basheight.B[EDI]
	JMP	4$.S

;Here if resizeing the window using the bottom border

resizebottom:
	MOVL	EAX, xwscursory#	;Get the new target height
	SUBL	EAX, win_basmvyofs.B[EDI]
	SUBL	EAX, win_basscnypos.B[EDI]
	CMPL	EAX, win_basheight.B[EDI] ;Different from current height?
	JE	6$.S			;No - nothing to do here
12$:	MOVL	ECX, win_basparent.B[EDI] ;Yes - need to resize the window
	PUSHL	EDI
	PUSHL	win_basxpos.B[EDI]
	PUSHL	win_basypos.B[EDI]
	PUSHL	win_baswidth.B[EDI]
	PUSHL	EAX
	JMP	4$

;Here if resizeing the window using the right border

resizeright:
	MOVL	EAX, xwscursorx#	;Get the new target width
	SUBL	EAX, win_basmvxofs.B[EDI]
	SUBL	EAX, win_basscnxpos.B[EDI]
	CMPL	EAX, win_baswidth.B[EDI] ;Different from current width?
	JE	6$			;No - nothing to do here
	MOVL	ECX, win_basparent.B[EDI] ;Yes - need to resize the window
	PUSHL	EDI
	PUSHL	win_basxpos.B[EDI]
	PUSHL	win_basypos.B[EDI]
	PUSHL	EAX
	PUSHL	win_basheight.B[EDI]
	JMP	4$

;Here if resizeing the window using the top-left corner

resizetl:
	MOVL	EAX, xwscursory#	;Get the new target top position
	SUBL	EAX, win_basmvyofs.B[EDI]
	SUBL	EAX, win_basscnypos.B[EDI] ;Different from current position?
	JE	resizeleft		;No - just resize left edge
	MOVL	EDX, EAX
	ADDL	EAX, win_basypos.B[EDI]
	NEGL	EDX
	ADDL	EDX, win_basheight.B[EDI]
	MOVL	ECX, win_basytop.B[EDI]
	ADDL	ECX, win_basybottom.B[EDI]
	ADDL	ECX, #2t.B
	SUBL	ECX, EDX
	JLE	14$.S
	SUBL	EAX, ECX
	ADDL	EDX, ECX
14$:	MOVL	EBX, xwscursorx#	;Get the new target left position
	SUBL	EBX, win_basmvxofs.B[EDI]
	SUBL	EBX, win_basscnxpos.B[EDI] ;Different from current position?
	JE	8$			;No - just resize top edge
	MOVL	ESI, EBX
	ADDL	EBX, win_basxpos.B[EDI]
	NEGL	ESI
	ADDL	ESI, win_baswidth.B[EDI]
	MOVL	ECX, win_basxleft.B[EDI]
	ADDL	ECX, win_basxright.B[EDI]
	ADDL	ECX, #2t.B
	SUBL	ECX, ESI
	JLE	16$.S
	SUBL	EBX, ECX
	ADDL	ESI, ECX
16$:	PUSHL	EDI
	PUSHL	EBX
	PUSHL	EAX
	PUSHL	ESI
	PUSHL	EDX
	JMP	4$

;Here if resizeing the window using the top-right corner

resizetr:
	MOVL	EAX, xwscursory#	;Get the new target top position
	SUBL	EAX, win_basmvyofs.B[EDI]
	SUBL	EAX, win_basscnypos.B[EDI] ;Different from current position?
	JE	resizeright		;No - just resize right edge
	MOVL	EDX, EAX
	ADDL	EAX, win_basypos.B[EDI]
	NEGL	EDX
	ADDL	EDX, win_basheight.B[EDI]
	MOVL	ECX, win_basytop.B[EDI]
	ADDL	ECX, win_basybottom.B[EDI]
	ADDL	ECX, #2t.B
	SUBL	ECX, EDX
	JLE	18$.S
	SUBL	EAX, ECX
	ADDL	EDX, ECX
18$:	MOVL	EBX, xwscursorx#	;Get the new target width
	SUBL	EBX, win_basmvxofs.B[EDI]
	SUBL	EBX, win_basscnxpos.B[EDI]
	CMPL	EBX, win_baswidth.B[EDI] ;Different from current width?
	JE	8$			;No - just resize top edge
	PUSHL	EDI
	PUSHL	win_basxpos.B[EDI]
	PUSHL	EAX
	PUSHL	EBX
	PUSHL	EDX
	JMP	4$

;Here if resizeing the window using the bottom-left corner

resizebl:
	MOVL	EAX, xwscursory#	;Get the new target height
	SUBL	EAX, win_basmvyofs.B[EDI]
	SUBL	EAX, win_basscnypos.B[EDI]
	CMPL	EAX, win_basheight.B[EDI] ;Different from current height?
	JE	resizeleft		;No - just change the width
	MOVL	EBX, xwscursorx#	;Get the new target left position
	SUBL	EBX, win_basmvxofs.B[EDI]
	SUBL	EBX, win_basscnxpos.B[EDI] ;Different from current position?
	JE	12$			;No - just change the height
	MOVL	EDX, EBX
	ADDL	EBX, win_basxpos.B[EDI]
	NEGL	EDX
	ADDL	EDX, win_baswidth.B[EDI]
	MOVL	ECX, win_basxleft.B[EDI]
	ADDL	ECX, win_basxright.B[EDI]
	ADDL	ECX, #2t.B
	SUBL	ECX, EDX
	JLE	20$.S
	SUBL	EBX, ECX
	ADDL	EDX, ECX
20$:	PUSHL	EDI
	PUSHL	EBX
	PUSHL	win_basypos.B[EDI]
	PUSHL	EDX
	PUSHL	EAX
	JMP	4$

;Here if resizeing the window using the bottom-right corner

resizebr:
	MOVL	EAX, xwscursory#	;Get the new target height
	SUBL	EAX, win_basmvyofs.B[EDI]
	SUBL	EAX, win_basscnypos.B[EDI]
	CMPL	EAX, win_basheight.B[EDI] ;Different from current height?
	JE	resizeright		;No - just change the width
	MOVL	EDX, xwscursorx#	;Get the new target width
	SUBL	EDX, win_basmvxofs.B[EDI]
	SUBL	EDX, win_basscnxpos.B[EDI]
	CMPL	EDX, win_baswidth.B[EDI] ;Different from current width?
	JE	12$			;No - just change the height
	MOVL	ECX, win_basparent.B[EDI] ;Yes - change both height and width
	PUSHL	EDI
	PUSHL	win_basxpos.B[EDI]
	PUSHL	win_basypos.B[EDI]
	PUSHL	EDX
	PUSHL	EAX
	JMP	4$
.PAGE
;Here if not now moving or resizing the window and the window has not grabbed
;  the cursor.  See if the cursor is over a border area.  Since we have not
;  grabbed the cursor, we know it must actually be within the window.

movnone:TESTL	EAX, EAX
	JE	notborder

;Here if not now moving or resizing the window and the cursor is now over
;  the window border

inborder:
	TESTB	mt_pressed.B[EBP], #40h	;Left button pressed?
	JE	notleft			;No
	JMPIL	bgndsp[EAX*4]		;Yes

bgndsp:	.LONG	0		;CS_NONE   = 0
	.LONG	bgnmove		;CS_MOVE   = 1
	.LONG	bgnresizetop	;CS_SIZET  = 2
	.LONG	bgnresizeleft	;CS_SIZEL  = 3
	.LONG	bgnresizebottom	;CS_SIZEB  = 4
	.LONG	bgnresizeright	;CS_SIZER  = 5
	.LONG	bgnresizetl	;CS_SIZETL = 6
	.LONG	bgnresizetr	;CS_SIZETR = 7
	.LONG	bgnresizebl	;CS_SIZEBL = 8
	.LONG	bgnresizebr	;CS_SIZEBR = 9

	.PSECT	_TEXT_p

;Here with left button pressed over top border

bgnresizetop:
	MOVL	EDX, xwscursory#
	JMP	4$.S

;Here with left button pressed over left border

bgnresizeleft:
	MOVL	EDX, xwscursorx#
	JMP	6$.S

;Here with left button pressed over bottom border

bgnresizebottom:
	MOVL	EDX, xwscursory#
	SUBL	EDX, win_basheight.B[EDI]
4$:	SUBL	EDX, win_basscnypos.B[EDI]
	MOVL	win_basmvyofs.B[EDI], EDX
	MOVL	EBX, xws_CurSizeNS#
	JMP	12$

;Here with left button pressed over right border

bgnresizeright:
	MOVL	EDX, xwscursorx#
	SUBL	EDX, win_baswidth.B[EDI]
6$:	SUBL	EDX, win_basscnxpos.B[EDI]
	MOVL	win_basmvxofs.B[EDI], EDX
	MOVL	EBX, xws_CurSizeEW#
	JMP	12$

;Here with left button pressed over top-left corner

bgnresizetl:
	MOVL	EDX, xwscursory#
	SUBL	EDX, win_basscnypos.B[EDI]
	MOVL	win_basmvyofs.B[EDI], EDX
	MOVL	EDX, xwscursorx#
	JMP	10$.S

;Here with left button pressed over top-right corner

bgnresizetr:
	MOVL	EDX, xwscursory#
	SUBL	EDX, win_basscnypos.B[EDI]
	MOVL	win_basmvyofs.B[EDI], EDX
	MOVL	EDX, xwscursorx#
	SUBL	EDX, win_baswidth.B[EDI]
	JMP	8$.S

;Here with left button pressed over bottom-left corner

bgnresizebl:
	MOVL	EDX, xwscursory#
	SUBL	EDX, win_basscnypos.B[EDI]
	SUBL	EDX, win_basheight.B[EDI]
	MOVL	win_basmvyofs.B[EDI], EDX
	MOVL	EDX, xwscursorx#
8$:	SUBL	EDX, win_basscnxpos.B[EDI]
	MOVL	win_basmvxofs.B[EDI], EDX
	MOVL	EBX, xws_CurSizeNESW#
	JMP	12$.S

;Here with left button pressed over top-right corner

bgnresizebr:
	MOVL	EDX, xwscursory#
	SUBL	EDX, win_basscnypos.B[EDI]
	SUBL	EDX, win_basheight.B[EDI]
	MOVL	win_basmvyofs.B[EDI], EDX
	MOVL	EDX, xwscursorx#
	SUBL	EDX, win_baswidth.B[EDI]
10$:	SUBL	EDX, win_basscnxpos.B[EDI]
	MOVL	win_basmvxofs.B[EDI], EDX
	MOVL	EBX, xws_CurSizeNWSE#
	JMP	12$.S

notleft:TESTB	mt_pressed.B[EBP], #10h ;Right button pressed?
	JE	16$.S			;No
bgnmove:MOVL	EDX, xwscursorx#	;Yes
	SUBL	EDX, win_basscnxpos.B[EDI]
	MOVL	win_basmvxofs.B[EDI], EDX
	MOVL	EDX, xwscursory#
	SUBL	EDX, win_basscnypos.B[EDI]
	MOVL	win_basmvyofs.B[EDI], EDX
	MOVB	AL, #CS_MOVE
	MOVL	EBX, xws_CurMove#
12$:	MOVB	win_basmoving.B[EDI], AL
	MOVL	xwscursorgrabwin#, EDI	;Grab the cursor
14$:	CALLI	xws_ScreenDRW#+drw_dspcursor
	JMP	mouseloop

;Here if neither left or right buttons pressed

16$:	TESTB	mt_released.B[EBP], #50h ;Right or left button released?
	JE	20$.S			;No
	CLRL	EAX			;Yes
	MOVL	xwscursorgrabwin#, EAX
	MOVB	win_basmoving.B[EDI], AL
	MOVL	mt_released.B[EBP], EAX
	MOVL	mt_pressed.B[EBP], EAX
	JMP	findwin

20$:	MOVL	EBX, xwscurtbl#-4[EAX*4]
	MOVL	EBX, [EBX]
	JMP	14$.S
.PAGE
notborder:
	MOVL	EBX, win_bascursor.B[EDI]
	TESTL	EBX, EBX
	JE	2$.S
	CALLI	xws_ScreenDRW+drw_dspcursor
2$:	MOVL	ESI, mt_buffer+12t.B[EBP] ;Get time
	MOVL	EAX, #EVENT_MOUSEMV	;Report the mouse move event
	CALL	genevent
	CLRL	EAX
	JMP	dobuts.S

;Here if no mouse movement

nomove:	CALL	xwsfindcursorwindow#	;Find the window for the cursor
	TESTL	EAX, EAX
	JNE	inborder
	MOVZBL	EAX, win_basmoving.B[EDI]
dobuts:	MOVL	mt_curstate.B[EBP], EAX
	TESTB	mt_pressed.B[EBP], #40h	;Left button pressed?
	JE	4$.S			;No
	MOVL	EAX, #EVENT_LBUTDN	;Yes
	CALL	genevent
4$:	TESTB	mt_released.B[EBP], #40h ;Left button released?
	JE	6$.S			;No
	MOVL	EAX, #EVENT_LBUTUP	;Yes
	CALL	genevent
6$:	TESTB	mt_pressed.B[EBP], #20h	;Center button pressed?
	JE	8$.S			;No
	MOVL	EAX, #EVENT_CBUTDN	;Yes
	CALL	genevent
8$:	TESTB	mt_released.B[EBP], #20h ;Center button released?
	JE	12$.S			;No
	MOVL	EAX, #EVENT_CBUTUP	;Yes
	CALL	genevent
10$:	TESTB	mt_pressed.B[EBP], #10h	;Right button pressed?
	JE	12$.S			;No
	MOVL	EAX, #EVENT_RBUTDN	;Yes
	CALL	genevent
12$:	TESTB	mt_released.B[EBP], #10h ;Right button released?
	JE	mouseloop		;No
	MOVL	EAX, #EVENT_RBUTUP	;Yes
	CALL	genevent
	JMP	mouseloop
.PAGE
;Subroutine to generate mouse event call
;	c{EAX} = Event type
;	c{ESI} = Time of the event
;	CALL	genevent

genevent:
	PUSHL	EAX
	PUSHL	EDI
	PUSHL	xwscursorx#
	PUSHL	xwscursory#
	PUSHL	ESI
	PUSHL	#0.B
	CALLI	win_bassysevent[EDI]
	RET
.PAGE
	.SBTTL	Data

	.PSECT	_DATA_p

buttons:.LONG	0

mouseparms:
	.BYTE	PAR$SET|REP_HEXV, 1
	.WORD	IOPAR_TRMHUVECT
mousevect:
	.BYTE	0
	.BYTE	PAR$SET|REP_HEXV, 1
	.WORD	IOPAR_TRMCPVECT
	.BYTE	72t
	.BYTE	0

mousechars:
	DCHAR	MOUSE, GET, TEXT, 20t
mname:	.BLKB	20t
	.BYTE	0

	.END
