MV=1
NV=8
EN=4
VENDOR=XOS

# 1.8.3           First XOS release
# 1.8.4 26-Aug-10 Added import and export pragmas, removed some unneeded comment
#                 output

.M86.OBJ:
	XMAC $(*DPN) /out=$(@DPN).obj

.C.OBJ:
	owxc /i=inc\ /i=inc\wasm\ /hc /d1+ /zq /6s /w3 /we /fpi87 /fp3 /s /mf \
		/za99 /ze /oi /oarfkhs /bt=XOS \
		/dNDEBUG /dMV=$(MV) /dNV=$(NV) /dEN=$(EN) \
		/fo=$(@DPN).obj /nm=$(@N) $(*DPN).c

all:		libs\cg386.obj libs\cg386lnx.obj libs\cf.obj libs\dw.obj \
		owxcx.run
	@echo Done

clean:
	-del cgobj\*.obj
	-del ccobj\*.obj
	-del libs\*.obj
	-del owxcx.run

owxcx.run:	ccobj\ascii.obj ccobj\asmstmt.obj ccobj\ccheck.obj \
		ccobj\ccmain.obj ccobj\cdata.obj ccobj\cdebug.obj \
		ccobj\cdecl1.obj ccobj\cdecl2.obj ccobj\cdinit.obj \
		ccobj\cdump.obj ccobj\cems.obj ccobj\cenum.obj \
		ccobj\cerror.obj ccobj\cfeinfo.obj ccobj\cinfo.obj \
		ccobj\cmac1.obj ccobj\cmac2.obj ccobj\cmemmgr.obj \
		ccobj\cmodel.obj ccobj\cmsg.obj ccobj\cname.obj \
		ccobj\coptions.obj ccobj\caux.obj ccobj\cpragma.obj \
		ccobj\cpurge.obj ccobj\cscan.obj ccobj\csizeof.obj \
		ccobj\cstats.obj ccobj\cstring.obj ccobj\csym.obj \
		ccobj\ctimepc.obj ccobj\ctoken.obj ccobj\ctype.obj \
		ccobj\cusage.obj ccobj\cutil.obj ccobj\ftoa.obj \
		ccobj\strsave.obj ccobj\swchar.obj ccobj\watcom.obj \
		ccobj\cgetch.obj ccobj\cmacadd.obj ccobj\asmalloc.obj \
		ccobj\asmmsg.obj ccobj\jis2unic.obj ccobj\pchdr.obj \
		ccobj\dwarf.obj ccobj\browsio.obj ccobj\intlload.obj \
		ccobj\autodept.obj ccobj\cexpr.obj ccobj\cfold.obj \
		ccobj\cmath.obj ccobj\cstmt.obj ccobj\cgen.obj \
		ccobj\cgendata.obj ccobj\exprtree.obj ccobj\treewalk.obj \
		ccobj\mngless.obj ccobj\asmins.obj ccobj\asmscan.obj \
		ccobj\asmline.obj ccobj\asmsym.obj ccobj\tbyte.obj \
		ccobj\asmlabel.obj ccobj\asmfixup.obj ccobj\asmmatch.obj \
		ccobj\asmdata.obj ccobj\asmjump.obj ccobj\asmexpnd.obj \
		ccobj\asmeval.obj ccobj\cpragx86.obj ccobj\prag386.obj \
		ccobj\cintmain.obj ccobj\setdbchar.obj ccobj\strtod.obj \
		ccobj\ldcvt.obj ccobj\inf_nan.obj ccobj\ldclass.obj \
		ccobj\hugeval.obj ccobj\seterrno.obj ccobj\bufld386.obj \
		libs\dw.obj libs\cg386.obj libs\cg386lnx.obj libs\cf.obj
#	gecko \xossrc\xlink\xxlink newlib:xos\defsegs \
	xlink newlib:xos\defsegs \
		ccobj\ascii ccobj\asmstmt ccobj\ccheck \
		ccobj\ccmain ccobj\cdata ccobj\cdebug \
		ccobj\cdecl1 ccobj\cdecl2 ccobj\cdinit \
		ccobj\cdump ccobj\cems ccobj\cenum \
		ccobj\cerror ccobj\cfeinfo ccobj\cinfo \
		ccobj\cmac1 ccobj\cmac2 ccobj\cmemmgr \
		ccobj\cmodel ccobj\cmsg ccobj\cname \
		ccobj\coptions ccobj\caux ccobj\cpragma \
		ccobj\cpurge ccobj\cscan ccobj\csizeof \
		ccobj\cstats ccobj\cstring ccobj\csym \
		ccobj\ctimepc ccobj\ctoken ccobj\ctype \
		ccobj\cusage ccobj\cutil ccobj\ftoa \
		ccobj\strsave ccobj\swchar ccobj\watcom \
		ccobj\cgetch ccobj\cmacadd ccobj\asmalloc \
		ccobj\asmmsg ccobj\jis2unic ccobj\pchdr \
		ccobj\dwarf ccobj\browsio ccobj\intlload \
		ccobj\autodept ccobj\cexpr ccobj\cfold \
		ccobj\cmath ccobj\cstmt ccobj\cgen \
		ccobj\cgendata ccobj\exprtree ccobj\treewalk \
		ccobj\mngless ccobj\asmins ccobj\asmscan \
		ccobj\asmline ccobj\asmsym ccobj\tbyte \
		ccobj\asmlabel ccobj\asmfixup ccobj\asmmatch \
		ccobj\asmdata ccobj\asmjump ccobj\asmexpnd \
		ccobj\asmeval ccobj\cpragx86 ccobj\prag386 \
		ccobj\cintmain ccobj\setdbchar ccobj\strtod \
		ccobj\ldcvt ccobj\inf_nan ccobj\ldclass \
		ccobj\hugeval ccobj\seterrno ccobj\bufld386 \
		libs\dw libs\cg386 libs\cg386lnx libs\cf \
		libs\dw libs\cg386 libs\cg386lnx libs\cf \
		libs\dw libs\cg386 libs\cg386lnx libs\cf \
		newlib:\xos\libc01 \
		/output=owxcx /map=owxcx /sym=owxcx
	copy /over owxcx.run NEWCMD:owxc.run

# reference DumpTree_, DumpCurrLoop_, DumpInsList_, DumpString_, DumpOpt_,
#	DumpOpcodeName_, DumpRegTree_, DumpSc_, DumpADataDag_, DumpGen_,
#	Dumpan_, DumpBlk_, DumpConflicts_

ccobj\ascii.obj:	cc\ascii.c
ccobj\asmstmt.obj:	cc\asmstmt.c
ccobj\ccheck.obj:	cc\ccheck.c
ccobj\ccmain.obj:	cc\ccmain.c
ccobj\cdata.obj:	cc\cdata.c
ccobj\cdebug.obj:	cc\cdebug.c
ccobj\cdecl1.obj:	cc\cdecl1.c
ccobj\cdecl2.obj:	cc\cdecl2.c
ccobj\cdinit.obj:	cc\cdinit.c
ccobj\cdump.obj:	cc\cdump.c
ccobj\cems.obj:		cc\cems.c
ccobj\cenum.obj:	cc\cenum.c
ccobj\cerror.obj:	cc\cerror.c
ccobj\cfeinfo.obj:	cc\cfeinfo.c
ccobj\cinfo.obj:	cc\cinfo.c
ccobj\cmac1.obj:	cc\cmac1.c
ccobj\cmac2.obj:	cc\cmac2.c
ccobj\cmemmgr.obj:	cc\cmemmgr.c
ccobj\cmodel.obj:	cc\cmodel.c
ccobj\cmsg.obj:		cc\cmsg.c
ccobj\cname.obj:	cc\cname.c
ccobj\coptions.obj:	cc\coptions.c
ccobj\caux.obj:		cc\caux.c
ccobj\cpragma.obj:	cc\cpragma.c
ccobj\cpurge.obj:	cc\cpurge.c
ccobj\cscan.obj:	cc\cscan.c
ccobj\csizeof.obj:	cc\csizeof.c
ccobj\cstats.obj:	cc\cstats.c
ccobj\cstring.obj:	cc\cstring.c
ccobj\csym.obj:		cc\csym.c
ccobj\ctimepc.obj:	cc\ctimepc.c
ccobj\ctoken.obj:	cc\ctoken.c
ccobj\ctype.obj:	cc\ctype.c
ccobj\cusage.obj:	cc\cusage.c
ccobj\cutil.obj:	cc\cutil.c
ccobj\ftoa.obj:		cc\ftoa.c
ccobj\strsave.obj:	cc\strsave.c
ccobj\swchar.obj:	watcom\swchar.c
ccobj\watcom.obj:	cc\watcom.c
ccobj\cgetch.obj:	cc\cgetch.c
ccobj\cmacadd.obj:	cc\cmacadd.c
ccobj\asmalloc.obj:	cc\asmalloc.c
ccobj\asmmsg.obj:	cc\asmmsg.c
ccobj\jis2unic.obj:	cc\jis2unic.c
ccobj\pchdr.obj:	cc\pchdr.c
ccobj\dwarf.obj:	cc\dwarf.c
ccobj\browsio.obj:	cc\browsio.c
ccobj\intlload.obj:	cc\intlload.c
ccobj\autodept.obj:	watcom\autodept.c
ccobj\cexpr.obj:	cc\cexpr.c
ccobj\cfold.obj:	cc\cfold.c
ccobj\cmath.obj:	cc\cmath.c
ccobj\cstmt.obj:	cc\cstmt.c
ccobj\cgen.obj:		cc\cgen.c
ccobj\cgendata.obj:	cc\cgendata.c
ccobj\exprtree.obj:	cc\exprtree.c
ccobj\treewalk.obj:	cc\treewalk.c
ccobj\mngless.obj:	watcom\mngless.c
ccobj\asmins.obj:	cc\asmins.c
ccobj\asmscan.obj:	cc\asmscan.c
ccobj\asmline.obj:	cc\asmline.c
ccobj\asmsym.obj:	cc\asmsym.c
ccobj\tbyte.obj:	cc\tbyte.c
ccobj\asmlabel.obj:	cc\asmlabel.c
ccobj\asmfixup.obj:	cc\asmfixup.c
ccobj\asmmatch.obj:	cc\asmmatch.c
ccobj\asmdata.obj:	cc\asmdata.c
ccobj\asmjump.obj:	cc\asmjump.c
ccobj\asmexpnd.obj:	cc\asmexpnd.c
ccobj\asmeval.obj:	cc\asmeval.c
ccobj\cpragx86.obj:	cc\cpragx86.c
ccobj\prag386.obj:	cc\prag386.c
ccobj\cintmain.obj:	cc\cintmain.c
ccobj\setdbchar.obj:	cc\setdbchar.c
ccobj\strtod.obj:	mathlib\strtod.c
ccobj\ldcvt.obj:	cc\ldcvt.c
ccobj\inf_nan.obj:	cc\inf_nan.c
ccobj\ldclass.obj:	cc\ldclass.c
ccobj\hugeval.obj:	cc\hugeval.c
ccobj\seterrno.obj:	cc\seterrno.c
ccobj\bufld386.obj:	cc\bufld386.m86

#dw.obj
#cg386.obj
#cg386lnx.obj
#cf.obj


cg386.obj:	libs\cg386.obj
	@echo Done

libs\cg386.obj:	cgobj\echoapi.obj \
		cgobj\dump87.obj \
		cgobj\dumptree.obj \
		cgobj\dumptab.obj \
		cgobj\dumpschd.obj \
		cgobj\dumpsc.obj \
		cgobj\dumpregt.obj \
		cgobj\dumpref.obj \
		cgobj\dumpopt.obj \
		cgobj\dumpio.obj \
		cgobj\dumpins.obj \
		cgobj\dumpindv.obj \
		cgobj\dumpconf.obj \
		cgobj\dumpblk.obj \
		cgobj\dumpan.obj \
		cgobj\memmgt.obj \
		cgobj\memwrap.obj \
		cgobj\zoiks.obj \
		cgobj\verify.obj \
		cgobj\varusage.obj \
		cgobj\utils.obj \
		cgobj\unroll.obj \
		cgobj\u32moddi.obj \
		cgobj\types.obj \
		cgobj\typemap.obj \
		cgobj\typeinit.obj \
		cgobj\treefold.obj \
		cgobj\treeconv.obj \
		cgobj\tree.obj \
		cgobj\trecurse.obj \
		cgobj\temps.obj \
		cgobj\splitvar.obj \
		cgobj\split.obj \
		cgobj\spawn.obj \
		cgobj\cgsrtlst.obj \
		cgobj\sczero.obj \
		cgobj\scutil.obj \
		cgobj\scthrash.obj \
		cgobj\scregs.obj \
		cgobj\scmain.obj \
		cgobj\scins.obj \
		cgobj\scinfo.obj \
		cgobj\scdata.obj \
		cgobj\scblock.obj \
		cgobj\rtrtn.obj \
		cgobj\revcond.obj \
		cgobj\regtree.obj \
		cgobj\regsave.obj \
		cgobj\regalloc.obj \
		cgobj\redefby.obj \
		cgobj\propind.obj \
		cgobj\peepopt.obj \
		cgobj\patch.obj \
		cgobj\parm.obj \
		cgobj\overlap.obj \
		cgobj\optvlbl.obj \
		cgobj\optutil.obj \
		cgobj\opttell.obj \
		cgobj\optrel.obj \
		cgobj\optq.obj \
		cgobj\optpush.obj \
		cgobj\optpull.obj \
		cgobj\optmkins.obj \
		cgobj\optmain.obj \
		cgobj\optlbl.obj \
		cgobj\optins.obj \
		cgobj\optimize.obj \
		cgobj\optdata.obj \
		cgobj\optcom.obj \
		cgobj\optask.obj \
		cgobj\optab.obj \
		cgobj\objname.obj \
		cgobj\object.obj \
		cgobj\nullprop.obj \
		cgobj\namelist.obj \
		cgobj\multiply.obj \
		cgobj\memtrack.obj \
		cgobj\makeins.obj \
		cgobj\makeblk.obj \
		cgobj\makeaddr.obj \
		cgobj\loopopts.obj \
		cgobj\loadstor.obj \
		cgobj\liveinfo.obj \
		cgobj\intrface.obj \
		cgobj\insutil.obj \
		cgobj\inssched.obj \
		cgobj\insdead.obj \
		cgobj\inline.obj \
		cgobj\i64.obj \
		cgobj\generate.obj \
		cgobj\freelist.obj \
		cgobj\foldins.obj \
		cgobj\flowsave.obj \
		cgobj\flood.obj \
		cgobj\flograph.obj \
		cgobj\fixindex.obj \
		cgobj\expand.obj \
		cgobj\encode.obj \
		cgobj\edge.obj \
		cgobj\dominate.obj \
		cgobj\display.obj \
		cgobj\dftypes.obj \
		cgobj\dfsyms.obj \
		cgobj\dbtypes.obj \
		cgobj\dbtable.obj \
		cgobj\dbsupp.obj \
		cgobj\dbsyms.obj \
		cgobj\dataflo.obj \
		cgobj\data.obj \
		cgobj\cvtypes.obj \
		cgobj\cvsyms.obj \
		cgobj\cse.obj \
		cgobj\conflict.obj \
		cgobj\condcode.obj \
		cgobj\cgaux.obj \
		cgobj\cachecon.obj \
		cgobj\breakrtn.obj \
		cgobj\blktrim.obj \
		cgobj\bldsel.obj \
		cgobj\bldins.obj \
		cgobj\bldcall.obj \
		cgobj\addrfold.obj \
		cgobj\addrcnst.obj \
		cgobj\parmreg.obj \
		cgobj\rtcall.obj \
		cgobj\wvtypes.obj \
		cgobj\wvsyms.obj \
		cgobj\i87table.obj \
		cgobj\i87sched.obj \
		cgobj\i87reg.obj \
		cgobj\i87opt.obj \
		cgobj\i87exp.obj \
		cgobj\i87data.obj \
		cgobj\i86ver.obj \
		cgobj\i86temps.obj \
		cgobj\i86stubs.obj \
		cgobj\i86splt3.obj \
		cgobj\i86split.obj \
		cgobj\i86sel.obj \
		cgobj\i86segs.obj \
		cgobj\i86rtlk.obj \
		cgobj\i86regsv.obj \
		cgobj\i86reg.obj \
		cgobj\i86proc.obj \
		cgobj\i86opcod.obj \
		cgobj\i86obj.obj \
		cgobj\i86mul.obj \
		cgobj\i86lesds.obj \
		cgobj\i86ldstr.obj \
		cgobj\i86index.obj \
		cgobj\i86half.obj \
		cgobj\i86esc.obj \
		cgobj\i86enc2.obj \
		cgobj\i86enc.obj \
		cgobj\i86dbsup.obj \
		cgobj\i86data.obj \
		cgobj\i86dfsup.obj \
		cgobj\i86cvsup.obj \
		cgobj\i86call.obj \
		cgobj\i86base.obj \
		cgobj\386setcc.obj \
		cgobj\386funit.obj \
		cgobj\i87wait.obj \
		cgobj\sib.obj \
		cgobj\i86enc32.obj \
		cgobj\386type.obj \
		cgobj\386tls.obj \
		cgobj\386table.obj \
		cgobj\386splt2.obj \
		cgobj\386sib.obj \
		cgobj\386score.obj \
		cgobj\386rtrtn.obj \
		cgobj\386rgtbl.obj \
		cgobj\386ptype.obj \
		cgobj\386optab.obj \
		cgobj\386opseg.obj \
		cgobj\386ilen.obj \
		cgobj\386conv.obj
	xlib libs\cg386 /create/list=cg386 \
		cgobj\echoapi \
		cgobj\dump87 \
		cgobj\dumptree \
		cgobj\dumptab \
		cgobj\dumpschd \
		cgobj\dumpsc \
		cgobj\dumpregt \
		cgobj\dumpref \
		cgobj\dumpopt \
		cgobj\dumpio \
		cgobj\dumpins \
		cgobj\dumpindv \
		cgobj\dumpconf \
		cgobj\dumpblk \
		cgobj\dumpan \
		cgobj\memmgt \
		cgobj\memwrap \
		cgobj\zoiks \
		cgobj\verify \
		cgobj\varusage \
		cgobj\utils \
		cgobj\unroll \
		cgobj\u32moddi \
		cgobj\types \
		cgobj\typemap \
		cgobj\typeinit \
		cgobj\treefold \
		cgobj\treeconv \
		cgobj\tree \
		cgobj\trecurse \
		cgobj\temps \
		cgobj\splitvar \
		cgobj\split \
		cgobj\spawn \
		cgobj\cgsrtlst \
		cgobj\sczero \
		cgobj\scutil \
		cgobj\scthrash \
		cgobj\scregs \
		cgobj\scmain \
		cgobj\scins \
		cgobj\scinfo \
		cgobj\scdata \
		cgobj\scblock \
		cgobj\rtrtn \
		cgobj\revcond \
		cgobj\regtree \
		cgobj\regsave \
		cgobj\regalloc \
		cgobj\redefby \
		cgobj\propind \
		cgobj\peepopt \
		cgobj\patch \
		cgobj\parm \
		cgobj\overlap \
		cgobj\optvlbl \
		cgobj\optutil \
		cgobj\opttell \
		cgobj\optrel \
		cgobj\optq \
		cgobj\optpush \
		cgobj\optpull \
		cgobj\optmkins \
		cgobj\optmain \
		cgobj\optlbl \
		cgobj\optins \
		cgobj\optimize \
		cgobj\optdata \
		cgobj\optcom \
		cgobj\optask \
		cgobj\optab \
		cgobj\objname \
		cgobj\object \
		cgobj\nullprop \
		cgobj\namelist \
		cgobj\multiply \
		cgobj\memtrack \
		cgobj\makeins \
		cgobj\makeblk \
		cgobj\makeaddr \
		cgobj\loopopts \
		cgobj\loadstor \
		cgobj\liveinfo \
		cgobj\intrface \
		cgobj\insutil \
		cgobj\inssched \
		cgobj\insdead \
		cgobj\inline \
		cgobj\i64 \
		cgobj\generate \
		cgobj\freelist \
		cgobj\foldins \
		cgobj\flowsave \
		cgobj\flood \
		cgobj\flograph \
		cgobj\fixindex \
		cgobj\expand \
		cgobj\encode \
		cgobj\edge \
		cgobj\dominate \
		cgobj\display \
		cgobj\dftypes \
		cgobj\dfsyms \
		cgobj\dbtypes \
		cgobj\dbtable \
		cgobj\dbsupp \
		cgobj\dbsyms \
		cgobj\dataflo \
		cgobj\data \
		cgobj\cvtypes \
		cgobj\cvsyms \
		cgobj\cse \
		cgobj\conflict \
		cgobj\condcode \
		cgobj\cgaux \
		cgobj\cachecon \
		cgobj\breakrtn \
		cgobj\blktrim \
		cgobj\bldsel \
		cgobj\bldins \
		cgobj\bldcall \
		cgobj\addrfold \
		cgobj\addrcnst \
		cgobj\wvtypes \
		cgobj\wvsyms \
		cgobj\parmreg \
		cgobj\i87table \
		cgobj\i87sched \
		cgobj\i87reg \
		cgobj\i87opt \
		cgobj\i87exp \
		cgobj\i87data \
		cgobj\i86ver \
		cgobj\i86temps \
		cgobj\i86stubs \
		cgobj\i86splt3 \
		cgobj\i86split \
		cgobj\i86sel \
		cgobj\i86segs \
		cgobj\i86rtlk \
		cgobj\i86regsv \
		cgobj\i86reg \
		cgobj\i86proc \
		cgobj\i86opcod \
		cgobj\i86obj \
		cgobj\i86mul \
		cgobj\i86lesds \
		cgobj\i86ldstr \
		cgobj\i86index \
		cgobj\i86half \
		cgobj\i86esc \
		cgobj\i86enc2 \
		cgobj\i86enc \
		cgobj\i86dbsup \
		cgobj\i86data \
		cgobj\i86dfsup \
		cgobj\i86cvsup \
		cgobj\i86call \
		cgobj\i86base \
		cgobj\386setcc \
		cgobj\386funit \
		cgobj\i87wait \
		cgobj\sib \
		cgobj\rtcall \
		cgobj\i86enc32 \
		cgobj\386type \
		cgobj\386tls \
		cgobj\386table \
		cgobj\386splt2 \
		cgobj\386sib \
		cgobj\386score \
		cgobj\386rtrtn \
		cgobj\386rgtbl \
		cgobj\386ptype \
		cgobj\386optab \
		cgobj\386opseg \
		cgobj\386ilen \
		cgobj\386conv

cgobj\echoapi.obj:	cg\echoapi.c
cgobj\dump87.obj:	cg\dump87.c
cgobj\dumptree.obj:	cg\dumptree.c
cgobj\dumptab.obj:	cg\dumptab.c
cgobj\dumpschd.obj:	cg\dumpschd.c
cgobj\dumpsc.obj:	cg\dumpsc.c
cgobj\dumpregt.obj:	cg\dumpregt.c
cgobj\dumpref.obj:	cg\dumpref.c
cgobj\dumpopt.obj:	cg\dumpopt.c
cgobj\dumpio.obj:	cg\dumpio.c
cgobj\dumpins.obj:	cg\dumpins.c
cgobj\dumpindv.obj:	cg\dumpindv.c
cgobj\dumpconf.obj:	cg\dumpconf.c
cgobj\dumpblk.obj:	cg\dumpblk.c
cgobj\dumpan.obj:	cg\dumpan.c
cgobj\memmgt.obj:	cg\memmgt.c
cgobj\memwrap.obj:	cg\memwrap.c
cgobj\zoiks.obj:	cg\zoiks.c
cgobj\verify.obj:	cg\verify.c
cgobj\varusage.obj:	cg\varusage.c
cgobj\utils.obj:	cg\utils.c
cgobj\unroll.obj:	cg\unroll.c
cgobj\u32moddi.obj:	cg\u32moddi.c
cgobj\types.obj:	cg\types.c
cgobj\typemap.obj:	cg\typemap.c
cgobj\typeinit.obj:	cg\typeinit.c
cgobj\treefold.obj:	cg\treefold.c
cgobj\treeconv.obj:	cg\treeconv.c
cgobj\tree.obj:		cg\tree.c
cgobj\trecurse.obj:	cg\trecurse.c
cgobj\temps.obj:	cg\temps.c
cgobj\splitvar.obj:	cg\splitvar.c
cgobj\split.obj:	cg\split.c
cgobj\spawn.obj:	cg\spawn.c
cgobj\cgsrtlst.obj:	cg\cgsrtlst.c
cgobj\sczero.obj:	cg\sczero.c
cgobj\scutil.obj:	cg\scutil.c
cgobj\scthrash.obj:	cg\scthrash.c
cgobj\scregs.obj:	cg\scregs.c
cgobj\scmain.obj:	cg\scmain.c
cgobj\scins.obj:	cg\scins.c
cgobj\scinfo.obj:	cg\scinfo.c
cgobj\scdata.obj:	cg\scdata.c
cgobj\scblock.obj:	cg\scblock.c
cgobj\rtrtn.obj:	cg\rtrtn.c
cgobj\revcond.obj:	cg\revcond.c
cgobj\regtree.obj:	cg\regtree.c
cgobj\regsave.obj:	cg\regsave.c
cgobj\regalloc.obj:	cg\regalloc.c
cgobj\redefby.obj:	cg\redefby.c
cgobj\propind.obj:	cg\propind.c
cgobj\peepopt.obj:	cg\peepopt.c
cgobj\patch.obj:	cg\patch.c
cgobj\parm.obj:		cg\parm.c
cgobj\overlap.obj:	cg\overlap.c
cgobj\optvlbl.obj:	cg\optvlbl.c
cgobj\optutil.obj:	cg\optutil.c
cgobj\opttell.obj:	cg\opttell.c
cgobj\optrel.obj:	cg\optrel.c
cgobj\optq.obj:		cg\optq.c
cgobj\optpush.obj:	cg\optpush.c
cgobj\optpull.obj:	cg\optpull.c
cgobj\optmkins.obj:	cg\optmkins.c
cgobj\optmain.obj:	cg\optmain.c
cgobj\optlbl.obj:	cg\optlbl.c
cgobj\optins.obj:	cg\optins.c
cgobj\optimize.obj:	cg\optimize.c
cgobj\optdata.obj:	cg\optdata.c
cgobj\optcom.obj:	cg\optcom.c
cgobj\optask.obj:	cg\optask.c
cgobj\optab.obj:	cg\optab.c
cgobj\objname.obj:	cg\objname.c
cgobj\object.obj:	cg\object.c
cgobj\nullprop.obj:	cg\nullprop.c
cgobj\namelist.obj:	cg\namelist.c
cgobj\multiply.obj:	cg\multiply.c
cgobj\memtrack.obj:	cg\memtrack.c
cgobj\makeins.obj:	cg\makeins.c
cgobj\makeblk.obj:	cg\makeblk.c
cgobj\makeaddr.obj:	cg\makeaddr.c
cgobj\loopopts.obj:	cg\loopopts.c
cgobj\loadstor.obj:	cg\loadstor.c
cgobj\liveinfo.obj:	cg\liveinfo.c
cgobj\intrface.obj:	cg\intrface.c
cgobj\insutil.obj:	cg\insutil.c
cgobj\inssched.obj:	cg\inssched.c
cgobj\insdead.obj:	cg\insdead.c
cgobj\inline.obj:	cg\inline.c
cgobj\i64.obj:		watcom\i64.c
cgobj\generate.obj:	cg\generate.c
cgobj\freelist.obj:	cg\freelist.c
cgobj\foldins.obj:	cg\foldins.c
cgobj\flowsave.obj:	cg\flowsave.c
cgobj\flood.obj:	cg\flood.c
cgobj\flograph.obj:	cg\flograph.c
cgobj\fixindex.obj:	cg\fixindex.c
cgobj\expand.obj:	cg\expand.c
cgobj\encode.obj:	cg\encode.c
cgobj\edge.obj:		cg\edge.c
cgobj\dominate.obj:	cg\dominate.c
cgobj\display.obj:	cg\display.c
cgobj\dftypes.obj:	cg\dftypes.c
cgobj\dfsyms.obj:	cg\dfsyms.c
cgobj\dbtypes.obj:	cg\dbtypes.c
cgobj\dbtable.obj:	cg\dbtable.c
cgobj\dbsupp.obj:	cg\dbsupp.c
cgobj\dbsyms.obj:	cg\dbsyms.c
cgobj\dataflo.obj:	cg\dataflo.c
cgobj\data.obj:		cg\data.c
cgobj\cvtypes.obj:	cg\cvtypes.c
cgobj\cvsyms.obj:	cg\cvsyms.c
cgobj\cse.obj:		cg\cse.c
cgobj\conflict.obj:	cg\conflict.c
cgobj\condcode.obj:	cg\condcode.c
cgobj\cgaux.obj:	cg\cgaux.c
cgobj\cachecon.obj:	cg\cachecon.c
cgobj\breakrtn.obj:	cg\breakrtn.c
cgobj\blktrim.obj:	cg\blktrim.c
cgobj\bldsel.obj:	cg\bldsel.c
cgobj\bldins.obj:	cg\bldins.c
cgobj\bldcall.obj:	cg\bldcall.c
cgobj\addrfold.obj:	cg\addrfold.c
cgobj\addrcnst.obj:	cg\addrcnst.c
cgobj\wvtypes.obj:	cg\intel\wvtypes.c
cgobj\wvsyms.obj:	cg\intel\wvsyms.c
cgobj\parmreg.obj:	cg\parmreg.c
cgobj\i87table.obj:	cg\intel\i87table.c
cgobj\i87sched.obj:	cg\intel\i87sched.c
cgobj\i87reg.obj:	cg\intel\i87reg.c
cgobj\i87opt.obj:	cg\intel\i87opt.c
cgobj\i87exp.obj:	cg\intel\i87exp.c
cgobj\i87data.obj:	cg\intel\i87data.c
cgobj\i86ver.obj:	cg\intel\i86ver.c
cgobj\i86temps.obj:	cg\intel\i86temps.c
cgobj\i86stubs.obj:	cg\intel\i86stubs.c
cgobj\i86splt3.obj:	cg\intel\i86splt3.c
cgobj\i86split.obj:	cg\intel\i86split.c
cgobj\i86sel.obj:	cg\intel\i86sel.c
cgobj\i86segs.obj:	cg\intel\i86segs.c
cgobj\i86rtlk.obj:	cg\intel\i86rtlk.c
cgobj\i86regsv.obj:	cg\intel\i86regsv.c
cgobj\i86reg.obj:	cg\intel\i86reg.c
cgobj\i86proc.obj:	cg\intel\i86proc.c
cgobj\i86opcod.obj:	cg\intel\i86opcod.c
cgobj\i86obj.obj:	cg\intel\i86obj.c
cgobj\i86mul.obj:	cg\intel\i86mul.c
cgobj\i86lesds.obj:	cg\intel\i86lesds.c
cgobj\i86ldstr.obj:	cg\intel\i86ldstr.c
cgobj\i86index.obj:	cg\intel\i86index.c
cgobj\i86half.obj:	cg\intel\i86half.c
cgobj\i86esc.obj:	cg\intel\i86esc.c
cgobj\i86enc2.obj:	cg\intel\i86enc2.c
cgobj\i86enc.obj:	cg\intel\i86enc.c
cgobj\i86dbsup.obj:	cg\intel\i86dbsup.c
cgobj\i86data.obj:	cg\intel\i86data.c
cgobj\i86dfsup.obj:	cg\intel\i86dfsup.c
cgobj\i86cvsup.obj:	cg\intel\i86cvsup.c
cgobj\i86call.obj:	cg\intel\i86call.c
cgobj\i86base.obj:	cg\intel\i86base.c
cgobj\386setcc.obj:	cg\intel\386setcc.c
cgobj\386funit.obj:	cg\intel\386funit.c
cgobj\i87wait.obj:	cg\intel\i87wait.c
cgobj\sib.obj:		cg\intel\386\sib.c
cgobj\rtcall.obj:	cg\rtcall.c
cgobj\i86enc32.obj:	cg\intel\386\i86enc32.c
cgobj\386type.obj:	cg\intel\386\386type.c
cgobj\386tls.obj:	cg\intel\386\386tls.c
cgobj\386table.obj:	cg\intel\386\386table.c
cgobj\386splt2.obj:	cg\intel\386\386splt2.c
cgobj\386sib.obj:	cg\intel\386\386sib.c
cgobj\386score.obj:	cg\intel\386\386score.c
cgobj\386rtrtn.obj:	cg\intel\386\386rtrtn.c
cgobj\386rgtbl.obj:	cg\intel\386\386rgtbl.c
cgobj\386ptype.obj:	cg\intel\386\386ptype.c
cgobj\386optab.obj:	cg\intel\386\386optab.c
cgobj\386opseg.obj:	cg\intel\386\386opseg.c
cgobj\386ilen.obj:	cg\intel\386\386ilen.c
cgobj\386conv.obj:	cg\intel\386\386conv.c


cg386lnx.obj:	libs\cg386lnx.obj
	@echo Done

libs\cg386lnx.obj: cgobj\stack.obj \
		cgobj\doblips.obj \
		cgobj\memmgr.obj \
		cgobj\qblip.obj \
		cgobj\qtimer.obj \
		cgobj\onexit.obj \
		cgobj\envvar.obj \
		cgobj\memlimit.obj \
		cgobj\posixio.obj
	xlib libs\cg386lnx /create/list=cg386lnx \
		cgobj\stack \
		cgobj\doblips \
		cgobj\memmgr \
		cgobj\qblip \
		cgobj\qtimer \
		cgobj\onexit \
		cgobj\envvar \
		cgobj\memlimit \
		cgobj\posixio

cgobj\stack.obj:	cg\stack.c
cgobj\doblips.obj:	cg\doblips.c
cgobj\memmgr.obj:	cg\memmgr.c
cgobj\qblip.obj:	cg\qblip.c
cgobj\qtimer.obj:	cg\qtimer.c
cgobj\onexit.obj:	cg\onexit.c
cgobj\envvar.obj:	cg\envvar.c
cgobj\memlimit.obj:	cg\memlimit.c
cgobj\posixio.obj:	cg\posixio.c


cf.obj:	libs\cf.obj
	@echo Done

libs\cf.obj:	cfobj\u32moddi.obj \
		cfobj\cfutil.obj \
		cfobj\cftof.obj \
		cfobj\cfmul.obj \
		cfobj\cfmem.obj \
		cfobj\cfmath.obj \
		cfobj\cfinvers.obj \
		cfobj\cfdiv.obj \
		cfobj\cfconv.obj
	xlib libs\cf /create/list=cf \
		cfobj\u32moddi \
		cfobj\cfutil \
		cfobj\cftof \
		cfobj\cfmul \
		cfobj\cfmem \
		cfobj\cfmath \
		cfobj\cfinvers \
		cfobj\cfdiv \
		cfobj\cfconv

cfobj\u32moddi.obj:	cf\u32moddi.c
cfobj\cfutil.obj:	cf\cfutil.c
cfobj\cftof.obj:	cf\cftof.c
cfobj\cfmul.obj:	cf\cfmul.c
cfobj\cfmem.obj:	cf\cfmem.c
cfobj\cfmath.obj:	cf\cfmath.c
cfobj\cfinvers.obj:	cf\cfinvers.c
cfobj\cfdiv.obj:	cf\cfdiv.c
cfobj\cfconv.obj:	cf\cfconv.c

dw.obj:		libs\dw.obj
	@echo Done

libs\dw.obj:	dwobj\dwutils.obj \
		dwobj\dwtype.obj \
		dwobj\dwsym.obj \
		dwobj\dwrefer.obj \
		dwobj\dwname.obj \
		dwobj\dwmem.obj \
		dwobj\dwmac.obj \
		dwobj\dwloc.obj \
		dwobj\dwlngen.obj \
		dwobj\dwline.obj \
		dwobj\dwinfo.obj \
		dwobj\dwhandle.obj \
		dwobj\dwgen.obj \
		dwobj\dwdecl.obj \
		dwobj\dwdie.obj \
		dwobj\dwcarve.obj \
		dwobj\dwarange.obj \
		dwobj\dwabbrev.obj
	xlib libs\dw /create/list=dw \
		dwobj\dwutils \
		dwobj\dwtype \
		dwobj\dwsym \
		dwobj\dwrefer \
		dwobj\dwname \
		dwobj\dwmem \
		dwobj\dwmac \
		dwobj\dwloc \
		dwobj\dwlngen \
		dwobj\dwline \
		dwobj\dwinfo \
		dwobj\dwhandle \
		dwobj\dwgen \
		dwobj\dwdecl \
		dwobj\dwdie \
		dwobj\dwcarve \
		dwobj\dwarange \
		dwobj\dwabbrev

dwobj\dwutils.obj:	dw\dwutils.c
dwobj\dwtype.obj:	dw\dwtype.c
dwobj\dwsym.obj:	dw\dwsym.c
dwobj\dwrefer.obj:	dw\dwrefer.c
dwobj\dwname.obj:	dw\dwname.c
dwobj\dwmem.obj:	dw\dwmem.c
dwobj\dwmac.obj:	dw\dwmac.c
dwobj\dwloc.obj:	dw\dwloc.c
dwobj\dwlngen.obj:	dw\dwlngen.c
dwobj\dwline.obj:	dw\dwline.c
dwobj\dwinfo.obj:	dw\dwinfo.c
dwobj\dwhandle.obj:	dw\dwhandle.c
dwobj\dwgen.obj:	dw\dwgen.c
dwobj\dwdecl.obj:	dw\dwdecl.c
dwobj\dwdie.obj:	dw\dwdie.c
dwobj\dwcarve.obj:	dw\dwcarve.c
dwobj\dwarange.obj:	dw\dwarange.c
dwobj\dwabbrev.obj:	dw\dwabbrev.c

test.run:	test.obj
	xlink newlib:xos\defsegs test newlib:\xos\libc01 \
		/output=test /map=test /sym=test

test.obj:	test.c
	gecko owxcx /hc /d2 /i=xosinc:owxc\ /zq /6s /w3 /we /j \
		/fpi87 /fp3 /s /mf /za99 /ze /nm=$(@N) $(*DPN).c

conv.run:	conv.obj
	xlink newlib:\xos\defsegs conv \
		newlib:\xos\libx01 newlib:\xos\libc01 \
		/output=conv /map=conv /sym=conv
	copy /over conv.run newcmd:conv.run

conv.obj:	conv.c
	owxc /hc /d2 /i=xosinc:\owxc\ /zq /6s /w3 /we \
		/fpi87 /fp3 /s /mf /za99 /ze /oarfkht /nm=$(@N) $(*DPN).c