MV=4
NV=5
EN=0
VENDOR=SYSTEM

.m86.obj:
	xmac $(@DPN)

.c.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht \
		/dMV=$(MV) /dNV=$(NV) /dEN=$(EN) /nm=$(@DPN) $(@DPN).c

all:		install.run
	@ECHO All done.

clean:
	-delete *.bak
	-delete *.dmp
	-delete *.err
	-delete *.exe
	-delete *.lke
	-delete *.lst
	-delete *.map
	-delete *.obj
	-delete *.rst
	-delete *.run
	-delete *.sym
	-delete *.tmp

install.run:	install.obj work.obj doscan.obj doscanwork.obj \
		doinst.obj doinstwork.obj doinit.obj doinitwork.obj \
		state.obj error.obj question.obj debug.obj crc32.obj
	xlink newlib:\xos\defsegs /imp=svc /imp=sys \
		install work doscan doscanwork \
		doinst doinstwork doinit doinitwork state error \
		question debug crc32 \
		newlib:\xos\mkbootf \
		newlib:\xos\diskutilfuncs \
		newlib:\xos\libformat \
		newlib:\xos\libxws03 \
		newlib:\xos\libjpeg6b newlib:\xos\libjpeg6b \
		newlib:\xos\libgif30 \
		newlib:\xos\libft201 newlib:\xos\libft201 \
		newlib:\xos\libz123 xoslib:\xos\libz123 \
		newlib:\xos\libx01 \
		newlib:\xos\libc01 \
		/majvrsn=$(MV) /minvrsn=$(NV) /editnum=$(EN) \
		/vendor=$(VENDOR) \
		/output=install /map=install /sym=install
	copy /over install.run newcmd:install.run

install.obj:	install.c
work.obj:	work.c
doscan.obj:	doscan.c
doscanwork.obj:	doscanwork.c
doinst.obj:	doinst.c
doinstwork.obj:	doinstwork.c
doinit.obj:	doinit.c
doinitwork.obj:	doinitwork.c
state.obj:	state.c
error.obj:	error.c
question.obj:	question.c
debug.obj:	debug.c
crc32.obj:	crc32.c
