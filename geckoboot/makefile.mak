.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	WCC386 /d1 /i=XOSINC:\WCX\ /nm=$(@DPN) $(@DPN)

ALL:	geckoboot.obj
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

geckoboot.obj:	geckoboot.m86 geckocmd.m86 help.m86 exprsn.m86 lglio.m86 \
		realio.m86 control.m86 optbl.m86 opin.m86 opout.m86 data.m86
	xmac geckoboot
	copy /over geckoboot.obj xoslib:\xos\geckoboot.obj
