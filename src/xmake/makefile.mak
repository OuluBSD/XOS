.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /orkht /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S \
#		/NM=$(@DPN) $(@DPN)

all:	xxmake.run
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

xxmake.run:	xmake.obj switch.obj tree.obj file.obj utility.obj xosrun.obj
	xlink xoslib:\xos\defsegs xmake switch tree file utility xosrun \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=xxmake /m=xxmake /sym=xxmake
	copy /over xxmake.run newcmd:xmake.run

xmake.obj:	xmake.c
switch.obj:	switch.c
tree.obj:	tree.c
file.obj:	file.c
utility.obj:	utility.c
xosrun.obj:	xosrun.c
