.M86.OBJ:
	XMAC $(@dpn)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S /E1 \
#		/NM=$(@DPN) $(@DPN)

all:	boot.run bootx.run mkboot.run rmboot.run
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

boot.run:	header.obj level1.obj level2.obj fat.obj xfs.obj
	xlink /nocheck header level1 level2 fat xfs /o=boot /m=boot /alone
	copy /over boot.run newsys:boot.run

header.obj:	header.m86
level1.obj:	level1.m86
level2.obj:	level2.m86
#	xmac level2 /l=level2
fat.obj:	fat.m86
#	xmac fat /l=fat
xfs.obj:	xfs.m86

bootx.run:	header.obj level1.obj level2x.obj fat.obj xfs.obj
	xlink /nocheck header level1 level2x fat xfs xoslib:\xos\geckoboot \
		/o=bootx /m=bootx /alone
	copy /over bootx.run newsys:bootx.run

level2x.obj:	level2.m86 debug.m86
	xmac debug level2 /l=level2x /o=level2x

mkboot.run:	mkboot.obj mkbootf.obj
	xlink xoslib:\xos\defsegs mkboot mkbootf \
	xoslib:\xos\libx01 xoslib:\xos\libc01 \
	/o=mkboot /m=mkboot /sym=mkboot
	copy /over mkboot.run newcmd:mkboot.run

mkboot.obj:	mkboot.c

mkbootf.obj:	mkbootf.c
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /nm=mkbootf mkbootf
	copy /over mkbootf.obj xoslib:xos\mkbootf.obj

rmboot.run:	rmboot.obj rmbootf.obj
	xlink xoslib:\xos\defsegs rmboot rmbootf \
	xoslib:\xos\libx01 xoslib:\xos\libc01 \
	/o=rmboot /m=rmboot /sym=rmboot
	copy /over rmboot.run newcmd:rmboot.run

rmboot.obj:	rmboot.c

rmbootf.obj:	rmbootf.c
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /nm=rmbootf rmbootf
	copy /over rmbootf.obj xoslib:xos\rmbootf.obj

dmpvect.run:	dmpvect.obj
	xlink dmpvect /o=dmpvect /m=dmpvect /alone

dmpvect.obj:	dmpvect.m86
