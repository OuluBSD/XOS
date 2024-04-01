.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zl /zld /zq /5s /j /w3 /we /fp3 /s \
		/zl /zld /mf /zc /ze /of /okhs /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S \
#		/NM=$(@DPN) $(@DPN)

all:	xxmac.run  objdmp.run
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

xxmac.run:	xmac.obj asmline.obj assemble.obj cond.obj\
		cref.obj exprsn.obj finish.obj io.obj list.obj listfin.obj\
		macro.obj opin.obj opinf.obj optable.obj pseudo.obj\
		pstable.obj codelist.obj binary.obj nonport.obj utility.obj
	xlink xoslib:\xos\defsegs xmac asmline assemble cond cref \
		exprsn finish io list listfin macro opin opinf optable \
		pseudo pstable codelist binary nonport utility \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=xxmac /map=xxmac /sym=xxmac
	copy /over xxmac.run newcmd:xmac.run

xmac.obj:	xmac.c
asmline.obj:	asmline.c
assemble.obj:	assemble.c
cond.obj:	cond.c
cref.obj:	cref.c
exprsn.obj:	exprsn.c
finish.obj:	finish.c
io.obj:		io.c
list.obj:	list.c
listfin.obj:	listfin.c
binary.obj:	binary.c
macro.obj:	macro.c
opin.obj:	opin.c
opinf.obj:	opinf.c
optable.obj:	optable.c
pseudo.obj:	pseudo.c
pstable.obj:	pstable.c
codelist.obj:	codelist.c
nonport.obj:	nonport.m86
utility.obj:	utility.c

objdmp.run:	objdmp.obj
	xlink xoslib:\xos\defsegs objdmp \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/out=objdmp /map=objdmp /sym=objdmp
	copy /over objdmp.run newcmd:objdmp.run

objdmp.obj:	objdmp.c
