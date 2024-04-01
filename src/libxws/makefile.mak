.m86.obj:
	xmac $(@DPN)

.c.obj:
	owxc /hc /d2 /i=xosinc:\owxc\ /i=xosinc:\ft2\ /zl /zld /zq /6s \
		/zpw /za99 /w3 /we /oarfkht /fp3 /s /mf /zc /ze \
		/nm=$(@DPN) $(@DPN).c

libxws03.obj:	xws.obj xws1.obj events.obj vesaa.obj \
		cursor.obj mouse.obj dummymouse.obj keyboard.obj screen.obj \
		timer.obj container.obj button.obj list.obj \
		edit.obj scrollbar.obj parms.obj dfltparms.obj \
		ellipse.obj line.obj rect.obj circle.obj display.obj \
		textput.obj textstore.obj textformat.obj textdisplay.obj \
		textsta.obj display1.obj font1.obj font2.obj \
		window.obj window1.obj region.obj print.obj bitmap1.obj \
		bitmapload.obj bitmap.obj \
		bitmapbmp.obj bitmapjpeg.obj bitmappng.obj bitmapgif.obj \
		bitmapmono.obj \
		bitmapscale.obj bitmapscalecolorlilow.obj memory.obj \
		bitmapscalesmoothlow.obj debug1.obj debug2.obj
	xlib libxws03/create/list=libxws03 \
		xws xws1 events vesaa cursor mouse dummymouse \
		keyboard screen timer container button list \
		edit scrollbar ellipse line parms dfltparms rect circle \
		display font1 font2 window window1 textput textstore \
		textformat textdisplay textsta display1 \
		region print bitmap1 bitmapload bitmap bitmapbmp \
		bitmapjpeg bitmappng bitmapgif bitmapmono bitmapscale \
		bitmapscalecolorlilow bitmapscalesmoothlow memory.obj debug1 \
		debug2
	copy /over libxws03.obj newlib:\xos\libxws03.obj

xws.obj:	xws.m86
xws1.obj:	xws1.c
events.obj:	events.m86
cursor.obj:	cursor.m86
mouse.obj:	mouse.m86
dummymouse.obj:	dummymouse.m86
keyboard.obj:	keyboard.m86
screen.obj:	screen.c
timer.obj:	timer.c
container.obj:	container.c
button.obj:	button.c
list.obj:	list.c
edit.obj:	edit.c
scrollbar.obj:	scrollbar.c
ellipse.obj:	ellipse.m86
line.obj:	line.m86
parms.obj:	parms.m86
dfltparms.obj:	dfltparms.c
rect.obj:	rect.m86
circle.obj:	circle.m86
window.obj:	window.m86
window1.obj:	window1.c
memory.obj:	memory.m86
display.obj:	display.m86
display1.obj:	display1.c
font1.obj:	font1.c
font2.obj:	font2.m86
textput.obj:	textput.c
textputa.obj:	textlst.c
textstore.obj:	textstore.c
textformat.obj:	textformat.c
textdisplay.obj: textdisplay.c
textsta.obj:	textsta.c
region.obj:	region.m86
print.obj:	print.c
bitmapload.obj:	bitmapload.c
bitmap.obj:	bitmap.m86
bitmap1.obj:	bitmap1.c
bitmapbmp.obj:	bitmapbmp.m86
bitmapjpeg.obj:	bitmapjpeg.c
bitmappng.obj:	bitmappng.c
bitmapgif.obj:	bitmapgif.c
bitmapmono.obj:	bitmapmono.m86
bitmapscale.obj: bitmapscale.c
bitmapscalecolorlilow.obj: bitmapscalecolorlilow.c
bitmapscalesmoothlow.obj: bitmapscalesmoothlow.c
#notify.obj:	notify.m86
s3a.obj:	s3a.m86
atib.obj:	atib.m86
vesaa.obj:	vesaa.m86
debug1.obj:	debug1.m86
debug2.obj:	debug2.c


XWSSRV.RUN: XWSSRV.OBJ XWSSRVC.OBJ
	XLINK XOSLIB:XOS\_MAINALT XWSSRV XWSSRVC \
		XOSLIB:XOS\XWS01 XOSLIB:\XOS\LIBFT01 \
		XOSLIB:XOS\LIBX01 XOSLIB:XOS\LIBC01 \
		/O=XWSSRV /M=XWSSRV /SYM=XWSSRV
	COPY /OVER XWSSRV.RUN XOSSYS:XWSSRV.RUN

XWSSRV.OBJ:	XWSSRV.M86

XWSSRVC.OBJ:	XWSSRVC.M86

utf8tool.run:	utf8tool.obj
	XLINK XOSLIB:\xos\defsegs utf8tool \
		XOSLIB:\xos\libx01 XOSLIB:\xos\libc01 \
		/OUTPUT=utf8tool /MAP=utf8tool /SYM=utf8tool

utf8tool.obj:	utf8tool.c



GTATI.RUN:	GTATI.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS GTATI XOSLIB:\XOS\XWS01 \
		XOSLIB:\XOS\LIBFT01 \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=GTATI /MAP=GTATI /SYM=GTATI

GTATI.OBJ:	GTATI.C

GTS3.RUN:	GTS3.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS GTS3 XOSLIB:\XOS\XWS01 \
		XOSLIB:\XOS\LIBFT01 \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=GTS3 /MAP=GTS3 /SYM=GTS3

GTS3.OBJ:	GTS3.C

ATITEST.RUN:	ATITEST.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS ATITEST XOSLIB:\XOS\XWS01 \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=ATITEST /MAP=ATITEST /SYM=ATITEST

ATITEST.OBJ:	ATITEST.C

ptrdmp.run:	ptrdmp.obj
	xlink xoslib:\xos\defsegs ptrdmp xoslib:\xos\xws01 \
		xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=ptrdmp /map=ptrdmp /sym=ptrdmp

ptrdmp.OBJ:	ptrdmp.C

vesatest.run:	vesatest.obj libxws03.obj
	xlink xoslib:\xos\defsegs vesatest xoslib:\xos\libxws03 \
		xoslib:\xos\libjpeg6b xoslib:\xos\libjpeg6b \
		xoslib:\xos\libgif30 \
		xoslib:\xos\libft201 xoslib:\xos\libft201 \
		xoslib:\xos\libx01 \
		xoslib:\xos\libc01 \
		/output=vesatest /map=vesatest /sym=vesatest
	copy /over vesatest.run xoscmd:vesatest.run

vesatest.obj:	vesatest.c

ptrtest.run:	ptrtest.obj libxws03.obj
	xlink xoslib:\xos\defsegs ptrtest xws vesaa xoslib:\xos\libxws03 \
		xoslib:\xos\libjpeg6b xoslib:\xos\libjpeg6b \
		xoslib:\xos\libgif30 \
		xoslib:\xos\libft201 xoslib:\xos\libft201 \
		xoslib:\xos\libx01 \
		xoslib:\xos\libc01 \
		/output=ptrtest /map=ptrtest /sym=ptrtest
	copy /over ptrtest.run xoscmd:ptrtest.run

ptrtest.obj:	ptrtest.c

VESATESTMIN.RUN:	VESATESTMIN.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS VESATESTMIN XOSLIB:\XOS\XWS01 \
		XOSLIB:\XOS\LIBFT01 XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=VESATESTMIN /MAP=VESATESTMIN /SYM=VESATESTMIN
	COPY /OVER VESATESTMIN.RUN XOSCMD:VESATESTMIN.RUN

VESATESTMIN.OBJ:	VESATESTMIN.C

OVRTEST.RUN:	OVRTEST.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS OVRTEST XOSLIB:\XOS\XWS01 \
		XOSLIB:\XOS\FREETYPE XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=OVRTEST /MAP=OVRTEST /SYM=OVRTEST

OVRTEST.OBJ:	OVRTEST.C

TXTTEST.RUN:	TXTTEST.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS TXTTEST XOSLIB:\XOS\XWS01 \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=TXTTEST /MAP=TXTTEST /SYM=TXTTEST

TXTTEST.OBJ:	TXTTEST.C

TTTEST.RUN:	TTTEST.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS TTTEST XOSLIB:\XOS\FREETYPE \
		XOSLIB:\XOS\XWS01 XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=TTTEST /MAP=TTTEST /SYM=TTTEST

TTTEST.OBJ:	TTTEST.C

PTEST.RUN:	PTEST.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS PTEST XOSLIB:\XOS\XWS01 \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=PTEST /MAP=PTEST /SYM=PTEST

PTEST.OBJ:	PTEST.C

PRTTEST.RUN:	PRTTEST.OBJ PRTASM.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS PRTTEST PRTASM XOSLIB:\XOS\XWS01 \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=PRTTEST /MAP=PRTTEST /SYM=PRTTEST

PRTTEST.OBJ:	PRTTEST.C

PRTASM.OBJ:	PRTASM.M86

XTEST.RUN:	XTEST.OBJ XWS01.OBJ
	XLINK XOSLIB:\XOS\DEFSEGS XTEST XOSLIB:\XOS\XWS01 \
		XOSLIB:\XOS\LIBX01 XOSLIB:\XOS\LIBC01 \
		/OUTPUT=XTEST /MAP=XTEST /SYM=XTEST

XTEST.OBJ:	XTEST.C
