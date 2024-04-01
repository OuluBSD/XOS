//*********************************************
// Functions to evaluate expressions for GECKOR
//*********************************************
// Written by John Goltz
//*********************************************

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

#define ALLOWWS 0

// Allocate local data

char *sympnt;					// Pointer to symbuf

#define REGNUM ((sizeof(regntbl))/5)

#define XERR_NA  0x00008000		// Value is not an address
#define XERR_X   0x00002000		// Invalid expression
#define XERR_D0  0x00000800		// Attempted to divide by 0
#define XERR_SYM 0x00000001		// Undefined symbol or module (message already
								//   displayed)
#define SYMMAXSZ 127
#define DIGMAXSZ 31

int  digcnt;
long errflag;

int  symsize;
char symbuf[SYMMAXSZ + 1];
char digbuf[DIGMAXSZ + 1];

static char getadg(char chr);
static int  getsym(char chr, char *bufr);
static void level1(VAL *value);
static void level2(VAL *value);
static void level3(VAL *value);
static void level4(VAL *value);
static void level5(VAL *value);
static void level6(VAL *value);
static void level7(VAL *value);
static void level8(VAL *value);
static void level9(VAL *value);
static void primary(VAL *value);


static int chksymbgn(char chr);



//***********************************************
// Function: lastnumeric - Require numberic value
//				to be at end of command line
// Returned: TRUE if OK, FALSE if error
//***********************************************

int lastnumeric(
	long *value)
{
	if (!numeric(value))
		return (FALSE);
	if (stopper != 0)
	{
		putstr("\r\n\1? Too many arguments\2\r\n");
		return (FALSE);
	}
	return (TRUE);
}


//****************************************************
// Function: numeric - Evaluate general expression and
//				return numeric value only
// Returned: TRUE if OK, FALSE if error
//****************************************************

int numeric(
	long *value)
{
	VAL val;

    if (!expression(&val))				// Evalulate expression
		return (FALSE);
	if (val.seg != 0)
	{
		putstr("\r\n\1? Value must be numeric\2\r\n");
		return (FALSE);
	}
	*value = val.val;
	return (TRUE);
}


//***************************************************
// Function: lastexpression - Require expression to
//					be a end of command line
// Returned: TRUE if OK, FALSE if error in expression
//***************************************************

int lastexpression(
	VAL *value)
{
	if (!expression(value))
		return (FALSE);
	if (stopper != 0)
	{
		putstr("\r\n\1? Too many arguments\2\r\n");
		return (FALSE);
	}
	return (TRUE);
}


//***************************************************
// Function: expression - Evaluate general expression
// Returned: TRUE if OK, FALSE if error in expression
//***************************************************

int expression(
	VAL *value)

{
	value->val = 0;
	value->seg = 0;
	errflag = 0;
	stopper = nxtnbc();
    level1(value);
	if (errflag == 0)
		return (TRUE);
	if (errflag & XERR_NA)
		putstr("\r\n\1? Value is not an address\2\r\n");
	else if (errflag & XERR_X)
		putstr("\r\n\1? Invalid expression\2\r\n");
	else if (errflag & XERR_D0)
		putstr("\r\n\1? Attempted to divide by 0\2\r\n");
	return (FALSE);
}


//***********************************
// Function: level1 - Process : and ;
// Returned: Nothing
//***********************************

static void level1(
	VAL *value)
{
	int   sel;
	uchar rma;

    level2(value);						// Evaluate left operand
    if (stopper == ':' || stopper == ';')
    {
		if (value->seg != 0 || (value->val & 0xFFFF0000) != 0)
		{
        	errflag |= XERR_X;
			return;
		}
		sel = value->val;
		rma = (stopper == ';');
#if ALLOWWS
    	stopper = nxtnbc();
#else
	    stopper = nxtchar();
#endif
		value->val = 0;
		value->seg = 0;
		level2(value);					// Evaluate right operand
		if (rma)
		{
			value->seg = 0xFFFF;
			if ((value->val & 0xFFFF0000) != 0)
			{
    	    	errflag |= XERR_X;
				return;
			}
			value->val += (sel << 16);
		}
		else
			value->seg = (ushort)sel;

	}
}


//*****************************
// Function: level2 - Process |
// Returned: Nothing
//*****************************

static void level2(
	VAL *value)
{
	VAL rval;

    level3(value);						// Evaluate left operand
    while (stopper == '|')
    {
#if ALLOWWS
	    stopper = nxtnbc();
#else
    	stopper = nxtchar();
#endif
		rval.val = 0;
		rval.seg = 0;
		level2(&rval);					// Evaluate right operand
		value->val |= rval.val;			// Do inclusive or
    }
}


//*****************************
// Function: level3 - Process ^
// Returned: Nothing
//*****************************

static void level3(
	VAL *value)
{
	VAL rval;

    level4(value);						// Evaluate left operand
    while (stopper == '^')
    {
#if ALLOWWS
    	stopper = nxtnbc();
#else
	    stopper = nxtchar();
#endif
		rval.val = 0;
		rval.seg = 0;
		level4(&rval);					// Evaluate right operand
		value->val ^= rval.val;			// Do exclusive or
    }
}


//*****************************
// Function: level4 - Process &
// Returned: Nothing
//*****************************

static void level4(
	VAL *value)
{
	VAL rval;

    level5(value);						// Evaluate left operand
    while (stopper == '&')
    {
#if ALLOWWS
	    stopper = nxtnbc();
#else
    	stopper = nxtchar();
#endif
		rval.val = 0;
		rval.seg = 0;
		level5(&rval);					// Evaluate right operand
		value->val &= rval.val;			// Do and
    }
}

//***********************************
// Function: level5 - Process > and <
// Returned: Nothing
//***********************************

static void level5(
	VAL *value)

{
	VAL  rval;
    char chr;

    level6(value);						// Evaluate left operand
    while (stopper == '<' || stopper == '>')
    {
		chr = stopper;
#if ALLOWWS
	    stopper = nxtnbc();
#else
    	stopper = nxtchar();
#endif
		rval.val = 0;
		rval.seg = 0;
		level6(&rval);					// Evaluate right operand
		if (chr == '<')
			value->val <<= rval.val;	// If left shift
		else
			value->val = ((unsigned long)value->val) >> rval.val;
    }
}


//***********************************
// Function: level6 - Process + and -
// Returned: Nothing
//***********************************

static void level6(
	VAL *value)
{
    VAL  rval;
	char chr;

    level7(value);						// Evaluate left operand
    while (stopper == '+' || stopper == '-')
    {
		chr = stopper;
#if ALLOWWS
	    stopper = nxtnbc();
#else
    	stopper = nxtchar();
#endif
		rval.val = 0;
		rval.seg = 0;
		level7(&rval);					// Evaluate right operand
		if (chr == '+')					// If add
			value->val += rval.val;
		else							// If subtract
			value->val -= rval.val;
	}
}


//***********************************
// Function: level7 - Process * and /
// Returned: Nothing
//***********************************

static void level7(
	VAL *value)
{
	VAL  rval;
	char chr;

    level8(value);						// Evaluate left operand
    while (stopper == '*' || stopper == '/')
    {
		chr = stopper;
#if ALLOWWS
	    stopper = nxtnbc();
#else
    	stopper = nxtchar();
#endif
		rval.val = 0;
		rval.seg = 0;
		level8(&rval);					// Evaluate right operand
		if (chr == '*')
			value->val *= rval.val;		// If multiply
		else
		{
			if (rval.val == 0)			// If divide
		        errflag |= XERR_D0;
			else
				value->val /= rval.val;
		}
    }
}


//**********************************
// Function: Process unary operators
// Returned: Nothing
//**********************************

static void level8(
	VAL *value)
{
    char chr;

    if (stopper == '~' || stopper == '-')
    {
		chr = stopper;
#if ALLOWWS
	    stopper = nxtnbc();
#else
    	stopper = nxtchar();
#endif
		level8(value);					// Evaluate operand
		if (chr == '~')
			value->val = ~value->val;
		else
			value->val = -value->val;
    }
    else
		level9(value);
}


//**********************************
// Function: level9 - Process braces
// Returned: Nothing
//**********************************

static void level9(
	VAL *value)
{
    if (stopper == '(')					// Start of sub-expression?
    {
#if ALLOWWS
	    stopper = nxtnbc();
#else
    	stopper = nxtchar();
#endif
		level2(value);					// Evaluate sub-expression
		if (stopper == ')')				// Must end with )
#if ALLOWWS
		    stopper = nxtnbc();
#else
    		stopper = nxtchar();
#endif
		else
	        errflag |= XERR_X;
    }
    else
    {
		primary(value);					// Get value of atom
#if ALLOWWS
		stopper = nxtnb0(stopper);
#endif
    }
}

//************************************
// Function: primary - Fetch next atom
// Returned: Nothing
//************************************

static void primary(
	VAL *value)
{
	REGNTBL *rtpnt;
    char    *pnt;
	SYM     *sym;
	SYM     *symmod;
	long     digval;
    int      cnt;
	int      tradix;
	char     modbuf[SYMMAXSZ + 1];
    char     chr;

    chr = stopper;						// Get first character
    if (chr == '\'')					// Single quote?
    {
        cnt = 0;
        value->val = 0;
		value->seg = 0;
        while ((chr=nxtchar()) != '\0' &&
                (chr != '\'' || (chr=nxtchar()) == '\''))
        {
            value->val |= ((long)chr << cnt);
            cnt += 8;
        }
#if ALLOWWS
	    stopper = nxtnbc();
#else
    	stopper = nxtchar();
#endif
		return;
    }
    if (chr == '"')						// Double quote?
    {
		((uchar *)&value->val)[1] = nxtchar() & 0x7F; // Yes
		((uchar *)&value->val)[0] = nxtchar() & 0x7F;
#if ALLOWWS
	    stopper = nxtnbc();
#else
    	stopper = nxtchar();
#endif
		return;
    }
    if (isdigit(chr))					// Is it a digit?
    {
		tradix = curmode.iradix;
		if (chr == '0')
		{
			if ((chr = toupper(nxtchar())) == 'X')
			{
				tradix = 16;
				goto numcommon;
			}
			else if (chr == 'Q' || chr == 'O')
			{
				tradix = 8;
				goto numcommon;
			}
			else if (chr == 'T')
			{
				tradix = 10;
			 numcommon:
				chr = nxtchar();
				if (!isxdigit(chr))
				{
					errflag |= XERR_X;
					value->val = 0;
					value->seg = 0;
					return;
				}
			}
			else
			{
				hldchar = chr;			// Need to put it back in case single
				chr = '0';				//   digit 0 value
			}
		}
		stopper = getadg(chr);			// Collect digits
		pnt = digbuf;
		digval = 0;
		while (--digcnt >= 0)
			digval = (digval * tradix) + *pnt++;
		value->val = digval;
		return;
    }
    if ((symsize = getsym(chr, symbuf)) == 0) // Try to collect symbol
    {
		value->val = 0;					// Not symbol - return 0
		value->seg = 0;
		return;
    }
    if (symsize == 1 && symbuf[0] == '$') // Is this the current addr?
    {
		*value = curaddr;
		return;
    }
	if (symsize >= 1 && symsize <= 3)	// Might this be a register name?
	{
		rtpnt= regntbl;
		cnt = 34;
		do
		{
			if (stricmp(rtpnt->name, symbuf) == 0)
			{
				if (rtpnt->size == 4)
					value->val = *(ulong *)(((uchar *)&regtbl) + rtpnt->offset);
				else if (rtpnt->size == 2)
					value->val = *(ushort *)(((uchar *)&regtbl) +
							rtpnt->offset);
				else
					value->val = *(uchar *)(((uchar *)&regtbl) + rtpnt->offset);
				value->seg = 0;
				return;
			}
			rtpnt++;
		} while (--cnt > 0);
	}

	// Here to look for a normal symbol. The following formats are allowed:
	//	 symbol			Search for any match in all modules. Global match has
	//					  presedance over single or multiple local matches.
	//	 symbol#		Search for any match in current module only. If there
	//					  is no current module, all modules are searched.
	//   symbol#module	Search for any match in specified module only.
	// A symbol should only be found to be ambiguous when searching all
	//   modules
	// Symbols of the form $ followed by all digits are assumed to be followed
	//   by #, effectively they are always treated as local symbols.

	symmod = NULL;						// Assume want to search all modules
	if (symbuf[0] == '$')				// Is this a compiler line number
	{									//   symbol?
		pnt = symbuf + 1;
		while ((chr = *pnt++) != 0 && isdigit(chr))
			;
		if (chr == 0)
			symmod = curmodule;			// Yes - just search current module
	}
    if (stopper == '#')					// Was symbol stopped by #?
    {									// Yes
#if ALLOWWS
	    stopper = nxtnbc();
#else
	    stopper = nxtchar();
#endif
		if (getsym(stopper, modbuf) != 0) // Did he specify a module name?
		{
			if ((symmod = findmodule(modbuf)) == NULL)
			{
				errflag |= XERR_SYM;
				return;
			}
		}
		else							// No name given after # - search the
			symmod = curmodule;			//   current module
	}
    if ((sym = findsymbol(symbuf, symmod)) == NULL)
	{									// Search the symbol table
		errflag |= XERR_SYM;			// If not found (error message already
		return;							//   displayed
	}
	value->val = sym->value;			// Found it
	value->seg = 0;
}


//**********************************
// Function: getadg - Collect digits
// Returned: Stopper character
//**********************************

static char getadg(
    char chr)
{
    char *pnt;
    int   cnt;

    pnt = digbuf;
    cnt = 0;
    do
    {
		if (cnt < DIGMAXSZ)
		{
			if (chr >= 'A')				// Adjust value between A and F
				chr += 9;
			*pnt++ = chr & 0xF;			// Store value in digit buffer
			++cnt;
		}
		else
	        errflag |= XERR_X;
		chr = nxtchar();				// Get next character
    } while (isxdigit(chr));
    digcnt = cnt;
    return (chr);
}


//***************************************************************
// Function: getsym - Collect symbol in symbuf (use this function
//				if already have first character of symbol
// Returned: TRUE if symbol collected, FALSE if not a symbol
//***************************************************************

static int getsym(
    char  chr,
	char *bufr)

{
	int len;

///#if ALLOWWS
///    chr = nxtnbc();
///#else
///    chr = nxtchar();
///#endif


    if (!chksymbgn(stopper))			// Can this start a symbol?
	{
		bufr[0] = 0;
		return (0);
	}
    len = 1;
	*bufr++ = stopper;
    while (isdigit(chr = nxtchar()) || chksymbgn(chr))
    {   if (symsize < SYMMAXSZ)			// Is symbol full now?
		{
			++len;						// No
			*bufr++ = chr;				// Store character in symbol
		}
    }
	*bufr = 0;
    stopper = chr;						// Store stopper
#if ALLOWWS
	stopper = nxtnb0(stopper);
#endif
    return (len);						// Indicate have symbol
}


//****************************************************************
// Function: chksymbgn - Determine if a character can be the first
//	character in a symbol
// Returned: TRUE if character can start a symbol, FALSE if not
//****************************************************************

static int chksymbgn(
    char chr)

{
    return (isalpha(chr) || chr == '%' || chr == '_' || chr == '$' ||
			chr == '@' || chr == '?');
}
