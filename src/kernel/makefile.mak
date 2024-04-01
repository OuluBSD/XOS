MV=4
NV=5
EN=1
VENDOR=SYSTEM

.M86.OBJ:
	XMAC $(@DPN)

.xc.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /wx /we /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkhs /nt=x_CODE /nd=x /nm=$(@DPN) \
		$(@DPN).xc

.oc.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /wx /we /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarkhs /nt=o_CODE /nd=o /nm=$(@DPN) \
		$(@DPN).oc

.uc.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkhs /nt=u_CODE /nd=u /nm=$(@DPN) \
		$(@DPN).uc

all:	xosx.run xost.run xos.run
	@ECHO Finished.

clean:	-DELETE *.BAK
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

COMDPD=	error.obj debugdisp.obj clock.obj cmos.obj sched1.obj sched2.obj \
	resource.obj terminate.obj spawn.obj signal.obj alarm.obj \
	event.obj critical.obj memory1.obj memory2.obj memory3.obj \
	memoryxfer.obj svc.obj utility.obj lib.obj istable.obj \
	logical.obj system.obj info.obj screen.obj graphic.obj \
	lke.obj ioopen.obj ioclose.obj iohold.obj ioutil1.obj \
	ioutil2.obj ioutil4.obj ioparam.obj ioparam2.obj \
	iochar.obj iochar2.obj iochar3.obj ioqueue.obj \
	iothread.obj iolock.obj iocopy.obj random.obj dma.obj \
	trmcls1.obj trmcls2.obj trmfnc.obj trmgraph.obj trmtbl.obj \
	ipmcls.obj pipecls.obj nullcls.obj usr.obj usrio.obj \
	usrwildcmp.obj \
#	real.obj realbios.obj realdos.obj country.obj \
#	prot.obj protdos.obj usrreal.obj \
#	usrdos1.obj usrdos2.obj usrdos3.obj usrdos4.obj usrbios.obj \
	usrrncmd.obj usrrn.obj usrrnrun.obj usrrndef.obj usrenv.obj \
	usrerrmsg.obj usrerrtbl.obj usrspace.obj usrfmt.obj usralias.obj \
	buglog.obj pdadef.obj onceuser.obj onceprot.obj oncemem.obj \
	onceramdsk.obj oncecon.obj oncechk.obj export.obj

COMLNK=	/majvrsn=$(MV) /minvrsn=$(NV) /editnum=$(EN) /vendor=$(VENDOR) \
	error debugdisp clock cmos sched1 sched2 resource terminate signal \
	spawn alarm event critical memory1 memory2 memory3 memoryxfer svc \
	utility lib istable logical system info screen graphic lke \
	ioopen ioclose iohold ioutil1 ioutil2 ioutil4 ioparam \
	ioparam2 iochar iochar2 iochar3 \
	ioqueue iothread iolock iocopy random dma \
	trmcls1 trmcls2 trmfnc trmgraph trmtbl ipmcls pipecls nullcls \
	usr usrio usrwildcmp \
#	real realbios realdos country prot protdos usrreal \
#	usrdos1 usrdos2 usrdos3 usrdos4 usrbios \
	usrrncmd usrrn usrrnrun usrrndef usrenv usrerrmsg usrerrtbl \
	usrspace usrfmt usralias buglog pdadef export \
	onceuser onceprot oncemem onceramdsk oncecon oncechk \
	/output=$(@DPN) /map=$(@DPN) /alone

xosx.run:	data.obj $(COMDPD) oncereal.obj nogeckot.obj
	xlink data $(COMLNK) oncereal nogeckot \
		xoslib:\xos\geckox /sym386
	copy /over $(@DPN).run newdir:$(@DPN).run

xost.run:	data.obj $(COMDPD) oncereal.obj nogeckox.obj
	xlink data $(COMLNK) oncereal nogeckox newlib:\xos\libgeckot /sym=xost
	copy /over $(@DPN).run newdir:$(@DPN).run
	copy /over $(@DPN).sym newsys:$(@DPN).sym

xosxt.run:	data.obj $(COMDPD) oncereal.obj
	xlink data $(COMLNK) oncereal xoslib:\xos\geckox xoslib:\xos\libgeckot \
		/sym386 /sym=xosxt
	copy /over $(@DPN).run newdir:$(@DPN).run
	copy /over $(@DPN).sym newsys:$(@DPN).sym

xosb.run:	datab.obj $(COMDPD) oncerealb.obj nogeckot.obj nogeckox.obj
	xlink datab $(COMLNK) oncerealb nogeckot nogeckox xoslib:\xos\geckoboot
	copy /over $(@DPN).run newdir:$(@DPN).run

xos.run:	data.obj $(COMDPD) oncereal.obj nogeckot.obj nogeckox.obj
	xlink data $(COMLNK) oncereal nogeckot nogeckox
	copy /over $(@DPN).run newdir:$(@DPN).run

data.obj:	data.m86 svcdef.inc
datab.obj:	data.m86 xosgecko.m86 svcdef.inc
	xmac	xosgecko data /o=datab.obj
error.obj:	error.m86
debugdisp.obj:	debugdisp.m86
clock.obj:	clock.m86
cmos.obj:	cmos.m86
sched1.obj:	sched1.m86
sched2.obj:	sched2.m86
resource.obj:	resource.m86
terminate.obj:	terminate.m86
spawn.obj:	spawn.m86
signal.obj:	signal.m86
alarm.obj:	alarm.m86
event.obj:	event.m86
critical.obj:	critical.m86
memory1.obj:	memory1.m86
memory2.obj:	memory2.m86
memory3.obj:	memory3.m86
memoryxfer.obj:	memoryxfer.m86
svc.obj:	svc.m86 svcdef.inc
utility.obj:	utility.m86
lib.obj:	lib.m86
istable.obj:	istable.m86
logical.obj:	logical.m86
system.obj:	system.m86
info.obj:	info.m86
screen.obj:	screen.m86
graphic.obj:	graphic.m86
lke.obj:	lke.m86
#errmsg.obj:	errmsg.m86 xosinc:\xmac\xoserr.par
usrerrmsg.obj:	usrerrmsg.uc
usrerrtbl.obj:	usrerrtbl.m86 xosinc:\xmac\xoserr.par
ioopen.obj:	ioopen.m86
ioutil1.obj:	ioutil1.m86
ioutil2.obj:	ioutil2.m86
ioutil4.obj:	ioutil4.xc
iochar.obj:	iochar.m86
iochar2.obj:	iochar2.xc
iochar3.obj:	iochar3.xc
ioparam.obj:	ioparam.m86
ioparam2.obj:	ioparam2.xc
ioqueue.obj:	ioqueue.m86
ioclose.obj:	ioclose.xc
iohold.obj:	iohold.m86
iothread.obj:	iothread.m86
iolock.obj:	iolock.m86
iocopy.obj:	iocopy.m86
random.obj:	random.m86
dma.obj:	dma.m86
usrrncmd.obj:	usrrncmd.uc
usrrn.obj:	usrrn.m86
usrrnrun.obj:	usrrnrun.m86
usrrndef.obj:	usrrndef.m86
real.obj:	real.m86
realbios.obj:	realbios.m86
realdos.obj:	realdos.m86
country.obj:	country.m86
trmcls1.obj:	trmcls1.m86
trmcls2.obj:	trmcls2.m86
trmfnc.obj:	trmfnc.m86
trmgraph.obj:	trmgraph.m86
trmtbl.obj:	trmtbl.m86
ipmcls.obj:	ipmcls.xc
pipecls.obj:	pipecls.m86
nullcls.obj:	nullcls.xc
prot.obj:	prot.m86
protdos.obj:	protdos.m86 prot.inc
usr.obj:	usr.m86 svcdef.inc
usrio.obj:	usrio.uc
usrwildcmp.obj: usrwildcmp.m86
usrenv.obj:	usrenv.uc
usrfmt.obj:	usrfmt.m86
usralias.obj:	usralias.m86
usrreal.obj:	usrreal.m86 svcdef.inc
usrdos1.obj:	usrdos1.m86
usrdos2.obj:	usrdos2.m86
usrdos3.obj:	usrdos3.m86
usrdos4.obj:	usrdos4.m86
usrbios.obj:	usrbios.m86
usrspace.obj:	usrspace.m86
onceuser.obj:	onceuser.m86
onceprot.obj:	onceprot.m86
oncemem.obj:	oncemem.m86
onceramdsk.obj:	onceramdsk.m86
oncecon.obj:	oncecon.m86
oncechk.obj:	oncechk.oc
oncelcd.obj:	oncelcd.m86
export.obj:	export.m86 svcdef.inc pdadef.inc
oncereal.obj:	oncereal.m86
oncerealb.obj:	oncereal.m86 xosgecko.m86
	xmac	xosgecko oncereal /o=oncerealb.obj
buglog.obj:	buglog.m86
pdadef.obj:	pdadef.m86 pdadef.inc
nogeckox.obj:	nogeckox.m86
nogeckot.obj:	nogeckot.m86


test.obj:	test.xc
