
;The table used when decoding instructions is formatted as follows:
;  Offset Size Description
;     0     4  Address of opcode string for word operands
;     4     4  Address of opcode string for long operands
;     8     1  Contents for first byte
;     9     1  Mask for first byte
;    10     1  Contents for second byte
;    11     1  Mask for second byte
;    12     1  Contents for third byte
;    13     1  Mask for third byte
;    24     4  Address of output routine


typedef struct
{	char  *wname;
	char  *lname;
	uchar  val1;
	uchar  mask1;
	uchar  val2;
	uchar  mask2;
	uchar  val3;
	uchar  mask3;
	void (*func)();
} OPENT;

OPENT optbl[] =



	{"ADDB   ,"ADDB   , 00, FD, 00, 00, 00, 00, dtwoopr
	{"ADDW   ,"ADDL   , 01, FD, 00, 00, 00, 00, dtwoopr
	{"ADDB   ,"ADDB   , 04, FF, 00, 00, 00, 00, dim2ax
	{"ADDW   ,"ADDL   , 05, FF, 00, 00, 00, 00, dim2ax
	{"PUSHW  ,"PUSHL  , 06, E7, 00, 00, 00, 00, dsegreg
	{"POPW   ,"POPL   , 07, FF, 00, 00, 00, 00, dsegrgc
	{"ORB    ,"ORB    , 08, FD, 00, 00, 00, 00, dtwoopr
	{"ORW    ,"ORL    , 09, FD, 00, 00, 00, 00, dtwoopr
	{"ORB    ,"ORB    , 0C, FF, 00, 00, 00, 00, dim2ax
	{"ORW    ,"ORL    , 0D, FF, 00, 00, 00, 00, dim2ax
	{"SLDT   ,"SLDT   , 0F, FF, 00, FF, 00, 38, dlng1wopr
	{"STR    ,"STR    , 0F, FF, 00, FF, 08, 38, dlng1wopr
	{"LLDT   ,"LLDT   , 0F, FF, 00, FF, 10, 38, dlng1wopr
	{"LTR    ,"LTR    , 0F, FF, 00, FF, 18, 38, dlng1wopr
	{"VERR   ,"VERR   , 0F, FF, 00, FF, 20, 38, dlng1wopr
	{"VERW   ,"VERW   , 0F, FF, 00, FF, 28, 38, dlng1wopr
	{"SGDTW  ,"SGDTL  , 0F, FF, 01, FF, 00, 38, dlng1wlopr
	{"SIDTW  ,"SGDTL  , 0F, FF, 01, FF, 08, 38, dlng1wlopr
	{"LIDTW  ,"LIDTL  , 0F, FF, 01, FF, 18, 38, dlng1wlopr
	{"LGDTW  ,"LGDTL  , 0F, FF, 01, FF, 10, 38, dlng1wlopr
	{"SMSW   ,"SMSW   , 0F, FF, 01, FF, 20, 38, dlng1wopr
	{"LMSW   ,"LMSW   , 0F, FF, 01, FF, 30, 38, dlng1wopr
	{"LARW   ,"LARL   , 0F, FF, 02, FF, 00, 00, dlng2oprd
	{"LSLW   ,"LSLL   , 0F, FF, 03, FF, 00, 00, dlng2opr
	{"CLTS   ,"CLTS   , 0F, FF, 06, FF, 00, 00, dtwobyt
	{"MOVL   ,"MOVL   , 0F, FF, 20, FD, C0, C0, dmovcr
	{"MOVL   ,"MOVL   , 0F, FF, 21, FD, C0, C0, dmovdr
	{"MOVL   ,"MOVL   , 0F, FF, 24, FD, C0, C0, dmovtr
	{"JO     ,"JO     , 0F, FF, 80, FF, 00, 00, dlngbr
	{"JNO    ,"JNO    , 0F, FF, 81, FF, 00, 00, dlngbr
	{"JC     ,"JC     , 0F, FF, 82, FF, 00, 00, dlngbr
	{"JNC    ,"JNC    , 0F, FF, 83, FF, 00, 00, dlngbr
	{"JE     ,"JE     , 0F, FF, 84, FF, 00, 00, dlngbr
	{"JNE    ,"JNE    , 0F, FF, 85, FF, 00, 00, dlngbr
	{"JBE    ,"JBE    , 0F, FF, 86, FF, 00, 00, dlngbr
	{"JA     ,"JA     , 0F, FF, 87, FF, 00, 00, dlngbr
	{"JS     ,"JS     , 0F, FF, 88, FF, 00, 00, dlngbr
	{"JNS    ,"JNS    , 0F, FF, 89, FF, 00, 00, dlngbr
	{"JPE    ,"JPE    , 0F, FF, 8A, FF, 00, 00, dlngbr
	{"JPO    ,"JPO    , 0F, FF, 8B, FF, 00, 00, dlngbr
	{"JL     ,"JL     , 0F, FF, 8C, FF, 00, 00, dlngbr
	{"JGE    ,"JGE    , 0F, FF, 8D, FF, 00, 00, dlngbr
	{"JLE    ,"JLE    , 0F, FF, 8E, FF, 00, 00, dlngbr
	{"JG     ,"JG     , 0F, FF, 8F, FF, 00, 00, dlngbr
	{"SETO   ,"SETO   , 0F, FF, 90, FF, 00, 00, dlng1bopr
	{"SETNO  ,"SETNO  , 0F, FF, 91, FF, 00, 00, dlng1bopr
	{"SETC   ,"SETC   , 0F, FF, 92, FF, 00, 00, dlng1bopr
	{"SETNC  ,"SETNC  , 0F, FF, 93, FF, 00, 00, dlng1bopr
	{"SETE   ,"SETE   , 0F, FF, 94, FF, 00, 00, dlng1bopr
	{"SETNE  ,"SETNE  , 0F, FF, 95, FF, 00, 00, dlng1bopr
	{"SETBE  ,"SETBE  , 0F, FF, 96, FF, 00, 00, dlng1bopr
	{"SETA   ,"SETA   , 0F, FF, 97, FF, 00, 00, dlng1bopr
	{"SETS   ,"SETS   , 0F, FF, 98, FF, 00, 00, dlng1bopr
	{"SETNS  ,"SETNS  , 0F, FF, 99, FF, 00, 00, dlng1bopr
	{"SETPE  ,"SETPE  , 0F, FF, 9A, FF, 00, 00, dlng1bopr
	{"SETPO  ,"SETPO  , 0F, FF, 9B, FF, 00, 00, dlng1bopr
	{"SETL   ,"SETL   , 0F, FF, 9C, FF, 00, 00, dlng1bopr
	{"SETGE  ,"SETGE  , 0F, FF, 9D, FF, 00, 00, dlng1bopr
	{"SETLE  ,"SETLE  , 0F, FF, 9E, FF, 00, 00, dlng1bopr
	{"SETG   ,"SETG   , 0F, FF, 9F, FF, 00, 00, dlng1bopr
	{"PUSHW  ,"PUSHL  , 0F, FF, A0, F7, 00, 00, dsg3reg
	{"POPW   ,"POPL   , 0F, FF, A1, F7, 00, 00, dsg3rgc
	{"BTW    ,"BTL    , 0F, FF, A3, FF, 00, 00, dlng2oprs
	{"SHLDW  ,"SHLDL  , 0F, FF, A4, FF, 00, 00, dlngsdim
	{"SHLDW  ,"SHLDL  , 0F, FF, A5, FF, 00, 00, dlngsdcl
	{"SHRDW  ,"SHRDL  , 0F, FF, AC, FF, 00, 00, dlngsdim
	{"SHRDW  ,"SHRDL  , 0F, FF, AD, FF, 00, 00, dlngsdcl
	{"IMULW  ,"IMULL  , 0F, FF, AF, FF, 00, 00, dlng2oprd
	{"BTW    ,"BTL    , 0F, FF, BA, FF, 20, 38, dlngimdb
	{"BTSW   ,"BTSL   , 0F, FF, AB, FF, 00, 00, dlng2oprs
	{"LSSW   ,"LSSL   , 0F, FF, B2, FF, 00, 00, dlng2oprd
	{"BTZW   ,"BTZL   , 0F, FF, B3, FF, 00, 00, dlng2oprs
	{"LFSW   ,"LFSL   , 0F, FF, B4, FF, 00, 00, dlng2oprd
	{"LGSW   ,"LGSL   , 0F, FF, B5, FF, 00, 00, dlng2oprd
	{"MOVZBW ,"MOVZBL , 0F, FF, B6, FF, 00, 00, dmovxb
	{"MOVZWW ,"MOVZWL , 0F, FF, B7, FF, 00, 00, dmovxw
	{"BTSW   ,"BTSL   , 0F, FF, BA, FF, 28, 38, dlngimdb
	{"BTZW   ,"BTZL   , 0F, FF, BA, FF, 30, 38, dlngimdb
	{"BTCW   ,"BTCL   , 0F, FF, BA, FF, 38, 38, dlngimdb
	{"BTCW   ,"BTCL   , 0F, FF, BB, FF, 00, 00, dlng2oprs
	{"BSFW   ,"BSFL   , 0F, FF, BC, FF, 00, 00, dlng2oprd
	{"BSRW   ,"BSRL   , 0F, FF, BD, FF, 00, 00, dlng2oprd
	{"MOVXBW ,"MOVXBL , 0F, FF, BE, FF, 00, 00, dmovxb
	{"MOVXWW ,"MOVXWL , 0F, FF, BF, FF, 00, 00, dmovxw
	{"BSWAPL ,"BSWAPL , 0F, FF, C8, F8, 00, 00, donefreg
	{"ADCB   ,"ADCB   , 10, FD, 00, 00, 00, 00, dtwoopr
	{"ADCW   ,"ADCL   , 11, FD, 00, 00, 00, 00, dtwoopr
	{"ADCB   ,"ADCB   , 14, FF, 00, 00, 00, 00, dim2ax
	{"ADCW   ,"ADCL   , 15, FF, 00, 00, 00, 00, dim2ax
	{"POPW   ,"POPL   , 17, FF, 00, 00, 00, 00, dsegrgc
	{"POPW   ,"POPL   , 1F, FF, 00, 00, 00, 00, dsegrgc
	{"SBBB   ,"SBBB   , 18, FD, 00, 00, 00, 00, dtwoopr
	{"SBBW   ,"SBBL   , 19, FD, 00, 00, 00, 00, dtwoopr
	{"SBBB   ,"SBBB   , 1C, FF, 00, 00, 00, 00, dim2ax
	{"SBBW   ,"SBBL   , 1D, FF, 00, 00, 00, 00, dim2ax
	{"ANDB   ,"ANDB   , 20, FD, 00, 00, 00, 00, dtwoopr
	{"ANDW   ,"ANDL   , 21, FD, 00, 00, 00, 00, dtwoopr
	{"ANDB   ,"ANDB   , 24, FF, 00, 00, 00, 00, dim2ax
	{"ANDW   ,"ANDL   , 25, FF, 00, 00, 00, 00, dim2ax
	{"       ,"       , 26, E7, 00, 00, 00, 00, dsegpfx
	{"DAA    ,"DAA    , 27, FF, 00, 00, 00, 00, donebyt
	{"SUBB   ,"SUBB   , 28, FD, 00, 00, 00, 00, dtwoopr
	{"SUBW   ,"SUBL   , 29, FD, 00, 00, 00, 00, dtwoopr
	{"SUBB   ,"SUBB   , 2C, FF, 00, 00, 00, 00, dim2ax
	{"SUBW   ,"SUBL   , 2D, FF, 00, 00, 00, 00, dim2ax
	{"CLRB   ,"CLRB   , 30, FD, C0, FF, 00, 00, doneopr
	{"CLRB   ,"CLRB   , 30, FD, C9, FF, 00, 00, doneopr
	{"CLRB   ,"CLRB   , 30, FD, D2, FF, 00, 00, doneopr
	{"CLRB   ,"CLRB   , 30, FD, DB, FF, 00, 00, doneopr
	{"CLRB   ,"CLRB   , 30, FD, E4, FF, 00, 00, doneopr
	{"CLRB   ,"CLRB   , 30, FD, ED, FF, 00, 00, doneopr
	{"CLRB   ,"CLRB   , 30, FD, F6, FF, 00, 00, doneopr
	{"CLRB   ,"CLRB   , 30, FD, FF, FF, 00, 00, doneopr
	{"CLRW   ,"CLRL   , 31, FD, C0, FF, 00, 00, doneopr
	{"CLRW   ,"CLRL   , 31, FD, C9, FF, 00, 00, doneopr
	{"CLRW   ,"CLRL   , 31, FD, D2, FF, 00, 00, doneopr
	{"CLRW   ,"CLRL   , 31, FD, DB, FF, 00, 00, doneopr
	{"CLRW   ,"CLRL   , 31, FD, E4, FF, 00, 00, doneopr
	{"CLRW   ,"CLRL   , 31, FD, ED, FF, 00, 00, doneopr
	{"CLRW   ,"CLRL   , 31, FD, F6, FF, 00, 00, doneopr
	{"CLRW   ,"CLRL   , 31, FD, FF, FF, 00, 00, doneopr
	{"XORB   ,"XORB   , 30, FD, 00, 00, 00, 00, dtwoopr
	{"XORW   ,"XORL   , 31, FD, 00, 00, 00, 00, dtwoopr
	{"XORB   ,"XORB   , 34, FF, 00, 00, 00, 00, dim2ax
	{"XORW   ,"XORL   , 35, FF, 00, 00, 00, 00, dim2ax
	{"AAA    ,"AAA    , 37, FF, 00, 00, 00, 00, donebyt
	{"CMPB   ,"CMPB   , 38, FD, 00, 00, 00, 00, dtwoopr
	{"CMPW   ,"CMPL   , 39, FD, 00, 00, 00, 00, dtwoopr
	{"CMPB   ,"CMPB   , 3C, FF, 00, 00, 00, 00, dim2ax
	{"CMPW   ,"CMPL   , 3D, FF, 00, 00, 00, 00, dim2ax
	{"AAS    ,"AAS    , 3F, FF, 00, 00, 00, 00, donebyt
	{"INCW   ,"INCL   , 40, F8, 00, 00, 00, 00, dregopr
	{"DECW   ,"DECL   , 48, F8, 00, 00, 00, 00, dregopr
	{"PUSHW  ,"PUSHL  , 50, F8, 00, 00, 00, 00, dregopr
	{"POPW   ,"POPL   , 58, F8, 00, 00, 00, 00, dregopr
	{"PUSHAW ,"PUSHAL , 60, FF, 00, 00, 00, 00, donebyt
	{"POPAW  ,"POPAL  , 61, FF, 00, 00, 00, 00, donebyt
	{"BOUNDW ,"BOUNDL , 62, FF, 00, 00, 00, 00, dtwoopr
	{"ARPL   ,"ARPL   , 63, FF, 00, 00, 00, 00, dtwoopm16
	{"       ,        , 64, FF, 00, 00, 00, 00, dsfspfx
	{"       ,        , 65, FF, 00, 00, 00, 00, dsgspfx
	{"       ,        , 66, FF, 00, 00, 00, 00, doszpfx
	{"       ,        , 67, FF, 00, 00, 00, 00, daszpfx
	{"PUSHW  ,"PUSHL  , 68, FD, 00, 00, 00, 00, dpushim
	{"IMULW  ,"IMULL  , 69, FD, 00, 00, 00, 00, dimul
	{"INSB   ,"INSB   , 6C, FF, 00, 00, 00, 00, dstring
	{"INSW   ,"INSL   , 6D, FF, 00, 00, 00, 00, dstring
	{"OUTSB  ,"OUTSB  , 6E, FF, 00, 00, 00, 00, dstring
	{"OUTSW  ,"OUTSL  , 6F, FF, 00, 00, 00, 00, dstring
	{"JO     ,"JO     , 70, FF, 00, 00, 00, 00, dbranch
	{"JNO    ,"JNO    , 71, FF, 00, 00, 00, 00, dbranch
	{"JC     ,"JC     , 72, FF, 00, 00, 00, 00, dbranch
	{"JNC    ,"JNC    , 73, FF, 00, 00, 00, 00, dbranch
	{"JE     ,"JE     , 74, FF, 00, 00, 00, 00, dbranch
	{"JNE    ,"JNE    , 75, FF, 00, 00, 00, 00, dbranch
	{"JBE    ,"JBE    , 76, FF, 00, 00, 00, 00, dbranch
	{"JA     ,"JA     , 77, FF, 00, 00, 00, 00, dbranch
	{"JS     ,"JS     , 78, FF, 00, 00, 00, 00, dbranch
	{"JNS    ,"JNS    , 79, FF, 00, 00, 00, 00, dbranch
	{"JPE    ,"JPE    , 7A, FF, 00, 00, 00, 00, dbranch
	{"JPO    ,"JPO    , 7B, FF, 00, 00, 00, 00, dbranch
	{"JL     ,"JL     , 7C, FF, 00, 00, 00, 00, dbranch
	{"JGE    ,"JGE    , 7D, FF, 00, 00, 00, 00, dbranch
	{"JLE    ,"JLE    , 7E, FF, 00, 00, 00, 00, dbranch
	{"JG     ,"JG     , 7F, FF, 00, 00, 00, 00, dbranch
	{"ADDB   ,"ADDB   , 80, FD, 00, 38, 00, 00, dims2mem
	{"ORB    ,"ORB    , 80, FD, 08, 38, 00, 00, dims2mem
	{"ADCB   ,"ADCB   , 80, FD, 10, 38, 00, 00, dims2mem
	{"SBBB   ,"SBBB   , 80, FD, 18, 38, 00, 00, dims2mem
	{"ANDB   ,"ANDB   , 80, FD, 20, 38, 00, 00, dims2mem
	{"SUBB   ,"SUBB   , 80, FD, 28, 38, 00, 00, dims2mem
	{"XORB   ,"XORB   , 80, FD, 30, 38, 00, 00, dims2mem
	{"CMPB   ,"CMPB   , 80, FD, 38, 38, 00, 00, dims2mem
	{"ADDW   ,"ADDL   , 81, FD, 00, 38, 00, 00, dims2mem
	{"ORW    ,"ORL    , 81, FD, 08, 38, 00, 00, dims2mem
	{"ADCW   ,"ADCL   , 81, FD, 10, 38, 00, 00, dims2mem
	{"SBBW   ,"SBBL   , 81, FD, 18, 38, 00, 00, dims2mem
	{"ANDW   ,"ANDL   , 81, FD, 20, 38, 00, 00, dims2mem
	{"SUBW   ,"SUBL   , 81, FD, 28, 38, 00, 00, dims2mem
	{"XORW   ,"XORL   , 81, FD, 30, 38, 00, 00, dims2mem
	{"CMPW   ,"CMPL   , 81, FD, 38, 38, 00, 00, dims2mem
	{"TESTB  ,"TESTB  , 84, FF, 00, 00, 00, 00, dtwoopr
	{"TESTW  ,"TESTL  , 85, FF, 00, 00, 00, 00, dtwoopr
	{"XCHGB  ,"XCHGB  , 86, FF, 00, 00, 00, 00, dtwoopr
	{"XCHGW  ,"XCHGL  , 87, FF, 00, 00, 00, 00, dtwoopr
	{"MOVB   ,"MOVB   , 88, FD, 00, 00, 00, 00, dtwoopr
	{"MOVW   ,"MOVL   , 89, FD, 00, 00, 00, 00, dtwoopr
	{"MOVW   ,"MOVL   , 8C, FD, 00, 00, 00, 00, dmemxseg
	{"LEAW   ,"LEAL   , 8D, FF, 00, 00, 00, 00, dtwoopx
	{"POPW   ,"POPL   , 8F, FF, 00, 38, 00, 00, doneopr
	{"NOP    ,"NOP    , 90, FF, 00, 00, 00, 00, donebyt
	{"XCHGW  ,"XCHGL  , 90, F8, 00, 00, 00, 00, dxchgax
	{"CBW    ,"CWL    , 98, FF, 00, 00, 00, 00, donebyt
	{"CWD    ,"CLQ    , 99, FF, 00, 00, 00, 00, donebyt
	{"CALLF  ,"CALLF  , 9A, FF, 00, 00, 00, 00, ddirjmpf
	{"FENI   ,"FENI   , 9B, FF, DB, FF, E0, FF, dthrbyt
	{"FDISI  ,"FDISI  , 9B, FF, DB, FF, E1, FF, dthrbyt
	{"FCLEX  ,"FCLEX  , 9B, FF, DB, FF, E2, FF, dthrbyt
	{"FINIT  ,"FINIT  , 9B, FF, DB, FF, E3, FF, dthrbyt
	{"FWAIT  ,"FWAIT  , 9B, FF, 00, 00, 00, 00, donebyt
	{"PUSHFW ,"PUSHFL , 9C, FF, 00, 00, 00, 00, donebyt
	{"POPFW  ,"POPFL  , 9D, FF, 00, 00, 00, 00, donebyt
	{"SAHF   ,"SAHF   , 9E, FF, 00, 00, 00, 00, donebyt
	{"LAHF   ,"LAHF   , 9F, FF, 00, 00, 00, 00, donebyt
	{"MOVB   ,"MOVB   , A0, FD, 00, 00, 00, 00, dmemxax
	{"MOVW   ,"MOVL   , A1, FD, 00, 00, 00, 00, dmemxax
	{"MOVSB  ,"MOVSB  , A4, FF, 00, 00, 00, 00, dstring
	{"MOVSW  ,"MOVSL  , A5, FF, 00, 00, 00, 00, dstring
	{"CMPSB  ,"CMPSB  , A6, FF, 00, 00, 00, 00, dstrcon
	{"CMPSW  ,"CMPSL  , A7, FF, 00, 00, 00, 00, dstrcon
	{"TESTB  ,"TESTB  , A8, FF, 00, 00, 00, 00, dim2ax
	{"TESTW  ,"TESTL  , A9, FF, 00, 00, 00, 00, dim2ax
	{"STOSB  ,"STOSB  , AA, FF, 00, 00, 00, 00, dstring
	{"STOSW  ,"STOSL  , AB, FF, 00, 00, 00, 00, dstring
	{"LODSB  ,"LODSB  , AC, FF, 00, 00, 00, 00, dstring
	{"LODSW  ,"LODSL  , AD, FF, 00, 00, 00, 00, dstring
	{"SCASB  ,"SCASB  , AE, FF, 00, 00, 00, 00, dstrcon
	{"SCASW  ,"SCASL  , AF, FF, 00, 00, 00, 00, dstrcon
	{"MOVB   ,"MOVB   , B0, F8, 00, 00, 00, 00, dim2reg
	{"MOVW   ,"MOVL   , B8, F8, 00, 00, 00, 00, dim2reg
	{"ROLB   ,"ROLB   , C0, FF, 00, 38, 00, 00, dsrotim
	{"RORB   ,"RORB   , C0, FF, 08, 38, 00, 00, dsrotim
	{"RCLB   ,"RCLB   , C0, FF, 10, 38, 00, 00, dsrotim
	{"RCRB   ,"RCRB   , C0, FF, 18, 38, 00, 00, dsrotim
	{"SHLB   ,"SHLB   , C0, FF, 20, 38, 00, 00, dsrotim
	{"SHRB   ,"SHRB   , C0, FF, 28, 38, 00, 00, dsrotim
	{"SARB   ,"SARB   , C0, FF, 38, 38, 00, 00, dsrotim
	{"ROLW   ,"ROLL   , C1, FF, 00, 38, 00, 00, dsrotim
	{"RORW   ,"RORL   , C1, FF, 08, 38, 00, 00, dsrotim
	{"RCLW   ,"RCLL   , C1, FF, 10, 38, 00, 00, dsrotim
	{"RCRW   ,"RCRL   , C1, FF, 18, 38, 00, 00, dsrotim
	{"SHLW   ,"SHLL   , C1, FF, 20, 38, 00, 00, dsrotim
	{"SHRW   ,"SHRL   , C1, FF, 28, 38, 00, 00, dsrotim
	{"SARW   ,"SARL   , C1, FF, 38, 38, 00, 00, dsrotim
	{"RETW   ,"RETL   , C2, FF, 00, 00, 00, 00, dretfix
	{"RETW   ,"RETL   , C3, FF, 00, 00, 00, 00, donebyt
	{"LESW   ,"LESL   , C4, FF, 00, 00, 00, 00, dtwoopx
	{"LDSW   ,"LDSL   , C5, FF, 00, 00, 00, 00, dtwoopx
	{"MOVB   ,"MOVB   , C6, FF, 00, 00, 00, 00, dim2mem
	{"MOVW   ,"MOVL   , C7, FF, 00, 00, 00, 00, dim2mem
	{"ENTER  ,"ENTER  , C8, FF, 00, 00, 00, 00, dentrins
	{"LEAVE  ,"LEAVE  , C9, FF, 00, 00, 00, 00, donebyt
	{"RETFW  ,"RETFL  , CA, FF, 00, 00, 00, 00, dretfix
	{"RETFW  ,"RETFL  , CB, FF, 00, 00, 00, 00, donebyt
	{"INT3   ,"INT3   , CC, FF, 00, 00, 00, 00, donebyt
	{"CRASH  ,"CRASH  , CD, FF, 1F, FF, 00, 00, dcrash
	{"INT    ,"INT    , CD, FF, 00, 00, 00, 00, dintins
	{"INTO   ,"INTO   , CE, FF, 00, 00, 00, 00, donebyt
	{"IRETW  ,"IRETL  , CF, FF, 00, 00, 00, 00, donebyt
	{"ROLB   ,"ROLB   , D0, FF, 00, 38, 00, 00, dsrotone
	{"RORB   ,"RORB   , D0, FF, 08, 38, 00, 00, dsrotone
	{"RCLB   ,"RCLB   , D0, FF, 10, 38, 00, 00, dsrotone
	{"RCRB   ,"RCRB   , D0, FF, 18, 38, 00, 00, dsrotone
	{"SHLB   ,"SHLB   , D0, FF, 20, 38, 00, 00, dsrotone
	{"SHRB   ,"SHRB   , D0, FF, 28, 38, 00, 00, dsrotone
	{"SARB   ,"SARB   , D0, FF, 38, 38, 00, 00, dsrotone
	{"ROLW   ,"ROLL   , D1, FF, 00, 38, 00, 00, dsrotone
	{"RORW   ,"RORL   , D1, FF, 08, 38, 00, 00, dsrotone
	{"RCLW   ,"RCLL   , D1, FF, 10, 38, 00, 00, dsrotone
	{"RCRW   ,"RCRL   , D1, FF, 18, 38, 00, 00, dsrotone
	{"SHLW   ,"SHLL   , D1, FF, 20, 38, 00, 00, dsrotone
	{"SHRW   ,"SHRL   , D1, FF, 28, 38, 00, 00, dsrotone
	{"SARW   ,"SARL   , D1, FF, 38, 38, 00, 00, dsrotone
	{"ROLB   ,"ROLB   , D2, FF, 00, 38, 00, 00, dsrotcl
	{"RORB   ,"RORB   , D2, FF, 08, 38, 00, 00, dsrotcl
	{"RCLB   ,"RCLB   , D2, FF, 10, 38, 00, 00, dsrotcl
	{"RCRB   ,"RCRB   , D2, FF, 18, 38, 00, 00, dsrotcl
	{"SHLB   ,"SHLB   , D2, FF, 20, 38, 00, 00, dsrotcl
	{"SHRB   ,"SHRB   , D2, FF, 28, 38, 00, 00, dsrotcl
	{"SARB   ,"SARB   , D2, FF, 38, 38, 00, 00, dsrotcl
	{"ROLW   ,"ROLL   , D3, FF, 00, 38, 00, 00, dsrotcl
	{"RORW   ,"RORL   , D3, FF, 08, 38, 00, 00, dsrotcl
	{"RCLW   ,"RCLL   , D3, FF, 10, 38, 00, 00, dsrotcl
	{"RCRW   ,"RCRL   , D3, FF, 18, 38, 00, 00, dsrotcl
	{"SHLW   ,"SHLL   , D3, FF, 20, 38, 00, 00, dsrotcl
	{"SHRW   ,"SHRL   , D3, FF, 28, 38, 00, 00, dsrotcl
	{"SARW   ,"SARL   , D3, FF, 38, 38, 00, 00, dsrotcl
	{"AAM    ,"AAM    , D4, FF, 0A, FF, 00, 00, dtwobyt
	{"AAD    ,"AAD    , D5, FF, 0A, FF, 00, 00, dtwobyt
	{"XLAT   ,"XLAT   , D7, FF, 00, 00, 00, 00, donebyt


	{"FCOM   ,"FCOM   , D8, FF, D1, FF, 00, 00, dtwobyt
	{"FCOMP  ,"FCOMP  , D8, FF, D9, FF, 00, 00, dtwobyt
	{"FADD   ,"FADD   , D8, FF, C0, F8, 00, 00, dtwoflt
	{"FMUL   ,"FMUL   , D8, FF, C8, F8, 00, 00, dtwoflt
	{"FCOM   ,"FCOM   , D8, FF, D0, F8, 00, 00, doneflt
	{"FCOMP  ,"FCOMP  , D8, FF, D8, F8, 00, 00, doneflt
	{"FSUB   ,"FSUB   , D8, FF, E0, F8, 00, 00, dtwoflt
	{"FSUBR  ,"FSUBR  , D8, FF, E8, F8, 00, 00, dtwoflt
	{"FDIV   ,"FDIV   , D8, FF, F0, F8, 00, 00, dtwoflt
	{"FDIVR  ,"FDIVR  , D8, FF, F8, F8, 00, 00, dtwoflt
	{"FADDF  ,"FADDF  , D8, FF, 00, 38, 00, 00, doneopr
	{"FMULF  ,"FMULF  , D8, FF, 08, 38, 00, 00, doneopr
	{"FCOMF  ,"FCOMF  , D8, FF, 10, 38, 00, 00, doneopr
	{"FCOMPF ,"FCOMPF , D8, FF, 18, 38, 00, 00, doneopr
	{"FSUBF  ,"FSUBF  , D8, FF, 20, 38, 00, 00, doneopr
	{"FSUBRF ,"FSUBRF , D8, FF, 28, 38, 00, 00, doneopr
	{"FDIVF  ,"FDIVF  , D8, FF, 30, 38, 00, 00, doneopr
	{"FDIVRF ,"FDIVRF , D8, FF, 38, 38, 00, 00, doneopr

	{"FXCH   ,"FXCH   , D9, FF, D9, FF, 00, 00, dtwobyt
	{"FNOP   ,"FNOP   , D9, FF, D0, FF, 00, 00, dtwobyt
	{"FCHS   ,"FCHS   , D9, FF, E0, FF, 00, 00, dtwobyt
	{"FABS   ,"FABS   , D9, FF, E1, FF, 00, 00, dtwobyt
	{"FTST   ,"FTST   , D9, FF, E4, FF, 00, 00, dtwobyt
	{"FXAM   ,"FXAM   , D9, FF, E5, FF, 00, 00, dtwobyt
	{"FTSTP  ,"FTSTP  , D9, FF, E6, FF, 00, 00, dtwobyt
	{"FLD1   ,"FLD1   , D9, FF, E8, FF, 00, 00, dtwobyt
	{"FLDL2T ,"FLDL2T , D9, FF, E9, FF, 00, 00, dtwobyt
	{"FLDL2E ,"FLDL2E , D9, FF, EA, FF, 00, 00, dtwobyt
	{"FLDPI  ,"FLDPI  , D9, FF, EB, FF, 00, 00, dtwobyt
	{"FLDLG2 ,"FLDLG2 , D9, FF, EC, FF, 00, 00, dtwobyt
	{"FLDLN2 ,"FLDLN2 , D9, FF, ED, FF, 00, 00, dtwobyt
	{"FLDZ   ,"FLDZ   , D9, FF, EE, FF, 00, 00, dtwobyt
	{"F2XM1  ,"F2XM1  , D9, FF, F0, FF, 00, 00, dtwobyt
	{"FYL2X  ,"FYL2X  , D9, FF, F1, FF, 00, 00, dtwobyt
	{"FPTAN  ,"FPTAN  , D9, FF, F2, FF, 00, 00, dtwobyt
	{"FPATAN ,"FPATAN , D9, FF, F3, FF, 00, 00, dtwobyt
	{"FXTRACT,"FXTRACT, D9, FF, F4, FF, 00, 00, dtwobyt
	{"FPREM1 ,"FPREM1 , D9, FF, F5, FF, 00, 00, dtwobyt
	{"FDECSTP,"FDECSTP, D9, FF, F6, FF, 00, 00, dtwobyt
	{"FINCSTP,"FINCSTP, D9, FF, F7, FF, 00, 00, dtwobyt
	{"FPREM  ,"FPREM  , D9, FF, F8, FF, 00, 00, dtwobyt
	{"FYL2XP1,"FYL2XP1, D9, FF, F9, FF, 00, 00, dtwobyt
	{"FSQRT  ,"FSQRT  , D9, FF, FA, FF, 00, 00, dtwobyt
	{"FSINCOS,"FSINCOS, D9, FF, FB, FF, 00, 00, dtwobyt
	{"FRNDINT,"FRNDINT, D9, FF, FC, FF, 00, 00, dtwobyt
	{"FSCALE ,"FSCALE , D9, FF, FD, FF, 00, 00, dtwobyt
	{"FSIN   ,"FSIN   , D9, FF, FE, FF, 00, 00, dtwobyt
	{"FCOS   ,"FCOS   , D9, FF, FF, FF, 00, 00, dtwobyt
	{"FLD    ,"FLD    , D9, FF, C0, F8, 00, 00, doneflt
	{"FXCH   ,"FXCH   , D9, FF, C8, F8, 00, 00, doneflt
	{"FLDF   ,"FLDF   , D9, FF, 00, 38, 00, 00, doneopr
	{"FSTF   ,"FSTF   , D9, FF, 10, 38, 00, 00, doneopr
	{"FSTPF  ,"FSTPF  , D9, FF, 18, 38, 00, 00, doneopr
	{"FLDENV ,"FLDENV , D9, FF, 20, 38, 00, 00, doneopr
	{"FLDCW  ,"FLDCW  , D9, FF, 28, 38, 00, 00, doneopr
	{"FNSTENV,"FNSTENV, D9, FF, 30, 38, 00, 00, doneopr
	{"FNSTCW ,"FNSTCW , D9, FF, 38, 38, 00, 00, doneopr

	{"FUCOMPP,"FUCOMPP, DA, FF, E9, FF, 00, 00, dtwobyt
	{"FADDL  ,"FADDL  , DA, FF, 00, 38, 00, 00, doneopr
	{"FMULL  ,"FMULL  , DA, FF, 08, 38, 00, 00, doneopr
	{"FCOML  ,"FCOML  , DA, FF, 10, 38, 00, 00, doneopr
	{"FCOMPL ,"FCOMPL , DA, FF, 18, 38, 00, 00, doneopr
	{"FSUBL  ,"FSUBL  , DA, FF, 20, 38, 00, 00, doneopr
	{"FSUBRL ,"FSUBRL , DA, FF, 28, 38, 00, 00, doneopr
	{"FDIVL  ,"FDIVL  , DA, FF, 30, 38, 00, 00, doneopr
	{"FDIVRL ,"FDIVRL , DA, FF, 38, 38, 00, 00, doneopr

	{"FNENI  ,"FNENI  , DB, FF, E0, FF, 00, 00, dtwobyt
	{"FNDISI ,"FNDISI , DB, FF, E1, FF, 00, 00, dtwobyt
	{"FNCLEX ,"FNCLEX , DB, FF, E2, FF, 00, 00, dtwobyt
	{"FNINIT ,"FNINIT , DB, FF, E3, FF, 00, 00, dtwobyt
	{"FSETPM ,"FSETPM , DB, FF, E4, FF, 00, 00, dtwobyt
	{"FRSTPM ,"FRSTPM , DB, FF, E5, FF, 00, 00, dtwobyt
	{"FSBP0  ,"FSBP0  , DB, FF, E8, FF, 00, 00, dtwobyt
	{"FSBP2  ,"FSBP2  , DB, FF, EA, FF, 00, 00, dtwobyt
	{"FSBP1  ,"FSBP1  , DB, FF, EB, FF, 00, 00, dtwobyt
	{"FRINT2 ,"FRINT2 , DB, FF, EC, FF, 00, 00, dtwobyt
	{"F4X4   ,"F4X4   , DB, FF, F1, FF, 00, 00, dtwobyt
	{"FILDL  ,"FILDL  , DB, FF, 00, 38, 00, 00, doneopr
	{"FISTTPL,"FISTTPL, DB, FF, 08, 38, 00, 00, doneopr
	{"FSTL   ,"FSTL   , DB, FF, 10, 38, 00, 00, doneopr
	{"FSTPL  ,"FSTPL  , DB, FF, 18, 38, 00, 00, doneopr
	{"FLDX   ,"FLDX   , DB, FF, 28, 38, 00, 00, doneopr
	{"FSTPX  ,"FSTPX  , DB, FF, 38, 38, 00, 00, doneopr

	{"FADD   ,"FADD   , DC, FF, C0, F8, 00, 00, dtwoflt
	{"FMUL   ,"FMUL   , DC, FF, C8, F8, 00, 00, dtwoflt
	{"FCOM   ,"FCOM   , DC, FF, D0, F8, 00, 00, dtwoflt
	{"FCOMP  ,"FCOMP  , DC, FF, D8, F8, 00, 00, dtwoflt
	{"FSUBR  ,"FSUBR  , DC, FF, E0, F8, 00, 00, dtwoflt
	{"FSUB   ,"FSUB   , DC, FF, E8, F8, 00, 00, dtwoflt
	{"FDIVR  ,"FDIVR  , DC, FF, F0, F8, 00, 00, dtwoflt
	{"FDIV   ,"FDIV   , DC, FF, F8, F8, 00, 00, dtwoflt
	{"FADDD  ,"FADDD  , DC, FF, 00, 38, 00, 00, doneopr
	{"FMULD  ,"FMULD  , DC, FF, 08, 38, 00, 00, doneopr
	{"FCOMD  ,"FCOMD  , DC, FF, 10, 38, 00, 00, doneopr
	{"FCOMPD ,"FCOMPD , DC, FF, 18, 38, 00, 00, doneopr
	{"FSUBD  ,"FSUBD  , DC, FF, 20, 38, 00, 00, doneopr
	{"FSUBRD ,"FSUBRD , DC, FF, 28, 38, 00, 00, doneopr
	{"FDIVD  ,"FDIVD  , DC, FF, 30, 38, 00, 00, doneopr
	{"FDIVRD ,"FDIVRD , DC, FF, 38, 38, 00, 00, doneopr

	{"FUCOM  ,"FUCOM  , DD, FF, E1, FF, 00, 00, dtwobyt
	{"FUCOMP ,"FUCOMP , DD, FF, E9, FF, 00, 00, dtwobyt
	{"FRICHOP,"FRICHOP, DD, FF, FC, FF, 00, 00, dtwobyt
	{"FFREE  ,"FFREE  , DD, FF, C0, F8, 00, 00, doneflt
	{"FST    ,"FST    , DD, FF, D0, F8, 00, 00, doneflt
	{"FSTP   ,"FSTP   , DD, FF, D8, F8, 00, 00, doneflt
	{"FUCOM  ,"FUCOM  , DD, FF, E0, F8, 00, 00, doneflt
	{"FUCOMP ,"FUCOMP , DD, FF, E8, F8, 00, 00, doneflt
	{"FLDD   ,"FLDD   , DD, FF, 00, 38, 00, 00, doneopr
	{"FISTTPQ,"FISTTPQ, DD, FF, 08, 38, 00, 00, doneopr
	{"FSTD   ,"FSTD   , DD, FF, 10, 38, 00, 00, doneopr
	{"FSTPD  ,"FSTPD  , DD, FF, 18, 38, 00, 00, doneopr
	{"FRSTOR ,"FRSTOR , DD, FF, 20, 38, 00, 00, doneopr
	{"FNSAVE ,"FNSAVE , DD, FF, 30, 38, 00, 00, doneopr
	{"FNSTSW ,"FNSTSW , DD, FF, 38, 38, 00, 00, doneopr

	{"FADDP  ,"FADDP  , DE, FF, C1, FF, 00, 00, dtwobyt
	{"FMULP  ,"FMULP  , DE, FF, C9, FF, 00, 00, dtwobyt
	{"FCOMPP ,"FCOMPP , DE, FF, D9, FF, 00, 00, dtwobyt
	{"FSUBR  ,"FSUBR  , DE, FF, E1, FF, 00, 00, dtwobyt
	{"FSUBP  ,"FSUBP  , DE, FF, E9, FF, 00, 00, dtwobyt
	{"FDIVRP ,"FDIVRP , DE, FF, F1, FF, 00, 00, dtwobyt
	{"FDIVP  ,"FDIVP  , DE, FF, F9, FF, 00, 00, dtwobyt
	{"FADDP  ,"FADDP  , DE, FF, C0, F8, 00, 00, dtwoflt
	{"FMULP  ,"FMULP  , DE, FF, C8, F8, 00, 00, dtwoflt
	{"FCOMP  ,"FCOMP  , DE, FF, D0, F8, 00, 00, dtwoflt
	{"FSUBP  ,"FSUBP  , DE, FF, E0, F8, 00, 00, dtwoflt
	{"FSUBRP ,"FSUBRP , DE, FF, E8, F8, 00, 00, dtwoflt
	{"FDIVP  ,"FDIVP  , DE, FF, F0, F8, 00, 00, dtwoflt
	{"FDIVRP ,"FDIVRP , DE, FF, F8, F8, 00, 00, dtwoflt
	{"FADDQ  ,"FADDQ  , DE, FF, 00, 38, 00, 00, doneopr
	{"FMULQ  ,"FMULQ  , DE, FF, 08, 38, 00, 00, doneopr
	{"FCOMQ  ,"FCOMQ  , DE, FF, 10, 38, 00, 00, doneopr
	{"FCOMPQ ,"FCOMPQ , DE, FF, 18, 38, 00, 00, doneopr
	{"FSUBQ  ,"FSUBQ  , DE, FF, 20, 38, 00, 00, doneopr
	{"FSUBRQ ,"FSUBRQ , DE, FF, 28, 38, 00, 00, doneopr
	{"FDIVQ  ,"FDIVQ  , DE, FF, 30, 38, 00, 00, doneopr
	{"FDIVRQ ,"FDIVRQ , DE, FF, 38, 38, 00, 00, doneopr

	{"FFREEP ,"FFREEP , DF, FF, C1, FF, 00, 00, dtwobyt
	{"FXCH   ,"FXCH   , DF, FF, C9, FF, 00, 00, dtwobyt
	{"FNSTSAX,"FNSTSAX, DF, FF, E0, FF, 00, 00, dtwobyt
	{"FRINEAR,"FRINEAR, DF, FF, FC, FF, 00, 00, dtwobyt
	{"FFREEP ,"FFREEP , DF, FF, C0, F8, 00, 00, doneflt
	{"FXCH   ,"FXCH   , DF, FF, C8, F8, 00, 00, doneflt
	{"FSTP   ,"FSTP   , DF, FF, D0, F8, 00, 00, doneflt
	{"FSTP   ,"FSTP   , DF, FF, D8, F8, 00, 00, doneflt
	{"FILDW  ,"FILDW  , DF, FF, 00, 38, 00, 00, doneopr
	{"FISTTPW,"FISTTPW, DF, FF, 08, 38, 00, 00, doneopr
	{"FSTQ   ,"FSTQ   , DF, FF, 10, 38, 00, 00, doneopr
	{"FSTPQ  ,"FSTPQ  , DF, FF, 18, 38, 00, 00, doneopr
	{"FLDB   ,"FLDB   , DF, FF, 20, 38, 00, 00, doneopr
	{"FILDQ  ,"FILDQ  , DF, FF, 28, 38, 00, 00, doneopr
	{"FSTPB  ,"FSTPB  , DF, FF, 30, 38, 00, 00, doneopr
	{"FSTPO  ,"FSTPO  , DF, FF, 38, 38, 00, 00, doneopr

	{"ESC    ,"ESC    , D8, F8, 00, 00, 00, 00, donebyt

	{"LOOPNE ,"LOOPNE , E0, FF, 00, 00, 00, 00, dloop
	{"LOOPE  ,"LOOPE  , E1, FF, 00, 00, 00, 00, dloop
	{"LOOP   ,"LOOP   , E2, FF, 00, 00, 00, 00, dloop
	{"JREGZ  ,"JREGZ  , E3, FF, 00, 00, 00, 00, dloop
	{"INB    ,"INB    , E4, FF, 00, 00, 00, 00, diofix
	{"INW    ,"INL    , E5, FF, 00, 00, 00, 00, diofix
	{"OUTB   ,"OUTB   , E6, FF, 00, 00, 00, 00, diofix
	{"OUTW   ,"OUTL   , E7, FF, 00, 00, 00, 00, diofix
	{"CALL   ,"CALL   , E8, FF, 00, 00, 00, 00, ddirjmpn
	{"JMP    ,"JMP    , E9, FF, 00, 00, 00, 00, ddirjmpn
	{"JMPF   ,"JMPF   , EA, FF, 00, 00, 00, 00, ddirjmpf
	{"FLTJMP ,"FLTJMP , EB, FF, 05, FF, 90, FF, dfltjmp
	{"JMP    ,"JMP    , EB, FF, 00, 00, 00, 00, dbranch
	{"INB    ,"INB    , EC, FF, 00, 00, 00, 00, diovar
	{"INW    ,"INL    , ED, FF, 00, 00, 00, 00, diovar
	{"OUTB   ,"OUTB   , EE, FF, 00, 00, 00, 00, diovar
	{"OUTW   ,"OUTL   , EF, FF, 00, 00, 00, 00, diovar
	{"LOCK   ,"LOCK   , F0, FF, 00, 00, 00, 00, donebyt
	{ NULL     ,NULL       , F2, FE, 00, 00, 00, 00, dreppfx
	{"HLT    ,"HLT    , F4, FF, 00, 00, 00, 00, donebyt
	{"CMC    ,"CMC    , F5, FF, 00, 00, 00, 00, donebyt
	{"TESTB  ,"TESTB  , F6, FF, 00, 38, 00, 00, dim2mem
	{"NOTB   ,"NOTB   , F6, FF, 10, 38, 00, 00, doneopr
	{"NEGB   ,"NEGB   , F6, FF, 18, 38, 00, 00, doneopr
	{"MULB   ,"MULB   , F6, FF, 20, 38, 00, 00, doneopr
	{"IMULB  ,"IMULB  , F6, FF, 28, 38, 00, 00, doneopr
	{"DIVB   ,"DIVB   , F6, FF, 30, 38, 00, 00, doneopr
	{"IDIVB  ,"IDIVB  , F6, FF, 38, 38, 00, 00, doneopr
	{"TESTW  ,"TESTL  , F7, FF, 00, 38, 00, 00, dim2mem
	{"NOTW   ,"NOTL   , F7, FF, 10, 38, 00, 00, doneopr
	{"NEGW   ,"NEGL   , F7, FF, 18, 38, 00, 00, doneopr
	{"MULW   ,"MULL   , F7, FF, 20, 38, 00, 00, doneopr
	{"IMULW  ,"IMULL  , F7, FF, 28, 38, 00, 00, doneopr
	{"DIVW   ,"DIVL   , F7, FF, 30, 38, 00, 00, doneopr
	{"IDIVW  ,"IDIVL  , F7, FF, 38, 38, 00, 00, doneopr
	{"CLC    ,"CLC    , F8, FF, 00, 00, 00, 00, donebyt
	{"STC    ,"STC    , F9, FF, 00, 00, 00, 00, donebyt
	{"CLI    ,"CLI    , FA, FF, 00, 00, 00, 00, donebyt
	{"STI    ,"STI    , FB, FF, 00, 00, 00, 00, donebyt
	{"CLD    ,"CLD    , FC, FF, 00, 00, 00, 00, donebyt
	{"STD    ,"STD    , FD, FF, 00, 00, 00, 00, donebyt
	{"INCB   ,"INCB   , FE, FF, 00, 38, 00, 00, doneopr
	{"DECB   ,"DECB   , FE, FF, 08, 38, 00, 00, doneopr
	{"INCW   ,"INCL   , FF, FF, 00, 38, 00, 00, doneopr
	{"DECW   ,"DECL   , FF, FF, 08, 38, 00, 00, doneopr
	{"CALLI  ,"CALLI  , FF, FF, 10, 38, 00, 00, doneopr
	{"CALLFI ,"CALLFI , FF, FF, 18, 38, 00, 00, doneopr
	{"JMPIW  ,"JMPIL  , FF, FF, 20, 38, 00, 00, doneopr
	{"JMPFIW ,"JMPFIL , FF, FF, 28, 38, 00, 00, doneopr
	{"PUSHW  ,"PUSHL  , FF, FF, 30, 38, 00, 00, doneopr

