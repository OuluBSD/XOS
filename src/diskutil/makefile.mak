.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkht /nm=$(@DPN) $(@DPN).c

all:	format.run diskutil.run makedata.run
	@ECHO	All done

format.run:	format.obj libformat.obj
	xlink xoslib:\xos\defsegs format libformat \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=format /m=format /sym=format
	copy /over format.run newcmd:format.run

format.obj:	format.c

libformat.obj:	xfsbblk.obj fmtfat.obj fmtfuncs.obj
	xlib libformat/create/list=libformat \
		xfsbblk fmtfat fmtfuncs
	copy /over libformat.obj newlib:\xos\libformat.obj

fmtfuncs.obj:	fmtfuncs.c bootblk.c
fmtxfs.obj:	fmtxfs.c
xfsbblk.obj:	xfsbblk.m86
fmtfat.obj:	fmtfat.c

makedata.run:	makedata.obj
	xlink xoslib:\xos\defsegs makedata \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=makedata /m=makedata /sym=makedata

makedata.obj:	makedata.c

# Generate the C code which specifies the MBR contents

mbrblk.c:	mbr.run
	makedata mbr.run mbrblk.c mbrblk 3000

mbr.run:	mbr.obj
	xlink mbr /alone /o=mbr /m=mbr /sym=mbr

mbr.obj:	mbr.m86

# Generate the C code which specifies the boot block contents

bootblk.c:	boot.run
	makedata boot.run bootblk.c bootblk 3000

boot.run:	boot.obj
	xlink boot /o=boot /alone

boot.obj:	boot.m86

# Generate diskutil program

diskutil.run:	diskutil.obj diskutilfuncs.obj
	xlink xoslib:\xos\defsegs diskutil diskutilfuncs \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=diskutil /m=diskutil /sym=diskutil
	copy /over diskutil.run newcmd:diskutil.run

diskutil.obj:	diskutil.c

# Generate the diskutil library

diskutilfuncs.obj: diskutilopen.obj \
		diskutilmakepart.obj \
		diskutilinitmbr.obj \
		diskutilreadmbr.obj \
		diskutilwipe.obj \
		diskutilfinish.obj \
		diskutilclose.obj
	xlib diskutilfuncs /create/list=diskutilfuncx \
		diskutilopen \
		diskutilmakepart \
		diskutilinitmbr \
		diskutilreadmbr \
		diskutilwipe.obj \
		diskutilfinish \
		diskutilclose
	copy /over diskutilfuncs.obj newlib:\xos\diskutilfuncs.obj

diskutilopen.obj:	diskutilopen.c
diskutilreadmbr.obj:	diskutilreadmbr.c
diskutilmakepart.obj:	diskutilmakepart.c
diskutilinitmbr.obj:	diskutilinitmbr.c mbrblk.c
diskutilwipe.obj:	diskutilwipe.c
diskutilfinish.obj:	diskutilfinish.c
diskutilclose.obj:	diskutilclose.c

mbrdmp.run:	mbrdmp.obj
	xlink newlib:\xos\defsegs mbrdmp \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=mbrdmp /map=mbrdmp /sym=mbrdmp
	copy /over mbrdmp.run newcmd:mbrdmp.run

mbrdmp.obj:	mbrdmp.c
