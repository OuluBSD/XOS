
.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht /nm=$(@DPN) $(@DPN).c

ALL:	LIBX01.OBJ
	@ECHO All done

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

libx01.obj:	global.obj threads.obj serverfbr.obj serverfwt.obj \
		srvmain.obj srvdata.obj \
		srvinfo.obj getuser.obj getusersig.obj \
		vfypswd.obj encpswd.obj getgroup.obj reqresp.obj \
		datetimeparse.obj lkeloadf.obj \
		runread.obj fixdrivef.obj rmvdrivef.obj procarg.obj \
		progarg.obj proghelp.obj \
		dirscan.obj seqname.obj \
		brkpnt.obj wildcmp.obj ftpfunc.obj bits.obj aheapsort.obj \
		hourmin2sys.obj xosrandom.obj qdes.obj setupsession.obj \
		socket.obj krandom.obj
	xlib libx01/create/list=libx01 \
		serverfbr srvinfo serverfwt threads global \
		srvmain srvdata \
		getuser getusersig vfypswd encpswd getgroup reqresp \
		datetimeparse lkeloadf \
		runread procarg progarg proghelp \
		fixdrivef rmvdrivef dirscan \
		seqname brkpnt wildcmp ftpfunc bits aheapsort.obj \
		hourmin2sys xosrandom qdes setupsession socket.obj krandom.obj
	copy /over/ver libx01.obj newlib:\xos\libx01.obj

global.obj:		global.c
threads.obj:		threads.m86
serverfbr.obj:		serverfbr.m86 serverfbr.inc
serverfwt.obj:		serverfwt.c
srvinfo.obj:		srvinfo.m86
getuser.obj:		getuser.m86
getusersig.obj:		getusersig.m86
vfypswd.obj:		vfypswd.m86
encpswd.obj:		encpswd.c
getgroup.obj:		getgroup.m86
reqresp.obj:		reqresp.m86
datetimeparse.obj:	datetimeparse.c
fixdrivef.obj:		fixdrivef.c
lkeloadf.obj:		lkeloadf.c
runread.obj:		runread.c
rmvdrivef.obj:		rmvdrivef.c
progarg.obj:		progarg.m86
procarg.obj:		procarg.m86
proghelp.obj:		proghelp.c
dirscan.obj:		dirscan.c
seqname.obj:		seqname.m86
srvmain.obj:		srvmain.m86
srvdata.obj:		srvdata.m86 srvdata.inc
brkpnt.obj:		brkpnt.m86
wildcmp.obj:		wildcmp.m86
ftpfunc.obj:		ftpfunc.c
bits.obj:		bits.m86
aheapsort.obj:		aheapsort.c
hourmin2sys.obj:	hourmin2sys.m86
xosrandom.obj:		xosrandom.c
qdes.obj:		qdes.c
setupsession.obj:	setupsession.c
socket.obj:		socket.c
krandom.obj:		krandom.c
