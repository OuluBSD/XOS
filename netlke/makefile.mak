.m86.obj:
	xmac $(@DPN)

.xc.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /wx /we /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oafkhs /nt=x_CODE /nd=x /nm=$(@DPN) \
		$(@DPN).xc

.oc.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /wx /we /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarkhs /nt=o_CODE /nd=o /nm=$(@DPN) \
		$(@DPN).oc

.uc.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /5s /w3 /we /fp3 /s /mf /zc \
		/zp4 /zpw /za99 /ze /oarfkhs /nt=u_CODE /nd=u /nm=$(@DPN) \
		$(@DPN).uc

all:	netcls.lke enrtadev.lke ipcls.lke arpcls.lke icmpcls.lke udpcls.lke
	@ECHO Finished.

#xfpcls.lke arpcls.lke enneadrv.lke \
#en3cadrv.lke enfjadrv.lke \
#ensmadrv.lke enwdadrv.lke enxxxdrv.lke \
#hdlcacls.lke lapbacls.lke ipscls.lke rcpcls.lke \
#slipadrv.lke snapcls.lke \
#ssanadrv.lke sswnadrv.lke tcpcls.lke \
#tlnacls.lke x25acls.lke udpcls.lke


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




test.obj:	test.xc

netcls.lke: 	netcls1.obj netcls2.obj
	xlink /imp=knl /imp=sys \
		xoslib:xos\lkesegs netcls1 netcls2 \
		/output=netcls.lke /map=netcls /lke /sym386 /sym=netcls
	copy /over netcls.lke newsys:netcls.lke
	copy /over netcls.sym newsys:netcls.sym

netcls1.obj:	netcls1.xc
netcls2.obj:	netcls2.xc

ipcls.lke:	ipcls1.obj ipcls2.obj ipcls3.obj iproute.obj
	xlink /imp=knl /imp=sys /imp=xosnet /imp=xosarp \
		xoslib:xos\lkesegs ipcls1 ipcls2 ipcls3 iproute \
		/output=ipcls.lke /map=ipcls /lke /sym386 /sym=ipcls
	copy /over ipcls.lke newsys:ipcls.lke
	copy /over ipcls.sym newsys:ipcls.sym


xipcls.lke:	ipcls1.obj ipcls2.obj iproute.obj
	gecko ..\xlink\xxlink /imp=knl /imp=sys /imp=xosnet \
		xoslib:xos\lkesegs ipcls1 ipcls2 iproute \
		/output=ipcls.lke /map=ipcls /lke /sym386 /sym=ipcls
	copy /over ipcls.lke newsys:ipcls.lke
	copy /over ipcls.sym newsys:ipcls.sym

ipcls1.obj:	ipcls1.xc
ipcls2.obj:	ipcls2.xc
ipcls3.obj:	ipcls3.m86
ipicmp.obj:	ipicmp.xc
iproute.obj:	iproute.xc


arpcls.lke:	arpcls.obj
	xlink /imp=knl /imp=sys /imp=xosnet \
		xoslib:xos\lkesegs arpcls \
		/output=arpcls.lke /map=arpcls /lke /sym386 /sym=arpcls
	copy /over arpcls.lke newsys:arpcls.lke
	copy /over arpcls.sym newsys:arpcls.sym

arpcls.obj:	arpcls.xc


icmpcls.lke:	icmpcls.obj
	xlink /imp=knl /imp=sys /imp=xosnet /imp=xosip \
		xoslib:xos\lkesegs icmpcls \
		/output=icmpcls.lke /map=icmpcls /lke /sym386 /sym=icmpcls
	copy /over icmpcls.lke newsys:icmpcls.lke
	copy /over icmpcls.sym newsys:icmpcls.sym

icmpcls.obj:	icmpcls.xc


udpcls.lke:	udpcls.obj
	xlink /imp=knl /imp=sys /imp=xospci /imp=xosnet /imp=xosip \
		xoslib:xos\lkesegs udpcls \
		/output=udpcls.lke /map=udpcls /lke /sym386 /sym=udpcls
	copy /over udpcls.lke newsys:udpcls.lke
	copy /over udpcls.sym newsys:udpcls.sym

udpcls.obj:	udpcls.xc


enrtadev.lke:	enrtadev.obj enrtadevi.obj
	xlink /imp=knl /imp=sys /imp=xospci /imp=xosnet \
		xoslib:xos\lkesegs enrtadev enrtadevi \
		/output=enrtadev.lke /map=enrtadev /lke /sym386 /sym=enrtadev
	copy /over enrtadev.lke newsys:enrtadev.lke
	copy /over enrtadev.sym newsys:enrtadev.sym

enrtadev.obj:	enrtadev.xc
enrtadevi.obj:	enrtadevi.m86


tcpcls.lke:	tcpcls1.obj tcpcls2.obj tcpcls3.obj
	xlink /imp=knl /imp=sys /imp=xosnet /imp=xosip \
		xoslib:xos\lkesegs tcpcls1 tcpcls2 tcpcls3 \
		/output=tcpcls.lke /map=tcpcls /lke /sym386 /sym=tcpcls
	copy /over tcpcls.lke newsys:tcpcls.lke
	copy /over tcpcls.sym newsys:tcpcls.sym

tcpcls1.obj:	tcpcls1.xc

tcpcls2.obj:	tcpcls2.xc

tcpcls3.obj:	tcpcls3.xc




xfpcls.lke:	xfpcls.obj
	xlink xfpcls /output=xfpcls.lke /map=xfpcls /lke /sym386 /sym=xfpcls
	copy /over xfpcls.lke newsys:xfpcls.lke
	copy /over xfpcls.sym newsys:xfpcls.sym

xfpcls.obj:	xfpcls.m86

xfpocls.lke:	xfpocls.obj
	xlink xfpocls /output=xfpocls.lke /map=xfpocls /lke /sym386
	copy /over xfpocls.lke newsys:xfpocls.lke

xfpocls.obj:	xfpocls.m86

EN3CADRV.LKE:	EN3CADRV.OBJ
	XLINK EN3CADRV /OUTPUT=EN3CADRV.LKE /MAP=EN3CADRV /LKE /SYM386
	COPY /OVER EN3CADRV.LKE NEWSYS:EN3CADRV.LKE

EN3CADRV.OBJ:	EN3CADRV.M86 NS8390.PAR

ENFJADRV.LKE:	ENFJADRV.OBJ
	XLINK ENFJADRV /OUTPUT=ENFJADRV.LKE /MAP=ENFJADRV /LKE /SYM386
	COPY /OVER ENFJADRV.LKE NEWSYS:ENFJADRV.LKE

ENFJADRV.OBJ:	ENFJADRV.M86

enneadrv.lke:	enneadrv.obj
	xlink enneadrv \
		/output=enneadrv.lke /map=enneadrv /lke /sym386 /sym=enneadrv
	copy /over enneadrv.lke newsys:enneadrv.lke
	copy /over enneadrv.sym newsys:enneadrv.sym

enneadrv.obj:	enneadrv.m86 ns8390.par

ENVIADRV.LKE:	ENVIADRV.OBJ
	XLINK ENVIADRV /OUTPUT=ENVIADRV.LKE /MAP=ENVIADRV /LKE /SYM386
	COPY /OVER ENVIADRV.LKE NEWSYS:ENVIADRV.LKE

ENVIADRV.OBJ:	ENVIADRV.M86 VIARHINE.PAR

ENSMADRV.LKE:	ENSMADRV.OBJ
	XLINK ENSMADRV /OUTPUT=ENSMADRV.LKE /MAP=ENSMADRV /LKE /SYM386
	COPY /OVER ENSMADRV.LKE NEWSYS:ENSMADRV.LKE

ENSMADRV.OBJ:	ENSMADRV.M86

ENWDADRV.LKE:	ENWDADRV.OBJ
	XLINK ENWDADRV /OUTPUT=ENWDADRV.LKE /MAP=ENWDADRV /LKE /SYM386
	COPY /OVER ENWDADRV.LKE NEWSYS:ENWDADRV.LKE

ENWDADRV.OBJ:	ENWDADRV.M86 NS8390.PAR

ENXXXDRV.LKE:	ENXXXDRV.OBJ
	XLINK ENXXXDRV /OUTPUT=ENXXXDRV.LKE /MAP=ENXXXDRV /LKE /SYM386
	COPY /OVER ENXXXDRV.LKE NEWSYS:ENXXXDRV.LKE

ENXXXDRV.OBJ:	ENXXXDRV.M86

HDLCACLS.LKE:	HDLCACLS.OBJ
	XLINK HDLCACLS /OUTPUT=HDLCACLS.LKE /MAP=HDLCACLS /LKE /SYM386
	COPY /OVER HDLCACLS.LKE NEWSYS:HDLCACLS.LKE

HDLCACLS.OBJ:	HDLCACLS.M86

LAPBACLS.LKE:	LAPBACLS.OBJ
	XLINK LAPBACLS /OUTPUT=LAPBACLS.LKE /MAP=LAPBACLS /LKE /SYM386
	COPY /OVER LAPBACLS.LKE NEWSYS:LAPBACLS.LKE

LAPBACLS.OBJ:	LAPBACLS.M86

HDLCCHK.RUN:	HDLCCHK.OBJ
	XLINK XOSLIB:XOS\DEFSEGS HDLCCHK \
		XOSLIB:\XOS\CLIBXOS XOSLIB:\XOS\CLIBANSI \
		/OUTPUT=HDLCCHK /SYM=HDLCCHK /MAP=HDLCCHK

HDLCCHK.OBJ:	HDLCCHK.C

HDLCMON.EXE:	HDLCMON.OBJ
	XLINK XOSLIB:XOS\DEFSEGS HDLCMON \
		XOSLIB:\XOS\CLIBXOS XOSLIB:\XOS\CLIBANSI \
		/OUTPUT=HDLCMON /SYM=HDLCMON /MAP=HDLCMON

HDLCMON.OBJ:	HDLCMON.C

HDLCRCV.EXE:	HDLCRCV.OBJ
	XLINK XOSLIB:XOS\DEFSEGS HDLCRCV \
		XOSLIB:\XOS\CLIBXOS XOSLIB:\XOS\CLIBANSI \
		/OUTPUT=HDLCRCV /SYM=HDLCRCV /MAP=HDLCRCV

HDLCRCV.OBJ:	HDLCRCV.C

HDLCSND.EXE:	HDLCSND.OBJ
	XLINK XOSLIB:XOS\DEFSEGS HDLCSND \
		XOSLIB:\XOS\CLIBXOS XOSLIB:\XOS\CLIBANSI \
		/OUTPUT=HDLCSND /SYM=HDLCSND /MAP=HDLCSND

HDLCSND.OBJ:	HDLCSND.C


ipscls.lke:	ipscls.obj ipsicmp.obj ipsroute.obj ipsdns.obj
	xlink ipscls ipsicmp ipsroute ipsdns \
		/output=ipscls.lke /map=ipscls /lke /sym386 /sym=ipscls
	copy /over ipscls.lke newsys:ipscls.lke
	copy /over ipscls.sym newsys:ipscls.sym

ipscls.obj:	ipscls.m86

ipsicmp.obj:	ipsicmp.m86

ipsroute.obj:	ipsroute.m86

ipsdns.obj:	ipsdns.m86


RCPACT.RUN:	RCPACT.OBJ
	XLINK XOSLIB:XOS\DEFSEGS RCPACT \
		XOSLIB:\XOS\CLIBXOS XOSLIB:\XOS\CLIBANSI \
		/OUTPUT=RCPACT /SYM=RCPACT /MAP=RCPACT

RCPACT.OBJ:	RCPACT.C

rcpcls.lke:	rcpcls1.obj rcpcls2.obj
	xlink rcpcls1 rcpcls2 \
		/output=rcpcls.lke /map=rcpcls /lke /sym386 /sym=rcpcls
	copy /over rcpcls.lke newsys:rcpcls.lke
	copy /over rcpcls.sym newsys:rcpcls.sym

rcpcls1.obj:	rcpcls1.m86

rcpcls2.obj:	rcpcls2.m86

RCPPAS.RUN:	RCPPAS.OBJ
	XLINK XOSLIB:XOS\DEFSEGS RCPPAS \
		XOSLIB:\XOS\CLIBXOS XOSLIB:\XOS\CLIBANSI \
		/OUTPUT=RCPPAS /SYM=RCPPAS /MAP=RCPPAS

RCPPAS.OBJ:	RCPPAS.C

slipadrv.lke:	slipadrv.obj
	xlink slipadrv \
		/output=slipadrv.lke /map=slipadrv /lke /sym386 /sym=slipadrv
	copy /over slipadrv.lke newsys:slipadrv.lke
	copy /over slipadrv.sym newsys:slipadrv.sym

slipadrv.obj:	slipadrv.m86

snapcls.lke:	snapcls.obj
	xlink snapcls \
		/output=snapcls.lke /map=snapcls /lke /sym386 /sym=snapcls
	copy /over snapcls.lke newsys:snapcls.lke
	copy /over snapcls.sym newsys:snapcls.sym

snapcls.obj:	snapcls.m86

SSANADRV.LKE:	SSANADRV.OBJ
	XLINK SSANADRV /OUTPUT=SSANADRV.LKE /MAP=SSANADRV /LKE /SYM386
	COPY /OVER SSANADRV.LKE NEWSYS:SSANADRV.LKE

SSANADRV.OBJ:	SSANADRV.M86 HD64570.PAR

SSWNADRV.LKE:	SSWNADRV.OBJ
	XLINK SSWNADRV /OUTPUT=SSWNADRV.LKE /MAP=SSWNADRV /LKE /SYM386
	COPY /OVER SSWNADRV.LKE NEWSYS:SSWNADRV.LKE

SSWNADRV.OBJ:	SSWNADRV.M86 Z8530.PAR

tlnacls.lke:	tlnacls.obj
	xlink tlnacls \
		/output=tlnacls.lke /map=tlnacls /lke /sym386t /sym=tlnacls
	copy /over tlnacls.lke newsys:tlnacls.lke
	copy /over tlnacls.sym newsys:tlnacls.sym

tlnacls.obj:	tlnacls.m86

X25ACLS.LKE:	X25ACLS1.OBJ X25ACLS2.OBJ
	XLINK X25ACLS1 X25ACLS2 \
		/OUTPUT=X25ACLS.LKE /MAP=X25ACLS /LKE /SYM386
	COPY /OVER X25ACLS.LKE NEWSYS:X25ACLS.LKE

X25ACLS1.OBJ:	X25ACLS1.M86

X25ACLS2.OBJ:	X25ACLS2.M86

X25TEST.RUN:	X25TEST.OBJ
	XLINK XOSLIB:XOS\DEFSEGS X25TEST \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=X25TEST /SYM=X25TEST /MAP=X25TEST
	COPY /OVER X25TEST.RUN XOSCMD:X25TEST.RUN

X25TEST.OBJ:	X25TEST.C

X25SEND.RUN:	X25SEND.OBJ
	XLINK XOSLIB:XOS\DEFSEGS X25SEND \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=X25SEND /SYM=X25SEND /MAP=X25SEND
	COPY /OVER X25SEND.RUN XOSCMD:X25SEND.RUN

X25SEND.OBJ:	X25SEND.C

X25ECHO.RUN:	X25ECHO.OBJ
	XLINK X25ECHO XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/O=X25ECHO /M=X25ECHO /SYM=X25ECHO
	COPY /OVER X25ECHO.RUN XOSCMD:X25ECHO.RUN

X25ECHO.OBJ:	X25ECHO.M86

X25SINK.RUN:	X25SINK.OBJ
	XLINK X25SINK XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/M=X25SINK /SYM=X25SINK
	COPY /OVER X25SINK.RUN XOSCMD:X25SINK.RUN

X25SINK.OBJ:	X25SINK.M86

#TNPCLS.LKE:	TNPCLS.OBJ
#	XLINK TNPCLS /OUTPUT=TNPCLS.LKE /MAP=TNPCLS /LKE /SYM386
#	COPY /OVER TNPCLS.LKE NEWSYS:TNPCLS.LKE

#TNPCLS.OBJ:	TNPCLS.M86
