CX86 = /3S
#CX86 = /5S

FPMODE = /fpi87
#FPMODE = /fp3

.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zpw /ze /oarfkht /nm=$(@DPN) $(@DPN).c
#	wcc386 /D1 /I=xosinc:\WCX\ /ZQ /3S /J /W4 /WE /FP3 /S \
#		/OD /7 /FP5 /NM=$(@DPN) $(@DPN)

#	WCC386 /D1 /MS /I=XOSINC:\WCX\ /NM=$(@DPN) $(@DPN)

.cpp.obj:
	WPP386 /D1 /I=XOSINC:\WCX\ /ZQ $(CX86) /J /W3 $(FPMODE) /S \
	/NM=$(@DPN) $(@DPN)

all:	libc01.obj _main.obj _mainalt.obj _mainmin.obj bigsegs.obj
	@echo All done

CLEAN:
	-DELETE *.BAK
	-DELETE *.DMP
	-DELETE *.ERR
	-DELETE *.EXE
	-DELETE *.LKE
	-DELETE *.LST
	-DELETE *.MAP
	-DELETE *.OBJ
	-DELETE *.RST
	-DELETE *.RUN
	-DELETE *.SYM
	-DELETE *.TMP

# Following removed: DATE_PRS.OBJ

libc01.obj:	add2sdt.obj axtoi.obj ddt2str.obj defsegs.obj highsegs.obj \
		errmsg.obj errormsg.obj femsg.obj __get_errno_ptr.obj \
		femsg2.obj fexist.obj fgtfinfo.obj fisctrm.obj getch.obj \
		getche.obj getfarg.obj getfinfo.obj getspace.obj \
		gtrqname.obj heapsort.obj heapsort4.obj isatty.obj \
		isctrm.obj kerdef.obj log.obj longadd.obj longlongadd.obj \
		ulongmuldiv.obj longmul.obj slongdiv.obj longadddiv.obj \
		div64by32.obj nullfunc.obj \
		_splitpath2.obj \
		rotate.obj sbrk.obj sdt2str.obj sdtadd.obj sdtsub.obj \
		sdtcmp.obj signal.obj setvectr.obj _makepath.obj \
		_searchenv.obj stricmp.obj strlower.obj strlwr.obj \
		strmov.obj strnchr.obj strnicmp.obj \
		strnmov.obj strupr.obj strnupr.obj import.obj xstrncmp.obj \
		abs.obj atexit.obj atoi.obj div.obj exit.obj fclose.obj \
		feof.obj ferror.obj fflush.obj \
		fgetstr.obj fgets.obj fgetsl.obj fgetc.obj setvbuf.obj \
		fopen.obj fdopen.obj __fopen.obj lowio.obj \
		fputc.obj fputs.obj fread.obj \
		rewind.obj fseek.obj ftell.obj \
		fwrite.obj getchar.obj setmode.obj getenv.obj \
		stat.obj __xosdt2unixdt.obj open.obj read.obj write.obj \
		close.obj filelength.obj getcwd.obj _fullpath.obj \
		clearerr.obj \
		_cmdname.obj _setmbcp.obj rand.obj wreslang.obj \
		isalnum.obj isalpha.obj \
		iscntrl.obj isdigit.obj isgraph.obj islower.obj isprint.obj \
		ispunct.obj isspace.obj isupper.obj isxdigit.obj \
		malloc.obj stackavail.obj \
		memchr.obj memscan.obj memcmp.obj memicmp.obj memcpy.obj \
		memcpylong.obj memmove.obj memset.obj memsetlong.obj \
		perror.obj mktime.obj \
		printf.obj fprintf.obj sprintf.obj snprintf.obj \
		vprintf.obj vfprintf.obj vsprintf.obj vsnprintf.obj $$fmt.obj \
		putchar.obj puts.obj \
		qsort.obj remove.obj rename.obj \
		setjmp_w.obj setjmp.obj \
		utoa.obj strcat.obj \
		strchr.obj strcmp.obj strcpy.obj strcspn.obj strerror.obj \
		strcmpcs.obj strrev.obj strlen.obj strnlen.obj strncat.obj \
		strncmp.obj strncpy.obj strncpyn.obj strpbrk.obj \
		strrchr.obj strspn.obj strstr.obj strtok.obj \
		strtol.obj strtoul.obj strtoll.obj strtoull.obj $$strtonum.obj \
		system.obj tmpnam.obj tolower.obj toupper.obj \
		ungetc.obj getpid.obj sysconf.obj \
#	times.obj \
		getrlimit.obj setrlimit.obj getrusage.obj gettimeofday.obj \
		time.obj localtime.obj gmtime.obj \
		_assert.obj abort.obj istable.obj _errno.obj __fmode.obj \
		_iob.obj _main.obj stosd.obj _wfloat.obj days2ticks.obj \
		ticks2days.obj sdt2days.obj $$i8shift.obj $$i8mul.obj \
		$$i8div.obj
#	gecko \xossrc\xlib\xlib libc01/create/list=libc01 \
	xlib libc01/create/list=libc01 \
		_main \
		femsg femsg2 __get_errno_ptr \
		errormsg errmsg getspace \
		getenv abs div atoi axtoi log longadd longlongadd \
		ulongmuldiv longmul slongdiv \
		longadddiv div64by32 rotate \
		add2sdt sdt2str ddt2str sdtadd sdtsub \
		sdtcmp utoa exit atexit setjmp_w setjmp \
		system perror qsort mktime \
		_splitpath2 \
		printf fprintf sprintf snprintf vprintf vfprintf \
		vsprintf vsnprintf $$fmt \
		ferror fflush fopen fdopen __fopen setvbuf lowio \
		puts fputs fread getchar SETMODE rewind fseek \
		ftell remove rename tmpnam ungetc getpid sysconf \
#	times \
		getrlimit setrlimit getrusage gettimeofday time localtime \
		gmtime fwrite putchar fclose feof fgets fgetsl fgetstr fgetc \
		fputc fexist fgtfinfo fisctrm getch getche getfarg \
		getfinfo isatty isctrm __fmode stat __xosdt2unixdt \
		open read write close filelength getcwd _fullpath \
		clearerr _cmdname _setmbcp rand wreslang \
		gtrqname signal setvectr \
		strtol strtoul strtoll strtoull $$strtonum \
		_makepath _searchenv stricmp \
		strlower strlwr strmov \
		strnchr strnicmp strnmov strupr strnupr xstrncmp strcat \
		strchr strcmp strcpy strcspn strcmpcs strerror \
		strrev strlen \
		strnlen strncat strncmp strncpy strncpyn strpbrk strrchr \
		strspn strstr stosd _wfloat \
		strtok isalnum isalpha \
		iscntrl isdigit isgraph islower isprint ispunct isspace \
		isupper isxdigit tolower toupper \
		malloc stackavail days2ticks ticks2days sdt2days \
		heapsort heapsort4 \
		lowio \
		sbrk memchr memscan memcmp memicmp memcpy memcpylong \
		memmove memset memsetlong _iob $$i8shift $$i8mul $$i8div \
		kerdef import _assert abort istable _errno nullfunc
	copy /over libc01.obj newlib:\xos\libc01.obj

add2sdt.obj:	add2sdt.m86
axtoi.obj:	axtoi.m86
date_prs.obj:	date_prs.c
ddt2str.obj:	ddt2str.m86

defsegs.obj:	defsegs.m86
	xmac defsegs
	copy /over defsegs.obj newlib:\xos\defsegs.obj

highsegs.obj:	highsegs.m86
	xmac highsegs
	copy /over highsegs.obj newlib:\xos\highsegs.obj

bigsegs.obj:	bigsegs.m86
	xmac bigsegs
	copy /over bigsegs.obj newlib:\xos\bigsegs.obj

errmsg.obj:	errmsg.m86
errormsg.obj:	errormsg.c
femsg.obj:	femsg.m86
femsg2.obj:	femsg2.c
__get_errno_ptr.obj: __get_errno_ptr.c
fexist.obj:	fexist.m86
fgtfinfo.obj:	fgtfinfo.m86
fisctrm.obj:	fisctrm.m86
getch.obj:	getch.c
getche.obj:	getche.c
getfarg.obj:	getfarg.m86
getfinfo.obj:	getfinfo.m86
getspace.obj:	getspace.m86
gtrqname.obj:	gtrqname.m86
heapsort.obj:	heapsort.c
heapsort4.obj:	heapsort4.c
stat.obj:	stat.c
__xosdt2unixdt.obj: __xosdt2unixdt.c
open.obj:	open.c
read.obj:	read.c
write.obj:	write.c
close.obj:	close.c
filelength.obj:	filelength.c
getcwd.obj:	getcwd.c
_fullpath.obj:	_fullpath.c
clearerr.obj:	clearerr.c
_cmdname.obj:	_cmdname.c
_setmbcp.obj:	_setmbcp.c
rand.obj:	rand.c
wreslang.obj:	wreslang.c
isatty.obj:	isatty.c
isctrm.obj:	isctrm.m86
kerdef.obj:	kerdef.m86
log.obj:	log.c
longadd.obj:	longadd.m86
longlongadd.obj: longlongadd.m86
ulongmuldiv.obj: ulongmuldiv.m86
longmul.obj:	longmul.m86
slongdiv.obj:	slongdiv.m86
longadddiv.obj:	longadddiv.m86
div64by32.obj:	div64by32.m86
nullfunc.obj:	nullfunc.m86
rotate.obj:	rotate.m86
sbrk.obj:	sbrk.m86
sdt2str.obj:	sdt2str.m86
sdtadd.obj:	sdtadd.m86
sdtsub.obj:	sdtsub.m86
sdtcmp.obj:	sdtcmp.m86
signal.obj:	signal.c
setvectr.obj:	setvectr.m86
utoa.obj:	utoa.m86
_makepath.obj:	_makepath.c
_searchenv.obj:	_searchenv.c
stricmp.obj:	stricmp.m86
strlower.obj:	strlower.m86
strlwr.obj:	strlwr.m86
strmov.obj:	strmov.m86
strnchr.obj:	strnchr.m86
strnicmp.obj:	strnicmp.m86
strnmov.obj:	strnmov.m86
strupr.obj:	strupr.m86
strnupr.obj:	strnupr.m86
import.obj:	import.m86
xstrncmp.obj:	xstrncmp.m86
days2ticks.obj:	days2ticks.m86
ticks2days.obj:	ticks2days.m86
sdt2days.obj:	sdt2days.m86

_main.obj:      _main.m86
	xmac _main
	copy /over _main.obj newlib:\xos\_main.obj

_mainalt.obj:	_mainalt.m86
	xmac _mainalt
	copy /over _mainalt.obj newlib:\xos\_mainalt.obj

_mainmin.obj:	_mainmin.m86
	xmac _mainmin
	copy /over _mainmin.obj newlib:\xos\_mainmin.obj

_mcpp.obj:	_mcpp.m86
	xmac _mcpp
	copy /over _mcpp.obj newlib:\xos\_mcpp.obj

abs.obj:        abs.m86
atexit.obj:     atexit.c
atoi.obj:       atoi.m86
div.obj:        div.m86

#ecvt.obj:	ecvt.c
#	wcc386 /3S /FPI87 /D1 /MS /I=XOSINC:\wcx\ /NM=$(@DPN) $(@DPN)

exit.obj:       exit.c
fclose.obj:     fclose.c
feof.obj:       feof.c
ferror.obj:     ferror.m86
fflush.obj:     fflush.c
fgets.obj:      fgets.m86
fgetsl.obj:	fgetsl.m86
fgetstr.obj:	fgetstr.m86
fgetc.obj:      fgetc.c
setvbuf.obj:	setvbuf.c
fopen.obj:      fopen.c
fdopen.obj:	fdopen.c
__fopen.obj:	__fopen.c
lowio.obj:	lowio.c
fputc.obj:      fputc.c
fputs.obj:      fputs.m86
fread.obj:      fread.m86
rewind.obj:	rewind.c
fseek.obj:      fseek.c
ftell.obj:      ftell.c
fwrite.obj:     fwrite.m86
getchar.obj:	getchar.m86
setmode.obj:	setmode.c
getenv.obj:     getenv.m86
isalnum.obj:    isalnum.m86
isalpha.obj:    isalpha.m86
iscntrl.obj:    iscntrl.m86
isdigit.obj:    isdigit.m86
isgraph.obj:    isgraph.m86
islower.obj:    islower.m86
isprint.obj:    isprint.m86
ispunct.obj:    ispunct.m86
isspace.obj:    isspace.m86
isupper.obj:    isupper.m86
isxdigit.obj:   isxdigit.m86
malloc.obj:     malloc.m86
stackavail.obj:	stackavail.m86
memchr.obj:     memchr.m86
memscan.obj:	memscan.m86
memcmp.obj:     memcmp.m86
memicmp.obj:	memicmp.m86
memcpy.obj:     memcpy.m86
memcpylong.obj:	memcpylong.m86
memmove.obj:    memmove.m86
memset.obj:     memset.m86
memsetlong.obj:	memsetlong.m86
perror.obj:     perror.m86
mktime.obj:	mktime.c
_splitpath2.obj: _splitpath2.c
printf.obj:	printf.m86
fprintf.obj:	fprintf.m86
sprintf.obj:	sprintf.m86
snprintf.obj:	snprintf.m86
vprintf.obj:	vprintf.m86
vfprintf.obj:	vfprintf.m86
vsprintf.obj:	vsprintf.m86
vsnprintf.obj:	vsnprintf.m86
printfx.obj:    printfx.m86
	xmac $(@DPN)
	copy /over printfx.obj newlib:\xos\printfx.obj


putchar.obj:	putchar.m86
puts.obj:       puts.m86
qsort.obj:	qsort.c
remove.obj:     remove.m86
rename.obj:     rename.m86
setjmp.obj:     setjmp.m86
setjmp_w.obj:	setjmp_w.m86
strcat.obj:     strcat.m86
strchr.obj:     strchr.m86
strcmp.obj:     strcmp.m86
strcpy.obj:     strcpy.m86
strcspn.obj:    strcspn.m86
strcmpcs.obj:	strcmpcs.m86
strerror.obj:   strerror.m86
strrev.obj:	strrev.m86
strlen.obj:     strlen.m86
strnlen.obj:	strnlen.m86
strncat.obj:    strncat.m86
strncmp.obj:    strncmp.m86
strncpy.obj:    strncpy.m86
strncpyn.obj:	strncpyn.m86
strpbrk.obj:    strpbrk.m86
strrchr.obj:    strrchr.m86
strspn.obj:     strspn.m86
strstr.obj:     strstr.m86
strtok.obj:     strtok.m86
strtol.obj:     strtol.m86
strtoul.obj:    strtoul.m86
strtoll.obj:	strtoll.m86
strtoull.obj:	strtoull.m86
$$strtonum.oBJ:	$$strtonum.m86
system.obj:     system.m86
tmpnam.obj:     tmpnam.m86
tolower.obj:    tolower.m86
toupper.obj:    toupper.m86
ungetc.obj:     ungetc.c
getpid.obj:	getpid.c
sysconf.obj:	sysconf.c
###times.obj:	times.c
getrlimit.obj:	getrlimit.c
setrlimit.obj:	setrlimit.c
getrusage.obj:	getrusage.c
gettimeofday.obj: gettimeofday.m86
time.obj:	time.m86
localtime.obj:	localtime.m86
gmtime.obj:	gmtime.c
$$fmt.obj:	$$fmt.m86
_assert.obj:    _assert.m86
abort.obj:	abort.c
istable.obj:	istable.m86
_errno.obj:     _errno.m86
__fmode.obj:	__fmode.m86
_iob.obj:       _iob.m86
$$i8shift.obj:	$$i8shift.m86
$$i8mul.obj:	$$i8mul.m86
$$i8div.obj:	$$i8div.m86
_wfloat.obj:	_wfloat.m86
STOSD.OBJ:	STOSD.M86

#timecvt.obj:	timecvt.cpp

testqsort.run:	testqsort.obj
	xlink xoslib:\xos\defsegs testqsort \
		xoslib:\xos\libx01 \
		xoslib:\xos\libc01 \
		/output=testqsort /map=testqsort /sym=testqsort
#	copy /over testqsort.run newcmd:testqsort.run
	@echo Done testqsort.run

testqsort.obj:	testqsort.c

test.run:	test.obj
	xlink xoslib:\xos\defsegs test \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/OUTPUT=test /MAP=test /SYM=test
#	COPY /OVER test.RUN NEWCMD:test.RUN
	@echo Done test.run

test.obj:	test.c libc01.obj
	owxc /d1 /i=xosinc:\wcx\ /zq /5s /j /w3 /we /fp3 /s /mf /zc \
		/ze /of /okht /nm=$(@DPN) $(@DPN).c
