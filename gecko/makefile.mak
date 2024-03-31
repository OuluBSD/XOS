.M86.OBJ:
	XMAC $(@DPN)

.c.obj:
	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S /OD \
		/NM=$(@DPN) $(@DPN)

ALL:	GECKO.RUN 
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

gecko.run:	gecko.obj command.obj help.obj exprsn.obj lglio.obj \
		userio.obj screen.obj f8x8.obj region.obj control.obj \
		optbl.obj opout.obj data.obj
	xlink gecko command help exprsn lglio userio screen f8x8 \
		region control optbl opout data \
		xoslib:\xos\libc01 /map=gecko /sym=gecko.sym
	copy /over gecko.run newcmd:gecko.run

gecko.obj:	gecko.m86
command.obj:	command.m86
help.obj:	help.m86
exprsn.obj:	exprsn.m86
lglio.obj:	lglio.m86
userio.obj:	userio.m86
screen.obj:	screen.m86
f8x8.obj:	f8x8.m86
region.obj:	region.m86
control.obj:	control.m86
optbl.obj:	optbl.m86
opout.obj:	opout.m86
data.obj:	data.m86
