	.file	"dir.c"
	.file 1 "dir.c"
	.section	.debug_abbrev,""
Ldebug_abbrev0:
	.section	.debug_info,""
Ldebug_info0:
	.section	.debug_line,""
Ldebug_line0:
	.section .text
Ltext0:
	.file 2 "inc/baseline.h"
	.file 3 "inc/STDIO.H"
	.file 4 "inc/STDLIB.H"
	.file 5 "inc/TIME.H"
	.file 6 "inc/XOSTRM.H"
	.file 7 "inc/XOS.H"
	.file 8 "inc/XOSTIME.H"
	.file 9 "inc/PROGARG.H"
	.file 10 "inc/PROGHELP.H"
	.file 11 "inc/DIRSCAN.H"
.globl _fileparm
	.section .data
	.p2align 5
_fileparm:
	.byte	-126
	.byte	4
	.word	1
	.long	2893888
	.byte	79
	.byte	0
	.word	2
	.long	0
	.word	0
	.word	0
	.word	0
	.space 2
	.byte	-126
	.byte	2
	.word	258
	.word	144
	.byte	66
	.byte	2
	.word	259
	.word	0
	.byte	-126
	.byte	4
	.word	257
	.long	0
	.byte	66
	.byte	4
	.word	3
	.long	0
	.byte	65
	.byte	4
	.word	265
	.long	0
	.byte	65
	.byte	4
	.word	266
	.long	0
	.byte	66
	.byte	8
	.word	270
	.long	0
	.space 4
	.byte	66
	.byte	8
	.word	271
	.long	0
	.space 4
	.byte	66
	.byte	8
	.word	269
	.long	0
	.space 4
	.byte	66
	.byte	4
	.word	272
	.long	0
	.byte	79
	.byte	0
	.word	273
	.long	_owner
	.word	0
	.word	36
	.word	36
	.space 2
	.space 4
.globl _dsd
	.p2align 5
_dsd:
	.long	_fileparm
	.byte	0
	.byte	1
	.byte	2
	.byte	1
	.long	0
	.long	_procfile
	.long	_errormsg
	.space 436
	.section .text
LC0:
	.ascii "T*OTALS\0"
LC1:
	.ascii "Show statistical totals\0"
LC2:
	.ascii "NOT*OTALS\0"
LC3:
	.ascii "Do not show totals\0"
LC4:
	.ascii "P*ATH\0"
LC5:
	.ascii "Show path\0"
LC6:
	.ascii "NOP*ATH\0"
LC7:
	.ascii "Do not show path\0"
LC8:
	.ascii "V*OLUME\0"
LC9:
	.ascii "Show device volume\0"
LC10:
	.ascii "NOV*OLUME\0"
LC11:
	.ascii "Do not show device volume\0"
LC12:
	.ascii "AT*TRIBUTES\0"
LC13:
	.ascii "Show file attributes\0"
LC14:
	.ascii "NOAT*TRIBUTES\0"
LC15:
	.ascii "Do not show file attributes\0"
LC16:
	.ascii "DOS*NAME\0"
LC17:
	.ascii "Show DOS (8x3) names\0"
LC18:
	.ascii "NODOS*NAME\0"
LC19:
	.ascii "Do not show DOS (8x3) names\0"
LC20:
	.ascii "L*ONGNAME\0"
LC21:
	.ascii "Show long names\0"
LC22:
	.ascii "NOL*ONGNAME\0"
LC23:
	.ascii "Do not show long names\0"
LC24:
	.ascii "D*OWN\0"
LC25:
	.ascii "Sort in columns\0"
LC26:
	.ascii "A*CROSS\0"
LC27:
	.ascii "Sort in rows\0"
LC28:
	.ascii "E*XPAND\0"
	.p2align 5
LC29:
	.ascii "Expand column widths for longest name\0"
LC30:
	.ascii "NOE*XPAND\0"
LC31:
	.ascii "Do not expand column widths\0"
.globl _discmd
	.section .data
	.p2align 5
_discmd:
	.long	LC0
	.long	LC1
	.long	LC2
	.long	LC3
	.long	LC4
	.long	LC5
	.long	LC6
	.long	LC7
	.long	LC8
	.long	LC9
	.long	LC10
	.long	LC11
	.long	LC12
	.long	LC13
	.long	LC14
	.long	LC15
	.long	LC16
	.long	LC17
	.long	LC18
	.long	LC19
	.long	LC20
	.long	LC21
	.long	LC22
	.long	LC23
	.long	LC24
	.long	LC25
	.long	LC26
	.long	LC27
	.long	LC28
	.long	LC29
	.long	LC30
	.long	LC31
	.long	0
	.long	0
	.section .text
LC32:
	.ascii "NO*NE\0"
LC33:
	.ascii "Do not sort\0"
LC34:
	.ascii "A*SCENDING\0"
LC35:
	.ascii "Sort in ascending order\0"
LC36:
	.ascii "R*EVERSE\0"
LC37:
	.ascii "Sort in reverse order\0"
LC38:
	.ascii "N*AME\0"
LC39:
	.ascii "Sort by name\0"
LC40:
	.ascii "E*XTENSION\0"
LC41:
	.ascii "Sort by extension\0"
LC42:
	.ascii "D*ATE\0"
LC43:
	.ascii "Sort by date\0"
LC44:
	.ascii "SI*ZE\0"
LC45:
	.ascii "Sort by size\0"
LC46:
	.ascii "DI*RFIRST\0"
LC47:
	.ascii "Sort directories first\0"
LC48:
	.ascii "NODI*RFIRST\0"
LC49:
	.ascii "Do not sort directories first\0"
.globl _sortcmd
	.section .data
	.p2align 5
_sortcmd:
	.long	LC32
	.long	LC33
	.long	LC34
	.long	LC35
	.long	LC36
	.long	LC37
	.long	LC38
	.long	LC39
	.long	LC40
	.long	LC41
	.long	LC42
	.long	LC43
	.long	LC44
	.long	LC45
	.long	LC46
	.long	LC47
	.long	LC48
	.long	LC49
	.long	0
	.long	0
.globl _shownames
	.p2align 2
_shownames:
	.long	1
.globl _debug
	.section	.bss
	.p2align 2
_debug:
	.space 4
.globl _page
	.p2align 2
_page:
	.space 4
.globl _lsum
	.p2align 2
_lsum:
	.space 4
.globl _onlytotals
	.p2align 2
_onlytotals:
	.space 4
	.section .text
LC50:
	.ascii "AL*L\0"
LC51:
	.ascii "Show all files\0"
LC52:
	.ascii "DE*BUG\0"
LC53:
	.ascii "DIR\0"
LC54:
	.ascii "Show directories only\0"
LC55:
	.ascii "NODIR\0"
LC56:
	.ascii "Do not show directories\0"
LC57:
	.ascii "DI*SPLAY\0"
LC58:
	.ascii "Specify display options\0"
LC59:
	.ascii "F*ULL\0"
LC60:
	.ascii "Display full listing\0"
LC61:
	.ascii "H*ELP\0"
LC62:
	.ascii "Display this message\0"
LC63:
	.ascii "L*ONG\0"
LC64:
	.ascii "Show file details\0"
LC65:
	.ascii "N*AMES\0"
LC66:
	.ascii "Show file names\0"
LC67:
	.ascii "O*NE\0"
	.p2align 5
LC68:
	.ascii "Display single column, short listing\0"
LC69:
	.ascii "P*AUSE\0"
LC70:
	.ascii "Pause at end of page\0"
LC71:
	.ascii "SH*ORT\0"
LC72:
	.ascii "Display short listing\0"
LC73:
	.ascii "SO*RT\0"
LC74:
	.ascii "Specify output order\0"
LC75:
	.ascii "Show totals only (no names)\0"
LC76:
	.ascii "W*IDE\0"
LC77:
	.ascii "X\0"
LC78:
	.ascii "Specify files to exclude\0"
LC79:
	.ascii "?\0"
.globl _options
	.section .data
	.p2align 5
_options:
	.long	LC50
	.long	0
	.long	0
	.space 12
	.long	_allhave
	.long	151
	.space 4
	.long	LC51
	.long	LC52
	.long	10
	.long	0
	.space 12
	.long	_debug
	.long	1
	.space 4
	.long	0
	.long	LC53
	.long	0
	.long	0
	.space 12
	.long	_optdir
	.long	0
	.space 4
	.long	LC54
	.long	LC55
	.long	0
	.long	0
	.space 12
	.long	_optnodir
	.long	0
	.space 4
	.long	LC56
	.long	LC57
	.long	2048
	.long	_discmd
	.space 12
	.long	_optdisplay
	.long	0
	.space 4
	.long	LC58
	.long	LC59
	.long	0
	.long	0
	.space 12
	.long	_optfull
	.long	0
	.space 4
	.long	LC60
	.long	LC61
	.long	0
	.long	0
	.space 12
	.long	_opthelp
	.long	0
	.space 4
	.long	LC62
	.long	LC63
	.long	0
	.long	0
	.space 12
	.long	_optlong
	.long	0
	.space 4
	.long	LC64
	.long	LC65
	.long	10
	.long	0
	.space 12
	.long	_shownames
	.long	1
	.space 4
	.long	LC66
	.long	LC67
	.long	0
	.long	0
	.space 12
	.long	_optone
	.long	0
	.space 4
	.long	LC68
	.long	LC69
	.long	10
	.long	0
	.space 12
	.long	_page
	.long	1
	.space 4
	.long	LC70
	.long	LC71
	.long	0
	.long	0
	.space 12
	.long	_optshort
	.long	0
	.space 4
	.long	LC72
	.long	LC73
	.long	2048
	.long	_sortcmd
	.space 12
	.long	_optsort
	.long	0
	.space 4
	.long	LC74
	.long	LC0
	.long	10
	.long	0
	.space 12
	.long	_onlytotals
	.long	1
	.space 4
	.long	LC75
	.long	LC76
	.long	0
	.long	0
	.space 12
	.long	_optshort
	.long	0
	.space 4
	.long	LC72
	.long	LC77
	.long	36864
	.long	0
	.space 12
	.long	_optx
	.long	0
	.space 4
	.long	LC78
	.long	LC79
	.long	0
	.long	0
	.space 12
	.long	_opthelp
	.long	0
	.space 4
	.long	LC62
	.long	0
	.long	0
	.long	0
	.space 12
	.long	0
	.long	0
	.space 4
	.long	0
.globl _longfmt
	.section	.bss
_longfmt:
	.space 1
.globl _fullfmt
_fullfmt:
	.space 1
.globl _prtvolid
_prtvolid:
	.space 1
.globl _prtpath
_prtpath:
	.space 1
.globl _showattr
	.section .data
_showattr:
	.byte	1
.globl _onecol
	.section	.bss
_onecol:
	.space 1
.globl _datesort
_datesort:
	.space 1
.globl _extsort
_extsort:
	.space 1
.globl _revsort
_revsort:
	.space 1
.globl _dirsort
_dirsort:
	.space 1
.globl _sizesort
_sizesort:
	.space 1
.globl _nosort
_nosort:
	.space 1
.globl _prtacross
	.section .data
_prtacross:
	.byte	1
.globl _prtexpand
	.section	.bss
_prtexpand:
	.space 1
.globl _prtnamedos
_prtnamedos:
	.space 1
.globl _prtnamelong
	.section .data
_prtnamelong:
	.byte	1
.globl _needlength
	.p2align 2
_needlength:
	.long	-1
.globl _needalloc
	.p2align 2
_needalloc:
	.long	-1
.globl _needcdate
	.p2align 2
_needcdate:
	.long	-1
.globl _needmdate
	.p2align 2
_needmdate:
	.long	-1
.globl _needadate
	.p2align 2
_needadate:
	.long	-1
.globl _needowner
	.p2align 2
_needowner:
	.long	-1
.globl _needprot
	.p2align 2
_needprot:
	.long	-1
.globl _neednamedos
	.p2align 2
_neednamedos:
	.long	-1
.globl _needname
	.section	.bss
	.p2align 2
_needname:
	.space 4
.globl _curline
	.section .data
	.p2align 2
_curline:
	.long	-1
.globl _numlisted
	.section	.bss
	.p2align 2
_numlisted:
	.space 4
.globl _copymsg
	.section .data
_copymsg:
	.ascii "\0"
.globl _prgname
_prgname:
	.ascii "DIR\0"
.globl _envname
_envname:
	.ascii "^XOS^DIR^OPT\0"
.globl _example
_example:
	.ascii "{/options} filespec\0"
.globl _description
	.p2align 5
_description:
	.ascii "This command produces a directory listing of the files on a specified disk drive.  The file specification given determines which files are included in the listing.  Wildcard and elipsis notation are allowed in the file specification.  If no file specification is given, *.* is assumed.  More than one file and/or wildcard may be specified on the command line.  Many options are possible to the directory listing.  It is recommended that the user select a preferred directory format using command line options and enter it as the default in the USTARTUP.BAT file using the DEFAULT command.\0"
.globl _snglparm
	.p2align 2
_snglparm:
	.byte	66
	.byte	4
	.word	513
	.long	0
	.byte	-126
	.byte	4
	.word	514
	.long	2
	.byte	-126
	.byte	4
	.word	513
	.long	16
	.space 4
.globl _normparm
	.p2align 2
_normparm:
	.byte	-126
	.byte	4
	.word	514
	.long	-1
	.byte	-126
	.byte	4
	.word	513
	.long	0
	.space 4
.globl _diparm
	.p2align 2
_diparm:
	.byte	65
	.byte	4
	.word	770
	.space 4
	.byte	65
	.byte	4
	.word	771
	.space 4
	.byte	65
	.byte	4
	.word	773
	.space 4
	.space 4
.globl _lblqab
	.p2align 5
_lblqab:
	.word	-32748
	.word	0
	.long	0
	.long	0
	.long	0
	.word	0
	.byte	0
	.byte	0
	.long	1
	.long	60
	.long	_vollabel
	.word	0
	.space 2
	.long	0
	.word	0
	.space 2
	.long	0
	.word	0
	.space 2
	.section .text
LC80:
	.ascii "*\0"
LC81:
	.ascii "*.*\0"
LC82:
	.ascii "y\0"
LC83:
	.ascii "ies\0"
	.p2align 5
LC84:
	.ascii "[%,d director%s listed containing a grand total of\12\0"
LC85:
	.ascii " %,ld director%s,\0"
LC86:
	.ascii "\0"
LC87:
	.ascii "s\0"
LC88:
	.ascii " %,ld file%s\0"
	.p2align 5
LC89:
	.ascii ", %,lld byte%s written, %,lld byte%s allocated\0"
LC90:
	.ascii "]\12\0"
LC91:
	.ascii "? DIR: File not found\12\0"
	.p2align 5
LC92:
	.ascii "Maximum memory: %,ld, current memory: %,ld\12\0"
.globl _main
_main:
LFB3:
	.loc 1 472 0
	pushl	%ebp
LCFI0:
	movl	%esp, %ebp
LCFI1:
	subl	$340, %esp
LCFI2:
	.loc 1 481 0
LBB2:
	pushl	$_pib
LCFI3:
	call	_reg_pib
	addl	$4, %esp
	.loc 1 483 0
LCFI4:
	call	_init_Vars
	.loc 1 487 0
	pushl	$1
LCFI5:
	call	_global_parameter
	addl	$4, %esp
	.loc 1 489 0
	movb	$1, _prtvolid
	.loc 1 490 0
	movb	$1, _prtpath
	.loc 1 491 0
	movb	$0, _nosort
	.loc 1 492 0
	movl	$1, _lsum
	.loc 1 496 0
	pushl	$0
	pushl	$256
	leal	-276(%ebp), %eax
	pushl	%eax
	pushl	$0
	pushl	$_envname
	pushl	$0
LCFI6:
	call	_svcSysFindEnv
	addl	$24, %esp
	testl	%eax, %eax
	jle	L2
	.loc 1 498 0
	leal	-276(%ebp), %eax
	movl	%eax, -12(%ebp)
	.loc 1 499 0
	movl	$0, -8(%ebp)
	.loc 1 500 0
	pushl	$0
	pushl	$0
	pushl	$0
	pushl	$0
	pushl	$0
	pushl	$_options
	pushl	$0
	leal	-12(%ebp), %eax
	pushl	%eax
LCFI7:
	call	_progarg
	addl	$32, %esp
L2:
	.loc 1 506 0
	cmpl	$1, 8(%ebp)
	jle	L3
	.loc 1 508 0
	leal	12(%ebp), %eax
	addl	$4, (%eax)
	.loc 1 509 0
	pushl	$0
	pushl	$0
	pushl	$0
	pushl	$_nonopt
	pushl	$0
	pushl	$_options
	pushl	$0
	pushl	12(%ebp)
	call	_progarg
	addl	$32, %esp
L3:
	.loc 1 515 0
	cmpl	$0, _onlytotals
	je	L4
	.loc 1 517 0
	movl	$1, _lsum
	.loc 1 518 0
	movl	$0, _shownames
L4:
	.loc 1 520 0
	cmpl	$0, _gcp_dosdrive
	je	L5
	.loc 1 521 0
	movl	$4999232, _fileparm+4
L5:
	.loc 1 523 0
	cmpb	$0, _prtnamedos
	je	L6
	.loc 1 524 0
	orl	$256, _fileparm+4
L6:
	.loc 1 525 0
	cmpb	$0, _prtnamedos
	je	L8
	cmpb	$0, _prtnamelong
	jne	L8
	jmp	L7
L8:
	.loc 1 526 0
	orl	$640, _fileparm+4
L7:
	.loc 1 527 0
	cmpl	$0, _firstspec
	jne	L9
	.loc 1 528 0
	pushl	$LC80
LCFI8:
	call	_nonopt
	addl	$4, %esp
L9:
	.loc 1 529 0
	cmpl	$0, _pib
	jne	L10
	.loc 1 530 0
	movl	$0, _page
L10:
	.loc 1 531 0
	cmpb	$0, _datesort
	je	L11
	.loc 1 533 0
	cmpl	$0, _needcdate
	jns	L21
	.loc 1 535 0
	cmpl	$0, _needalloc
	js	L13
	movl	$4, -292(%ebp)
	jmp	L14
L13:
	movl	$0, -292(%ebp)
L14:
	movl	-292(%ebp), %eax
	movl	%eax, -288(%ebp)
	cmpl	$0, _needprot
	js	L15
	addl	$4, -288(%ebp)
L15:
	movl	-288(%ebp), %eax
	movl	%eax, -284(%ebp)
	cmpl	$0, _needowner
	js	L16
	addl	$36, -284(%ebp)
L16:
	movl	-284(%ebp), %eax
	movl	%eax, -280(%ebp)
	cmpl	$0, _needadate
	js	L17
	addl	$8, -280(%ebp)
L17:
	movl	-280(%ebp), %eax
	movl	%eax, -296(%ebp)
	cmpl	$0, _needmdate
	js	L18
	addl	$8, -296(%ebp)
L18:
	movl	-296(%ebp), %eax
	movl	%eax, _needcdate
	.loc 1 538 0
	cmpl	$0, _needlength
	js	L19
	.loc 1 539 0
	addl	$8, _needlength
L19:
	.loc 1 540 0
	cmpl	$0, _neednamedos
	js	L20
	.loc 1 541 0
	addl	$8, _neednamedos
L20:
	.loc 1 542 0
	addl	$8, _needname
	jmp	L21
L11:
	.loc 1 545 0
	cmpb	$0, _sizesort
	je	L21
	.loc 1 547 0
	cmpl	$0, _needlength
	jns	L21
	.loc 1 549 0
	cmpl	$0, _needalloc
	js	L24
	movl	$4, -316(%ebp)
	jmp	L25
L24:
	movl	$0, -316(%ebp)
L25:
	movl	-316(%ebp), %eax
	movl	%eax, -312(%ebp)
	cmpl	$0, _needprot
	js	L26
	addl	$4, -312(%ebp)
L26:
	movl	-312(%ebp), %eax
	movl	%eax, -308(%ebp)
	cmpl	$0, _needowner
	js	L27
	addl	$36, -308(%ebp)
L27:
	movl	-308(%ebp), %eax
	movl	%eax, -304(%ebp)
	cmpl	$0, _needadate
	js	L28
	addl	$8, -304(%ebp)
L28:
	movl	-304(%ebp), %eax
	movl	%eax, -300(%ebp)
	cmpl	$0, _needmdate
	js	L29
	addl	$8, -300(%ebp)
L29:
	movl	-300(%ebp), %eax
	movl	%eax, -320(%ebp)
	cmpl	$0, _needcdate
	js	L30
	addl	$8, -320(%ebp)
L30:
	movl	-320(%ebp), %eax
	movl	%eax, _needlength
	.loc 1 552 0
	cmpl	$0, _neednamedos
	js	L31
	.loc 1 553 0
	addl	$8, _neednamedos
L31:
	.loc 1 554 0
	addl	$8, _needname
L21:
	.loc 1 557 0
	cmpb	$0, _prtnamedos
	je	L32
	cmpb	$0, _prtnamelong
	je	L32
	.loc 1 559 0
	movl	_needname, %eax
	movl	%eax, _neednamedos
	.loc 1 560 0
	addl	$16, _needname
L32:
	.loc 1 562 0
	cmpl	$0, _needname
	je	L34
	cmpl	$0, _neednamedos
	jne	L33
L34:
	.loc 1 564 0
	movb	$1, _dsd+4
	.loc 1 565 0
	movb	$0, _fileparm+44
L33:
	.loc 1 567 0
	cmpb	$0, _longfmt
	jne	L35
	cmpb	$0, _fullfmt
	jne	L35
	cmpb	$0, _prtnamedos
	je	L35
	.loc 1 568 0
	movb	$0, _prtnamelong
L35:
	.loc 1 572 0
	movl	_firstspec, %eax
	movl	%eax, _thisspec
L36:
	.loc 1 575 0
	movl	_thisspec, %eax
	cmpl	$0, 4(%eax)
	je	L39
	.loc 1 577 0
	movl	_thisspec, %eax
	movl	_thisspec, %edx
	movl	32(%eax), %eax
	subl	%edx, %eax
	subl	$36, %eax
	movl	%eax, -20(%ebp)
	.loc 1 578 0
	movl	_thisspec, %eax
	pushl	32(%eax)
	call	_strlen
	addl	$4, %esp
	addl	-20(%ebp), %eax
	addl	$4, %eax
	pushl	%eax
	call	_getmem
	addl	$4, %esp
	movl	%eax, -4(%ebp)
	.loc 1 579 0
	pushl	$LC81
	pushl	-20(%ebp)
	movl	_thisspec, %eax
	addl	$36, %eax
	pushl	%eax
	pushl	-4(%ebp)
LCFI9:
	call	_strnmov
	addl	$12, %esp
	pushl	%eax
LCFI10:
	call	_strmov
	addl	$8, %esp
	.loc 1 580 0
	pushl	$_dsd
	pushl	-4(%ebp)
	call	_dirscan
	addl	$8, %esp
	.loc 1 581 0
	pushl	-4(%ebp)
LCFI11:
	call	_free
	addl	$4, %esp
	jmp	L40
L39:
	.loc 1 584 0
	pushl	$_dsd
	movl	_thisspec, %eax
	addl	$36, %eax
	pushl	%eax
LCFI12:
	call	_dirscan
	addl	$8, %esp
L40:
	.loc 1 585 0
	cmpl	$0, _numfiles
	je	L41
	.loc 1 586 0
LCFI13:
	call	_printfiles
L41:
	.loc 1 587 0
	movl	_thisspec, %eax
	movl	(%eax), %eax
	movl	%eax, -16(%ebp)
	.loc 1 588 0
	pushl	_thisspec
LCFI14:
	call	_free
	addl	$4, %esp
	.loc 1 589 0
	movl	-16(%ebp), %eax
	movl	%eax, _thisspec
	testl	%eax, %eax
	jne	L36
	.loc 1 593 0
	cmpl	$0, _lsum
	je	L43
	cmpl	$1, _numlisted
	jle	L43
	.loc 1 595 0
LCFI15:
	call	_pagecheck
	.loc 1 596 0
	pushl	$10
LCFI16:
	call	_putchar
	addl	$4, %esp
	.loc 1 597 0
LCFI17:
	call	_pagecheck
	.loc 1 598 0
	cmpl	$1, _numlisted
	jne	L44
	movl	$LC82, -324(%ebp)
	jmp	L45
L44:
	movl	$LC83, -324(%ebp)
L45:
	pushl	-324(%ebp)
	pushl	_numlisted
	pushl	$LC84
LCFI18:
	call	_printf
	addl	$12, %esp
	.loc 1 600 0
	leal	-276(%ebp), %eax
	movl	%eax, -4(%ebp)
	.loc 1 601 0
	cmpl	$0, _gdircnt
	je	L46
	.loc 1 602 0
	cmpl	$1, _gdircnt
	jne	L47
	movl	$LC82, -328(%ebp)
	jmp	L48
L47:
	movl	$LC83, -328(%ebp)
L48:
	pushl	-328(%ebp)
	pushl	_gdircnt
	pushl	$LC85
	pushl	-4(%ebp)
LCFI19:
	call	_sprintf
	addl	$16, %esp
	movl	%eax, %edx
	leal	-4(%ebp), %eax
	addl	%edx, (%eax)
L46:
	.loc 1 604 0
	cmpl	$1, _gfilecnt
	jne	L49
	movl	$LC86, -332(%ebp)
	jmp	L50
L49:
	movl	$LC87, -332(%ebp)
L50:
	pushl	-332(%ebp)
	pushl	_gfilecnt
	pushl	$LC88
	pushl	-4(%ebp)
	call	_sprintf
	addl	$16, %esp
	movl	%eax, %edx
	leal	-4(%ebp), %eax
	addl	%edx, (%eax)
	.loc 1 605 0
	cmpb	$0, _longfmt
	je	L51
	.loc 1 606 0
	cmpl	$1, _gtotalalloc
	jne	L52
	cmpl	$0, _gtotalalloc+4
	jne	L52
	movl	$LC86, -336(%ebp)
	jmp	L53
L52:
	movl	$LC87, -336(%ebp)
L53:
	pushl	-336(%ebp)
	pushl	_gtotalalloc+4
	pushl	_gtotalalloc
	cmpl	$1, _gtotallen
	jne	L54
	cmpl	$0, _gtotallen+4
	jne	L54
	movl	$LC86, -340(%ebp)
	jmp	L55
L54:
	movl	$LC87, -340(%ebp)
L55:
	pushl	-340(%ebp)
	pushl	_gtotallen+4
	pushl	_gtotallen
	pushl	$LC89
	pushl	-4(%ebp)
LCFI20:
	call	_sprintf
	addl	$32, %esp
	movl	%eax, %edx
	leal	-4(%ebp), %eax
	addl	%edx, (%eax)
L51:
	.loc 1 610 0
LCFI21:
	call	_pagecheck
	.loc 1 611 0
	pushl	$LC90
	pushl	-4(%ebp)
LCFI22:
	call	_strmov
	addl	$8, %esp
	.loc 1 612 0
	pushl	$__stdout
	leal	-276(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
L43:
	.loc 1 614 0
	cmpb	$0, _errdone
	jne	L56
	movl	_gfilecnt, %eax
	addl	_gdircnt, %eax
	testl	%eax, %eax
	jne	L56
	.loc 1 615 0
	pushl	$__stderr
	pushl	$LC91
	call	_fputs
	addl	$8, %esp
L56:
	.loc 1 616 0
	cmpl	$0, _debug
	je	L57
	.loc 1 618 0
LCFI23:
	call	_pagecheck
	.loc 1 619 0
	pushl	$10
LCFI24:
	call	_putchar
	addl	$4, %esp
	.loc 1 620 0
	pushl	__malloc_amount
	pushl	_maxmem
	pushl	$LC92
LCFI25:
	call	_printf
	addl	$12, %esp
L57:
	.loc 1 623 0
	pushl	$0
LCFI26:
	call	_exit
	.loc 1 624 0
LBE2:
LFE3:
LC93:
	.ascii "Dec 12 2003\0"
.globl _init_Vars
_init_Vars:
LFB5:
	.loc 1 633 0
	pushl	%ebp
LCFI27:
	movl	%esp, %ebp
LCFI28:
	.loc 1 636 0
	movl	$_options, _pib+144
	.loc 1 637 0
	movl	$0, _pib+148
	.loc 1 638 0
	movl	$LC93, _pib+132
	.loc 1 639 0
	movl	$3, _pib+4
	.loc 1 640 0
	movl	$8, _pib+8
	.loc 1 641 0
	movl	$_copymsg, _pib+124
	.loc 1 642 0
	movl	$_prgname, _pib+128
	.loc 1 643 0
	movl	$_description, _pib+136
	.loc 1 644 0
	movl	$_example, _pib+140
	.loc 1 645 0
	movl	$0, _pib+12
	.loc 1 646 0
	call	_getTrmParms
	.loc 1 647 0
	call	_getHelpClr
	.loc 1 648 0
	popl	%ebp
	ret
LFE5:
.globl _allhave
_allhave:
LFB7:
	.loc 1 657 0
	pushl	%ebp
LCFI29:
	movl	%esp, %ebp
LCFI30:
	.loc 1 658 0
	movl	8(%ebp), %eax
	movw	8(%eax), %ax
	movw	%ax, _fileparm+28
	.loc 1 659 0
	movl	$1, %eax
	.loc 1 660 0
	popl	%ebp
	ret
LFE7:
.globl _procfile
_procfile:
LFB9:
	.loc 1 672 0
	pushl	%ebp
LCFI31:
	movl	%esp, %ebp
LCFI32:
	subl	$276, %esp
LCFI33:
	.loc 1 678 0
LBB3:
	movb	_dsd+20, %al
	orb	%al, _changed
	.loc 1 679 0
	movl	_thisspec, %eax
	cmpl	$0, 4(%eax)
	je	L61
	.loc 1 681 0
	movl	_thisspec, %eax
	movl	%eax, -8(%ebp)
L62:
	.loc 1 684 0
	movl	-8(%ebp), %eax
	movl	24(%eax), %eax
	shrl	$19, %eax
	andl	$1, %eax
	pushl	%eax
	pushl	_dsd+152
	movl	-8(%ebp), %eax
	pushl	32(%eax)
LCFI34:
	call	_wildcmp
	addl	$12, %esp
	testl	%eax, %eax
	jne	L64
	.loc 1 686 0
	jmp	L63
L64:
	.loc 1 687 0
	movl	-8(%ebp), %eax
	movl	4(%eax), %eax
	movl	%eax, -8(%ebp)
	testl	%eax, %eax
	jne	L62
L63:
	.loc 1 688 0
	cmpl	$0, -8(%ebp)
	jne	L61
	.loc 1 689 0
	movl	$1, -276(%ebp)
	jmp	L60
L61:
	.loc 1 691 0
	cmpl	$0, _firstname
	je	L68
	.loc 1 693 0
	movl	_firstname, %eax
	movl	%eax, -12(%ebp)
L69:
	.loc 1 696 0
	movl	-8(%ebp), %eax
	movl	24(%eax), %eax
	shrl	$19, %eax
	andl	$1, %eax
	pushl	%eax
	pushl	_dsd+152
	movl	-12(%ebp), %eax
	addl	$4, %eax
	pushl	%eax
	call	_wildcmp
	addl	$12, %esp
	testl	%eax, %eax
	jne	L71
	.loc 1 698 0
	movl	$1, -276(%ebp)
	jmp	L60
L71:
	.loc 1 699 0
	movl	-12(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, -12(%ebp)
	testl	%eax, %eax
	jne	L69
L68:
	.loc 1 705 0
	cmpb	$0, _changed
	je	L74
	cmpl	$0, _numfiles
	je	L74
	.loc 1 706 0
LCFI35:
	call	_printfiles
L74:
	.loc 1 707 0
	cmpl	$0, _dsd+220
	jns	L75
	cmpb	$0, _fullfmt
	jne	L77
	cmpb	$0, _longfmt
	jne	L77
	jmp	L76
L77:
	cmpl	$-41, _dsd+220
	je	L75
	cmpl	$-81, _dsd+220
	je	L75
	cmpl	$-81, _dsd+220
	je	L75
	cmpl	$-84, _dsd+220
	je	L75
	cmpl	$-40, _dsd+220
	jne	L76
	jmp	L75
L76:
	.loc 1 714 0
LBB4:
	pushl	$_dsd+28
	leal	-272(%ebp), %eax
	pushl	%eax
LCFI36:
	call	_strmov
	addl	$8, %esp
	movl	%eax, -4(%ebp)
	.loc 1 715 0
	movl	$254, %eax
	subl	_dsd+60, %eax
	movl	%eax, -16(%ebp)
	.loc 1 716 0
	pushl	-16(%ebp)
	pushl	_dsd+68
	pushl	-4(%ebp)
LCFI37:
	call	_strnmov
	addl	$12, %esp
	movl	%eax, -4(%ebp)
	.loc 1 717 0
	movl	_dsd+72, %edx
	leal	-16(%ebp), %eax
	subl	%edx, (%eax)
	movl	-16(%ebp), %eax
	testl	%eax, %eax
	jle	L78
	.loc 1 719 0
	pushl	-16(%ebp)
	pushl	$_dsd+84
	pushl	-4(%ebp)
	call	_strnmov
	addl	$12, %esp
	movl	%eax, -4(%ebp)
	.loc 1 720 0
	movl	_dsd+116, %edx
	leal	-16(%ebp), %eax
	subl	%edx, (%eax)
	movl	-16(%ebp), %eax
	testl	%eax, %eax
	jle	L78
	.loc 1 722 0
	pushl	-16(%ebp)
	pushl	_dsd+120
	pushl	-4(%ebp)
	call	_strnmov
	addl	$12, %esp
	movl	%eax, -4(%ebp)
	.loc 1 723 0
	movl	_dsd+124, %edx
	leal	-16(%ebp), %eax
	subl	%edx, (%eax)
	movl	-16(%ebp), %eax
	testl	%eax, %eax
	jle	L78
	.loc 1 724 0
	pushl	-16(%ebp)
	pushl	_dsd+152
	pushl	-4(%ebp)
	call	_strnmov
	addl	$12, %esp
	movl	%eax, -4(%ebp)
L78:
	.loc 1 727 0
	movb	$0, -17(%ebp)
	.loc 1 728 0
	pushl	_dsd+220
	leal	-272(%ebp), %eax
	pushl	%eax
LCFI38:
	call	_errormsg
	addl	$8, %esp
	.loc 1 729 0
	movl	$0, -276(%ebp)
	jmp	L60
L75:
	.loc 1 731 0
LBE4:
	movl	_dsd+172, %eax
	addl	_dsd+156, %eax
	testl	%eax, %eax
	jne	L81
	.loc 1 732 0
	movl	$1, -276(%ebp)
	jmp	L60
L81:
	.loc 1 733 0
	cmpb	$0, _changed
	je	L82
	.loc 1 734 0
LCFI39:
	call	_printheading
L82:
	.loc 1 735 0
	movb	$0, _changed
	.loc 1 736 0
	movswl	_fileparm+28,%eax
	andl	$2, %eax
	testl	%eax, %eax
	jne	L83
	cmpl	$0, _dsd+156
	jle	L83
	cmpl	$2, _dsd+156
	jg	L83
	movl	_dsd+152, %eax
	cmpb	$46, (%eax)
	jne	L83
	cmpl	$1, _dsd+156
	je	L84
	movl	_dsd+152, %eax
	incl	%eax
	cmpb	$46, (%eax)
	je	L84
	jmp	L83
L84:
	.loc 1 741 0
	movl	$1, -276(%ebp)
	jmp	L60
L83:
	.loc 1 743 0
	cmpb	$0, _prtnamelong
	je	L85
	.loc 1 745 0
	movl	_needname, %eax
	addl	_dsd+156, %eax
	addl	$19, %eax
	pushl	%eax
LCFI40:
	call	_getmem
	addl	$4, %esp
	movl	%eax, _thisfile
	.loc 1 747 0
	movl	_dsd+156, %eax
	movl	%eax, _namesize
	.loc 1 748 0
	movswl	_dsd+22,%eax
	andl	$16, %eax
	testl	%eax, %eax
	je	L86
	.loc 1 749 0
	incl	_namesize
L86:
	.loc 1 750 0
	movl	_needed, %eax
	cmpl	_namesize, %eax
	jge	L87
	.loc 1 751 0
	movl	_namesize, %eax
	movl	%eax, _needed
L87:
	.loc 1 753 0
	pushl	_dsd+152
	movl	_needname, %eax
	addl	_thisfile, %eax
	addl	$16, %eax
	pushl	%eax
LCFI41:
	call	_strmov
	addl	$8, %esp
	.loc 1 755 0
	cmpl	$0, _neednamedos
	js	L89
	.loc 1 756 0
	pushl	_dsd+168
	movl	_neednamedos, %eax
	addl	_thisfile, %eax
	addl	$16, %eax
	pushl	%eax
	call	_strmov
	addl	$8, %esp
	jmp	L89
L85:
	.loc 1 760 0
	movl	_needname, %eax
	addl	_dsd+172, %eax
	addl	$19, %eax
	pushl	%eax
LCFI42:
	call	_getmem
	addl	$4, %esp
	movl	%eax, _thisfile
	.loc 1 762 0
	pushl	_dsd+168
	movl	_needname, %eax
	addl	_thisfile, %eax
	addl	$16, %eax
	pushl	%eax
LCFI43:
	call	_strmov
	addl	$8, %esp
L89:
	.loc 1 764 0
	incl	_numfiles
	.loc 1 765 0
	movl	_thisfile, %edx
	movl	_dsd+220, %eax
	movl	%eax, 12(%edx)
	.loc 1 767 0
	cmpl	$0, _firstfile
	jne	L90
	.loc 1 768 0
	movl	_thisfile, %eax
	movl	%eax, _firstfile
	jmp	L91
L90:
	.loc 1 770 0
	movl	_lastfile, %edx
	movl	_thisfile, %eax
	movl	%eax, (%edx)
L91:
	.loc 1 771 0
	movl	_thisfile, %eax
	movl	%eax, _lastfile
	.loc 1 772 0
	movl	_thisfile, %eax
	movl	$0, (%eax)
	.loc 1 773 0
	cmpl	$0, _needlength
	js	L92
	.loc 1 774 0
	movl	_needlength, %eax
	addl	_thisfile, %eax
	leal	16(%eax), %edx
	movl	_fileparm+56, %eax
	movl	%eax, (%edx)
L92:
	.loc 1 775 0
	cmpl	$0, _needalloc
	js	L93
	.loc 1 776 0
	movl	_needalloc, %eax
	addl	_thisfile, %eax
	leal	16(%eax), %edx
	movl	_fileparm+64, %eax
	movl	%eax, (%edx)
L93:
	.loc 1 777 0
	cmpl	$0, _needcdate
	js	L94
	.loc 1 778 0
	movl	_needcdate, %eax
	addl	_thisfile, %eax
	leal	16(%eax), %ecx
	movl	_fileparm+72, %eax
	movl	_fileparm+76, %edx
	movl	%eax, (%ecx)
	movl	%edx, 4(%ecx)
L94:
	.loc 1 779 0
	cmpl	$0, _needmdate
	js	L95
	.loc 1 780 0
	movl	_needmdate, %eax
	addl	_thisfile, %eax
	leal	16(%eax), %ecx
	movl	_fileparm+84, %eax
	movl	_fileparm+88, %edx
	movl	%eax, (%ecx)
	movl	%edx, 4(%ecx)
L95:
	.loc 1 781 0
	cmpl	$0, _needadate
	js	L96
	.loc 1 782 0
	movl	_needadate, %eax
	addl	_thisfile, %eax
	leal	16(%eax), %ecx
	movl	_fileparm+96, %eax
	movl	_fileparm+100, %edx
	movl	%eax, (%ecx)
	movl	%edx, 4(%ecx)
L96:
	.loc 1 783 0
	cmpl	$0, _needprot
	js	L97
	.loc 1 784 0
	movl	_needprot, %eax
	addl	_thisfile, %eax
	leal	16(%eax), %edx
	movl	_fileparm+108, %eax
	movl	%eax, (%edx)
L97:
	.loc 1 785 0
	cmpl	$0, _needowner
	js	L98
	.loc 1 786 0
	pushl	$_owner
	movl	_needowner, %eax
	addl	_thisfile, %eax
	addl	$16, %eax
	pushl	%eax
	call	_strcpy
	addl	$8, %esp
L98:
	.loc 1 787 0
	movl	_thisfile, %edx
	movb	_dsd+22, %al
	movb	%al, 8(%edx)
	movb	8(%edx), %al
	andl	$255, %eax
	andl	$16, %eax
	testl	%eax, %eax
	je	L99
	.loc 1 788 0
	incl	_dircnt
	jmp	L100
L99:
	.loc 1 790 0
	incl	_filecnt
L100:
	.loc 1 791 0
	cmpl	$0, _dsd+220
	js	L101
	.loc 1 793 0
	pushl	_fileparm+56
	pushl	$_totallen
	call	_longadd
	addl	$8, %esp
	.loc 1 794 0
	pushl	_fileparm+64
	pushl	$_totalalloc
	call	_longadd
	addl	$8, %esp
L101:
	.loc 1 796 0
	movl	$1, -276(%ebp)
	.loc 1 797 0
L60:
LBE3:
	movl	-276(%ebp), %eax
	leave
	ret
LFE9:
LC94:
	.ascii "Volume: %s%s%s\0"
LC95:
	.ascii " (%.36s)\0"
LC96:
	.ascii "  \0"
LC97:
	.ascii "\12\0"
LC98:
	.ascii "Path: %s\0"
.globl _printheading
_printheading:
LFB11:
	.loc 1 806 0
	pushl	%ebp
LCFI44:
	movl	%esp, %ebp
LCFI45:
	subl	$220, %esp
LCFI46:
	.loc 1 811 0
LBB5:
	movb	$-32, %al
	andb	_dsd+20, %al
	testb	%al, %al
	je	L103
	cmpb	$0, _prtvolid
	je	L103
	.loc 1 814 0
	movl	_fileparm+40, %eax
	movl	%eax, _lblqab+12
	.loc 1 815 0
	pushl	$_lblqab
LCFI47:
	call	_svcIoQueue
	addl	$4, %esp
	testl	%eax, %eax
	js	L105
	cmpl	$0, _lblqab+4
	js	L105
	cmpl	$24, _lblqab+8
	jle	L105
	jmp	L103
L105:
	.loc 1 817 0
	movb	$0, _vollabel+24
L103:
	.loc 1 819 0
	leal	-216(%ebp), %eax
	movl	%eax, -4(%ebp)
	.loc 1 820 0
LCFI48:
	call	_pagecheck
	.loc 1 821 0
	pushl	$10
LCFI49:
	call	_putchar
	addl	$4, %esp
	.loc 1 822 0
	cmpb	$0, _prtvolid
	je	L106
	.loc 1 824 0
	pushl	$_dsd+84
	pushl	_dsd+68
	pushl	$_dsd+28
	pushl	$LC94
	pushl	-4(%ebp)
LCFI50:
	call	_sprintf
	addl	$20, %esp
	movl	%eax, %edx
	leal	-4(%ebp), %eax
	addl	%edx, (%eax)
	.loc 1 826 0
	cmpb	$0, _vollabel+24
	je	L107
	.loc 1 827 0
	pushl	$_vollabel+24
	pushl	$LC95
	pushl	-4(%ebp)
LCFI51:
	call	_sprintf
	addl	$12, %esp
	movl	%eax, %edx
	leal	-4(%ebp), %eax
	addl	%edx, (%eax)
L107:
	.loc 1 828 0
	cmpb	$0, _prtpath
	je	L108
	movl	$LC96, -220(%ebp)
	jmp	L109
L108:
	movl	$LC97, -220(%ebp)
L109:
	pushl	-220(%ebp)
	pushl	-4(%ebp)
LCFI52:
	call	_strmov
	addl	$8, %esp
	movl	%eax, -4(%ebp)
L106:
	.loc 1 830 0
	movl	-4(%ebp), %eax
	movl	%eax, -8(%ebp)
	.loc 1 831 0
	cmpb	$0, _prtpath
	je	L110
	.loc 1 832 0
	pushl	_dsd+120
	pushl	$LC98
	pushl	-4(%ebp)
LCFI53:
	call	_sprintf
	addl	$12, %esp
	movl	%eax, %edx
	leal	-4(%ebp), %eax
	addl	%edx, (%eax)
L110:
	.loc 1 833 0
	cmpb	$0, _prtvolid
	jne	L112
	cmpb	$0, _prtpath
	jne	L112
	jmp	L102
L112:
	.loc 1 835 0
LCFI54:
	call	_pagecheck
	.loc 1 836 0
	pushl	$LC97
	pushl	-4(%ebp)
LCFI55:
	call	_strmov
	addl	$8, %esp
	.loc 1 837 0
	cmpb	$0, _prtvolid
	je	L113
	cmpb	$0, _prtpath
	je	L113
	leal	-216(%ebp), %eax
	movl	-4(%ebp), %edx
	subl	%eax, %edx
	movl	%edx, %eax
	cmpl	_pib+16, %eax
	jle	L113
	.loc 1 840 0
	pushl	$LC97
	movl	-8(%ebp), %eax
	subl	$2, %eax
	pushl	%eax
	call	_strmov
	addl	$8, %esp
	.loc 1 841 0
	pushl	$__stdout
	leal	-216(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
	.loc 1 842 0
LCFI56:
	call	_pagecheck
	.loc 1 843 0
	pushl	$__stdout
	pushl	-8(%ebp)
LCFI57:
	call	_fputs
	addl	$8, %esp
	jmp	L102
L113:
	.loc 1 846 0
	pushl	$__stdout
	leal	-216(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
	.loc 1 848 0
L102:
	leave
	ret
LBE5:
LFE11:
LC99:
	.ascii "%Z %h:%m:%s %D-%3n-%l\0"
LC100:
	.ascii "%,14ld\0"
LC101:
	.ascii "\\\0"
LC102:
	.ascii " %s%s\0"
LC103:
	.ascii "File: %s%s\12\0"
LC104:
	.ascii "  File attributes: \0"
LC105:
	.ascii "Read only, \0"
LC106:
	.ascii "Read/write, \0"
LC107:
	.ascii "Hidden, \0"
LC108:
	.ascii "Visible, \0"
LC109:
	.ascii "System, \0"
LC110:
	.ascii "User, \0"
LC111:
	.ascii "Modified\12\0"
LC112:
	.ascii "Not modified\12\0"
	.p2align 5
LC113:
	.ascii "%Z  Created:  %h:%m:%s %D-%3n-%l\12\0"
	.p2align 5
LC114:
	.ascii "%Z  Modified: %h:%m:%s %D-%3n-%l\12\0"
	.p2align 5
LC115:
	.ascii "%Z  Accessed: %h:%m:%s %D-%3n-%l\12\0"
LC116:
	.ascii "  Written length:   %,ld\12\0"
LC117:
	.ascii "  Allocated length: %,ld\12\0"
	.p2align 5
LC118:
	.ascii "  Can not obtain file information: \0"
LC119:
	.ascii "[\0"
LC120:
	.ascii "%,d director%s, \0"
LC121:
	.ascii "%,d file%s\0"
	.p2align 5
LC122:
	.ascii ",%s %,lld bytes written, %,lld bytes allocated\0"
.globl _printfiles
_printfiles:
LFB13:
	.loc 1 864 0
	pushl	%ebp
LCFI58:
	movl	%esp, %ebp
LCFI59:
	subl	$336, %esp
LCFI60:
	.loc 1 876 0
LBB6:
	cmpl	$0, _shownames
	je	L116
	.loc 1 878 0
	cmpb	$0, _longfmt
	je	L117
	.loc 1 879 0
	movb	$1, _prtacross
L117:
	.loc 1 880 0
	cmpb	$0, _prtacross
	jne	L118
	.loc 1 881 0
	movb	$1, _prtexpand
L118:
	.loc 1 882 0
	cmpb	$0, _nosort
	jne	L119
	.loc 1 883 0
	pushl	$0
	pushl	$_comp
	pushl	_firstfile
LCFI61:
	call	_heapsort
	addl	$12, %esp
	movl	%eax, _firstfile
L119:
	.loc 1 886 0
	cmpb	$0, _longfmt
	jne	L120
	.loc 1 888 0
	cmpb	$0, _onecol
	jne	L122
	movl	_pib+16, %eax
	movl	%eax, -252(%ebp)
	cmpb	$0, _prtexpand
	je	L124
	movl	_needed, %eax
	incl	%eax
	movl	-252(%ebp), %edx
	movl	%eax, %ecx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	%ecx
	movl	%eax, -248(%ebp)
	jmp	L123
L124:
	movl	-252(%ebp), %eax
	movl	%eax, -256(%ebp)
	cmpl	$0, -256(%ebp)
	jns	L126
	addl	$15, -256(%ebp)
L126:
	movl	-256(%ebp), %edx
	sarl	$4, %edx
	movl	%edx, -248(%ebp)
	jmp	L123
L122:
	movl	$1, -248(%ebp)
L123:
	movl	-248(%ebp), %eax
	movl	%eax, _numcolumns
	cmpl	$5, %eax
	jle	L121
	.loc 1 890 0
	movl	$5, _numcolumns
L121:
	.loc 1 891 0
	movl	_pib+16, %edx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	_numcolumns
	movl	%eax, _colwidth
	.loc 1 892 0
	leal	-244(%ebp), %eax
	movl	%eax, -16(%ebp)
	.loc 1 893 0
	movl	_numcolumns, %eax
	movl	%eax, _left
	.loc 1 894 0
	cmpb	$0, _prtacross
	je	L133
L128:
	.loc 1 898 0
	pushl	-16(%ebp)
	leal	-244(%ebp), %eax
	pushl	%eax
	pushl	_firstfile
	call	_wideentry
	addl	$12, %esp
	movl	%eax, -16(%ebp)
	.loc 1 899 0
	movl	_firstfile, %eax
	movl	(%eax), %eax
	movl	%eax, -8(%ebp)
	.loc 1 900 0
	pushl	_firstfile
LCFI62:
	call	_free
	addl	$4, %esp
	.loc 1 901 0
	movl	-8(%ebp), %eax
	movl	%eax, _firstfile
	testl	%eax, %eax
	jne	L128
	.loc 1 902 0
LCFI63:
	call	_pagecheck
	.loc 1 903 0
	pushl	$LC97
	pushl	-16(%ebp)
LCFI64:
	call	_strmov
	addl	$8, %esp
	.loc 1 904 0
	pushl	$__stdout
	leal	-244(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
	jmp	L116
L133:
	.loc 1 909 0
	movl	_numcolumns, %eax
	addl	_numfiles, %eax
	leal	-1(%eax), %edx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	_numcolumns
	movl	%eax, _collength
	.loc 1 910 0
	cmpl	$0, _page
	je	L136
	.loc 1 912 0
	movl	_curline, %edx
	movl	_pib+20, %eax
	subl	%edx, %eax
	subl	$3, %eax
	movl	%eax, -32(%ebp)
	movl	-32(%ebp), %eax
	testl	%eax, %eax
	jg	L137
	.loc 1 913 0
	movl	_pib+20, %eax
	subl	$3, %eax
	movl	%eax, -32(%ebp)
L137:
	.loc 1 914 0
	movl	_collength, %eax
	cmpl	-32(%ebp), %eax
	jle	L136
	.loc 1 915 0
	movl	-32(%ebp), %eax
	movl	%eax, _collength
L136:
	.loc 1 917 0
	movl	_collength, %eax
	imull	_numcolumns, %eax
	subl	%eax, _numfiles
	.loc 1 918 0
	movl	_numcolumns, %eax
	movl	%eax, -24(%ebp)
	.loc 1 919 0
	movl	$_colarray, -12(%ebp)
L139:
	.loc 1 921 0
	movl	-12(%ebp), %eax
	movl	%eax, %edx
	movl	_firstfile, %eax
	movl	%eax, (%edx)
	leal	-12(%ebp), %eax
	addl	$4, (%eax)
	.loc 1 922 0
	movl	_collength, %eax
	movl	%eax, -32(%ebp)
	.loc 1 923 0
L142:
	cmpl	$0, _firstfile
	je	L143
	leal	-32(%ebp), %eax
	decl	(%eax)
	cmpl	$0, -32(%ebp)
	jg	L144
	jmp	L143
L144:
	.loc 1 924 0
	movl	_firstfile, %eax
	movl	(%eax), %eax
	movl	%eax, _firstfile
	jmp	L142
L143:
	.loc 1 925 0
	cmpl	$0, _firstfile
	je	L141
	.loc 1 927 0
	movl	_firstfile, %eax
	movl	(%eax), %eax
	movl	%eax, -8(%ebp)
	.loc 1 928 0
	movl	_firstfile, %eax
	movl	$0, (%eax)
	.loc 1 929 0
	movl	-8(%ebp), %eax
	movl	%eax, _firstfile
L141:
	.loc 1 931 0
	leal	-24(%ebp), %eax
	decl	(%eax)
	cmpl	$0, -24(%ebp)
	jg	L139
	nop
L148:
	.loc 1 933 0
	movl	_numcolumns, %eax
	movl	%eax, -24(%ebp)
	.loc 1 934 0
	movl	$_colarray, -12(%ebp)
	.loc 1 935 0
L151:
	movl	-12(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, -4(%ebp)
	testl	%eax, %eax
	je	L152
	leal	-24(%ebp), %eax
	decl	(%eax)
	cmpl	$0, -24(%ebp)
	jns	L153
	jmp	L152
L153:
	.loc 1 937 0
	pushl	-16(%ebp)
	leal	-244(%ebp), %eax
	pushl	%eax
	pushl	-4(%ebp)
LCFI65:
	call	_wideentry
	addl	$12, %esp
	movl	%eax, -16(%ebp)
	.loc 1 938 0
	movl	-12(%ebp), %edx
	movl	-4(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, (%edx)
	.loc 1 939 0
	pushl	-4(%ebp)
LCFI66:
	call	_free
	addl	$4, %esp
	.loc 1 940 0
	leal	-12(%ebp), %eax
	addl	$4, (%eax)
	jmp	L151
L152:
	.loc 1 942 0
LCFI67:
	call	_pagecheck
	.loc 1 943 0
	pushl	$LC97
	pushl	-16(%ebp)
LCFI68:
	call	_strmov
	addl	$8, %esp
	.loc 1 944 0
	pushl	$__stdout
	leal	-244(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
	.loc 1 945 0
	leal	-244(%ebp), %eax
	movl	%eax, -16(%ebp)
	.loc 1 946 0
	movl	_numcolumns, %eax
	movl	%eax, _left
	.loc 1 947 0
	cmpl	$0, _colarray
	jne	L148
	.loc 1 948 0
	cmpl	$0, _page
	je	L135
	cmpl	$0, _firstfile
	je	L135
	.loc 1 950 0
LCFI69:
	call	_pagecheck
	.loc 1 951 0
	pushl	$10
LCFI70:
	call	_putchar
	addl	$4, %esp
L135:
	.loc 1 953 0
	cmpl	$0, _firstfile
	jne	L133
	jmp	L116
L120:
	.loc 1 956 0
	cmpb	$0, _fullfmt
	jne	L195
L160:
	.loc 1 960 0
	leal	-244(%ebp), %eax
	movl	%eax, -16(%ebp)
	.loc 1 961 0
	cmpb	$0, _prtnamedos
	jne	L164
	cmpb	$0, _prtnamelong
	jne	L163
L164:
	.loc 1 963 0
	movl	$14, -24(%ebp)
	.loc 1 964 0
	cmpb	$0, _prtnamelong
	je	L165
	movl	_neednamedos, %eax
	movl	_firstfile, %ecx
	addl	%eax, %ecx
	movl	%ecx, -260(%ebp)
	addl	$16, -260(%ebp)
	jmp	L166
L165:
	movl	_needname, %eax
	movl	_firstfile, %edx
	addl	%eax, %edx
	movl	%edx, -260(%ebp)
	addl	$16, -260(%ebp)
L166:
	movl	-260(%ebp), %ecx
	movl	%ecx, -20(%ebp)
	.loc 1 966 0
L167:
	movl	-20(%ebp), %eax
	movb	(%eax), %dl
	movb	%dl, -33(%ebp)
	leal	-20(%ebp), %eax
	incl	(%eax)
	testb	%dl, %dl
	jne	L169
	jmp	L168
L169:
	.loc 1 968 0
	movl	-16(%ebp), %eax
	movl	%eax, %edx
	movb	-33(%ebp), %al
	movb	%al, (%edx)
	leal	-16(%ebp), %eax
	incl	(%eax)
	.loc 1 969 0
	leal	-24(%ebp), %eax
	decl	(%eax)
	jmp	L167
L168:
	.loc 1 971 0
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$16, %eax
	testl	%eax, %eax
	je	L170
	.loc 1 973 0
	movl	-16(%ebp), %eax
	movb	$92, (%eax)
	leal	-16(%ebp), %eax
	incl	(%eax)
	.loc 1 974 0
	leal	-24(%ebp), %eax
	decl	(%eax)
L170:
	nop
	.loc 1 976 0
L171:
	leal	-24(%ebp), %eax
	decl	(%eax)
	cmpl	$0, -24(%ebp)
	jg	L173
	jmp	L163
L173:
	.loc 1 977 0
	movl	-16(%ebp), %eax
	movb	$32, (%eax)
	leal	-16(%ebp), %eax
	incl	(%eax)
	jmp	L171
L163:
	.loc 1 979 0
	movl	_firstfile, %eax
	cmpl	$0, 12(%eax)
	js	L174
	.loc 1 981 0
	cmpb	$0, _showattr
	je	L175
	.loc 1 983 0
	movl	-16(%ebp), %eax
	movl	%eax, -264(%ebp)
	leal	-16(%ebp), %eax
	incl	(%eax)
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$1, %eax
	testl	%eax, %eax
	je	L176
	movb	$82, -265(%ebp)
	jmp	L177
L176:
	movb	$45, -265(%ebp)
L177:
	movb	-265(%ebp), %al
	movl	-264(%ebp), %edx
	movb	%al, (%edx)
	.loc 1 985 0
	movl	-16(%ebp), %eax
	movl	%eax, -272(%ebp)
	leal	-16(%ebp), %eax
	incl	(%eax)
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$2, %eax
	testl	%eax, %eax
	je	L178
	movb	$72, -273(%ebp)
	jmp	L179
L178:
	movb	$45, -273(%ebp)
L179:
	movb	-273(%ebp), %cl
	movl	-272(%ebp), %eax
	movb	%cl, (%eax)
	.loc 1 987 0
	movl	-16(%ebp), %eax
	movl	%eax, -280(%ebp)
	leal	-16(%ebp), %eax
	incl	(%eax)
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$4, %eax
	testl	%eax, %eax
	je	L180
	movb	$83, -281(%ebp)
	jmp	L181
L180:
	movb	$45, -281(%ebp)
L181:
	movb	-281(%ebp), %dl
	movl	-280(%ebp), %ecx
	movb	%dl, (%ecx)
	.loc 1 989 0
	movl	-16(%ebp), %eax
	movl	%eax, -288(%ebp)
	leal	-16(%ebp), %eax
	incl	(%eax)
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$32, %eax
	testl	%eax, %eax
	je	L182
	movb	$77, -289(%ebp)
	jmp	L183
L182:
	movb	$45, -289(%ebp)
L183:
	movb	-289(%ebp), %al
	movl	-288(%ebp), %edx
	movb	%al, (%edx)
L175:
	.loc 1 992 0
	movl	_needcdate, %eax
	addl	_firstfile, %eax
	addl	$16, %eax
	pushl	%eax
	pushl	$LC99
	pushl	-16(%ebp)
LCFI71:
	call	_sdt2str
	addl	$12, %esp
	movl	%eax, %edx
	leal	-16(%ebp), %eax
	addl	%edx, (%eax)
	.loc 1 994 0
	movl	_needlength, %eax
	addl	_firstfile, %eax
	addl	$16, %eax
	pushl	(%eax)
	pushl	$LC100
	pushl	-16(%ebp)
	call	_sprintf
	addl	$12, %esp
	movl	%eax, %edx
	leal	-16(%ebp), %eax
	addl	%edx, (%eax)
	jmp	L184
L174:
	.loc 1 999 0
	pushl	-16(%ebp)
	pushl	$3
	movl	_firstfile, %eax
	pushl	12(%eax)
	call	_svcSysErrMsg
	addl	$12, %esp
	movl	%eax, %edx
	leal	-16(%ebp), %eax
	addl	%edx, (%eax)
	.loc 1 1000 0
	leal	-244(%ebp), %edx
	movl	-16(%ebp), %eax
	subl	%edx, %eax
	movl	%eax, -296(%ebp)
	cmpb	$0, _showattr
	je	L185
	movl	$38, %eax
	subl	-296(%ebp), %eax
	movl	%eax, -300(%ebp)
	jmp	L186
L185:
	movl	$33, %eax
	subl	-296(%ebp), %eax
	movl	%eax, -300(%ebp)
L186:
	movl	-300(%ebp), %ecx
	movl	%ecx, -28(%ebp)
	.loc 1 1001 0
L187:
	leal	-28(%ebp), %eax
	decl	(%eax)
	cmpl	$0, -28(%ebp)
	jg	L189
	jmp	L184
L189:
	.loc 1 1002 0
	movl	-16(%ebp), %eax
	movb	$32, (%eax)
	leal	-16(%ebp), %eax
	incl	(%eax)
	jmp	L187
L184:
	.loc 1 1004 0
	cmpb	$0, _prtnamelong
	je	L190
	.loc 1 1005 0
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$16, %eax
	testl	%eax, %eax
	je	L191
	movl	$LC101, -304(%ebp)
	jmp	L192
L191:
	movl	$LC86, -304(%ebp)
L192:
	pushl	-304(%ebp)
	movl	_needname, %eax
	addl	_firstfile, %eax
	addl	$16, %eax
	pushl	%eax
	pushl	$LC102
	pushl	-16(%ebp)
LCFI72:
	call	_sprintf
	addl	$16, %esp
	movl	%eax, %edx
	leal	-16(%ebp), %eax
	addl	%edx, (%eax)
L190:
	.loc 1 1007 0
LCFI73:
	call	_pagecheck
	.loc 1 1008 0
	pushl	$LC97
	pushl	-16(%ebp)
LCFI74:
	call	_strmov
	addl	$8, %esp
	.loc 1 1009 0
	pushl	$__stdout
	leal	-244(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
	.loc 1 1010 0
	movl	_firstfile, %eax
	movl	(%eax), %eax
	movl	%eax, -8(%ebp)
	.loc 1 1011 0
	pushl	_firstfile
LCFI75:
	call	_free
	addl	$4, %esp
	.loc 1 1012 0
	movl	-8(%ebp), %eax
	movl	%eax, _firstfile
	testl	%eax, %eax
	jne	L160
	jmp	L116
L195:
	.loc 1 1018 0
LCFI76:
	call	_pagecheck
	.loc 1 1019 0
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$16, %eax
	testl	%eax, %eax
	je	L198
	movl	$LC101, -308(%ebp)
	jmp	L199
L198:
	movl	$LC86, -308(%ebp)
L199:
	pushl	-308(%ebp)
	movl	_needname, %eax
	addl	_firstfile, %eax
	addl	$16, %eax
	pushl	%eax
	pushl	$LC103
LCFI77:
	call	_printf
	addl	$12, %esp
	.loc 1 1021 0
LCFI78:
	call	_pagecheck
	.loc 1 1022 0
	movl	_firstfile, %eax
	cmpl	$0, 12(%eax)
	js	L200
	.loc 1 1024 0
	pushl	$LC104
	leal	-244(%ebp), %eax
	pushl	%eax
LCFI79:
	call	_strmov
	addl	$8, %esp
	movl	%eax, -16(%ebp)
	.loc 1 1025 0
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$1, %eax
	testl	%eax, %eax
	je	L201
	movl	$LC105, -312(%ebp)
	jmp	L202
L201:
	movl	$LC106, -312(%ebp)
L202:
	pushl	-312(%ebp)
	pushl	-16(%ebp)
	call	_strmov
	addl	$8, %esp
	movl	%eax, -16(%ebp)
	.loc 1 1027 0
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$2, %eax
	testl	%eax, %eax
	je	L203
	movl	$LC107, -316(%ebp)
	jmp	L204
L203:
	movl	$LC108, -316(%ebp)
L204:
	pushl	-316(%ebp)
	pushl	-16(%ebp)
	call	_strmov
	addl	$8, %esp
	movl	%eax, -16(%ebp)
	.loc 1 1029 0
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$4, %eax
	testl	%eax, %eax
	je	L205
	movl	$LC109, -320(%ebp)
	jmp	L206
L205:
	movl	$LC110, -320(%ebp)
L206:
	pushl	-320(%ebp)
	pushl	-16(%ebp)
	call	_strmov
	addl	$8, %esp
	movl	%eax, -16(%ebp)
	.loc 1 1031 0
	movl	_firstfile, %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$32, %eax
	testl	%eax, %eax
	je	L207
	movl	$LC111, -324(%ebp)
	jmp	L208
L207:
	movl	$LC112, -324(%ebp)
L208:
	pushl	-324(%ebp)
	pushl	-16(%ebp)
	call	_strmov
	addl	$8, %esp
	movl	%eax, -16(%ebp)
	.loc 1 1033 0
	pushl	$__stdout
	leal	-244(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
	.loc 1 1034 0
	movl	_needcdate, %eax
	addl	_firstfile, %eax
	addl	$16, %eax
	pushl	%eax
	pushl	$LC113
	leal	-244(%ebp), %eax
	pushl	%eax
LCFI80:
	call	_sdt2str
	addl	$12, %esp
	.loc 1 1036 0
LCFI81:
	call	_pagecheck
	.loc 1 1037 0
	pushl	$__stdout
	leal	-244(%ebp), %eax
	pushl	%eax
LCFI82:
	call	_fputs
	addl	$8, %esp
	.loc 1 1038 0
	movl	_needmdate, %eax
	addl	_firstfile, %eax
	addl	$16, %eax
	pushl	%eax
	pushl	$LC114
	leal	-244(%ebp), %eax
	pushl	%eax
LCFI83:
	call	_sdt2str
	addl	$12, %esp
	.loc 1 1040 0
LCFI84:
	call	_pagecheck
	.loc 1 1041 0
	pushl	$__stdout
	leal	-244(%ebp), %eax
	pushl	%eax
LCFI85:
	call	_fputs
	addl	$8, %esp
	.loc 1 1042 0
	movl	_needadate, %eax
	addl	_firstfile, %eax
	addl	$16, %eax
	pushl	%eax
	pushl	$LC115
	leal	-244(%ebp), %eax
	pushl	%eax
LCFI86:
	call	_sdt2str
	addl	$12, %esp
	.loc 1 1044 0
LCFI87:
	call	_pagecheck
	.loc 1 1045 0
	pushl	$__stdout
	leal	-244(%ebp), %eax
	pushl	%eax
LCFI88:
	call	_fputs
	addl	$8, %esp
	.loc 1 1046 0
LCFI89:
	call	_pagecheck
	.loc 1 1047 0
	movl	_needlength, %eax
	addl	_firstfile, %eax
	addl	$16, %eax
	pushl	(%eax)
	pushl	$LC116
LCFI90:
	call	_printf
	addl	$8, %esp
	.loc 1 1048 0
LCFI91:
	call	_pagecheck
	.loc 1 1049 0
	movl	_needalloc, %eax
	addl	_firstfile, %eax
	addl	$16, %eax
	pushl	(%eax)
	pushl	$LC117
LCFI92:
	call	_printf
	addl	$8, %esp
	jmp	L209
L200:
	.loc 1 1053 0
	pushl	$LC118
	leal	-244(%ebp), %eax
	pushl	%eax
	call	_strmov
	addl	$8, %esp
	movl	%eax, -16(%ebp)
	.loc 1 1055 0
	pushl	-16(%ebp)
	pushl	$3
	movl	_firstfile, %eax
	pushl	12(%eax)
LCFI93:
	call	_svcSysErrMsg
	addl	$12, %esp
	.loc 1 1056 0
	pushl	$LC97
	pushl	-16(%ebp)
LCFI94:
	call	_strmov
	addl	$8, %esp
	.loc 1 1057 0
	pushl	$__stdout
	leal	-244(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
L209:
	.loc 1 1059 0
	movl	_firstfile, %eax
	movl	(%eax), %eax
	movl	%eax, -8(%ebp)
	.loc 1 1060 0
	pushl	_firstfile
LCFI95:
	call	_free
	addl	$4, %esp
	.loc 1 1061 0
	movl	-8(%ebp), %eax
	movl	%eax, _firstfile
	testl	%eax, %eax
	jne	L195
L116:
	.loc 1 1064 0
	cmpl	$0, _lsum
	je	L211
	.loc 1 1066 0
LCFI96:
	call	_pagecheck
	.loc 1 1067 0
	pushl	$LC119
	leal	-244(%ebp), %eax
	pushl	%eax
LCFI97:
	call	_strmov
	addl	$8, %esp
	movl	%eax, -16(%ebp)
	.loc 1 1068 0
	cmpl	$0, _dircnt
	je	L212
	.loc 1 1069 0
	cmpl	$1, _dircnt
	jne	L213
	movl	$LC82, -328(%ebp)
	jmp	L214
L213:
	movl	$LC83, -328(%ebp)
L214:
	pushl	-328(%ebp)
	pushl	_dircnt
	pushl	$LC120
	pushl	-16(%ebp)
LCFI98:
	call	_sprintf
	addl	$16, %esp
	movl	%eax, %edx
	leal	-16(%ebp), %eax
	addl	%edx, (%eax)
L212:
	.loc 1 1071 0
	cmpl	$1, _filecnt
	jne	L215
	movl	$LC86, -332(%ebp)
	jmp	L216
L215:
	movl	$LC87, -332(%ebp)
L216:
	pushl	-332(%ebp)
	pushl	_filecnt
	pushl	$LC121
	pushl	-16(%ebp)
	call	_sprintf
	addl	$16, %esp
	movl	%eax, %edx
	leal	-16(%ebp), %eax
	addl	%edx, (%eax)
	.loc 1 1073 0
	cmpb	$0, _longfmt
	je	L217
	.loc 1 1074 0
	pushl	_totalalloc+4
	pushl	_totalalloc
	pushl	_totallen+4
	pushl	_totallen
	cmpb	$0, _longfmt
	je	L218
	cmpl	$0, _dircnt
	je	L218
	movl	$LC97, -336(%ebp)
	jmp	L219
L218:
	movl	$LC86, -336(%ebp)
L219:
	pushl	-336(%ebp)
	pushl	$LC122
	pushl	-16(%ebp)
LCFI99:
	call	_sprintf
	addl	$28, %esp
	movl	%eax, %edx
	leal	-16(%ebp), %eax
	addl	%edx, (%eax)
L217:
	.loc 1 1077 0
	pushl	$LC90
	pushl	-16(%ebp)
LCFI100:
	call	_strmov
	addl	$8, %esp
	.loc 1 1078 0
	pushl	$__stdout
	leal	-244(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
L211:
	.loc 1 1080 0
	movl	$0, _firstfile
	.loc 1 1081 0
	movl	_filecnt, %eax
	addl	%eax, _gfilecnt
	.loc 1 1082 0
	movl	$0, _filecnt
	.loc 1 1083 0
	movl	_dircnt, %eax
	addl	%eax, _gdircnt
	.loc 1 1084 0
	movl	$0, _dircnt
	.loc 1 1085 0
	pushl	$_totallen
	pushl	$_gtotallen
	call	_longlongadd
	addl	$8, %esp
	.loc 1 1086 0
	movl	$0, _totallen+4
	movl	$0, _totallen
	.loc 1 1087 0
	pushl	$_totalalloc
	pushl	$_gtotalalloc
	call	_longlongadd
	addl	$8, %esp
	.loc 1 1088 0
	movl	$0, _totalalloc+4
	movl	$0, _totalalloc
	.loc 1 1089 0
	movl	$0, _numfiles
	.loc 1 1090 0
	incl	_numlisted
	.loc 1 1092 0
	leave
	ret
LBE6:
LFE13:
.globl _wideentry
_wideentry:
LFB15:
	.loc 1 1105 0
	pushl	%ebp
LCFI101:
	movl	%esp, %ebp
LCFI102:
	subl	$4, %esp
LCFI103:
	.loc 1 1108 0
LBB7:
	movl	_needname, %eax
	addl	8(%ebp), %eax
	addl	$16, %eax
	pushl	%eax
LCFI104:
	call	_strlen
	addl	$4, %esp
	movl	%eax, -4(%ebp)
	.loc 1 1109 0
	movl	8(%ebp), %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$16, %eax
	testl	%eax, %eax
	je	L221
	.loc 1 1110 0
	leal	-4(%ebp), %eax
	incl	(%eax)
L221:
	.loc 1 1112 0
	movl	_colwidth, %eax
	movl	-4(%ebp), %edx
	addl	%eax, %edx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	_colwidth
	movl	%eax, _needed
	.loc 1 1113 0
	movl	_needed, %eax
	cmpl	_left, %eax
	jle	L222
	.loc 1 1115 0
LCFI105:
	call	_pagecheck
	.loc 1 1116 0
	pushl	$LC97
	pushl	16(%ebp)
LCFI106:
	call	_strmov
	addl	$8, %esp
	.loc 1 1117 0
	pushl	$__stdout
	pushl	12(%ebp)
	call	_fputs
	addl	$8, %esp
	.loc 1 1118 0
	movl	12(%ebp), %eax
	movl	%eax, 16(%ebp)
	.loc 1 1119 0
	movl	_numcolumns, %eax
	movl	%eax, _left
	jmp	L223
L222:
	.loc 1 1121 0
	movl	_left, %eax
	cmpl	_numcolumns, %eax
	je	L223
	.loc 1 1123 0
	movl	12(%ebp), %edx
	movl	16(%ebp), %eax
	subl	%edx, %eax
	movl	%eax, %edx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	_colwidth
	movl	_colwidth, %eax
	subl	%edx, %eax
	movl	%eax, _fill
L225:
	.loc 1 1125 0
	movl	16(%ebp), %eax
	movb	$32, (%eax)
	leal	16(%ebp), %eax
	incl	(%eax)
	.loc 1 1126 0
	decl	_fill
	cmpl	$0, _fill
	jg	L225
L223:
	.loc 1 1128 0
	movl	_needname, %eax
	addl	8(%ebp), %eax
	addl	$16, %eax
	pushl	%eax
	pushl	16(%ebp)
	call	_strmov
	addl	$8, %esp
	movl	%eax, 16(%ebp)
	.loc 1 1129 0
	movl	_needed, %eax
	subl	%eax, _left
	.loc 1 1130 0
	movl	8(%ebp), %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$16, %eax
	testl	%eax, %eax
	je	L229
	.loc 1 1131 0
	movl	16(%ebp), %eax
	movb	$92, (%eax)
	leal	16(%ebp), %eax
	incl	(%eax)
L229:
	.loc 1 1132 0
	movl	16(%ebp), %eax
	.loc 1 1133 0
	leave
	ret
LBE7:
LFE15:
.globl _comp
_comp:
LFB17:
	.loc 1 1146 0
	pushl	%ebp
LCFI107:
	movl	%esp, %ebp
LCFI108:
	subl	$44, %esp
LCFI109:
	.loc 1 1152 0
LBB8:
	cmpb	$0, _dirsort
	je	L231
	.loc 1 1154 0
	movl	8(%ebp), %eax
	movb	8(%eax), %al
	andl	$16, %eax
	movb	%al, -13(%ebp)
	.loc 1 1155 0
	movsbl	-13(%ebp),%edx
	movl	12(%ebp), %eax
	movb	8(%eax), %al
	andl	$255, %eax
	andl	$16, %eax
	cmpl	%eax, %edx
	je	L231
	.loc 1 1156 0
	cmpb	$0, -13(%ebp)
	je	L233
	movl	$-1, -28(%ebp)
	jmp	L234
L233:
	movl	$1, -28(%ebp)
L234:
	movl	-28(%ebp), %eax
	movl	%eax, -24(%ebp)
	jmp	L230
L231:
	.loc 1 1158 0
	cmpb	$0, _revsort
	je	L235
	.loc 1 1162 0
LBB9:
	movl	8(%ebp), %eax
	movl	%eax, -20(%ebp)
	.loc 1 1163 0
	movl	12(%ebp), %eax
	movl	%eax, 8(%ebp)
	.loc 1 1164 0
	movl	-20(%ebp), %eax
	movl	%eax, 12(%ebp)
L235:
	.loc 1 1166 0
LBE9:
	cmpb	$0, _datesort
	je	L236
	.loc 1 1168 0
	movl	_needcdate, %eax
	addl	8(%ebp), %eax
	leal	16(%eax), %ecx
	movl	_needcdate, %eax
	addl	12(%ebp), %eax
	leal	16(%eax), %edx
	movl	4(%ecx), %eax
	cmpl	4(%edx), %eax
	je	L237
	.loc 1 1169 0
	movl	_needcdate, %eax
	addl	8(%ebp), %eax
	leal	16(%eax), %ecx
	movl	_needcdate, %eax
	addl	12(%ebp), %eax
	leal	16(%eax), %edx
	movl	4(%ecx), %eax
	cmpl	4(%edx), %eax
	jge	L238
	movl	$-1, -32(%ebp)
	jmp	L239
L238:
	movl	$1, -32(%ebp)
L239:
	movl	-32(%ebp), %eax
	movl	%eax, -24(%ebp)
	jmp	L230
L237:
	.loc 1 1171 0
	movl	_needcdate, %eax
	addl	8(%ebp), %eax
	leal	16(%eax), %ecx
	movl	_needcdate, %eax
	addl	12(%ebp), %eax
	leal	16(%eax), %edx
	movl	(%ecx), %eax
	cmpl	(%edx), %eax
	je	L243
	.loc 1 1172 0
	movl	_needcdate, %eax
	addl	8(%ebp), %eax
	leal	16(%eax), %ecx
	movl	_needcdate, %eax
	addl	12(%ebp), %eax
	leal	16(%eax), %edx
	movl	(%ecx), %eax
	cmpl	(%edx), %eax
	jae	L241
	movl	$-1, -36(%ebp)
	jmp	L242
L241:
	movl	$1, -36(%ebp)
L242:
	movl	-36(%ebp), %eax
	movl	%eax, -24(%ebp)
	jmp	L230
L236:
	.loc 1 1175 0
	cmpb	$0, _extsort
	je	L244
	.loc 1 1177 0
	pushl	$46
	movl	_needname, %eax
	addl	8(%ebp), %eax
	addl	$16, %eax
	pushl	%eax
LCFI110:
	call	_strrchr
	addl	$8, %esp
	movl	%eax, -4(%ebp)
	.loc 1 1178 0
	pushl	$46
	movl	_needname, %eax
	addl	12(%ebp), %eax
	addl	$16, %eax
	pushl	%eax
	call	_strrchr
	addl	$8, %esp
	movl	%eax, -8(%ebp)
	.loc 1 1179 0
	cmpl	$0, -4(%ebp)
	je	L245
	cmpl	$0, -8(%ebp)
	je	L245
	.loc 1 1181 0
	pushl	-8(%ebp)
	pushl	-4(%ebp)
	call	_strcmp
	addl	$8, %esp
	movl	%eax, -12(%ebp)
	cmpl	$0, -12(%ebp)
	je	L243
	.loc 1 1182 0
	movl	-12(%ebp), %eax
	movl	%eax, -24(%ebp)
	jmp	L230
L245:
	.loc 1 1184 0
	movl	-4(%ebp), %eax
	cmpl	-8(%ebp), %eax
	je	L243
	.loc 1 1185 0
	cmpl	$0, -4(%ebp)
	jne	L249
	movl	$-1, -40(%ebp)
	jmp	L250
L249:
	movl	$1, -40(%ebp)
L250:
	movl	-40(%ebp), %eax
	movl	%eax, -24(%ebp)
	jmp	L230
L244:
	.loc 1 1187 0
	cmpb	$0, _sizesort
	je	L243
	.loc 1 1189 0
	movl	_needlength, %eax
	addl	8(%ebp), %eax
	leal	16(%eax), %ecx
	movl	_needlength, %eax
	addl	12(%ebp), %eax
	leal	16(%eax), %edx
	movl	(%ecx), %eax
	cmpl	(%edx), %eax
	je	L243
	.loc 1 1190 0
	movl	_needlength, %eax
	addl	8(%ebp), %eax
	leal	16(%eax), %ecx
	movl	_needlength, %eax
	addl	12(%ebp), %eax
	leal	16(%eax), %edx
	movl	(%ecx), %eax
	cmpl	(%edx), %eax
	jge	L254
	movl	$-1, -44(%ebp)
	jmp	L255
L254:
	movl	$1, -44(%ebp)
L255:
	movl	-44(%ebp), %eax
	movl	%eax, -24(%ebp)
	jmp	L230
L243:
	.loc 1 1192 0
	movl	_needname, %eax
	addl	12(%ebp), %eax
	addl	$16, %eax
	pushl	%eax
	movl	_needname, %eax
	addl	8(%ebp), %eax
	addl	$16, %eax
	pushl	%eax
	call	_stricmp
	addl	$8, %esp
	movl	%eax, -24(%ebp)
	.loc 1 1193 0
L230:
LBE8:
	movl	-24(%ebp), %eax
	leave
	ret
LFE17:
	.p2align 5
LC123:
	.ascii "\33[7m-MORE- ^C, G, H, Q, <Enter> or <Space>\33[0m\0"
LC124:
	.ascii "\15\33[K\0"
	.p2align 5
LC125:
	.ascii "\15\33[K\33[7m ^C      - Exit                    \12  G      - Go, don't ask for -MORE-\12  H or ? - Help, this message      \12  Q      - Quit program            \12 <Enter> - Next screen             \12 <Space> - Next screen             \12-MORE- ^C, G, H, Q, <Enter> or <Space>\33[0m\0"
.globl _pagecheck
_pagecheck:
LFB19:
	.loc 1 1202 0
	pushl	%ebp
LCFI111:
	movl	%esp, %ebp
LCFI112:
	subl	$12, %esp
LCFI113:
	.loc 1 1206 0
LBB10:
	cmpl	$0, _page
	je	L256
	incl	_curline
	movl	_pib+20, %eax
	subl	$2, %eax
	cmpl	%eax, _curline
	jl	L256
	.loc 1 1208 0
	pushl	$__stdout
	pushl	$LC123
LCFI114:
	call	_fputs
	addl	$8, %esp
	.loc 1 1209 0
	movl	$1, -8(%ebp)
	.loc 1 1210 0
L258:
	cmpl	$1, -8(%ebp)
	je	L260
	jmp	L259
L260:
	.loc 1 1212 0
	pushl	$_snglparm
	pushl	$5
	call	_svcIoInSingleP
	addl	$8, %esp
	movl	%eax, -4(%ebp)
	.loc 1 1214 0
	movl	_snglparm, %eax
	movl	_snglparm+4, %edx
	movl	%eax, _normparm+8
	movl	%edx, _normparm+12
	.loc 1 1215 0
	pushl	$_normparm
	pushl	$0
	pushl	$0
	pushl	$5
LCFI115:
	call	_svcIoInBlockP
	addl	$16, %esp
	.loc 1 1216 0
	movl	$0, -8(%ebp)
	.loc 1 1218 0
	movl	-4(%ebp), %eax
	andl	$127, %eax
	pushl	%eax
LCFI116:
	call	_toupper
	addl	$4, %esp
	movl	%eax, -12(%ebp)
	cmpl	$32, -12(%ebp)
	je	L266
	cmpl	$32, -12(%ebp)
	jg	L272
	cmpl	$3, -12(%ebp)
	je	L263
	cmpl	$3, -12(%ebp)
	jg	L273
	cmpl	$0, -12(%ebp)
	je	L270
	jmp	L269
L273:
	cmpl	$13, -12(%ebp)
	je	L266
	jmp	L269
L272:
	cmpl	$71, -12(%ebp)
	je	L264
	cmpl	$71, -12(%ebp)
	jg	L274
	jmp	L269
L274:
	cmpl	$72, -12(%ebp)
	je	L269
	cmpl	$81, -12(%ebp)
	je	L263
	jmp	L269
L263:
	.loc 1 1222 0
	pushl	$__stdout
	pushl	$LC124
LCFI117:
	call	_fputs
	addl	$8, %esp
	.loc 1 1223 0
	pushl	$0
LCFI118:
	call	_exit
L264:
	.loc 1 1226 0
	movl	$0, _page
	.loc 1 1227 0
	jmp	L258
L266:
	.loc 1 1231 0
	movl	$0, _curline
	.loc 1 1232 0
	jmp	L258
L269:
	.loc 1 1237 0
	pushl	$__stdout
	pushl	$LC125
LCFI119:
	call	_fputs
	addl	$8, %esp
L270:
	.loc 1 1248 0
	movl	$1, -8(%ebp)
	.loc 1 1249 0
	jmp	L258
L259:
	.loc 1 1252 0
	pushl	$__stdout
	pushl	$LC124
	call	_fputs
	addl	$8, %esp
	.loc 1 1254 0
L256:
	leave
	ret
LBE10:
LFE19:
	.p2align 5
LC126:
	.ascii "? %s: Invalid DISPLAY option value, %d\12\0"
.globl _optdisplay
_optdisplay:
LFB21:
	.loc 1 1265 0
	pushl	%ebp
LCFI120:
	movl	%esp, %ebp
LCFI121:
	subl	$8, %esp
LCFI122:
	.loc 1 1266 0
	movl	8(%ebp), %eax
	movl	12(%eax), %eax
	andl	$2048, %eax
	testl	%eax, %eax
	jne	L276
	.loc 1 1267 0
	movl	$1, -4(%ebp)
	jmp	L275
L276:
	.loc 1 1268 0
	movl	8(%ebp), %eax
	movl	%eax, -8(%ebp)
	movl	-8(%ebp), %edx
	cmpl	$15, 4(%edx)
	ja	L294
	movl	-8(%ebp), %edx
	movl	4(%edx), %eax
	sall	$2, %eax
	movl	L295(%eax), %eax
	jmp	*%eax
	.p2align 2
	.p2align 2
L295:
	.long	L278
	.long	L279
	.long	L280
	.long	L281
	.long	L282
	.long	L283
	.long	L284
	.long	L285
	.long	L286
	.long	L287
	.long	L288
	.long	L289
	.long	L290
	.long	L291
	.long	L292
	.long	L293
L278:
	.loc 1 1271 0
	movl	$1, _lsum
	.loc 1 1272 0
	jmp	L277
L279:
	.loc 1 1275 0
	movl	$0, _lsum
	.loc 1 1276 0
	jmp	L277
L280:
	.loc 1 1279 0
	movb	$1, _prtpath
	.loc 1 1280 0
	jmp	L277
L281:
	.loc 1 1283 0
	movb	$0, _prtpath
	.loc 1 1284 0
	jmp	L277
L282:
	.loc 1 1287 0
	movb	$1, _prtvolid
	.loc 1 1288 0
	jmp	L277
L283:
	.loc 1 1291 0
	movb	$0, _prtvolid
	.loc 1 1292 0
	jmp	L277
L284:
	.loc 1 1295 0
	movb	$1, _showattr
	.loc 1 1296 0
	jmp	L277
L285:
	.loc 1 1299 0
	movb	$0, _showattr
	.loc 1 1300 0
	jmp	L277
L286:
	.loc 1 1303 0
	movb	$1, _prtnamedos
	.loc 1 1304 0
	jmp	L277
L287:
	.loc 1 1307 0
	movb	$0, _prtnamedos
	.loc 1 1308 0
	movb	$1, _prtnamelong
	.loc 1 1309 0
	jmp	L277
L288:
	.loc 1 1312 0
	movb	$1, _prtnamelong
	.loc 1 1313 0
	jmp	L277
L289:
	.loc 1 1316 0
	movb	$0, _prtnamelong
	.loc 1 1317 0
	movb	$1, _prtnamedos
	.loc 1 1318 0
	jmp	L277
L290:
	.loc 1 1321 0
	movb	$0, _prtacross
	.loc 1 1322 0
	jmp	L277
L291:
	.loc 1 1325 0
	movb	$1, _prtacross
	.loc 1 1326 0
	jmp	L277
L292:
	.loc 1 1329 0
	movb	$1, _prtexpand
	.loc 1 1330 0
	jmp	L277
L293:
	.loc 1 1333 0
	movb	$0, _prtexpand
	.loc 1 1334 0
	jmp	L277
L294:
	.loc 1 1337 0
	movl	8(%ebp), %eax
	pushl	4(%eax)
	pushl	$_prgname
	pushl	$LC126
	pushl	$__stderr
LCFI123:
	call	_fprintf
	addl	$16, %esp
	.loc 1 1339 0
	pushl	$3
LCFI124:
	call	_exit
L277:
	.loc 1 1341 0
	movl	$1, -4(%ebp)
	.loc 1 1342 0
L275:
	movl	-4(%ebp), %eax
	leave
	ret
LFE21:
LC127:
	.ascii "after time = %s\12\0"
.globl _optafter
_optafter:
LFB23:
	.loc 1 1352 0
	pushl	%ebp
LCFI125:
	movl	%esp, %ebp
LCFI126:
	subl	$4, %esp
LCFI127:
	.loc 1 1355 0
LBB11:
	movl	8(%ebp), %eax
	movl	4(%eax), %eax
	movl	%eax, -4(%ebp)
	.loc 1 1356 0
	pushl	-4(%ebp)
	pushl	$LC127
LCFI128:
	call	_printf
	addl	$8, %esp
	.loc 1 1357 0
	movl	$1, %eax
	.loc 1 1358 0
	leave
	ret
LBE11:
LFE23:
.globl _optall
_optall:
LFB25:
	.loc 1 1367 0
	pushl	%ebp
LCFI129:
	movl	%esp, %ebp
LCFI130:
	.loc 1 1368 0
	movw	$151, _fileparm+28
	.loc 1 1369 0
	movl	$1, %eax
	.loc 1 1370 0
	popl	%ebp
	ret
LFE25:
LC128:
	.ascii "before time = %s\12\0"
.globl _optbefore
_optbefore:
LFB27:
	.loc 1 1380 0
	pushl	%ebp
LCFI131:
	movl	%esp, %ebp
LCFI132:
	subl	$4, %esp
LCFI133:
	.loc 1 1383 0
LBB12:
	movl	8(%ebp), %eax
	movl	4(%eax), %eax
	movl	%eax, -4(%ebp)
	.loc 1 1384 0
	pushl	-4(%ebp)
	pushl	$LC128
LCFI134:
	call	_printf
	addl	$8, %esp
	.loc 1 1385 0
	movl	$1, %eax
	.loc 1 1386 0
	leave
	ret
LBE12:
LFE27:
.globl _optdir
_optdir:
LFB29:
	.loc 1 1395 0
	pushl	%ebp
LCFI135:
	movl	%esp, %ebp
LCFI136:
	.loc 1 1396 0
	movw	$16, _fileparm+28
	.loc 1 1397 0
	movl	$1, %eax
	.loc 1 1398 0
	popl	%ebp
	ret
LFE29:
.globl _optnodir
_optnodir:
LFB31:
	.loc 1 1407 0
	pushl	%ebp
LCFI137:
	movl	%esp, %ebp
LCFI138:
	.loc 1 1408 0
	andw	$-17, _fileparm+28
	.loc 1 1409 0
	movl	$1, %eax
	.loc 1 1410 0
	popl	%ebp
	ret
LFE31:
.globl _optone
_optone:
LFB33:
	.loc 1 1419 0
	pushl	%ebp
LCFI139:
	movl	%esp, %ebp
LCFI140:
	.loc 1 1420 0
	movb	$1, _onecol
	.loc 1 1421 0
	call	_optshort
	.loc 1 1422 0
	movl	$1, %eax
	.loc 1 1423 0
	popl	%ebp
	ret
LFE33:
.globl _optfull
_optfull:
LFB35:
	.loc 1 1432 0
	pushl	%ebp
LCFI141:
	movl	%esp, %ebp
LCFI142:
	.loc 1 1433 0
	movb	$1, _fullfmt
	.loc 1 1434 0
	movb	$1, _longfmt
	.loc 1 1435 0
	movl	$0, _needalloc
	.loc 1 1436 0
	movl	$4, _needowner
	.loc 1 1437 0
	movl	$40, _needprot
	.loc 1 1438 0
	movl	$44, _needadate
	.loc 1 1439 0
	movl	$52, _needmdate
	.loc 1 1440 0
	movl	$60, _needcdate
	.loc 1 1441 0
	movl	$68, _needlength
	.loc 1 1442 0
	movl	$72, _needname
	.loc 1 1443 0
	movl	$1, %eax
	.loc 1 1444 0
	popl	%ebp
	ret
LFE35:
.globl _optshort
_optshort:
LFB37:
	.loc 1 1453 0
	pushl	%ebp
LCFI143:
	movl	%esp, %ebp
LCFI144:
	.loc 1 1454 0
	movb	$0, _longfmt
	.loc 1 1455 0
	movb	$0, _fullfmt
	.loc 1 1456 0
	movl	$-1, _needalloc
	.loc 1 1457 0
	movl	$-1, _needowner
	.loc 1 1458 0
	movl	$-1, _needprot
	.loc 1 1459 0
	movl	$-1, _needadate
	.loc 1 1460 0
	movl	$-1, _needmdate
	.loc 1 1461 0
	movl	$-1, _needcdate
	.loc 1 1462 0
	movl	$-1, _needlength
	.loc 1 1463 0
	movl	$0, _needname
	.loc 1 1464 0
	movl	$1, %eax
	.loc 1 1465 0
	popl	%ebp
	ret
LFE37:
.globl _optlong
_optlong:
LFB39:
	.loc 1 1474 0
	pushl	%ebp
LCFI145:
	movl	%esp, %ebp
LCFI146:
	.loc 1 1475 0
	movb	$1, _longfmt
	.loc 1 1476 0
	movb	$0, _fullfmt
	.loc 1 1477 0
	movl	$-1, _needalloc
	.loc 1 1478 0
	movl	$0, _needowner
	.loc 1 1479 0
	movl	$36, _needprot
	.loc 1 1480 0
	movl	$-1, _needadate
	.loc 1 1481 0
	movl	$-1, _needmdate
	.loc 1 1482 0
	movl	$40, _needcdate
	.loc 1 1483 0
	movl	$48, _needlength
	.loc 1 1484 0
	movl	$52, _needname
	.loc 1 1485 0
	movl	$1, %eax
	.loc 1 1486 0
	popl	%ebp
	ret
LFE39:
	.p2align 5
LC129:
	.ascii "? %s: Invalid SORT option value, %d\12\0"
.globl _optsort
_optsort:
LFB41:
	.loc 1 1496 0
	pushl	%ebp
LCFI147:
	movl	%esp, %ebp
LCFI148:
	subl	$8, %esp
LCFI149:
	.loc 1 1497 0
	movl	8(%ebp), %eax
	movl	12(%eax), %eax
	andl	$2048, %eax
	testl	%eax, %eax
	jne	L306
	.loc 1 1499 0
	movb	$1, _datesort
	.loc 1 1500 0
	movb	$1, _revsort
	.loc 1 1501 0
	movb	$0, _extsort
	.loc 1 1502 0
	movb	$0, _nosort
	.loc 1 1503 0
	movl	$1, -4(%ebp)
	jmp	L305
L306:
	.loc 1 1505 0
	movl	8(%ebp), %eax
	movl	%eax, -8(%ebp)
	movl	-8(%ebp), %edx
	cmpl	$8, 4(%edx)
	ja	L317
	movl	-8(%ebp), %edx
	movl	4(%edx), %eax
	sall	$2, %eax
	movl	L318(%eax), %eax
	jmp	*%eax
	.p2align 2
	.p2align 2
L318:
	.long	L308
	.long	L309
	.long	L310
	.long	L311
	.long	L312
	.long	L313
	.long	L314
	.long	L315
	.long	L316
L308:
	.loc 1 1508 0
	movb	$1, _nosort
	.loc 1 1509 0
	movb	$0, _dirsort
	.loc 1 1510 0
	movb	$0, _revsort
	.loc 1 1511 0
	movb	$0, _extsort
	.loc 1 1512 0
	movb	$0, _datesort
	.loc 1 1513 0
	jmp	L307
L309:
	.loc 1 1516 0
	movb	$0, _revsort
	.loc 1 1517 0
	movb	$0, _nosort
	.loc 1 1518 0
	jmp	L307
L310:
	.loc 1 1521 0
	movb	$1, _revsort
	.loc 1 1522 0
	movb	$0, _nosort
	.loc 1 1523 0
	jmp	L307
L311:
	.loc 1 1526 0
	movb	$0, _datesort
	.loc 1 1527 0
	movb	$0, _extsort
	.loc 1 1528 0
	movb	$0, _sizesort
	.loc 1 1529 0
	movb	$0, _nosort
	.loc 1 1530 0
	jmp	L307
L312:
	.loc 1 1533 0
	movb	$1, _extsort
	.loc 1 1534 0
	movb	$0, _datesort
	.loc 1 1535 0
	movb	$0, _sizesort
	.loc 1 1536 0
	movb	$0, _nosort
	.loc 1 1537 0
	jmp	L307
L313:
	.loc 1 1540 0
	movb	$1, _datesort
	.loc 1 1541 0
	movb	$0, _extsort
	.loc 1 1542 0
	movb	$0, _sizesort
	.loc 1 1543 0
	movb	$0, _nosort
	.loc 1 1544 0
	jmp	L307
L314:
	.loc 1 1547 0
	movb	$1, _sizesort
	.loc 1 1548 0
	movb	$0, _extsort
	.loc 1 1549 0
	movb	$0, _datesort
	.loc 1 1550 0
	movb	$0, _nosort
	.loc 1 1551 0
	jmp	L307
L315:
	.loc 1 1554 0
	movb	$1, _dirsort
	.loc 1 1555 0
	movb	$0, _nosort
	.loc 1 1556 0
	jmp	L307
L316:
	.loc 1 1559 0
	movb	$0, _dirsort
	.loc 1 1560 0
	movb	$0, _nosort
	.loc 1 1561 0
	jmp	L307
L317:
	.loc 1 1564 0
	movl	8(%ebp), %eax
	pushl	4(%eax)
	pushl	$_prgname
	pushl	$LC129
	pushl	$__stderr
LCFI150:
	call	_fprintf
	addl	$16, %esp
	.loc 1 1566 0
	pushl	$3
LCFI151:
	call	_exit
L307:
	.loc 1 1568 0
	movl	$1, -4(%ebp)
	.loc 1 1569 0
L305:
	movl	-4(%ebp), %eax
	leave
	ret
LFE41:
.globl _optx
_optx:
LFB43:
	.loc 1 1579 0
	pushl	%ebp
LCFI152:
	movl	%esp, %ebp
LCFI153:
	subl	$4, %esp
LCFI154:
	.loc 1 1582 0
LBB13:
	movl	8(%ebp), %eax
	movl	16(%eax), %eax
	addl	$10, %eax
	pushl	%eax
LCFI155:
	call	_getmem
	addl	$4, %esp
	movl	%eax, -4(%ebp)
	.loc 1 1583 0
	movl	8(%ebp), %eax
	pushl	4(%eax)
	movl	-4(%ebp), %eax
	addl	$4, %eax
	pushl	%eax
LCFI156:
	call	_strmov
	addl	$8, %esp
	.loc 1 1584 0
	movl	-4(%ebp), %edx
	movl	_firstname, %eax
	movl	%eax, (%edx)
	.loc 1 1585 0
	movl	-4(%ebp), %eax
	movl	%eax, _firstname
	.loc 1 1586 0
	movl	$1, %eax
	.loc 1 1587 0
	leave
	ret
LBE13:
LFE43:
	.section .data
	.p2align 5
_glbidparm.0:
	.byte	66
	.byte	16
	.word	5
	.byte	0
	.space 15
	.byte	66
	.byte	4
	.word	3
	.long	0
	.space 4
	.section .text
LC130:
	.ascii "...\0"
.globl _nonopt
_nonopt:
LFB45:
	.loc 1 1597 0
	pushl	%ebp
LCFI157:
	movl	%esp, %ebp
LCFI158:
	pushl	%ebx
LCFI159:
	subl	$28, %esp
LCFI160:
	.loc 1 1614 0
LBB14:
	movb	$0, _hvdefault
	.loc 1 1615 0
	pushl	8(%ebp)
LCFI161:
	call	_strlen
	addl	$4, %esp
	movl	%eax, -24(%ebp)
	.loc 1 1616 0
	movl	-24(%ebp), %eax
	addl	8(%ebp), %eax
	decl	%eax
	movb	(%eax), %al
	movb	%al, -25(%ebp)
	cmpb	$58, %al
	je	L322
	cmpb	$92, -25(%ebp)
	je	L322
	jmp	L321
L322:
	.loc 1 1618 0
	movb	$1, _hvdefault
	.loc 1 1619 0
	leal	-24(%ebp), %eax
	addl	$2, (%eax)
L321:
	.loc 1 1621 0
	movl	-24(%ebp), %eax
	addl	$42, %eax
	pushl	%eax
	call	_getmem
	addl	$4, %esp
	movl	%eax, -8(%ebp)
	.loc 1 1622 0
	movl	-8(%ebp), %eax
	movl	$0, 4(%eax)
	.loc 1 1623 0
	pushl	8(%ebp)
	movl	-8(%ebp), %eax
	addl	$36, %eax
	pushl	%eax
LCFI162:
	call	_strmov
	addl	$8, %esp
	movl	%eax, -16(%ebp)
	.loc 1 1624 0
	cmpb	$0, _hvdefault
	je	L323
	.loc 1 1625 0
	pushl	$LC80
	pushl	-16(%ebp)
	call	_strmov
	addl	$8, %esp
L323:
	.loc 1 1632 0
	movl	-8(%ebp), %ebx
	pushl	$LC130
	movl	-8(%ebp), %eax
	addl	$36, %eax
	pushl	%eax
	call	_strstr
	addl	$8, %esp
	movl	%eax, 28(%ebx)
	movl	28(%ebx), %eax
	testl	%eax, %eax
	je	L324
	.loc 1 1634 0
	movl	-8(%ebp), %eax
	movl	28(%eax), %eax
	movl	%eax, -32(%ebp)
	movl	-8(%ebp), %edx
	movl	-8(%ebp), %eax
	addl	$36, %eax
	cmpl	%eax, 28(%edx)
	jne	L325
	incl	-32(%ebp)
L325:
	movl	-32(%ebp), %eax
	movb	$0, (%eax)
	.loc 1 1635 0
	pushl	$_glbidparm.0
	movl	-8(%ebp), %eax
	addl	$36, %eax
	pushl	%eax
	pushl	$268435456
LCFI163:
	call	_svcIoDevParm
	addl	$12, %esp
	movl	%eax, -20(%ebp)
	.loc 1 1636 0
	movl	-8(%ebp), %eax
	movl	28(%eax), %edx
	movl	-8(%ebp), %eax
	movl	28(%eax), %eax
	incl	%eax
	movb	$46, (%eax)
	movb	$46, (%edx)
	jmp	L326
L324:
	.loc 1 1639 0
	pushl	$_glbidparm.0
	movl	-8(%ebp), %eax
	addl	$36, %eax
	pushl	%eax
	pushl	$268435456
	call	_svcIoDevParm
	addl	$12, %esp
	movl	%eax, -20(%ebp)
L326:
	.loc 1 1640 0
	cmpl	$0, -20(%ebp)
	jns	L327
	.loc 1 1642 0
	movl	-8(%ebp), %eax
	movl	$-1, 8(%eax)
	.loc 1 1643 0
	movl	-8(%ebp), %edx
	movl	-8(%ebp), %eax
	movl	%eax, 12(%edx)
	.loc 1 1644 0
	movl	-8(%ebp), %edx
	movl	-8(%ebp), %eax
	movl	$0, 20(%eax)
	movl	$0, 16(%edx)
	jmp	L328
L327:
	.loc 1 1647 0
	pushl	$16
	pushl	$_glbidparm.0+4
	movl	-8(%ebp), %eax
	addl	$8, %eax
	pushl	%eax
	call	_memcpy
	addl	$12, %esp
L328:
	.loc 1 1648 0
	movl	-8(%ebp), %edx
	movl	_glbidparm.0+24, %eax
	movl	%eax, 24(%edx)
	.loc 1 1649 0
	movl	-8(%ebp), %edx
	movl	-8(%ebp), %eax
	addl	$36, %eax
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %eax
	movl	%eax, 32(%edx)
	.loc 1 1650 0
L329:
	movl	-16(%ebp), %eax
	movb	(%eax), %dl
	movb	%dl, -25(%ebp)
	leal	-16(%ebp), %eax
	incl	(%eax)
	testb	%dl, %dl
	jne	L331
	jmp	L330
L331:
	.loc 1 1652 0
	cmpb	$58, -25(%ebp)
	je	L333
	cmpb	$92, -25(%ebp)
	je	L333
	cmpb	$47, -25(%ebp)
	je	L333
	jmp	L329
L333:
	.loc 1 1653 0
	movl	-8(%ebp), %edx
	movl	-16(%ebp), %eax
	movl	%eax, 32(%edx)
	jmp	L329
L330:
	.loc 1 1655 0
	cmpl	$0, _firstspec
	jne	L334
	.loc 1 1656 0
	movl	-8(%ebp), %eax
	movl	%eax, _firstspec
	jmp	L335
L334:
	.loc 1 1659 0
	movl	_firstspec, %eax
	movl	%eax, -12(%ebp)
	.loc 1 1660 0
	movl	-8(%ebp), %ecx
	movl	-8(%ebp), %eax
	movl	28(%eax), %edx
	movl	32(%ecx), %eax
	subl	%edx, %eax
	movl	%eax, -20(%ebp)
L336:
	.loc 1 1663 0
	pushl	$16
	movl	-12(%ebp), %eax
	addl	$8, %eax
	pushl	%eax
	movl	-8(%ebp), %eax
	addl	$8, %eax
	pushl	%eax
	call	_memcmp
	addl	$12, %esp
	testl	%eax, %eax
	jne	L338
	.loc 1 1665 0
	movl	-8(%ebp), %eax
	cmpl	$0, 28(%eax)
	je	L337
	.loc 1 1667 0
	movl	-12(%ebp), %ecx
	movl	-12(%ebp), %eax
	movl	28(%eax), %edx
	movl	32(%ecx), %eax
	subl	%edx, %eax
	cmpl	%eax, -20(%ebp)
	jne	L338
	pushl	-20(%ebp)
	movl	-12(%ebp), %eax
	pushl	28(%eax)
	movl	-8(%ebp), %eax
	pushl	28(%eax)
	call	_strnicmp
	addl	$12, %esp
	testl	%eax, %eax
	jne	L338
	jmp	L337
L338:
	.loc 1 1673 0
	movl	-12(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, -12(%ebp)
	testl	%eax, %eax
	jne	L336
L337:
	.loc 1 1674 0
	cmpl	$0, -12(%ebp)
	je	L344
	.loc 1 1675 0
	movl	-12(%ebp), %edx
	movl	-8(%ebp), %eax
	movl	%eax, 4(%edx)
	jmp	L335
L344:
	.loc 1 1677 0
	movl	_thisspec, %edx
	movl	-8(%ebp), %eax
	movl	%eax, (%edx)
L335:
	.loc 1 1679 0
	movl	-8(%ebp), %edx
	movl	-8(%ebp), %eax
	movl	$0, 4(%eax)
	movl	$0, (%edx)
	.loc 1 1680 0
	movl	-8(%ebp), %eax
	movl	%eax, _thisspec
	.loc 1 1681 0
	movl	$1, %eax
	.loc 1 1682 0
	movl	-4(%ebp), %ebx
	leave
	ret
LBE14:
LFE45:
	.p2align 5
LC131:
	.ascii "? DIR: Not enough memory available\12\0"
.globl _getmem
_getmem:
LFB47:
	.loc 1 1693 0
	pushl	%ebp
LCFI164:
	movl	%esp, %ebp
LCFI165:
	subl	$4, %esp
LCFI166:
	.loc 1 1696 0
LBB15:
	pushl	8(%ebp)
LCFI167:
	call	_malloc
	addl	$4, %esp
	movl	%eax, -4(%ebp)
	.loc 1 1697 0
	cmpl	$0, -4(%ebp)
	jne	L347
	.loc 1 1699 0
	pushl	$__stderr
	pushl	$LC131
LCFI168:
	call	_fputs
	addl	$8, %esp
	.loc 1 1700 0
	pushl	$2
LCFI169:
	call	_exit
L347:
	.loc 1 1702 0
	movl	_maxmem, %eax
	cmpl	__malloc_amount, %eax
	jae	L348
	.loc 1 1703 0
	movl	__malloc_amount, %eax
	movl	%eax, _maxmem
L348:
	.loc 1 1704 0
	movl	-4(%ebp), %eax
	.loc 1 1705 0
	leave
	ret
LBE15:
LFE47:
LC132:
	.ascii "; \0"
LC133:
	.ascii "\12? DIR: %s%s%s\12\0"
LC134:
	.ascii "\12? DIR: %s\12\0"
.globl _errormsg
_errormsg:
LFB49:
	.loc 1 1716 0
	pushl	%ebp
LCFI170:
	movl	%esp, %ebp
LCFI171:
	subl	$84, %esp
LCFI172:
	.loc 1 1719 0
LBB16:
	cmpl	$0, _numfiles
	je	L350
	.loc 1 1720 0
	call	_printfiles
L350:
	.loc 1 1721 0
	call	_pagecheck
	.loc 1 1722 0
	cmpl	$0, 12(%ebp)
	je	L351
	.loc 1 1724 0
	leal	-80(%ebp), %eax
	pushl	%eax
	pushl	$3
	pushl	12(%ebp)
LCFI173:
	call	_svcSysErrMsg
	addl	$12, %esp
	.loc 1 1725 0
	pushl	8(%ebp)
	movl	8(%ebp), %eax
	cmpb	$0, (%eax)
	je	L352
	movl	$LC132, -84(%ebp)
	jmp	L353
L352:
	movl	$LC86, -84(%ebp)
L353:
	pushl	-84(%ebp)
	leal	-80(%ebp), %eax
	pushl	%eax
	pushl	$LC133
	pushl	$__stderr
LCFI174:
	call	_fprintf
	addl	$20, %esp
	jmp	L354
L351:
	.loc 1 1729 0
	pushl	8(%ebp)
	pushl	$LC134
	pushl	$__stderr
LCFI175:
	call	_fprintf
	addl	$12, %esp
L354:
	.loc 1 1730 0
	movb	$1, _errdone
	.loc 1 1731 0
	leave
	ret
LBE16:
LFE49:
.globl _firstspec
	.section	.bss
	.p2align 2
_firstspec:
	.space 4
.globl _thisspec
	.p2align 2
_thisspec:
	.space 4
.globl _firstname
	.p2align 2
_firstname:
	.space 4
.globl _firstfile
	.p2align 2
_firstfile:
	.space 4
.globl _lastfile
	.p2align 2
_lastfile:
	.space 4
.globl _thisfile
	.p2align 2
_thisfile:
	.space 4
.globl _owner
	.p2align 5
_owner:
	.space 36
.globl _changed
_changed:
	.space 1
.globl _needed
	.p2align 2
_needed:
	.space 4
.globl _namesize
	.p2align 2
_namesize:
	.space 4
.globl _after_time
	.p2align 2
_after_time:
	.space 8
.globl _before_time
	.p2align 2
_before_time:
	.space 8
.globl _errdone
_errdone:
	.space 1
.globl _hvdefault
_hvdefault:
	.space 1
.globl _numfiles
	.p2align 2
_numfiles:
	.space 4
.globl _filecnt
	.p2align 2
_filecnt:
	.space 4
.globl _dircnt
	.p2align 2
_dircnt:
	.space 4
.globl _totallen
	.p2align 2
_totallen:
	.space 8
.globl _totalalloc
	.p2align 2
_totalalloc:
	.space 8
.globl _gfilecnt
	.p2align 2
_gfilecnt:
	.space 4
.globl _gdircnt
	.p2align 2
_gdircnt:
	.space 4
.globl _gtotallen
	.p2align 2
_gtotallen:
	.space 8
.globl _gtotalalloc
	.p2align 2
_gtotalalloc:
	.space 8
.globl _fspnt
	.p2align 2
_fspnt:
	.space 4
.globl _pib
	.p2align 5
_pib:
	.space 152
.globl _vollabel
	.p2align 5
_vollabel:
	.space 60
.globl _maxmem
	.p2align 2
_maxmem:
	.space 4
.globl _colwidth
	.p2align 2
_colwidth:
	.space 4
.globl _collength
	.p2align 2
_collength:
	.space 4
.globl _fill
	.p2align 2
_fill:
	.space 4
.globl _left
	.p2align 2
_left:
	.space 4
.globl _numcolumns
	.p2align 2
_numcolumns:
	.space 4
.globl _colarray
	.p2align 2
_colarray:
	.space 20
	.file 12 "inc/GLOBAL.H"
	.section	.debug_frame,""
Lframe0:
	.long	LECIE0-LSCIE0
LSCIE0:
	.long	0xffffffff
	.byte	0x1
	.ascii "\0"
	.uleb128 0x1
	.sleb128 -4
	.byte	0x8
	.byte	0xc
	.uleb128 0x5
	.uleb128 0x4
	.byte	0x88
	.uleb128 0x1
	.p2align 2
LECIE0:
LSFDE0:
	.long	LEFDE0-LASFDE0
LASFDE0:
	.long	Lframe0
	.long	LFB3
	.long	LFE3-LFB3
	.byte	0x4
	.long	LCFI0-LFB3
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI1-LCFI0
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI3-LCFI1
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI4-LCFI3
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI5-LCFI4
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI6-LCFI5
	.byte	0x2e
	.uleb128 0x18
	.byte	0x4
	.long	LCFI7-LCFI6
	.byte	0x2e
	.uleb128 0x20
	.byte	0x4
	.long	LCFI8-LCFI7
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI9-LCFI8
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI10-LCFI9
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI11-LCFI10
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI12-LCFI11
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI13-LCFI12
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI14-LCFI13
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI15-LCFI14
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI16-LCFI15
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI17-LCFI16
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI18-LCFI17
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI19-LCFI18
	.byte	0x2e
	.uleb128 0x10
	.byte	0x4
	.long	LCFI20-LCFI19
	.byte	0x2e
	.uleb128 0x20
	.byte	0x4
	.long	LCFI21-LCFI20
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI22-LCFI21
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI23-LCFI22
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI24-LCFI23
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI25-LCFI24
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI26-LCFI25
	.byte	0x2e
	.uleb128 0x4
	.p2align 2
LEFDE0:
LSFDE2:
	.long	LEFDE2-LASFDE2
LASFDE2:
	.long	Lframe0
	.long	LFB5
	.long	LFE5-LFB5
	.byte	0x4
	.long	LCFI27-LFB5
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI28-LCFI27
	.byte	0xd
	.uleb128 0x4
	.p2align 2
LEFDE2:
LSFDE4:
	.long	LEFDE4-LASFDE4
LASFDE4:
	.long	Lframe0
	.long	LFB7
	.long	LFE7-LFB7
	.byte	0x4
	.long	LCFI29-LFB7
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI30-LCFI29
	.byte	0xd
	.uleb128 0x4
	.p2align 2
LEFDE4:
LSFDE6:
	.long	LEFDE6-LASFDE6
LASFDE6:
	.long	Lframe0
	.long	LFB9
	.long	LFE9-LFB9
	.byte	0x4
	.long	LCFI31-LFB9
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI32-LCFI31
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI34-LCFI32
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI35-LCFI34
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI36-LCFI35
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI37-LCFI36
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI38-LCFI37
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI39-LCFI38
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI40-LCFI39
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI41-LCFI40
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI42-LCFI41
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI43-LCFI42
	.byte	0x2e
	.uleb128 0x8
	.p2align 2
LEFDE6:
LSFDE8:
	.long	LEFDE8-LASFDE8
LASFDE8:
	.long	Lframe0
	.long	LFB11
	.long	LFE11-LFB11
	.byte	0x4
	.long	LCFI44-LFB11
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI45-LCFI44
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI47-LCFI45
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI48-LCFI47
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI49-LCFI48
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI50-LCFI49
	.byte	0x2e
	.uleb128 0x14
	.byte	0x4
	.long	LCFI51-LCFI50
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI52-LCFI51
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI53-LCFI52
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI54-LCFI53
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI55-LCFI54
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI56-LCFI55
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI57-LCFI56
	.byte	0x2e
	.uleb128 0x8
	.p2align 2
LEFDE8:
LSFDE10:
	.long	LEFDE10-LASFDE10
LASFDE10:
	.long	Lframe0
	.long	LFB13
	.long	LFE13-LFB13
	.byte	0x4
	.long	LCFI58-LFB13
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI59-LCFI58
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI61-LCFI59
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI62-LCFI61
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI63-LCFI62
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI64-LCFI63
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI65-LCFI64
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI66-LCFI65
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI67-LCFI66
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI68-LCFI67
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI69-LCFI68
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI70-LCFI69
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI71-LCFI70
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI72-LCFI71
	.byte	0x2e
	.uleb128 0x10
	.byte	0x4
	.long	LCFI73-LCFI72
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI74-LCFI73
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI75-LCFI74
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI76-LCFI75
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI77-LCFI76
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI78-LCFI77
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI79-LCFI78
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI80-LCFI79
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI81-LCFI80
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI82-LCFI81
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI83-LCFI82
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI84-LCFI83
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI85-LCFI84
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI86-LCFI85
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI87-LCFI86
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI88-LCFI87
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI89-LCFI88
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI90-LCFI89
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI91-LCFI90
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI92-LCFI91
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI93-LCFI92
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI94-LCFI93
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI95-LCFI94
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI96-LCFI95
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI97-LCFI96
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI98-LCFI97
	.byte	0x2e
	.uleb128 0x10
	.byte	0x4
	.long	LCFI99-LCFI98
	.byte	0x2e
	.uleb128 0x1c
	.byte	0x4
	.long	LCFI100-LCFI99
	.byte	0x2e
	.uleb128 0x8
	.p2align 2
LEFDE10:
LSFDE12:
	.long	LEFDE12-LASFDE12
LASFDE12:
	.long	Lframe0
	.long	LFB15
	.long	LFE15-LFB15
	.byte	0x4
	.long	LCFI101-LFB15
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI102-LCFI101
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI104-LCFI102
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI105-LCFI104
	.byte	0x2e
	.uleb128 0x0
	.byte	0x4
	.long	LCFI106-LCFI105
	.byte	0x2e
	.uleb128 0x8
	.p2align 2
LEFDE12:
LSFDE14:
	.long	LEFDE14-LASFDE14
LASFDE14:
	.long	Lframe0
	.long	LFB17
	.long	LFE17-LFB17
	.byte	0x4
	.long	LCFI107-LFB17
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI108-LCFI107
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI110-LCFI108
	.byte	0x2e
	.uleb128 0x8
	.p2align 2
LEFDE14:
LSFDE16:
	.long	LEFDE16-LASFDE16
LASFDE16:
	.long	Lframe0
	.long	LFB19
	.long	LFE19-LFB19
	.byte	0x4
	.long	LCFI111-LFB19
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI112-LCFI111
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI114-LCFI112
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI115-LCFI114
	.byte	0x2e
	.uleb128 0x10
	.byte	0x4
	.long	LCFI116-LCFI115
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI117-LCFI116
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI118-LCFI117
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI119-LCFI118
	.byte	0x2e
	.uleb128 0x8
	.p2align 2
LEFDE16:
LSFDE18:
	.long	LEFDE18-LASFDE18
LASFDE18:
	.long	Lframe0
	.long	LFB21
	.long	LFE21-LFB21
	.byte	0x4
	.long	LCFI120-LFB21
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI121-LCFI120
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI123-LCFI121
	.byte	0x2e
	.uleb128 0x10
	.byte	0x4
	.long	LCFI124-LCFI123
	.byte	0x2e
	.uleb128 0x4
	.p2align 2
LEFDE18:
LSFDE20:
	.long	LEFDE20-LASFDE20
LASFDE20:
	.long	Lframe0
	.long	LFB23
	.long	LFE23-LFB23
	.byte	0x4
	.long	LCFI125-LFB23
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI126-LCFI125
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI128-LCFI126
	.byte	0x2e
	.uleb128 0x8
	.p2align 2
LEFDE20:
LSFDE22:
	.long	LEFDE22-LASFDE22
LASFDE22:
	.long	Lframe0
	.long	LFB25
	.long	LFE25-LFB25
	.byte	0x4
	.long	LCFI129-LFB25
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI130-LCFI129
	.byte	0xd
	.uleb128 0x4
	.p2align 2
LEFDE22:
LSFDE24:
	.long	LEFDE24-LASFDE24
LASFDE24:
	.long	Lframe0
	.long	LFB27
	.long	LFE27-LFB27
	.byte	0x4
	.long	LCFI131-LFB27
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI132-LCFI131
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI134-LCFI132
	.byte	0x2e
	.uleb128 0x8
	.p2align 2
LEFDE24:
LSFDE26:
	.long	LEFDE26-LASFDE26
LASFDE26:
	.long	Lframe0
	.long	LFB29
	.long	LFE29-LFB29
	.byte	0x4
	.long	LCFI135-LFB29
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI136-LCFI135
	.byte	0xd
	.uleb128 0x4
	.p2align 2
LEFDE26:
LSFDE28:
	.long	LEFDE28-LASFDE28
LASFDE28:
	.long	Lframe0
	.long	LFB31
	.long	LFE31-LFB31
	.byte	0x4
	.long	LCFI137-LFB31
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI138-LCFI137
	.byte	0xd
	.uleb128 0x4
	.p2align 2
LEFDE28:
LSFDE30:
	.long	LEFDE30-LASFDE30
LASFDE30:
	.long	Lframe0
	.long	LFB33
	.long	LFE33-LFB33
	.byte	0x4
	.long	LCFI139-LFB33
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI140-LCFI139
	.byte	0xd
	.uleb128 0x4
	.p2align 2
LEFDE30:
LSFDE32:
	.long	LEFDE32-LASFDE32
LASFDE32:
	.long	Lframe0
	.long	LFB35
	.long	LFE35-LFB35
	.byte	0x4
	.long	LCFI141-LFB35
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI142-LCFI141
	.byte	0xd
	.uleb128 0x4
	.p2align 2
LEFDE32:
LSFDE34:
	.long	LEFDE34-LASFDE34
LASFDE34:
	.long	Lframe0
	.long	LFB37
	.long	LFE37-LFB37
	.byte	0x4
	.long	LCFI143-LFB37
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI144-LCFI143
	.byte	0xd
	.uleb128 0x4
	.p2align 2
LEFDE34:
LSFDE36:
	.long	LEFDE36-LASFDE36
LASFDE36:
	.long	Lframe0
	.long	LFB39
	.long	LFE39-LFB39
	.byte	0x4
	.long	LCFI145-LFB39
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI146-LCFI145
	.byte	0xd
	.uleb128 0x4
	.p2align 2
LEFDE36:
LSFDE38:
	.long	LEFDE38-LASFDE38
LASFDE38:
	.long	Lframe0
	.long	LFB41
	.long	LFE41-LFB41
	.byte	0x4
	.long	LCFI147-LFB41
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI148-LCFI147
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI150-LCFI148
	.byte	0x2e
	.uleb128 0x10
	.byte	0x4
	.long	LCFI151-LCFI150
	.byte	0x2e
	.uleb128 0x4
	.p2align 2
LEFDE38:
LSFDE40:
	.long	LEFDE40-LASFDE40
LASFDE40:
	.long	Lframe0
	.long	LFB43
	.long	LFE43-LFB43
	.byte	0x4
	.long	LCFI152-LFB43
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI153-LCFI152
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI155-LCFI153
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI156-LCFI155
	.byte	0x2e
	.uleb128 0x8
	.p2align 2
LEFDE40:
LSFDE42:
	.long	LEFDE42-LASFDE42
LASFDE42:
	.long	Lframe0
	.long	LFB45
	.long	LFE45-LFB45
	.byte	0x4
	.long	LCFI157-LFB45
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI158-LCFI157
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI160-LCFI158
	.byte	0x83
	.uleb128 0x3
	.byte	0x4
	.long	LCFI161-LCFI160
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI162-LCFI161
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI163-LCFI162
	.byte	0x2e
	.uleb128 0xc
	.p2align 2
LEFDE42:
LSFDE44:
	.long	LEFDE44-LASFDE44
LASFDE44:
	.long	Lframe0
	.long	LFB47
	.long	LFE47-LFB47
	.byte	0x4
	.long	LCFI164-LFB47
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI165-LCFI164
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI167-LCFI165
	.byte	0x2e
	.uleb128 0x4
	.byte	0x4
	.long	LCFI168-LCFI167
	.byte	0x2e
	.uleb128 0x8
	.byte	0x4
	.long	LCFI169-LCFI168
	.byte	0x2e
	.uleb128 0x4
	.p2align 2
LEFDE44:
LSFDE46:
	.long	LEFDE46-LASFDE46
LASFDE46:
	.long	Lframe0
	.long	LFB49
	.long	LFE49-LFB49
	.byte	0x4
	.long	LCFI170-LFB49
	.byte	0xe
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x2
	.byte	0x4
	.long	LCFI171-LCFI170
	.byte	0xd
	.uleb128 0x4
	.byte	0x4
	.long	LCFI173-LCFI171
	.byte	0x2e
	.uleb128 0xc
	.byte	0x4
	.long	LCFI174-LCFI173
	.byte	0x2e
	.uleb128 0x14
	.byte	0x4
	.long	LCFI175-LCFI174
	.byte	0x2e
	.uleb128 0xc
	.p2align 2
LEFDE46:
	.section .text
Letext0:
	.section	.debug_info,""
	.long	0x35a0
	.word	0x2
	.long	Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	Ldebug_line0
	.long	Letext0
	.long	Ltext0
	.ascii "dir.c\0"
	.ascii "e:/xcc\0"
	.ascii "GNU C 3.3.2\0"
	.byte	0x1
	.uleb128 0x2
	.ascii "_size_t\0"
	.byte	0x2
	.byte	0x2f
	.long	0x41
	.uleb128 0x3
	.long	LC135
	.byte	0x4
	.byte	0x7
	.uleb128 0x2
	.ascii "_time_t\0"
	.byte	0x2
	.byte	0x31
	.long	0x57
	.uleb128 0x4
	.ascii "long int\0"
	.byte	0x4
	.byte	0x5
	.uleb128 0x2
	.ascii "_clock_t\0"
	.byte	0x2
	.byte	0x33
	.long	0x57
	.uleb128 0x2
	.ascii "_ptrdiff_t\0"
	.byte	0x2
	.byte	0x35
	.long	0x85
	.uleb128 0x4
	.ascii "unsigned int\0"
	.byte	0x4
	.byte	0x7
	.uleb128 0x2
	.ascii "_wchar_t\0"
	.byte	0x2
	.byte	0x37
	.long	0xa5
	.uleb128 0x4
	.ascii "short unsigned int\0"
	.byte	0x2
	.byte	0x7
	.uleb128 0x2
	.ascii "uchar\0"
	.byte	0x2
	.byte	0x3c
	.long	0xc8
	.uleb128 0x4
	.ascii "unsigned char\0"
	.byte	0x1
	.byte	0x8
	.uleb128 0x2
	.ascii "schar\0"
	.byte	0x2
	.byte	0x3d
	.long	0xe6
	.uleb128 0x4
	.ascii "signed char\0"
	.byte	0x1
	.byte	0x6
	.uleb128 0x2
	.ascii "u8\0"
	.byte	0x2
	.byte	0x3f
	.long	0xc8
	.uleb128 0x2
	.ascii "s8\0"
	.byte	0x2
	.byte	0x40
	.long	0xe6
	.uleb128 0x2
	.ascii "ushort\0"
	.byte	0x2
	.byte	0x42
	.long	0xa5
	.uleb128 0x2
	.ascii "sshort\0"
	.byte	0x2
	.byte	0x43
	.long	0x125
	.uleb128 0x4
	.ascii "short int\0"
	.byte	0x2
	.byte	0x5
	.uleb128 0x2
	.ascii "u16\0"
	.byte	0x2
	.byte	0x45
	.long	0xa5
	.uleb128 0x2
	.ascii "s16\0"
	.byte	0x2
	.byte	0x46
	.long	0x125
	.uleb128 0x2
	.ascii "uint\0"
	.byte	0x2
	.byte	0x48
	.long	0x85
	.uleb128 0x2
	.ascii "sint\0"
	.byte	0x2
	.byte	0x49
	.long	0x160
	.uleb128 0x4
	.ascii "int\0"
	.byte	0x4
	.byte	0x5
	.uleb128 0x2
	.ascii "u32\0"
	.byte	0x2
	.byte	0x4b
	.long	0x85
	.uleb128 0x2
	.ascii "s32\0"
	.byte	0x2
	.byte	0x4c
	.long	0x160
	.uleb128 0x2
	.ascii "ulong\0"
	.byte	0x2
	.byte	0x4e
	.long	0x41
	.uleb128 0x2
	.ascii "slong\0"
	.byte	0x2
	.byte	0x4f
	.long	0x57
	.uleb128 0x2
	.ascii "size_t\0"
	.byte	0x3
	.byte	0x27
	.long	0x32
	.uleb128 0x5
	.long	0x291
	.ascii "_iobuf\0"
	.byte	0x2c
	.byte	0x3
	.byte	0x2c
	.uleb128 0x6
	.ascii "iob_handle\0"
	.byte	0x3
	.byte	0x2d
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "iob_flag\0"
	.byte	0x3
	.byte	0x2e
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "iob_error\0"
	.byte	0x3
	.byte	0x2f
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "iob_length\0"
	.byte	0x3
	.byte	0x30
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x6
	.ascii "iob_ungetc\0"
	.byte	0x3
	.byte	0x31
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x6
	.ascii "iob_offset\0"
	.byte	0x3
	.byte	0x32
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x6
	.ascii "iob_buffer\0"
	.byte	0x3
	.byte	0x33
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x6
	.ascii "iob_bsize\0"
	.byte	0x3
	.byte	0x34
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x6
	.ascii "iob_pnt\0"
	.byte	0x3
	.byte	0x35
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x6
	.ascii "iob_count\0"
	.byte	0x3
	.byte	0x36
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0x6
	.ascii "iob_end\0"
	.byte	0x3
	.byte	0x37
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.byte	0x0
	.uleb128 0x2
	.ascii "FILE\0"
	.byte	0x3
	.byte	0x3a
	.long	0x1a5
	.uleb128 0x2
	.ascii "fpos_t\0"
	.byte	0x3
	.byte	0x3c
	.long	0x41
	.uleb128 0x5
	.long	0x2da
	.ascii "sysdttm\0"
	.byte	0x8
	.byte	0x3
	.byte	0x6e
	.uleb128 0x6
	.ascii "time\0"
	.byte	0x3
	.byte	0x6f
	.long	0x17d
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "date\0"
	.byte	0x3
	.byte	0x70
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x2
	.ascii "time_s\0"
	.byte	0x3
	.byte	0x71
	.long	0x2ab
	.uleb128 0x5
	.long	0x391
	.ascii "fileinfo\0"
	.byte	0x48
	.byte	0x3
	.byte	0x78
	.uleb128 0x6
	.ascii "wsize\0"
	.byte	0x3
	.byte	0x79
	.long	0x17d
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "asize\0"
	.byte	0x3
	.byte	0x7a
	.long	0x17d
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "create\0"
	.byte	0x3
	.byte	0x7b
	.long	0x2da
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "modify\0"
	.byte	0x3
	.byte	0x7c
	.long	0x2da
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x6
	.ascii "access\0"
	.byte	0x3
	.byte	0x7d
	.long	0x2da
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x6
	.ascii "protect\0"
	.byte	0x3
	.byte	0x7e
	.long	0x17d
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x6
	.ascii "owner\0"
	.byte	0x3
	.byte	0x7f
	.long	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0x6
	.ascii "group\0"
	.byte	0x3
	.byte	0x80
	.long	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x34
	.uleb128 0x6
	.ascii "grpsize\0"
	.byte	0x3
	.byte	0x81
	.long	0x17d
	.byte	0x2
	.byte	0x23
	.uleb128 0x44
	.byte	0x0
	.uleb128 0x7
	.long	0x3a1
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0xf
	.byte	0x0
	.uleb128 0x3
	.long	LC135
	.byte	0x4
	.byte	0x7
	.uleb128 0x4
	.ascii "char\0"
	.byte	0x1
	.byte	0x6
	.uleb128 0x2
	.ascii "file_info\0"
	.byte	0x3
	.byte	0x82
	.long	0x2e8
	.uleb128 0x2
	.ascii "va_list\0"
	.byte	0x3
	.byte	0x89
	.long	0x3d0
	.uleb128 0x9
	.byte	0x4
	.long	0x3a8
	.uleb128 0x2
	.ascii "wchar_t\0"
	.byte	0x4
	.byte	0x30
	.long	0x95
	.uleb128 0xa
	.long	0x40b
	.byte	0x8
	.byte	0x4
	.byte	0x37
	.uleb128 0x6
	.ascii "quot\0"
	.byte	0x4
	.byte	0x35
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "rem\0"
	.byte	0x4
	.byte	0x36
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x2
	.ascii "div_t\0"
	.byte	0x4
	.byte	0x37
	.long	0x3e5
	.uleb128 0xa
	.long	0x43e
	.byte	0x8
	.byte	0x4
	.byte	0x3c
	.uleb128 0x6
	.ascii "quot\0"
	.byte	0x4
	.byte	0x3a
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "rem\0"
	.byte	0x4
	.byte	0x3b
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x2
	.ascii "ldiv_t\0"
	.byte	0x4
	.byte	0x3c
	.long	0x418
	.uleb128 0xa
	.long	0x472
	.byte	0x8
	.byte	0x4
	.byte	0x41
	.uleb128 0x6
	.ascii "low\0"
	.byte	0x4
	.byte	0x3f
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "high\0"
	.byte	0x4
	.byte	0x40
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x2
	.ascii "LLONG\0"
	.byte	0x4
	.byte	0x41
	.long	0x44c
	.uleb128 0x2
	.ascii "FNEXIT\0"
	.byte	0x4
	.byte	0x57
	.long	0x48d
	.uleb128 0x9
	.byte	0x4
	.long	0x493
	.uleb128 0xb
	.long	0x49a
	.uleb128 0xc
	.byte	0x0
	.uleb128 0x2
	.ascii "time_t\0"
	.byte	0x5
	.byte	0x3a
	.long	0x48
	.uleb128 0x2
	.ascii "clock_t\0"
	.byte	0x5
	.byte	0x3c
	.long	0x63
	.uleb128 0x5
	.long	0x563
	.ascii "tm\0"
	.byte	0x24
	.byte	0x5
	.byte	0x41
	.uleb128 0x6
	.ascii "tm_sec\0"
	.byte	0x5
	.byte	0x42
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "tm_min\0"
	.byte	0x5
	.byte	0x43
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "tm_hour\0"
	.byte	0x5
	.byte	0x44
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "tm_mday\0"
	.byte	0x5
	.byte	0x45
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x6
	.ascii "tm_mon\0"
	.byte	0x5
	.byte	0x46
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x6
	.ascii "tm_year\0"
	.byte	0x5
	.byte	0x47
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x6
	.ascii "tm_wday\0"
	.byte	0x5
	.byte	0x48
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x6
	.ascii "tm_yday\0"
	.byte	0x5
	.byte	0x49
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x6
	.ascii "tm_isdst\0"
	.byte	0x5
	.byte	0x4a
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.byte	0x0
	.uleb128 0x5
	.long	0x640
	.ascii "trmmodes\0"
	.byte	0x1c
	.byte	0x6
	.byte	0x43
	.uleb128 0x6
	.ascii "dm_textres\0"
	.byte	0x6
	.byte	0x43
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "dm_actpage\0"
	.byte	0x6
	.byte	0x44
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x6
	.ascii "dm_prifont\0"
	.byte	0x6
	.byte	0x45
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x6
	.ascii "dm_secfont\0"
	.byte	0x6
	.byte	0x46
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x3
	.uleb128 0x6
	.ascii "dm_columns\0"
	.byte	0x6
	.byte	0x47
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "dm_rows\0"
	.byte	0x6
	.byte	0x48
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "dm_horiz\0"
	.byte	0x6
	.byte	0x49
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x6
	.ascii "dm_vert\0"
	.byte	0x6
	.byte	0x4a
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x6
	.ascii "dm_disptype\0"
	.byte	0x6
	.byte	0x4b
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x6
	.ascii "dm_curtype\0"
	.byte	0x6
	.byte	0x4c
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.byte	0x0
	.uleb128 0x2
	.ascii "TRMMODES\0"
	.byte	0x6
	.byte	0x4d
	.long	0x563
	.uleb128 0xd
	.long	0x6c6
	.ascii "trmattrib\0"
	.byte	0x14
	.byte	0x6
	.word	0x142
	.uleb128 0xe
	.ascii "foregnd\0"
	.byte	0x6
	.word	0x142
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xe
	.ascii "backgnd\0"
	.byte	0x6
	.word	0x143
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0xe
	.ascii "forefill\0"
	.byte	0x6
	.word	0x144
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xe
	.ascii "backfill\0"
	.byte	0x6
	.word	0x145
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "attrbits\0"
	.byte	0x6
	.word	0x146
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0xf
	.ascii "TRMATTRIB\0"
	.byte	0x6
	.word	0x147
	.long	0x650
	.uleb128 0x5
	.long	0x83f
	.ascii "qab\0"
	.byte	0x34
	.byte	0x7
	.byte	0xf9
	.uleb128 0x6
	.ascii "qab_func\0"
	.byte	0x7
	.byte	0xfa
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "qab_status\0"
	.byte	0x7
	.byte	0xfb
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x6
	.ascii "qab_error\0"
	.byte	0x7
	.byte	0xfc
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "qab_amount\0"
	.byte	0x7
	.byte	0xfd
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "qab_handle\0"
	.byte	0x7
	.byte	0xfe
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "qab_vector\0"
	.byte	0x7
	.word	0x100
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0xe
	.ascii "qab_level\0"
	.byte	0x7
	.word	0x101
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0xe
	.ascii "qab_prevlvl\0"
	.byte	0x7
	.word	0x103
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x13
	.uleb128 0xe
	.ascii "qab_option\0"
	.byte	0x7
	.word	0x104
	.long	0x17d
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0xe
	.ascii "qab_count\0"
	.byte	0x7
	.word	0x105
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0xe
	.ascii "qab_buffer1\0"
	.byte	0x7
	.word	0x106
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0xe
	.ascii "qab_buffer1x\0"
	.byte	0x7
	.word	0x107
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0xe
	.ascii "qab_buffer2\0"
	.byte	0x7
	.word	0x108
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0xe
	.ascii "qab_buffer2x\0"
	.byte	0x7
	.word	0x109
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0xe
	.ascii "qab_parm\0"
	.byte	0x7
	.word	0x10a
	.long	0x845
	.byte	0x2
	.byte	0x23
	.uleb128 0x2c
	.uleb128 0xe
	.ascii "qab_parmx\0"
	.byte	0x7
	.word	0x10b
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x3a8
	.uleb128 0x10
	.byte	0x4
	.uleb128 0xf
	.ascii "QAB\0"
	.byte	0x7
	.word	0x10c
	.long	0x6d8
	.uleb128 0xd
	.long	0x887
	.ascii "piddata\0"
	.byte	0x4
	.byte	0x7
	.word	0x3b5
	.uleb128 0xe
	.ascii "number\0"
	.byte	0x7
	.word	0x3b6
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xe
	.ascii "code\0"
	.byte	0x7
	.word	0x3b7
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.byte	0x0
	.uleb128 0xf
	.ascii "pid_data\0"
	.byte	0x7
	.word	0x3b8
	.long	0x853
	.uleb128 0xd
	.long	0x8fa
	.ascii "imgdata\0"
	.byte	0x10
	.byte	0x7
	.word	0x3bd
	.uleb128 0xe
	.ascii "imgspec\0"
	.byte	0x7
	.word	0x3be
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xe
	.ascii "imgspecsz\0"
	.byte	0x7
	.word	0x3bf
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0xe
	.ascii "cmdtail\0"
	.byte	0x7
	.word	0x3c0
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xe
	.ascii "cmdtailsz\0"
	.byte	0x7
	.word	0x3c1
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.byte	0x0
	.uleb128 0xf
	.ascii "img_data\0"
	.byte	0x7
	.word	0x3c2
	.long	0x898
	.uleb128 0xd
	.long	0x94c
	.ascii "byte0parm\0"
	.byte	0x4
	.byte	0x7
	.word	0x3c7
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x3c8
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x3c9
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x3ca
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.byte	0x0
	.uleb128 0xf
	.ascii "byte0_parm\0"
	.byte	0x7
	.word	0x3cb
	.long	0x90b
	.uleb128 0xd
	.long	0x9af
	.ascii "byte1parm\0"
	.byte	0x6
	.byte	0x7
	.word	0x3d0
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x3d1
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x3d2
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x3d3
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x3d4
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0xf
	.ascii "byte1_parm\0"
	.byte	0x7
	.word	0x3d5
	.long	0x95f
	.uleb128 0xd
	.long	0xa12
	.ascii "byte2parm\0"
	.byte	0x6
	.byte	0x7
	.word	0x3da
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x3db
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x3dc
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x3dd
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x3de
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0xf
	.ascii "byte2_parm\0"
	.byte	0x7
	.word	0x3df
	.long	0x9c2
	.uleb128 0xd
	.long	0xa75
	.ascii "byte4parm\0"
	.byte	0x8
	.byte	0x7
	.word	0x3e4
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x3e5
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x3e6
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x3e7
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x3e8
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0xf
	.ascii "byte4_parm\0"
	.byte	0x7
	.word	0x3e9
	.long	0xa25
	.uleb128 0xd
	.long	0xad8
	.ascii "byte8parm\0"
	.byte	0xc
	.byte	0x7
	.word	0x3ee
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x3ef
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x3f0
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x3f1
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x3f2
	.long	0xad8
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x7
	.long	0xae8
	.long	0x57
	.uleb128 0x8
	.long	0x3a1
	.byte	0x1
	.byte	0x0
	.uleb128 0xf
	.ascii "byte8_parm\0"
	.byte	0x7
	.word	0x3f3
	.long	0xa88
	.uleb128 0xd
	.long	0xb4b
	.ascii "time8parm\0"
	.byte	0xc
	.byte	0x7
	.word	0x3f8
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x3f9
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x3fa
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x3fb
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x3fc
	.long	0x2da
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0xf
	.ascii "time8_parm\0"
	.byte	0x7
	.word	0x3fd
	.long	0xafb
	.uleb128 0xd
	.long	0xbae
	.ascii "text2parm\0"
	.byte	0x6
	.byte	0x7
	.word	0x402
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x403
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x404
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x405
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x406
	.long	0xbae
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x7
	.long	0xbbe
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0x1
	.byte	0x0
	.uleb128 0xf
	.ascii "text2_parm\0"
	.byte	0x7
	.word	0x407
	.long	0xb5e
	.uleb128 0xd
	.long	0xc21
	.ascii "text4parm\0"
	.byte	0x8
	.byte	0x7
	.word	0x40c
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x40d
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x40e
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x40f
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x410
	.long	0xc21
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x7
	.long	0xc31
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0x3
	.byte	0x0
	.uleb128 0xf
	.ascii "text4_parm\0"
	.byte	0x7
	.word	0x411
	.long	0xbd1
	.uleb128 0xd
	.long	0xc94
	.ascii "text8parm\0"
	.byte	0xc
	.byte	0x7
	.word	0x416
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x417
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x418
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x419
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x41a
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x7
	.long	0xca4
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0x7
	.byte	0x0
	.uleb128 0xf
	.ascii "text8_parm\0"
	.byte	0x7
	.word	0x41b
	.long	0xc44
	.uleb128 0xd
	.long	0xd08
	.ascii "byte16parm\0"
	.byte	0x14
	.byte	0x7
	.word	0x420
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x421
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x422
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x423
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x424
	.long	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0xf
	.ascii "byte16_parm\0"
	.byte	0x7
	.word	0x425
	.long	0xcb7
	.uleb128 0xd
	.long	0xd9a
	.ascii "lngstrparm\0"
	.byte	0x10
	.byte	0x7
	.word	0x42a
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x42b
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x42c
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC138
	.byte	0x7
	.word	0x42d
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC140
	.byte	0x7
	.word	0x42e
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0xe
	.ascii "xxx\0"
	.byte	0x7
	.word	0x42f
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x11
	.long	LC141
	.byte	0x7
	.word	0x430
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.uleb128 0x11
	.long	LC142
	.byte	0x7
	.word	0x431
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.byte	0x0
	.uleb128 0xf
	.ascii "lngstr_parm\0"
	.byte	0x7
	.word	0x432
	.long	0xd1c
	.uleb128 0xd
	.long	0xdfe
	.ascii "byte1char\0"
	.byte	0xb
	.byte	0x7
	.word	0x437
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x438
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x439
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x43a
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x43b
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.byte	0x0
	.uleb128 0xf
	.ascii "byte1_char\0"
	.byte	0x7
	.word	0x43c
	.long	0xdae
	.uleb128 0xd
	.long	0xe9f
	.ascii "byte1charip\0"
	.byte	0x18
	.byte	0x7
	.word	0x43f
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x440
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x441
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x442
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC144
	.byte	0x7
	.word	0x443
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "xxx\0"
	.byte	0x7
	.word	0x444
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x11
	.long	LC145
	.byte	0x7
	.word	0x445
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x11
	.long	LC146
	.byte	0x7
	.word	0x446
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x447
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x16
	.byte	0x0
	.uleb128 0xf
	.ascii "byte1_charip\0"
	.byte	0x7
	.word	0x448
	.long	0xe11
	.uleb128 0xd
	.long	0xf04
	.ascii "byte2char\0"
	.byte	0xc
	.byte	0x7
	.word	0x44d
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x44e
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x44f
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x450
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x451
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.byte	0x0
	.uleb128 0xf
	.ascii "byte2_char\0"
	.byte	0x7
	.word	0x452
	.long	0xeb4
	.uleb128 0xd
	.long	0xfa5
	.ascii "byte2charip\0"
	.byte	0x18
	.byte	0x7
	.word	0x455
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x456
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x457
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x458
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC144
	.byte	0x7
	.word	0x459
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "xxx\0"
	.byte	0x7
	.word	0x45a
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x11
	.long	LC145
	.byte	0x7
	.word	0x45b
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x11
	.long	LC146
	.byte	0x7
	.word	0x45c
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x45d
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x16
	.byte	0x0
	.uleb128 0xf
	.ascii "byte2_charip\0"
	.byte	0x7
	.word	0x45e
	.long	0xf17
	.uleb128 0xd
	.long	0x100a
	.ascii "byte4char\0"
	.byte	0x10
	.byte	0x7
	.word	0x463
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x464
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x465
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x466
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x467
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.byte	0x0
	.uleb128 0xf
	.ascii "byte4_char\0"
	.byte	0x7
	.word	0x468
	.long	0xfba
	.uleb128 0xd
	.long	0x10ab
	.ascii "byte4charip\0"
	.byte	0x1c
	.byte	0x7
	.word	0x46b
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x46c
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x46d
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x46e
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC144
	.byte	0x7
	.word	0x46f
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "xxx\0"
	.byte	0x7
	.word	0x470
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x11
	.long	LC145
	.byte	0x7
	.word	0x471
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x11
	.long	LC146
	.byte	0x7
	.word	0x472
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x473
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.byte	0x0
	.uleb128 0xf
	.ascii "byte4_charip\0"
	.byte	0x7
	.word	0x474
	.long	0x101d
	.uleb128 0xd
	.long	0x1110
	.ascii "text2char\0"
	.byte	0xc
	.byte	0x7
	.word	0x477
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x478
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x479
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x47a
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x47b
	.long	0xbae
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.byte	0x0
	.uleb128 0xf
	.ascii "text2_char\0"
	.byte	0x7
	.word	0x47c
	.long	0x10c0
	.uleb128 0xd
	.long	0x1173
	.ascii "text4char\0"
	.byte	0xe
	.byte	0x7
	.word	0x47f
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x480
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x481
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x482
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x483
	.long	0xc21
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.byte	0x0
	.uleb128 0xf
	.ascii "text4_char\0"
	.byte	0x7
	.word	0x484
	.long	0x1123
	.uleb128 0xd
	.long	0x1214
	.ascii "text4charip\0"
	.byte	0x1c
	.byte	0x7
	.word	0x487
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x488
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x489
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x48a
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC144
	.byte	0x7
	.word	0x48b
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "xxx\0"
	.byte	0x7
	.word	0x48c
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x11
	.long	LC145
	.byte	0x7
	.word	0x48d
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x11
	.long	LC146
	.byte	0x7
	.word	0x48e
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x48f
	.long	0xc21
	.byte	0x2
	.byte	0x23
	.uleb128 0x16
	.byte	0x0
	.uleb128 0xf
	.ascii "text4_charip\0"
	.byte	0x7
	.word	0x490
	.long	0x1186
	.uleb128 0xd
	.long	0x1279
	.ascii "text8char\0"
	.byte	0x12
	.byte	0x7
	.word	0x493
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x494
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x495
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x496
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x497
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.byte	0x0
	.uleb128 0xf
	.ascii "text8_char\0"
	.byte	0x7
	.word	0x498
	.long	0x1229
	.uleb128 0xd
	.long	0x131a
	.ascii "text8charip\0"
	.byte	0x20
	.byte	0x7
	.word	0x49b
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x49c
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x49d
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x49e
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC144
	.byte	0x7
	.word	0x49f
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "xxx\0"
	.byte	0x7
	.word	0x4a0
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x11
	.long	LC145
	.byte	0x7
	.word	0x4a1
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x11
	.long	LC146
	.byte	0x7
	.word	0x4a2
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x4a3
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x16
	.byte	0x0
	.uleb128 0xf
	.ascii "text8_charip\0"
	.byte	0x7
	.word	0x4a4
	.long	0x128c
	.uleb128 0xd
	.long	0x1380
	.ascii "text16char\0"
	.byte	0x1a
	.byte	0x7
	.word	0x4a7
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x4a8
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x4a9
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x4aa
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x4ab
	.long	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.byte	0x0
	.uleb128 0xf
	.ascii "text16_char\0"
	.byte	0x7
	.word	0x4ac
	.long	0x132f
	.uleb128 0xd
	.long	0x1423
	.ascii "text16charip\0"
	.byte	0x28
	.byte	0x7
	.word	0x4af
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x4b0
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x4b1
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x4b2
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC144
	.byte	0x7
	.word	0x4b3
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "xxx\0"
	.byte	0x7
	.word	0x4b4
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x11
	.long	LC145
	.byte	0x7
	.word	0x4b5
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x11
	.long	LC146
	.byte	0x7
	.word	0x4b6
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x11
	.long	LC139
	.byte	0x7
	.word	0x4b7
	.long	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x16
	.byte	0x0
	.uleb128 0xf
	.ascii "text16_charip\0"
	.byte	0x7
	.word	0x4b8
	.long	0x1394
	.uleb128 0xd
	.long	0x14b7
	.ascii "lngstrchar\0"
	.byte	0x18
	.byte	0x7
	.word	0x4bd
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x4be
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x4bf
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x4c0
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC140
	.byte	0x7
	.word	0x4c1
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "xxx\0"
	.byte	0x7
	.word	0x4c2
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x11
	.long	LC141
	.byte	0x7
	.word	0x4c3
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x11
	.long	LC142
	.byte	0x7
	.word	0x4c4
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.byte	0x0
	.uleb128 0xf
	.ascii "lngstr_char\0"
	.byte	0x7
	.word	0x4c5
	.long	0x1439
	.uleb128 0xd
	.long	0x1589
	.ascii "lngstrcharip\0"
	.byte	0x24
	.byte	0x7
	.word	0x4c8
	.uleb128 0x11
	.long	LC136
	.byte	0x7
	.word	0x4c9
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC137
	.byte	0x7
	.word	0x4ca
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0x11
	.long	LC143
	.byte	0x7
	.word	0x4cb
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x11
	.long	LC144
	.byte	0x7
	.word	0x4cc
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xe
	.ascii "xxx1\0"
	.byte	0x7
	.word	0x4cd
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x11
	.long	LC145
	.byte	0x7
	.word	0x4ce
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x11
	.long	LC146
	.byte	0x7
	.word	0x4cf
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x11
	.long	LC140
	.byte	0x7
	.word	0x4d0
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0xe
	.ascii "xxx2\0"
	.byte	0x7
	.word	0x4d1
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x11
	.long	LC141
	.byte	0x7
	.word	0x4d2
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x1e
	.uleb128 0x11
	.long	LC142
	.byte	0x7
	.word	0x4d3
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.byte	0x0
	.uleb128 0xf
	.ascii "lngstr_charip\0"
	.byte	0x7
	.word	0x4d4
	.long	0x14cb
	.uleb128 0x5
	.long	0x1636
	.ascii "tmx\0"
	.byte	0x12
	.byte	0x8
	.byte	0x2f
	.uleb128 0x12
	.long	LC147
	.byte	0x8
	.byte	0x2f
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "tmx_sec\0"
	.byte	0x8
	.byte	0x30
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x6
	.ascii "tmx_min\0"
	.byte	0x8
	.byte	0x31
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.long	LC148
	.byte	0x8
	.byte	0x32
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x6
	.uleb128 0x12
	.long	LC149
	.byte	0x8
	.byte	0x33
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "tmx_mon\0"
	.byte	0x8
	.byte	0x34
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.uleb128 0x12
	.long	LC150
	.byte	0x8
	.byte	0x35
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x12
	.long	LC151
	.byte	0x8
	.byte	0x36
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0xe
	.uleb128 0x12
	.long	LC152
	.byte	0x8
	.byte	0x37
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0x2
	.ascii "time_d\0"
	.byte	0x8
	.byte	0x38
	.long	0x159f
	.uleb128 0x5
	.long	0x1703
	.ascii "tmxz\0"
	.byte	0x16
	.byte	0x8
	.byte	0x3b
	.uleb128 0x12
	.long	LC147
	.byte	0x8
	.byte	0x3b
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "tmx_sec\0"
	.byte	0x8
	.byte	0x3c
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x6
	.ascii "tmx_min\0"
	.byte	0x8
	.byte	0x3d
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.long	LC148
	.byte	0x8
	.byte	0x3e
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x6
	.uleb128 0x12
	.long	LC149
	.byte	0x8
	.byte	0x3f
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "tmx_mon\0"
	.byte	0x8
	.byte	0x40
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.uleb128 0x12
	.long	LC150
	.byte	0x8
	.byte	0x41
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x12
	.long	LC151
	.byte	0x8
	.byte	0x42
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0xe
	.uleb128 0x12
	.long	LC152
	.byte	0x8
	.byte	0x43
	.long	0xa5
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x6
	.ascii "tmx_tzone\0"
	.byte	0x8
	.byte	0x44
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x6
	.ascii "tmx_dlst\0"
	.byte	0x8
	.byte	0x45
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.byte	0x0
	.uleb128 0x2
	.ascii "time_dz\0"
	.byte	0x8
	.byte	0x46
	.long	0x1644
	.uleb128 0x5
	.long	0x1762
	.ascii "sysdttmtz\0"
	.byte	0xc
	.byte	0x8
	.byte	0x71
	.uleb128 0x6
	.ascii "time\0"
	.byte	0x8
	.byte	0x72
	.long	0x41
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "date\0"
	.byte	0x8
	.byte	0x73
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "tzone\0"
	.byte	0x8
	.byte	0x74
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "dlst\0"
	.byte	0x8
	.byte	0x75
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.byte	0x0
	.uleb128 0x2
	.ascii "time_sz\0"
	.byte	0x8
	.byte	0x76
	.long	0x1712
	.uleb128 0xa
	.long	0x1796
	.byte	0x1c
	.byte	0x8
	.byte	0x7c
	.uleb128 0x6
	.ascii "sys\0"
	.byte	0x8
	.byte	0x7a
	.long	0x2da
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "dos\0"
	.byte	0x8
	.byte	0x7b
	.long	0x1636
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.byte	0x0
	.uleb128 0x2
	.ascii "time_x\0"
	.byte	0x8
	.byte	0x7c
	.long	0x1771
	.uleb128 0x5
	.long	0x17d8
	.ascii "hrtime\0"
	.byte	0x8
	.byte	0x8
	.byte	0x7f
	.uleb128 0x6
	.ascii "second\0"
	.byte	0x8
	.byte	0x80
	.long	0x41
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "fraction\0"
	.byte	0x8
	.byte	0x81
	.long	0x41
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x13
	.long	0x17fc
	.byte	0x4
	.byte	0x9
	.byte	0x43
	.uleb128 0x14
	.ascii "n\0"
	.byte	0x9
	.byte	0x40
	.long	0x57
	.uleb128 0x14
	.ascii "s\0"
	.byte	0x9
	.byte	0x41
	.long	0x83f
	.uleb128 0x14
	.ascii "c\0"
	.byte	0x9
	.byte	0x42
	.long	0xc21
	.byte	0x0
	.uleb128 0x5
	.long	0x1859
	.ascii "argdata\0"
	.byte	0x14
	.byte	0x9
	.byte	0x3e
	.uleb128 0x12
	.long	LC143
	.byte	0x9
	.byte	0x3e
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "val\0"
	.byte	0x9
	.byte	0x43
	.long	0x17d8
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "data\0"
	.byte	0x9
	.byte	0x44
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "flags\0"
	.byte	0x9
	.byte	0x45
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x6
	.ascii "length\0"
	.byte	0x9
	.byte	0x46
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0x2
	.ascii "arg_data\0"
	.byte	0x9
	.byte	0x47
	.long	0x17fc
	.uleb128 0x5
	.long	0x189d
	.ascii "suboptions\0"
	.byte	0x8
	.byte	0x9
	.byte	0x5a
	.uleb128 0x6
	.ascii "option\0"
	.byte	0x9
	.byte	0x5a
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "desc\0"
	.byte	0x9
	.byte	0x5b
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x2
	.ascii "SubOpts\0"
	.byte	0x9
	.byte	0x5c
	.long	0x1869
	.uleb128 0x13
	.long	0x18c7
	.byte	0x10
	.byte	0x9
	.byte	0x66
	.uleb128 0x14
	.ascii "s\0"
	.byte	0x9
	.byte	0x64
	.long	0x18c7
	.uleb128 0x14
	.ascii "l\0"
	.byte	0x9
	.byte	0x65
	.long	0x18cd
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x189d
	.uleb128 0x7
	.long	0x18dd
	.long	0x18dd
	.uleb128 0x8
	.long	0x3a1
	.byte	0x3
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x57
	.uleb128 0x13
	.long	0x1901
	.byte	0x4
	.byte	0x9
	.byte	0x6a
	.uleb128 0x14
	.ascii "func\0"
	.byte	0x9
	.byte	0x68
	.long	0x1917
	.uleb128 0x14
	.ascii "t\0"
	.byte	0x9
	.byte	0x69
	.long	0x845
	.byte	0x0
	.uleb128 0x15
	.long	0x1911
	.byte	0x1
	.long	0x57
	.uleb128 0x16
	.long	0x1911
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x1859
	.uleb128 0x9
	.byte	0x4
	.long	0x1901
	.uleb128 0x13
	.long	0x1941
	.byte	0x8
	.byte	0x9
	.byte	0x6f
	.uleb128 0x14
	.ascii "l\0"
	.byte	0x9
	.byte	0x6c
	.long	0x57
	.uleb128 0x14
	.ascii "i\0"
	.byte	0x9
	.byte	0x6d
	.long	0x1941
	.uleb128 0x14
	.ascii "c\0"
	.byte	0x9
	.byte	0x6e
	.long	0xc21
	.byte	0x0
	.uleb128 0x7
	.long	0x1951
	.long	0x160
	.uleb128 0x8
	.long	0x3a1
	.byte	0x1
	.byte	0x0
	.uleb128 0x5
	.long	0x19c3
	.ascii "argspec\0"
	.byte	0x28
	.byte	0x9
	.byte	0x61
	.uleb128 0x12
	.long	LC143
	.byte	0x9
	.byte	0x61
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "flags\0"
	.byte	0x9
	.byte	0x62
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "svalues\0"
	.byte	0x9
	.byte	0x66
	.long	0x18ac
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "func\0"
	.byte	0x9
	.byte	0x6a
	.long	0x18e3
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x6
	.ascii "data\0"
	.byte	0x9
	.byte	0x6f
	.long	0x191d
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x6
	.ascii "help_str\0"
	.byte	0x9
	.byte	0x70
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.byte	0x0
	.uleb128 0x2
	.ascii "arg_spec\0"
	.byte	0x9
	.byte	0x71
	.long	0x1951
	.uleb128 0x5
	.long	0x1a12
	.ascii "strnode\0"
	.byte	0xc
	.byte	0xa
	.byte	0x32
	.uleb128 0x6
	.ascii "prev\0"
	.byte	0xa
	.byte	0x33
	.long	0x1a12
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.long	LC153
	.byte	0xa
	.byte	0x34
	.long	0x1a12
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "string\0"
	.byte	0xa
	.byte	0x35
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x19d3
	.uleb128 0x2
	.ascii "STR_NODE\0"
	.byte	0xa
	.byte	0x36
	.long	0x19d3
	.uleb128 0x5
	.long	0x1ab2
	.ascii "strctrlb\0"
	.byte	0x20
	.byte	0xa
	.byte	0x38
	.uleb128 0x6
	.ascii "total\0"
	.byte	0xa
	.byte	0x39
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "pos\0"
	.byte	0xa
	.byte	0x3a
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "page\0"
	.byte	0xa
	.byte	0x3b
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "lines\0"
	.byte	0xa
	.byte	0x3c
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x6
	.ascii "head\0"
	.byte	0xa
	.byte	0x3d
	.long	0x1ab2
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x6
	.ascii "tail\0"
	.byte	0xa
	.byte	0x3e
	.long	0x1ab2
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x6
	.ascii "curr\0"
	.byte	0xa
	.byte	0x3f
	.long	0x1ab2
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x12
	.long	LC140
	.byte	0xa
	.byte	0x40
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x1a18
	.uleb128 0x2
	.ascii "STR_CB\0"
	.byte	0xa
	.byte	0x41
	.long	0x1a28
	.uleb128 0x5
	.long	0x1b1e
	.ascii "col_data\0"
	.byte	0x14
	.byte	0xa
	.byte	0x54
	.uleb128 0x6
	.ascii "fgc\0"
	.byte	0xa
	.byte	0x55
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "bgc\0"
	.byte	0xa
	.byte	0x56
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "fgf\0"
	.byte	0xa
	.byte	0x57
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "bgf\0"
	.byte	0xa
	.byte	0x58
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x6
	.ascii "atr\0"
	.byte	0xa
	.byte	0x59
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0x2
	.ascii "COLDATA\0"
	.byte	0xa
	.byte	0x5a
	.long	0x1ac6
	.uleb128 0x5
	.long	0x1c99
	.ascii "proginfo\0"
	.byte	0x98
	.byte	0xa
	.byte	0x5c
	.uleb128 0x6
	.ascii "console\0"
	.byte	0xa
	.byte	0x5d
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "majedt\0"
	.byte	0xa
	.byte	0x5e
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "minedt\0"
	.byte	0xa
	.byte	0x5f
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "errno\0"
	.byte	0xa
	.byte	0x60
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x6
	.ascii "screen_width\0"
	.byte	0xa
	.byte	0x61
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x6
	.ascii "screen_height\0"
	.byte	0xa
	.byte	0x62
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x6
	.ascii "handle\0"
	.byte	0xa
	.byte	0x63
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x6
	.ascii "page\0"
	.byte	0xa
	.byte	0x64
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x6
	.ascii "old_color\0"
	.byte	0xa
	.byte	0x65
	.long	0x1b1e
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x6
	.ascii "hdr_color\0"
	.byte	0xa
	.byte	0x66
	.long	0x1b1e
	.byte	0x2
	.byte	0x23
	.uleb128 0x34
	.uleb128 0x6
	.ascii "bdy_color\0"
	.byte	0xa
	.byte	0x67
	.long	0x1b1e
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.uleb128 0x6
	.ascii "scb\0"
	.byte	0xa
	.byte	0x68
	.long	0x1ab8
	.byte	0x2
	.byte	0x23
	.uleb128 0x5c
	.uleb128 0x6
	.ascii "copymsg\0"
	.byte	0xa
	.byte	0x69
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x7c
	.uleb128 0x6
	.ascii "prgname\0"
	.byte	0xa
	.byte	0x6a
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0x80
	.uleb128 0x6
	.ascii "build\0"
	.byte	0xa
	.byte	0x6b
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0x84
	.uleb128 0x6
	.ascii "desc\0"
	.byte	0xa
	.byte	0x6c
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0x88
	.uleb128 0x6
	.ascii "example\0"
	.byte	0xa
	.byte	0x6d
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0x8c
	.uleb128 0x6
	.ascii "opttbl\0"
	.byte	0xa
	.byte	0x6e
	.long	0x1c99
	.byte	0x3
	.byte	0x23
	.uleb128 0x90
	.uleb128 0x6
	.ascii "kwdtbl\0"
	.byte	0xa
	.byte	0x6f
	.long	0x1c99
	.byte	0x3
	.byte	0x23
	.uleb128 0x94
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x19c3
	.uleb128 0x2
	.ascii "Prog_Info\0"
	.byte	0xa
	.byte	0x70
	.long	0x1b2d
	.uleb128 0x5
	.long	0x1d17
	.ascii "dirscanpl\0"
	.byte	0x30
	.byte	0xb
	.byte	0x33
	.uleb128 0x12
	.long	LC154
	.byte	0xb
	.byte	0x33
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.long	LC155
	.byte	0xb
	.byte	0x34
	.long	0xd9a
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.long	LC156
	.byte	0xb
	.byte	0x35
	.long	0xa12
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x12
	.long	LC157
	.byte	0xb
	.byte	0x36
	.long	0xa12
	.byte	0x2
	.byte	0x23
	.uleb128 0x1e
	.uleb128 0x12
	.long	LC158
	.byte	0xb
	.byte	0x37
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0x6
	.ascii "end\0"
	.byte	0xb
	.byte	0x38
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x2c
	.byte	0x0
	.uleb128 0x2
	.ascii "DIRSCANPL\0"
	.byte	0xb
	.byte	0x39
	.long	0x1cb0
	.uleb128 0x2
	.ascii "DIRNAME\0"
	.byte	0xb
	.byte	0x46
	.long	0x1d37
	.uleb128 0x5
	.long	0x1d76
	.ascii "dirname\0"
	.byte	0xc
	.byte	0xb
	.byte	0x46
	.uleb128 0x12
	.long	LC153
	.byte	0xb
	.byte	0x48
	.long	0x1d76
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "sortpnt\0"
	.byte	0xb
	.byte	0x49
	.long	0x1d76
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.long	LC143
	.byte	0xb
	.byte	0x4a
	.long	0xbae
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x1d28
	.uleb128 0x2
	.ascii "RMTDATA\0"
	.byte	0xb
	.byte	0x5b
	.long	0x1d8b
	.uleb128 0x5
	.long	0x1e2e
	.ascii "rmtdata\0"
	.byte	0x38
	.byte	0xb
	.byte	0x5b
	.uleb128 0x12
	.long	LC153
	.byte	0xb
	.byte	0x5d
	.long	0x1e2e
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "nodename\0"
	.byte	0xb
	.byte	0x5e
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "nodenamelen\0"
	.byte	0xb
	.byte	0x5f
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "nodenamebase\0"
	.byte	0xb
	.byte	0x60
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x6
	.ascii "nodenamesize\0"
	.byte	0xb
	.byte	0x61
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x6
	.ascii "rmtdevname\0"
	.byte	0xb
	.byte	0x62
	.long	0x1e34
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x6
	.ascii "rmtdevnamelen\0"
	.byte	0xb
	.byte	0x63
	.long	0x160
	.byte	0x2
	.byte	0x23
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x1d7c
	.uleb128 0x7
	.long	0x1e44
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0x1f
	.byte	0x0
	.uleb128 0xa
	.long	0x1e93
	.byte	0x2c
	.byte	0xb
	.byte	0xb6
	.uleb128 0x12
	.long	LC154
	.byte	0xb
	.byte	0xb1
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.long	LC155
	.byte	0xb
	.byte	0xb2
	.long	0xd9a
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.long	LC156
	.byte	0xb
	.byte	0xb3
	.long	0xa12
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x12
	.long	LC159
	.byte	0xb
	.byte	0xb4
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x6
	.ascii "end\0"
	.byte	0xb
	.byte	0xb5
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.byte	0x0
	.uleb128 0xa
	.long	0x1ec8
	.byte	0x14
	.byte	0xb
	.byte	0xbb
	.uleb128 0x6
	.ascii "iopos\0"
	.byte	0xb
	.byte	0xb8
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.long	LC159
	.byte	0xb
	.byte	0xb9
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "end\0"
	.byte	0xb
	.byte	0xba
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0xa
	.long	0x1f17
	.byte	0x2c
	.byte	0xb
	.byte	0xc2
	.uleb128 0x12
	.long	LC154
	.byte	0xb
	.byte	0xbd
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.long	LC155
	.byte	0xb
	.byte	0xbe
	.long	0xd9a
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.long	LC156
	.byte	0xb
	.byte	0xbf
	.long	0xa12
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x12
	.long	LC158
	.byte	0xb
	.byte	0xc0
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x6
	.ascii "end\0"
	.byte	0xb
	.byte	0xc1
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.byte	0x0
	.uleb128 0x17
	.long	0x2298
	.ascii "dirscandata\0"
	.word	0x1c8
	.byte	0xb
	.byte	0x79
	.uleb128 0x6
	.ascii "parmlist\0"
	.byte	0xb
	.byte	0x79
	.long	0x2298
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "repeat\0"
	.byte	0xb
	.byte	0x7a
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "dfltwildext\0"
	.byte	0xb
	.byte	0x7b
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x5
	.uleb128 0x6
	.ascii "function\0"
	.byte	0xb
	.byte	0x7c
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x6
	.uleb128 0x6
	.ascii "sort\0"
	.byte	0xb
	.byte	0x7f
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x7
	.uleb128 0x6
	.ascii "hvellip\0"
	.byte	0xb
	.byte	0x80
	.long	0x22b4
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "func\0"
	.byte	0xb
	.byte	0x82
	.long	0x22b4
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x12
	.long	LC160
	.byte	0xb
	.byte	0x84
	.long	0x22d6
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x6
	.ascii "changed\0"
	.byte	0xb
	.byte	0x86
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x6
	.ascii "xxx\0"
	.byte	0xb
	.byte	0x87
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x15
	.uleb128 0x6
	.ascii "attr\0"
	.byte	0xb
	.byte	0x88
	.long	0x125
	.byte	0x2
	.byte	0x23
	.uleb128 0x16
	.uleb128 0x6
	.ascii "level\0"
	.byte	0xb
	.byte	0x8a
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x6
	.ascii "devname\0"
	.byte	0xb
	.byte	0x8b
	.long	0x1e34
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x6
	.ascii "devnamelen\0"
	.byte	0xb
	.byte	0x8c
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x3c
	.uleb128 0x6
	.ascii "rmtdata\0"
	.byte	0xb
	.byte	0x8e
	.long	0x1d7c
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x6
	.ascii "pathname\0"
	.byte	0xb
	.byte	0x90
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x78
	.uleb128 0x6
	.ascii "pathnamelen\0"
	.byte	0xb
	.byte	0x91
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x7c
	.uleb128 0x6
	.ascii "pathnamebase\0"
	.byte	0xb
	.byte	0x92
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0x80
	.uleb128 0x6
	.ascii "pathnamesize\0"
	.byte	0xb
	.byte	0x93
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0x84
	.uleb128 0x6
	.ascii "pathdos\0"
	.byte	0xb
	.byte	0x95
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0x88
	.uleb128 0x6
	.ascii "pathdoslen\0"
	.byte	0xb
	.byte	0x96
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0x8c
	.uleb128 0x6
	.ascii "pathdosbase\0"
	.byte	0xb
	.byte	0x97
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0x90
	.uleb128 0x6
	.ascii "pathdossize\0"
	.byte	0xb
	.byte	0x98
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0x94
	.uleb128 0x6
	.ascii "filename\0"
	.byte	0xb
	.byte	0x9a
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0x98
	.uleb128 0x6
	.ascii "filenamelen\0"
	.byte	0xb
	.byte	0x9b
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0x9c
	.uleb128 0x6
	.ascii "filenamebase\0"
	.byte	0xb
	.byte	0x9c
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0xa0
	.uleb128 0x6
	.ascii "filenamesize\0"
	.byte	0xb
	.byte	0x9d
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0xa4
	.uleb128 0x6
	.ascii "filedos\0"
	.byte	0xb
	.byte	0x9f
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0xa8
	.uleb128 0x6
	.ascii "filedoslen\0"
	.byte	0xb
	.byte	0xa0
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0xac
	.uleb128 0x6
	.ascii "filedosbase\0"
	.byte	0xb
	.byte	0xa1
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0xb0
	.uleb128 0x6
	.ascii "filedossize\0"
	.byte	0xb
	.byte	0xa2
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0xb4
	.uleb128 0x6
	.ascii "version\0"
	.byte	0xb
	.byte	0xa4
	.long	0x1e34
	.byte	0x3
	.byte	0x23
	.uleb128 0xb8
	.uleb128 0x6
	.ascii "versionlen\0"
	.byte	0xb
	.byte	0xa5
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0xd8
	.uleb128 0x6
	.ascii "error\0"
	.byte	0xb
	.byte	0xa7
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0xdc
	.uleb128 0x6
	.ascii "newfilespec\0"
	.byte	0xb
	.byte	0xaa
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0xe0
	.uleb128 0x6
	.ascii "namepnt\0"
	.byte	0xb
	.byte	0xab
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0xe4
	.uleb128 0x6
	.ascii "elippos\0"
	.byte	0xb
	.byte	0xac
	.long	0x83f
	.byte	0x3
	.byte	0x23
	.uleb128 0xe8
	.uleb128 0x6
	.ascii "firstdir\0"
	.byte	0xb
	.byte	0xae
	.long	0x1d76
	.byte	0x3
	.byte	0x23
	.uleb128 0xec
	.uleb128 0x6
	.ascii "thisdir\0"
	.byte	0xb
	.byte	0xaf
	.long	0x1d76
	.byte	0x3
	.byte	0x23
	.uleb128 0xf0
	.uleb128 0x6
	.ascii "diropnparm\0"
	.byte	0xb
	.byte	0xb6
	.long	0x1e44
	.byte	0x3
	.byte	0x23
	.uleb128 0xf4
	.uleb128 0x6
	.ascii "dirposparm\0"
	.byte	0xb
	.byte	0xbb
	.long	0x1e93
	.byte	0x3
	.byte	0x23
	.uleb128 0x120
	.uleb128 0x6
	.ascii "dirsrchparm\0"
	.byte	0xb
	.byte	0xc2
	.long	0x1ec8
	.byte	0x3
	.byte	0x23
	.uleb128 0x134
	.uleb128 0x6
	.ascii "dirqab\0"
	.byte	0xb
	.byte	0xc3
	.long	0x847
	.byte	0x3
	.byte	0x23
	.uleb128 0x160
	.uleb128 0x6
	.ascii "fileqab\0"
	.byte	0xb
	.byte	0xc4
	.long	0x847
	.byte	0x3
	.byte	0x23
	.uleb128 0x194
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x1d17
	.uleb128 0x15
	.long	0x22ae
	.byte	0x1
	.long	0x57
	.uleb128 0x16
	.long	0x22ae
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x1f17
	.uleb128 0x9
	.byte	0x4
	.long	0x229e
	.uleb128 0x18
	.long	0x22cb
	.byte	0x1
	.uleb128 0x16
	.long	0x22cb
	.uleb128 0x16
	.long	0x57
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x22d1
	.uleb128 0x19
	.long	0x3a8
	.uleb128 0x9
	.byte	0x4
	.long	0x22ba
	.uleb128 0x2
	.ascii "DIRSCANDATA\0"
	.byte	0xb
	.byte	0xc5
	.long	0x1f17
	.uleb128 0x2
	.ascii "FILEDESCP\0"
	.byte	0x1
	.byte	0x6d
	.long	0x2300
	.uleb128 0x5
	.long	0x236b
	.ascii "filedescp\0"
	.byte	0x14
	.byte	0x1
	.byte	0x6d
	.uleb128 0x12
	.long	LC153
	.byte	0x1
	.byte	0x8b
	.long	0x243d
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "sort\0"
	.byte	0x1
	.byte	0x8c
	.long	0x243d
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.long	LC157
	.byte	0x1
	.byte	0x8d
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x6
	.ascii "xxx\0"
	.byte	0x1
	.byte	0x8e
	.long	0xbb
	.byte	0x2
	.byte	0x23
	.uleb128 0x9
	.uleb128 0x6
	.ascii "error\0"
	.byte	0x1
	.byte	0x8f
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x6
	.ascii "data\0"
	.byte	0x1
	.byte	0x90
	.long	0xbae
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0x2
	.ascii "FS\0"
	.byte	0x1
	.byte	0x71
	.long	0x2375
	.uleb128 0x5
	.long	0x23e9
	.ascii "fs\0"
	.byte	0x28
	.byte	0x1
	.byte	0x71
	.uleb128 0x12
	.long	LC153
	.byte	0x1
	.byte	0x73
	.long	0x23e9
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x6
	.ascii "link\0"
	.byte	0x1
	.byte	0x74
	.long	0x23e9
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x6
	.ascii "glbid\0"
	.byte	0x1
	.byte	0x76
	.long	0x23ef
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.long	LC159
	.byte	0x1
	.byte	0x77
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x6
	.ascii "ellip\0"
	.byte	0x1
	.byte	0x78
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x12
	.long	LC143
	.byte	0x1
	.byte	0x79
	.long	0x83f
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x6
	.ascii "spec\0"
	.byte	0x1
	.byte	0x7a
	.long	0xc21
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x236b
	.uleb128 0x7
	.long	0x23ff
	.long	0x57
	.uleb128 0x8
	.long	0x3a1
	.byte	0x3
	.byte	0x0
	.uleb128 0x2
	.ascii "XNAME\0"
	.byte	0x1
	.byte	0x80
	.long	0x240c
	.uleb128 0x5
	.long	0x2437
	.ascii "xname\0"
	.byte	0x8
	.byte	0x1
	.byte	0x80
	.uleb128 0x12
	.long	LC153
	.byte	0x1
	.byte	0x82
	.long	0x2437
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.long	LC143
	.byte	0x1
	.byte	0x83
	.long	0xc21
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x23ff
	.uleb128 0x9
	.byte	0x4
	.long	0x22ef
	.uleb128 0xa
	.long	0x251f
	.byte	0x84
	.byte	0x1
	.byte	0xe1
	.uleb128 0x12
	.long	LC154
	.byte	0x1
	.byte	0xd3
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.long	LC155
	.byte	0x1
	.byte	0xd4
	.long	0xd9a
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.long	LC156
	.byte	0x1
	.byte	0xd5
	.long	0xa12
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x12
	.long	LC157
	.byte	0x1
	.byte	0xd6
	.long	0xa12
	.byte	0x2
	.byte	0x23
	.uleb128 0x1e
	.uleb128 0x12
	.long	LC158
	.byte	0x1
	.byte	0xd7
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0x12
	.long	LC159
	.byte	0x1
	.byte	0xd8
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x2c
	.uleb128 0x6
	.ascii "length\0"
	.byte	0x1
	.byte	0xd9
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x34
	.uleb128 0x6
	.ascii "alloc\0"
	.byte	0x1
	.byte	0xda
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x3c
	.uleb128 0x6
	.ascii "cdate\0"
	.byte	0x1
	.byte	0xdb
	.long	0xae8
	.byte	0x2
	.byte	0x23
	.uleb128 0x44
	.uleb128 0x6
	.ascii "mdate\0"
	.byte	0x1
	.byte	0xdc
	.long	0xae8
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x6
	.ascii "adate\0"
	.byte	0x1
	.byte	0xdd
	.long	0xae8
	.byte	0x2
	.byte	0x23
	.uleb128 0x5c
	.uleb128 0x6
	.ascii "prot\0"
	.byte	0x1
	.byte	0xde
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x68
	.uleb128 0x6
	.ascii "owner\0"
	.byte	0x1
	.byte	0xdf
	.long	0xd9a
	.byte	0x2
	.byte	0x23
	.uleb128 0x70
	.uleb128 0x6
	.ascii "end\0"
	.byte	0x1
	.byte	0xe0
	.long	0x3a8
	.byte	0x3
	.byte	0x23
	.uleb128 0x80
	.byte	0x0
	.uleb128 0x1a
	.long	0x256b
	.byte	0x1c
	.byte	0x1
	.word	0x1a0
	.uleb128 0xe
	.ascii "modev\0"
	.byte	0x1
	.word	0x19c
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xe
	.ascii "modec\0"
	.byte	0x1
	.word	0x19d
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xe
	.ascii "modes\0"
	.byte	0x1
	.word	0x19e
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0xe
	.ascii "end\0"
	.byte	0x1
	.word	0x19f
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.byte	0x0
	.uleb128 0x1a
	.long	0x25a6
	.byte	0x14
	.byte	0x1
	.word	0x1aa
	.uleb128 0xe
	.ascii "modec\0"
	.byte	0x1
	.word	0x1a7
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xe
	.ascii "modes\0"
	.byte	0x1
	.word	0x1a8
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xe
	.ascii "end\0"
	.byte	0x1
	.word	0x1a9
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0xd
	.long	0x25fa
	.ascii "diparm\0"
	.byte	0x1c
	.byte	0x1
	.word	0x1b0
	.uleb128 0xe
	.ascii "cblksz\0"
	.byte	0x1
	.word	0x1b0
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xe
	.ascii "clssz\0"
	.byte	0x1
	.word	0x1b1
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xe
	.ascii "avail\0"
	.byte	0x1
	.word	0x1b2
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0xe
	.ascii "end\0"
	.byte	0x1
	.word	0x1b3
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.byte	0x0
	.uleb128 0x1a
	.long	0x2642
	.byte	0x3c
	.byte	0x1
	.word	0x1bf
	.uleb128 0xe
	.ascii "type\0"
	.byte	0x1
	.word	0x1bb
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xe
	.ascii "date\0"
	.byte	0x1
	.word	0x1bc
	.long	0x2da
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xe
	.ascii "xxx\0"
	.byte	0x1
	.word	0x1bd
	.long	0xc94
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x11
	.long	LC143
	.byte	0x1
	.word	0x1be
	.long	0x2642
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.byte	0x0
	.uleb128 0x7
	.long	0x2652
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0x23
	.byte	0x0
	.uleb128 0x1b
	.long	0x26e3
	.byte	0x1
	.ascii "main\0"
	.byte	0x1
	.word	0x1d8
	.byte	0x1
	.long	0x160
	.long	LFB3
	.long	LFE3
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "argc\0"
	.byte	0x1
	.word	0x1d5
	.long	0x160
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x1c
	.ascii "argv\0"
	.byte	0x1
	.word	0x1d6
	.long	0x26e3
	.byte	0x2
	.byte	0x91
	.sleb128 12
	.uleb128 0x1d
	.ascii "pnt\0"
	.byte	0x1
	.word	0x1d9
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.uleb128 0x1d
	.ascii "envpnt\0"
	.byte	0x1
	.word	0x1da
	.long	0x26e9
	.byte	0x2
	.byte	0x91
	.sleb128 -12
	.uleb128 0x1d
	.ascii "temp\0"
	.byte	0x1
	.word	0x1db
	.long	0x23e9
	.byte	0x2
	.byte	0x91
	.sleb128 -16
	.uleb128 0x1d
	.ascii "rtn\0"
	.byte	0x1
	.word	0x1dc
	.long	0x57
	.byte	0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0x1d
	.ascii "strbuf\0"
	.byte	0x1
	.word	0x1dd
	.long	0x26f9
	.byte	0x3
	.byte	0x91
	.sleb128 -276
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x83f
	.uleb128 0x7
	.long	0x26f9
	.long	0x83f
	.uleb128 0x8
	.long	0x3a1
	.byte	0x1
	.byte	0x0
	.uleb128 0x7
	.long	0x2709
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0xff
	.byte	0x0
	.uleb128 0x1e
	.byte	0x1
	.ascii "init_Vars\0"
	.byte	0x1
	.word	0x279
	.byte	0x1
	.long	LFB5
	.long	LFE5
	.byte	0x1
	.byte	0x55
	.uleb128 0x1b
	.long	0x2753
	.byte	0x1
	.ascii "allhave\0"
	.byte	0x1
	.word	0x291
	.byte	0x1
	.long	0x57
	.long	LFB7
	.long	LFE7
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "arg\0"
	.byte	0x1
	.word	0x28f
	.long	0x1911
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.byte	0x0
	.uleb128 0x1b
	.long	0x27cd
	.byte	0x1
	.ascii "procfile\0"
	.byte	0x1
	.word	0x2a0
	.byte	0x1
	.long	0x160
	.long	LFB9
	.long	LFE9
	.byte	0x1
	.byte	0x55
	.uleb128 0x1d
	.ascii "pnt\0"
	.byte	0x1
	.word	0x2a1
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.uleb128 0x1d
	.ascii "spec\0"
	.byte	0x1
	.word	0x2a2
	.long	0x23e9
	.byte	0x2
	.byte	0x91
	.sleb128 -8
	.uleb128 0x1f
	.long	LC143
	.byte	0x1
	.word	0x2a3
	.long	0x2437
	.byte	0x2
	.byte	0x91
	.sleb128 -12
	.uleb128 0x1d
	.ascii "fill\0"
	.byte	0x1
	.word	0x2a4
	.long	0x160
	.byte	0x2
	.byte	0x91
	.sleb128 -16
	.uleb128 0x20
	.long	LBB4
	.long	LBE4
	.uleb128 0x1f
	.long	LC140
	.byte	0x1
	.word	0x2c8
	.long	0x26f9
	.byte	0x3
	.byte	0x91
	.sleb128 -272
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.long	0x281d
	.byte	0x1
	.ascii "printheading\0"
	.byte	0x1
	.word	0x326
	.byte	0x1
	.long	LFB11
	.long	LFE11
	.byte	0x1
	.byte	0x55
	.uleb128 0x1f
	.long	LC161
	.byte	0x1
	.word	0x327
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.uleb128 0x1d
	.ascii "pnt\0"
	.byte	0x1
	.word	0x328
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -8
	.uleb128 0x1f
	.long	LC162
	.byte	0x1
	.word	0x329
	.long	0x281d
	.byte	0x3
	.byte	0x91
	.sleb128 -216
	.byte	0x0
	.uleb128 0x7
	.long	0x282d
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0xc7
	.byte	0x0
	.uleb128 0x21
	.long	0x28e7
	.byte	0x1
	.ascii "printfiles\0"
	.byte	0x1
	.word	0x360
	.byte	0x1
	.long	LFB13
	.long	LFE13
	.byte	0x1
	.byte	0x55
	.uleb128 0x1d
	.ascii "file\0"
	.byte	0x1
	.word	0x361
	.long	0x243d
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.uleb128 0x1f
	.long	LC153
	.byte	0x1
	.word	0x362
	.long	0x243d
	.byte	0x2
	.byte	0x91
	.sleb128 -8
	.uleb128 0x1d
	.ascii "pnt\0"
	.byte	0x1
	.word	0x363
	.long	0x28e7
	.byte	0x2
	.byte	0x91
	.sleb128 -12
	.uleb128 0x1f
	.long	LC161
	.byte	0x1
	.word	0x364
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -16
	.uleb128 0x1d
	.ascii "cpnt\0"
	.byte	0x1
	.word	0x365
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0x1d
	.ascii "cnt\0"
	.byte	0x1
	.word	0x366
	.long	0x160
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x1f
	.long	LC137
	.byte	0x1
	.word	0x367
	.long	0x160
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x1d
	.ascii "temp\0"
	.byte	0x1
	.word	0x368
	.long	0x160
	.byte	0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x1d
	.ascii "chr\0"
	.byte	0x1
	.word	0x369
	.long	0x3a8
	.byte	0x2
	.byte	0x91
	.sleb128 -33
	.uleb128 0x1f
	.long	LC162
	.byte	0x1
	.word	0x36a
	.long	0x281d
	.byte	0x3
	.byte	0x91
	.sleb128 -244
	.byte	0x0
	.uleb128 0x9
	.byte	0x4
	.long	0x243d
	.uleb128 0x1b
	.long	0x294d
	.byte	0x1
	.ascii "wideentry\0"
	.byte	0x1
	.word	0x451
	.byte	0x1
	.long	0x83f
	.long	LFB15
	.long	LFE15
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "file\0"
	.byte	0x1
	.word	0x44d
	.long	0x243d
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x22
	.long	LC162
	.byte	0x1
	.word	0x44e
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 12
	.uleb128 0x22
	.long	LC161
	.byte	0x1
	.word	0x44f
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 16
	.uleb128 0x1f
	.long	LC137
	.byte	0x1
	.word	0x452
	.long	0x160
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.byte	0x0
	.uleb128 0x1b
	.long	0x29e8
	.byte	0x1
	.ascii "comp\0"
	.byte	0x1
	.word	0x47a
	.byte	0x1
	.long	0x160
	.long	LFB17
	.long	LFE17
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "one\0"
	.byte	0x1
	.word	0x477
	.long	0x243d
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x1c
	.ascii "two\0"
	.byte	0x1
	.word	0x478
	.long	0x243d
	.byte	0x2
	.byte	0x91
	.sleb128 12
	.uleb128 0x1d
	.ascii "aext\0"
	.byte	0x1
	.word	0x47b
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.uleb128 0x1d
	.ascii "bext\0"
	.byte	0x1
	.word	0x47c
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -8
	.uleb128 0x1d
	.ascii "retval\0"
	.byte	0x1
	.word	0x47d
	.long	0x160
	.byte	0x2
	.byte	0x91
	.sleb128 -12
	.uleb128 0x1d
	.ascii "onetype\0"
	.byte	0x1
	.word	0x47e
	.long	0x3a8
	.byte	0x2
	.byte	0x91
	.sleb128 -13
	.uleb128 0x20
	.long	LBB9
	.long	LBE9
	.uleb128 0x1d
	.ascii "temp\0"
	.byte	0x1
	.word	0x488
	.long	0x243d
	.byte	0x2
	.byte	0x91
	.sleb128 -20
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.long	0x2a27
	.byte	0x1
	.ascii "pagecheck\0"
	.byte	0x1
	.word	0x4b2
	.byte	0x1
	.long	LFB19
	.long	LFE19
	.byte	0x1
	.byte	0x55
	.uleb128 0x1d
	.ascii "temp\0"
	.byte	0x1
	.word	0x4b3
	.long	0x57
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.uleb128 0x1d
	.ascii "more\0"
	.byte	0x1
	.word	0x4b4
	.long	0x160
	.byte	0x2
	.byte	0x91
	.sleb128 -8
	.byte	0x0
	.uleb128 0x1b
	.long	0x2a5a
	.byte	0x1
	.ascii "optdisplay\0"
	.byte	0x1
	.word	0x4f1
	.byte	0x1
	.long	0x57
	.long	LFB21
	.long	LFE21
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "arg\0"
	.byte	0x1
	.word	0x4ef
	.long	0x1911
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.byte	0x0
	.uleb128 0x1b
	.long	0x2a9a
	.byte	0x1
	.ascii "optafter\0"
	.byte	0x1
	.word	0x548
	.byte	0x1
	.long	0x57
	.long	LFB23
	.long	LFE23
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "arg\0"
	.byte	0x1
	.word	0x546
	.long	0x1911
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x1f
	.long	LC163
	.byte	0x1
	.word	0x549
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.byte	0x0
	.uleb128 0x23
	.byte	0x1
	.ascii "optall\0"
	.byte	0x1
	.word	0x557
	.byte	0x1
	.long	0x57
	.long	LFB25
	.long	LFE25
	.byte	0x1
	.byte	0x55
	.uleb128 0x1b
	.long	0x2af6
	.byte	0x1
	.ascii "optbefore\0"
	.byte	0x1
	.word	0x564
	.byte	0x1
	.long	0x57
	.long	LFB27
	.long	LFE27
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "arg\0"
	.byte	0x1
	.word	0x562
	.long	0x1911
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x1f
	.long	LC163
	.byte	0x1
	.word	0x565
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.byte	0x0
	.uleb128 0x23
	.byte	0x1
	.ascii "optdir\0"
	.byte	0x1
	.word	0x573
	.byte	0x1
	.long	0x57
	.long	LFB29
	.long	LFE29
	.byte	0x1
	.byte	0x55
	.uleb128 0x23
	.byte	0x1
	.ascii "optnodir\0"
	.byte	0x1
	.word	0x57f
	.byte	0x1
	.long	0x57
	.long	LFB31
	.long	LFE31
	.byte	0x1
	.byte	0x55
	.uleb128 0x23
	.byte	0x1
	.ascii "optone\0"
	.byte	0x1
	.word	0x58b
	.byte	0x1
	.long	0x57
	.long	LFB33
	.long	LFE33
	.byte	0x1
	.byte	0x55
	.uleb128 0x23
	.byte	0x1
	.ascii "optfull\0"
	.byte	0x1
	.word	0x598
	.byte	0x1
	.long	0x57
	.long	LFB35
	.long	LFE35
	.byte	0x1
	.byte	0x55
	.uleb128 0x23
	.byte	0x1
	.ascii "optshort\0"
	.byte	0x1
	.word	0x5ad
	.byte	0x1
	.long	0x57
	.long	LFB37
	.long	LFE37
	.byte	0x1
	.byte	0x55
	.uleb128 0x23
	.byte	0x1
	.ascii "optlong\0"
	.byte	0x1
	.word	0x5c2
	.byte	0x1
	.long	0x57
	.long	LFB39
	.long	LFE39
	.byte	0x1
	.byte	0x55
	.uleb128 0x1b
	.long	0x2bce
	.byte	0x1
	.ascii "optsort\0"
	.byte	0x1
	.word	0x5d8
	.byte	0x1
	.long	0x57
	.long	LFB41
	.long	LFE41
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "arg\0"
	.byte	0x1
	.word	0x5d6
	.long	0x1911
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.byte	0x0
	.uleb128 0x1b
	.long	0x2c0a
	.byte	0x1
	.ascii "optx\0"
	.byte	0x1
	.word	0x62b
	.byte	0x1
	.long	0x57
	.long	LFB43
	.long	LFE43
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "arg\0"
	.byte	0x1
	.word	0x629
	.long	0x1911
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x1f
	.long	LC143
	.byte	0x1
	.word	0x62c
	.long	0x2437
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.byte	0x0
	.uleb128 0x1b
	.long	0x2ce6
	.byte	0x1
	.ascii "nonopt\0"
	.byte	0x1
	.word	0x63d
	.byte	0x1
	.long	0x160
	.long	LFB45
	.long	LFE45
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "arg\0"
	.byte	0x1
	.word	0x63b
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x1d
	.ascii "spec\0"
	.byte	0x1
	.word	0x63e
	.long	0x23e9
	.byte	0x2
	.byte	0x91
	.sleb128 -8
	.uleb128 0x1d
	.ascii "spnt\0"
	.byte	0x1
	.word	0x63f
	.long	0x23e9
	.byte	0x2
	.byte	0x91
	.sleb128 -12
	.uleb128 0x1d
	.ascii "pnt\0"
	.byte	0x1
	.word	0x640
	.long	0x83f
	.byte	0x2
	.byte	0x91
	.sleb128 -16
	.uleb128 0x1d
	.ascii "rtn\0"
	.byte	0x1
	.word	0x641
	.long	0x57
	.byte	0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0x1f
	.long	LC137
	.byte	0x1
	.word	0x642
	.long	0x160
	.byte	0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x1d
	.ascii "chr\0"
	.byte	0x1
	.word	0x643
	.long	0x3a8
	.byte	0x2
	.byte	0x91
	.sleb128 -25
	.uleb128 0x1a
	.long	0x2ccd
	.byte	0x20
	.byte	0x1
	.word	0x649
	.uleb128 0xe
	.ascii "glbid\0"
	.byte	0x1
	.word	0x646
	.long	0xd08
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x11
	.long	LC159
	.byte	0x1
	.word	0x647
	.long	0xa75
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0xe
	.ascii "end\0"
	.byte	0x1
	.word	0x648
	.long	0x3a8
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.byte	0x0
	.uleb128 0x1d
	.ascii "glbidparm\0"
	.byte	0x1
	.word	0x649
	.long	0x2c94
	.byte	0x5
	.byte	0x3
	.long	_glbidparm.0
	.byte	0x0
	.uleb128 0x1b
	.long	0x2d24
	.byte	0x1
	.ascii "getmem\0"
	.byte	0x1
	.word	0x69d
	.byte	0x1
	.long	0x845
	.long	LFB47
	.long	LFE47
	.byte	0x1
	.byte	0x55
	.uleb128 0x22
	.long	LC137
	.byte	0x1
	.word	0x69b
	.long	0x197
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x1d
	.ascii "ptr\0"
	.byte	0x1
	.word	0x69e
	.long	0x845
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.byte	0x0
	.uleb128 0x24
	.long	0x2d6c
	.byte	0x1
	.long	LC160
	.byte	0x1
	.word	0x6b4
	.byte	0x1
	.long	LFB49
	.long	LFE49
	.byte	0x1
	.byte	0x55
	.uleb128 0x1c
	.ascii "arg\0"
	.byte	0x1
	.word	0x6b1
	.long	0x22cb
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x1c
	.ascii "code\0"
	.byte	0x1
	.word	0x6b2
	.long	0x57
	.byte	0x2
	.byte	0x91
	.sleb128 12
	.uleb128 0x1f
	.long	LC140
	.byte	0x1
	.word	0x6b5
	.long	0x2d6c
	.byte	0x3
	.byte	0x91
	.sleb128 -80
	.byte	0x0
	.uleb128 0x7
	.long	0x2d7c
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0x4f
	.byte	0x0
	.uleb128 0x25
	.ascii "_stdout\0"
	.byte	0x3
	.byte	0x59
	.long	0x291
	.byte	0x1
	.byte	0x1
	.uleb128 0x25
	.ascii "_stderr\0"
	.byte	0x3
	.byte	0x5a
	.long	0x291
	.byte	0x1
	.byte	0x1
	.uleb128 0x25
	.ascii "gcp_dosdrive\0"
	.byte	0xc
	.byte	0x2a
	.long	0x160
	.byte	0x1
	.byte	0x1
	.uleb128 0x26
	.ascii "firstspec\0"
	.byte	0x1
	.byte	0x7d
	.long	0x23e9
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_firstspec
	.uleb128 0x26
	.ascii "thisspec\0"
	.byte	0x1
	.byte	0x7e
	.long	0x23e9
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_thisspec
	.uleb128 0x26
	.ascii "firstname\0"
	.byte	0x1
	.byte	0x86
	.long	0x2437
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_firstname
	.uleb128 0x26
	.ascii "firstfile\0"
	.byte	0x1
	.byte	0xa8
	.long	0x243d
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_firstfile
	.uleb128 0x26
	.ascii "lastfile\0"
	.byte	0x1
	.byte	0xa9
	.long	0x243d
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_lastfile
	.uleb128 0x26
	.ascii "thisfile\0"
	.byte	0x1
	.byte	0xaa
	.long	0x243d
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_thisfile
	.uleb128 0x26
	.ascii "owner\0"
	.byte	0x1
	.byte	0xd0
	.long	0x2642
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_owner
	.uleb128 0x26
	.ascii "fileparm\0"
	.byte	0x1
	.byte	0xe1
	.long	0x2443
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_fileparm
	.uleb128 0x26
	.ascii "dsd\0"
	.byte	0x1
	.byte	0xf2
	.long	0x22dc
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_dsd
	.uleb128 0x7
	.long	0x2e8e
	.long	0x189d
	.uleb128 0x8
	.long	0x3a1
	.byte	0x10
	.byte	0x0
	.uleb128 0x27
	.ascii "discmd\0"
	.byte	0x1
	.word	0x101
	.long	0x2e7e
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_discmd
	.uleb128 0x7
	.long	0x2eb4
	.long	0x189d
	.uleb128 0x8
	.long	0x3a1
	.byte	0x9
	.byte	0x0
	.uleb128 0x27
	.ascii "sortcmd\0"
	.byte	0x1
	.word	0x115
	.long	0x2ea4
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_sortcmd
	.uleb128 0x27
	.ascii "shownames\0"
	.byte	0x1
	.word	0x124
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_shownames
	.uleb128 0x27
	.ascii "debug\0"
	.byte	0x1
	.word	0x125
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_debug
	.uleb128 0x27
	.ascii "page\0"
	.byte	0x1
	.word	0x126
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_page
	.uleb128 0x27
	.ascii "lsum\0"
	.byte	0x1
	.word	0x127
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_lsum
	.uleb128 0x27
	.ascii "onlytotals\0"
	.byte	0x1
	.word	0x128
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_onlytotals
	.uleb128 0x7
	.long	0x2f4b
	.long	0x19c3
	.uleb128 0x8
	.long	0x3a1
	.byte	0x11
	.byte	0x0
	.uleb128 0x27
	.ascii "options\0"
	.byte	0x1
	.word	0x12a
	.long	0x2f3b
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_options
	.uleb128 0x27
	.ascii "longfmt\0"
	.byte	0x1
	.word	0x15c
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_longfmt
	.uleb128 0x27
	.ascii "fullfmt\0"
	.byte	0x1
	.word	0x15d
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_fullfmt
	.uleb128 0x27
	.ascii "prtvolid\0"
	.byte	0x1
	.word	0x15e
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_prtvolid
	.uleb128 0x27
	.ascii "prtpath\0"
	.byte	0x1
	.word	0x15f
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_prtpath
	.uleb128 0x27
	.ascii "showattr\0"
	.byte	0x1
	.word	0x160
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_showattr
	.uleb128 0x27
	.ascii "onecol\0"
	.byte	0x1
	.word	0x161
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_onecol
	.uleb128 0x27
	.ascii "datesort\0"
	.byte	0x1
	.word	0x162
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_datesort
	.uleb128 0x27
	.ascii "extsort\0"
	.byte	0x1
	.word	0x163
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_extsort
	.uleb128 0x27
	.ascii "revsort\0"
	.byte	0x1
	.word	0x164
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_revsort
	.uleb128 0x27
	.ascii "dirsort\0"
	.byte	0x1
	.word	0x165
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_dirsort
	.uleb128 0x27
	.ascii "sizesort\0"
	.byte	0x1
	.word	0x166
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_sizesort
	.uleb128 0x27
	.ascii "nosort\0"
	.byte	0x1
	.word	0x167
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_nosort
	.uleb128 0x27
	.ascii "prtacross\0"
	.byte	0x1
	.word	0x168
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_prtacross
	.uleb128 0x27
	.ascii "prtexpand\0"
	.byte	0x1
	.word	0x169
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_prtexpand
	.uleb128 0x27
	.ascii "prtnamedos\0"
	.byte	0x1
	.word	0x16a
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_prtnamedos
	.uleb128 0x27
	.ascii "prtnamelong\0"
	.byte	0x1
	.word	0x16b
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_prtnamelong
	.uleb128 0x27
	.ascii "changed\0"
	.byte	0x1
	.word	0x16c
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_changed
	.uleb128 0x27
	.ascii "needed\0"
	.byte	0x1
	.word	0x16d
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_needed
	.uleb128 0x27
	.ascii "namesize\0"
	.byte	0x1
	.word	0x16e
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_namesize
	.uleb128 0x27
	.ascii "needlength\0"
	.byte	0x1
	.word	0x16f
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_needlength
	.uleb128 0x27
	.ascii "needalloc\0"
	.byte	0x1
	.word	0x170
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_needalloc
	.uleb128 0x27
	.ascii "needcdate\0"
	.byte	0x1
	.word	0x171
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_needcdate
	.uleb128 0x27
	.ascii "needmdate\0"
	.byte	0x1
	.word	0x172
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_needmdate
	.uleb128 0x27
	.ascii "needadate\0"
	.byte	0x1
	.word	0x173
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_needadate
	.uleb128 0x27
	.ascii "needowner\0"
	.byte	0x1
	.word	0x174
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_needowner
	.uleb128 0x27
	.ascii "needprot\0"
	.byte	0x1
	.word	0x175
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_needprot
	.uleb128 0x27
	.ascii "neednamedos\0"
	.byte	0x1
	.word	0x176
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_neednamedos
	.uleb128 0x27
	.ascii "needname\0"
	.byte	0x1
	.word	0x177
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_needname
	.uleb128 0x27
	.ascii "curline\0"
	.byte	0x1
	.word	0x178
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_curline
	.uleb128 0x27
	.ascii "after_time\0"
	.byte	0x1
	.word	0x179
	.long	0x2da
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_after_time
	.uleb128 0x27
	.ascii "before_time\0"
	.byte	0x1
	.word	0x17a
	.long	0x2da
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_before_time
	.uleb128 0x27
	.ascii "errdone\0"
	.byte	0x1
	.word	0x17b
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_errdone
	.uleb128 0x27
	.ascii "hvdefault\0"
	.byte	0x1
	.word	0x17c
	.long	0x3a8
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_hvdefault
	.uleb128 0x27
	.ascii "numlisted\0"
	.byte	0x1
	.word	0x17d
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_numlisted
	.uleb128 0x27
	.ascii "numfiles\0"
	.byte	0x1
	.word	0x17e
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_numfiles
	.uleb128 0x27
	.ascii "filecnt\0"
	.byte	0x1
	.word	0x17f
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_filecnt
	.uleb128 0x27
	.ascii "dircnt\0"
	.byte	0x1
	.word	0x180
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_dircnt
	.uleb128 0x27
	.ascii "totallen\0"
	.byte	0x1
	.word	0x181
	.long	0x472
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_totallen
	.uleb128 0x27
	.ascii "totalalloc\0"
	.byte	0x1
	.word	0x182
	.long	0x472
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_totalalloc
	.uleb128 0x27
	.ascii "gfilecnt\0"
	.byte	0x1
	.word	0x183
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_gfilecnt
	.uleb128 0x27
	.ascii "gdircnt\0"
	.byte	0x1
	.word	0x184
	.long	0x57
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_gdircnt
	.uleb128 0x27
	.ascii "gtotallen\0"
	.byte	0x1
	.word	0x185
	.long	0x472
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_gtotallen
	.uleb128 0x27
	.ascii "gtotalalloc\0"
	.byte	0x1
	.word	0x186
	.long	0x472
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_gtotalalloc
	.uleb128 0x27
	.ascii "fspnt\0"
	.byte	0x1
	.word	0x187
	.long	0x83f
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_fspnt
	.uleb128 0x27
	.ascii "pib\0"
	.byte	0x1
	.word	0x18b
	.long	0x1c9f
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_pib
	.uleb128 0x7
	.long	0x33ad
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0x0
	.byte	0x0
	.uleb128 0x27
	.ascii "copymsg\0"
	.byte	0x1
	.word	0x18c
	.long	0x339d
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_copymsg
	.uleb128 0x7
	.long	0x33d4
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0x3
	.byte	0x0
	.uleb128 0x27
	.ascii "prgname\0"
	.byte	0x1
	.word	0x18d
	.long	0x33c4
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_prgname
	.uleb128 0x7
	.long	0x33fb
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0xc
	.byte	0x0
	.uleb128 0x27
	.ascii "envname\0"
	.byte	0x1
	.word	0x18e
	.long	0x33eb
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_envname
	.uleb128 0x7
	.long	0x3422
	.long	0x3a8
	.uleb128 0x8
	.long	0x3a1
	.byte	0x13
	.byte	0x0
	.uleb128 0x27
	.ascii "example\0"
	.byte	0x1
	.word	0x18f
	.long	0x3412
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_example
	.uleb128 0x7
	.long	0x344a
	.long	0x3a8
	.uleb128 0x28
	.long	0x3a1
	.word	0x24c
	.byte	0x0
	.uleb128 0x27
	.ascii "description\0"
	.byte	0x1
	.word	0x190
	.long	0x3439
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_description
	.uleb128 0x27
	.ascii "snglparm\0"
	.byte	0x1
	.word	0x1a0
	.long	0x251f
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_snglparm
	.uleb128 0x27
	.ascii "normparm\0"
	.byte	0x1
	.word	0x1aa
	.long	0x256b
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_normparm
	.uleb128 0x27
	.ascii "diparm\0"
	.byte	0x1
	.word	0x1b4
	.long	0x25a6
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_diparm
	.uleb128 0x27
	.ascii "vollabel\0"
	.byte	0x1
	.word	0x1bf
	.long	0x25fa
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_vollabel
	.uleb128 0x27
	.ascii "lblqab\0"
	.byte	0x1
	.word	0x1c1
	.long	0x847
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_lblqab
	.uleb128 0x29
	.ascii "_malloc_amount\0"
	.byte	0x1
	.word	0x1d1
	.long	0x148
	.byte	0x1
	.byte	0x1
	.uleb128 0x27
	.ascii "maxmem\0"
	.byte	0x1
	.word	0x1d2
	.long	0x148
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_maxmem
	.uleb128 0x27
	.ascii "colwidth\0"
	.byte	0x1
	.word	0x357
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_colwidth
	.uleb128 0x27
	.ascii "collength\0"
	.byte	0x1
	.word	0x358
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_collength
	.uleb128 0x27
	.ascii "fill\0"
	.byte	0x1
	.word	0x359
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_fill
	.uleb128 0x27
	.ascii "left\0"
	.byte	0x1
	.word	0x35a
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_left
	.uleb128 0x27
	.ascii "numcolumns\0"
	.byte	0x1
	.word	0x35b
	.long	0x160
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_numcolumns
	.uleb128 0x7
	.long	0x358b
	.long	0x243d
	.uleb128 0x8
	.long	0x3a1
	.byte	0x4
	.byte	0x0
	.uleb128 0x27
	.ascii "colarray\0"
	.byte	0x1
	.word	0x35c
	.long	0x357b
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	_colarray
	.byte	0x0
	.section	.debug_abbrev,""
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x10
	.uleb128 0x6
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x1b
	.uleb128 0x8
	.uleb128 0x25
	.uleb128 0x8
	.uleb128 0x13
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x2
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x5
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x6
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x7
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x8
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x9
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xa
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0xb
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xc
	.uleb128 0x18
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.uleb128 0xd
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.byte	0x0
	.byte	0x0
	.uleb128 0xe
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0xf
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x10
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x11
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x12
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x13
	.uleb128 0x17
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x14
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x15
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x16
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x17
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0x5
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x18
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x27
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.uleb128 0x19
	.uleb128 0x26
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1a
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.byte	0x0
	.byte	0x0
	.uleb128 0x1b
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x1c
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x1d
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x1e
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x1f
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x20
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x22
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x23
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x24
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x25
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.uleb128 0x26
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x27
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x28
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0x5
	.byte	0x0
	.byte	0x0
	.uleb128 0x29
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.section	.debug_pubnames,""
	.long	0x521
	.word	0x2
	.long	Ldebug_info0
	.long	0x35a4
	.long	0x2652
	.ascii "main\0"
	.long	0x2709
	.ascii "init_Vars\0"
	.long	0x2723
	.ascii "allhave\0"
	.long	0x2753
	.ascii "procfile\0"
	.long	0x27cd
	.ascii "printheading\0"
	.long	0x282d
	.ascii "printfiles\0"
	.long	0x28ed
	.ascii "wideentry\0"
	.long	0x294d
	.ascii "comp\0"
	.long	0x29e8
	.ascii "pagecheck\0"
	.long	0x2a27
	.ascii "optdisplay\0"
	.long	0x2a5a
	.ascii "optafter\0"
	.long	0x2a9a
	.ascii "optall\0"
	.long	0x2ab5
	.ascii "optbefore\0"
	.long	0x2af6
	.ascii "optdir\0"
	.long	0x2b11
	.ascii "optnodir\0"
	.long	0x2b2e
	.ascii "optone\0"
	.long	0x2b49
	.ascii "optfull\0"
	.long	0x2b65
	.ascii "optshort\0"
	.long	0x2b82
	.ascii "optlong\0"
	.long	0x2b9e
	.ascii "optsort\0"
	.long	0x2bce
	.ascii "optx\0"
	.long	0x2c0a
	.ascii "nonopt\0"
	.long	0x2ce6
	.ascii "getmem\0"
	.long	0x2d24
	.ascii "errormsg\0"
	.long	0x2db4
	.ascii "firstspec\0"
	.long	0x2dcc
	.ascii "thisspec\0"
	.long	0x2de3
	.ascii "firstname\0"
	.long	0x2dfb
	.ascii "firstfile\0"
	.long	0x2e13
	.ascii "lastfile\0"
	.long	0x2e2a
	.ascii "thisfile\0"
	.long	0x2e41
	.ascii "owner\0"
	.long	0x2e55
	.ascii "fileparm\0"
	.long	0x2e6c
	.ascii "dsd\0"
	.long	0x2e8e
	.ascii "discmd\0"
	.long	0x2eb4
	.ascii "sortcmd\0"
	.long	0x2ecb
	.ascii "shownames\0"
	.long	0x2ee4
	.ascii "debug\0"
	.long	0x2ef9
	.ascii "page\0"
	.long	0x2f0d
	.ascii "lsum\0"
	.long	0x2f21
	.ascii "onlytotals\0"
	.long	0x2f4b
	.ascii "options\0"
	.long	0x2f62
	.ascii "longfmt\0"
	.long	0x2f79
	.ascii "fullfmt\0"
	.long	0x2f90
	.ascii "prtvolid\0"
	.long	0x2fa8
	.ascii "prtpath\0"
	.long	0x2fbf
	.ascii "showattr\0"
	.long	0x2fd7
	.ascii "onecol\0"
	.long	0x2fed
	.ascii "datesort\0"
	.long	0x3005
	.ascii "extsort\0"
	.long	0x301c
	.ascii "revsort\0"
	.long	0x3033
	.ascii "dirsort\0"
	.long	0x304a
	.ascii "sizesort\0"
	.long	0x3062
	.ascii "nosort\0"
	.long	0x3078
	.ascii "prtacross\0"
	.long	0x3091
	.ascii "prtexpand\0"
	.long	0x30aa
	.ascii "prtnamedos\0"
	.long	0x30c4
	.ascii "prtnamelong\0"
	.long	0x30df
	.ascii "changed\0"
	.long	0x30f6
	.ascii "needed\0"
	.long	0x310c
	.ascii "namesize\0"
	.long	0x3124
	.ascii "needlength\0"
	.long	0x313e
	.ascii "needalloc\0"
	.long	0x3157
	.ascii "needcdate\0"
	.long	0x3170
	.ascii "needmdate\0"
	.long	0x3189
	.ascii "needadate\0"
	.long	0x31a2
	.ascii "needowner\0"
	.long	0x31bb
	.ascii "needprot\0"
	.long	0x31d3
	.ascii "neednamedos\0"
	.long	0x31ee
	.ascii "needname\0"
	.long	0x3206
	.ascii "curline\0"
	.long	0x321d
	.ascii "after_time\0"
	.long	0x3237
	.ascii "before_time\0"
	.long	0x3252
	.ascii "errdone\0"
	.long	0x3269
	.ascii "hvdefault\0"
	.long	0x3282
	.ascii "numlisted\0"
	.long	0x329b
	.ascii "numfiles\0"
	.long	0x32b3
	.ascii "filecnt\0"
	.long	0x32ca
	.ascii "dircnt\0"
	.long	0x32e0
	.ascii "totallen\0"
	.long	0x32f8
	.ascii "totalalloc\0"
	.long	0x3312
	.ascii "gfilecnt\0"
	.long	0x332a
	.ascii "gdircnt\0"
	.long	0x3341
	.ascii "gtotallen\0"
	.long	0x335a
	.ascii "gtotalalloc\0"
	.long	0x3375
	.ascii "fspnt\0"
	.long	0x338a
	.ascii "pib\0"
	.long	0x33ad
	.ascii "copymsg\0"
	.long	0x33d4
	.ascii "prgname\0"
	.long	0x33fb
	.ascii "envname\0"
	.long	0x3422
	.ascii "example\0"
	.long	0x344a
	.ascii "description\0"
	.long	0x3465
	.ascii "snglparm\0"
	.long	0x347d
	.ascii "normparm\0"
	.long	0x3495
	.ascii "diparm\0"
	.long	0x34ab
	.ascii "vollabel\0"
	.long	0x34c3
	.ascii "lblqab\0"
	.long	0x34f2
	.ascii "maxmem\0"
	.long	0x3508
	.ascii "colwidth\0"
	.long	0x3520
	.ascii "collength\0"
	.long	0x3539
	.ascii "fill\0"
	.long	0x354d
	.ascii "left\0"
	.long	0x3561
	.ascii "numcolumns\0"
	.long	0x358b
	.ascii "colarray\0"
	.long	0x0
	.section	.debug_aranges,""
	.long	0x1c
	.word	0x2
	.long	Ldebug_info0
	.byte	0x4
	.byte	0x0
	.word	0x0
	.word	0x0
	.long	Ltext0
	.long	Letext0-Ltext0
	.long	0x0
	.long	0x0
	.section	.debug_str,""
LC139:
	.ascii "value\0"
LC151:
	.ascii "tmx_wday\0"
LC158:
	.ascii "dirhndl\0"
LC160:
	.ascii "errormsg\0"
LC157:
	.ascii "filattr\0"
LC140:
	.ascii "buffer\0"
LC136:
	.ascii "desp\0"
LC163:
	.ascii "time_ptr\0"
LC135:
	.ascii "long unsigned int\0"
LC146:
	.ascii "infolen\0"
LC159:
	.ascii "devsts\0"
LC143:
	.ascii "name\0"
LC162:
	.ascii "linebufr\0"
LC138:
	.ascii "index\0"
LC152:
	.ascii "tmx_yday\0"
LC145:
	.ascii "infosz\0"
LC144:
	.ascii "infopnt\0"
LC156:
	.ascii "srcattr\0"
LC155:
	.ascii "filspec\0"
LC149:
	.ascii "tmx_mday\0"
LC142:
	.ascii "strlen\0"
LC154:
	.ascii "filoptn\0"
LC141:
	.ascii "bfrlen\0"
LC153:
	.ascii "next\0"
LC161:
	.ascii "linepnt\0"
LC150:
	.ascii "tmx_year\0"
LC148:
	.ascii "tmx_hour\0"
LC147:
	.ascii "tmx_msec\0"
LC137:
	.ascii "size\0"
	.ident	"GCC: (GNU) 3.3.2"
