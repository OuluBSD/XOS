.ASM.OBJ:
	MASM $(@dpn).ASM /IP:\MASM /MX /ML /Z /B63,;

.M86.OBJ:
	XMAC $(@dpn)

.c.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S \
#		/NM=$(@DPN) $(@DPN)

vidx.run: vid.obj edit.obj edit2.obj display.obj getkey.obj search.obj \
		utility.obj ibmpc.obj io.obj data.obj
	xlink xoslib:xos\defsegs xoslib:xos\_mainmin vid edit edit2 display \
		getkey search utility ibmpc io data \
		xoslib:xos\libx01 xoslib:xos\libc01 \
		/o=vidx /m=vidx /sym=vidx
	copy /over vidx.run newcmd:vidx.run

vid.obj:	vid.m86
edit.obj: 	edit.m86
edit2.obj:	edit2.c
display.obj:	display.m86
getkey.obj:	getkey.m86
search.obj:	search.m86
utility.obj:	utility.m86
ibmpc.obj:	ibmpc.m86
io.obj:		io.m86
data.obj:	data.m86
