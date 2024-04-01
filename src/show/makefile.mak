.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht /nm=$(@DPN) $(@DPN).c

#	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S \
#		/NM=$(@DPN) $(@DPN)

ALL:	show.run netshow.run
	@ECHO Finished.

CLEAN:
	-DELETE *.BAK
	-DELETE *.COM
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

show.run:	show.obj dcsinfo.obj devinfo.obj lkeinfo.obj \
		diskinfo.obj intinfo.obj ioinfo.obj crshinfo.obj pciinfo.obj
	xlink xoslib:\xos\defsegs show dcsinfo devinfo lkeinfo \
		diskinfo intinfo ioinfo crshinfo pciinfo \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=show /map=show /sym=show
	copy /over show.run newcmd:show.run

show.obj:	show.c
dcsinfo.obj:	dcsinfo.c
devinfo.obj:	devinfo.c
lkeinfo.obj:	lkeinfo.c
diskinfo.obj:	diskinfo.c
intinfo.obj:	intinfo.c
ioinfo.obj:	ioinfo.c
crshinfo.obj:	crshinfo.c
pciinfo.obj:	pciinfo.c

netshow.run:	netshow.obj nsroute.obj nsdns.obj
	xlink xoslib:\xos\defsegs netshow nsroute nsdns \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=netshow /map=netshow /sym=netshow
	copy /over netshow.run newcmd:netshow.run

netshow.obj:	netshow.c
nsroute.obj:	nsroute.c
nsdns.obj:	nsdns.c
