CX86 = /3S
#CX86 = /5S

FPMODE = /fpi87
#FPMODE = /fp3

.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	WCC386 /D1 /MS /FP5 /7 /I=XOSINC:\WCX\ /NM=$(@DPN) $(@DPN)

.cpp.obj:
	WPP386 /D1 /I=XOSINC:\WCX\ /ZQ $(CX86) /J /W3 $(FPMODE) /S \
	/NM=$(@DPN) $(@DPN)

djpeg.run:	cdjpeg.obj djpeg.obj rdcolmap.obj wrbmp.obj wrgif.obj \
		wrppm.obj wrrle.obj wrtarga.obj sscanf.obj
	XLINK XOSLIB:\XOS\DEFSEGS cdjpeg djpeg rdcolmap wrbmp wrgif \
		wrppm wrrle wrtarga sscanf XOSLIB:\XOS\LIBJPEG6B\
		XOSLIB:\XOS\LIBJPEG6B\
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=djpeg /MAP=djpeg /SYM=djpeg
	COPY /OVER djpeg.run XOSCMD:djpeg.run

cdjpeg.obj:	cdjpeg.c
djpeg.obj:	djpeg.c
rdcolmap.obj:	rdcolmap.c
wrbmp.obj:	wrbmp.c
wrgif.obj:	wrgif.c
wrppm.obj:	wrppm.c
wrrle.obj:	wrrle.c
wrtarga.obj:	wrtarga.c
sscanf.obj:	sscanf.c
