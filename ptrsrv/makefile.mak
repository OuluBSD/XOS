.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /OD /S \
#		/NM=$(@DPN) $(@DPN)

all:		ptrsrv.run
	@echo All done.

clean:
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

ptrsrv.run:	ptrsrv.obj cmdadd.obj cmdremove.obj cmdpause.obj \
		cmdcontinue.obj cmdcancel.obj cmdname.obj \
		cmdstatus.obj cmdterminate.obj outthread.obj inthread.obj
	xlink xoslib:\xos\defsegs xoslib:\xos\_mainalt ptrsrv cmdadd \
		cmdremove cmdpause cmdcontinue cmdcancel \
		cmdname.obj cmdstatus cmdterminate outthread inthread \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=ptrsrv /m=ptrsrv /sym=ptrsrv
	copy /over ptrsrv.run newsys:ptrsrv.run
	copy /over ptrsrv.sym newsys:ptrsrv.sym

ptrsrv.obj:		ptrsrv.c
cmdadd.obj:		cmdadd.c
cmdremove.obj:		cmdremove.c
cmdpause.obj:		cmdpause.c
cmdcontinue.obj:	cmdcontinue.c
cmdcancel.obj:		cmdcancel.c
cmdname.obj:		cmdname.c
cmdstatus.obj:		cmdstatus.c
cmdterminate.obj:	cmdterminate.c
outthread.obj:		outthread.c
inthread.obj:		inthread.c
