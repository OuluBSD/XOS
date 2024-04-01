.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	WCC386 /d1 /I=XOSINC:\WCX\ /nm=$(@DPN) $(@DPN)

ALL:		conlnk.lke kbdcls.lke kbdconadev.lke vgaadev.lke \
		vgachk.lke
	@ECHO Finished.

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

conlnk.lke:	conlnk.obj conkey.obj conend.obj
	xlink conlnk conkey conend \
		/o=conlnk.lke /m=conlnk /lke /sym386 /sym=conlnk
	copy /over conlnk.lke newsys:conlnk.lke
	copy /over conlnk.sym newsys:conlnk.sym

conlnk.obj:	conlnk.m86

conkey.obj:	conkey.m86

conend.obj:	conend.m86

kbdcls.lke:	kbdcls.obj
	xlink kbdcls /o=kbdcls.lke /m=kbdcls /lke /sym386 /sym=kbdcls
	copy /over kbdcls.lke newsys:kbdcls.lke
	copy /over kbdcls.sym newsys:kbdcls.sym

kbdcls.obj:	kbdcls.m86

kbdconadev.lke:	kbdconadev.obj
	xlink kbdconadev /o=kbdconadev.lke /m=kbdconadev /lke /sym386 \
		/sym=kbdconadev
	copy /over kbdconadev.lke newsys:kbdconadev.lke
	copy /over kbdconadev.sym newsys:kbdconadev.sym

kbdconadev.obj:	kbdconadev.m86

vgaadev.lke:	vgaadev1.obj vgaadev2.obj vgaadev3.obj vgaadev4.obj \
		vgaadevt.obj
	xlink vgaadev1 vgaadev2 vgaadev3 vgaadev4 vgaadevt \
		/o=vgaadev.lke /m=vgaadev /lke /sym386 /sym=vgaadev
	copy /over vgaadev.lke newsys:vgaadev.lke
	copy /over vgaadev.sym newsys:vgaadev.sym

vgaadev1.obj:	vgaadev1.m86

vgaadev2.obj:	vgaadev2.m86

vgaadev3.obj:	vgaadev3.m86

vgaadev4.obj:	vgaadev4.m86

vgaadevt.obj:	vgaadevt.m86

vgachk.lke:	vgachk.obj
	xlink vgachk /o=vgachk.lke /m=vgachk /lke /sym386 /sym=vgachk
	copy /over vgachk.lke newsys:vgachk.lke
	copy /over vgachk.sym newsys:vgachk.sym

vgachk.obj:	vgachk.m86
