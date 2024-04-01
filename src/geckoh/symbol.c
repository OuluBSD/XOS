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
#include "xoswildcard.h"

// Format of the symbol table file:
//   File header (12 bytes):
//     ID        (4 bytes)  = 0x40222D4
//     Length    (4 bytes)  = Number of bytes in file (does not include this
//                            12-byte header)
//     Count     (4 bytes)  = Number of table entries in file
//   Symbol definition:
//     Flags     (1 byte)   = Symbol flags:
//                 80 - Symbol is a selector value
//                 40 - Symbol is multiply defined
//                 20 - Symbol is an address
//                 10 - Symbol is suppressed
//                 08 - Symbol is global
//                 04 - Symbol is a msect selector/offset value
//                 02 - Symbol is a module name
//                 01 - Symbol value is relocatable
//     Offset    (4 bytes)  = Offset part of value
//     Selector  (2 bytes)  = Selector part of value if absolute or msect
//                            number if relocatable
//     Name      (variable) = Symbol name (high bit set on last byte)

// The symbol information in memory is formated in a number of linked symbol
//   "tables". Each table is a set of consectitive variable length symbol
//   entries. Each table starts with a 128-byte header as follows:
//     next:   (SYMTBL *)  Address if the next symbol table. NULL is this is the
//							 last table.
//     count:  (long)      Number of entries in this table.
//	   number: (ushort)    LKE number (0 if kernel or program)
//     name:   (char[118]) Name of the lke or program
//  This is immediately followed in memory by the first symbol entry.
// Format of a symbol entry in memory:
//     length: (uchar)   Length of symbol name (in bytes)
//	   flags:  (uchar)   Symbol flags:
//						   SF_MLT = 40 - Symbol is multiply defined
//						   SF_ADR = 20 - Symbol is an address
//						   SF_SUP = 10 - Symbol is suppressed
//						   SF_GLB = 08 - Symbol is global
//						   SF_MOD = 02 - Symbol is a module name
//						   SF_V86 = 01 - Symbol is a V86 address
//     value:  (long)    Value
//     name:   (char[n]) Symbol name
// Symbol entries are tightly packeted. There is at least one 0 byte after
//   the last entry in a table. The next table starts on a long boundry.
// Even though there is a linked list of symbol tables they are actually
//   allocated continuously in a separate symbol table msect. If a symbol
//   table is removed the tables above it are moved down any any excess
//   memory is given up. Symbol tables are always added after the last
//   current table. To ensure future compatability, all code should use
//   the pointer in the table header to advance to the next table and not
//   count on the fact that it immediately follows the previous table.

// We count on the fact that an LKE has a rather rigid format. It always
//   consists of two segments. Segment #1 contains two msects. Msect #1
//   is always the data msect. Msect #2 is always the symbol table msect.
//   Currently the symbol msect is always present, even if the LKE is
//   built without symbols. In this case it is 4 bytes long. Segment #2
//   is always the code segment. It contains a single msect (msect #3).
//   Since there should never be any symbols defined for locations in the
//   symbol table itself, if we see msect #2 in the symbol file we assume
//   that this is the code msect and that the LKE was built without a
//   symbol table msect. If we see both msects #2 and #3 we say the file
//   is invalid.


static FILE *file;

typedef struct
{	long label;
	long bytes;
	long symbols;
} FILEHEAD;

typedef _Packed struct
{	uchar  flags;
	long   value;
	ushort sel;
	char   name[1];
} FILESYM;

union
{	uchar  *c;
	SYM    *e;
	SYMTBL *t;
}      symnext;				// Address for next symbol entry
uchar *symtop;				// Next address to allocate

char   *sympath;
char   *sympathend;
SYM    *thissym;
char   *name;
SYMTBL *lastsymtbl;
SYMTBL *lastsymtblsv;
int     lastlkenum;

char memmsg[] = "Cannot allocate memory for symbol table";
char filemsg[] = "Error reading sysbol table file entry";

char msecterr[] = "Invalid relocation information in symbol table";

static int  allocsymspace(int size);
static void symfail(long code, char *text);
static void symerror(char *msg, char *name, SYM *module);


//*********************************************************
// Function: syminit - Initialize the symbol table routines
// Returned: TRUE if OK, FALSE if error
//*********************************************************

void syminit(
	char *path)
{
	static char immsg[] = "Cannot allocate initial memory for symbol table";

	long rtn;

	if ((rtn = svcMemChange(&symtbl, PG_WRITE, 0x1000)) < 0)
		setupfail(rtn, immsg);
	symnext.t = &symtbl;
	symtop = symnext.c + rtn;
	rtn = strlen(path);
	if ((sympath = malloc(rtn + 40)) == NULL)
		setupfail(rtn, immsg);
	sympathend = strmov(sympath, path);
}

FILE *debuglog;
long fpos;
long symleft;

//******************************************
// Function: addsymtbl - Load a symbol table
// Returned: TRUE if OK, FALSE if error
//******************************************

int addsymtbl(
	char  *namearg,		// Name of LKE or program (no extension) or NULL for
						//   kernel (if program name, the buffer must have
						//   room for 4 additional bytes)
	uchar *codeaddr,	// Code address (LKE only)
	uchar *dataaddr,	// Data address (LKE only)
	int    number)		// Symbol table number: negative if program, 0 if
						//   kernel, or positive if LKE
{
	uchar   *pnt;
	uchar   *ext;
	FILEHEAD head;
	FILESYM  filesym;
	int      rtn;
	int      len;
	int      codemsect;
	int      symcnt;
	uchar    trunc;
	char     chr;
	char     text[200];
	char     errtxt[100];

	ensureleft();
	if (number < 0)
	{
		name = namearg;
		ext = name + strlen(name);
	}
	else
	{
		name = (namearg == NULL) ? "XOST" : namearg;
		ext = strmov(sympathend, name);
		name = sympath;
	}
	strmov(ext, ".sym");
	file = fopen(name, "rb");
	*ext = 0;
	if (file == NULL)
	{
		ensureleft();
		if (errno == -ER_FILNF)
			putstr("\1? Symbol table file for %s was not found\2\r\n", name);
		else
		{
			svcSysErrMsg(-errno, 0x03, errtxt);
			putstr("\1? Error opening sysbol table file for %s\r\n  %s\2",
					name, errtxt);
		}
		return (FALSE);
	}
	if ((rtn = fread(&head, 1, sizeof(FILEHEAD), file)) < 0)
	{
		svcSysErrMsg(-errno, 0x03, errtxt);
		ensureleft();
		putstr("\1? Error reading sysbol table file header for %s\n\r"
				"    %s\2\r\n", errtxt, name);
		fclose(file);
		return (FALSE);
	}

	fpos = sizeof(FILEHEAD);

	if (head.label != 0x040222D4)
	{
		ensureleft();
		putstr("\1? Sysbol table file header label for %s is "
				"incorrect\2\r\n", name);
		fclose(file);
		return (FALSE);
	}
	codemsect = 0;
	lastsymtblsv = lastsymtbl;
	if (lastsymtbl != NULL)				// Set up the module header
		lastsymtbl->next = symnext.t;
	lastsymtbl = symnext.t;
	if (!allocsymspace(offsetof(SYMTBL, sym)))
		return (FALSE);

///	lastsymtbl->count = head.symbols;

	strncpy(lastsymtbl->name, name, sizeof(lastsymtbl->name));
	lastsymtbl->number = number;
	symcnt = 0;

	symleft = head.symbols;

	debuglog = fopen("debug.log", "w");

	while (--head.symbols >= 0)
	{
		--symleft;
		if (symleft != head.symbols)
			INT3;

		if (ftell(file) != fpos)
			INT3;

		thissym = symnext.e;
		pnt = &symnext.e->flags;
		if (!allocsymspace(offsetof(SYM, name))) // Allocate space for the
			return (FALSE);						 //   symbol table entry
		if ((rtn = fread((char *)&filesym, 1, offsetof(FILESYM, name),
				file)) < 0)						// Read the symbol header
		{
			INT3;

			fclose(debuglog);
			symfail(errno, filemsg);
			return (FALSE);
		}

		fpos += offsetof(FILESYM, name);

		thissym->flags = filesym.flags;
		thissym->value = filesym.value;
		len = 0;
		trunc = FALSE;
		while (TRUE)					// Read the symbol name allocating
		{								//   space as we do
			if ((rtn = fread(&chr, 1, 1, file)) < 0)
			{
				INT3;

				fclose(debuglog);
				symfail(errno, filemsg);
				return(FALSE);
			}

			fpos += 1;

			if (len < SYMMAX)
			{
				if (!allocsymspace(1))
					return (FALSE);
				symnext.c[-1] = chr;
				len++;
			}
			else if (!trunc)
			{
				ensureleft();
				sprintf(text, "\1% Symbol name %.*s truncated\2\n\r", len,
						thissym->name);
				trunc = TRUE;
			}
			if (chr & 0x80)
			{
				symnext.c[-1] &= 0x7F;
				break;
			}
		}
		thissym->length = len;

		fprintf(debuglog, "%6d |%.*s|\n", symcnt, len, thissym->name);

		// If loading symbols for an LKE, relocate the value if it is
		//   relocatable.

		if (filesym.flags & SF_REL)		// Relocatable symbol?
		{
			if (number < 0)				// Yes - for a program?
			{
				ensureleft();
				putstr("Relocatable symbol %.*s discarded for program %s.%s"
						"\r\n", len, thissym->name, name, ext);
				symnext.e = thissym;
				continue;
			}
			else if (number > 0)		// Is it for an LKE?
			{
				switch (filesym.sel)
				{
				 case 1:
					thissym->value += (long)dataaddr;
					break;

				 case 2:
					if (codemsect == 3)
					{
						ensureleft();
						putstr("Illegal relocation (2) for symbol %.*s in "
								"%s.%s\r\n", len, thissym->name, name, ext);
						break;
					}
					codemsect = 2;
					goto reloccode;

				 case 3:
					if (codemsect == 2)
					{
						ensureleft();
						putstr("Illegal relocation (3) for symbol %.*s in "
								"%s.%s\r\n", len, thissym->name, name, ext);
						break;
					}
					codemsect = 3;
				 reloccode:
					thissym->value += (long)codeaddr;
					break;

				 default:
					ensureleft();
					putstr("Illegal relocation (%d) for symbol %.*s\r in "
							"%s.%s\r\n", len, thissym->name, name, ext);
					break;
				}
			}
			else						// Relocatable symbol for the kernel -
			{							//   just discard it quietly. (The only
				symnext.e = thissym;	//   relocatable symbols for the kernel
				continue;				//   should be the ones used to locate
			}							//   real mode segments when loading.
		}								//   We really don't card about these.)
		lastsymtbl->count++;
		symcnt++;
	}
	fclose(debuglog);
	fclose(file);
	if (!allocsymspace(1))
		return (FALSE);
	symnext.c[-1] = 0xFF;
	ensureleft();
	putstr("%6d symbol%s loaded for %s (%d)\r\n", symcnt, (symcnt == 1) ?
			"" : "s", name, number);
	return (TRUE);
}


static int allocsymspace(
	int size)
{
	long rtn;

	symnext.c += size;
	if (symnext.c > symtop)
	{
		if ((rtn = svcMemChange(&symtbl, PG_WRITE,
				symnext.c - (uchar *)&symtbl)) < 0)
		{
			symfail(errno, memmsg);
			return (FALSE);
		}
		symtop = ((uchar *)&symtbl) + rtn;
	}
	return (TRUE);
}


void removesymtbl(
	int number,
	int quiet)
{
	SYMTBL *tpnt;
	SYMTBL *ppnt;
	long    temp;
	char    name[SYMMAX + 4];

///	putstr("\r\nRequest to remove number = %d\r\n", number);

	tpnt = &symtbl;
	ppnt = NULL;
	do
	{
		if (tpnt->number == number)
		{
			strncpy(name, tpnt->name, SYMMAX);
			name[SYMMAX] = 0;

///			putstr("\r\nCurrent size is 0x%X\r\n", symnext.c -
///					(uchar *)&symtbl);

			if (tpnt->next != 0)		// Removing last table?
			{							// No
				temp = ((uchar *)tpnt->next) - (uchar *)tpnt;

///				putstr("Reducing size by %d\r\n", temp);

				memcpy(tpnt, tpnt->next, symnext.c - (uchar *)tpnt->next);
				while (tpnt->next != 0)
				{
					tpnt->next = (SYMTBL *)(((uchar *)(tpnt->next)) - temp);
					tpnt = tpnt->next;
				}
				symnext.c -= temp;
				lastsymtbl = (SYMTBL *)(((uchar *)lastsymtbl) - temp);
			}
			else						// Yes
			{
				symnext.t = tpnt;
				if (ppnt != NULL)		// Removing only table?
				{
					ppnt->next = NULL;	// No
					lastsymtbl = ppnt;
				}
				else
				{
					symtbl.count = 0;	// If removing only table
					symtbl.next = NULL;
					lastsymtbl = NULL;
				}
			}
			if ((((long)symnext.t) & 0xFFFFF000) !=
					(((long)symtop) & 0xFFFFF000))
			{
				temp = symnext.c - (uchar *)&symtbl;

///				putstr("Reducing size to 0x%X\r\n", temp);

///				INT3;

				symtop = ((uchar *)&symtbl) + svcMemChange(&symtbl,
						PG_READ|PG_WRITE, (temp != 0) ? temp : 0x1000);

///				putstr("New size is 0x%X\r\n", symtop - (uchar *)&symtbl);
			}

///			putstr("lastsymtbl: %X, symnext: %X, symtop: %X\r\n", lastsymtbl,
///					symnext.t, symtop);

			ensureleft();
			putstr("Symbol table %d for %s has been removed\r\n", number,
					name);
			return;
		}
		ppnt = tpnt;
	} while ((tpnt = tpnt->next) != NULL);
	if (!quiet)
	{
		ensureleft();
		putstr("1? Symbol table %d does not exist\2\r\n", number);
	}
}


static void symfail(
	long  code,
	char *msg)
{
	char  errtxt[100];

	fclose(file);
	thissym->length =0xFF;
	symnext.t = (lastsymtbl != NULL) ? lastsymtbl : &symtbl;
	lastsymtbl = lastsymtblsv;
	svcSysErrMsg(-code, 0x03, errtxt);
	ensureleft();
	putstr("\1? %s for %s\r\n  %s\2\r\n", msg, name, errtxt);
}


void listsymtbls(void)
{
	SYMTBL *tpnt;
	int     cnt;

	if (symtbl.count == 0)
	{
		putstr("\r\nNo symbol tables are loaded\r\n");
		return;
	}
	putstr("\r\n Count Nmbr LKE or program\r\n");
	tpnt = &symtbl;
	cnt = 0;
	do
	{
		putstr("%6d %3d  %.120s [%08X %08X]\r\n", tpnt->count, tpnt->number,
				tpnt->name, tpnt, tpnt->next);
		cnt++;
	} while ((tpnt = tpnt->next) != NULL);

	putstr("%d symbol table%s loaded\r\n", cnt, (cnt == 1) ? " is" : "s are");
}


//*****************************************
// Function: setmodule - Set current module
// Returned: 
//*****************************************

int setmodule(
	char *name)
{
	SYM *module;

	if (name[0] == '?' && name[1] == 0)
		curmodule = NULL;
	else
	{
		if ((module = findmodule(name)) == NULL)
			return (FALSE);
		curmodule = module;
	}
	return (TRUE);
}



SYM *findmodule(
	char *name)
{
	SYMTBL *tpnt;
	union
	{	uchar *c;
		SYM   *s;
	}       spnt;
	int     len;

	len = strlen(name);
	tpnt = &symtbl;
	do
	{
		spnt.s = tpnt->sym;
		while (spnt.s->length != 0xFF)
		{
			if (spnt.s->flags & SF_MOD)
			{
				if (spnt.s->length == len && strncmp(name, spnt.s->name,
						spnt.s->length) == 0)
					return (spnt.s);
			}
			spnt.c += (offsetof(SYM, name) + spnt.s->length);
		}
	} while ((tpnt = tpnt->next) != NULL);
	ensureleft();
	putstr("\1? Module %s was not found\2\r\n", name);
	return (NULL);
}


//*****************************************************************
// Function: findsymbol = Find symbol table entry given symbol name
// Returned: Address of entry or NULL if not found
//*****************************************************************

// If a module is specified for the symbol, only symbols in that module are
//   looked at. If no module is specified, all modules looked at but symbols
//   found in the current module (if any) are given precedence. If the symbol
//   is specified as being local, only local symbols are looked at. Otherwise
//   all symbols are looked at with global values given precedence. If multiple
//   matchs that cannot be resolved by precendence are found, an "Ambiguous
//   Symbol" error is indicated.

SYM *findsymbol(
	char *name,			// Symbol name
	SYM  *module)		// Restrict search to this module if not NULL
{
	SYMTBL *tpnt;
	union
	{	uchar *c;
		SYM   *s;
	}       spnt;
	SYM    *thismodule;
	SYM    *notlclsym;
	SYM    *modlclsym;
	SYM    *notgblsym;
	SYM    *modgblsym;
	int     len;
	uchar   notlclambig;
	uchar   modlclambig;
	uchar   notgblambig;

	thismodule = NULL;
	notlclsym = NULL;
	modlclsym = NULL;
	notgblsym = NULL;
	modgblsym = NULL;
	notlclambig = FALSE;
	modlclambig = FALSE;
	notgblambig = FALSE;
	len = strlen(name);
	if (module != NULL)
	{
		tpnt = NULL;
		spnt.s = module;
	}
	else
	{
		tpnt = &symtbl;
		spnt.s = symtbl.sym;
	}
	while (TRUE)
	{
		while (spnt.s->length != 0xFF)
		{
			if (spnt.s->flags & SF_MOD)
			{
				if (modgblsym != NULL || tpnt == NULL && spnt.s != module)
					break;
				thismodule = spnt.s;
			}
			else
			{
				if (len == spnt.s->length && strncmp(name, spnt.s->name,
						len) == 0)
				{
					if (spnt.s->flags & SF_GBL)
					{
						if (thismodule == module)
						{
							// Here with match on global symbol from the
							//   requested module

							if (modgblsym != NULL) // Already have one?
							{
								if (modgblsym->value != spnt.s->value)
								{
									symerror("is ambiguous", name, module);
									return (NULL);
								}
							}
							else
								modgblsym = spnt.s;
						}
						else
						{
							// Here with match on global symbol not from
							//   the requested module

							if (notgblsym != NULL) // Already have one?
							{
								if (notgblsym->value != spnt.s->value)
									notgblambig = TRUE;
							}
							else
								notgblsym = spnt.s;
						}
					}
					else
					{
						if (thismodule == module)
						{
							// Here with match on local symbol from the
							//   requested module

							if (modlclsym != NULL) // Already have one?
							{
								if (modlclsym->value != spnt.s->value)
									modlclambig = TRUE;
							}
							else
								modlclsym = spnt.s;
						}
						else
						{
							// Here with match on global symbol not from
							//   the requested module

							if (notlclsym != NULL) // Already have one?
							{
								if (notlclsym->value != spnt.s->value)
									notlclambig = TRUE;
							}
							else
								notlclsym = spnt.s;
						}
					}
				}
			}
			spnt.c += (offsetof(SYM, name) + spnt.s->length);
		}
	    if (tpnt == NULL || modgblsym != NULL || (tpnt = tpnt->next) == NULL)
			break;
		spnt.s = tpnt->sym;
	}
	if (modgblsym != NULL)
		return (modgblsym);
	if (notgblambig)
	{
		symerror("is ambiguous", name, module);
		return (NULL);
	}
	if (notgblsym != NULL)
		return (notgblsym);

	if (modlclambig)
	{
		symerror("is ambiguous", name, module);
		return (NULL);
	}
	if (modlclsym != NULL)
		return (modlclsym);

	if (notlclambig)
	{
		symerror("is ambiguous", name, module);
		return (NULL);
	}
	if (notlclsym != NULL)
		return (notlclsym);
	symerror("was not found", name, module);
	return (NULL);
}


static void symerror(
	char *msg,
	char *name,
	SYM  *module)
{
	char text[128];

	if (module != NULL)
	{
		text[0] = '#';
		strnmov(text + 1, module->name, module->length);
		text[module->length + 1] = 0;
	}
	else
		text[0] = 0;
	ensureleft();
	putstr("\1? Symbol %s%s %s\2\r\n", name, text, msg);
}


//***************************************************
// Function: findbestsym - Find the "best" symbol
//				to use when displaying a value
// Returned: Address of symbol table entry for symbol
//***************************************************

// If multiple exact matches are found, they are used as follows:
//	 1) Global defined in current module
//   2) Local defined in current module
//   3) Global defined in other module
//   4) Local defined in other module
// If no exact match is found, the symbol which gives the smallest offset
//   is returned provided the offset is less than 0x1000. If multiple symbols
//   are found with the same value, the priority show above is used except
//   that local symbols from other modules are never returned with an offset.
// If there are identical matches for more than one symbol, the first one
//   encountered is returned.

SYM *findbestsym(
	ulong  value,
	SYM  **module)
{
	SYMTBL *tpnt;
	union
	{	uchar *c;
		SYM   *s;
	}       spnt;

	SYM  *bestgblcursym;		// Global symbol defined in current module with
	ulong bestgblcurdiff;		//   best match

	SYM  *bestlclcursym;		// Local symbol defined in current module with
	ulong bestlclcurdiff;		//   best match

	SYM  *bestgblothsym;		// Global sysbol defined in other module with
	ulong bestgblothdiff;		//   best match
	SYM  *bestgblmod;

	SYM  *bestlclothsym;		// Local sysbol defined in other module with
	ulong bestlclothdiff;		//   best match
	SYM  *bestlclmod;

	SYM  *thismodule;
	ulong diff;
	uchar incurmod;

	bestgblcursym = NULL;
	bestgblcurdiff = 0xFFFFFFFF;
	bestlclcursym = NULL;
	bestlclcurdiff = 0xFFFFFFFF;

	bestgblothsym = NULL;
	bestgblothdiff = 0xFFFFFFFF;
	bestgblmod = NULL;
	bestlclothsym = NULL;
	bestlclothdiff = 0xFFFFFFFF;
	bestlclmod = NULL;

///	bestextmod = NULL;

	thismodule = NULL;
	tpnt = &symtbl;
	incurmod = FALSE;
	do
	{
		spnt.s = tpnt->sym;
		while (spnt.s->length != 0xFF)
		{
			if (spnt.s->flags & SF_MOD)
			{
				thismodule = spnt.s;
				incurmod = (spnt.s == curmodule);
			}
			else if ((spnt.s->flags & SF_SUP) == 0 &&
					(diff = value - (ulong)spnt.s->value) < 0x1000)
			{
				// Here with a symbol we might be able to use

				if (spnt.s->flags & SF_GBL) // Is this a global symbol?
				{
					if (incurmod)
					{
						if (diff == 0)	// Exact match for a global defined in
						{				//   in the current module?
							*module = curmodule;
							return (spnt.s); // Yes - return it
						}
						if (diff < bestgblcurdiff)
						{
							bestgblcurdiff = diff;
							bestgblcursym = spnt.s;
						}
					}
					else
					{
						if (diff < bestgblothdiff)
						{
							bestgblothdiff = diff;
							bestgblothsym = spnt.s;
							bestgblmod = thismodule;
						}
					}
				}
				else
				{
					if (incurmod)
					{
						if (diff < bestlclcurdiff)
						{
							bestlclcurdiff = diff;
							bestlclcursym = spnt.s;
						}
					}
					else
					{
						if (diff < bestlclothdiff)
						{
							bestlclothdiff = diff;
							bestlclothsym = spnt.s;
							bestlclmod = thismodule;
						}
					}
				}
			}
			spnt.c += (offsetof(SYM, name) + spnt.s->length);
		}
	} while ((tpnt = tpnt->next) != NULL);

	// Finished searching the symbol table - return the best we found

	if (bestlclcurdiff == 0)			// First look for exact matches
	{
		*module = curmodule;
		return (bestlclcursym);
	}
	if (bestgblothdiff == 0)
	{
		*module = bestgblmod;
		return (bestgblothsym);
	}
	if (bestlclothdiff == 0)
	{
		*module = bestlclmod;
		return (bestlclothsym);
	}

	// No exact match = we need to return the closest usable one we found.

	if (bestgblcurdiff <= bestlclcurdiff && bestgblcurdiff <= bestgblothdiff)
	{
		*module = curmodule;
		return (bestgblcursym);
	}
	if (bestlclcurdiff <= bestgblothdiff)
	{
		*module = curmodule;
		return (bestlclcursym);
	}
	*module = bestgblmod;
	return (bestgblothsym);
}


//*************************************************************
// Function: wildsearch - Find next matching symbol table entry
//				using wildcard search and/or value match
// Returned: Address of symbol table entry or NULL if no match
//*************************************************************

// Warning: This function uses static storage.

SYM *wildsearch(
	SYM  *sym,		// Address of next entry, NULL to start search
	char *name,		// Name to search, may contain wildcard characters
	long  value,	// Value to match
	int   flags,	// Bit 0 set for case sensitive search, bit 1 set to use
					//   name, bit 2 set to use value
	SYM **mod)		// Receives address of symbol entry for module
{
	static SYMTBL *tblpnt;

	SYM *thismod;
	char sname[SYMMAX + 4];

	if (sym == NULL)
	{
		tblpnt = &symtbl;
		sym = &symtbl.sym;
	}
	thismod = NULL;
	while (TRUE)
	{
		if (sym->length == 0xFF)			// End of table?
		{
			if ((tblpnt = tblpnt->next) == NULL)
				return (NULL);
			sym = tblpnt->sym;
		}

		if (sym->flags & SF_MOD)
			thismod = sym;
		else
		{
			while ((flags & 0x04) == 0 || value == sym->value)
			{
				if (flags & 0x02)
				{
					strncpy(sname, sym->name, sym->length);
					sname[sym->length] = 0;
					if (wildcmp(name, sname, flags & 0x01) != 0)
						break;
				}
				if (thismod != NULL)
					*mod = thismod;
				return (sym);
			}
		}
		*(uchar **)&sym += (offsetof(SYM, name) + sym->length);
	}
}
