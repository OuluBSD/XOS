CX86 = /3S
#CX86 = /5S

FPMODE = /fpi87
#FPMODE = /fp3

.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zl /zld /zq /zp4 /5s /j /zpw /w3 \
		/fpi87 /fp5 /s /zl /zld /mf /zc /ze /of /okhs /nm=$(@N) \
		$(*DPN)
#	WCC386 /D1 /MS /7 /FP5 /I=XOSINC:\WCX\ /NM=$(@DPN) $(@DPN)

.cpp.obj:
	WPP386 /D1 /I=XOSINC:\WCX\ /ZQ $(CX86) /J /W3 $(FPMODE) /S \
	/NM=$(@DPN) $(@DPN)

libjpeg.obj:	cdjpeg.obj djpeg.obj jcapimin.obj jcapistd.obj jccoefct.obj \
		jccolor.obj jcdctmgr.obj jchuff.obj jcinit.obj jcmainct.obj \
		jcmarker.obj jcmaster.obj jcomapi.obj jcparam.obj \
		jcphuff.obj jcprepct.obj jcsample.obj jctrans.obj \
		jdapimin.obj jdapistd.obj jdatadst.obj jdatasrc.obj \
		jdcoefct.obj jdcolor.obj jddctmgr.obj jdhuff.obj jdinput.obj \
		jdmainct.obj jdmarker.obj jdmaster.obj jdmerge.obj \
		jdphuff.obj jdpostct.obj jdsample.obj jdtrans.obj jerror.obj \
		jfdctflt.obj jfdctfst.obj jfdctint.obj jidctflt.obj \
		jidctfst.obj jidctint.obj jmemmgr.obj \
		jmemnobs.obj jutils.obj
	xlib libjpeg6b/create/list=libjpeg6b \
		cdjpeg djpeg jcapimin jcapistd jccoefct jccolor jcdctmgr \
		jchuff jcinit jcmainct jcmarker jcmaster jcomapi jcparam \
		jcphuff jcprepct jcsample jctrans jdapimin jdapistd \
		jdatadst jdatasrc jdcoefct jdcolor jddctmgr jdhuff \
		jdinput jdmainct jdmarker jdmaster jdmerge jdphuff \
		jdpostct jdsample jdtrans jerror jfdctflt jfdctfst \
		jfdctint jidctflt jidctfst jidctint jmemmgr \
		jmemnobs jutils
	copy /over libjpeg6b.obj newlib:\xos\libjpeg6b.obj

cdjpeg.obj:	cdjpeg.c
djpeg.obj:	djpeg.c
jcapimin.obj:	jcapimin.c
jcapistd.obj:	jcapistd.c
jccoefct.obj:	jccoefct.c
jccolor.obj:	jccolor.c
jcdctmgr.obj:	jcdctmgr.c
jchuff.obj:	jchuff.c
jcinit.obj:	jcinit.c
jcmainct.obj:	jcmainct.c
jcmarker.obj:	jcmarker.c
jcmaster.obj:	jcmaster.c
jcomapi.obj:	jcomapi.c
jcparam.obj:	jcparam.c
jcphuff.obj:	jcphuff.c
jcprepct.obj:	jcprepct.c
jcsample.obj:	jcsample.c
jctrans.obj:	jctrans.c
jdapimin.obj:	jdapimin.c
jdapistd.obj:	jdapistd.c
jdatadst.obj:	jdatadst.c
jdatasrc.obj:	jdatasrc.c
jdcoefct.obj:	jdcoefct.c
jdcolor.obj:	jdcolor.c
jddctmgr.obj:	jddctmgr.c
jdhuff.obj:	jdhuff.c
jdinput.obj:	jdinput.c
jdmainct.obj:	jdmainct.c
jdmarker.obj:	jdmarker.c
jdmaster.obj:	jdmaster.c
jdmerge.obj:	jdmerge.c
jdphuff.obj:	jdphuff.c
jdpostct.obj:	jdpostct.c
jdsample.obj:	jdsample.c
jdtrans.obj:	jdtrans.c
jerror.obj:	jerror.c
jfdctflt.obj:	jfdctflt.c
jfdctfst.obj:	jfdctfst.c
jfdctint.obj:	jfdctint.c
jidctflt.obj:	jidctflt.c
jidctfst.obj:	jidctfst.c
jidctint.obj:	jidctint.c
#jidctred.obj:	jidctred.c
jmemmgr.obj:	jmemmgr.c
jmemnobs.obj:	jmemnobs.c
#jquant1.obj:	jquant1.c
#jquant2.obj:	jquant2.c
jutils.obj:	jutils.c
