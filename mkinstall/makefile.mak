.m86.obj:
	XMAC $(@DPN)

.c.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht /nm=$(@DPN) $(@DPN).c

all:	mkinstall.run chkinstall.run
	@echo Done

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

mkinstall.run:	mkinstall.obj readins.obj findfiles.obj storeitems.obj \
		crc32.obj
	xlink xoslib:\xos\defsegs mkinstall readins findfiles storeitems \
		crc32 \
		xoslib:\xos\libz123 xoslib:\xos\libz123 \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=mkinstall /map=mkinstall /sym=mkinstall
	copy /over mkinstall.run newcmd:mkinstall.run

mkinstall.obj:	mkinstall.c
readins.obj:	readins.c
findfiles.obj:	findfiles.c
storeitems.obj:	storeitems.c
crc32.obj:	crc32.c

chkinstall.run:	chkinstall.obj crc32.obj
	xlink xoslib:\xos\defsegs chkinstall crc32 \
		xoslib:\xos\libz123 xoslib:\xos\libz123 \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=chkinstall /map=chkinstall /sym=chkinstall
	copy /over chkinstall.run newcmd:chkinstall.run

chkinstall.obj:	chkinstall.c

dmpinstall.run:	dmpinstall.obj crc32.obj
	xlink xoslib:\xos\defsegs dmpinstall crc32 \
		xoslib:\xos\libz123 xoslib:\xos\libz123 \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=dmpinstall /map=dmpinstall /sym=dmpinstall
	copy /over dmpinstall.run newcmd:dmpinstall.run

dmpinstall.obj:	dmpinstall.c
