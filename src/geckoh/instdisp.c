// ++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include "geckoh.h"


#define RTN_DONE  0
#define RTN_MORE  1
#define RTN_ERROR 2

// Define values for the rgopsz byte

#define SZ_OPR 0		// Use size of operand for register size
#define SZ_8   2		// 8 bit register
#define SZ_16  4		// 16 bit register
#define SZ_32  6		// 32 bit register


#define IRG_AX 0
#define IRG_CX 1
#define IRG_DX 2
#define IRG_BX 3
#define IRG_SP 4
#define IRG_BP 5
#define IRG_SI 6
#define IRG_DI 7
#define IRG_AL 8
#define IRG_CL 9
#define IRG_DL 10
#define IRG_BL 11
#define IRG_AH 12
#define IRG_CH 13
#define IRG_DH 14
#define IRG_BH 15
#define IRG_ES 16
#define IRG_CS 17
#define IRG_SS 18
#define IRG_DS 19
#define IRG_FS 20
#define IRG_GS 21

static char reg32tbl[][4] =
{	"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI",
	"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH",
	"ES", "CS", "SS", "DS", "FS", "GS", "?S", "?S"
};

static char *reg16tbl[] = {"BX+SI", "BX+DI", "BP+SI", "BP+DI", "SI", "DI",
		NULL, "BX"};




typedef int  IFUNC(void);
typedef int  RFUNC(IFUNC **);


IFUNC instADD, instPUSHES, instPOPES, instOR, instPUSHCS;
IFUNC esc0F, instADC, instPUSHSS, instPOPSS, instSBB;
IFUNC instPUSHDS, instPOPDS, instAND, pfixES, instDAA;
IFUNC instSUB, pfixCS, instDAS, instXOR, pfixSS;
IFUNC instAAA, instCMP, pfixDS, instAAS, instPUSHA;
IFUNC instPOPA, instBOUND, instARPL, pfixFS, pfixGS;
IFUNC pfixosz, pfixasz, instPOPmem, instPUSHi, xinstIMUL2, instIMUL3, instINS;
IFUNC instOUTS, instimmed1, instTEST, instTESTi, instXCHG, instXCHGAX, instMOV;
IFUNC instLEA, instNOP, instshift;
IFUNC instCBW, instCWD, instCALLF, instFWAIT, instPUSHF;
IFUNC instPOPF, instSAHF, instLAHF, instMOVS;
IFUNC instCMPS, instSTOS, instMOVo, instMOVsr;
IFUNC instLODS, instSCAS, instRET;
IFUNC instLDS, instLES, xinstLSS, xinstLFS, xinstLGS;
IFUNC instmigrp, instENTER, instLEAVE;
IFUNC instINT3, instINT, instINTO, instIRET;
IFUNC instAAMAAD, instbad, instXLAT, instESC;
IFUNC instLOOP, instINOUT, instCALL, instJMP, instJMPF, instJMPs;
IFUNC instLOCK, pfixREPNE, pfixREP;
IFUNC instHLT, instCMC, instgroup3, instCLC, instSTC;
IFUNC instCLI, instSTI, instCLD, instSTD, instgroup4, instgroup5;

RFUNC dorow, instINCDEC, instPUSHPOPreg, instJcc, instMOVi;


IFUNC xinstbad;

IFUNC xinstLAR;
IFUNC xinstLSL;
IFUNC xinstCLTS;

IFUNC xinstINVD;
IFUNC xinstWBINVD;

IFUNC xinstUD2;

IFUNC xinstgroup6;
IFUNC xinstgroup7;
IFUNC xinstgroup9;

IFUNC xinstMOVcrdr;

IFUNC xinstWRMSR;
IFUNC xinstRDTSC;
IFUNC xinstRDMSR;
IFUNC xinstRDPMC;
IFUNC xinstCPUID;
IFUNC xinstRSM;

IFUNC xinstSYSENTER;
IFUNC xinstSYSEXIT;

IFUNC xinstPPFSGS;
IFUNC xinstPPFSGS;

IFUNC xinstBTx;
IFUNC xinstBTxi;

IFUNC xinstSHxD;

IFUNC xinstCMPXCHG;

IFUNC xinstBSF;
IFUNC xinstBSR;

IFUNC xinstMOVX;

IFUNC xinstXADD;
IFUNC xinstBSWAP;

RFUNC xrowbad, xinstMOVcc, xinstSETcc, xinstJcc;

static void  putopcode(char *text, int notbyte);
static void  putopcodereg(char *text, int regnum);
static void  putaddrreg(int regnum);
static void  putdatareg(int regnum);
static void  putreladdr(long val);
static void  putsimaddr(long addr);
static void  putregindir(int reg);
static void  putimmedvalue(int notbyte, int expand);
static int   instcommon(char *optxt, int opval, int expand);
static int   putgenaddr(int opval);
static int   putaddroverride(void);
static int   commonjmp(char *optxt);
static int   commonjmpf(char *optxt);
static void  strlopcommon(void);
static void  strsopcommon(void);
static void  str2opcommon(void);
static void  instimmediate(char *tbl[], int opval);


IFUNC *row0tbl[] =
{	instADD,
	instADD,
	instADD,
	instADD,
	instADD,
	instADD,
	instPUSHES,
	instPOPES,
	instOR,
	instOR,
	instOR,
	instOR,
	instOR,
	instOR,
	instPUSHCS,
	esc0F
};

IFUNC *row1tbl[] =
{	instADC,
	instADC,
	instADC,
	instADC,
	instADC,
	instADC,
	instPUSHSS,
	instPOPSS,
	instSBB,
	instSBB,
	instSBB,
	instSBB,
	instSBB,
	instSBB,
	instPUSHDS,
	instPOPDS
};

IFUNC *row2tbl[] =
{	instAND,
	instAND,
	instAND,
	instAND,
	instAND,
	instAND,
	pfixES,
	instDAA,
	instSUB,
	instSUB,
	instSUB,
	instSUB,
	instSUB,
	instSUB,
	pfixCS,
	instDAS
};

IFUNC *row3tbl[] =
{	instXOR,
	instXOR,
	instXOR,
	instXOR,
	instXOR,
	instXOR,
	pfixSS,
	instAAA,
	instCMP,
	instCMP,
	instCMP,
	instCMP,
	instCMP,
	instCMP,
	pfixDS,
	instAAS
};

IFUNC *row6tbl[] =
{	instPUSHA,
	instPOPA,
	instBOUND,
	instARPL,
	pfixFS,
	pfixGS,
	pfixosz,
	pfixasz,
	instPUSHi,
	instIMUL3,
	instPUSHi,
	instIMUL3,
	instINS,
	instINS,
	instOUTS,
	instOUTS
};

IFUNC *row8tbl[] =
{	instimmed1,
	instimmed1,
	instimmed1,
	instimmed1,
	instTEST,
	instTEST,
	instXCHG,
	instXCHG,
	instMOV,
	instMOV,
	instMOV,
	instMOV,
	instMOVsr,
	instLEA,
	instMOVsr,
	instPOPmem
};

IFUNC *row9tbl[] =
{	instNOP,
	instXCHGAX,
	instXCHGAX,
	instXCHGAX,
	instXCHGAX,
	instXCHGAX,
	instXCHGAX,
	instXCHGAX,
	instCBW,
	instCWD,
	instCALLF,
	instFWAIT,
	instPUSHF,
	instPOPF,
	instSAHF,
	instLAHF
};

IFUNC *rowAtbl[] =
{	instMOVo,
	instMOVo,
	instMOVo,
	instMOVo,
	instMOVS,
	instMOVS,
	instCMPS,
	instCMPS,
	instTESTi,
	instTESTi,
	instSTOS,
	instSTOS,
	instLODS,
	instLODS,
	instSCAS,
	instSCAS
};

IFUNC *rowCtbl[] =
{	instshift,
	instshift,
	instRET,
	instRET,
	instLES,
	instLDS,
	instmigrp,
	instmigrp,
	instENTER,
	instLEAVE,
	instRET,
	instRET,
	instINT3,
	instINT,
	instINTO,
	instIRET
};

IFUNC *rowDtbl[] =
{	instshift,
	instshift,
	instshift,
	instshift,
	instAAMAAD,
	instAAMAAD,
	instbad,
	instXLAT,
	instESC,
	instESC,
	instESC,
	instESC,
	instESC,
	instESC,
	instESC,
	instESC
};

IFUNC *rowEtbl[] =
{	instLOOP,
	instLOOP,
	instLOOP,
	instLOOP,
	instINOUT,
	instINOUT,
	instINOUT,
	instINOUT,
	instCALL,
	instJMP,
	instJMPF,
	instJMPs,
	instINOUT,
	instINOUT,
	instINOUT,
	instINOUT
};

IFUNC *rowFtbl[] =
{	instLOCK,
	instbad,
	pfixREPNE,
	pfixREP,
	instHLT,
	instCMC,
	instgroup3,
	instgroup3,
	instCLC,
	instSTC,
	instCLI,
	instSTI,
	instCLD,
	instSTD,
	instgroup4,
	instgroup5
};

typedef struct
{	RFUNC *func;
	IFUNC **data;
} ROW;

ROW rowtbl[] =
{	{dorow         , row0tbl},
	{dorow         , row1tbl},
	{dorow         , row2tbl},
	{dorow         , row3tbl},
	{instINCDEC    , NULL},
	{instPUSHPOPreg, NULL},
	{dorow         , row6tbl},
	{instJcc       , NULL},
	{dorow         , row8tbl},
	{dorow         , row9tbl},
	{dorow         , rowAtbl},
	{instMOVi      , NULL},
	{dorow         , rowCtbl},
	{dorow         , rowDtbl},
	{dorow         , rowEtbl},
	{dorow         , rowFtbl},
};




IFUNC *xrow0tbl[] =
{	xinstgroup6,
	xinstgroup7,
	xinstLAR,
	xinstLSL,
	xinstbad,
	xinstbad,
	xinstCLTS,
	xinstbad,
	xinstINVD,
	xinstWBINVD,
	xinstbad,
	xinstUD2,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad
};


IFUNC *xrow2tbl[] =
{	xinstMOVcrdr,
	xinstMOVcrdr,
	xinstMOVcrdr,
	xinstMOVcrdr,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad
};


IFUNC *xrow3tbl[] =
{	xinstWRMSR,
	xinstRDTSC,
	xinstRDMSR,
	xinstRDPMC,
	xinstSYSENTER,
	xinstSYSEXIT,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad
};


IFUNC *xrowAtbl[] =
{	xinstPPFSGS,
	xinstPPFSGS,
	xinstCPUID,
	xinstBTx,
	xinstSHxD,
	xinstSHxD,
	xinstbad,
	xinstbad,
	xinstPPFSGS,
	xinstPPFSGS,
	xinstRSM,
	xinstBTx,
	xinstSHxD,
	xinstSHxD,
	xinstbad,
	xinstIMUL2
};



IFUNC *xrowBtbl[] =
{	xinstCMPXCHG,
	xinstCMPXCHG,
	xinstLSS,
	xinstBTx,
	xinstLFS,
	xinstLGS,
	xinstMOVX,
	xinstMOVX,
	xinstbad,
	xinstbad,
	xinstBTxi,
	xinstBTx,
	xinstBSF,
	xinstBSR,
	xinstMOVX,
	xinstMOVX
};


IFUNC *xrowCtbl[] =
{	xinstXADD,
	xinstXADD,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstbad,
	xinstgroup9,
	xinstBSWAP,
	xinstBSWAP,
	xinstBSWAP,
	xinstBSWAP,
	xinstBSWAP,
	xinstBSWAP,
	xinstBSWAP,
	xinstBSWAP
};


ROW xrowtbl[] =
{	{dorow     , xrow0tbl},
	{xrowbad   , NULL},
	{dorow     , xrow2tbl},
	{dorow     , xrow3tbl},
	{xinstMOVcc, NULL},
	{xrowbad   , NULL},
	{xrowbad   , NULL},
	{xrowbad   , NULL},
	{xinstJcc  , NULL},
	{xinstSETcc, NULL},
	{dorow     , xrowAtbl},
	{dorow     , xrowBtbl},
	{dorow     , xrowCtbl},
	{xrowbad   , NULL},
	{xrowbad   , NULL},
	{xrowbad   , NULL}
};


char condtbl[][4] =
{	"O", "NO", "C", "NC", "E", "NE", "BE", "A",
	"S", "NS", "P", "NP", "L", "GE", "LE", "G"
};

char *looptbl[] = {"LOOPNE", "LOOPE", "LOOP", "JREGZ"};
char *immed1tbl[] = {"ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"};
char *shifttbl[] = {"ROL", "ROR", "RCL", "RCR", "SHL", "SHR", NULL, "SAR"};
char *group3tbl[] = {"TEST", NULL, "NOT", "NEG", "MUL", "IMUL", "DIV", "IDIV"};
char *group5tbl[] = {"INC", "DEC", "CALLI", "CALLFI", "JMPI", "JMPFI", "PUSH",
		NULL};
char *group6tbl[] = {"SLDT", "STR", "LLDT", "LTR", "VERR", "VERW", NULL, NULL};
char *group7tbl[] = {"SGDT", "SIDT", "LGDT", "LIDT", "SMSW", NULL, "LMSW",
		"INVLPG"};
char *bttbl[] = {"BT", "BTS", "BTR", "BTC"};

uchar reppfx;
uchar segpfx;
uchar addrsz32;
uchar datasz32;
uchar oofetch;

#define ST_KNOWN 0x04
#define ST_32BIT 0x02
#define ST_CODE  0x01

uchar *instpnt;
int    dodisp;
int    inst32;


//*************************************************
// Function: instdisp - Diplay value as instruction
// Returned: Number of instruction bytes used
//*************************************************

int instdisp(
	int    type,
	uchar *inst,
	char  *text)
{
	ROW *rowpnt;
	int  rtn;

	reppfx = 0;							// No repeat prefix yet
	segpfx = 0;							// No segment prefix yet
	oofetch = 0;						// No additional bytes yet
	instpnt = inst;
	txtpnt = text;
	datasz32 = addrsz32 = ((type & SGTP_16BIT) == 0);
	do
	{
		rowpnt = rowtbl + (instpnt[0] >> 4);
	} while ((rtn = (rowpnt->func)(rowpnt->data)) == RTN_MORE);
	return (instpnt - inst);
}


// **********************************************************
// Function: dorow - Dispatch on the instruction within a row
// Returned: Return code
// **********************************************************

int dorow(
	IFUNC **func)
{
	return ((func[instpnt[0] & 0x0F])());
}


// ***********************************************************************
// Function: instINCDEC - Display increment/decrement instructions (row 4)
// Returned: Return code
// ***********************************************************************

int instINCDEC(
	IFUNC **func)
{
	func = func;

	putopcodereg((instpnt[0] & 0x08) ? "DEC" : "INC",
			instpnt[0] & 0x07);
	instpnt++;
	return (RTN_DONE);
}


// ****************************************************************
// Function: instPUSHPOPreg - Display push/pop instructions (row 5)
// Returned: Return code
// ****************************************************************

int instPUSHPOPreg(
	IFUNC **func)
{
	func = func;

	putopcodereg((instpnt[0] & 0x08) ? "POP" : "PUSH",
			instpnt[0] & 0x07);
	instpnt++;
	return (RTN_DONE);
}


// *****************************************************************
// Function: instJcc - Display conditional jump instructions (row 7)
// Returned: Return code
// *****************************************************************

int instJcc(
	IFUNC **func)
{
	func = func;

	*txtpnt++ = 'J';
	txtpnt = strmov(txtpnt, condtbl[instpnt[0] & 0x0F]);
	*txtpnt++ = '\t';
	instpnt += 2;
	putreladdr(*(si8 *)(instpnt - 1));
	txtpnt = strmov(txtpnt, ".S");
	return (RTN_DONE);
}


// ***************************************************************
// Function: instMOVi - Display MOV immediate instructions (row B)
// Returned: Return code
// ***************************************************************

int instMOVi(
	IFUNC **func)
{
	func = func;

	putopcodereg("MOV", (instpnt[0] & 0x0F) ^ 0x08);
	instpnt++;
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	putimmedvalue(instpnt[-1] & 0x08, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: instTESTi -
// Returned: Nothing
//***************************************

int instTESTi(void)
{
	int notbyte;

	notbyte = instpnt[0] & 0x01;
	putopcode("TEST", notbyte);;
	*txtpnt++ = '\t';
	putdatareg((notbyte) ? IRG_AX : IRG_AL);
	instpnt++;
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	putimmedvalue(notbyte, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: pfixES -
// Returned: Nothing
//***************************************

int pfixES(void)
{
	segpfx = IRG_ES;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: pfixCS -
// Returned: Nothing
//***************************************

int pfixCS(void)
{
	segpfx = IRG_CS;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: pfixSS -
// Returned: Nothing
//***************************************

int pfixSS(void)
{
	segpfx = IRG_SS;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: pfixDS -
// Returned: Nothing
//***************************************

int pfixDS(void)
{
	segpfx = IRG_DS;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: pfixFS -
// Returned: Nothing
//***************************************

int pfixFS(void)
{
	segpfx = IRG_FS;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: pfixGS -
// Returned: Nothing
//***************************************

int pfixGS(void)
{
	segpfx = IRG_GS;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: pfixosz -
// Returned: Nothing
//***************************************

int pfixosz(void)
{
	datasz32 ^= 0x01;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: pfixasz -
// Returned: Nothing
//***************************************

int pfixasz(void)
{
	addrsz32 ^= 0x01;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: pfixRENE -
// Returned: Nothing
//***************************************

int pfixREPNE(void)
{
	reppfx = 2;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: pfixREP -
// Returned: Nothing
//***************************************

int pfixREP(void)
{
	reppfx = 1;
	instpnt++;
	return (RTN_MORE);
}


//***************************************
// Function: instADD -
// Returned: Nothing
//***************************************

int instADD(void)
{
	return (instcommon("ADD", instpnt[0], TRUE));
}


//***************************************
// Function: instADC -
// Returned: Nothing
//***************************************

int instADC(void)
{
	return (instcommon("ADC", instpnt[0], TRUE));
}


//***************************************
// Function: instAND -
// Returned: Nothing
//***************************************

int instAND(void)
{
	return (instcommon("AND", instpnt[0], TRUE));
}


//***************************************
// Function: instXOR -
// Returned: Nothing
//***************************************

int instXOR(void)
{
	if ((instpnt[1] & 0xC0) == 0xC0 &&	// Is this really CLR?
			((instpnt[1] >> 3) & 0x07) == (instpnt[1] & 0x07))
	{
		putopcodereg("CLR", (instpnt[1] & 0x07) +
				(((instpnt[0] ^ 0x01) & 0x01) << 3));
		instpnt += 2;
		return (RTN_DONE);
	}
	return (instcommon("XOR", instpnt[0], TRUE));
}


//***************************************
// Function: instOR -
// Returned: Nothing
//***************************************

int instOR(void)
{
	return (instcommon("OR", instpnt[0], TRUE));
}


//***************************************
// Function: instSBB -
// Returned: Nothing
//***************************************

int instSBB(void)
{
	return (instcommon("SBB", instpnt[0], TRUE));
}


//***************************************
// Function: instSUB -
// Returned: Nothing
//***************************************

int instSUB(void)
{
	return (instcommon("SUB", instpnt[0], TRUE));
}


//***************************************
// Function: instCMP -
// Returned: Nothing
//***************************************

int instCMP(void)
{
	return (instcommon("CMP", instpnt[0], TRUE));
}


//***************************************
// Function: instTEST -
// Returned: Nothing
//***************************************

int instTEST(void)
{
	return (instcommon("TEST", instpnt[0] & 0x01, FALSE));
}


//***************************************
// Function: instXCHG -
// Returned: Nothing
//***************************************

int instXCHG(void)
{
	return (instcommon("XCHG", instpnt[0] & 0x01, FALSE));
}


//***************************************
// Function: instMOV -
// Returned: Nothing
//***************************************

int instMOV(void)
{
	return (instcommon("MOV", instpnt[0] & 0x03, FALSE));
}


//*******************************************************************
// Function: instcommon - Display instructions using ModR/M-SIB bytes
// Returned: Return code
//*******************************************************************

static int instcommon(
	char *optxt,
	int   opval,
	int   expand)
{
	int size;

	if (opval & 0x04)				// Immediate mode instruction?
	{
		putopcodereg(optxt, (opval & 0x01) ? IRG_AX : IRG_AL);
		*txtpnt++ = ',';
		*txtpnt++ = ' ';
		instpnt++;
		putimmedvalue(opval & 0x01, expand);
		return (RTN_DONE);
	}
	putopcode(optxt, opval & 0x01);
	*txtpnt++ = '\t';
	if (opval & 0x02)
	{
///		putdatareg(((instpnt[1] >> 3) & 0x07) + ((instpnt[0] & 0x01) ? 0 : 8));
		putdatareg(((instpnt[1] >> 3) & 0x07) + ((opval & 0x01) ? 0 : 8));
		*txtpnt++ = ',';
		*txtpnt++ = ' ';
		size = putgenaddr(opval);
	}
	else
	{
		size = putgenaddr(opval);
		*txtpnt++ = ',';
		*txtpnt++ = ' ';
///		putdatareg(((instpnt[1] >> 3) & 0x07) + ((instpnt[0] & 0x01) ? 0 : 8));
		putdatareg(((instpnt[1] >> 3) & 0x07) + ((opval & 0x01) ? 0 : 8));
	}
	instpnt += size;
	return (RTN_DONE);
}


static int putgenaddr(
	int opval)
{
	static char reginxtbl[] = {0, '2', '4', '8'};

	int rm;
	int mod;
	int base;
	int scale;
	int index;
	int size;
	int selval;

	size = 2;							// We at least have the ModR/M byte in
	rm = instpnt[1];					//   addition to the opcode bytes
	mod = rm >> 6;
	rm &= 0x07;
	if (mod != 3)
		selval = putaddroverride();
	if (addrsz32)
	{
		if (rm == 4 && mod != 3)		// Do we have an SIB byte?
		{
			size++;						// Yes
			base = instpnt[2];
			scale = base >> 6;
			index = (base >> 3) & 0x07;
			base &= 0x07;
			if (mod == 0 && base == 5)	// Special mode?
			{
				base = -1;				// Yes - indicate no base register
				mod = 2;				// Indicate 32-bit offset
			}
			else if (mod == 1)			// 8-bit offset?
			{
				size++;
				txtpnt = fmtnumeric(txtpnt, *(si8 *)(instpnt + 3));
				*txtpnt++ = '.';
				*txtpnt++ = 'B';
			}
			else if (mod == 2)			// 32-bit offset?
			{
				size += 4;
				txtpnt = fmtnumeric(txtpnt, *(long *)(instpnt + 3));
			}
			if (base >= 0)
			{
				*txtpnt++ = '[';
				putaddrreg(base);
				if (index != 4)
					*txtpnt++ = '+';
			}
			else
			{
				size += 4;
				txtpnt = fmtnumeric(txtpnt, *(long *)(instpnt + 3));
				*txtpnt++ = '[';
			}
			if (index != 4)
			{
				putaddrreg(index);
				if (scale != 0)
				{
					*txtpnt++ = '*';
					*txtpnt++ = reginxtbl[scale];
				}
			}
			*txtpnt++ = ']';
		}
		else if (mod == 0)				// No SIB - index without offset?
		{
			if (rm == 5)				// Maybe - simple memory address?
			{
				size += 4;
				txtpnt = fmtnumeric(txtpnt, *(long *)(instpnt + 2));
			}
			else						// Really index without offset?
			{
				*txtpnt++ = '[';
				putaddrreg(rm);
				*txtpnt++ = ']';
				*txtpnt = 0;
			}
		}
		else if (mod == 1)				// Index + 8-bit offset?
		{
			size++;
			txtpnt = fmtnumeric(txtpnt, *(si8 *)(instpnt + 2));
			txtpnt = strmov(txtpnt, ".B[");
			putaddrreg(rm);
			*txtpnt++ = ']';
		}
		else if (mod == 2)				// Index + 32-bit offset?
		{
			size += 4;
			txtpnt = fmtnumeric(txtpnt, *(long *)(instpnt + 2));
			*txtpnt++ = '[';
			putaddrreg(rm);
			*txtpnt++ = ']';
		}
		else							// Register
			putdatareg((instpnt[1] & 0x07) +
					((opval & 0x01) ? 0 : 8));
	}
	else								// 16-bit addressing
	{
		if (mod == 0)					// Index without offset?
		{
			if (rm == 6)				// Maybe - simple memory address?
			{
				size += 2;
				txtpnt = fmtnumeric(txtpnt, *(ushort *)(instpnt + 2));
			}
			else
				txtpnt += sprintf(txtpnt, "[%s]", reg16tbl[rm]);
		}
		else if (mod == 1)				// Index + 8-bit displacement?
		{
			size++;
			txtpnt = fmtnumeric(txtpnt, *(si8 *)(instpnt + 2));
			txtpnt = strmov(txtpnt, ".B[");
			putaddrreg(rm);
			*txtpnt++ = ']';
		}
		else if (mod == 2)				// Index + 16-bit displacement?
		{
			size += 2;
			txtpnt = fmtnumeric(txtpnt, *(short *)(instpnt + 2));
			*txtpnt++ = '[';
			putaddrreg(rm);
			*txtpnt++ = ']';
		}
		else
			putdatareg((instpnt[1] & 0x07) +
					((opval & 0x01) ? 0 : 8));
	}
	*txtpnt = 0;						// Make sure have null at end
	return (size);
}


//**********************************************************
// Function: instMOVo - Display MOV AL/AX with simple offset
// Returned: Nothing
//**********************************************************

int instMOVo(void)
{
	long addr;
	int  reg;
	int  size;

	putopcode("MOV", instpnt[0] & 0x01);
	reg = (instpnt[0] & 0x01) ? 0 : 8;
	if (datasz32)
	{
		addr = *(long *)(instpnt + 1);
		size = 5;
	}
	else
	{
		addr = *(ushort *)(instpnt + 1);
		size = 3;
	}
	*txtpnt++ = '\t';
	if (instpnt[0] & 0x02)
	{
		putsimaddr(addr);
		*txtpnt++ = ',';
		*txtpnt++ = ' ';
		putdatareg(reg);
	}
	else
	{
		putdatareg(reg);
		*txtpnt++ = ',';
		*txtpnt++ = ' ';
		putsimaddr(addr);
	}
	instpnt += size;
	return (RTN_DONE);
}


//**************************************************************
// Function: instMOVsg - Display MOV to or from segment register
// Returned: Nothing
//**************************************************************

int instMOVsr(void)
{
	int size;

	txtpnt = strmov(txtpnt, "MOVW\t");
	datasz32 = FALSE;
	if (instpnt[0] & 0x02)
	{
		txtpnt = strmov(txtpnt, reg32tbl[((instpnt[1] >> 3) & 0x07) + 16]);
		*txtpnt++ = ',';
		*txtpnt++ = ' ';
		size = putgenaddr(0x01);
	}
	else
	{
		size = putgenaddr(0x01);
		*txtpnt++ = ',';
		*txtpnt++ = ' ';
		txtpnt = strmov(txtpnt, reg32tbl[((instpnt[1] >> 3) & 0x07) + 16]);
	}
	instpnt += size;
	return (RTN_DONE);
}


//***************************************
// Function: instXCHGAX -
// Returned: Nothing
//***************************************

int instXCHGAX(void)
{
	putopcodereg("XCHG", TRUE);
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	putdatareg(instpnt[0] & 0x07);
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instimmed1 -
// Returned: Nothing
//***************************************

int instimmed1(void)
{
	instimmediate(immed1tbl, instpnt[0]);
	return (RTN_DONE);
}


static void instimmediate(
	char *tbl[],
	int   opval)
{
	putopcode(tbl[(instpnt[1] >> 3) & 0x07], opval & 0x01);
	*txtpnt++ = '\t';
	instpnt += putgenaddr(opval);
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	putimmedvalue((opval & 0x03) == 1, (opval & 0x03) == 3);
}


//*****************************************
// Function: instshift - Shift instructions
// Returned: Nothing
//*****************************************

int instshift(void)
{
	int opval;
	int inx;

	opval = instpnt[0];
	inx = (instpnt[1] >> 3) & 0x07;
	if (shifttbl[inx] == NULL)
		return (RTN_ERROR);
	putopcode(shifttbl[inx], opval & 0x01);
	*txtpnt++ = '\t';
	instpnt += putgenaddr(opval);
	if ((opval & 0x10) == 0)
	{
		txtpnt = strmov(txtpnt, ", #");
		txtpnt = fmtnumeric(txtpnt, *instpnt);
		instpnt++;
	}
	else if (opval & 0x02)
		txtpnt = strmov(txtpnt, (opval & 0x02) ? ", CL" : ", #1");
	return (RTN_DONE);
}


//***************************************
// Function: instgroup3 -
// Returned: Nothing
//***************************************

int instgroup3(void)
{
	int opval;
	int inx;

	opval = instpnt[0];
	if ((instpnt[1] & 0x38) == 0)
		instimmediate(group3tbl, instpnt[0] & 0x01);
	else
	{
		inx = (instpnt[1] >> 3) & 0x07;
		if (group3tbl[inx] == NULL)
			return (RTN_ERROR);
		putopcode(group3tbl[inx], opval & 0x01);
		*txtpnt++ = '\t';
		instpnt += putgenaddr(opval);
	}
	return (RTN_DONE);
}


//***************************************
// Function: instPUSHES -
// Returned: Nothing
//***************************************

int instPUSHES(void)
{
	txtpnt = strmov(txtpnt, (addrsz32) ? "PUSHL\tES" : "PUSHW\tES");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPOPES -
// Returned: Nothing
//***************************************

int instPOPES(void)
{
	txtpnt = strmov(txtpnt, (addrsz32) ? "POPL\tES" : "POPW\tES");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPUSHCS -
// Returned: Nothing
//***************************************

int instPUSHCS(void)
{
	txtpnt = strmov(txtpnt, (addrsz32) ? "PUSHL\tCS" : "PUSHW\tCS");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPUSHSS -
// Returned: Nothing
//***************************************

int instPUSHSS(void)
{
	txtpnt = strmov(txtpnt, (addrsz32) ? "PUSHL\tSS" : "PUSHW\tSS");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPOPSS -
// Returned: Nothing
//***************************************

int instPOPSS(void)
{
	txtpnt = strmov(txtpnt, (addrsz32) ? "POPL\tSS" : "POPW\tSS");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPUSHDS -
// Returned: Nothing
//***************************************

int instPUSHDS(void)
{
	txtpnt = strmov(txtpnt, (addrsz32) ? "PUSHL\tDS" : "PUSHW\tDS");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPOPDS -
// Returned: Nothing
//***************************************

int instPOPDS(void)
{
	txtpnt = strmov(txtpnt, (addrsz32) ? "POPL\tDS" : "POPW\tDS");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instDAA -
// Returned: Nothing
//***************************************

int instDAA(void)
{
	txtpnt = strmov(txtpnt, "DAA");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instDAS -
// Returned: Nothing
//***************************************

int instDAS(void)
{
	txtpnt = strmov(txtpnt, "DAS");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instAAA -
// Returned: Nothing
//***************************************

int instAAA(void)
{
	txtpnt = strmov(txtpnt, "AAA");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instAAS -
// Returned: Nothing
//***************************************

int instAAS(void)
{
	txtpnt = strmov(txtpnt, "AAS");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPUSHA -
// Returned: Nothing
//***************************************

int instPUSHA(void)
{
	txtpnt = strmov(txtpnt, (addrsz32) ? "PUSHAL" : "PUSHAW");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPOPA -
// Returned: Nothing
//***************************************

int instPOPA(void)
{
	txtpnt = strmov(txtpnt, (addrsz32) ? "POPAL" : "POPAW");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instBOUND -
// Returned: Nothing
//***************************************

int instBOUND(void)
{
	instcommon("BOUND", 0x01, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: instARPL -
// Returned: Nothing
//***************************************

int instARPL(void)
{
	datasz32 = FALSE;
	instcommon("ARPL", 0, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: instPOPmem -
// Returned: Nothing
//***************************************

int instPOPmem(void)
{
	putopcode("POP", TRUE);
	*txtpnt++ = '\t';
	instpnt += putgenaddr(TRUE);
	return (RTN_DONE);
}


//***************************************
// Function: xinstIMUL2 -
// Returned: Nothing
//***************************************

int xinstIMUL2(void)
{
	return (instcommon("IMUL", 0x03, TRUE));
}


//***************************************
// Function: instIMUL3 -
// Returned: Nothing
//***************************************

int instIMUL3(void)
{
	int notbyte;

	notbyte = (instpnt[0] & 0x02) ^ 0x02;
	instcommon("IMUL", 0x03, FALSE);
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	putimmedvalue(notbyte, TRUE);
	return (RTN_DONE);
}


//***************************************
// Function: instINS -
// Returned: Nothing
//***************************************

int instINS(void)
{
	if (reppfx != 0)
		*txtpnt++ = 'R';
	putopcode("INS", instpnt[0] & 0x01);
	strsopcommon();
	return (RTN_DONE);
}


//***************************************
// Function: instOUTS -
// Returned: Nothing
//***************************************

int instOUTS(void)
{
	if (reppfx != 0)
		*txtpnt++ = 'R';
	putopcode("STOS", instpnt[0] & 0x01);
	strlopcommon();
	return (RTN_DONE);
}


//***************************************
// Function: instLEA -
// Returned: Nothing
//***************************************

int instLEA(void)
{
	return (instcommon("LEA", 0x03, FALSE));
}


//***************************************
// Function: instNOP -
// Returned: Nothing
//***************************************

int instNOP(void)
{
	txtpnt = strmov(txtpnt, "NOP");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instCBW -
// Returned: Nothing
//***************************************

int instCBW(void)
{
	txtpnt = strmov(txtpnt, (datasz32) ? "CBL" : "CBW");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instCWD -
// Returned: Nothing
//***************************************

int instCWD(void)
{
	txtpnt = strmov(txtpnt, "CWD");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instFWAIT -
// Returned: Nothing
//***************************************

int instFWAIT(void)
{
	txtpnt = strmov(txtpnt, "FWAIT");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPUSHF -
// Returned: Nothing
//***************************************

int instPUSHF(void)
{
	txtpnt = strmov(txtpnt, (datasz32) ? "PUSHFL" : "PUSHFW");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instPOPF -
// Returned: Nothing
//***************************************

int instPOPF(void)
{
	txtpnt = strmov(txtpnt, (datasz32) ? "POPFL" : "POPFW");
	instpnt++;
	return (RTN_DONE);
}


//*******************************************************
// Function: instPUSHi - Immediate mode PUSH instructions
// Returned: Nothing
//*******************************************************

int instPUSHi(void)
{
	putopcode("PUSH", TRUE);
	instpnt++;
	*txtpnt++ = '\t';
	putimmedvalue((instpnt[-1] & 0x02) == 0, TRUE);
	return (RTN_DONE);
}


//***************************************
// Function: instSAHF -
// Returned: Nothing
//***************************************

int instSAHF(void)
{
	txtpnt = strmov(txtpnt, "SAHF");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instLAHF -
// Returned: Nothing
//***************************************

int instLAHF(void)
{
	txtpnt = strmov(txtpnt, "LAHF");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instMOVS -
// Returned: Nothing
//***************************************

int instMOVS(void)
{
	if (reppfx != 0)
		*txtpnt++ = 'R';
	putopcode("MOVS", instpnt[0] & 0x01);
	str2opcommon();
	return (RTN_DONE);
}


//***************************************
// Function: instCMPS -
// Returned: Nothing
//***************************************

int instCMPS(void)
{
	if (reppfx != 0)
	{
		*txtpnt++ = 'R';
		*txtpnt++ = (reppfx == 1) ? 'E' : 'N';
	}
	putopcode("CMPS", instpnt[0] & 0x01);
	str2opcommon();
	return (RTN_DONE);
}


static void str2opcommon(void)
{
	*txtpnt++ = '\t';
	*txtpnt++ = '[';
	putaddrreg(IRG_DI);
	txtpnt = strmov(txtpnt, "], ");
	putregindir(IRG_SI);
	instpnt++;
}


//***************************************
// Function: instSTOS -
// Returned: Nothing
//***************************************

int instSTOS(void)
{
	if (reppfx != 0)
		*txtpnt++ = 'R';
	putopcode("STOS", instpnt[0] & 0x01);
	strsopcommon();
	return (RTN_DONE);
}


//***************************************
// Function: instLODS -
// Returned: Nothing
//***************************************

int instLODS(void)
{
	if (reppfx != 0)
		*txtpnt++ = 'R';
	putopcode("LODS", instpnt[0] & 0x01);
	strlopcommon();
	return (RTN_DONE);
}


//***************************************
// Function: instSCAS -
// Returned: Nothing
//***************************************

int instSCAS(void)
{
	if (reppfx != 0)
	{
		*txtpnt++ = 'R';
		*txtpnt++ = (reppfx == 1) ? 'E' : 'N';
	}
	putopcode("SCAS", instpnt[0] & 0x01);
	strsopcommon();
	return (RTN_DONE);
}


static void strsopcommon(void)
{
	*txtpnt++ = '\t';
	*txtpnt++ = '[';
	putaddrreg(IRG_DI);
	*txtpnt++ = ']';
	*txtpnt = 0;
	instpnt++;
}


static void strlopcommon(void)
{
	*txtpnt++ = '\t';
	putregindir(IRG_SI);
	instpnt++;
}


//***************************************
// Function: instRET -
// Returned: Nothing
//***************************************

int instRET(void)
{
	txtpnt = strmov(txtpnt, "RET");
	if (instpnt[0] & 0x08)
		*txtpnt++ = 'F';
	*txtpnt++ = (addrsz32) ? 'L' : 'W';
	*txtpnt = 0;
	if ((instpnt[0] & 0x01) == 0)
	{
		*txtpnt++ = '\t';
		txtpnt = fmtnumeric(txtpnt, *(ushort *)(instpnt + 1));
		instpnt += 2;
	}
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instLDS -
// Returned: Nothing
//***************************************

int instLDS(void)
{
	return (instcommon("LDS", 0x03, FALSE));
}


//***************************************
// Function: instLES -
// Returned: Nothing
//***************************************

int instLES(void)
{
	return (instcommon("LES", 0x03, FALSE));
}


//***************************************
// Function: xinstLSS -
// Returned: Return code
//***************************************

int xinstLSS(void)
{
	return (instcommon("LSS", 0x03, FALSE));
}


//***************************************
// Function: xinstLFS -
// Returned: Return code
//***************************************

int xinstLFS(void)
{
	return (instcommon("LFS", 0x03, FALSE));
}


//***************************************
// Function: xinstLGS -
// Returned: Return code
//***************************************

int xinstLGS(void)
{
	return (instcommon("LGS", 0x03, FALSE));
}


//*******************************************
// Function: instmigrp - MOV immediate to r/m
// Returned: Nothing
//*******************************************

int instmigrp(void)
{
	int opval;

	opval = instpnt[0];
	putopcode("MOV", opval & 0x01);
	*txtpnt++ = '\t';
	instpnt += putgenaddr(opval);
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	putimmedvalue(opval & 0x01, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: instENTER -
// Returned: Nothing
//***************************************

int instENTER(void)
{
	txtpnt = strmov(txtpnt, "ENTER\t");
	txtpnt = fmtnumeric(txtpnt, *(ushort *)(instpnt + 1));
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	txtpnt = fmtnumeric(txtpnt, instpnt[3]);
	instpnt += 4;
	return (RTN_ERROR);
}


//***************************************
// Function: instLEAVE -
// Returned: Nothing
//***************************************

int instLEAVE(void)
{
	txtpnt = strmov(txtpnt, "LEAVE");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instINT3 -
// Returned: Nothing
//***************************************

int instINT3(void)
{
	txtpnt = strmov(txtpnt, "INT3");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instINT -
// Returned: Nothing
//***************************************

int instINT(void)
{
	if (instpnt[1] == 0x1F)
	{
		txtpnt += sprintf(txtpnt, "CRASH\t%.4s", instpnt+2);
		instpnt += 6;
	}
	else
	{
		txtpnt += sprintf(txtpnt, "INT\t%02X", instpnt[1]);
		instpnt += 2;
	}
	return (RTN_DONE);
}


//***************************************
// Function: instINTO -
// Returned: Nothing
//***************************************

int instINTO(void)
{
	txtpnt = strmov(txtpnt, "INTO");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instIRET -
// Returned: Nothing
//***************************************

int instIRET(void)
{
	txtpnt = strmov(txtpnt, "IRET");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instAAMAAD -
// Returned: Nothing
//***************************************

int instAAMAAD(void)
{
	txtpnt = strmov(txtpnt, (instpnt[0] & 0x01) ? "AAD\t" : "AAM\t");
	instpnt++;
	putimmedvalue(FALSE, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: instbad -
// Returned: Nothing
//***************************************

int instbad(void)
{
	return (RTN_ERROR);
}


//***************************************
// Function: instXLAT -
// Returned: Nothing
//***************************************

int instXLAT(void)
{
	txtpnt = strmov(txtpnt, "XLATB\t");
	putaddrreg(IRG_BX);
	return (RTN_DONE);
}


//***************************************
// Function: instESC -
// Returned: Nothing
//***************************************

int instESC(void)
{
	return (RTN_ERROR);
}


//***************************************
// Function: instLOOP -
// Returned: Nothing
//***************************************

int instLOOP(void)
{
	txtpnt = strmov(txtpnt, looptbl[instpnt[0] & 0x03]);
	*txtpnt++ = '\t';
	putaddrreg(IRG_CX);
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	instpnt += 2;
	putreladdr(*(si8 *)(instpnt - 1));
	return (RTN_DONE);
}


//**************************************
// Function: instINOUT - IO instructions
// Returned: Nothing
//**************************************

int instINOUT(void)
{
	putopcode((instpnt[0] & 0x02) ? "OUT" : "IN", instpnt[0] & 0x01);
	*txtpnt++ = '\t';
	instpnt++;
	if (instpnt[-1] & 0x08)
	{
		addrsz32 = FALSE;
		putaddrreg(IRG_DX);
	}
	else
		putimmedvalue(FALSE, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: instJMPs -
// Returned: Nothing
//***************************************

int instJMPs(void)
{
	txtpnt = strmov(txtpnt, "JMP\t");
	instpnt += 2;
	putreladdr(*(si8 *)(instpnt - 1));
	txtpnt = strmov(txtpnt, ".S");
	return (RTN_DONE);
}


//***************************************
// Function: instCALL -
// Returned: Nothing
//***************************************

int instCALL(void)
{
	return (commonjmp("CALL"));
}


//***************************************
// Function: instJMP -
// Returned: Nothing
//***************************************

int instJMP(void)
{
	return (commonjmp("JMP"));
}


static int commonjmp(
	char *optxt)
{
	long addr;

	if (addrsz32)
	{
		addr = *(long *)(instpnt + 1);
		instpnt += 5;
	}
	else
	{
		addr = *(short *)(instpnt + 1);
		instpnt += 3;
	}
	txtpnt = strmov(txtpnt, optxt);
	*txtpnt++ = '\t';
	putreladdr(addr);
	return (RTN_DONE);
}



//***************************************
// Function: instCALLF -
// Returned: Nothing
//***************************************

int instCALLF(void)
{
	return (commonjmpf("CALLF"));
}


//***************************************
// Function: instJMPF -
// Returned: Nothing
//***************************************

int instJMPF(void)
{
	return (commonjmpf("JMPF"));
}


static int commonjmpf(
	char *optxt)
{
	VAL addr;

	txtpnt = strmov(txtpnt, optxt);
	if (addrsz32)
	{
		addr.val = *(long *)(instpnt + 1);
		addr.seg = *(ushort *)(instpnt + 5);
		instpnt += 7;
	}
	else
	{
		addr.val = *(ushort *)(instpnt + 1);
		addr.seg = *(ushort *)(instpnt + 3);
		instpnt += 5;
	}

	// Display the segment selector unconditionally here

	txtpnt += sprintf(txtpnt, "\t0x%04X%c", addr.seg, (rmaddr) ? ';' : ':');
	addr.seg = 0;
	txtpnt = fmtvalue(txtpnt, &addr);
	return (RTN_DONE);
}


//***************************************
// Function: instLOCK -
// Returned: Nothing
//***************************************

int instLOCK(void)
{
	txtpnt = strmov(txtpnt, "LOCK");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instHLT -
// Returned: Nothing
//***************************************

int instHLT(void)
{
	txtpnt = strmov(txtpnt, "HLT");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instCMC -
// Returned: Nothing
//***************************************

int instCMC(void)
{
	txtpnt = strmov(txtpnt, "CMC");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instCLC -
// Returned: Nothing
//***************************************

int instCLC(void)
{
	txtpnt = strmov(txtpnt, "CLC");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instSTC -
// Returned: Nothing
//***************************************

int instSTC(void)
{
	txtpnt = strmov(txtpnt, "STC");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instCLI -
// Returned: Nothing
//***************************************

int instCLI(void)
{
	txtpnt = strmov(txtpnt, "CLI");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instSTI -
// Returned: Nothing
//***************************************

int instSTI(void)
{
	txtpnt = strmov(txtpnt, "STI");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instCLD -
// Returned: Nothing
//***************************************

int instCLD(void)
{
	txtpnt = strmov(txtpnt, "CLD");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instSTD -
// Returned: Nothing
//***************************************

int instSTD(void)
{
	txtpnt = strmov(txtpnt, "STD");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: instgroup4 -
// Returned: Nothing
//***************************************

int instgroup4(void)
{
	int inx;

	if ((inx = (instpnt[1] >> 3) & 0x07) > 1)
		return (RTN_ERROR);
	putopcode(group5tbl[inx], FALSE);
	*txtpnt++ = '\t';
	instpnt += putgenaddr(0);	
	return (RTN_DONE);
}


//***************************************
// Function: instgroup5 -
// Returned: Nothing
//***************************************

int instgroup5(void)
{
	char *text;
	int   inx;

	inx = (instpnt[1] >> 3) & 0x07;
	if (group5tbl[inx] == NULL)
		return (RTN_ERROR);
	text = group5tbl[inx];
	txtpnt = strmov(txtpnt, text);
	if (text[0] != 'C')
		*txtpnt++ = (datasz32) ? 'L' : 'W';
	*txtpnt++ = '\t';
	instpnt += putgenaddr(0);	
	return (RTN_DONE);
}


//***************************************
// Function: esc0F -
// Returned: Nothing
//***************************************

int esc0F(void)
{
	ROW *rowpnt;
	int  rtn;

	instpnt++;
	do
	{
		rowpnt = xrowtbl + (instpnt[0] >> 4);
	} while ((rtn = (rowpnt->func)(rowpnt->data)) == RTN_MORE);
	return (rtn);
}


int xrowbad(
	IFUNC **func)
{
	func = func;

	txtpnt += sprintf(txtpnt, "<0F %02X>", *instpnt);
	return (RTN_ERROR);
}


int xinstMOVcc(
	IFUNC **func)
{
	func = func;


	return (RTN_DONE);
}


int xinstSETcc(
	IFUNC **func)
{
	func = func;


	return (RTN_DONE);
}


int xinstJcc(
	IFUNC **func)
{
	func = func;

	*txtpnt++ = 'J';
	txtpnt = strmov(txtpnt, condtbl[instpnt[0] & 0x0F]);
	*txtpnt++ = '\t';
	instpnt += 5;
	putreladdr(*(long *)(instpnt - 4));
	return (RTN_DONE);
}


//***************************************
// Function: xinstbad -
// Returned: Return code
//***************************************

int xinstbad(void)
{
	txtpnt += sprintf(txtpnt, "<0F %02X>", *instpnt);
	return (RTN_ERROR);
}


//***************************************
// Function: xinstLAR -
// Returned: Return code
//***************************************

int xinstLAR(void)
{
	instcommon("LAR", 0x01, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: xinstLSL -
// Returned: Return code
//***************************************

int xinstLSL(void)
{
	instcommon("LSL", 0x01, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: xinstCLTS -
// Returned: Return code
//***************************************

int xinstCLTS(void)
{
	txtpnt = strmov(txtpnt, "CLTS");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstINVD -
// Returned: Return code
//***************************************

int xinstINVD(void)
{
	txtpnt = strmov(txtpnt, "INVD");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstWBINVD -
// Returned: Return code
//***************************************

int xinstWBINVD(void)
{
	txtpnt = strmov(txtpnt, "WBINVD");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstUD2 -
// Returned: Return code
//***************************************

int xinstUD2(void)
{
	txtpnt = strmov(txtpnt, "UD2");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstgroup6 -
// Returned: Return code
//***************************************

int xinstgroup6(void)
{
	int inx;

	inx = (instpnt[1] >> 3) & 0x7;
	if (group6tbl[inx] == NULL)
		return (RTN_ERROR);
	txtpnt = strmov(txtpnt, group6tbl[inx]);
	*txtpnt++ = '\t';
	instpnt += putgenaddr(0x01);
	return (RTN_DONE);
}


//***************************************
// Function: xinstgroup7 -
// Returned: Return code
//***************************************

int xinstgroup7(void)
{
	int inx;

	inx = (instpnt[1] >> 3) & 0x7;
	if (group7tbl[inx] == NULL)
		return (RTN_ERROR);
	txtpnt = strmov(txtpnt, group7tbl[inx]);
	*txtpnt++ = '\t';
	instpnt += putgenaddr(0x01);
	return (RTN_DONE);
}


//***************************************
// Function: xinstgroup9 -
// Returned: Return code
//***************************************

int xinstgroup9(void)
{
	if (((instpnt[1] >> 3) & 0x7) != 1)
		return (RTN_ERROR);
	txtpnt = strmov(txtpnt, "CMPXCH8B\t");
	instpnt += putgenaddr(0x01);
	return (RTN_DONE);
}


//***************************************
// Function: xinstMOVcrdr -
// Returned: Return code
//***************************************

int xinstMOVcrdr(void)
{
	char *regtxt;
	int  opval;
	int  size;

	opval = instpnt[0];
	datasz32 = TRUE;
	regtxt = (opval & 0x01) ? "DR" : "CR";
	txtpnt += sprintf(txtpnt, "MOV%s\t", regtxt);

	if (opval & 0x02)
	{
		txtpnt += sprintf(txtpnt, "%s%d, ", regtxt, (instpnt[1] >> 3) & 0x07);
		size = putgenaddr(opval);
	}
	else
	{
		size = putgenaddr(opval);
		txtpnt += sprintf(txtpnt, "%, s%d", regtxt, (instpnt[1] >> 3) & 0x07);
	}
	instpnt += size;
	return (RTN_DONE);
}


//***************************************
// Function: xinstWRMSR -
// Returned: Return code
//***************************************

int xinstWRMSR(void)
{
	txtpnt = strmov(txtpnt, "WRMSR");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstRDTSC -
// Returned: Return code
//***************************************

int xinstRDTSC(void)
{
	txtpnt = strmov(txtpnt, "RDTSC");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstRDMSR -
// Returned: Return code
//***************************************

int xinstRDMSR(void)
{
	txtpnt = strmov(txtpnt, "RDMSR");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstRDPMC -
// Returned: Return code
//***************************************

int xinstRDPMC(void)
{
	txtpnt = strmov(txtpnt, "RDPMC");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstCPUID -
// Returned: Return code
//***************************************

int xinstCPUID(void)
{
	txtpnt = strmov(txtpnt, "CPUID");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstRSM -
// Returned: Return code
//***************************************

int xinstRSM(void)
{
	txtpnt = strmov(txtpnt, "RSM");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstSYSENTER -
// Returned: Return code
//***************************************

int xinstSYSENTER(void)
{
	txtpnt = strmov(txtpnt, "SYSENTER");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstSYSEXIT -
// Returned: Return code
//***************************************

int xinstSYSEXIT(void)
{
	txtpnt = strmov(txtpnt, "SYSEXIT");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstPPFSGS -
// Returned: Return code
//***************************************

int xinstPPFSGS(void)
{
	txtpnt = strmov(txtpnt, (instpnt[0] & 0x01) ? "POPW\t" : "PUSHW\t");
	txtpnt = strmov(txtpnt, (instpnt[0] & 0x08) ? "GS" : "FS");
	instpnt++;
	return (RTN_DONE);
}


//***************************************
// Function: xinstBTx -
// Returned: Return code
//***************************************

int xinstBTx(void)
{
	int size;

	putopcode(bttbl[(instpnt[1] >> 3) & 0x03], TRUE);
	*txtpnt++ = '\t';
	size = putgenaddr(0x01);
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	putdatareg(((instpnt[1] >> 3) & 0x07) + ((instpnt[0] & 0x01) ?
			IRG_AX : IRG_AL));
	instpnt += size;
	return (RTN_DONE);
}


//*******************************
// Function: xinstBTxi - Bit test
// Returned: Return code
//*******************************

int xinstBTxi(void)
{
	int inx;
	int size;

	if ((inx = ((instpnt[1] >> 3) & 0x07) - 4) < 0)
		return (RTN_ERROR);
	putopcode(bttbl[inx], TRUE);
	*txtpnt++ = '\t';
	size = putgenaddr(0x01);
	txtpnt = strmov(txtpnt, ", #");
	txtpnt = fmtnumeric(txtpnt, instpnt[size]);
	instpnt += (size + 1);
	return (RTN_DONE);
}


//***********************************
// Function: xinstSHxD - Double shift
// Returned: Return code
//***********************************

int xinstSHxD(void)
{
	int opval;

	opval = instpnt[0];
	instcommon((opval & 0x08) ? "SHLD" : "SHRD", 0x01, FALSE);
	if (opval & 0x01)
		txtpnt = strmov(txtpnt, ", CL");
	else
		putimmedvalue(FALSE, TRUE);
	return (RTN_DONE);
}


//***************************************
// Function: xinstCMPXCHG -
// Returned: Return code
//***************************************

int xinstCMPXCHG(void)
{
	instcommon("CMPXCHG", 0x01, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: xinstBSF -
// Returned: Return code
//***************************************

int xinstBSF(void)
{
	instcommon("BSF", 0x03, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: xinstBSR -
// Returned: Return code
//***************************************

int xinstBSR(void)
{
	instcommon("BSR", 0x03, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: xinstMOVX -
// Returned: Return code
//***************************************

int xinstMOVX(void)
{
	putopcode((instpnt[0] & 0x08) ? ((instpnt[0] & 0x01) ? "MOVXW" : "MOVXB") :
			((instpnt[0] & 0x01) ? "MOVZW" : "MOVZB"), TRUE);
	*txtpnt++ = '\t';
	putdatareg((instpnt[1] >> 3) & 0x07);
	*txtpnt++ = ',';
	*txtpnt++ = ' ';
	datasz32 = FALSE;
	instpnt += putgenaddr(instpnt[0] & 0x01);
	return (RTN_DONE);
}


//***************************************
// Function: xinstXADD -
// Returned: Return code
//***************************************

int xinstXADD(void)
{
	instcommon("XADD", 0x01, FALSE);
	return (RTN_DONE);
}


//***************************************
// Function: xinstBSWAP -
// Returned: Return code
//***************************************

int xinstBSWAP(void)
{
	putopcode("BSWAP", TRUE);
	*txtpnt++ = '\t';
	putdatareg(instpnt[0] & 0x07);
	instpnt++;
	return (RTN_DONE);
}


//=================================================================
// Local functions used to construct various part of an instruction
//=================================================================


//************************************************************
// Function: putopcodereg - Insert opcode followed by register
// Returned: Nothing
//************************************************************

static void putopcodereg(
	char *optxt,
	int   regnum)
{
	putopcode(optxt, regnum < 8);
	*txtpnt++ = '\t';
	putdatareg(regnum);
}


//********************************************
// Function: putdatareg - Insert data register
// Returned: Nothing
//********************************************

static void putdatareg(
	int   regnum)
{
	if (regnum < 8 && datasz32)
		*txtpnt++ = 'E';
	txtpnt = strmov(txtpnt, reg32tbl[regnum]);
}


//***********************************************
// Function: putaddrreg - Insert address register
// Returned: Nothing
//***********************************************

static void putaddrreg(
	int   regnum)
{
	if (regnum < 8 && addrsz32)
		*txtpnt++ = 'E';
	txtpnt = strmov(txtpnt, reg32tbl[regnum]);
}


//************************************
// Function: putopcode - Insert opcode
// Returned: Nothing
//************************************

static void putopcode(
	char *text,
	int   notbyte)
{
	txtpnt = strmov(txtpnt, text);
	*txtpnt++ = (!notbyte) ? 'B' : (datasz32) ? 'L' : 'W';
}



static void putsimaddr(
	long addr)
{
	txtpnt = fmtnumeric(txtpnt, addr);
}


static int putaddroverride(void)
{
	static long *segvaltbl[] =
	{	&regtbl.es.value,
		&regtbl.cs.value,
		&regtbl.ss.value,
		&regtbl.ds.value,
		&regtbl.fs.value,
		&regtbl.gs.value
	};

	if (segpfx != 0)
	{
		txtpnt = strmov(txtpnt, reg32tbl[segpfx]);
		*txtpnt++ = ':';
		return (*segvaltbl[segpfx - 16]);
	}
	return (regtbl.ds.value);
}


//***********************************************
// Function: putreladdr - Insert relative address
// Returned: Nothing
//***********************************************

static void putreladdr(
	long  value)
{
	value += (rspbufr.rdmem.addr + (instpnt - rspbufr.rdmem.data));
	if (!addrsz32)
		value &= 0xFFFF;
	txtpnt = fmtnumeric(txtpnt, value);
}

static void putregindir(
	int reg)
{
	if (segpfx != 0)
	{
		txtpnt = strmov(txtpnt, reg32tbl[segpfx]);
		*txtpnt++ = ':';
	}
	*txtpnt++ = '[';
	putaddrreg(reg);
	*txtpnt++ = ']';
	*txtpnt = 0;
}



//*************************************************
// Function: putimmedvalue - Insert immediate value
// Returned: Nothing
//*************************************************

static void putimmedvalue(
	int   notbyte,
	int   expand)
{
	long value;

	*txtpnt++ = '#';
	if (!notbyte)
	{
		value = (expand) ? *(si8 *)instpnt : *(ui8 *)instpnt;
		instpnt++;
	}
	else if (datasz32)
	{
		value = *(ulong *)instpnt;
		instpnt += 4;
	}
	else
	{
		value = *(ushort *)instpnt;
		instpnt += 2;
	}
	txtpnt = fmtnumeric(txtpnt, value);
	if (!notbyte && expand)
		txtpnt = strmov(txtpnt, ".B");
}
