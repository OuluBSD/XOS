.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	WCC386 /D1 /I=XOSINC:\WCX\ /I=..\INC\ /ZQ /3S /J /W3 /WE /FP3 /S \
		/OD /NM=$(@DPN) $(@DPN)

all:		xfpsrv.run
	@ECHO All done.

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

xfpsrv.run:	xfpsrv.obj tcp.obj client.obj session.obj file.obj \
		dataxfer.obj addr.obj
	xlink xoslib:\xos\defsegs xoslib:\xos\_mainalt xfpsrv tcp client \
		session file dataxfer addr \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=xfpsrv /m=xfpsrv /sym=xfpsrv
	copy /o xfpsrv.run newsys:xfpsrv.run
	copy /o xfpsrv.sym newsys:xfpsrv.sym

xfpsrv.obj:	xfpsrv.c
tcp.obj:	tcp.c
client.obj:	client.c
session.obj:	session.c
file.obj:	file.c
dataxfer.obj:	dataxfer.c
addr.obj:	addr.m86
