.M86.OBJ:
	XMAC $(@DPN)

.C.OBJ:
	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /OD /S \
		/NM=$(@DPN) $(@DPN)

ALL:		FTPSRV.RUN FTP.RUN
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

ftpsrv.run:	ftpsrv.obj tcp.obj client.obj command.obj datacon.obj \
		utility.obj
	xlink xoslib:\xos\defsegs xoslib:\xos\_mainalt ftpsrv tcp client \
		command datacon utility \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=ftpsrv /m=ftpsrv /sym=ftpsrv
	copy /over ftpsrv.run newsys:ftpsrv.run

ftpsrv.obj:	ftpsrv.c
tcp.obj:	tcp.c
client.obj:	client.c
command.obj:	command.c
datacon.obj:	datacon.c
utility.obj:	utility.c

ftp.run:	ftp.obj
	xlink xoslib:\xos\defsegs ftp \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/o=ftp /m=ftp /sym=ftp
	copy /over ftp.run newcmd:ftp.run

ftp.obj:	ftp.c
