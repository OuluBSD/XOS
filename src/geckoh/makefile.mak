.M86.OBJ:
	XMAC $(@DPN)

.c.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht /nm=$(@DPN) $(@DPN).c

ALL:	geckoh.run
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

geckoh.run:	geckoh.obj address.obj console.obj usbio.obj command.obj \
		expression.obj symbol.obj format.obj response.obj \
		rspstate.obj rsplkelist.obj rspmem.obj rspio.obj rsppci.obj \
		rspgetva.obj rspgetpa.obj rspbp.obj rspwp.obj rspxctrl.obj \
		rsperror.obj instdisp.obj modify.obj utility.obj
	xlink xoslib:\xos\defsegs geckoh address console usbio command \
		expression symbol format response rspstate rsplkelist \
		rspmem rspio rsppci rspgetva rspgetpa rspbp rspwp rspxctrl \
		rsperror instdisp modify utility \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=geckoh /map=geckoh /sym=geckoh
	copy /over geckoh.run newcmd:geckoh.run

geckoh.obj:	geckoh.c
address.obj:	address.m86
console.obj:	console.c
usbio.obj:	usbio.c
command.obj:	command.c
expression.obj:	expression.c
symbol.obj:	symbol.c
format.obj:	format.c
response.obj:	response.c
rspstate.obj:	rspstate.c
rsplkelist.obj:	rsplkelist.c
rspmem.obj:	rspmem.c
rspio.obj:	rspio.c
rsppci.obj:	rsppci.c
rspgetva.obj:	rspgetva.c
rspgetpa.obj:	rspgetpa.c
rspbp.obj:	rspbp.c
rspwp.obj:	rspwp.c
rspxctrl.obj:	rspxctrl.c
rsperror.obj:	rsperror.c
instdisp.obj:	instdisp.c
modify.obj:	modify.c
utility.obj:	utility.c

fixup.run:	fixup.obj
	xlink xoslib:\xos\defsegs fixup \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=fixup /map=fixup /sym=fixup

fixup.obj:	fixup.c

fixup2.run:	fixup2.obj
	xlink xoslib:\xos\defsegs fixup2 \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=fixup2 /map=fixup2 /sym=fixup2

fixup2.obj:	fixup2.c
