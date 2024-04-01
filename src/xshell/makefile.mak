# Makefile for XOS SHELL/BATCH

.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fpi87 /fp3 /s /mf \
		/zc /ze /orkht /nm=$(@DPN) $(@DPN).c
#	WCC386 /D1 /I=XOSINC:\WCXX\ /ZQ /3S /J /W3 /WE /FP3 /S \
#		/NM=$(@DPN) $(@DPN)

all:	xshell.run
	@ECHO Finished.

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

xshell.run:	xshell.obj signal.obj command.obj findapps.obj batch.obj \
		termination.obj prompt.obj history.obj utility.obj cmdcd.obj \
		cmdcls.obj cmdexit.obj cmddelay.obj cmdlogout.obj \
		cmdcall.obj cmdfor.obj \
		cmdgoto.obj cmdhistory.obj cmdif.obj cmdpause.obj cmdrem.obj \
		cmdreturn.obj cmdshift.obj cmdver.obj cmdecho.obj \
		cmdjump.obj cmdbatopt.obj
	xlink xoslib:\xos\defsegs xshell signal command findapps batch \
		termination prompt history utility cmdcd cmdcls cmdexit \
		cmddelay cmdlogout \
		cmdcall cmdfor cmdgoto cmdhistory cmdif cmdpause cmdrem \
		cmdreturn cmdshift cmdver cmdecho cmdjump cmdbatopt \
		xoslib:\xos\_mainalt xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=xshell /map=xshell /sym=xshell
#	copy /over xshell.run NEWCMD:xshell.run

xshell.obj:		xshell.c
signal.obj:		signal.c
command.obj:		command.c
findapps.obj:		findapps.c
batch.obj:		batch.c
termination.obj:	termination.c
prompt.obj:		prompt.c
history.obj:		history.c
utility.obj:		utility.c
cmdcd.obj:		cmdcd.c
cmdcls.obj:		cmdcls.c
cmdexit.obj:		cmdexit.c
cmddelay.obj:		cmddelay.c
cmdlogout.obj:		cmdlogout.c
cmdcall.obj:		cmdcall.c
cmdfor.obj:		cmdfor.c
cmdgoto.obj:		cmdgoto.c
cmdhistory.obj:		cmdhistory.c
cmdif.obj:		cmdif.c
cmdpause.obj:		cmdpause.c
cmdrem.obj:		cmdrem.c
cmdreturn.obj:		cmdreturn.c
cmdshift.obj:		cmdshift.c
cmdver.obj:		cmdver.c
cmdecho.obj:		cmdecho.c
cmdjump.obj:		cmdjump.c
cmdbatopt.obj:		cmdbatopt.c

#utility.obj:	utility.c shell.h shellext.h

