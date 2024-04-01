.m86.obj:
	XMAC $(@DPN)

.c.obj:
	owxc /hc /d1+ /i=xosinc:\owxc\ /zq /6s /w3 /we \
		/fpi87 /fp3 /s /mf /zc /zp4 /zpw /za99 /ze /oarkhs \
		/nm=$(@DPN) $(@DPN).c

#	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
#		/zp4 /zpw /za99 /ze /oarfkht /nm=$(@DPN) $(@DPN).c

all:	addunit.run \
#	attrib.run \
	bincom.run clschar.run color.run config.run copyx.run \
	crshsave.run daytime.run dateutil.run fragment.run \
	default.run deletex.run delay.run \
	devchar.run dirx.run display.run fixdrive.run rmvdrive.run \
	dosquirk.run echox.run killproc.run btblkdmp.run \
#	label.run \
	lkeload.run logical.run \
	mkdir.run path.run rename.run rmdir.run \
#	set.run setenv.run prompt.run \
	symbiont.run session.run syschar.run dump.run \
	sysdis.run \
#	type.run \
	xgrep.run dump.run syslog.run \
#	undump.run \
	dismount.run diskinst.run coninst.run menusel.run
	@ECHO Finished.

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

addunit.run:	addunit.obj
	xlink newlib:\xos\defsegs addunit \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=addunit /map=addunit /sym=addunit
	copy /over addunit.run newcmd:addunit.run

addunit.obj:	addunit.c

attrib.run:	attrib.obj
	xlink newlib:\xos\defsegs attrib \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=attrib /map=attrib /sym=attrib
	copy /over attrib.run newcmd:attrib.run

attrib.obj:	attrib.c

bincom.run:	bincom.obj
	xlink newlib:\xos\defsegs bincom \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=bincom /map=bincom /sym=bincom
	copy /over bincom.run newcmd:bincom.run

bincom.obj:	bincom.c

btblkdmp.run:	btblkdmp.obj
	xlink newlib:\xos\defsegs btblkdmp \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=btblkdmp /map=btblkdmp /sym=btblkdmp
	copy /over btblkdmp.run newcmd:btblkdmp.run

btblkdmp.obj:	btblkdmp.c

clschar.run:	clschar.obj
	xlink newlib:\xos\defsegs clschar \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=clschar /map=clschar /sym=clschar
	copy /over clschar.run newcmd:clschar.run

clschar.obj:	devchar.c
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht /nm=clschar /fo=clschar.obj \
		/dclschar devchar.c

color.run:	color.obj
	xlink newlib:\xos\defsegs color \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=color /map=color /sym=color
	copy /over color.run newcmd:color.run

color.obj:	color.c

config.run:	config.obj
	xlink newlib:\xos\defsegs config \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=config /map=config /sym=config
	copy /over config.run newcmd:config.run

config.obj:	config.c

copyx.run:	copy.obj
	xlink newlib:\xos\defsegs copy \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=copyx /map=copyx /sym=copyx
	copy /over copyx.run newcmd:copy.run
	copy /over copyx.sym newcmd:copy.sym

copy.obj:	copy.c

crshsave.run:	crshsave.obj
	xlink newlib:\xos\defsegs crshsave \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=crshsave /map=crshsave /sym=crshsave
	copy /over crshsave.run newcmd:crshsave.run

crshsave.obj:	crshsave.c

daytime.run:	daytime.obj
	xlink newlib:\xos\defsegs daytime \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=daytime /map=daytime /sym=daytime
	copy /over daytime.run newcmd:daytime.run

daytime.obj:	daytime.c

default.run:	default.obj
	xlink newlib:\xos\defsegs default \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=default /map=default /sym=default
	copy /over default.run newcmd:default.run

default.obj:	default.c

deletex.run:	delete.obj
	xlink newlib:\xos\defsegs delete \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=deletex /map=deletex /sym=deletex
	copy /over deletex.run newcmd:delete.run
	copy /over deletex.run newcmd:del.run

delete.obj:	delete.c

devchar.run:	devchar.obj
	xlink newlib:\xos\defsegs devchar \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=devchar /map=devchar /sym=devchar
	copy /over devchar.run newcmd:devchar.run

devchar.obj:	devchar.c
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht /nm=devchar /fo=devchar.obj \
		/ddevchar devchar.c

dirx.run:	dir.obj
	xlink newlib:\xos\defsegs dir \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=dirx /map=dirx /sym=dirx
	copy /over dirx.run newcmd:dir.run

dir.obj:	dir.c

xgrep.run:	xgrep.obj
	xlink newlib:\xos\defsegs xgrep \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=xgrep /map=xgrep /sym=xgrep
	copy /over xgrep.run newcmd:xgrep.run

xgrep.obj:	xgrep.c

dateutil.run:	dateutil.obj
	xlink newlib:\xos\defsegs dateutil \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=dateutil /map=dateutil /sym=dateutil
	copy /over dateutil.run newcmd:dateutil.run

dateutil.obj:	dateutil.c

fragment.run:	fragment.obj
	xlink newlib:\xos\defsegs fragment \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=fragment /map=fragment /sym=fragment
	copy /over fragment.run newcmd:fragment.run

fragment.obj:	fragment.c

display.run:	display.obj
	xlink newlib:\xos\defsegs display \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=display /map=display /sym=display
	copy /over display.run newcmd:display.run

display.obj:	display.c

fixdrive.run:	fixdrive.obj
	xlink newlib:\xos\defsegs fixdrive \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=fixdrive /map=fixdrive /sym=fixdrive
	copy /over fixdrive.run newcmd:fixdrive.run

fixdrive.obj:	fixdrive.c

rmvdrive.run:	rmvdrive.obj
	xlink newlib:\xos\defsegs rmvdrive \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=rmvdrive /map=rmvdrive /sym=rmvdrive
	copy /over rmvdrive.run newcmd:rmvdrive.run

rmvdrive.obj:	rmvdrive.c

doslpt.run:	doslpt.obj
	xlink newlib:\xos\defsegs doslpt \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=doslpt /map=doslpt /sym=doslpt
	copy /over doslpt.run newcmd:doslpt.run

doslpt.obj:	doslpt.c

dosquirk.run:	dosquirk.obj
	xlink newlib:\xos\defsegs dosquirk \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=dosquirk /map=dosquirk /sym=dosquirk
	copy /over dosquirk.run newcmd:dosquirk.run

dosquirk.obj:	dosquirk.c

dump.run:	dump.obj
	xlink newlib:\xos\defsegs dump \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=dump /map=dump /sym=dump
	copy /over dump.run newcmd:dump.run

dump.obj:	dump.c

envtest.run:	envtest.obj
	xlink newlib:\xos\defsegs envtest \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=envtest /map=envtest /sym=envtest
	copy /over envtest.run newcmd:envtest.run

envtest.obj:	envtest.c

iotest.run:	iotest.obj
	xlink newlib:\xos\defsegs iotest \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=iotest /map=iotest /sym=iotest
	copy /over iotest.run newcmd:iotest.run

iotest.obj:	iotest.c

filldir.run:	filldir.obj
	xlink newlib:\xos\defsegs filldir \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=filldir /map=filldir /sym=filldir
	copy /over filldir.run newcmd:filldir.run

filldir.obj:	filldir.c

undump.run:	undump.obj
	xlink newlib:\xos\defsegs undump \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=undump /map=undump /sym=undump
	copy /over undump.run newcmd:undump.run

undump.obj:	undump.c

timezone.run:	timezone.obj
	xlink newlib:\xos\defsegs timezone \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=timezone /map=timezone /sym=timezone
	copy /over timezone.run newcmd:timezone.run

timezone.obj:	timezone.c

echox.run:	echo.obj
	xlink newlib:\xos\defsegs echo \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=echox /map=echox /sym=echox
	copy /over echox.run newcmd:echo.run

echo.obj:	echo.c

killproc.run:    killproc.obj
	xlink newlib:\xos\defsegs killproc \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=killproc /map=killproc /sym=killproc
	copy /over killproc.run newcmd:killproc.run

killproc.obj:	killproc.c

label.run:	label.obj
	xlink newlib:\xos\defsegs label \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=label /map=label /sym=label
	copy /over label.run newcmd:label.run

label.obj:	label.c

lkeload.run:	lkeload.obj
	xlink newlib:\xos\defsegs lkeload \
		newlib:\xos\_mainmin newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=lkeload /map=lkeload /sym=lkeload
	copy /over lkeload.run newcmd:lkeload.run

lkeload.obj:	lkeload.c

exportdmp.run:	exportdmp.obj
	xlink newlib:\xos\defsegs exportdmp \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=exportdmp /map=exportdmp /sym=exportdmp
	copy /over exportdmp.run newcmd:exportdmp.run

exportdmp.obj:	exportdmp.c

logical.run:	logical.obj
	xlink newlib:\xos\defsegs logical \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=logical /map=logical /sym=logical
	copy /over logical.run newcmd:logical.run

logical.obj:	logical.c

mkdir.run:	mkdir.obj
	xlink newlib:\xos\defsegs mkdir \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=mkdir /map=mkdir /sym=mkdir
	copy /over mkdir.run newcmd:mkdir.run

mkdir.obj:	mkdir.c

more.run:	more.obj
	xlink newlib:\xos\defsegs more \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=more /map=more /sym=more
	copy /over more.run newcmd:more.run

path.run:	path.obj
	xlink newlib:\xos\defsegs path \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=path /map=path /sym=path
	copy /over path.run newcmd:path.run

path.obj:	path.c

prompt.run:	prompt.obj
	xlink newlib:\xos\defsegs prompt \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=prompt /map=prompt /sym=prompt
	copy /over prompt.run newcmd:prompt.run

prompt.obj:	prompt.c

protfs.run:	protfs.obj
	xlink newlib:\xos\defsegs protfs \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=protfs /map=protfs /sym=protfs
	copy /over protfs.run newcmd:protfs.run

protfs.obj:	protfs.c

rename.run:	rename.obj
	xlink newlib:\xos\defsegs rename \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=rename /map=rename /sym=rename
	copy /over rename.run newcmd:rename.run

rename.obj:	rename.c

rmdir.run:	rmdir.obj
	xlink newlib:\xos\defsegs rmdir \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=rmdir /map=rmdir /sym=rmdir
	copy /over rmdir.run newcmd:rmdir.run

rmdir.obj:	rmdir.c

set.run:	set.obj
	xlink newlib:\xos\defsegs set \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=set /map=set /sym=set
	copy /over set.run newcmd:set.run

set.obj:	set.c

setenv.run:	setenv.obj
	xlink newlib:\xos\defsegs setenv \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=setenv /map=setenv /sym=setenv
	copy /over setenv.run newcmd:setenv.run

setenv.obj:	setenv.c

symbiont.run:	symbiont.obj
	xlink newlib:\xos\defsegs newlib:\xos\_mainalt symbiont \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=symbiont /map=symbiont /sym=symbiont
	copy /over symbiont.run newcmd:symbiont.run

symbiont.obj:	symbiont.c

session.run:	session.obj
	xlink newlib:\xos\defsegs newlib:\xos\_mainalt session \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=session /map=session /sym=session
	copy /over session.run newcmd:session.run

session.obj:	session.c

syschar.run:	syschar.obj
	xlink newlib:\xos\defsegs syschar \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=syschar /map=syschar /sym=syschar
	copy /over syschar.run newcmd:syschar.run

syschar.obj:	devchar.c
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht /nm=sysschar /fo=syschar.obj \
		/dsyschar devchar.c

sysdis.run:	sysdis.obj
	xlink sysdis newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=sysdis /map=sysdis /sym=sysdis
	copy /over sysdis.run newcmd:sysdis.run

sysdis.obj: sysdis.m86

type.run:	type.obj
	xlink newlib:\xos\defsegs type \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=type /map=type /sym=type
	copy /over type.run newcmd:type.run

type.obj:	type.c

command.com:	command.obj
	link command /map /line, command;
	gensym command
	exe2com command
	-delete command.exe
	-delete command.map
	copy /over command.com newcmd:command.com

command.obj:	command.asm
	tasm command

dismount.run:	dismount.obj
	xlink newlib:\xos\defsegs dismount \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=dismount /map=dismount /sym=dismount
	copy /over dismount.run newcmd:dismount.run

dismount.obj:	dismount.c

metric.exe:	metric.obj
	xlink newlib:\xos\defsegs metric \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=metric /map=metric /sym=metric

metric.obj: metric.c

lprt.run:	lprt.obj
	xlink newlib:\xos\defsegs lprt \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/o=lprt /m=lprt /sym=lprt
	copy /over lprt.run newcmd:lprt.run

lprt.obj: lprt.c

netinst.run: netinst.obj
	xlink newlib:\xos\defsegs netinst \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/o=netinst /m=netinst /sym=netinst
	copy /over netinst.run newcmd:netinst.run

netinst.obj: netinst.c

makegroup.run: makegroup.obj
	xlink newlib:\xos\defsegs makegroup \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/o=makegroup /m=makegroup /sym=makegroup
	copy /over makegroup.run newcmd:makegroup.run

makegroup.obj: makegroup.c

diskinst.run: diskinst.obj
	xlink newlib:\xos\defsegs diskinst \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/o=diskinst /m=diskinst /sym=diskinst
	copy /over diskinst.run newcmd:diskinst.run

diskinst.obj: diskinst.c

coninst.run: coninst.obj
	xlink newlib:\xos\defsegs coninst \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/o=coninst /m=coninst /sym=coninst
	copy /over coninst.run newcmd:coninst.run

coninst.obj: coninst.c

concmd.run:	concmd.obj
	xlink newlib:\xos\defsegs newlib:\xos\_mainalt concmd \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=concmd /map=concmd /sym=concmd
	copy /over concmd.run newcmd:concmd.run

concmd.obj:	concmd.c

menusel.run:	menusel.obj
	xlink newlib:\xos\defsegs menusel \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=menusel /map=menusel /sym=menusel
	copy /over menusel.run newcmd:menusel.run

menusel.obj:	menusel.c

delay.run:	delay.obj
	xlink newlib:\xos\defsegs delay \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=delay /map=delay /sym=delay
	copy /over delay.run newcmd:delay.run

delay.obj:	delay.c

segtest.run:	segtest.obj
	xlink newlib:\xos\defsegs segtest \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=segtest /map=segtest /sym=segtest
	copy /over segtest.run newcmd:segtest.run

segtest.obj:	segtest.c

fptest.run:	fptest.obj
	xlink newlib:\xos\defsegs fptest \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=fptest /map=fptest /sym=fptest
	copy /over fptest.run newcmd:fptest.run

fptest.obj:	fptest.c

syslog.run:	syslog.obj
	xlink newlib:\xos\defsegs syslog \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=syslog /map=syslog /sym=syslog
	copy /over syslog.run newcmd:syslog.run

syslog.obj:	syslog.c



badsuspend.run:	badsuspend.obj
	xlink newlib:\xos\defsegs badsuspend \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=badsuspend /map=badsuspend /sym=badsuspend
#	copy /over badsuspend.run newcmd:badsuspend.run

badsuspend.obj:	badsuspend.c


ctest.run:	ctest.obj
	xlink newlib:\xos\defsegs ctest \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=ctest /map=ctest /sym=ctest
	copy /over ctest.run newcmd:ctest.run

ctest.obj:	ctest.c



