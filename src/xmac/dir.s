	.file	"dir.c"
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
	.long	0
	.word	0
	.word	0
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
	.long	0
	.word	36
	.word	36
	.space 1
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
	.space 423
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
	.space 1
.globl _normparm
_normparm:
	.byte	-126
	.byte	4
	.word	514
	.long	-1
	.byte	-126
	.byte	4
	.word	513
	.long	0
	.space 1
.globl _diparm
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
	.space 1
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
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
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
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	subl	$264, %esp
	movl	12(%ebp), %esi
	pushl	$_pib
	call	_reg_pib
	call	_init_Vars
	pushl	$1
	call	_global_parameter
	movb	$1, _prtvolid
	movb	$1, _prtpath
	movb	$0, _nosort
	movl	$1, _lsum
	addl	$8, %esp
	leal	-264(%ebp), %edx
/APP
	pushl $0
	pushl %ds
	pushl $_envname
	pushl %ds
	pushl $0
	pushl %ds
	pushl %edx
	pushl $256
	pushl %ds
	pushl $0
	callf _svcSysFindEnv##
/NO_APP
	testl	%eax, %eax
	jle	L2
	movl	%edx, -272(%ebp)
	leal	-272(%ebp), %eax
	movl	$0, 4(%eax)
	pushl	$0
	pushl	$0
	pushl	$0
	pushl	$0
	pushl	$0
	pushl	$_options
	pushl	$0
	pushl	%eax
	call	_progarg
	addl	$32, %esp
L2:
	cmpl	$1, 8(%ebp)
	jle	L3
	addl	$4, %esi
	pushl	$0
	pushl	$0
	pushl	$0
	pushl	$_nonopt
	pushl	$0
	pushl	$_options
	pushl	$0
	pushl	%esi
	call	_progarg
	addl	$32, %esp
L3:
	cmpl	$0, _onlytotals
	je	L4
	movl	$1, _lsum
	movl	$0, _shownames
L4:
	cmpl	$0, _gcp_dosdrive
	je	L5
	movl	$4999232, _fileparm+4
L5:
	cmpb	$0, _prtnamedos
	je	L8
	orl	$256, _fileparm+4
	cmpb	$0, _prtnamedos
	je	L8
	cmpb	$0, _prtnamelong
	je	L7
L8:
	orl	$640, _fileparm+4
L7:
	cmpl	$0, _firstspec
	jne	L9
	pushl	$LC80
	call	_nonopt
	addl	$4, %esp
L9:
	cmpl	$0, _pib
	jne	L10
	movl	$0, _page
L10:
	cmpb	$0, _datesort
	je	L11
	cmpl	$0, _needcdate
	jns	L21
	cmpl	$0, _needalloc
	js	L13
	movl	$4, %eax
	jmp	L14
L13:
	movl	$0, %eax
L14:
	cmpl	$0, _needprot
	js	L15
	addl	$4, %eax
L15:
	cmpl	$0, _needowner
	js	L16
	addl	$36, %eax
L16:
	cmpl	$0, _needadate
	js	L17
	addl	$8, %eax
L17:
	cmpl	$0, _needmdate
	js	L18
	addl	$8, %eax
L18:
	movl	%eax, _needcdate
	cmpl	$0, _needlength
	js	L59
	addl	$8, _needlength
	jmp	L59
L11:
	cmpb	$0, _sizesort
	je	L21
	cmpl	$0, _needlength
	jns	L21
	cmpl	$0, _needalloc
	js	L24
	movl	$4, %eax
	jmp	L25
L24:
	movl	$0, %eax
L25:
	cmpl	$0, _needprot
	js	L26
	addl	$4, %eax
L26:
	cmpl	$0, _needowner
	js	L27
	addl	$36, %eax
L27:
	cmpl	$0, _needadate
	js	L28
	addl	$8, %eax
L28:
	cmpl	$0, _needmdate
	js	L29
	addl	$8, %eax
L29:
	cmpl	$0, _needcdate
	js	L30
	addl	$8, %eax
L30:
	movl	%eax, _needlength
L59:
	cmpl	$0, _neednamedos
	js	L31
	addl	$8, _neednamedos
L31:
	addl	$8, _needname
L21:
	cmpb	$0, _prtnamedos
	je	L32
	cmpb	$0, _prtnamelong
	je	L32
	movl	_needname, %eax
	movl	%eax, _neednamedos
	addl	$16, %eax
	movl	%eax, _needname
L32:
	cmpl	$0, _needname
	je	L34
	cmpl	$0, _neednamedos
	jne	L33
L34:
	movb	$1, _dsd+4
	movb	$0, _fileparm+44
L33:
	cmpb	$0, _longfmt
	jne	L35
	cmpb	$0, _fullfmt
	jne	L35
	cmpb	$0, _prtnamedos
	je	L35
	movb	$0, _prtnamelong
L35:
	movl	_firstspec, %eax
	movl	%eax, _thisspec
L36:
	movl	_thisspec, %edx
	cmpl	$0, 4(%edx)
	je	L39
	movl	32(%edx), %eax
	movl	%eax, %esi
	subl	%edx, %esi
	subl	$36, %esi
	movl	%eax, %edi
	cld
	movb	$0, %al
	movl	$-1, %ecx
	repne
	scasb
	notl	%ecx
	leal	3(%esi,%ecx), %ecx
	pushl	%ecx
	call	_getmem
	movl	%eax, %edi
	pushl	$LC81
	pushl	%esi
	movl	_thisspec, %eax
	addl	$36, %eax
	pushl	%eax
	pushl	%edi
	call	_strnmov
	addl	$12, %esp
	pushl	%eax
	call	_strmov
	pushl	$_dsd
	pushl	%edi
	call	_dirscan
	pushl	%edi
	call	_free
	addl	$24, %esp
	jmp	L40
L39:
	pushl	$_dsd
	movl	_thisspec, %eax
	addl	$36, %eax
	pushl	%eax
	call	_dirscan
	addl	$8, %esp
L40:
	cmpl	$0, _numfiles
	je	L41
	call	_printfiles
L41:
	movl	_thisspec, %eax
	movl	(%eax), %esi
	pushl	%eax
	call	_free
	addl	$4, %esp
	movl	%esi, _thisspec
	testl	%esi, %esi
	jne	L36
	cmpl	$0, _lsum
	je	L43
	cmpl	$1, _numlisted
	jle	L43
	call	_pagecheck
	pushl	$10
	call	_putchar
	call	_pagecheck
	cmpl	$1, _numlisted
	jne	L44
	movl	$LC82, %eax
	jmp	L45
L44:
	movl	$LC83, %eax
L45:
	pushl	%eax
	pushl	_numlisted
	pushl	$LC84
	call	_printf
	leal	-264(%ebp), %edi
	addl	$16, %esp
	cmpl	$0, _gdircnt
	je	L46
	cmpl	$1, _gdircnt
	jne	L47
	movl	$LC82, %eax
	jmp	L48
L47:
	movl	$LC83, %eax
L48:
	pushl	%eax
	pushl	_gdircnt
	pushl	$LC85
	pushl	%edi
	call	_sprintf
	addl	%eax, %edi
	addl	$16, %esp
L46:
	cmpl	$1, _gfilecnt
	jne	L49
	movl	$LC86, %eax
	jmp	L50
L49:
	movl	$LC87, %eax
L50:
	pushl	%eax
	pushl	_gfilecnt
	pushl	$LC88
	pushl	%edi
	call	_sprintf
	addl	%eax, %edi
	addl	$16, %esp
	cmpb	$0, _longfmt
	je	L51
	cmpl	$1, _gtotalalloc
	jne	L52
	cmpl	$0, _gtotalalloc+4
	jne	L52
	movl	$LC86, %eax
	jmp	L53
L52:
	movl	$LC87, %eax
L53:
	pushl	%eax
	pushl	_gtotalalloc+4
	pushl	_gtotalalloc
	cmpl	$1, _gtotallen
	jne	L54
	cmpl	$0, _gtotallen+4
	jne	L54
	movl	$LC86, %eax
	jmp	L55
L54:
	movl	$LC87, %eax
L55:
	pushl	%eax
	pushl	_gtotallen+4
	pushl	_gtotallen
	pushl	$LC89
	pushl	%edi
	call	_sprintf
	addl	%eax, %edi
	addl	$32, %esp
L51:
	call	_pagecheck
	pushl	$LC90
	pushl	%edi
	call	_strmov
	pushl	$__stdout
	leal	-264(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$16, %esp
L43:
	cmpb	$0, _errdone
	jne	L56
	movl	_gdircnt, %eax
	addl	_gfilecnt, %eax
	testl	%eax, %eax
	jne	L56
	pushl	$__stderr
	pushl	$22
	pushl	$1
	pushl	$LC91
	call	_fwrite
	addl	$16, %esp
L56:
	cmpl	$0, _debug
	je	L57
	call	_pagecheck
	pushl	$10
	call	_putchar
	pushl	__malloc_amount
	pushl	_maxmem
	pushl	$LC92
	call	_printf
	addl	$16, %esp
L57:
	pushl	$0
	call	_exit
LC93:
	.ascii "Dec 15 2003\0"
.globl _init_Vars
_init_Vars:
	pushl	%ebp
	movl	%esp, %ebp
	movl	$_options, _pib+144
	movl	$0, _pib+148
	movl	$LC93, _pib+132
	movl	$3, _pib+4
	movl	$8, _pib+8
	movl	$_copymsg, _pib+124
	movl	$_prgname, _pib+128
	movl	$_description, _pib+136
	movl	$_example, _pib+140
	movl	$0, _pib+12
	call	_getTrmParms
	call	_getHelpClr
	popl	%ebp
	ret
.globl _allhave
_allhave:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movw	8(%eax), %ax
	movw	%ax, _fileparm+28
	movl	$1, %eax
	popl	%ebp
	ret
.globl _procfile
_procfile:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	subl	$256, %esp
	movb	_changed, %al
	orb	_dsd+20, %al
	movb	%al, _changed
	movl	_thisspec, %eax
	cmpl	$0, 4(%eax)
	je	L63
	movl	%eax, %esi
L64:
	movl	24(%esi), %eax
	shrl	$19, %eax
	andl	$1, %eax
	pushl	%eax
	pushl	_dsd+152
	pushl	32(%esi)
	call	_wildcmp
	addl	$12, %esp
	testl	%eax, %eax
	je	L65
	movl	4(%esi), %esi
	testl	%esi, %esi
	jne	L64
	jmp	L103
L65:
	testl	%esi, %esi
	je	L103
L63:
	cmpl	$0, _firstname
	je	L70
	movl	_firstname, %edi
L71:
	movl	24(%esi), %eax
	shrl	$19, %eax
	andl	$1, %eax
	pushl	%eax
	pushl	_dsd+152
	leal	4(%edi), %eax
	pushl	%eax
	call	_wildcmp
	addl	$12, %esp
	movl	$1, %edx
	testl	%eax, %eax
	je	L62
	movl	(%edi), %edi
	testl	%edi, %edi
	jne	L71
L70:
	cmpb	$0, _changed
	je	L76
	cmpl	$0, _numfiles
	je	L76
	call	_printfiles
L76:
	cmpl	$0, _dsd+220
	jns	L77
	cmpb	$0, _fullfmt
	jne	L79
	cmpb	$0, _longfmt
	je	L78
L79:
	cmpl	$-41, _dsd+220
	je	L77
	cmpl	$-81, _dsd+220
	je	L77
	cmpl	$-84, _dsd+220
	je	L77
	cmpl	$-40, _dsd+220
	je	L77
L78:
	pushl	$_dsd+28
	leal	-264(%ebp), %eax
	pushl	%eax
	call	_strmov
	movl	%eax, %edx
	movl	$254, %eax
	movl	%eax, %esi
	subl	_dsd+60, %esi
	pushl	%esi
	pushl	_dsd+68
	pushl	%edx
	call	_strnmov
	subl	_dsd+72, %esi
	addl	$20, %esp
	testl	%esi, %esi
	jle	L80
	pushl	%esi
	pushl	$_dsd+84
	pushl	%eax
	call	_strnmov
	subl	_dsd+116, %esi
	addl	$12, %esp
	testl	%esi, %esi
	jle	L80
	pushl	%esi
	pushl	_dsd+120
	pushl	%eax
	call	_strnmov
	subl	_dsd+124, %esi
	addl	$12, %esp
	testl	%esi, %esi
	jle	L80
	pushl	%esi
	pushl	_dsd+152
	pushl	%eax
	call	_strnmov
	addl	$12, %esp
L80:
	movb	$0, -9(%ebp)
	pushl	_dsd+220
	leal	-264(%ebp), %eax
	pushl	%eax
	call	_errormsg
	movl	$0, %edx
	jmp	L62
L77:
	movl	_dsd+156, %eax
	addl	_dsd+172, %eax
	movl	$1, %edx
	testl	%eax, %eax
	je	L62
	cmpb	$0, _changed
	je	L84
	call	_printheading
L84:
	movb	$0, _changed
	testb	$2, _fileparm+28
	jne	L85
	cmpl	$0, _dsd+156
	jle	L85
	cmpl	$2, _dsd+156
	jg	L85
	movl	_dsd+152, %eax
	cmpb	$46, (%eax)
	jne	L85
	cmpl	$1, _dsd+156
	je	L103
	cmpb	$46, 1(%eax)
	je	L103
L85:
	cmpb	$0, _prtnamelong
	je	L87
	movl	_dsd+156, %eax
	addl	_needname, %eax
	addl	$15, %eax
	pushl	%eax
	call	_getmem
	movl	%eax, _thisfile
	movl	_dsd+156, %eax
	movl	%eax, _namesize
	addl	$4, %esp
	testb	$16, _dsd+22
	je	L88
	incl	%eax
	movl	%eax, _namesize
L88:
	movl	_needed, %eax
	cmpl	_namesize, %eax
	jge	L89
	movl	_namesize, %eax
	movl	%eax, _needed
L89:
	pushl	_dsd+152
	movl	_thisfile, %eax
	addl	_needname, %eax
	addl	$14, %eax
	pushl	%eax
	call	_strmov
	addl	$8, %esp
	cmpl	$0, _neednamedos
	js	L91
	pushl	_dsd+168
	movl	_thisfile, %eax
	addl	_neednamedos, %eax
	addl	$14, %eax
	pushl	%eax
	call	_strmov
	addl	$8, %esp
	jmp	L91
L87:
	movl	_dsd+172, %eax
	addl	_needname, %eax
	addl	$15, %eax
	pushl	%eax
	call	_getmem
	movl	%eax, _thisfile
	pushl	_dsd+168
	addl	_needname, %eax
	addl	$14, %eax
	pushl	%eax
	call	_strmov
	addl	$12, %esp
L91:
	incl	_numfiles
	movl	_dsd+220, %edx
	movl	_thisfile, %eax
	movl	%edx, 10(%eax)
	cmpl	$0, _firstfile
	jne	L92
	movl	_thisfile, %eax
	movl	%eax, _firstfile
	jmp	L93
L92:
	movl	_thisfile, %edx
	movl	_lastfile, %eax
	movl	%edx, (%eax)
L93:
	movl	_thisfile, %eax
	movl	%eax, _lastfile
	movl	$0, (%eax)
	cmpl	$0, _needlength
	js	L94
	movl	_fileparm+56, %ecx
	movl	_thisfile, %edx
	movl	_needlength, %eax
	movl	%ecx, 14(%eax,%edx)
L94:
	cmpl	$0, _needalloc
	js	L95
	movl	_fileparm+64, %ecx
	movl	_thisfile, %edx
	movl	_needalloc, %eax
	movl	%ecx, 14(%eax,%edx)
L95:
	cmpl	$0, _needcdate
	js	L96
	movl	_fileparm+72, %eax
	movl	_fileparm+76, %edx
	movl	_thisfile, %ebx
	movl	_needcdate, %ecx
	movl	%eax, 14(%ecx,%ebx)
	movl	%edx, 18(%ecx,%ebx)
L96:
	cmpl	$0, _needmdate
	js	L97
	movl	_fileparm+84, %eax
	movl	_fileparm+88, %edx
	movl	_thisfile, %ebx
	movl	_needmdate, %ecx
	movl	%eax, 14(%ecx,%ebx)
	movl	%edx, 18(%ecx,%ebx)
L97:
	cmpl	$0, _needadate
	js	L98
	movl	_fileparm+96, %eax
	movl	_fileparm+100, %edx
	movl	_thisfile, %ebx
	movl	_needadate, %ecx
	movl	%eax, 14(%ecx,%ebx)
	movl	%edx, 18(%ecx,%ebx)
L98:
	cmpl	$0, _needprot
	js	L99
	movl	_fileparm+108, %ecx
	movl	_thisfile, %edx
	movl	_needprot, %eax
	movl	%ecx, 14(%eax,%edx)
L99:
	cmpl	$0, _needowner
	js	L100
	pushl	$_owner
	movl	_thisfile, %eax
	addl	_needowner, %eax
	addl	$14, %eax
	pushl	%eax
	call	_strcpy
	addl	$8, %esp
L100:
	movb	_dsd+22, %dl
	movl	_thisfile, %eax
	movb	%dl, 8(%eax)
	testb	$16, %dl
	je	L101
	incl	_dircnt
	jmp	L102
L101:
	incl	_filecnt
L102:
	cmpl	$0, _dsd+220
	js	L103
	pushl	_fileparm+56
	pushl	$_totallen
	call	_longadd
	pushl	_fileparm+64
	pushl	$_totalalloc
	call	_longadd
	addl	$16, %esp
L103:
	movl	$1, %edx
L62:
	movl	%edx, %eax
	leal	-8(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
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
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	subl	$208, %esp
	testb	$-32, _dsd+20
	je	L108
	cmpb	$0, _prtvolid
	je	L108
	movl	_fileparm+40, %eax
	movl	%eax, _lblqab+12
/APP
	pushl %ds
	pushl $_lblqab
	callf _svcIoQueue##
/NO_APP
	testl	%eax, %eax
	js	L110
	cmpl	$0, _lblqab+4
	js	L110
	cmpl	$24, _lblqab+8
	jg	L108
L110:
	movb	$0, _vollabel+24
L108:
	leal	-216(%ebp), %esi
	call	_pagecheck
	pushl	$10
	call	_putchar
	addl	$4, %esp
	cmpb	$0, _prtvolid
	je	L111
	pushl	$_dsd+84
	pushl	_dsd+68
	pushl	$_dsd+28
	pushl	$LC94
	pushl	%esi
	call	_sprintf
	addl	%eax, %esi
	addl	$20, %esp
	cmpb	$0, _vollabel+24
	je	L112
	pushl	$_vollabel+24
	pushl	$LC95
	pushl	%esi
	call	_sprintf
	addl	%eax, %esi
	addl	$12, %esp
L112:
	cmpb	$0, _prtpath
	je	L113
	movl	$LC96, %eax
	jmp	L114
L113:
	movl	$LC97, %eax
L114:
	pushl	%eax
	pushl	%esi
	call	_strmov
	movl	%eax, %esi
	addl	$8, %esp
L111:
	movl	%esi, %edi
	cmpb	$0, _prtpath
	je	L115
	pushl	_dsd+120
	pushl	$LC98
	pushl	%esi
	call	_sprintf
	leal	(%eax,%esi), %esi
	addl	$12, %esp
L115:
	cmpb	$0, _prtvolid
	jne	L117
	cmpb	$0, _prtpath
	je	L107
L117:
	call	_pagecheck
	pushl	$LC97
	pushl	%esi
	call	_strmov
	addl	$8, %esp
	cmpb	$0, _prtvolid
	je	L118
	cmpb	$0, _prtpath
	je	L118
	movl	%esi, %eax
	leal	-216(%ebp), %edx
	subl	%edx, %eax
	cmpl	_pib+16, %eax
	jle	L118
	pushl	$LC97
	leal	-2(%edi), %eax
	pushl	%eax
	call	_strmov
	pushl	$__stdout
	leal	-216(%ebp), %eax
	pushl	%eax
	call	_fputs
	call	_pagecheck
	pushl	$__stdout
	pushl	%edi
	call	_fputs
	addl	$24, %esp
	jmp	L107
L118:
	pushl	$__stdout
	leal	-216(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$8, %esp
L107:
	leal	-8(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
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
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	subl	$216, %esp
	cmpl	$0, _shownames
	je	L121
	cmpb	$0, _longfmt
	je	L122
	movb	$1, _prtacross
L122:
	cmpb	$0, _prtacross
	jne	L123
	movb	$1, _prtexpand
L123:
	cmpb	$0, _nosort
	jne	L124
	pushl	$0
	pushl	$_comp
	pushl	_firstfile
	call	_heapsort
	movl	%eax, _firstfile
	addl	$12, %esp
L124:
	cmpb	$0, _longfmt
	jne	L125
	cmpb	$0, _onecol
	jne	L127
	movl	_pib+16, %edx
	cmpb	$0, _prtexpand
	je	L129
	movl	_needed, %eax
	incl	%eax
	movl	%eax, %ecx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	%ecx
	jmp	L128
L129:
	movl	%edx, %eax
	testl	%edx, %edx
	jns	L131
	addl	$15, %eax
L131:
	sarl	$4, %eax
	jmp	L128
L127:
	movl	$1, %eax
L128:
	movl	%eax, _numcolumns
	cmpl	$5, %eax
	jle	L126
	movl	$5, _numcolumns
L126:
	movl	_pib+16, %edx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	_numcolumns
	movl	%eax, _colwidth
	leal	-216(%ebp), %eax
	movl	%eax, -224(%ebp)
	movl	_numcolumns, %eax
	movl	%eax, _left
	cmpb	$0, _prtacross
	je	L138
	leal	-216(%ebp), %edi
L133:
	pushl	-224(%ebp)
	pushl	%edi
	pushl	_firstfile
	call	_wideentry
	movl	%eax, -224(%ebp)
	movl	_firstfile, %eax
	movl	(%eax), %esi
	pushl	%eax
	call	_free
	addl	$16, %esp
	movl	%esi, _firstfile
	testl	%esi, %esi
	jne	L133
	call	_pagecheck
	pushl	$LC97
	pushl	-224(%ebp)
	call	_strmov
	pushl	$__stdout
	leal	-216(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$16, %esp
	jmp	L121
L138:
	movl	_numfiles, %edx
	addl	_numcolumns, %edx
	decl	%edx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	_numcolumns
	movl	%eax, _collength
	cmpl	$0, _page
	je	L141
	movl	_pib+20, %ecx
	movl	%ecx, %eax
	subl	_curline, %eax
	leal	-3(%eax), %edx
	testl	%edx, %edx
	jg	L142
	leal	-3(%ecx), %edx
L142:
	cmpl	%edx, _collength
	jle	L141
	movl	%edx, _collength
L141:
	movl	_collength, %eax
	imull	_numcolumns, %eax
	subl	%eax, _numfiles
	movl	_numcolumns, %edi
	movl	$_colarray, -220(%ebp)
L144:
	movl	_firstfile, %eax
	movl	-220(%ebp), %edx
	movl	%eax, (%edx)
	addl	$4, %edx
	movl	%edx, -220(%ebp)
	movl	_collength, %edx
	cmpl	$0, _firstfile
	je	L146
	decl	%edx
	testl	%edx, %edx
	jle	L148
L151:
	movl	_firstfile, %eax
	movl	(%eax), %eax
	movl	%eax, _firstfile
	testl	%eax, %eax
	je	L146
	decl	%edx
	testl	%edx, %edx
	jg	L151
L148:
	cmpl	$0, _firstfile
	je	L146
	movl	_firstfile, %eax
	movl	(%eax), %esi
	movl	$0, (%eax)
	movl	%esi, _firstfile
L146:
	decl	%edi
	testl	%edi, %edi
	jg	L144
L154:
	movl	_numcolumns, %edi
	movl	$_colarray, -220(%ebp)
	movl	_colarray, %esi
	testl	%esi, %esi
	je	L158
	decl	%edi
	js	L158
L161:
	pushl	-224(%ebp)
	leal	-216(%ebp), %ecx
	pushl	%ecx
	pushl	%esi
	call	_wideentry
	movl	%eax, -224(%ebp)
	movl	(%esi), %eax
	movl	-220(%ebp), %edx
	movl	%eax, (%edx)
	pushl	%esi
	call	_free
	addl	$4, -220(%ebp)
	addl	$16, %esp
	movl	-220(%ebp), %ecx
	movl	(%ecx), %esi
	testl	%esi, %esi
	je	L158
	decl	%edi
	jns	L161
L158:
	call	_pagecheck
	pushl	$LC97
	pushl	-224(%ebp)
	call	_strmov
	pushl	$__stdout
	leal	-216(%ebp), %eax
	pushl	%eax
	call	_fputs
	leal	-216(%ebp), %edx
	movl	%edx, -224(%ebp)
	movl	_numcolumns, %eax
	movl	%eax, _left
	addl	$16, %esp
	cmpl	$0, _colarray
	jne	L154
	cmpl	$0, _page
	je	L140
	cmpl	$0, _firstfile
	je	L121
	call	_pagecheck
	pushl	$10
	call	_putchar
	addl	$4, %esp
L140:
	cmpl	$0, _firstfile
	jne	L138
	jmp	L121
L125:
	cmpb	$0, _fullfmt
	jne	L166
L167:
	leal	-216(%ebp), %ecx
	movl	%ecx, -224(%ebp)
	cmpb	$0, _prtnamedos
	jne	L171
	cmpb	$0, _prtnamelong
	jne	L170
L171:
	movl	$14, %edi
	cmpb	$0, _prtnamelong
	je	L172
	movl	_firstfile, %eax
	addl	_neednamedos, %eax
	jmp	L236
L172:
	movl	_firstfile, %eax
	addl	_needname, %eax
L236:
	addl	$14, %eax
	movb	(%eax), %dl
	incl	%eax
	testb	%dl, %dl
	je	L233
L177:
	movl	-224(%ebp), %ecx
	movb	%dl, (%ecx)
	incl	%ecx
	movl	%ecx, -224(%ebp)
	decl	%edi
	movb	(%eax), %dl
	incl	%eax
	testb	%dl, %dl
	jne	L177
L233:
	movl	_firstfile, %eax
	testb	$16, 8(%eax)
	je	L178
	movl	-224(%ebp), %eax
	movb	$92, (%eax)
	incl	%eax
	movl	%eax, -224(%ebp)
	decl	%edi
L178:
	decl	%edi
	testl	%edi, %edi
	jle	L170
L182:
	movl	-224(%ebp), %edx
	movb	$32, (%edx)
	incl	%edx
	movl	%edx, -224(%ebp)
	decl	%edi
	testl	%edi, %edi
	jg	L182
L170:
	movl	_firstfile, %eax
	cmpl	$0, 10(%eax)
	js	L183
	cmpb	$0, _showattr
	je	L184
	movl	-224(%ebp), %edx
	incl	-224(%ebp)
	testb	$1, 8(%eax)
	je	L185
	movb	$82, %al
	jmp	L186
L185:
	movb	$45, %al
L186:
	movb	%al, (%edx)
	movl	-224(%ebp), %edx
	incl	-224(%ebp)
	movl	_firstfile, %eax
	testb	$2, 8(%eax)
	je	L187
	movb	$72, %al
	jmp	L188
L187:
	movb	$45, %al
L188:
	movb	%al, (%edx)
	movl	-224(%ebp), %edx
	incl	-224(%ebp)
	movl	_firstfile, %eax
	testb	$4, 8(%eax)
	je	L189
	movb	$83, %al
	jmp	L190
L189:
	movb	$45, %al
L190:
	movb	%al, (%edx)
	movl	-224(%ebp), %edx
	incl	-224(%ebp)
	movl	_firstfile, %eax
	testb	$32, 8(%eax)
	je	L191
	movb	$77, %al
	jmp	L192
L191:
	movb	$45, %al
L192:
	movb	%al, (%edx)
L184:
	movl	_firstfile, %eax
	addl	_needcdate, %eax
	addl	$14, %eax
	pushl	%eax
	pushl	$LC99
	pushl	-224(%ebp)
	call	_sdt2str
	addl	%eax, -224(%ebp)
	movl	_firstfile, %edx
	movl	_needlength, %eax
	pushl	14(%eax,%edx)
	pushl	$LC100
	pushl	-224(%ebp)
	call	_sprintf
	addl	%eax, -224(%ebp)
	addl	$24, %esp
	jmp	L193
L183:
	movl	_firstfile, %eax
/APP
	pushl 10(%eax)
	pushl $3
	pushl %ds
	pushl -224(%ebp)
	callf _svcSysErrMsg##
/NO_APP
	addl	%eax, -224(%ebp)
	leal	-216(%ebp), %eax
	movl	-224(%ebp), %edx
	subl	%eax, %edx
	cmpb	$0, _showattr
	je	L194
	movl	$38, %eax
	jmp	L237
L194:
	movl	$33, %eax
L237:
	subl	%edx, %eax
	decl	%eax
	testl	%eax, %eax
	jle	L193
L199:
	movl	-224(%ebp), %ecx
	movb	$32, (%ecx)
	incl	%ecx
	movl	%ecx, -224(%ebp)
	decl	%eax
	testl	%eax, %eax
	jg	L199
L193:
	cmpb	$0, _prtnamelong
	je	L200
	movl	_firstfile, %eax
	testb	$16, 8(%eax)
	je	L201
	movl	$LC101, %eax
	jmp	L202
L201:
	movl	$LC86, %eax
L202:
	pushl	%eax
	movl	_firstfile, %eax
	addl	_needname, %eax
	addl	$14, %eax
	pushl	%eax
	pushl	$LC102
	pushl	-224(%ebp)
	call	_sprintf
	addl	%eax, -224(%ebp)
	addl	$16, %esp
L200:
	call	_pagecheck
	pushl	$LC97
	pushl	-224(%ebp)
	call	_strmov
	pushl	$__stdout
	leal	-216(%ebp), %eax
	pushl	%eax
	call	_fputs
	movl	_firstfile, %eax
	movl	(%eax), %esi
	pushl	%eax
	call	_free
	addl	$20, %esp
	movl	%esi, _firstfile
	testl	%esi, %esi
	jne	L167
	jmp	L121
L166:
	leal	-216(%ebp), %edi
L205:
	call	_pagecheck
	movl	_firstfile, %eax
	testb	$16, 8(%eax)
	je	L208
	movl	$LC101, %eax
	jmp	L209
L208:
	movl	$LC86, %eax
L209:
	pushl	%eax
	movl	_firstfile, %eax
	addl	_needname, %eax
	addl	$14, %eax
	pushl	%eax
	pushl	$LC103
	call	_printf
	call	_pagecheck
	addl	$12, %esp
	movl	_firstfile, %eax
	cmpl	$0, 10(%eax)
	js	L210
	pushl	$LC104
	pushl	%edi
	call	_strmov
	movl	%eax, -224(%ebp)
	movl	_firstfile, %eax
	testb	$1, 8(%eax)
	je	L211
	movl	$LC105, %eax
	jmp	L212
L211:
	movl	$LC106, %eax
L212:
	pushl	%eax
	pushl	-224(%ebp)
	call	_strmov
	movl	%eax, -224(%ebp)
	movl	_firstfile, %eax
	testb	$2, 8(%eax)
	je	L213
	movl	$LC107, %eax
	jmp	L214
L213:
	movl	$LC108, %eax
L214:
	pushl	%eax
	pushl	-224(%ebp)
	call	_strmov
	movl	%eax, -224(%ebp)
	movl	_firstfile, %eax
	testb	$4, 8(%eax)
	je	L215
	movl	$LC109, %eax
	jmp	L216
L215:
	movl	$LC110, %eax
L216:
	pushl	%eax
	pushl	-224(%ebp)
	call	_strmov
	movl	%eax, -224(%ebp)
	addl	$32, %esp
	movl	_firstfile, %eax
	testb	$32, 8(%eax)
	je	L217
	movl	$LC111, %eax
	jmp	L218
L217:
	movl	$LC112, %eax
L218:
	pushl	%eax
	pushl	-224(%ebp)
	call	_strmov
	pushl	$__stdout
	pushl	%edi
	call	_fputs
	movl	_firstfile, %eax
	addl	_needcdate, %eax
	addl	$14, %eax
	pushl	%eax
	pushl	$LC113
	pushl	%edi
	call	_sdt2str
	call	_pagecheck
	pushl	$__stdout
	pushl	%edi
	call	_fputs
	addl	$36, %esp
	movl	_firstfile, %eax
	addl	_needmdate, %eax
	addl	$14, %eax
	pushl	%eax
	pushl	$LC114
	pushl	%edi
	call	_sdt2str
	call	_pagecheck
	pushl	$__stdout
	pushl	%edi
	call	_fputs
	movl	_firstfile, %eax
	addl	_needadate, %eax
	addl	$14, %eax
	pushl	%eax
	pushl	$LC115
	pushl	%edi
	call	_sdt2str
	addl	$32, %esp
	call	_pagecheck
	pushl	$__stdout
	pushl	%edi
	call	_fputs
	call	_pagecheck
	movl	_firstfile, %edx
	movl	_needlength, %eax
	pushl	14(%eax,%edx)
	pushl	$LC116
	call	_printf
	call	_pagecheck
	movl	_firstfile, %edx
	movl	_needalloc, %eax
	pushl	14(%eax,%edx)
	pushl	$LC117
	call	_printf
	addl	$24, %esp
	jmp	L219
L210:
	pushl	$LC118
	pushl	%edi
	call	_strmov
	movl	%eax, -224(%ebp)
	addl	$8, %esp
	movl	_firstfile, %eax
/APP
	pushl 10(%eax)
	pushl $3
	pushl %ds
	pushl -224(%ebp)
	callf _svcSysErrMsg##
/NO_APP
	pushl	$LC97
	pushl	-224(%ebp)
	call	_strmov
	pushl	$__stdout
	pushl	%edi
	call	_fputs
	addl	$16, %esp
L219:
	movl	_firstfile, %eax
	movl	(%eax), %esi
	pushl	%eax
	call	_free
	addl	$4, %esp
	movl	%esi, _firstfile
	testl	%esi, %esi
	jne	L205
L121:
	cmpl	$0, _lsum
	je	L221
	call	_pagecheck
	pushl	$LC119
	leal	-216(%ebp), %eax
	pushl	%eax
	call	_strmov
	movl	%eax, -224(%ebp)
	addl	$8, %esp
	cmpl	$0, _dircnt
	je	L222
	cmpl	$1, _dircnt
	jne	L223
	movl	$LC82, %eax
	jmp	L224
L223:
	movl	$LC83, %eax
L224:
	pushl	%eax
	pushl	_dircnt
	pushl	$LC120
	pushl	-224(%ebp)
	call	_sprintf
	addl	%eax, -224(%ebp)
	addl	$16, %esp
L222:
	cmpl	$1, _filecnt
	jne	L225
	movl	$LC86, %eax
	jmp	L226
L225:
	movl	$LC87, %eax
L226:
	pushl	%eax
	pushl	_filecnt
	pushl	$LC121
	pushl	-224(%ebp)
	call	_sprintf
	addl	%eax, -224(%ebp)
	addl	$16, %esp
	cmpb	$0, _longfmt
	je	L227
	pushl	_totalalloc+4
	pushl	_totalalloc
	pushl	_totallen+4
	pushl	_totallen
	je	L228
	cmpl	$0, _dircnt
	je	L228
	movl	$LC97, %eax
	jmp	L229
L228:
	movl	$LC86, %eax
L229:
	pushl	%eax
	pushl	$LC122
	pushl	-224(%ebp)
	call	_sprintf
	addl	%eax, -224(%ebp)
	addl	$28, %esp
L227:
	pushl	$LC90
	pushl	-224(%ebp)
	call	_strmov
	pushl	$__stdout
	leal	-216(%ebp), %eax
	pushl	%eax
	call	_fputs
	addl	$16, %esp
L221:
	movl	$0, _firstfile
	movl	_gfilecnt, %eax
	addl	_filecnt, %eax
	movl	%eax, _gfilecnt
	movl	$0, _filecnt
	movl	_gdircnt, %eax
	addl	_dircnt, %eax
	movl	%eax, _gdircnt
	movl	$0, _dircnt
	pushl	$_totallen
	pushl	$_gtotallen
	call	_longlongadd
	movl	$0, _totallen+4
	movl	$0, _totallen
	pushl	$_totalalloc
	pushl	$_gtotalalloc
	call	_longlongadd
	movl	$0, _totalalloc+4
	movl	$0, _totalalloc
	movl	$0, _numfiles
	incl	_numlisted
	leal	-8(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.globl _wideentry
_wideentry:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	movl	16(%ebp), %esi
	movl	_needname, %eax
	movl	8(%ebp), %edx
	leal	14(%eax,%edx), %edi
	cld
	movl	$-1, %ecx
	movb	$0, %al
	repne
	scasb
	movl	%ecx, %eax
	notl	%eax
	leal	-1(%eax), %edx
	movl	8(%ebp), %ecx
	testb	$16, 8(%ecx)
	je	L239
	movl	%eax, %edx
L239:
	addl	_colwidth, %edx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	_colwidth
	movl	%eax, _needed
	cmpl	_left, %eax
	jle	L240
	call	_pagecheck
	pushl	$LC97
	pushl	%esi
	call	_strmov
	pushl	$__stdout
	pushl	12(%ebp)
	call	_fputs
	movl	12(%ebp), %esi
	movl	_numcolumns, %eax
	movl	%eax, _left
	addl	$16, %esp
	jmp	L241
L240:
	movl	_left, %eax
	cmpl	_numcolumns, %eax
	je	L241
	movl	%esi, %edx
	subl	12(%ebp), %edx
	movl	%edx, %eax
	sarl	$31, %edx
	idivl	_colwidth
	movl	_colwidth, %eax
	subl	%edx, %eax
	movl	%eax, _fill
L243:
	movb	$32, (%esi)
	incl	%esi
	decl	_fill
	cmpl	$0, _fill
	jg	L243
L241:
	movl	8(%ebp), %eax
	addl	_needname, %eax
	addl	$14, %eax
	pushl	%eax
	pushl	%esi
	call	_strmov
	movl	%eax, %esi
	movl	_left, %eax
	subl	_needed, %eax
	movl	%eax, _left
	addl	$8, %esp
	movl	8(%ebp), %eax
	testb	$16, 8(%eax)
	je	L247
	movb	$92, (%esi)
	incl	%esi
L247:
	movl	%esi, %eax
	leal	-8(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.globl _comp
_comp:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	subl	$4, %esp
	movl	8(%ebp), %edi
	movl	12(%ebp), %esi
	cmpb	$0, _dirsort
	je	L249
	movb	8(%edi), %cl
	andl	$16, %ecx
	movsbl	%cl,%edx
	movl	$0, %eax
	movb	8(%esi), %al
	andl	$16, %eax
	cmpl	%eax, %edx
	je	L249
	testb	%cl, %cl
	sete	%al
	jmp	L275
L249:
	cmpb	$0, _revsort
	je	L253
	movl	%edi, %eax
	movl	%esi, %edi
	movl	%eax, %esi
L253:
	cmpb	$0, _datesort
	je	L254
	movl	_needcdate, %eax
	movl	18(%eax,%edi), %edx
	cmpl	18(%eax,%esi), %edx
	je	L255
	cmpl	18(%eax,%esi), %edx
	jmp	L277
L255:
	movl	_needcdate, %eax
	movl	14(%eax,%edi), %edx
	cmpl	14(%eax,%esi), %edx
	je	L261
	cmpl	14(%eax,%esi), %edx
	jb	L276
	jmp	L272
L254:
	cmpb	$0, _extsort
	je	L262
	pushl	$46
	movl	%edi, %eax
	addl	_needname, %eax
	addl	$14, %eax
	pushl	%eax
	call	_strrchr
	movl	%eax, -12(%ebp)
	pushl	$46
	movl	%esi, %eax
	addl	_needname, %eax
	addl	$14, %eax
	pushl	%eax
	call	_strrchr
	addl	$16, %esp
	cmpl	$0, -12(%ebp)
	je	L263
	testl	%eax, %eax
	je	L263
	pushl	%eax
	pushl	-12(%ebp)
	call	_strcmp
	addl	$8, %esp
	movl	%eax, %edx
	testl	%eax, %eax
	jne	L248
	jmp	L261
L263:
	cmpl	%eax, -12(%ebp)
	je	L261
	cmpl	$0, -12(%ebp)
	setne	%al
L275:
	andl	$255, %eax
	leal	-1(%eax,%eax), %edx
	jmp	L248
L262:
	cmpb	$0, _sizesort
	je	L261
	movl	_needlength, %eax
	movl	14(%eax,%edi), %edx
	cmpl	14(%eax,%esi), %edx
	je	L261
	cmpl	14(%eax,%esi), %edx
L277:
	jge	L272
L276:
	movl	$-1, %eax
	jmp	L274
L272:
	movl	$1, %eax
	jmp	L274
L261:
	movl	_needname, %eax
	leal	14(%eax,%esi), %edx
	pushl	%edx
	leal	14(%eax,%edi), %eax
	pushl	%eax
	call	_stricmp
L274:
	movl	%eax, %edx
L248:
	movl	%edx, %eax
	leal	-8(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
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
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	cmpl	$0, _page
	je	L278
	incl	_curline
	movl	_pib+20, %eax
	subl	$2, %eax
	cmpl	%eax, _curline
	jl	L278
	pushl	$__stdout
	pushl	$46
	pushl	$1
	pushl	$LC123
	call	_fwrite
	addl	$16, %esp
L297:
/APP
	pushl $5
	pushl %ds
	pushl $_snglparm
	callf _svcIoInSingleP##
/NO_APP
	movl	%eax, %ebx
	movl	_snglparm, %edx
	movl	_snglparm+4, %ecx
	movl	%edx, _normparm+8
	movl	%ecx, _normparm+12
/APP
	pushl $5
	pushl %ds
	pushl $0
	pushl $0
	pushl %ds
	pushl $_normparm
	callf _svcIoInBlockP##
/NO_APP
	movl	$0, %esi
	movl	%ebx, %eax
	andl	$127, %eax
	pushl	%eax
	call	_toupper
	addl	$4, %esp
	cmpl	$32, %eax
	je	L288
	cmpl	$32, %eax
	jg	L294
	cmpl	$3, %eax
	je	L285
	cmpl	$3, %eax
	jg	L295
	testl	%eax, %eax
	je	L292
	jmp	L291
L295:
	cmpl	$13, %eax
	je	L288
	jmp	L291
L294:
	cmpl	$71, %eax
	je	L286
	cmpl	$71, %eax
	jle	L291
	cmpl	$72, %eax
	je	L291
	cmpl	$81, %eax
	jne	L291
L285:
	pushl	$__stdout
	pushl	$4
	pushl	$1
	pushl	$LC124
	call	_fwrite
	pushl	$0
	call	_exit
L286:
	movl	$0, _page
	jmp	L280
L288:
	movl	$0, _curline
	jmp	L280
L291:
	pushl	$__stdout
	pushl	$266
	pushl	$1
	pushl	$LC125
	call	_fwrite
	addl	$16, %esp
L292:
	movl	$1, %esi
L280:
	cmpl	$1, %esi
	je	L297
	pushl	$__stdout
	pushl	$4
	pushl	$1
	pushl	$LC124
	call	_fwrite
	addl	$16, %esp
L278:
	movl	-4(%ebp), %esi
	leave
	ret
	.p2align 5
LC126:
	.ascii "? %s: Invalid DISPLAY option value, %d\12\0"
.globl _optdisplay
_optdisplay:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	testb	$8, 13(%eax)
	je	L302
	cmpl	$15, 4(%eax)
	ja	L319
	movl	4(%eax), %eax
	jmp	*L320(,%eax,4)
	.p2align 2
	.p2align 2
L320:
	.long	L303
	.long	L304
	.long	L305
	.long	L306
	.long	L307
	.long	L308
	.long	L309
	.long	L310
	.long	L321
	.long	L312
	.long	L313
	.long	L314
	.long	L315
	.long	L316
	.long	L317
	.long	L318
L303:
	movl	$1, _lsum
	jmp	L302
L304:
	movl	$0, _lsum
	jmp	L302
L305:
	movb	$1, _prtpath
	jmp	L302
L306:
	movb	$0, _prtpath
	jmp	L302
L307:
	movb	$1, _prtvolid
	jmp	L302
L308:
	movb	$0, _prtvolid
	jmp	L302
L309:
	movb	$1, _showattr
	jmp	L302
L310:
	movb	$0, _showattr
	jmp	L302
L312:
	movb	$0, _prtnamedos
L313:
	movb	$1, _prtnamelong
	jmp	L302
L314:
	movb	$0, _prtnamelong
L321:
	movb	$1, _prtnamedos
	jmp	L302
L315:
	movb	$0, _prtacross
	jmp	L302
L316:
	movb	$1, _prtacross
	jmp	L302
L317:
	movb	$1, _prtexpand
	jmp	L302
L318:
	movb	$0, _prtexpand
	jmp	L302
L319:
	pushl	4(%eax)
	pushl	$_prgname
	pushl	$LC126
	pushl	$__stderr
	call	_fprintf
	pushl	$3
	call	_exit
L302:
	movl	$1, %eax
	movl	%ebp, %esp
	popl	%ebp
	ret
LC127:
	.ascii "after time = %s\12\0"
.globl _optafter
_optafter:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	pushl	4(%eax)
	pushl	$LC127
	call	_printf
	movl	$1, %eax
	movl	%ebp, %esp
	popl	%ebp
	ret
.globl _optall
_optall:
	pushl	%ebp
	movl	%esp, %ebp
	movw	$151, _fileparm+28
	movl	$1, %eax
	popl	%ebp
	ret
LC128:
	.ascii "before time = %s\12\0"
.globl _optbefore
_optbefore:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	pushl	4(%eax)
	pushl	$LC128
	call	_printf
	movl	$1, %eax
	movl	%ebp, %esp
	popl	%ebp
	ret
.globl _optdir
_optdir:
	pushl	%ebp
	movl	%esp, %ebp
	movw	$16, _fileparm+28
	movl	$1, %eax
	popl	%ebp
	ret
.globl _optnodir
_optnodir:
	pushl	%ebp
	movl	%esp, %ebp
	andw	$-17, _fileparm+28
	movl	$1, %eax
	popl	%ebp
	ret
.globl _optone
_optone:
	pushl	%ebp
	movl	%esp, %ebp
	movb	$1, _onecol
	call	_optshort
	movl	$1, %eax
	popl	%ebp
	ret
.globl _optfull
_optfull:
	pushl	%ebp
	movl	%esp, %ebp
	movb	$1, _fullfmt
	movb	$1, _longfmt
	movl	$0, _needalloc
	movl	$4, _needowner
	movl	$40, _needprot
	movl	$44, _needadate
	movl	$52, _needmdate
	movl	$60, _needcdate
	movl	$68, _needlength
	movl	$72, _needname
	movl	$1, %eax
	popl	%ebp
	ret
.globl _optshort
_optshort:
	pushl	%ebp
	movl	%esp, %ebp
	movb	$0, _longfmt
	movb	$0, _fullfmt
	movl	$-1, _needalloc
	movl	$-1, _needowner
	movl	$-1, _needprot
	movl	$-1, _needadate
	movl	$-1, _needmdate
	movl	$-1, _needcdate
	movl	$-1, _needlength
	movl	$0, _needname
	movl	$1, %eax
	popl	%ebp
	ret
.globl _optlong
_optlong:
	pushl	%ebp
	movl	%esp, %ebp
	movb	$1, _longfmt
	movb	$0, _fullfmt
	movl	$-1, _needalloc
	movl	$0, _needowner
	movl	$36, _needprot
	movl	$-1, _needadate
	movl	$-1, _needmdate
	movl	$40, _needcdate
	movl	$48, _needlength
	movl	$52, _needname
	movl	$1, %eax
	popl	%ebp
	ret
	.p2align 5
LC129:
	.ascii "? %s: Invalid SORT option value, %d\12\0"
.globl _optsort
_optsort:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	testb	$8, 13(%eax)
	jne	L332
	movb	$1, _datesort
	movb	$1, _revsort
	movb	$0, _extsort
	jmp	L345
L332:
	cmpl	$8, 4(%eax)
	ja	L343
	movl	4(%eax), %eax
	jmp	*L344(,%eax,4)
	.p2align 2
	.p2align 2
L344:
	.long	L334
	.long	L335
	.long	L336
	.long	L337
	.long	L338
	.long	L339
	.long	L340
	.long	L341
	.long	L342
L334:
	movb	$1, _nosort
	movb	$0, _dirsort
	movb	$0, _revsort
	movb	$0, _extsort
	movb	$0, _datesort
	jmp	L333
L335:
	movb	$0, _revsort
	jmp	L345
L336:
	movb	$1, _revsort
	jmp	L345
L337:
	movb	$0, _datesort
	jmp	L347
L338:
	movb	$1, _extsort
	movb	$0, _datesort
L346:
	movb	$0, _sizesort
	jmp	L345
L339:
	movb	$1, _datesort
L347:
	movb	$0, _extsort
	jmp	L346
L340:
	movb	$1, _sizesort
	movb	$0, _extsort
	movb	$0, _datesort
	jmp	L345
L341:
	movb	$1, _dirsort
	jmp	L345
L342:
	movb	$0, _dirsort
L345:
	movb	$0, _nosort
	jmp	L333
L343:
	pushl	4(%eax)
	pushl	$_prgname
	pushl	$LC129
	pushl	$__stderr
	call	_fprintf
	pushl	$3
	call	_exit
L333:
	movl	$1, %eax
	movl	%ebp, %esp
	popl	%ebp
	ret
.globl _optx
_optx:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	movl	8(%ebp), %edi
	movl	16(%edi), %eax
	addl	$10, %eax
	pushl	%eax
	call	_getmem
	movl	%eax, %esi
	pushl	4(%edi)
	leal	4(%eax), %eax
	pushl	%eax
	call	_strmov
	movl	_firstname, %eax
	movl	%eax, (%esi)
	movl	%esi, _firstname
	movl	$1, %eax
	leal	-8(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section .data
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
	.space 1
	.section .text
LC130:
	.ascii "...\0"
.globl _nonopt
_nonopt:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	subl	$12, %esp
	movl	8(%ebp), %esi
	movb	$0, _hvdefault
	movl	%esi, %edi
	cld
	movl	$-1, %ecx
	movb	$0, %al
	repne
	scasb
	notl	%ecx
	leal	-1(%ecx), %eax
	movb	-1(%eax,%esi), %dl
	cmpb	$58, %dl
	je	L351
	cmpb	$92, %dl
	jne	L350
L351:
	movb	$1, _hvdefault
	addl	$2, %eax
L350:
	addl	$42, %eax
	pushl	%eax
	call	_getmem
	movl	%eax, -12(%ebp)
	movl	$0, 4(%eax)
	pushl	%esi
	movl	-12(%ebp), %eax
	addl	$36, %eax
	pushl	%eax
	call	_strmov
	addl	$12, %esp
	cmpb	$0, _hvdefault
	je	L352
	pushl	$LC80
	pushl	%eax
	call	_strmov
	addl	$8, %esp
L352:
	pushl	$LC130
	movl	-12(%ebp), %esi
	addl	$36, %esi
	pushl	%esi
	call	_strstr
	movl	-12(%ebp), %edx
	movl	%eax, 28(%edx)
	addl	$8, %esp
	testl	%eax, %eax
	je	L353
	cmpl	%esi, %eax
	jne	L354
	movl	%edx, %eax
	addl	$37, %eax
L354:
	movb	$0, (%eax)
	movl	-12(%ebp), %edx
	addl	$36, %edx
/APP
	pushl $268435456
	pushl %ds
	pushl %edx
	pushl %ds
	pushl $_glbidparm.0
	callf _svcIoDevParm##
/NO_APP
	movl	%eax, -20(%ebp)
	movl	-12(%ebp), %edx
	movl	28(%edx), %eax
	movb	$46, 1(%eax)
	movb	$46, (%eax)
	jmp	L355
L353:
	movl	-12(%ebp), %edx
	addl	$36, %edx
/APP
	pushl $268435456
	pushl %ds
	pushl %edx
	pushl %ds
	pushl $_glbidparm.0
	callf _svcIoDevParm##
/NO_APP
	movl	%eax, -20(%ebp)
L355:
	cmpl	$0, -20(%ebp)
	jns	L356
	movl	-12(%ebp), %edx
	movl	$-1, 8(%edx)
	movl	%edx, 12(%edx)
	movl	$0, 20(%edx)
	movl	$0, 16(%edx)
	jmp	L357
L356:
	movl	_glbidparm.0+4, %eax
	movl	-12(%ebp), %edx
	movl	%eax, 8(%edx)
	movl	_glbidparm.0+8, %eax
	movl	%eax, 12(%edx)
	movl	_glbidparm.0+12, %eax
	movl	%eax, 16(%edx)
	movl	_glbidparm.0+16, %eax
	movl	%eax, 20(%edx)
L357:
	movl	_glbidparm.0+24, %eax
	movl	-12(%ebp), %edx
	movl	%eax, 24(%edx)
	movl	%edx, %eax
	addl	$36, %eax
	movl	%eax, 32(%edx)
	movl	-12(%ebp), %eax
	movb	36(%eax), %dl
	addl	$37, %eax
	testb	%dl, %dl
	je	L377
L363:
	cmpb	$58, %dl
	je	L362
	cmpb	$92, %dl
	je	L362
	cmpb	$47, %dl
	jne	L358
L362:
	movl	-12(%ebp), %edx
	movl	%eax, 32(%edx)
L358:
	movb	(%eax), %dl
	incl	%eax
	testb	%dl, %dl
	jne	L363
L377:
	cmpl	$0, _firstspec
	jne	L364
	movl	-12(%ebp), %eax
	movl	%eax, _firstspec
	jmp	L365
L364:
	movl	_firstspec, %edx
	movl	%edx, -16(%ebp)
	movl	-12(%ebp), %edx
	movl	32(%edx), %eax
	subl	28(%edx), %eax
	movl	%eax, -20(%ebp)
L366:
	movl	-12(%ebp), %esi
	addl	$8, %esi
	movl	-16(%ebp), %edi
	addl	$8, %edi
	movl	$16, %ecx
	cld
	repe
	cmpsb
	seta	%dl
	setb	%al
	cmpb	%al, %dl
	jne	L368
	movl	-12(%ebp), %eax
	cmpl	$0, 28(%eax)
	je	L367
	movl	-16(%ebp), %edx
	movl	32(%edx), %eax
	subl	28(%edx), %eax
	cmpl	%eax, -20(%ebp)
	jne	L368
	pushl	-20(%ebp)
	pushl	28(%edx)
	movl	-12(%ebp), %eax
	pushl	28(%eax)
	call	_strnicmp
	addl	$12, %esp
	testl	%eax, %eax
	je	L367
L368:
	movl	-16(%ebp), %edx
	movl	(%edx), %edx
	movl	%edx, -16(%ebp)
	testl	%edx, %edx
	jne	L366
	jmp	L374
L367:
	cmpl	$0, -16(%ebp)
	je	L374
	movl	-12(%ebp), %edx
	movl	-16(%ebp), %eax
	movl	%edx, 4(%eax)
	jmp	L365
L374:
	movl	_thisspec, %eax
	movl	-12(%ebp), %edx
	movl	%edx, (%eax)
L365:
	movl	-12(%ebp), %eax
	movl	$0, 4(%eax)
	movl	$0, (%eax)
	movl	%eax, _thisspec
	movl	$1, %eax
	leal	-8(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 5
LC131:
	.ascii "? DIR: Not enough memory available\12\0"
.globl _getmem
_getmem:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	8(%ebp)
	call	_malloc
	movl	%eax, %edx
	addl	$4, %esp
	testl	%eax, %eax
	jne	L379
	pushl	$__stderr
	pushl	$35
	pushl	$1
	pushl	$LC131
	call	_fwrite
	pushl	$2
	call	_exit
L379:
	movl	_maxmem, %eax
	cmpl	__malloc_amount, %eax
	jae	L380
	movl	__malloc_amount, %eax
	movl	%eax, _maxmem
L380:
	movl	%edx, %eax
	movl	%ebp, %esp
	popl	%ebp
	ret
LC132:
	.ascii "; \0"
LC133:
	.ascii "\12? DIR: %s%s%s\12\0"
LC134:
	.ascii "\12? DIR: %s\12\0"
.globl _errormsg
_errormsg:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	subl	$80, %esp
	movl	8(%ebp), %edi
	movl	12(%ebp), %esi
	cmpl	$0, _numfiles
	je	L382
	call	_printfiles
L382:
	call	_pagecheck
	testl	%esi, %esi
	je	L383
	leal	-88(%ebp), %eax
/APP
	pushl %esi
	pushl $3
	pushl %ds
	pushl %eax
	callf _svcSysErrMsg##
/NO_APP
	pushl	%edi
	cmpb	$0, (%edi)
	je	L384
	movl	$LC132, %eax
	jmp	L385
L384:
	movl	$LC86, %eax
L385:
	pushl	%eax
	leal	-88(%ebp), %eax
	pushl	%eax
	pushl	$LC133
	pushl	$__stderr
	call	_fprintf
	addl	$20, %esp
	jmp	L386
L383:
	pushl	%edi
	pushl	$LC134
	pushl	$__stderr
	call	_fprintf
	addl	$12, %esp
L386:
	movb	$1, _errdone
	leal	-8(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
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
_after_time:
	.space 8
.globl _before_time
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
_totallen:
	.space 8
.globl _totalalloc
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
_gtotallen:
	.space 8
.globl _gtotalalloc
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
	.ident	"GCC: (GNU) 3.3.2"
