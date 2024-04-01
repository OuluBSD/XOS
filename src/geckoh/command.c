//++++
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


VAL  wildval;
SYM *wildsym;
SYM *wildmod;
int  wildbits;
char wildname[SYMMAX+20];
int  wildhead;
int  wildcnt;

long symtblnum = -1;

VAL  argval;

uchar sztbl[] = {1, 2, 4, 8, 1, 1, 1, 1};
uchar needarg = TRUE;

static int dispvalue(void);
static int updstate(void);
static int openreg(void);
static int openbyte(void);
static int openword(void);
static int openlong(void);
static int openprev(void);
static int opennext(void);
static int chngreg(void);
static int chngbyte(void);
static int chngword(void);
static int chnglong(void);
static int chngprev(void);
static int chngnext(void);
static int chngpci(void);
static int clrscreen(void);
static int examinst(void);
static int exambyte(void);
static int examword(void);
static int examlstl(void);
static int examlong(void);
static int examprev(void);
static int examsame(void);
static int examnext(void);
static int exampci(void);
static int examcurinst(void);
static int evalexp(void);
static int outputb(void);
static int outputw(void);
static int outputl(void);
static int inputb(void);
static int inputw(void);
static int inputl(void);
static int dumpbyte(void);
static int dumpword(void);
static int dumplong(void);
static int dumpinst(void);
static int dumpregs(void);
static int dumplstb(void);
static int dumplstw(void);
static int dumplstl(void);
static int getvaddr(void);
static int getpaddr(void);
static int getpciaddr(long *val);
static int startp(void);
static int proceed(void);
static int trace(void);
static int reboot(void);
static int srchval(void);
static int srchname(void);
static int srchfirst(void);
static int dosetmodule(void);
static int symlist(void);
static int symadd(void);
static int symremove(void);
static int setmodes(void);
static int goexit(void);
static int help(void);
static int setbkp(void);
static int rmvbkp(void);
static int clrbkp(void);
static int lstbkp(void);
static int setwbwtp(void);
static int setwwwtp(void);
static int setwlwtp(void);
static int setabwtp(void);
static int setawwtp(void);
static int setalwtp(void);
static int setiwtp(void);
static int lstwtp(void);
static int rmvwtp(void);
static int clrwtp(void);
static int togglegmn(void);
static int toggledbo(void);

static int dodump(int size);
static int dodumplist(int size);
static int doexam(int size);
static int getioin(int size);
static int putioout(int size);
static int wpcommon(int func, int size);


int (*curcmdtbl[])(void) =
{	exambyte,		// CCX_EB  = 0  - Examine byte
	examword,		// CCX_EW  = 1  - Examine word
	examlong,		// CCX_EL  = 2  - Examine long
	examinst,		// CCX_EI  = 3  - Examine instruction
	openbyte,		// CCX_OB  = 4  - Open byte
	openword,		// CCX_OW  = 5  - Open word
	openlong,		// CCX_OL  = 6  - Open long
};


typedef _Packed struct
{	uchar chr;
	int (*func)(void);
} SPCTBL;

SPCTBL spctbl[] =
{	{0x82, updstate},		// F2  - Update target state
	{0x84, examcurinst},	// F4  - Examine current instruction
	{0x85, examprev},		// F5  - Examine previous
	{0x86, examsame},		// F6  - Examine same
	{0x87, examnext},		// F6  - Examine next
	{0x89, trace},			// F9  - Trace instruction
	{0x8A, startp}			// F10 - Start execution
};


static int wildbegin(int bits);
static int bpcommon(int func);
static int strtexe(int func);
static int getmode(int chr, MODE *mode);
static int getradix(int chr, int *radix);


typedef struct
{	char *cmd;
	int (*func)(void);
} CMDTBL;

CMDTBL cmdtbl[] =
{	{"V"   , dispvalue},	// Display numeric unsigned value
	{"OR"  , openreg},		// Open register
	{"OB"  , openbyte},		// Open byte
	{"OW"  , openword},		// Open word
	{"OL"  , openlong},		// Open long
	{"OP"  , openprev},		// Open previous
	{"ON"  , opennext},		// Open next
	{"CR"  , chngreg},		// Change register
	{"CB"  , chngbyte},		// Change byte
	{"CW"  , chngword},		// Change word
	{"CL"  , chnglong},		// Change long
	{"CP"  , chngprev},		// Change previous
	{"CN"  , chngnext},		// Change next
	{"CPCI", chngpci},		// Change PCI configuration register
	{"CLS" , clrscreen},	// Clear screen
	{"EI"  , examinst},		// Examine instruction
	{"EB"  , exambyte},		// Examine byte
	{"EW"  , examword},		// Examine word
	{"EL"  , examlong},		// Examine long
	{"EP"  , examprev},		// Examine previous
	{"EN"  , examnext},		// Examine next
	{"ELL" , examlstl},		// Examine list long
	{"EPCI", exampci},		// Examine PCI configuration register
	{"E"   , evalexp},		// Evaluate expression
	{"OUTB", outputb},		// Output byte
	{"OUTW", outputw},		// Output word
	{"OUTL", outputl},		// Output long
	{"INB" , inputb},		// Input byte
	{"INW" , inputw},		// Input word
	{"INL" , inputl},		// Input long
	{"DB"  , dumpbyte},		// Dump as bytes
	{"DW"  , dumpword},		// Dump as words
	{"DL"  , dumplong},		// Dump as longs
	{"DI"  , dumpinst},		// Dump as instructions
	{"DR"  , dumpregs},		// Dump registers
	{"DLB" , dumplstb},		// Dump list as bytes
	{"DLW" , dumplstw},		// Dump list as words
	{"DLL" , dumplstl},		// Dump list as longs
	{"VA"  , getvaddr},		// Get virtual address(es)
	{"PA"  , getpaddr},		// Get physical address
	{"G"   , startp},		// Start execution
	{"P"   , proceed},		// Proceed from breakpoint
	{"T"   , trace},		// Trace instruction
	{"BOOT", reboot},		// Reboot the kernel
	{"SV"  , srchval},		// Search symbol table by value
	{"SN"  , srchname},		// Search symbol table by name
	{"SF"  , srchfirst},	// Search symbol table by name for first match
	{"SM"  , dosetmodule},	// Select module
	{"SL"  , symlist},		// List symbol tables
	{"SA"  , symadd},		// Add symbol table
	{"SR"  , symremove},	// Remove symbol table
	{"MD"  , setmodes},		// Set mode and radix
	{"QUIT", goexit},		// Exit GECKO
	{"HELP", help},			// Display help info
	{"BS"  , setbkp},		// Set breakpoint
	{"BR"  , rmvbkp},		// Remove breakpoint
	{"BC"  , clrbkp},		// Remove all breakpoints
	{"BL"  , lstbkp},		// List breakpoints
	{"US"  , updstate},		// Update target state
	{"WSWB", setwbwtp},		// Set data write watchpoint on byte
	{"WSWW", setwwwtp},		// Set data write watchpoint on word
	{"WSWL", setwlwtp},		// Set data write watchpoint on long
	{"WSAB", setabwtp},		// Set data access watchpoint on byte
	{"WSAW", setawwtp},		// Set data access watchpoint on word
	{"WSAL", setalwtp},		// Set data access watchpoint on long
	{"WSI" , setiwtp},		// Set instruction watchpoint
	{"WL"  , lstwtp},		// List watchpoints
	{"WR"  , rmvwtp},		// Remove watchpoint
	{"WC"  , clrwtp},		// Clear watchpoints
	{"TGMN", togglegmn},	// Toggle display of global's module name
	{"TDBO", toggledbo}		// Toggle debug output
};



void begincmd(void)
{
	if (curchar != 0)
		putstr("\r\n");
	putstr("%%\x05");
	cmdpnt = cmdline;
	cmdstate = CMD_CMD;
	echochrs();

/*
	if (cmdstate == CMD_CMD)
		cmdpnt = cmdline;
	else
	{
		cmdstate = CMD_CMD;
		*cmdpnt = 0;
		putstr(cmdline);
	}
*/}


//*****************************************************
// Function: docommand - Process the command in cmdline
// Returned: Nothing
//*****************************************************

void docommand(void)
{
	CMDTBL *tpnt;
	SPCTBL *spnt;
	char   *apnt;
	int     cnt;
	char    atom[12];
	uchar   chr;

	cmdpnt = cmdline;
	apnt = atom;
	cnt = 11;
	while (((chr = *cmdpnt) & 0x80) == 0 && chr != 0 && isspace(chr))
		cmdpnt++;
	while (((chr = *cmdpnt) & 0x80) == 0 && chr != 0 && isalpha(chr))
	{
		if (--cnt >= 0)
			*apnt++ = chr;
		cmdpnt++;
	}
	stopper = chr;
	while ((stopper & 0x80) == 0 && stopper != 0 && isspace(stopper))
		stopper = *++cmdpnt;
	if (stopper & 0x80)
		stopper = 0;
	curmode = prmmode;
	if (chr > 0x80)
	{
		spnt = spctbl;
		cnt = sizeof(spctbl)/sizeof(SPCTBL);
		do
		{
			if (chr == spnt->chr)
			{
				cmdpnt = cmdline;
				cmdline[0] = 0;
				cmdstate = (spnt->func)();
				cmdpnt = cmdline;
				cmdline[0] = 0;
				if (cmdstate == CMD_CMD)
					begincmd();
				return;
			}
			spnt++;
		} while (--cnt > 0);
		begincmd();
		return;
	}
	*apnt = 0;
	if (atom[0] != 0)
	{
		tpnt = cmdtbl;
		cnt = sizeof(cmdtbl)/sizeof(CMDTBL);
		do
		{
			if (stricmp(atom, tpnt->cmd) == 0)
			{
				while ((chr = nxtnbc()) == '/')
				{
					if (!getmode(toupper(nxtchar()), &curmode))
					{
						begincmd();
						return;
					}
				}
				hldchar = chr;
				cmdstate = (tpnt->func)();
				cmdpnt = cmdline;
				if (cmdstate == CMD_CMD)
					begincmd();
				return;
			}
			tpnt++;
		} while (--cnt > 0);
		putstr("\n\r\1? Unknown command\2\n\r");
	}
	begincmd();
}


// Display value

static int dispvalue(void)
{
	VAL  value;
	char valtxt[256];
	char text[260];

	if (expression(&value))
	{
		curmode.omode = OM_NUMERIC;
		fmtvalue(valtxt, &value);
		sprintf(text, " = %s\r\n", valtxt);
		putstr(text);
	}
	return (CMD_CMD);
}


// Update target state

static int updstate(void)
{
	if (sendrequest(DBREQ_STATE, 0) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Open register

static int openreg(void)
{
	char *pnt;
	VAL   value;
	int   cnt;
	char  name[16];
	char  text[300];

	if (getname(FALSE, name, 15) < 0)
		return (CMD_CMD);
	if (stopper != 0)
	{
		putstr("\n\r\1? Syntax error\2\n\r");
		return (CMD_CMD);
	}
	value.seg = 0;
	curreg = regntbl;
	cnt = regntblsz;
	do
	{
		if (stricmp(curreg->name, name) == 0)
		{
			if (curreg->size == 4)
				value.val = *(ulong *)(((uchar *)&regtbl) + curreg->offset);
			else if (curreg->size == 2)
				value.val = *(ushort *)(((uchar *)&regtbl) +
						curreg->offset);
			else
				value.val = *(uchar *)(((uchar *)&regtbl) + curreg->offset);
			if (curreg->offset < 0)
			{
				putstr("\n\r\1? Access to remote registers is not implimented "
						"yet\2\n\r");
				return (CMD_CMD);
			}
			text[0] = '\r';
			pnt = strmov(text + 1, curreg->name);
			pnt = strmov(pnt, " = ");
			pnt = fmtvalue(pnt, &value);
			strmov(pnt, " %%");
			putstr(text);
			return (CMD_ROPEN);
		}
		curreg++;
	} while (--cnt > 0);
	putstr("\n\r\1? Not a register name\2\n\r");
	return (CMD_CMD);
}


// Open byte

static int openbyte(void)
{
	int rtn;

	if ((rtn = doexam(DMEM_OPENDATA | DMEM_BYTE)) == CMD_WWTO)
		curcmdinx = CCX_EB;
	return (rtn);
}


// Open word

static int openword(void)
{
	int rtn;

	if ((rtn = doexam(DMEM_OPENDATA | DMEM_WORD)) == CMD_WWTO)
		curcmdinx = CCX_EW;
	return (rtn);
}


// Open long

static int openlong(void)
{
	int rtn;

	if ((rtn = doexam(DMEM_OPENDATA | DMEM_LONG)) == CMD_WWTO)
		curcmdinx = CCX_EL;
	return (rtn);
}


// Open previous

static int openprev(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}


// Open next

static int opennext(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}


// Change register

static int chngreg(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}


// Change byte

static int chngbyte(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}


// Change word

static int chngword(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}


// Change long

static int chnglong(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}


// Change previous

static int chngprev(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}


// Change next

static int chngnext(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}

// Change PCI configuration register

static int chngpci(void)
{
	long addr;

	if (!getpciaddr(&addr))
		return (CMD_CMD);
	reqbufr.wrtpci.addr = addr;
	if (!lastexpression(&argval))
		return (CMD_CMD);
	reqbufr.wrtpci.value = argval.val;
	if (sendrequest(DBREQ_WRTPCI, sizeof(WRTPCIRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}

// Examine instruction

static int examinst(void)
{
	if (needarg && !lastexpression(&argval))
		return (CMD_CMD);
	reqbufr.rdmem.count = 0;
	reqbufr.rdmem.size = DMEM_DISPINST | DMEM_INST32;
	reqbufr.rdmem.addr = argval.val;
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}

// Examine current instruction

static int examcurinst(void)
{
	int rtn;

	needarg = FALSE;
	argval.val = regtbl.eip.value;
	rtn = examinst();
	needarg = TRUE;
	if (rtn == CMD_WWTO)
		curcmdinx = CCX_EI;
	return (rtn);
}


// Examine byte

static int exambyte(void)
{
	int rtn;

	if ((rtn = doexam(DMEM_DISPDATA | DMEM_BYTE)) == CMD_WWTO)
		curcmdinx = CCX_EB;
	return (rtn);
}


// Examine word

static int examword(void)
{
	int rtn;

	if ((rtn = doexam(DMEM_DISPDATA | DMEM_WORD)) == CMD_WWTO)
		curcmdinx = CCX_EW;
	return (rtn);
}


// Examine long

static int examlong(void)
{
	int rtn;

	if ((rtn = doexam(DMEM_DISPDATA | DMEM_LONG)) == CMD_WWTO)
		curcmdinx = CCX_EL;
	return (rtn);
}


//********************************************************************
// Function: doexam - Common function for commands that examine memory
// Returned: CMD_CMD if should go to command mode
//				immediately or timeout value
//********************************************************************

static int doexam(
	int size)
{
	if (needarg && !lastexpression(&argval))
		return (CMD_CMD);
	reqbufr.rdmem.count = 1;
	reqbufr.rdmem.size = size;
	reqbufr.rdmem.addr = argval.val;
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}

// Examine previous

static int examprev(void)
{
	int rtn;

	if (curcmdinx != CCX_EI)
	{
		needarg = FALSE;
		argval = curaddr;
		argval.val -= cursize;
		rtn = (curcmdtbl[curcmdinx])();
		needarg = TRUE;
		return (rtn);
	}
	reqbufr.rdmem.count = 80;
	reqbufr.rdmem.size = DMEM_DISPPREV | DMEM_BYTE;
	reqbufr.rdmem.addr = curaddr.val - 80;
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Examine next

static int examnext(void)
{
	int rtn;

	needarg = FALSE;
	argval = curaddr;
	argval.val += cursize;
	rtn = (curcmdtbl[curcmdinx])();
	needarg = TRUE;
	return (rtn);
}


// Examine same

static int examsame(void)
{
	int rtn;

	needarg = FALSE;
	argval = curaddr;
	rtn = (curcmdtbl[curcmdinx])();
	needarg = TRUE;
	return (rtn);
}

// Examine PCI configuration register

static int exampci(void)
{
	long addr;

	if (!getpciaddr(&addr))
		return (CMD_CMD);
	if (stopper != 0)
	{
		putstr("\r\n\1? Too many arguments\2\r\n");
		return (CMD_CMD);
	}
	reqbufr.rdpci.addr = addr;
	if (sendrequest(DBREQ_RDPCI, sizeof(RDPCIRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}

// Examine list long - First argument is address of first list item. Second
//   argument is offset in the item of the next pointer.

static int examlstl(void)
{
	if (!expression(&listaddr))
		return (CMD_CMD);
	if (!lastnumeric(&listoffset))
		return (CMD_CMD);
	reqbufr.rdmem.count = 1;
	reqbufr.rdmem.size = DMEM_DISPLIST | DMEM_LONG;
	reqbufr.rdmem.addr = listaddr.val + listoffset;
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (CMD_CMD);
	putstr("\n");
	return (CMD_WWTO);
}

// Evaluate expression

static int evalexp(void)
{
	VAL  value;
	char valtxt[256];
	char text[260];

	if (expression(&value))
	{
		fmtvalue(valtxt, &value);
		sprintf(text, " = %s\r\n", valtxt);
		putstr(text);
	}
	return (CMD_CMD);
}


// Output byte

static int outputb(void)
{
	return (putioout(DMEM_BYTE));
}


// Output word

static int outputw(void)
{
	return (putioout(DMEM_WORD));
}


// Output long

static int outputl(void)
{
	return (putioout(DMEM_LONG));
}


static int putioout(
	int size)
{
	VAL ioval;

	if (!expression(&argval) || !lastexpression(&ioval))
		return (CMD_CMD);
	reqbufr.wrtio.size = size;
	reqbufr.wrtio.port = argval.val;
	reqbufr.wrtio.value = ioval.val;
	if (sendrequest(DBREQ_WRTIO, sizeof(WRTIORQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Input byte

static int inputb(void)
{
	return (getioin(DMEM_BYTE));
}


// Input word

static int inputw(void)
{
	return (getioin(DMEM_WORD));
}


// Input long

static int inputl(void)
{
	return (getioin(DMEM_LONG));
}


static int getioin(
	int size)
{
	if (!lastexpression(&argval))
		return (CMD_CMD);
	reqbufr.rdio.size = size;
	reqbufr.rdio.port = argval.val;
	if (sendrequest(DBREQ_RDIO, sizeof(RDIORQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Dump as bytes

static int dumpbyte(void)
{
	return (dodump(DMEM_DUMPDATA | DMEM_BYTE));
}


// Dump as words

static int dumpword(void)
{
	return (dodump(DMEM_DUMPDATA | DMEM_WORD));
}


// Dump as longs

static int dumplong(void)
{
	return (dodump(DMEM_DUMPDATA | DMEM_LONG));
}


// Dump as instructions

static int dumpinst(void)
{
	return (dodump(DMEM_DUMPINST | DMEM_INST32));
}


//*****************************************************************
// Function: dodump - Common function for commands that dump memory
// Returned: CMD_CMD if should go to command mode
//				immediately or timeout value
//*****************************************************************

static int dodump(
	int size)
{
	long lines;
	int  count;
	int  sz;

	sz = sztbl[size & 0x07];
	if (stopper == 0)
	{
		argval = lstaddr;
		lines = 16;
	}
	else
	{
		if (!expression(&argval))
			return (TRUE);
		if (stopper != 0)
		{
			if (!numeric(&lines))
				return (CMD_CMD);
			if (stopper != 0)
			{
				putstr("\r\n\1? Invalid value\2\r\n");
				return (CMD_CMD);
			}
		}
		else
			lines = 16;
	}
	count = lines * (16/sz);
	if ((reqbufr.rdmem.count = count) > (RSPBFRSZ - 8))
	{
		putstr("\r\n\1? Too much data requested\2\r\n");
		return (CMD_CMD);
	}
	reqbufr.rdmem.size = size;
	reqbufr.rdmem.addr = argval.val;
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Dump registers

static int dumpregs(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}


// Dump list as bytes

static int dumplstb(void)
{
	return (dodumplist(DMEM_DUMPLST1 | DMEM_BYTE));
}


// Dump list as words

static int dumplstw(void)
{
	return (dodumplist(DMEM_DUMPLST1 | DMEM_WORD));
}


// Dump list as longs

static int dumplstl(void)
{
	return (dodumplist(DMEM_DUMPLST1 | DMEM_LONG));
}


//*****************************************************************
// Function: dodumplist - Common routine for the dump list commands
// Returned: New command state
//*****************************************************************

// Format of the dump commands:
//   dlx address offset numlines

static int dodumplist(
	int size)
{
	long lines;
	int  sz;

	sz = sztbl[size & 0x07];
	if (stopper == 0)
	{
		argval = lstaddr;
		lines = 16;
	}
	else
	{
		if (!expression(&listaddr))
			return (CMD_CMD);
		if (stopper != 0)
		{
			if (!numeric(&listoffset))
				return (CMD_CMD);
			if (stopper != 0)
			{
				if (!numeric(&lines))
					return (CMD_CMD);
				if (stopper != 0)
				{
					putstr("\r\n\1? Invalid value\2\r\n");
					return (CMD_CMD);
				}
			}
		}
		else
			lines = 4;
	}
	listcount = lines * (16/sz);
	if ((reqbufr.rdmem.count = listcount) > (RSPBFRSZ - 8))
	{
		putstr("\r\n\1? Too much data requested\2\r\n");
		return (CMD_CMD);
	}
	reqbufr.rdmem.size = listsize = size;
	reqbufr.rdmem.addr = listaddr.val;
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Get corresponding virtual address(es)

static int getvaddr(void)
{
	if (!lastexpression(&argval))
		return (CMD_CMD);
	reqbufr.getva.paddr = argval.val;
	if (sendrequest(DBREQ_GETVA, sizeof(GETVARQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Get corresponding physical address

static int getpaddr(void)
{
	if (!lastexpression(&argval))
		return (CMD_CMD);
	reqbufr.getpa.vaddr = argval.val;
	if (sendrequest(DBREQ_GETPA, sizeof(GETPARQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Proceed from breakpoint

static int proceed(void)
{
	VAL val;
	int size;

	val.val = 0;
	if (!lastexpression(&val))
		return (CMD_CMD);
	if (val.val == 0)
		size = 1;
	reqbufr.xctrl.func = XCTRL_PROCEED;
	if (sendrequest(DBREQ_XCTRL, size) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Start execution

static int startp(void)
{
	if (strtexe(XCTRL_START))
		return (CMD_CMD);
	return (CMD_WAIT);
}


// Trace instruction (single step with display)

static int trace(void)
{
	if (strtexe(XCTRL_SINGLE))
		return (CMD_CMD);
	return (CMD_WAIT);
}


static int strtexe(
	int func)
{
	VAL val;
	int size;

	val.val = 0;
	if (!lastexpression(&val))
		return (CMD_CMD);
	if (val.val != 0)
		size = 9;
	else
		size = 1;
	reqbufr.xctrl.func = func;
	return (sendrequest(DBREQ_XCTRL, size) < 0);
}


// BOOT instruction

static int reboot(void)
{
	reqbufr.xctrl.func = XCTRL_BOOT;
	return ((sendrequest(DBREQ_XCTRL, 1) < 0) ? CMD_CMD : CMD_WAIT);
}


// Search symbol table by value

static int srchval(void)
{
	if (!lastexpression(&wildval))
		return (CMD_CMD);
	return (wildbegin(0x04));
}

// Search symbol table by name

static int srchname(void)
{
	int bits;

	if (getname(TRUE, wildname, sizeof(wildname)) < 0)
		return (CMD_CMD);
	if (stopper != 0)
	{
		if (!lastexpression(&wildval))
			return (CMD_CMD);
		bits = 0x07;
	}
	else
		bits = 0x03;
	return (wildbegin(bits));
}


// Search symbol table by name for first match

static int srchfirst(void)
{
	if (getname(TRUE, wildname, sizeof(wildname)) < 0)
		return (CMD_CMD);
	if (stopper != 0)
	{
		if (!lastexpression(&wildval))
			return (CMD_CMD);
		wildbits = 0x07;
	}
	else
		wildbits = 0x03;

	if ((wildsym = wildsearch(wildsym, wildname, wildval.val, wildbits,
			&wildmod)) == NULL || wildmod == NULL)
		putstr("\r\n  No matches were found");
	else
	{
		putstr("\r\n     Value  Sup Name\r\n");
		putstr("%08X %c %-18.*s%c %.*s\r\n", wildsym->value,
				(wildsym->flags & SF_SUP) ? '!' : ' ', wildsym->length,
				wildsym->name, (wildsym->flags & SF_GBL) ? '#' : '@',
				wildmod->length, wildmod->name);
	}
	return (CMD_CMD);
}


// 

static int wildbegin(
	int bits)
{
	wildhead = FALSE;
	wildsym = NULL;
	wildmod = NULL;
	wildbits = bits;
	wildcnt = 0;
	linenum = 2;
	return (wildlist());
}


int wildlist(void)
{
	while (TRUE)
	{
		if (wildcnt == -1)
			return (CMD_CMD);
		if ((wildsym = wildsearch(wildsym, wildname, wildval.val, wildbits,
				&wildmod)) == NULL || wildmod == NULL)
		{
			if (wildcnt == 0)
				putstr("\r\n  No matches were found");
			else
				putstr("  %d match%s found", wildcnt, (wildcnt == 1) ?
						" was" : "es were");
			return (CMD_CMD);
		}
		if (!wildhead)
		{
			wildhead = TRUE;
			linenum++;
			putstr("\r\n     Value  Sup Name\r\n");
		}
		if (!linechk(wildlist))
			return (CMD_PAUSED);
		putstr("%08X %c %-18.*s%c %.*s\r\n", wildsym->value,
				(wildsym->flags & SF_SUP) ? '!' : ' ', wildsym->length,
				wildsym->name, (wildsym->flags & SF_GBL) ? '#' : '@',
				wildmod->length, wildmod->name);
		wildcnt++;
		*(uchar **)&wildsym = ((uchar *)wildsym) + offsetof(SYM, name) +
				wildsym->length;
	}
}


// Select module

static int dosetmodule(void)
{
	int  len;
	char name[128];
	char text[400];

	if ((len = getname(FALSE, name, sizeof(name))) < 0)
		return (CMD_CMD);
	if (stopper != 0)
	{
		putstr("\r\n\1? Syntax error\2\r\n");
		return (CMD_CMD);
	}
	if (name[0] == 0)
	{
		if (curmodule != NULL)
			sprintf(text, "\rModule %.*s is the current module\r\n",
					curmodule->length, curmodule->name);
		else
			strmov(text, "\rThere is no current module\r\n");
	}
	else if (setmodule(name))
	{
		sprintf(text,  "\rModule %s is now the current module\r\n", name);
		putstr(text);
	}
	return (CMD_CMD);
}


// List symbol tables

static int symlist(void)
{
	listsymtbls();
	return (CMD_CMD);
}


// Add symbol table

static int symadd(void)
{
	char *pnt;
	int   cnt;
	char  name[128];
	char  chr;

	pnt = name;
	cnt = 127;
	while ((chr = nxtchar()) != 0 && !isspace(chr) && --cnt > 0)
		*pnt++ = chr;	
	if (cnt <= 0)
	{
		putstr("\r\n\1? Name is too long\2\r\n");
		return (CMD_CMD);
	}
	if (chr != 0)
	{
		putstr("\r\n\1? Syntax error\2\r\n");
		return (CMD_CMD);
	}
	*pnt = 0;
	addsymtbl(name, 0, 0, symtblnum--);
	return (CMD_CMD);
}


// Remove symbol table

static int symremove(void)
{
	long number;

	if (!numeric(&number))
		return (CMD_CMD);
	if (stopper != 0)
	{
		putstr("\r\n\1? Invalid value\2\r\n");
		return (CMD_CMD);
	}
	removesymtbl(number, FALSE);
	return (CMD_CMD);
}


// Set output modes

static int setmodes(void)
{
	MODE newmode;
	int  chr;
	char text[128];

	if ((chr = nxtnbc()) != 0)
	{
		hldchar = chr;
		newmode = prmmode;
		while ((chr = toupper(nxtnbc())) != 0)
		{
			if (!getmode(chr, &newmode))
				return (CMD_CMD);
			if ((chr = nxtchar()) != 0 && !isspace(chr))
			{
				putstr("\r\n\1? Syntax error\2\r\n");
				return (CMD_CMD);
			}
		}
		prmmode = newmode;
	}
	sprintf(text, "\rOutput Mode: %s  Output Radix: %d  Input Radix: %d\r\n",
			(prmmode.omode == OM_SYMBOLIC) ? "Symbolic" :
			(prmmode.omode == OM_NUMERIC) ? "Numeric" : "Character",
			prmmode.oradix, prmmode.iradix);
	putstr(text);
	return (CMD_CMD);
}


static int getmode(
	int   chr,
	MODE *mode)
{
	switch(chr)
	{
	 case 'S':
		mode->omode = OM_SYMBOLIC;
		break;

	 case 'N':
		mode->omode = OM_NUMERIC;
		break;

	 case 'C':
		mode->omode = OM_CHAR;
		break;

	 case 'I':
		if (!getradix(toupper(nxtchar()), &mode->iradix))
		{
			putstr("\r\n\1? Invalid input radix\2\r\n");
			return (FALSE);
		}
		break;

	 default:
		if (!getradix(chr, &mode->oradix))
		{
			putstr("\r\n\1? Invalid output radix or mode\2\r\n");
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}



static int getradix(
	int  chr,
	int *radix)
{
	int newradix;

	switch (chr)
	{
	 case 'B':
	 case '2':
		newradix = 2;
		break;

	 case 'O':
	 case '0':
	 case '8':
		newradix = 8;
		break;

	 case '1':
		if ((chr = nxtchar()) == '0')
			newradix = 10;
		else if (chr == '6')
			newradix = 16;
		else
			goto badopt;
		break;

	 case 'D':
	 case 'T':
		newradix = 10;
		break;

	 case 'H':
	 case 'X':
		newradix = 16;
		break;

	 default:
	 badopt:
		return (FALSE);
	}
	*radix = newradix;
	return (TRUE);
}


// Exit GeckoR

static int goexit(void)
{
	svcIoOutString(DH_STDTRM, "\x1B[2J", 0);
	exit(0);
	return (0);
}


// Display help info

static int help(void)
{
	putstr("\r\n\1? Not implimented yet!\2\r\n");
	return (CMD_CMD);
}


// Set breakpoint

static int setbkp(void)
{
	return (bpcommon(1));
}


// Remove breakpoint

static int rmvbkp(void)
{
	return (bpcommon(2));
}



static int bpcommon(
	int func)
{
	long count;

	if (!expression(&argval))
		return (TRUE);
	if (stopper != 0)
	{
		if (!numeric(&count))
			return (CMD_CMD);
		if (stopper != 0)
		{
			putstr("\r\n\1? Invalid value\2\r\n");
			return (CMD_CMD);
		}
	}
	else
		count = 0;
	reqbufr.bpctrl.func = func;
	reqbufr.bpctrl.addr = argval.val;
	reqbufr.bpctrl.count = count;
	if (sendrequest(DBREQ_BPCTRL, sizeof(BPCTRLRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}

// Remove all breakpoints

static int clrbkp(void)
{
	if (nxtnbc() != 0)
	{
		putstr("\r\n\1? Command has no arguments\2\r\n");
		return (CMD_CMD);
	}
	reqbufr.bpctrl.func = 3;
	if (sendrequest(DBREQ_BPCTRL, 1) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// List breakpoints

static int lstbkp(void)
{
	if (nxtnbc() != 0)
	{
		putstr("\r\n\1? Command has no arguments\2\r\n");
		return (CMD_CMD);
	}
	if (sendrequest(DBREQ_BPLIST, 0) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Set data write watchpoint on byte

static int setwbwtp(void)
{
	return (wpcommon(2, 1));
}


// Set data write watchpoint on word

static int setwwwtp(void)
{
	return (wpcommon(2, 2));
}


// Set data write watchpoint on long

static int setwlwtp(void)
{
	return (wpcommon(2, 4));
}


// Set data access watchpoint on byte

static int setabwtp(void)
{
	return (wpcommon(1, 1));
}


// Set data access watchpoint on word

static int setawwtp(void)
{
	return (wpcommon(1, 2));
}


// Set data access watchpoint on long

static int setalwtp(void)
{
	return (wpcommon(1, 4));
}


// Set instruction watchpoint

static int setiwtp(void)
{
	return (wpcommon(3, 0));
}


// Remove watchpoint

static int rmvwtp(void)
{
	return (wpcommon(4, 0));
}


static int wpcommon(
	int func,
	int size)
{
	long count;

	if (!expression(&argval))
		return (TRUE);
	if (stopper != 0)
	{
		if (!numeric(&count))
			return (CMD_CMD);
		if (stopper != 0)
		{
			putstr("\r\n\1? Invalid value\2\r\n");
			return (CMD_CMD);
		}
	}
	else
		count = 0;
	reqbufr.wpctrl.func = func;
	reqbufr.wpctrl.size = size;
	reqbufr.wpctrl.addr = argval.val;
	reqbufr.wpctrl.count = count;
	if (sendrequest(DBREQ_WPCTRL, sizeof(WPCTRLRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Remove all watchpoints

static int clrwtp(void)
{
	if (nxtnbc() != 0)
	{
		putstr("\r\n\1? Command has no arguments\2\r\n");
		return (CMD_CMD);
	}
	reqbufr.bpctrl.func = 5;
	if (sendrequest(DBREQ_WPCTRL, 1) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// List watchpoints

static int lstwtp(void)
{
	if (nxtnbc() != 0)
	{
		putstr("\n\1? Command has no arguments\2\n");
		return (CMD_CMD);
	}
	if (sendrequest(DBREQ_WPLIST, 0) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


// Toggle display of global symbol's module name

static int togglegmn(void)
{
	showgmn ^= 0x01;
	putstr("\4Global sysbol module name display is %s", (showgmn) ?
			"on" : "off");
	return (CMD_CMD);
}


// Toggle debug output

static int toggledbo(void)
{
	debugout ^= 0x01;
	ensureleft();
	putstr((debugout) ? "Debug output is on" : "Debug output is off");
	return (CMD_CMD);
}


//***********************************************************
// Function: getpciaddr - Get PCI configuration address value
// Returnes: TRUE if OK, FALSE if error
//***********************************************************

// A PCI configuration consists of 4 values in the following order:
//   Bus number
//   Device number
//   Function number
//   Register number
// Values are separated by whitespace.

static int getpciaddr(
	long *val)
{
	long value;
	long addr;

	if (!numeric(&value))
		return (FALSE);
	if (((ulong)value) > 255)
	{
		putstr("\r\n\1? Invalid PCI bus number\2\r\n");
		return (FALSE);
	}
	addr = value << 16;
	if (!numeric(&value))
		return (FALSE);
	if (((ulong)value) > 31)
	{
		putstr("\r\n\1? Invalid PCI device number\2\r\n");
		return (FALSE);
	}
	addr |= (value << 11);
	if (!numeric(&value))
		return (FALSE);
	if (((ulong)value) > 7)
	{
		putstr("\r\n\1? Invalid PCI function number\2\r\n");
		return (FALSE);
	}
	addr |= (value << 8);
	if (!numeric(&value))
		return (FALSE);
	if (((ulong)value) > 63)
	{
		putstr("\r\n\1? Invalid PCI register number\2\r\n");
		return (FALSE);
	}
	addr |= (value << 2);
	*val = addr;
	return (TRUE);
}


// Clear screen

static int clrscreen(void)
{
	clearscreen();
	return (CMD_CMD);
}
