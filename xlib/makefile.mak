.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S \
#		/NM=$(@DPN) $(@DPN)

ALL:	XLIB.RUN
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

xlib.run:	xlib.obj passone.obj list.obj passtwo.obj io.obj
	xlink xoslib:\xos\defsegs xlib passone list passtwo io \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=xlib /map=xlib /sym=xlib
	copy /over xlib.run newcmd:xlib.run

xlib.obj:	xlib.c
passone.obj:	passone.c
passtwo.obj:	passtwo.c
List.obj:	list.c
io.obj:		io.c
