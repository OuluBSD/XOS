.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oafrkht /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S \
#		/NM=$(@DPN) $(@DPN)

all:		init.run once.run screen.run server.run login.run logger.run
	@ECHO All done.

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

init.run:	init.obj initsymbreq.obj inittermdata.obj initsesnreq.obj \
		initsession.obj initchild.obj initdata.obj
	xlink xoslib:\xos\defsegs xoslib:\xos\_mainmin init initsymbreq \
		inittermdata initsesnreq initsession initchild initdata \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=init /sym=init /map=init
	copy /over init.run newsys:init.run

init.obj:		init.c
initsymbreq.obj:	initsymbreq.c
inittermdata.obj:	inittermdata.c
initsesnreq.obj:	initsesnreq.c
initsession.obj:	initsession.c
initchild.obj:		initchild.c
initdata.obj:		initdata.c

once.run:	once.obj onceaddr.obj
	xlink xoslib:\xos\defsegs xoslib:\xos\_mainmin once onceaddr \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=once /sym=once /map=once
	copy /over once.run newsys:once.run

once.obj:	once.c
onceaddr.obj:	onceaddr.m86

login.run:	login.obj
	xlink xoslib:\xos\defsegs login \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=login /sym=login /map=login
	copy /over login.run newsys:login.run

login.obj:	login.c

screen.run:	screen.obj
	xlink screen xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=screen /m=screen /sym=screen
	copy /over screen.run newsys:screen.run

screen.obj:	screen.m86

server.run: 	server.obj
	xlink xoslib:\xos\defsegs xoslib:\xos\_mainalt server \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=server /m=server /sym=server
	copy /over server.run newcmd:server.run

server.obj:	server.c

unspool.run:	unspool.obj
	xlink xoslib:\xos\defsegs unspool \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=unspool /sym=unspool /map=unspool
	copy /over unspool.run newsys:unspool.run

unspool.obj:	unspool.c

logger.run:	logger.obj
	xlink xoslib:\xos\defsegs xoslib:\xos\_mainalt logger \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=logger /m=logger /sym=logger
	copy /over logger.run newsys:logger.run

logger.obj:	logger.c

killlog.run:	killlog.obj
	xlink xoslib:\xos\defsegs killlog \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=killlog /sym=killlog /map=killlog
	copy /over killlog.run newsys:killlog.run

killlog.obj:	killlog.c
