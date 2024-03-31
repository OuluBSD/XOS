//-----------------------------------------------------------------------
// LKELOADF.C
// Subroutine to load LKE
// 
// Written by: John R. Goltz
// 
// This function is intended to be called by LKELOAD and INSTALL
//-----------------------------------------------------------------------

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrun.h>
#include <xosstr.h>
#include <xoserr.h>
#include <xcmalloc.h>
#include <lkeload.h>

// Usage notes:

//   This function references the following function which the caller must
//   provide:
//		void message(int level, char *text);

//   This function is called whenever LKELOADF has information to display to
//   the user.  The level argument specifies the kind of information as
//   follows:
//		LKEML_INFO     = 1 - Routine information
//		LKEML_ERROR    = 2 - Error information
//		LKEML_FININFO  = 3 - Final routine information
//		LKEML_FINERROR = 4 - Final error information

//   The calling of the this function is controled by the value of the "quiet" 
//   argument as follows:
//		Bit 0 - Set if function is not to be called for routine information
//		Bit 1 - Set if function is not be be called for ER_FILNF errors

//   This function is thread-safe if called in a multi-threaded environment.
//   It uses no variable static storage. (The cfgchar structure is the one
//   partial exception - while it is technically variable, all threads using
//   it will always set it to the same value, so it is effectively constant!)
//   It may use up to about 1200 bytes of stack space.

//   This function defines an internal char array named lkename which is 64
//   bytes long.  The caller must store the full pathname for the LKE to load
//   into this array before calling lkeloadf.  This array is initialized to
//   "XOSSYS:" so the caller can just store the filename starting at byte 7
//   if desired.

//   The addbfr argument must point to a device characteristics list.  It may
//   be NULL if there are no characteristics.

//   This function does NO output to the controlling terminal unless DEBUG is
//   non-zero and thus may normally be freely called in a symbiont.

//   The value returned is the value returned by the second call to
//   svcSysLoadLke in LKEARGS.lla_value (always positive) if no error or
//   a negative XOS error code if error (message has been called to report
//   the error).

#define DEBUG 0

extern char prgname[];

typedef _Packed struct
{   BYTE4PARM pos;
    char      end;
} INPPARMS;



_Packed typedef struct
{	long   cnt;
	long   size;
	char  *pnt;
	char  *bufr;
	long   file;

	long   undefcnt;
	char   loaddone;		// TRUE if LKE has been loaded

	char   *code;
	char   *data;
	char   *symbol;
	long   *importtbl;
	EXPORTTABLE
	       *exportbfr;

	int     quietval;

	LKEARGS lkeargs;

	jmp_buf errjmp;

	RUNHEAD header;
	INPPARMS inpparms;
	char     lkename[72];
} LKE;


static INPPARMS inpparms =
{  {PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS}
};

static char erriomsg[] = "Error reading LKE file";

static struct
{
    BYTE4CHAR debug;
    char      end;
} cfgchar =
{
    {(PAR_GET|REP_TEXT), 4, "DEBUG"}
};


static ulong getbyte(LKE *lke);
static ulong getlong(LKE *lke);
static int   getprefix(char *pfx, char *sym);
static int   getsym(LKE *lke, char *name);
static long  getvar(LKE *lke, int size);
static ulong getword(LKE *lke);
static void  lkefail(LKE *lke, char *msg, long code);
static void  relocate(LKE *lke, char *location, long offset, long count,
		long base);
static void  setpos(LKE *lke, long pos);


int lkeloadf(
    int    quiet,
	char  *lkename,
    struct lkechar *addbfr)
{
	char *opnt;

	EXPORTENTRY
	     *symbolpnt;
	EXPORTTABLE
	     *exportpnt;


	union
	{	char   *c;
		ushort *s;
		long   *l;
	}     vpnt;

	LKE   lke;
    long  rtn;

	int   pfxsize;

    long  msnum;
    ulong header1;
    ulong header2;
    ulong value;
    int   valsize;
    int   cnt;
	int   segcnt;
	char  obufr[512];

	long   *importpnt;
	long    importcnt;
	long    exportcnt;
	long    symcnt;

	RUNSEG   dataseg;
	RUNMSECT datamsect;
	RUNMSECT symbolmsect;
	RUNSEG   codeseg;
	RUNMSECT codemsect;
	RUNSEG   extraseg;
	RUNMSECT extramsect;

	union
	{   char  c[8];
		llong ll;
	}        prefix;
	long     symsize;
	char     symname[52];
	INPPARMS rdparms;

	memset(&lke, 0, sizeof(lke));
	lke.inpparms = inpparms;
	rdparms = inpparms;
	lke.quietval = quiet;
	*strnmov(strmov(lke.lkename, "XOSSYS:"), lkename, 64) = 0;
	if ((rtn = setjmp(&lke.errjmp)) != 0)
	{
		if (lke.file > 0)
			svcIoClose(lke.file, 0);
		if (lke.code != NULL)
			free(lke.code);
		if (lke.data != NULL)
			free(lke.data);
		if (lke.symbol != NULL)
			free(lke.symbol);
		if (lke.importtbl != NULL)
			free(lke.importtbl);
		if (lke.exportbfr != NULL)
			free(lke.exportbfr);
		return((rtn < 0) ? rtn : lke.lkeargs.lla_value);
	}
	if ((lke.bufr = malloc(1024)) == NULL)
		lkefail(&lke, "Error allocating input buffer", -errno);

	//=====================================================================
	// First we open the LKE file and read the headers
	//=====================================================================

	lke.loaddone = FALSE;
	if ((lke.file = svcIoOpen(XO_IN, lke.lkename, NULL)) <= 0)
										// Open the LKE file
		lkefail(&lke, "Error opening LKE image file", lke.file);
	if ((rtn = svcIoInBlock(lke.file, (char *)&lke.header,
			sizeof(RUNHEAD))) < 1)
		lkefail(&lke, "Error reading LKE image file header", rtn);
	if (lke.header.magic != 0x22D7)		// Check magic number
	{
		sprintf(obufr, "Invalid signature word %04X in image header",
				lke.header.magic);
		lkefail(&lke, obufr, 0);
	}
	if (lke.header.fmtvrsn != 2)		// Check RUN version number
		lkefail(&lke, "LKE is not a version 2 image file", 0);
	if (lke.header.proctype != RUNP_80386 && lke.header.proctype !=
			RUNP_80486 && lke.header.proctype != RUNP_80586)
		lkefail(&lke, "LKE is not an 80386 image file", 0);
	if (lke.header.imagetype != RUNT_XOSLKE)
		lkefail(&lke, "LKE is not an LKE image", 0);
	if ((cnt = lke.header.hdrsize - 58) < 0)
		lkefail(&lke, "LKE image header is too small", 0);
	if (cnt != 0 && (rtn = svcIoSetPos(lke.file, cnt, 0)) < 0)
			lkefail(&lke, "Error positioning for IO in LKE image file", rtn);
	if ((segcnt = lke.header.numsegs - 2) < 0)
		lkefail(&lke, "LKE image does not contain at least two segments", 0);

	// Read the data segment header - There must exactly one data segment
	//   which must contain one or two msects. The first must be the data
	//   segment and the second (if there are two) must be the symbol msect.

	if ((rtn = svcIoInBlock(lke.file, (uchar *)&dataseg, sh_SIZE)) < 1)
		lkefail(&lke, "Error reading data segment header in LKE", rtn);
	if (dataseg.hdrsize != 12)
		lkefail(&lke, "Data segment header in LKE is not 12 bytes long", 0);
	if (dataseg.type != 1)
		lkefail(&lke, "First segment in LKE is not a data segment", 0);
	if ((dataseg.status & 1) == 1)
		lkefail(&lke, "Data segment in LKE specifies load address", 0);
	if (dataseg.nummsc != 2)
		lkefail(&lke, "Data segment does not contain exactly two msects", 0);

    // Read the data msect header

	if ((rtn = svcIoInBlock(lke.file, (uchar *)&datamsect,
			sizeof(RUNMSECT))) < 1)
		lkefail(&lke, "Error reading data msect header in LKE", rtn);
	if (datamsect.hdrsize != 36)
		lkefail(&lke, "Data msect header size incorrect in LKE", 0);

    // Read the symbol table msect header

	if ((rtn = svcIoInBlock(lke.file, (uchar *)&symbolmsect,
			sizeof(RUNMSECT))) < 1)
		lkefail(&lke, "Error reading symbol table msect header in LKE", rtn);
	if (datamsect.hdrsize != 36)
		lkefail(&lke, "Symbol table msect header size incorrect in LKE", 0);

	// Read the code segment header - There must be exactly one code segment,
	//   which must contain a single msect.

	if ((rtn = svcIoInBlock(lke.file, (uchar *)&codeseg,
			sizeof(RUNSEG))) < 1)
		lkefail(&lke, "Error reading code segment header in LKE", rtn);
	if (codeseg.hdrsize != 12)
		lkefail(&lke, "Code segment header in LKE is not 12 bytes long", 0);
	if (codeseg.type != 2)
		lkefail(&lke, "Second segment in LKE is not a code segment", 0);
	if ((codeseg.status & 1) == 1)
		lkefail(&lke, "Code segment in LKE specifies load address", 0);
	if (codeseg.nummsc != 1)
		lkefail(&lke, "Code segment does not contain exactly one msect", 0);
	if ((rtn = svcIoInBlock(lke.file, (uchar *)&codemsect,
			sizeof(RUNMSECT))) < 1)
		lkefail(&lke, "Error reading code msect header in LKE", rtn);
	if (codemsect.hdrsize != 36)
		lkefail(&lke, "Code msect header size incorrect in LKE", 0);
	while (--segcnt >= 0)
	{
		if ((rtn = svcIoInBlock(lke.file, (uchar *)&extraseg, sh_SIZE)) < 1)
			lkefail(&lke, "Error reading extra segment header in LKE", rtn);
		if (dataseg.hdrsize != 12)
			lkefail(&lke, "Extra segment header in LKE is not 12 bytes long",
					0);
		while (--*((long *)&extraseg.nummsc) >= 0)
		{
			if ((rtn = svcIoInBlock(lke.file, (uchar *)&extramsect,
					sizeof(RUNMSECT))) < 1)
				lkefail(&lke, "Error reading extra msect header in LKE", rtn);
			if (datamsect.hdrsize != 36)
				lkefail(&lke, "Extra msect header size incorrect in LKE", 0);
			if (extramsect.datasz != 0)
				lkefail(&lke, "More than 2 non-empty segments in LKE", 0);
		}
	}

    //=====================================================================
    // Now read the imported symbol table from the LKE we are loading.
    //   We search for each symbol as we read the symbol from the LKE and
    //   store its value in our in-memory imported symbol table.  We do not
    //   save the names, since we don't care about them once we have the
    //   associated values.
    //=====================================================================

    if (lke.header.importnum != 0)
    {
        if ((lke.importtbl = importpnt = (long *)malloc(sizeof(long) *
				lke.header.importnum)) == NULL)
	        lkefail(&lke, "Error allocating the import table", -errno);
        setpos(&lke, lke.header.importos);
        importcnt = lke.header.importnum;
		while (--importcnt >= 0)
        {
            getbyte(&lke);				// Discard the header byte
            symsize = getsym(&lke, symname); // Get symbol name
            symcnt = -1;

			if ((pfxsize = getprefix(prefix.c, symname)) == 0)
			{
				sprintf(obufr, "Invalid imported symbol name %s", symname);
				lkefail(&lke, obufr, 0);
			}
#if DEBUG
			sprintf(obufr, "pfs: |%s| %d\nsym: |%s|\n", prefix.c, pfxsize,
					symname);
			svcIoOutString(DH_STDOUT, obufr, 0);
#endif
            exportpnt = knlExportTable;
			do
			{
#if DEBUG
				sprintf(obufr, "Checking |%s| %08X %08X == %08X %08X\n",
						exportpnt->prefix, (long)(prefix.ll >> 32),
						(long)prefix.ll, *(long *)(exportpnt->prefix + 4),
						*(long *)exportpnt->prefix);
				svcIoOutString(DH_STDOUT, obufr, 0);
#endif
				if (*(llong *)(exportpnt->prefix) == prefix.ll)
					break;
			} while ((exportpnt = exportpnt->next) != NULL);

			// Here with the exported symbol table for this symbol

			if (exportpnt != NULL)
			{
#if DEBUG
				svcIoOutString(DH_STDOUT, "found table\n", 0);
#endif
				symcnt = exportpnt->num;
				symbolpnt = exportpnt->entry;
				symsize -= (pfxsize + 1); // Adjust length of name
				do
				{
#if DEBUG
					sprintf(obufr, "%d |%s| %d |%.*s|\n", symsize + 1,
							symname + pfxsize, (symbolpnt.c[1] & 0x1F) + 1,
							(symbolpnt.c[1] & 0x1F) + 1, symbolpnt.c + 2);
					svcIoOutString(DH_STDOUT, obufr, 0);
#endif
					if (symsize == (symbolpnt->cnt2 & 0x1F) &&
							(strncmp(symbolpnt->name, symname + pfxsize,
							symsize + 1) == 0))
						break;
                    *(char **)&symbolpnt += symbolpnt->cnt1;
                } while (--symcnt > 0);
            }
            if (symcnt > 0)
            {
                header1 = symbolpnt->cnt1;
                header2 = symbolpnt->cnt2;

				vpnt.c = symbolpnt->name + (header2 & 0x1F) + 1;
                valsize = header1 - (header2 & 0x1F) - 3;
                switch (valsize)
                {
                 case 0:
                    value = (header2 & 0x20)? -1: 0;
                    break;

                 case 1:
                    value = *vpnt.c;
                    if (header2 & 0x20)
                        value |= 0xFFFFFF00;
                    break;

                 case 2:
                    value = *vpnt.s;
                    if (header2 & 0x20)
                        value |= 0xFFFF0000;
                    break;

                 case 3:
                    value = *vpnt.s + (vpnt.c[2] << 16);
                    if (header2 & 0x20)
                        value |= 0xFF000000;
                    break;

                 case 4:
                    value = *vpnt.l;
                    break;

                 default:
					sprintf(obufr, "Illegal value size (%d) for kernel symbol "
							"%s", valsize, symname);
					lkefail(&lke, obufr, 0);
					break;
                }
                *importpnt = value;
            }
            else
            {
				opnt = obufr;
                if (lke.undefcnt == 0)
					opnt += sprintf(opnt, "? %s: Unable to load LKE %s\n",
							prgname, lke.lkename);
                ++lke.undefcnt;
				rtn = strlen(prgname) + 4;
				sprintf(opnt, "%*sImported symbol %s is undefined\n", rtn, " ",
						symname);
                message(LKEML_ERROR, obufr);
            }
            ++importpnt;
        }
        if (lke.undefcnt != 0)			// Did we have any undefined symbols?
        {								// Yes - stop now!

			sprintf(obufr, "%*s%d undefined imported symbol%s in %s\n", rtn,
					" ", lke.undefcnt, (lke.undefcnt != 1) ? "s" : "",
					lke.lkename);
            message(LKEML_FINERROR, obufr);
		    longjmp(&lke.errjmp, 2);
        }
    }

    //=====================================================================
    // Allocate memory to hold the code, data, and symbol table msects
    //=====================================================================

	codemsect.alloc = (codemsect.alloc + 3) & ~0x03;
    if ((lke.code = malloc(codemsect.alloc)) == NULL)
        lkefail(&lke, "Error allocating code msect buffer", -errno);

    if (datamsect.alloc != 0)
	{
		datamsect.alloc = (datamsect.alloc + 3) & ~0x03;
        if ((lke.data = malloc(datamsect.alloc)) == NULL)
	        lkefail(&lke, "Error allocating data msect buffer", -errno);
	}
    if (symbolmsect.alloc >= 8)
    {
        svcIoClsChar("SYSTEM:", &cfgchar); // See if XDT is loaded
        if ((char)cfgchar.debug.value == 'Y')
		{
			symbolmsect.alloc = (symbolmsect.alloc + 3) & ~0x03;
            if ((lke.symbol = malloc(symbolmsect.alloc)) == NULL)
		        lkefail(&lke, "Error allocating symbol msect buffer", -errno);
		}
        else
        {
            symbolmsect.alloc = 0;
            symbolmsect.datasz = 0;
        }
    }
    else
    {
        symbolmsect.alloc = 0;
        symbolmsect.datasz = 0;
    }

	//=====================================================================
	// Read the code, data, and symbol table msects
	//=====================================================================

	if (datamsect.datasz != 0)
	{
		rdparms.pos.value = datamsect.dataos;
		if ((rtn = svcIoInBlockP(lke.file, lke.data, datamsect.datasz,
				&rdparms)) < 1)
			lkefail(&lke, "Error reading data msect contents", rtn);
	}
	if (symbolmsect.datasz != 0)
	{
		rdparms.pos.value = symbolmsect.dataos;
		if ((rtn = svcIoInBlockP(lke.file, lke.symbol, symbolmsect.datasz,
				&rdparms)) < 1)
			lkefail(&lke, "Error reading symbol table msect contents", rtn);
	}
	rdparms.pos.value = codemsect.dataos;
	if ((rtn = svcIoInBlockP(lke.file, lke.code, codemsect.datasz,
			&rdparms)) < 1)
		lkefail(&lke, "Error reading code msect contents", rtn);
    lke.lkeargs.lla_char = (char *)addbfr; // Store address of characteristics
    lke.lkeargs.lla_ddata = lke.data;	   //   list
    lke.lkeargs.lla_dcount = datamsect.alloc;
    lke.lkeargs.lla_sdata = lke.symbol;
    lke.lkeargs.lla_scount = symbolmsect.alloc;
    lke.lkeargs.lla_cdata = lke.code;
    lke.lkeargs.lla_ccount = codemsect.alloc;

    //=====================================================================
    // Issue begin_load SVC
    //=====================================================================

    if ((rtn = svcSysLoadLke(&lke.lkeargs)) < 0) // Error in begin_load
		lkefail(&lke, "Error when initializing LKE load operation", rtn);
    if (lke.header.start != 0)
        lke.lkeargs.lla_init = lke.header.start + lke.lkeargs.lla_caddr;
										// Relocate starting address
    relocate(&lke, lke.data, datamsect.relos, datamsect.relsz,
			lke.lkeargs.lla_daddr);		// Relocate the data msect
    if (symbolmsect.datasz != 0)
    {
        relocate(&lke, lke.symbol, symbolmsect.relos, symbolmsect.relsz,
				lke.lkeargs.lla_saddr);	// Relocate the symbol msect
    }
    relocate(&lke, lke.code, codemsect.relos, codemsect.relsz,
			lke.lkeargs.lla_caddr);		// Relocate the code msect

    //=====================================================================
    // Read the exported symbol defintions from the LKE image file and
    //   construct the new kernel exported symbol table which we will pass
    //   with the svcSysLoadLke SVC.  It is built in exactly the same
    //   format as it appears in the kernel's exported symbol table. The
	//   table starts with a 16 byte header as follows:
	//		typedef struct
	//		{   EXPORTTABLE *next;		// Address of next table, 0 if end
	//		    char         prefix[8];	// Prefix string
	//		    long         num;		// Number of entries
	//		    EXPORTENTRY  entry[];
	//		} EXPORTTABLE;
	// There is one entry in for each symbol defined. It has the following
	//   format:
	//		typedef struct
	//		{   uchar length1;		// Total length of this entry,
	//								//   including this byte.
	//		    uchar length2;		// Length of the name string - 1
	//		    char  name[n];		// Name string (does not include the
	//								//   prefix bytes)
	//		    xxxx  value;		// Value (may be 1, 2, or 4 bytes)
	//		} EXPORTENTRY;
	// We initally allocate memory for this table. If the total length of
	//   the table is not specified (old xlink version), we assume that
	//   each symbol is 50 characters! We also assume that each value is
	//   4 bytes long (this really makes little difference).
    //=====================================================================

    if ((exportcnt = lke.header.exportnum) != 0)
    {
		if (lke.header.exportsz == 0)
			lke.header.exportsz = exportcnt * 50;
		if ((lke.lkeargs.lla_xdata = lke.exportbfr =
				(EXPORTTABLE *)malloc(lke.header.exportsz + 6 * exportcnt +
				26)) == NULL)
	        lkefail(&lke, "Error allocating exported symbol table", -errno);

		memset(lke.exportbfr, 0, sizeof(EXPORTTABLE));
		lke.exportbfr->num = exportcnt;
		symbolpnt = lke.exportbfr->entry;
        setpos(&lke, lke.header.exportos);
        do
        {
            header1 = getbyte(&lke);	// Get the header byte

            switch (header1 & 0x03)		// Get the address value
            {
             case 0:
                value = 0;
                break;

             case 1:
                value = getbyte(&lke);
                if (header1 & 0x04)
                    value |= 0xFFFFFF00;
                break;

             case 2:
                value = getword(&lke);
                if (header1 & 0x04)
                    value |= 0xFFFF0000;
                break;

             case 3:
                value = getlong(&lke);
                break;
            }

            if (header1 & 0x20)
            {
                if (header1 & 0x80)
                    lkefail(&lke, "Absolute selector specified for exported"
                            " symbol", 0);
                else
                    msnum = getbyte(&lke);
            }
            else
                msnum = 0;
            symsize = getsym(&lke, symname); // Get symbol name
            if (symsize < 7)
            {
				sprintf(obufr, "Exported symbol name %s is too short", symname);
                lkefail(&lke, obufr, 0);
            }
			switch (msnum)
			{
			 case 0:
				break;

             case 1:
                value += lke.lkeargs.lla_daddr;
				break;

             case 2:
                value += lke.lkeargs.lla_saddr;
				break;

			 case 3:
				value += lke.lkeargs.lla_caddr;
				break;

			 default:
				sprintf(obufr, "Msect number (%d) for exported symbol %s is "
						"out of range", msnum, symname);
				lkefail(&lke, obufr, 0);
				break;
			}
			if (lke.exportbfr->prefix[0] == 0)
			{
				// Here if this is the first exported symbol - remember the
				//   prefix used

				if ((pfxsize = getprefix(lke.exportbfr->prefix, symname)) == 0)
				{
					sprintf(obufr, "Invalid exported symbol name %s", symname);
					lkefail(&lke, obufr, 0);
				}
			}
			else
			{
				// Here if not the first exported symbol - Verify that the
				//   prefix is the same
#if DEBUG
				sprintf(obufr, "sym: |%s|\npfx: |%s| %d\n", symname,
						lke.exportbfr->prefix, pfxsize);
				svcIoOutString(DH_STDOUT, obufr, 0);
#endif
                if (strncmp(symname, lke.exportbfr->prefix, pfxsize) != 0 ||
						symname[pfxsize] == 0 || islower(symname[pfxsize]))
				{
					sprintf(obufr, "More than one exported symbol prefix "
							"(%.8s/%s) used in the LKE", lke.exportbfr->prefix,
							symname);
					lkefail(&lke, obufr, 0);
				}
			}
            if (value == 0 || value == -1)
                valsize = 0;
            else if ((value & 0xFFFFFF00) == 0 ||
                    (value & 0xFFFFFF00) == 0xFFFFFF00)
                valsize = 1;
            else if ((value & 0xFFFF0000) == 0 ||
                    (value & 0xFFFF0000) == 0xFFFF0000)
                valsize = 2;
            else if ((value & 0xFF000000) == 0 ||
                    (value & 0xFF000000) == 0xFF000000)
                valsize = 3;
            else
                valsize = 4;
            header2 = symsize - pfxsize - 1;
            if (valsize != 4 && (value & 0x80000000))
                header2 |= 0x20;

            header1 = symsize + valsize - pfxsize + 2;
			symbolpnt->cnt1 = header1;	// Store the entry
			symbolpnt->cnt2 = header2;
            vpnt.c = strnmov(symbolpnt->name, symname + pfxsize,
					symsize - pfxsize);
            switch (valsize)
            {
             case 1:
                *vpnt.c++ = (char)value;
                break;

             case 2:
                *vpnt.s++ = (short)value;
                break;

             case 3:
                *vpnt.c++ = (char)value;
                *vpnt.s++ = (short)(value >> 8);
                break;

             case 4:
                *vpnt.l++ = value;
                break;
            }
			symbolpnt = (EXPORTENTRY *)vpnt.c;
		} while (--exportcnt > 0);
		symbolpnt->cnt1 = 0;				// Insure a zero byte at the end
		lke.lkeargs.lla_xcount = (&symbolpnt->cnt1 - ((char *)lke.exportbfr) +
				3) & 0xFFFFFFFC;
    }

    //=====================================================================
    // Issue do_load SVC
    //=====================================================================

    if ((rtn = svcSysLoadLke(&lke.lkeargs)) < 0)
		lkefail(&lke, "Error when doing LKE load operation", rtn);
    lke.loaddone = TRUE;
    if ((lke.quietval & 0x01) == 0)
    {
		opnt = obufr + sprintf(obufr, "%s: LKE %s %s\n", prgname, lkename,
				(lke.lkeargs.lla_csize == 0) ? "initialized but not loaded" :
				"loaded");
		rtn = strlen(prgname) + 2;
		if (lke.lkeargs.lla_csize == 0)
			sprintf(opnt, "%*sData size: %d\n", rtn, " ",
					lke.lkeargs.lla_dsize);
		else
		{
			opnt += sprintf(opnt, "%*sCode size: %d, Data size: %d", rtn, " ",
					lke.lkeargs.lla_csize, lke.lkeargs.lla_dsize);
			if (lke.lkeargs.lla_scount != 0)
				opnt += sprintf(opnt, ", Symbol size: %d",
						lke.lkeargs.lla_ssize);
			*opnt++ = '\n';

			if (lke.header.exportsz != 0)
				sprintf(opnt, "%*sExported symbol table updated, %d symbol%s "
						"added, prefix is %.8s\n", rtn , " ",
						lke.header.exportsz, (lke.header.exportsz != 1) ?
						"s" : "", lke.exportbfr->prefix);
			else
				*opnt = 0;
		}
		message(LKEML_FININFO, obufr);
    }
    longjmp(&lke.errjmp, 1);
}


//***************************************************
// Function: lkefail - display message on fatal error
// Returned: Never returns
//***************************************************

static void lkefail(
	LKE  *lke,
	char *msg,
	long  code)
{
	char *opnt;
	int   len;
	char  obufr[512];

	len = strlen(prgname) + 4;
	if ((code != ER_FILNF || (lke->quietval & 0x02) == 0) &&
			(code != ER_LKEAL || (lke->quietval & 0x04) == 0))
	{
		opnt = obufr;
		if (lke->undefcnt == 0 || lke->loaddone)
		{
			if (lke->loaddone)
				opnt += sprintf(opnt, "? %s: Unable to update system export "
						"definitions after\n%*sloading LKE %s\n", prgname,
						len, " ", lke->lkename);
			else
				opnt += sprintf(opnt, "? %s: Unable to load LKE %s\n", prgname,
						lke->lkename);
		}
		opnt += sprintf(opnt, "%*s%s\n", len, " ", msg);
		if (code != 0)
		{
			opnt += sprintf(opnt, "%*s", len, " ");
			opnt += svcSysErrMsg(code, 0x03, opnt);
			*opnt++ = '\n';
			*opnt = 0;
		}
		message((code == ER_PDNAV) ? LKEML_FININFO : LKEML_FINERROR, obufr);
	}
	longjmp(&lke->errjmp, (code == 0) ? ER_ERROR : code);
}


//*************************************************************
// Function: relocate - Process relocation records for an msect
// Returned: Nothing
//*************************************************************

static void relocate(
	LKE  *lke,
	char *buffer,			// Address where msect has been loaded
	long  offset,			// Offset in file of first relocation item
	long  count,			// Number of relocation items
	long  base)				// Address when msect will be placed in the kernel
{
    char *pnt;
    ulong item;
    ulong location;
    ulong relocaddr;
    uchar byte;
    uchar type;
    uchar kind;

	if (count == 0)
		return;
    setpos(lke, offset);
    location = 0;
	do
    {
        byte = getbyte(lke);
        type = byte >> 4;				// Get relocation type
        kind = (byte >> 2) & 0x03;		// Get relocation kind
        item = getbyte(lke);				// Get item number
        if (item & 0x80)
        {
            item = (item & 0x7F) << 8;
            item |= getbyte(lke);
            if (item & 0x4000)
            {
                item = (item & 0x3FFF) << 8;
                item |= getbyte(lke);
                if (item & 0x200000)
                {
                    item = (item & 0x1FFFFF) << 8;
                    item |= getbyte(lke); 
                }
            }
        }
        switch (kind)
		{
		 case 1:
			switch (item)
			{
			 case 1:
				relocaddr = lke->lkeargs.lla_daddr;
				break;

			 case 2:
				relocaddr = lke->lkeargs.lla_saddr;
				break;

			 case 3:
				relocaddr = lke->lkeargs.lla_caddr;
				break;

			 default:
				lkefail(lke, "Bad relocation information in LKE: Msect number "
						"out of range", 0);
				break;
			}
			break;

         case 2:
            if (item == 0 || item > lke->header.importnum)
                lkefail(lke, "Bad relocation information in LKE: Symbol number "
                        " out of range", 0);
            relocaddr = lke->importtbl[item-1];
			break;

		 default:
            lkefail(lke, "Bad relocation information in LKE: Illegal relocation"
					" kind", 0);
			break;
        }
        location += getvar(lke, byte);	// Get location for relocation
        pnt = buffer + location;

        switch (type)
        {
         case 5:						// 8-bit relative offset
            relocaddr -= (location + base);

         case 4:						// 8-bit absolute offset
			*((char *)pnt) += (char)relocaddr;
            break;

         case 7:						// 16-bit relative offset
            relocaddr -= (location + base);

         case 6:						// 16-bit absolute value
			*((short *)pnt) += (short)relocaddr;
            break;

         case 9:						// 32-bit relative offset
            relocaddr -= (location + base);

         case 8:						// 32-bit absolute value
			*((long *)pnt) += relocaddr;
            break;

         default:
            lkefail(lke, "Bad relocation information in LKE: Illegal relocation"
                        " type", 0);
		}
    } while (--count > 0);
}


//**************************************************************
// Function: getsym - Collect symbol name from relocation record
// Returned: Length of the symbol name
//**************************************************************

static int getsym(
	LKE  *lke,
	char *name)
{
    int   cnt;
    char *pnt;
    char  chr;
	char  obufr[96];

    cnt = 47;
    pnt = name;
    while (!((chr = getbyte(lke)) & 0x80))
    {
        if (--cnt < 0)
		{
			sprintf(obufr, "Symbol %.47s in LKE is too long", name);
            lkefail(lke, obufr, 0);
		}
        *pnt++ = chr;
    }
    *pnt++ = chr & 0x7F;
    *pnt = 0;
    return (pnt - name);
}


//************************************************************************
// Function: getvar - Collect variable length value from relocation record
// Returend: Value collected
//************************************************************************

static long getvar(
	LKE *lke,
    int  size)

{
    switch (size & 0x03)
    {
     case 0:
        return (getbyte(lke));

     case 1:
        return (getword(lke));
        break;

     case 2:
     {
        long temp;

        temp  = getbyte(lke);
        return ((getword(lke) << 8) | temp);
        break;
     }

     default:
        return (getlong(lke));
    }    
}


//*******************************************************
// Function: getprefix - Get prefix part of a symbol name
// Returned: Length of prefix, 0 if invalid name
//*******************************************************

static int getprefix(
	char *pfx,
	char *sym)
{
	char *pnt;
	int   cnt;
	char  chr;

	*(llong *)pfx = 0;
	cnt = 8;
	pnt = pfx;
	do
	{
		if ((chr = *sym++) != 0 && islower(chr))
			*pnt++ = chr;
		else
			break;
	} while (--cnt > 0);
	if (cnt == 0 && (chr = *sym) == 0 || islower(chr))
		return (0);
	return (pnt - pfx);
}


//*********************************************************
// Function: setpos - Set IO position for next getbyte call
// Returned: Nothing
//*********************************************************

static void setpos(
	LKE *lke,
    long pos)
{
    long block;
    long offset;
    long rtn;

    block = pos & ~0x3FF;
    offset = pos & 0x3FF;
    if (block != lke->inpparms.pos.value)
    {
        lke->inpparms.pos.value = block;
        if ((rtn = svcIoInBlockP(lke->file, lke->bufr, 1024,
				&lke->inpparms)) < 1)
            lkefail(lke, erriomsg, rtn);
        lke->size = rtn;
    }
    lke->cnt = lke->size - offset;
    lke->pnt = lke->bufr + offset;
}


//***************************************************
// Function: getbyte - Get byte value from image file
// Returned: Value
//***************************************************

static ulong getbyte(
	LKE *lke)
{
    long  rtn;

    if (--lke->cnt < 0)
    {
        lke->inpparms.pos.value += 1024;
        if ((rtn = svcIoInBlockP(lke->file, lke->bufr, 1024,
				(char *)&lke->inpparms)) < 1)
            lkefail(lke, erriomsg, rtn);
        lke->cnt = rtn - 1;
        lke->size = rtn;
        lke->pnt = lke->bufr;
    }
    return (*lke->pnt++);
}


//***************************************************
// Function: getword - Get word value from image file
// Returned: Value
//***************************************************

static ulong getword(
	LKE *lke)
{
    ulong data;

    data = getbyte(lke);
    data |= (getbyte(lke) << 8);
    return (data);
}


//***************************************************
// Function: getlong - Get long value from image file
// Returned: Value
//***************************************************

static ulong getlong(
	LKE *lke)
{
    ulong data;

    data = getword(lke);
    data |= (getword(lke) << 16);
    return (data);
}
