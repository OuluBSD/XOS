.m86.obj:
	xmac $(@dpn)

.c.obj:
	owxc /hc /d1+ /i=xosinc:\owxc\ /zq /6s /w3 /we \
		/fpi87 /fp3 /s /mf /zc /zp4 /zpw /za99 /ze /oarkhs \
		/nm=$(@DPN) $(@DPN).c

.xc.obj:
	owxc /hc /d1+ /i=xosinc:\owxc\ /zq /6s /wx /we /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /zl /zld /zls /oarfkhs /nt=x_CODE \
		/nd=x /nm=$(@DPN) $(@DPN).xc

.oc.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /wx /we /fp3 /s /mf /zc /zl /zls \
		/zp4 /zpw /za99 /ze /oarkhs /nt=o_CODE /nd=o /nm=$(@DPN) \
		$(@DPN).oc

.uc.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fp3 /s /mf /zc /zl /zls \
		/zp4 /zpw /za99 /ze /oarfkhs /nt=u_CODE /nd=u /nm=$(@DPN) \
		$(@DPN).uc

all:	lkesegs.obj ffscls.lke dskcls.lke fdkadev.lke hdkadev.lke \
#		seradev.lke \
#		serbdrv.lke serddrv.lke seredev.lke \
		addmem.lke mousecls.lke \
#		mseadev.lke msebdev.lke \
		diskchk.lke pcicls.lke \
		scsisup.lke \
		ptrcls.lke splcls.lke \
		usbcls.lke usbuhcdev.lke usbohcdev.lke usbehcdev.lke \
		usbdskalnk.lke usbkbdalnk.lke usbptralnk.lke \
		usbmsealnk.lke \
		usbbulkalnk.lke bulkcls.lke \
		sndacls.lke widgets.run \
#		sndbcls.lke \
		speedctlp4cls.lke
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

lkeend.obj:	lkeend.m86

lkesegs.obj:	lkesegs.m86
	xmac lkesegs
	copy /over lkesegs.obj newlib:xos\lkesegs.obj

addmem.lke:	addmem.obj
	xlink /imp=knl /imp=sys xoslib:xos\lkesegs addmem \
		/output=addmem.lke /map=addmem /lke /sym386 /sym=addmem
	copy /over addmem.lke newsys:addmem.lke
	copy /over addmem.sym newsys:addmem.sym

addmem.obj:	addmem.xc

cdracls.lke:	cdracls1.obj cdracls2.obj cdraend.obj
	xlink cdracls1 cdracls2 cdraend \
		/output=cdracls.lke /map=cdracls /lke /sym386 /sym=cdracls
	copy /over cdracls.lke newsys:cdracls.lke
	copy /over cdracls.sym newsys:cdracls.sym

cdracls1.obj:	cdracls1.m86

cdracls2.obj:	cdracls2.m86

cdraend.obj:	cdraend.m86

chkspeed.run:	chkspeed.obj
	xlink xoslib:\xos\defsegs chkspeed \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=chkspeed /map=chkspeed /sym=chkspeed

chkspeed.obj:	chkspeed.c

ffscls.lke:	ffscls.obj ffscls2.obj ffsopen.obj ffsrename.obj \
		ffsdelete.obj ffsio.obj ffsclose.obj ffsfat.obj \
		ffsdirect.obj ffsfib.obj ffsfunc.obj ffsend.obj
	xlink /imp=xosdsk /imp=knl /imp=sys \
		ffscls ffscls2 ffsopen ffsrename ffsdelete ffsio \
		ffsclose ffsfat ffsdirect ffsfib ffsfunc ffsend \
		/output=ffscls.lke /map=ffscls /lke /sym386 /sym=ffscls
	copy /over ffscls.lke newsys:ffscls.lke
	copy /over ffscls.sym newsys:ffscls.sym

ffscls.obj:	ffscls.m86

ffscls2.obj:	ffscls2.xc
#	gecko \xosaux\owxc\owxcx /hc /i=xosinc:\owxc\ /zq /5s /wx /we /fp3 /s /mf /zc \
#		/zl /zld /zls /zp4 /zpw /za99 /ze /oarfkhs /nt=x_CODE \
#		/nd=x /nm=$(@DPN) $(@DPN).xc

ffsopen.obj:	ffsopen.m86
ffsrename.obj:	ffsrename.m86
ffsdelete.obj:	ffsdelete.m86
ffsio.obj:	ffsio.m86
ffsclose.obj:	ffsclose.m86
ffsfat.obj:	ffsfat.m86
ffsdirect.obj:	ffsdirect.m86
ffsfib.obj:	ffsfib.m86
ffsfunc.obj:	ffsfunc.m86
ffsend.obj:	ffsend.m86

diskchk.lke:	diskchk.obj
	xlink diskchk \
		/output=diskchk.lke /map=diskchk /lke /sym386 /sym=diskchk
	copy /over diskchk.lke newsys:diskchk.lke
	copy /over diskchk.sym newsys:diskchk.sym

diskchk.obj:	diskchk.m86

dskcls.lke:	dskcls1.obj dskcls2.obj buffer.obj path.obj \
		dskend.obj
	xlink dskcls1 dskcls2 buffer path dskend\
		/output=dskcls.lke /map=dskcls /lke /sym386 /sym=dskcls
	copy /over dskcls.lke newsys:dskcls.lke
	copy /over dskcls.sym newsys:dskcls.sym

dskcls1.obj:	dskcls1.m86
dskcls2.obj:	dskcls2.m86
buffer.obj:	buffer.m86
#lclfile.obj:	lclfile.m86
path.obj:	path.m86
dskend.obj:	dskend.m86

fdkadev.lke:	fdkadev.obj
	xlink fdkadev \
		/output=fdkadev.lke /map=fdkadev /lke /sym386 /sym=fdkadev
	copy /over fdkadev.lke newsys:fdkadev.lke
	copy /over fdkadev.sym newsys:fdkadev.sym

fdkadev.obj:	fdkadev.m86

hdkadev.lke:	hdkadev.obj
	xlink hdkadev \
		/output=hdkadev.lke /map=hdkadev /lke /sym386 /sym=hdkadev
	copy /over hdkadev.lke newsys:hdkadev.lke
	copy /over hdkadev.sym newsys:hdkadev.sym

hdkadev.obj:	hdkadev.m86

IBCACLS.LKE:	IBCACLS.OBJ
	XLINK IBCACLS /OUTPUT=IBCACLS.LKE /MAP=IBCACLS /LKE /SYM386
	COPY /OVER IBCACLS.LKE NEWSYS:IBCACLS.LKE

IBCACLS.OBJ:	IBCACLS.M86

IBSACLS.LKE:	IBSACLS.OBJ
	XLINK IBSACLS /OUTPUT=IBSACLS.LKE /MAP=IBSACLS /LKE /SYM386
	COPY /OVER IBSACLS.LKE NEWSYS:IBSACLS.LKE

IBSACLS.OBJ:	IBSACLS.M86

MDKADRV.LKE:	MDKADRV.OBJ
	XLINK MDKADRV /OUTPUT=MDKADRV.LKE /MAP=MDKADRV /LKE /SYM386
	COPY /OVER MDKADRV.LKE NEWSYS:MDKADRV.LKE

MDKADRV.OBJ:	MDKADRV.M86

sndacls.lke:	sndacls.obj sndawidget.obj sndaclsi.obj
	xlink /imp=knl /imp=sys /imp=xospci \
		xoslib:xos\lkesegs sndacls sndawidget sndaclsi \
		/output=sndacls.lke /map=sndacls /lke /sym386 /sym=sndacls
	copy /over sndacls.lke newsys:sndacls.lke
	copy /over sndacls.sym newsys:sndacls.sym

sndacls.obj:	sndacls.xc
sndawidget.obj:	sndawidget.xc
sndaclsi.obj:	sndaclsi.m86

widgets.run:	widgets.obj
	xlink xoslib:\xos\defsegs widgets \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=widgets /map=widgets /sym=widgets
	copy /over widgets.run newcmd:widgets.run

widgets.obj:	widgets.c

sndbcls.lke:	sndbcls.obj
	xlink sndbcls \
		/output=sndbcls.lke /map=sndbcls /lke /sym386 /sym=sndbcls
	copy /over sndbcls.lke newsys:sndbcls.lke
	copy /over sndbcls.sym newsys:sndbcls.sym

sndbcls.obj:	sndbcls.m86

speedctlp4cls.lke: speedctlp4cls.obj
	xlink speedctlp4cls \
		/output=speedctlp4cls.lke /map=speedctlp4cls /lke /sym386 \
		/sym=speedctlp4cls.sym
	copy /over speedctlp4cls.lke newsys:speedctlp4cls.lke
	copy /over speedctlp4cls.sym newsys:speedctlp4cls.sym

speedctlp4cls.obj: speedctlp4cls.m86

stest.run:	stest.obj stesta.obj
	xlink xoslib:\xos\defsegs stest stesta \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=stest /map=stest /sym=stest
	copy /over stest.run newcmd:stest.run

stest.obj:	stest.c
stesta.obj:	stesta.m86

splay.run:	splay.obj splaya.obj soundplay.obj
	xlink xoslib:\xos\defsegs splay splaya \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=splay /map=splay /sym=splay
	copy /over splay.run newcmd:splay.run

splay.obj:	splay.c
splaya.obj:	splaya.m86
soundplay.obj:	soundplay.c

wplay.run:	wplay.obj wplaya.obj soundplay.obj
	xlink xoslib:\xos\defsegs wplay wplaya soundplay \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=wplay /map=wplay /sym=wplay
	copy /over wplay.run newcmd:wplay.run

wplay.obj:	wplay.c
wplaya.obj:	wplaya.m86

pcncls.lke:	pcncls1.obj pcncls2.obj pcncls3.obj pcnend.obj
	xlink pcncls1 pcncls2 pcncls3 pcnend \
		/output=pcncls.lke /map=pcncls /lke /sym386 /sym=pcncls
	copy /over pcncls.lke newsys:pcncls.lke
	copy /over pcncls.sym newsys:pcncls.sym

pcncls1.obj:	pcncls1.m86

pcncls2.obj:	pcncls2.m86

pcncls3.obj:	pcncls3.m86

pcnend.obj:	pcnend.m86

PCNTEST.RUN:	PCNTEST.OBJ
	XLINK PCNTEST XOSLIB:\XOS\LIBC01 \
		/OUTPUT=PCNTEST /MAP=PCNTEST /SYM=PCNTEST

PCNTEST.OBJ:	PCNTEST.M86

PPRCLS.LKE:	PPRCLS.OBJ
	XLINK PPRCLS /OUTPUT=PPRCLS.LKE /MAP=PPRCLS /LKE /SYM386
	COPY /OVER PPRCLS.LKE NEWSYS:PPRCLS.LKE

PPRCLS.OBJ:	PPRCLS.M86

ptrcls.lke:	ptrcls.obj
	xlink ptrcls /output=ptrcls.lke /map=ptrcls /lke /sym386 /sym=ptrcls
	copy /over ptrcls.lke newsys:ptrcls.lke
	copy /over ptrcls.sym newsys:ptrcls.sym

ptrcls.obj:	ptrcls.m86


pt.run:	pt.obj
	xlink xoslib:\xos\defsegs pt \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=pt /map=pt /sym=pt
	copy /over pt.run newcmd:pt.run

pt.obj:		pt.c

print.run:	print.obj
	xlink xoslib:\xos\defsegs print \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=print /map=print /sym=print
	copy /over print.run newcmd:print.run

print.obj:	print.c

bulkcls.lke:	bulkcls.obj
	xlink bulkcls \
		/output=bulkcls.lke /map=bulkcls /lke /sym386 /sym=bulkcls
	copy /over bulkcls.lke newsys:bulkcls.lke
	copy /over bulkcls.sym newsys:bulkcls.sym

bulkcls.obj:	bulkcls.m86

mousecls.lke:	mousecls.obj
	xlink mousecls \
		/output=mousecls.lke /map=mousecls /lke /sym386 /sym=mousecls
	copy /over mousecls.lke newsys:mousecls.lke
	copy /over mousecls.sym newsys:mousecls.sym

mousecls.obj:	mousecls.m86

mseadev.lke:	mseadev.obj
	xlink mseadev /output=mseadev.lke /map=mseadev /lke /sym386
	copy /over mseadev.lke newsys:mseadev.lke

mseadev.obj:	mseadev.m86

msebdev.lke: msebdev.obj
	xlink msebdev /output=msebdev.lke /map=msebdev /lke /sym386
	copy /over msebdev.lke newsys:msebdev.lke

msebdev.obj:	msebdev.m86

usbmsealnk.lke: usbmsealnk.obj
	xlink usbmsealnk \
		/output=usbmsealnk.lke /map=usbmsealnk /lke /sym386 \
		/sym=usbmsealnk
	copy /over usbmsealnk.lke newsys:usbmsealnk.lke
	copy /over usbmsealnk.sym newsys:usbmsealnk.sym

usbmsealnk.obj:	usbmsealnk.m86

mtest.run:	mtest.obj
	xlink xoslib:\xos\defsegs mtest \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=mtest /map=mtest /sym=mtest
	copy /over mtest.run newcmd:mtest.run

mtest.obj:	mtest.c

rdtape.run:	rdtape.obj
	xlink xoslib:\xos\defsegs rdtape \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=rdtape /map=rdtape /sym=rdtape

rdtape.obj:	rdtape.c

rddisk.run:	rddisk.obj
	xlink xoslib:\xos\defsegs rddisk \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=rddisk /map=rddisk /sym=rddisk

rddisk.obj:	rddisk.c

scsiadta.lke:	scsiadta.obj
	xlink scsiadta /output=scsiadta.lke /map=scsiadta /lke /sym386
	copy /over scsiadta.lke newsys:scsiadta.lke

scsiadta.obj:	scsiadta.m86

scsicls.lke:	scsicls.obj
	xlink scsicls /output=scsicls.lke /map=scsicls /lke /sym386
	copy /over scsicls.lke newsys:scsicls.lke

scsicls.obj:	scsicls.m86

scsiinq.run:	scsiinq.obj
	xlink xoslib:\xos\defsegs scsiinq \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=scsiinq /map=scsiinq /sym=scsiinq
	copy /over scsiinq.run newcmd:scsiinq.run

scsiinq.obj:	scsiinq.c

sdskdrv.lke:	sdskdrv.obj
	xlink sdskdrv /output=sdskdrv.lke /map=sdskdrv /lke /sym386
	copy /over sdskdrv.lke newsys:sdskdrv.lke

sdskdrv.obj:	sdskdrv.m86

scsisup.lke:	scsisup.obj
	xlink scsisup \
		/output=scsisup.lke /map=scsisup /lke /sym386 /sym=scsisup
	copy /over scsisup.lke newsys:scsisup.lke
	copy /over scsisup.sym newsys:scsisup.sym

scsisup.obj:	scsisup.m86

usbcls.lke:	usbcls1.obj usbcls2.obj lkeend.obj
	xlink usbcls1 usbcls2 lkeend \
		/output=usbcls.lke /map=usbcls /lke /sym386 /sym=usbcls
	copy /over usbcls.lke newsys:usbcls.lke
	copy /over usbcls.sym newsys:usbcls.sym

usbcls1.obj:	usbcls1.m86
usbcls2.obj:	usbcls2.m86

usbehcdev.lke:	usbehcdev.obj
	xlink usbehcdev \
		/output=usbehcdev.lke /map=usbehcdev /lke /sym386 /sym=usbehcdev
	copy /over usbehcdev.lke newsys:usbehcdev.lke
	copy /over usbehcdev.sym newsys:usbehcdev.sym

usbehcdev.obj:	usbehcdev.m86

usbuhcdev.lke:	usbuhcdev.obj
	xlink usbuhcdev \
		/output=usbuhcdev.lke /map=usbuhcdev /lke /sym386 /sym=usbuhcdev
	copy /over usbuhcdev.lke newsys:usbuhcdev.lke
	copy /over usbuhcdev.sym newsys:usbuhcdev.sym

usbuhcdev.obj:	usbuhcdev.m86

usbohcdev.lke:	usbohcdev.obj
	xlink usbohcdev \
		/output=usbohcdev.lke /map=usbohcdev /lke /sym386 /sym=usbohcdev
	copy /over usbohcdev.lke newsys:usbohcdev.lke
	copy /over usbohcdev.sym newsys:usbohcdev.sym

usbohcdev.obj:	usbohcdev.m86

usbdskalnk.lke:	usbdskalnk.obj
	xlink usbdskalnk /output=usbdskalnk.lke /map=usbdskalnk /lke /sym386 \
		/sym=usbdskalnk
	copy /over usbdskalnk.lke newsys:usbdskalnk.lke
	copy /over usbdskalnk.sym newsys:usbdskalnk.sym

usbdskalnk.obj:	usbdskalnk.m86

usbptralnk.lke:	usbptralnk.obj
	xlink usbptralnk /output=usbptralnk.lke /map=usbptralnk /lke \
		/sym386 /sym=usbptralnk
	copy /over usbptralnk.lke newsys:usbptralnk.lke
	copy /over usbptralnk.sym newsys:usbptralnk.sym

usbptralnk.obj:	usbptralnk.m86

usbbulkalnk.lke: usbbulkalnk.obj
	xlink usbbulkalnk /output=usbbulkalnk.lke /map=usbbulkalnk /lke \
		/sym386 /sym=usbbulkalnk
	copy /over usbbulkalnk.lke newsys:usbbulkalnk.lke
	copy /over usbbulkalnk.sym newsys:usbbulkalnk.sym

usbbulkalnk.obj: usbbulkalnk.m86

usbkbdalnk.lke:	usbkbdalnk.obj
	xlink usbkbdalnk \
		/output=usbkbdalnk.lke /map=usbkbdalnk /lke /sym386 \
		/sym=usbkbdalnk
	copy /over usbkbdalnk.lke newsys:usbkbdalnk.lke
	copy /over usbkbdalnk.sym newsys:usbkbdalnk.sym

usbkbdalnk.obj:	usbkbdalnk.m86

kbdcls.lke:	kbdcls.obj
	xlink kbdcls /output=kbdcls.lke /map=kbdcls /lke /sym386 /sym=kbdcls
	copy /over kbdcls.lke newsys:kbdcls.lke
	copy /over kbdcls.sym newsys:kbdcls.sym

kbdcls.obj:	kbdcls.m86

pccacls.lke:	pccacls.obj
	xlink pccacls /output=pccacls.lke /map=pccacls /lke /sym386
	copy /over pccacls.lke newsys:pccacls.lke

pccacls.obj:	pccacls.m86

pccbcls.lke:	pccbcls.obj
	xlink pccbcls /output=pccbcls.lke /map=pccbcls /lke /sym386
	copy /over pccbcls.lke newsys:pccbcls.lke

pccbcls.obj:	pccbcls.m86

LCDACLS.LKE:	LCDACLS.OBJ
	XLINK LCDACLS /OUTPUT=LCDACLS.LKE /MAP=LCDACLS /LKE /SYM386
	COPY /OVER LCDACLS.LKE NEWSYS:LCDACLS.LKE

LCDACLS.OBJ:	LCDACLS.M86

CISDUMP.RUN:	CISDUMP.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS CISDUMP \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=CISDUMP /MAP=CISDUMP /SYM=CISDUMP
	COPY /OVER CISDUMP.RUN NEWCMD:CISDUMP.RUN

CISDUMP.OBJ:	CISDUMP.C

ELANAEXT.LKE:	ELANAEXT.OBJ
	XLINK ELANAEXT /OUTPUT=ELANAEXT.LKE /MAP=ELANAEXT /LKE /SYM386
	COPY /OVER ELANAEXT.LKE NEWSYS:ELANAEXT.LKE

ELANAEXT.OBJ:	ELANAEXT.M86

MDBACLS.LKE:	MDBACLS.OBJ
	XLINK MDBACLS /OUTPUT=MDBACLS.LKE /MAP=MDBACLS /LKE /SYM386
	COPY /OVER MDBACLS.LKE NEWSYS:MDBACLS.LKE

MDBACLS.OBJ:	MDBACLS.M86

seradev.lke:	seradev.obj
	xlink seradev \
		/output=seradev.lke /map=seradev /lke /sym386 /sym=seradev
	copy /over seradev.lke newsys:seradev.lke
	copy /over seradev.sym newsys:seradev.sym

seradev.obj:	seradev.m86

SERBDRV.LKE:	SERBDRV.OBJ
	XLINK SERBDRV /OUTPUT=SERBDRV.LKE /MAP=SERBDRV /LKE /SYM386
	COPY /OVER SERBDRV.LKE NEWSYS:SERBDRV.LKE

SERBDRV.OBJ:	SERBDRV.M86

SERCDRV.LKE:	SERCDRV.OBJ
	XLINK SERCDRV /OUTPUT=SERCDRV.LKE /MAP=SERCDRV /LKE /SYM386
	COPY /OVER SERCDRV.LKE NEWSYS:SERCDRV.LKE

SERCDRV.OBJ:	SERCDRV.M86

SERDDRV.LKE:	SERDDRV.OBJ
	XLINK SERDDRV /OUTPUT=SERDDRV.LKE /MAP=SERDDRV /LKE /SYM386
	COPY /OVER SERDDRV.LKE NEWSYS:SERDDRV.LKE

SERDDRV.OBJ:	SERDDRV.M86

seredev.LKE:	seredev.OBJ
	XLINK seredev /OUTPUT=seredev.LKE /MAP=seredev /LKE /SYM386
	COPY /OVER seredev.LKE NEWSYS:seredev.LKE

seredev.OBJ:	seredev.M86

splcls.lke:	splcls.obj
	xlink splcls /output=splcls.lke /map=splcls /lke /sym386 /sym=splcls
	copy /over splcls.lke newsys:splcls.lke
	copy /over splcls.sym newsys:splcls.sym

splcls.obj:	splcls.m86

STAPDRV.LKE:	STAPDRV.OBJ
	XLINK STAPDRV /OUTPUT=STAPDRV.LKE /MAP=STAPDRV /LKE /SYM386
	COPY /OVER STAPDRV.LKE NEWSYS:STAPDRV.LKE

STAPDRV.OBJ:	STAPDRV.M86

TAPECLS.LKE:	TAPECLS.OBJ
	XLINK TAPECLS /OUTPUT=TAPECLS.LKE /MAP=TAPECLS /LKE /SYM386
	COPY /OVER TAPECLS.LKE NEWSYS:TAPECLS.LKE

TAPECLS.OBJ:	TAPECLS.M86

WRTAPE.RUN:	WRTAPE.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS WRTAPE \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=WRTAPE /MAP=WRTAPE /SYM=WRTAPE

WRTAPE.OBJ:	WRTAPE.C

WRTDSK.RUN:	WRTDSK.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS WRTDSK \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=WRTDSK /MAP=WRTDSK /SYM=WRTDSK

WRTDSK.OBJ:	WRTDSK.C

PPRTEST.RUN:	PPRTEST.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS PPRTEST \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=PPRTEST /MAP=PPRTEST /SYM=PPRTEST

PPRTEST.OBJ:	PPRTEST.C

XFSCLS.LKE:	XFSCLS1.OBJ XFSCLS2.OBJ XFSCLS3.OBJ XFSCLS3.OBJ XFSEND.OBJ
	XLINK XFSCLS1 XFSCLS2 XFSCLS3 XFSEND\
		/OUTPUT=XFSCLS.LKE /MAP=XFSCLS /LKE /SYM386
	COPY /OVER XFSCLS.LKE NEWSYS:XFSCLS.LKE

XFSCLS1.OBJ:	XFSCLS1.M86

XFSCLS2.OBJ:	XFSCLS2.M86

XFSCLS3.OBJ:	XFSCLS3.M86

XFSCLS4.OBJ:	XFSCLS4.M86

XFSEND.OBJ:	XFSEND.M86

FILLUP.RUN:	FILLUP.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS FILLUP \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=FILLUP /MAP=FILLUP /SYM=FILLUP

FILLUP.OBJ:	FILLUP.C

MDBTEST.RUN:	MDBTEST.OBJ MDBFUNC.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS MDBTEST MDBFUNC \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=MDBTEST /MAP=MDBTEST /SYM=MDBTEST

MDBTEST.OBJ:	MDBTEST.C
MDBFUNC.OBJ:	MDBFUNC.C

MDBMON.RUN:	MDBMON.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS MDBMON \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=MDBMON /MAP=MDBMON /SYM=MDBMON

MDBMON.OBJ:	MDBMON.C

pcicls.lke:	pcicls.obj
	xlink /imp=knl /imp=sys \
		xoslib:xos\lkesegs pcicls \
		/output=pcicls.lke /map=pcicls /lke /sym386 /sym=pcicls
	copy /over pcicls.lke newsys:pcicls.lke
	copy /over pcicls.sym newsys:pcicls.sym

pcicls.obj:	pcicls.xc

dskdt.run:	dskdt.obj
	xlink xoslib:\xos\defsegs dskdt \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=dskdt /map=dskdt /sym=dskdt
	copy /over dskdt.run newcmd:dskdt.run

dskdt.obj:	dskdt.c

