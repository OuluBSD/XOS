.m86.obj:
	xmac $(@dpn)

.c.obj:
	wcc386 /d1 /i=xosinc:\wcx\ /nm=$(@dpn) $(@dpn)

all:	geckot.obj libgeckot.obj
	@echo Finished.

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

libgeckot.obj:	geckot.obj usbio.obj usbinit.obj control.obj debug.obj \
		reqbpctrl.obj reqbplist.obj reqlkelist.obj reqnoop.obj \
		reqrdio.obj reqrdmem.obj reqrdreg.obj reqrdpci.obj \
		reqgetpa.obj reqgetva.obj  reqstate.obj reqwpctrl.obj \
		reqwplist.obj reqwrtio.obj reqwrtmem.obj reqwrtreg.obj \
		reqwrtpci.obj reqxctrl.obj memory.obj data.obj
	xlib libgeckot /create/list=libgeckot \
		geckot usbio usbinit control debug reqbpctrl reqbplist \
		reqlkelist reqnoop reqrdio reqrdmem reqrdreg reqrdpci \
		reqgetpa reqstate reqgetva reqwpctrl reqwplist reqwrtio \
		reqwrtmem reqwrtreg reqwrtpci reqxctrl memory data
	copy /over libgeckot.obj newlib:xos\libgeckot.obj

geckot.obj:	geckot.m86
usbio.obj:	usbio.m86
usbinit.obj:	usbinit.m86
control.obj:	control.m86
debug.obj:	debug.m86
reqbpctrl.obj:	reqbpctrl.m86
reqbplist.obj:	reqbplist.m86
reqlkelist.obj:	reqlkelist.m86
reqnoop.obj:	reqnoop.m86
reqrdio.obj:	reqrdio.m86
reqrdmem.obj:	reqrdmem.m86
reqrdreg.obj:	reqrdreg.m86
reqrdpci.obj:	reqrdpci.m86
reqgetpa.obj:	reqgetpa.m86
reqgetva.obj:	reqgetva.m86
reqstate.obj:	reqstate.m86
reqwpctrl.obj:	reqwpctrl.m86
reqwplist.obj:	reqwplist.m86
reqwrtio.obj:	reqwrtio.m86
reqwrtmem.obj:	reqwrtmem.m86
reqwrtreg.obj:	reqwrtreg.m86
reqwrtpci.obj:	reqwrtpci.m86
reqxctrl.obj:	reqxctrl.m86
memory.obj:	memory.m86
data.obj:	data.m86
