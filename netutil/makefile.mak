.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	owxc /hc /d1+ /i=xosinc:\owxc\ /zq /6s /w3 /we \
		/fpi87 /fp3 /s /mf /zc /zp4 /zpw /za99 /ze /oarkhs \
		/nm=$(@DPN) $(@DPN).c

ALL:		BOOTSRV.RUN \
#		TELNET.RUN \
		TLNSRV.RUN PING.RUN NETMODEM.RUN NETLINK.RUN \
		DNSRSLV.RUN DNSTEST.RUN
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

ping.run: 	ping.obj
	xlink xoslib:\xos\defsegs ping \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=ping /m=ping /sym=ping
	copy /over ping.run newcmd:ping.run

ping.obj:	ping.c

dnstest.run: 	dnstest.obj
	xlink xoslib:\xos\defsegs dnstest \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=dnstest /m=dnstest /sym=dnstest
	copy /over dnstest.run newcmd:dnstest.run

dnstest.obj:	dnstest.c

dhcpclient.run: dhcpclient.obj
	xlink xoslib:\xos\defsegs dhcpclient \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=dhcpclient /m=dhcpclient /sym=dhcpclient
	copy /over dhcpclient.run newcmd:dhcpclient.run

dhcpclient.obj:	dhcpclient.c

netlink.run:	netlink.obj
	xlink xoslib:\xos\defsegs netlink \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=netlink /m=netlink /sym=netlink
	copy /over netlink.run newcmd:netlink.run

netlink.obj:	netlink.c

netmodem.run:	netmodem.obj
	xlink xoslib:\xos\defsegs netmodem \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=netmodem /m=netmodem /sym=netmodem
	copy /over netmodem.run newcmd:netmodem.run

netmodem.obj:	netmodem.c

CKSUM.RUN:	CKSUM.OBJ
	LINK L:\TCV\$$MAIN CKSUM, CKSUM /MAP /LINE, CKSUM, \
		L:\TCV\ANSITV01 L:\TCV\XOSTV01;
	GENSYM CKSUM
	EXE2RUN CKSUM
	DEL CKSUM.EXE

CKSUM.OBJ:	CKSUM.C

MSG.RUN:	MSG.OBJ
	LINK L:\TCV\$$MAINX MSG, MSG /MAP /LINE, MSG, \
		L:\TCV\ANSITV01 L:\TCV\XOSTV01;
	GENSYM MSG
	EXE2RUN MSG
	DEL MSG.EXE
	COPY /OVER MSG.RUN NEWCMD:MSG.RUN

MSG.OBJ:	MSG.C

XFPOSRV.RUN:	XFPOSRV.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS XOSLIB:\XOS\_MAINALT XFPOSRV \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/O=XFPOSRV /M=XFPOSRV /SYM=XFPOSRV
	COPY /OVER XFPOSRV.RUN NEWSYS:XFPOSRV.RUN

XFPOSRV.OBJ:	XFPOSRV.M86

xfpnsrv.run:	xfpnsrv.obj
	xlink xoslib:\xos\defsegs xoslib:\xos\_mainalt xfpnsrv \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=xfpnsrv /m=xfpnsrv /sym=xfpnsrv
	copy /over xfpnsrv.run newsys:xfpnsrv.run

xfpnsrv.obj:	xfpnsrv.m86

DNSRSLV.RUN: 	DNSRSLV.OBJ
	XLINK XOSLIB:\XOS\_MAINALT DNSRSLV \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/O=DNSRSLV /M=DNSRSLV /SYM=DNSRSLV
	COPY /OVER DNSRSLV.RUN NEWSYS:DNSRSLV.RUN
	COPY /OVER DNSRSLV.SYM NEWSYS:DNSRSLV.SYM

DNSRSLV.OBJ:	DNSRSLV.M86

IPSSRV.RUN:	IPSSRV.OBJ IPSSRVT.OBJ IPSSRVE.OBJ IPSSRVXE.OBJ IPSSRVG.OBJ \
		IPSSRVD.OBJ IPSSRVS.OBJ
	XLINK @IPSSRV.LNK
	COPY /OVER IPSSRV.RUN NEWSYS:IPSSRV.RUN

IPSSRV.OBJ:	IPSSRV.M86

IPSSRVT.OBJ:	IPSSRVT.M86

IPSSRVE.OBJ:	IPSSRVE.M86

IPSSRVXE.OBJ:	IPSSRVXE.M86

IPSSRVG.OBJ:	IPSSRVG.M86

IPSSRVD.OBJ:	IPSSRVD.M86

IPSSRVS.OBJ:	IPSSRVS.M86

BOOTSRV.RUN:	BOOTSRV.OBJ BOOTRARP.OBJ BOOTTFTP.OBJ BOOTBTP.OBJ
	XLINK XOSLIB:\XOS\_MAINALT BOOTSRV BOOTRARP BOOTTFTP BOOTBTP \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/O=BOOTSRV /M=BOOTSRV /SYM=BOOTSRV
	COPY /OVER BOOTSRV.RUN NEWSYS:BOOTSRV.RUN

BOOTSRV.OBJ:	BOOTSRV.M86

BOOTRARP.OBJ:	BOOTRARP.M86

BOOTTFTP.OBJ:	BOOTTFTP.M86

BOOTBTP.OBJ:	BOOTBTP.M86

RARPTEST.RUN:	RARPTEST.OBJ
	XLINK RARPTEST XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
	/O=RARPTEST /M=RARPTEST /SYM=RARPTEST

RARPTEST.OBJ:	RARPTEST.M86

telnet.run:	telnet.obj
	xlink xoslib:\xos\defsegs telnet xoslib:\xos\_mainalt \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=telnet /m=telnet /sym=telnet
	copy /over telnet.run newcmd:telnet.run

telnet.obj:	telnet.m86

tlnsrv.run:	tlnsrv.obj
	xlink xoslib:\xos\defsegs xoslib:\xos\_mainalt tlnsrv \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=tlnsrv /m=tlnsrv /sym=tlnsrv
	copy /over tlnsrv.run newsys:tlnsrv.run

tlnsrv.obj:	tlnsrv.m86

NETSPY.RUN:	NETSPY.OBJ
	XLINK NETSPY \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/O=NETSPY /M=NETSPY /SYM=NETSPY
	COPY /OVER NETSPY.RUN NEWCMD:NETSPY.RUN

NETSPY.OBJ:	NETSPY.M86
