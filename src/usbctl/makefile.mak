.m86.obj:
	xmac $(@DPN)

.c.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht /nm=$(@DPN) $(@DPN).c

all:		usbctl.run usbwait.run
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

usbctl.run:	usbctl.obj cmdstatus.obj cmdretry.obj cmdlog.obj \
		cmdactive.obj root.obj device.obj vendor.obj  hub.obj \
		dska.obj ptra.obj hidboota.obj generic.obj parsehid.obj \
		usbmalloc.obj address.obj
	xlink newlib:\xos\defsegs newlib:\xos\_mainalt \
		usbctl cmdstatus cmdretry cmdlog cmdactive root device \
		vendor hub dska ptra hidboota generic parsehid usbmalloc \
		address \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/o=usbctl /m=usbctl /sym=usbctl
	copy /o usbctl.run newsys:usbctl.run
	copy /o usbctl.sym newsys:usbctl.sym

usbctl.obj:	usbctl.c
cmdstatus.obj:	cmdstatus.c
cmdretry.obj:	cmdretry.c
cmdlog.obj:	cmdlog.c
cmdactive.obj:	cmdactive.c
root.obj:	root.c
device.obj:	device.c
vendor.obj:	vendor.c
hub.obj:	hub.c
dska.obj:	dska.c
ptra.obj:	ptra.c
hidboota.obj:	hidboota.c
generic.obj:	generic.c
parsehid.obj:	parsehid.c
usbmalloc.obj:	usbmalloc.c
address.obj:	address.m86

usbwait.run:	usbwait.obj
	xlink newlib:\xos\defsegs usbwait \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/o=usbwait /m=usbwait /sym=usbwait
	copy /o usbwait.run newcmd:usbwait.run
	copy /o usbwait.sym newcmd:usbwait.sym

usbwait.obj:	usbwait.c
