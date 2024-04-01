.m86.obj:
	XMAC $(@DPN)

.c.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /nm=$(@DPN) $(@DPN).c

all:	wplay.run
	@ECHO All done

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


wplay.run:	wplay.obj xossound.obj
	xlink xoslib:\xos\defsegs wplay xossound \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=wplay /map=wplay /sym=wplay
	copy /over wplay.run newcmd:wplay.run

wplay.obj:	wplay.c
xossound.obj:	xossound.c
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /nm=$(@DPN) $(@DPN).c
	copy /o xossound.obj newlib:xos\xossound.obj

vocdmp.run:	vocdmp.obj
	xlink xoslib:\xos\defsegs vocdmp \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=vocdmp /map=vocdmp /sym=vocdmp
	copy /over vocdmp.run newcmd:vocdmp.run

vocdmp.obj:	vocdmp.c

wavdmp.run:	wavdmp.obj
	xlink xoslib:\xos\defsegs wavdmp \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=wavdmp /map=wavdmp /sym=wavdmp
	copy /over wavdmp.run newcmd:wavdmp.run

wavdmp.obj:	wavdmp.c

vocplay.run:	vocplay.obj
	xlink xoslib:\xos\defsegs vocplay \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=vocplay /map=vocplay /sym=vocplay
	copy /over vocplay.run newcmd:vocplay.run

vocplay.obj:	vocplay.c
