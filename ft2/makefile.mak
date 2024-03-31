.m86.obj:
	XMAC $(@DPN)

.c.obj:
	owxc /hc /d2 /i=$(*p) /i=inc\ /i=xosinc:\owxc\ /zl /zld /zq /5s /j /w3 \
		/fpi87 /fp5 /s /zl /zld /mf /zc /ze /of /okhs /nm=$(@N) \
		$(*DPN)
#	WCC386 /D1 /I=inc\ /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S /OD \
#		/NM=$(@N) $(*DPN)
#	XCC /cO /cg /cWall /cW /cWerror /nolink /lmap=$(@N).map $(@DPN).c

libft201.obj:	ftsystem.obj ftinit.obj ftdebug.obj ftbase.obj \
			ftbbox.obj ftglyph.obj ftbdf.obj \
			ftbitmap.obj ftgxval.obj ftmm.obj \
			ftotval.obj ftpfr.obj ftstroke.obj \
			ftsynth.obj fttype1.obj ftwinfnt.obj \
			fterrmsg.obj \
		bdfdrivr.obj bdflib.obj \
		cffdrivr.obj cffparse.obj cffload.obj \
			cffobjs.obj cffgload.obj cffcmap.obj \
		cidparse.obj cidload.obj cidobjs.obj \
			cidriver.obj cidgload.obj \
		pcfdrivr.obj pcfread.obj pcfutil.obj \
		ttload.obj ttmtx.obj ttcmap.obj ttkern.obj \
			sfobjs.obj sfdriver.obj ttsbit.obj \
			ttpost.obj ttbdf.obj \
		pfrload.obj pfrgload.obj pfrcmap.obj \
			pfrobjs.obj pfrdrivr.obj pfrsbit.obj \
		ttdriver.obj ttobjs.obj ttpload.obj ttgload.obj \
			ttinterp.obj ttgxvar.obj \
		t1driver.obj t1load.obj t1parse.obj t1objs.obj \
		t42objs.obj t42parse.obj t42drivr.obj \
			t1gload.obj t1afm.obj \
		winfnt.obj \
		ftsmooth.obj \
		ftraster.obj ftrend1.obj ftgrays.obj \
		afangles.obj aflatin.obj afcjk.obj afloader.obj afglobal.obj \
			afhints.obj afdummy.obj afmodule.obj afwarp.obj \
		ftcmru.obj ftcmanag.obj ftccache.obj ftccmap.obj \
			ftcglyph.obj ftcimage.obj ftcsbits.obj \
			ftcbasic.obj \
		ftgzip.obj \
		ftlzw.obj \
		psobjs.obj psauxmod.obj t1decode.obj \
			t1cmap.obj afmparse.obj psconv.obj \
		pshrec.obj pshglob.obj pshalgo.obj pshmod.obj \
		psmodule.obj \
		gxvfeat.obj gxvcommn.obj gxvbsln.obj gxvtrak.obj \
			gxvjust.obj gxvmort.obj gxvmort0.obj \
			gxvmort1.obj gxvmort2.obj gxvmort4.obj \
			gxvmort5.obj gxvmorx.obj gxvmorx0.obj \
			gxvmorx1.obj gxvmorx2.obj gxvmorx4.obj \
			gxvmorx5.obj gxvkern.obj gxvopbd.obj \
			gxvprop.obj gxvlcar.obj gxvmod.obj \
		otvbase.obj otvcommn.obj otvgdef.obj otvgpos.obj \
			otvgsub.obj otvjstf.obj otvmod.obj
	xlib libft201 /create /list=libft201 \
		ftinit \
		bdfdrivr bdflib \
		cffdrivr cffobjs cffload cffparse cffgload cffcmap \
		cidriver cidgload cidload cidparse cidobjs \
		pcfdrivr pcfutil pcfread \
		sfdriver sfobjs \
		pfrdrivr pfrobjs pfrload pfrgload pfrcmap pfrsbit \
		ttdriver ttobjs ttpload ttgload ttinterp ttgxvar \
		t1driver t1objs t1load t1parse \
		t42drivr t42objs t42parse t1gload t1afm \
		winfnt \
		ttload ttmtx ttcmap ttkern ttsbit ttpost ttbdf \
		ftrend1 ftraster ftsmooth ftgrays afloader afglobal \
			aflatin afcjk afmodule afhints \
			afangles afwarp afdummy \
#		ftcmru ftcmanag ftccache ftccmap ftcglyph ftcimage \
#			ftcsbits ftcbasic \
		ftgzip \
		ftlzw \
		psauxmod psobjs t1decode t1cmap afmparse psconv \
		pshrec pshglob pshalgo pshmod \
		psmodule \
		gxvfeat gxvcommn gxvbsln gxvtrak gxvjust gxvmort gxvmort0 \
			gxvmort1 gxvmort2 gxvmort4 gxvmort5 gxvmorx gxvmorx0 \
			gxvmorx1 gxvmorx2 gxvmorx4 gxvmorx5 gxvkern gxvopbd \
			gxvprop gxvlcar gxvmod \
		otvbase otvcommn otvgdef otvgpos otvgsub otvjstf otvmod \
		ftbase ftsystem ftdebug ftbbox ftglyph ftbdf \
			ftbitmap ftgxval ftmm ftotval ftpfr ftstroke \
			ftsynth fttype1 ftwinfnt fterrmsg
	copy /over libft201.obj newlib:\xos\libft201.obj


test.run:	test.obj
	xlink xoslib:\xos\defsegs test \
		xoslib:\xos\libft201 xoslib:\xos\libft201 xoslib:\xos\libft201 \
		xoslib:\xos\libft201 xoslib:\xos\libft201 xoslib:\xos\libft201 \
		xoslib:\xos\libft201 xoslib:\xos\libx01 xoslib:\xos\libc01 \
		/output=test /map=test /sym=test
	copy /over test.run newcmd:test.run

test.obj:	test.c


clean:
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

# Base components (required)

ftsystem.obj:	base\ftsystem.c
ftinit.obj:	base\ftinit.c
ftdebug.obj:	base\ftdebug.c
ftbase.obj:	base\ftbase.c
ftbbox.obj:	base\ftbbox.c
ftglyph.obj:	base\ftglyph.c
ftbdf.obj:	base\ftbdf.c
ftbitmap.obj:	base\ftbitmap.c
ftgxval.obj:	base\ftgxval.c
ftmm.obj:	base\ftmm.c
ftotval.obj:	base\ftotval.c
ftpfr.obj:	base\ftpfr.c
ftstroke.obj:	base\ftstroke.c
ftsynth.obj:	base\ftsynth.c
fttype1.obj:	base\fttype1.c
ftwinfnt.obj:	base\ftwinfnt.c
fterrmsg.obj:	base\fterrmsg.c

# Font drivers (optional; at least one is needed)

# Font driver for BDF fonts

bdflib.obj:	bdf\bdflib.c
bdfdrivr.obj:	bdf\bdfdrivr.c

# Font driver for CFF fonts

cffdrivr.obj:	cff\cffdrivr.c
cffparse.obj:	cff\cffparse.c
cffload.obj:	cff\cffload.c
cffobjs.obj:	cff\cffobjs.c
cffgload.obj:	cff\cffgload.c
cffcmap.obj:	cff\cffcmap.c

# Font driver for Type 1 CID fonts

cidparse.obj:	cid\cidparse.c
cidload.obj:	cid\cidload.c
cidobjs.obj:	cid\cidobjs.c
cidriver.obj:	cid\cidriver.c
cidgload.obj:	cid\cidgload.c

# Fond driver for PCF fonts

pcfutil.obj:	pcf\pcfutil.c
pcfread.obj:	pcf\pcfread.c
pcfdrivr.obj:	pcf\pcfdrivr.c

# Font driver for PFR fonts

pfrload.obj:	pfr\pfrload.c
pfrgload.obj:	pfr\pfrgload.c
pfrcmap.obj:	pfr\pfrcmap.c
pfrobjs.obj:	pfr\pfrobjs.c
pfrdrivr.obj:	pfr\pfrdrivr.c
pfrsbit.obj:	pfr\pfrsbit.c

# Font driver for SFNT fonts

ttload.obj:	sfnt\ttload.c
ttmtx.obj:	sfnt\ttmtx.c
ttcmap.obj:	sfnt\ttcmap.c
ttkern.obj:	sfnt\ttkern.c
sfobjs.obj:	sfnt\sfobjs.c
sfdriver.obj:	sfnt\sfdriver.c
ttsbit.obj:	sfnt\ttsbit.c
ttpost.obj:	sfnt\ttpost.c
ttbdf.obj:	sfnt\ttbdf.c

# Font driver for TrueType fonts

ttdriver.obj:	truetype\ttdriver.c
ttpload.obj:	truetype\ttpload.c
ttgload.obj:	truetype\ttgload.c
ttobjs.obj:	truetype\ttobjs.c
### FOLLOWING IS THE HINT PROCESSOR!!!
ttinterp.obj:	truetype\ttinterp.c
ttgxvar.obj:	truetype\ttgxvar.c

# Font driver for Type 1 PS fonts

t1parse.obj:	type1\t1parse.c
t1load.obj:	type1\t1load.c
t1objs.obj:	type1\t1objs.c
t1driver.obj:	type1\t1driver.c
t1gload.obj:	type1\t1gload.c
t1afm.obj:	type1\t1afm.c

# Font driver for Type 42 fonts

t42objs.obj:	type42\t42objs.c
t42parse.obj:	type42\t42parse.c
t42drivr.obj:	type42\t42drivr.c

# Font driver for Windows fonts

winfnt.obj:	win\winfnt.c

# Rasterizers

ftraster.obj:	raster\ftraster.c
ftrend1.obj:	raster\ftrend1.c
ftgrays.obj:	raster\ftgrays.c
ftsmooth.obj:	raster\ftsmooth.c

# Autofit module

afangles.obj:	autofit\afangles.c
afglobal.obj:	autofit\afglobal.c
afhints.obj:	autofit\afhints.c
afdummy.obj:	autofit\afdummy.c
aflatin.obj:	autofit\aflatin.c
afcjk.obj:	autofit\afcjk.c
afloader.obj:	autofit\afloader.c
afmodule.obj:	autofit\afmodule.c
afwarp.obj:	autofit\afwarp.c

# Cache module

ftcmru.obj:	cache\ftcmru.c
ftcmanag.obj:	cache\ftcmanag.c
ftccache.obj:	cache\ftccache.c
ftccmap.obj:	cache\ftccmap.c
ftcglyph.obj:	cache\ftcglyph.c
ftcimage.obj:	cache\ftcimage.c
ftcsbits.obj:	cache\ftcsbits.c
ftcbasic.obj:	cache\ftcbasic.c

# ZGIP module

ftgzip.obj:	gzip\ftgzip.c gzip\zutil.c gzip\inftrees.c \
		gzip\infutil.c gzip\infcodes.c gzip\infblock.c \
		gzip\inflate.c gzip\adler32.c
	owxc /hc /d2 /i=gzip /i=inc\ /i=xosinc:\owxc\ /zl /zld /zq /5s /j /w3 \
		/fpi87 /fp5 /s /zl /zld /mf /zc /ze /of /okhs /nm=ftgzip \
		gzip\ftgzip
#	WCC386 /D1 /I=inc\ /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S /OD \
#		/NM=ftgzip gzip\ftgzip

# LZW moudle

ftlzw.obj:	lzw\ftlzw.c lzw\ftzopen.c
	owxc /hc /d2 /i=lzw /i=inc\ /i=xosinc:\owxc\ /zl /zld /zq /5s /j /w3 \
		/fpi87 /fp5 /s /zl /zld /mf /zc /ze /of /okhs /nm=ftlzw \
		lzw\ftlzw
#	WCC386 /D1 /I=inc\ /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S /OD \
#		/NM=ftlzw lzw\ftlzw

# GXvalid module

#gxvalid\gxvalid.c

gxvfeat.obj:	gxvalid\gxvfeat.c
gxvcommn.obj:	gxvalid\gxvcommn.c
gxvbsln.obj:	gxvalid\gxvbsln.c
gxvtrak.obj:	gxvalid\gxvtrak.c
gxvjust.obj:	gxvalid\gxvjust.c
gxvmort.obj:	gxvalid\gxvmort.c
gxvmort0.obj:	gxvalid\gxvmort0.c
gxvmort1.obj:	gxvalid\gxvmort1.c
gxvmort2.obj:	gxvalid\gxvmort2.c
gxvmort4.obj:	gxvalid\gxvmort4.c
gxvmort5.obj:	gxvalid\gxvmort5.c
gxvmorx.obj:	gxvalid\gxvmorx.c
gxvmorx0.obj:	gxvalid\gxvmorx0.c
gxvmorx1.obj:	gxvalid\gxvmorx1.c
gxvmorx2.obj:	gxvalid\gxvmorx2.c
gxvmorx4.obj:	gxvalid\gxvmorx4.c
gxvmorx5.obj:	gxvalid\gxvmorx5.c
gxvkern.obj:	gxvalid\gxvkern.c
gxvopbd.obj:	gxvalid\gxvopbd.c
gxvprop.obj:	gxvalid\gxvprop.c
gxvlcar.obj:	gxvalid\gxvlcar.c
gxvmod.obj:	gxvalid\gxvmod.c

# OTvalid modue

otvbase.obj:	otvalid\otvbase.c
otvcommn.obj:	otvalid\otvcommn.c
otvgdef.obj:	otvalid\otvgdef.c
otvgpos.obj:	otvalid\otvgpos.c
otvgsub.obj:	otvalid\otvgsub.c
otvjstf.obj:	otvalid\otvjstf.c
otvmod.obj:	otvalid\otvmod.c

# PSAUX module

psobjs.obj:	psaux\psobjs.c
psauxmod.obj:	psaux\psauxmod.c
t1decode.obj:	psaux\t1decode.c
t1cmap.obj:	psaux\t1cmap.c
afmparse.obj:	psaux\afmparse.c
psconv.obj:	psaux\psconv.c

# PSHINTER module

pshrec.obj:	pshinter\pshrec.c
pshglob.obj:	pshinter\pshglob.c
pshalgo.obj:	pshinter\pshalgo.c
pshmod.obj:	pshinter\pshmod.c

# PSNAMES moduel

psmodule.obj:	psnames\psmodule.c
