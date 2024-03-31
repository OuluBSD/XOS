
.m86.obj:
	xmac $(@DPN)

.c.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/ze /oarfkht /dWIN32 /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCX\ /J /W3 /WE /FP3 /S /ZQ /3S \
#		/NM=$(@DPN) $(@DPN)

all:	libz123.obj
	@ECHO All done

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

libz123.obj:	adler32.obj compress.obj crc32.obj deflate.obj gzio.obj \
		infback.obj inffast.obj inflate.obj inftrees.obj \
		trees.obj uncompr.obj zutil.obj
	xlib libz123/create/list=libz123 \
		adler32 compress crc32 deflate gzio infback inffast \
		inflate inftrees trees uncompr zutil
	copy /over libZ123.obj newlib:\xos\libz123.obj

adler32.obj:	adler32.c
compress.obj:	compress.c
crc32.obj:	crc32.c
deflate.obj:	deflate.c
gzio.obj:	gzio.c
infback.obj:	infback.c
inffast.obj:	inffast.c
inflate.obj:	inflate.c
inftrees.obj:	inftrees.c
trees.obj:	trees.c
uncompr.obj:	uncompr.c
zutil.obj:	zutil.c


example.run:	example.obj
	xlink xoslib:\xos\defsegs example \
		libz123 libz123 libz123 xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=example /map=example /sym=example
	copy /over example.run newcmd:example.run

example.obj:	example.c

test1.run:	test1.obj
	xlink xoslib:\xos\defsegs test1 \
		libz123 libz123 libz123 xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=test1 /map=test1 /sym=test1
	copy /over test1.run newcmd:test1.run

test1.obj:	test1.c

