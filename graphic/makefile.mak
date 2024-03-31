.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S \
#		/NM=$(@DPN) $(@DPN)

all:		gphvesa.run
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

gphintel.run:	gphintel.obj asmintel.obj
	xlink xoslib:\xos\highsegs xoslib:\xos\_mainalt gphintel asmintel \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=gphintel /sym=gphintel /map=gphintel
	copy /over gphintel.run newsys:gphintel.run

gphintel.obj:	gphintel.c
asmintel.obj:	asmintel.m86

gphvesa.run:	gphvesa.obj asmvesa.obj
	xlink xoslib:\xos\highsegs xoslib:\xos\_mainmin gphvesa asmvesa \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=gphvesa /sym=gphvesa /map=gphvesa
	copy /over gphvesa.run newsys:gphvesa.run

gphvesa.obj:	gphvesa.c
asmvesa.obj:	asmvesa.m86
