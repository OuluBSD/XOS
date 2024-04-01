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

// This module implements a remote program which communicates with exec-mode
//   GECKO using a USB debug cable connected to the USB debug port on the
//   target machine.


uchar debuginpbuf[16];


int    rsptype = -1;
int    rspseq;
int    rsplen;
int    rspleft;
uchar *rsppnt;
RSPBFR rspbufr;
REQBFR reqbufr;

int    stopper;
int    hldchar;
int    linenum;

long   curval;
long   cursel;
SYM   *curmodule;

VAL    listaddr;				// Dump list: address
long   listoffset;				// Dump list: offset in item of next pointer
long   listcount;				// Dump list: item count
long   listsize;				// Dump list: item size value

int  (*contfunc)(void);

char  *txtpnt;

VAL    curaddr;
int    cursize;
int    curszx;
VAL    lstaddr;
REGNTBL *curreg;

TARGET target;

MODE curmode = {OM_SYMBOLIC, 16, 16};
MODE prmmode = {OM_SYMBOLIC, 16, 16};

uchar  cmdline[200];
uchar *cmdpnt = cmdline;
uchar  cmdstate = CMD_CMD;
int    curcmdinx = CCX_EB;
uchar  entrytype;				// Current entry type
uchar  doingdump;
uchar  hvgdrsp;
uchar  suspaddr;
uchar  showgmn;
uchar  debugout;
uchar  havenoop = FALSE;
uchar  rmaddr;					// TRUE if last value displayed was a real
								//   mode address

FLGS flgtbl[] =
{	{{"vm", "VM"}            , 17, 0x01},
	{{"rf", "RF"}            , 16, 0x01},
	{{"nt", "NT"}            , 14, 0x01},
	{{"P0", "P1", "P2", "P3"}, 12, 0x03},
	{{"ov", "OV"}            , 11, 0x01},
	{{"UP", "DN"}            , 10, 0x01},
	{{"ie", "IE"}            ,  9, 0x01},
	{{"tp", "TP"}            ,  8, 0x01},
	{{"sn", "SN"}            ,  7, 0x01},
	{{"zr", "ZR"}            ,  6, 0x01},
	{{"ac", "AC"}            ,  4, 0x01},
	{{"pe", "PE"}            ,  2, 0x01},
	{{"cy", "CY"}            ,  0, 0x01}
};
int flgtblsz = sizeof(flgtbl)/sizeof(FLGS);

// Define the procarg table

/*
optword:.LONG	sques , 0                   , 0, dohelp  , 0
	.LONG	sh    , 0                   , 0, dohelp  , 0
	.LONG	shel  , 0                   , 0, dohelp  , 0
	.LONG	shelp , 0                   , 0, dohelp  , 0
	.LONG	slin  , ASF$VALREQ|ASF$NVAL , 0, fnclines, 0
	.LONG	slines, ASF$VALREQ|ASF$NVAL , 0, fnclines, 0
	.LONG	spor  , ASF$VALREQ|ASF$LSVAL, 0, fncport , 0
	.LONG	sport , ASF$VALREQ|ASF$LSVAL, 0, fncport , 0

;Option name strings for procarg

sques:	.ASCIZ  "?"
sh:	.ASCIZ  "H"
shel:	.ASCIZ  "HEL"
shelp:	.ASCIZ  "HELP"
slin:	.ASCIZ  "LIN"
slines:	.ASCIZ  "LINES"
spor:	.ASCIZ  "POR"
sport:	.ASCIZ  "PORT"
*/

REGS regtbl =
{	{0, (ushort *)(scrnbufr+182), "EAX"},	// REG_EAX = 1
	{0, (ushort *)(scrnbufr+218), "EBX"},	// REG_EBX = 2
	{0, (ushort *)(scrnbufr+254), "ECX"},	// REG_ECX = 3
	{0, (ushort *)(scrnbufr+290), "EDX"},	// REG_EDX = 4
	{0, (ushort *)(scrnbufr+342), "ESP"},	// REG_ESP = 5
	{0, (ushort *)(scrnbufr+378), "EBP"},	// REG_EBP = 6
	{0, (ushort *)(scrnbufr+414), "EDI"},	// REG_EDI = 7
	{0, (ushort *)(scrnbufr+450), "ESI"},	// REG_ESI = 8
	{0, (ushort *)(scrnbufr+500), "CS"},	// REG_CS  = 9
	{0, (ushort *)(scrnbufr+524), "SS"},	// REG_SS  = 10
	{0, (ushort *)(scrnbufr+548), "DS"},	// REG_DS  = 11
	{0, (ushort *)(scrnbufr+572), "ES"},	// REG_ES  = 12
	{0, (ushort *)(scrnbufr+596), "FS"},	// REG_FS  = 13
	{0, (ushort *)(scrnbufr+620), "GS"},	// REG_GS  = 14
	{0, (ushort *)(scrnbufr+662), "EIP"},	// REG_FS  = 15
	{0, (ushort *)(scrnbufr+698), "EFR"}	// REG_EFR = 16
};


#define or(reg) offsetof(REGS,reg)


REGNTBL regntbl[] =				// Register name table
{	{"AL"  , REG_EAX, 1, or(eax.bytel)},
	{"AH"  , REG_EAX, 1, or(eax.byteh)},
	{"BL"  , REG_EBX, 1, or(ebx.bytel)},
	{"BH"  , REG_EBX, 1, or(ebx.byteh)},
	{"CL"  , REG_ECX, 1, or(ecx.bytel)},
	{"CH"  , REG_ECX, 1, or(ecx.byteh)},
	{"DL"  , REG_EDX, 1, or(edx.bytel)},
	{"DH"  , REG_EDX, 1, or(edx.byteh)},
	{"AX"  , REG_EAX, 2, or(eax.word)},
	{"BX"  , REG_EBX, 2, or(ebx.word)},
	{"CX"  , REG_ECX, 2, or(ecx.word)},
	{"DX"  , REG_EDX, 2, or(edx.word)},
	{"SI"  , REG_ESI, 2, or(esi.word)},
	{"DI"  , REG_EDI, 2, or(edi.word)},
	{"BP"  , REG_EBP, 2, or(ebp.word)},
	{"SP"  , REG_ESP, 2, or(esp.word)},
	{"EAX" , REG_EAX, 4, or(eax.value)},
	{"EBX" , REG_EBX, 4, or(ebx.value)},
	{"ECX" , REG_ECX, 4, or(ecx.value)},
	{"EDX" , REG_EDX, 4, or(edx.value)},
	{"ESI" , REG_ESI, 4, or(esi.value)},
	{"EDI" , REG_EDI, 4, or(edi.value)},
	{"EBP" , REG_EBP, 4, or(ebp.value)},
	{"ESP" , REG_ESP, 4, or(esp.value)},
	{"CS"  , REG_CS,  2, or(cs.word)},
	{"DS"  , REG_DS,  2, or(ds.word)},
	{"SS"  , REG_SS,  2, or(ss.word)},
	{"ES"  , REG_ES,  2, or(es.word)},
	{"FS"  , REG_FS,  2, or(fs.word)},
	{"GS"  , REG_GS,  2, or(gs.word)},
	{"IP"  , REG_EIP, 2, or(eip.word)},
	{"EIP" , REG_EIP, 4, or(eip.value)},
	{"FR"  , REG_EFR, 2, or(efr.value)},
	{"EFR" , REG_EFR, 4, or(efr.value)},
	{"LDTR", REG_LDTR,2, -1},
	{"MSW" , REG_MSW, 2, -1},
	{"TR"  , REG_TR,  2, -1},
	{"GDTR", REG_GDTR,6, -1},
	{"IDTR", REG_IDTR,6, -1},
	{"CR0" , REG_CR0, 4, -1},
	{"CR2" , REG_CR2, 4, -1},
	{"CR3" , REG_CR3, 4, -1},
	{"CR4" , REG_CR4, 4, -1},
	{"DR0" , REG_DR0, 4, -1},
	{"DR1" , REG_DR1, 4, -1},
	{"DR2" , REG_DR2, 4, -1},
	{"DR3" , REG_DR3, 4, -1},
	{"DR6" , REG_DR6, 4, -1},
	{"DR7" , REG_DR7, 4, -1},
	{"TR6" , REG_TR6, 4, -1},
	{"TR7" , REG_TR7, 4, -1},
	{"ST"  , REG_ST0, 4, -1},
	{"ST0" , REG_ST0, 4, -1},
	{"ST1" , REG_ST1, 4, -1},
	{"ST2" , REG_ST2, 4, -1},
	{"ST3" , REG_ST3, 4, -1},
	{"ST4" , REG_ST4, 4, -1},
	{"ST5" , REG_ST5, 4, -1},
	{"ST6" , REG_ST6, 4, -1},
	{"ST7" , REG_ST7, 4, -1}
};
int regntblsz = sizeof(regntbl)/sizeof(REGNTBL);


void chrlf(void);
void setcur(void);
void coninpsig(void);
void cntrlc(void);
void borderline(char *text, int line);

void putstrns(ushort *pos, uchar *str);
void putchrns(ushort *pos, int chr);

void puthex(char *text, long value, int size);


void main(
    int   argc,
    char *argv[])
{
	svcSchSetLevel(8);
	setvector(VECT_USBINP, 0x04, usbinpsig);
	setvector(VECT_CONINP, 0x04, coninpsig);
	consoleinit();						// Initialize the display
	syminit((argc >= 2) ? argv[1] : "XOSSYS:");
	addsymtbl(NULL, 0, 0, 0);			// Load the kernel symbols
	svcSchSetLevel(0);					// Enable all signals
	usbinit();							// Initialize the debug device
	begincmd();

	// Here with all set up complete
	
	while (TRUE)
		svcSchSuspend(NULL, -1, -1);
}


void setupfail(
	long  code,
	char *msg)
{
	char text[100];

	fprintf(stderr, "\x1B[H\x1B[J\n? GECKOR: %s\n", msg);
	if (code < 0)
	{
		svcSysErrMsg(code, 0x03, text);
		fprintf(stderr, "          %s\n", text);
	}
	fputs("          Cannot continue\n", stderr);
	exit (1);
}
