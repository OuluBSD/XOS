//*******************************************************
// Routines to read single file for second pass for XLINK                                     */
//*******************************************************
// Written by John Goltz
//*******************************************************

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcmalloc.h>
#include <ctype.h>
#include <procarg.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserrmsg.h>
#include <xosstr.h>
#include "xlink.h"

long symval;
MD  *symmsd;
PD  *sympsd;
SY  *sympnt;
char symabs;
char junked;

static char badoffset[] = "MS fix-up offset is too large";

static void ms2mcomdat(void);
static void ms2mcomdef(void);
static void ms2mcoment(void);
static void ms2mfixupp(void);
static void ms2mlxdata(void);
static void ms2mlinnum(void);
static void ms2mlocsym(void);
static void ms2mmodend(void);
static void ms2mpubdef(void);
static void ms2msegdef(void);
static void ms2mtheadr(void);
static void ms2mtypdef(void);
static void twodata(void);
static void twodebug(void);
static void twoeom(void);
static void twoimport(void);
static void twointern(void);
static void twolocal(void);
static void twopsect(void);
static void twostack(void);
static void twostart(void);
static void twostradr(void);

static void msfixsel(char boffset, SY *sym, SD *seg);


//****************************
// Section type dispatch table
//****************************

#define skipit (void (*)(void))nullfunc


void (*dsptwo[])(void) =
{
    twoeom,				// SC_EOM     = 0  End of module
	skipit,				// SC_INDEX   = 1  Library index section
    twostart,			// SC_START   = 2  Start of module section
	skipit,				// SC_SEG     = 3  Segment definition section
	skipit,				// SC_MSECT   = 4  Msect definition section
    twopsect,			// SC_PSECT   = 5  Psect definition section
    twodata,			// SC_DATA    = 6  Data section
    twointern,			// SC_INTERN  = 7  Internal symbol definition section
	skipit,				// SC_LOCAL   = 8  Local symbol definition section
    twointern,			// SC_EXPORT  = 9  Exported symbol definition section
    twostradr,			// SC_STRADR  = 10 Starting address section
    twodebug,			// SC_DEBUG   = 11 Debugger address section
    twostack,			// SC_STACK   = 12 Initial stack address section
	skipit,				// SC_FILEREQ = 13 File request section
	skipit,				// SC_EXTERN  = 14 External symbol definition section
    badsec,				// SC_SHARED  = 15 Shared library definition section
	skipit,				// SC_IMPORT  = 16 Imported symbol definition section
    badsec,				//            = 17 Illegal
    badsec,				//            = 17 Illegal
    badsec,				//            = 19 Illegal
    twoeom,				// SC_ENTRY   = 20 Entry list section
    badsec,				// SC_BAD     = 21 Illegal MS section type
    ms2mtheadr,			// SC_MTHEADR = 22 (0x80) Module header
    ms2mcoment,			// SC_MCOMENT = 23 (0x88) Comment
    ms2mmodend,			// SC_MMODEND = 24 (0x8A, 0x8B) Module end
	skipit,				// SC_MEXTDEF = 25 (0x8C) External definition
    ms2mtypdef,			// SC_MTYPDEF = 26 (0x8E) Type definition
    ms2mpubdef,			// SC_MPUBDEF = 27 (0x90, 0x91) Public definition
    ms2mlocsym,			// SC_MLOCSYM = 28 (0x92, 0x93) Local symbols
	skipit,				// SC_MLINNUM = 29 (0x94, 0x95) Source line number
	skipit,				// SC_MLNAMES = 30 (0x96) Name list
    ms2msegdef,			// SC_MSEGDEF = 31 (0x98, 0x99) Segment definition
	skipit,				// SC_MGRPDEF = 32 (0x9A) Group definition
    ms2mfixupp,			// SC_MFIXUPP = 33 (0x9C, 0x9D) Fix up previous data
    ms2mlxdata,			// SC_MLEDATA = 34 (0xA0, 0xA1) Logical data
    ms2mlxdata,			// SC_MLIDATA = 35 (0xA2, 0xA3) Logical repeated data
    ms2mcomdef,			// SC_MCOMDEF = 36 (0xB0) Communal names definition
	skipit,				// SC_MLOCEXD = 37 (0xB4) Local external definition
    skipit,				// SC_MFLCEXD = 38 (0xB5) Local external definition
    ms2mpubdef,			// SC_MLOCPUB = 39 (0xB6, 0xB7) Local public definition
    ms2mcomdef,			// SC_MLOCCOM = 40 (0xB8) Local communal name
    ms2mcomdat			// SC_MCOMDAT = 41 (0xC2, 0xC3) Initialized communal
};						//                        data


//********************************
// Polish item type dispatch table
//********************************

void (*twoddsp[])(int) =
{
    polopr,					// 10000xxx - Arithmetic operators
    polopr,					// 10001xxx - Arithmetic operators
    polopr,					// 10010xxx - Store operators
    polopr,					// 10011xxx - Store operators
    polpav,					// 10100xxx - Push absolute value
    polpar,					// 10101xxx - Push relative absolute value
    polrac,					// 10110xxx - Push offset value for current psect
    (void (*)(int))badfmt,	// 10111xxx - Illegal
    polraa,					// 11000xxx - Push offset value for any psect
    polrra,					// 11001xxx - Push offset value, relative for any
							//              psect
    (void (*)(int))badfmt,	// 11010xxx - Illegal
    (void (*)(int))badfmt,	// 11011xxx - Illegal
    (void (*)(int))badfmt,	// 11100xxx - Illegal
    (void (*)(int))badfmt,	// 11101xxx - Illegal
    polexan,				// 11110xxx - Push symbol offset value
    polexrn					// 11111xxx - Push symbol offset value, relative
};

//****************************************************
// Polish arithmetic and store operator dispatch table
//****************************************************

void (*oprdsp[]) (void) =
{	athadd,			// 10000000 - Add
    athsub,			// 10000001 - Subtract
    athmul,			// 10000010 - Multiply
    athdiv,			// 10000011 - Divide
    athsft,			// 10000100 - Shift
    athand,			// 10000101 - And
    athior,			// 10000110 - Inclusive or
    athxor,			// 10000111 - Exclusive or
    athcom,			// 10001000 - Complement
    athneg,			// 10001001 - Negate
    badfmt,			// 10001010 - Illegal
    badfmt,			// 10001011 - Illegal
    badfmt,			// 10001100 - Illegal
    badfmt,			// 10001101 - Illegal
    badfmt,			// 10001110 - Illegal
    badfmt,			// 10001111 - Illegal
    badfmt,			// 10010000 - Illegal
    badfmt,			// 10010001 - Illegal
    polsscp,		// 10010010 - Push segment selector for current psect
    polssap,		// 10010011 - Push segment selector for any psect,
					//              psect number follows
    polssam,		// 10010100 - Push segment selector for any msect,
					//              msect number follows
    polssas,		// 10010101 - Push segment selector for any sesgment,
					//              segment number follows
    polosam,		// 10010110 - Push offset for any msect, msect number
					//              follows
    polssexn,		// 10010111 - Push selector value for symbol
    polstraddrw,	// 10011000 - Store word address
    polstraddrl,	// 10011001 - Store long address
    polstrub,		// 10011010 - Store unsigned byte
    polstruw,		// 10011011 - Store unsigned word
    polstrl,		// 10011100 - Store unsigned long
    polstrsb,		// 10011101 - Store signed byte
    polstrsw,		// 10011110 - Store signed word
    polstrl			// 10011111 - Store signed long
};

#define PO_ADD        0x80	// Add
#define PO_SUB        0x81	// Subtract
#define PO_MUL        0x82	// Multiply
#define PO_DIV        0x83	// Divide
#define PO_SHF        0x84	// Shift
#define PO_AND        0x85	// And
#define PO_IOR        0x86	// Inclusive or
#define PO_XOR        0x87	// Exclusive or
#define PO_COM        0x88	// 1s complement
#define PO_NEG        0x89	// Negate (2s complement)
#define PO_PSELCP     0x92	// Push selector, current psect
#define PO_PSELAP     0x93	// Push selector, any psect
#define PO_PSELAM     0x94	// Push selector, any msect
#define PO_PSELAS     0x95	// Push selector, any segment
#define PO_POFSAM     0x96	// Push offset, any msect
#define PO_PSELSYM    0x97	// Push selector, external symbol
#define PO_STRAW      0x98	// Store word address
#define PO_STRAL      0x99	// Store long address


#define PO_STRAQ      0x00	// Store quad address (SOON!)

#define PO_STRUB      0x9A	// Store unsigned byte
#define PO_STRUW      0x9B	// Store unsigned word
#define PO_STRUL      0x9C	// Store unsigned long
#define PO_STRSB      0x9D	// Store signed byte
#define PO_STRSW      0x9E	// Store signed word
#define PO_STRSL      0x9F	// Store signed long
#define PO_STRQ       0x00	// Atore quad (SOON!)
#define PO_PVAL       0xA0	// Push absolute value
#define PO_PVALREL    0xA8	// Push absolute value, relative
#define PO_POFSCP     0xB0	// Push relocated value, current psect
#define PO_POFSAP     0xC0	// Push relocated value, any psect
#define PO_POFSRELAP  0xC8	// Push relocated value, relative, any psect
#define PO_POFSSYM    0xF0	// Push symbol offset value
#define PO_POFSRELSYM 0xF8	// Push symbol offset value, relative


//********************************************************
// Function: filetwo - Process single file for second pass
// Returned: Nothing
//********************************************************

void filetwo(
    OB *obj)					// Pointer to OBJ block

{
#if CFG_XLINK_DEBUG
	fprintf( stdout, "->filetwo(obj) obj_mdl=%08X obj_fsp=[%.40s]\n"
				, (int)obj->obj_mdl, obj->obj_fsp );
#endif

	if (obj->obj_mdl == NULL)			// Nothing to do if this is a
		return;				      		//   library file we don't need

	opnobj(obj);						// Open the OBJ file
	if (!libflag)						// Library file?
	{									// No - just one module
		curmod = obj->obj_mdl;
		moduletwo();
	}
    else
    {
		if ((curmod = obj->obj_mdl) != NULL) // Yes - scan module list
		{
			do
			{
				setblk(curmod->mb_mod);	// Setup to read module
				moduletwo();			// Load the module
			} while ((curmod = curmod->mb_next) != NULL );
		}
	}
	closeobj();							// Close the input file
}


//********************************************************************
// Function: moduletwo - Process single program module for second pass
// Returned: Nothing
//********************************************************************

void moduletwo(void)

{
    setobj();
    psnumpnt = curmod->mb_psectnumtbl;
    psnumcnt = 0;
    msnumpnt = curmod->mb_msectnumtbl;
    msnumcnt = 0;
    segnumpnt = curmod->mb_segnumtbl;
    segnumcnt = 0;
    done = FALSE;
    do
    {
        getsec();
        if (debugflag)
            printf("p2: mod=0x%02.2X, size=%d\n", sectype, seccnt);
        (*dsptwo[sectype])();		// Dispatch on section type
    } while (!done);
}


//*************************************************
// Function: twoeom - Process end of module section
// Returned: Nothing
//*************************************************

static void twoeom(void)

{
/*
    SD *sgd;
    MD *msd;
    PD *psd;

    if ((sgd=sgdhead) != NULL)		// Scan all segments (if any)
    {
        do
        {   if ((msd=sgd->sd_head) != NULL) // Scan all msects (if any)
            {
                do
                {   if ((psd=msd->md_head) != NULL)
                    {				// Scan all psects (if any)
                        do
						{
							if (!(psd->pd_flag & PA_OVER)) // Overlayed?
                                psd->pd_ofs = psd->pd_nxo; // No - update
                        } while ((psd=psd->pd_next) != NULL); //   offset
                    }
                } while ((msd=msd->md_next) != NULL);
            }
        } while ((sgd=sgd->sd_next) != NULL);
    }
*/
    done = TRUE;						// Indicate finished with file
    modname[0] = 0;						// No current module now
}


//*****************************************************
// Function: twostart - Process start of module section
// Returned: Nothing
//*****************************************************

static void twostart(void)

{
    getsym();							// Get module name
    strmov(modname, symbuf+1);			// Save module name
}


//******************************************************
// Function: twopsect - Process psect definition section
// Returned: Nothing
//******************************************************

static void twopsect(void)

{
    PD  *psd;
	int  hbyte;

    while (seccnt > 0)
    {
        if (++psnumcnt > curmod->mb_psectnummax)
            fail("More psects encountered in pass 2 than in pass 1");
        psd = *psnumpnt++;
  
		getitem(getbyte());				// Discard size

///        if (!(psd->pd_flag & PA_OVER))
///            psd->pd_nxo = psd->pd_ofs + size;
										// Calculate offset for next file

        getitem(getbyte());				// Discard loaded size of psect
		hbyte = getbyte();
        getitem(hbyte);					// Discard address
		getitem(hbyte >> 4);			// Discard modulus
        getbyte();						// Discard msect number
        getsym();						// Get name of psect
    }
}


//*****************************************
// Function: twodata - Process data section
// Returned: Nothing
//*****************************************

static void twodata(void)

{
    int header;

    polspnt = polstk + POLSSIZE;		// Reset polish stack pointer
    while (seccnt != 0)
    {
		if ((header=getbyte()) & 0x80)	// Get item header byte

			(*twoddsp[(header>>3)&0x0F])(header);// If polish item
		else
		{
			if (header >= 0x78)			// Is this an address?
			{
				offset = getitem(header); // Yes - get address
				curpsd = getpsd(getbyte());
                curmsd = curpsd->pd_msd;
                cursgd = curmsd->md_sgd;
				offset += curpsd->pd_offset; // Get offset in RUN file
			}
			else
			{
				while (--header >= 0)	// Load absolute data
					putbyte(getbyte());
			}
		}
    }
    if (polspnt != polstk + POLSSIZE)
        fail("Invalid expression");
}


//*****************************************************************
// Function: polopr - Process polish arithmetic and store operators
// Returned: Nothing
//*****************************************************************

void polopr(
    int header)

{
    (*oprdsp[header&0x1F])();			// Dispatch to arithmetic routine
}


//***********************************************
// Function: athadd - Process add polish operator
// Returned: Nothing
//***********************************************

void athadd(void)

{
    int  type1;
    long value1;
    MD  *pnt1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;		// Get first item off of the stack
        value1 = polspnt->ps_value;
        pnt1 = polspnt->ps_pnt.m;
        ++polspnt;						// Pop the stack pointer
        if (type1 != PT_ABSOLUTE && polspnt->ps_type != PT_ABSOLUTE)
            badrexp();					// At least one must be absolute
        if (type1 != PT_ABSOLUTE)		// Use right type
        {
            polspnt->ps_type = type1;
            polspnt->ps_pnt.m = pnt1;
        }
        polspnt->ps_value += value1;	// Do the add
    }
}


//****************************************************
// Function: athsub - Process subtract polish operator
// Returned: Nothing
//****************************************************

void athsub(void)

{
    int  type1;
    long value1;
    MD  *pnt1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;		// Get first item off of the stack
        value1 = polspnt->ps_value;
        pnt1 = polspnt->ps_pnt.m;
        ++polspnt;			// Pop the stack pointer
        if (polspnt->ps_type != PT_ABSOLUTE && (polspnt->ps_type != type1 ||
                polspnt->ps_pnt.m != pnt1))
            badrexp();
        if (polspnt->ps_type == PT_ABSOLUTE)
        {
            polspnt->ps_type = type1;
            polspnt->ps_pnt.m = pnt1;
        }
        else
            polspnt->ps_type = PT_ABSOLUTE;
        polspnt->ps_value = value1 - polspnt->ps_value;
    }
}


//****************************************************
// Function: athmul - Process multiply polish operator
// Returned: Nothing
//****************************************************

void athmul(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;		// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;						// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();					// Both must be absolute
        polspnt->ps_value *= value1;	// Do the multiply
    }
}


//**************************************************
// Function: athdiv - Process divide polish operator
// Returned: Nothing
//**************************************************

void athdiv(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;		// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;						// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();					// Both must be absolute
        polspnt->ps_value = value1 / polspnt->ps_value;
    }
}


//*************************************************
// Function: athsft - Process shift polish operator
// Returned: Nothing
//*************************************************

void athsft(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;		// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;						// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();					// Both must be absolute
        if (value1 >= 0)
            polspnt->ps_value <<= value1; // Shift left
        else
            polspnt->ps_value >>= (-value1); // Shift right
    }
}


//***********************************************
// Function: athand - Process and polish operator
// Returned: Nothing
//***********************************************

void athand(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// Both must be absolute
        polspnt->ps_value &= value1;	// Do the and
    }
}


//********************************************************
// Function: athior - Process inclusive or polish operator
// Returned: Nothing
//********************************************************

void athior(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// Both must be absolute
        polspnt->ps_value |= value1;	// Do the inclusive or
    }
}


//********************************************************
// Function: athxor - Process exclusive or polish operator
// Returned: Nothing
//********************************************************

void athxor(void)

{
    int  type1;
    long value1;

    if (polspnt >= (polstk + (POLSSIZE-1))) // Check for stack underflow
        underflow();
    else
    {
        type1 = polspnt->ps_type;	// Get first item off of the stack
        value1 = polspnt->ps_value;
        ++polspnt;			// Pop the stack pointer
        if (type1 != PT_ABSOLUTE || polspnt->ps_type != PT_ABSOLUTE)
            badrexp();			// Both must be absolute
        polspnt->ps_value ^= value1;	// Do the exclusive or
    }
}


//******************************************************
// Function: athcom - Process complement polish operator
// Returned: Nothing
//******************************************************

void athcom(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type != PT_ABSOLUTE) // Must be absolute
            badrexp();
        polspnt->ps_value = ~polspnt->ps_value; // Do the complement
    }
}


//**************************************************
// Function: athneg - Process negate polish operator
// Returned: Nothing
//**************************************************

void athneg(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type != PT_ABSOLUTE) // Must be absolute
            badrexp();
        polspnt->ps_value = -polspnt->ps_value; // Do the negate
    }
}


//***************************************************************
// Function: posssexn - Process push selector for external symbol
// Returned: Nothing
//***************************************************************

void polssexn(void)

{
    symnum();							// Get symbol number
    if (symmsd == NULL)
    {
        if (sympnt->sy_flag & SYF_IMPORT)
		{
///			printf("### sym sel: %s\n", sympnt->sy_name);

            polpush(0, 0, PT_SELSYM, sympnt);
		}
        else
        {
            if (sympnt->sy_sel == 0 && !(sympnt->sy_flag & SYF_UNDEF))
                nosymseg();				// Complain if no segment defined
            polpush((ulong)(sympnt->sy_sel), 0, PT_ABSOLUTE, NULL);
        }
    }
    else
        segpush(symmsd->md_sgd);
}


//********************************************************************
// Function: polsscp - Process push segment selector for current psect
// Returned: Nothing
//********************************************************************

void polsscp(void)

{
    segpush(cursgd);
}


//****************************************************************
// Function: polssap - Process push segment selector for any psect
// Returned: Nothing
//****************************************************************

void polssap(void)

{
    segpush(getpsd(getbyte())->pd_msd->md_sgd);
}


//*****************************************************************
// Function: polsssam - Process push segment selector for any msect
// Returned: Nothing
//*****************************************************************

void polssam(void)

{
    segpush(getmsd(getbyte())->md_sgd);
}


//******************************************************************
// Function: polssas - Process push segment selector for any segment
// Returned: Nothing
//******************************************************************

void polssas(void)

{
    segpush(getsgd(getbyte()));
}


//******************************************************
// Function: polosam - Process push offset for any msect
// Returned: Nothing
//******************************************************

void polosam(void)

{
    MD *msd;

    msd = getmsd(getbyte());
    if (msd->md_flag & MA_ADDR)		// Address specified for this msect?
        polpush(msd->md_addr, msd->md_sgd->sd_select, PT_ABSOLUTE, NULL); // Yes
    else
        polpush(0, 0, PT_ABSOFSMS, msd); // No
}


//************************************************************
// Function: polstraddrw - Process store word address operator
// Returned: Nothing
//************************************************************

void polstraddrw(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE) // Check for truncation if should
        {
            if (polspnt->ps_value & 0xFFFF0000L)
                truncate(16);
            putword(polspnt->ps_value);
            putword(polspnt->ps_select);
        }
        else
        {
            if (polspnt->ps_type == PT_ABSOFSMS)
            {
                if (polspnt->ps_pnt.m->md_flag & MA_ADDR)
                {
                    polspnt->ps_type = PT_SELMS;
                    polspnt->ps_pnt.s = polspnt->ps_pnt.m->md_sgd;
                    offset += 2;
                    putreld(polspnt, 16);
                    offset -= 2;
                }
                else
                {
                    polspnt->ps_type = PT_ADDRMS;
                    putreld(polspnt, 16);
                }
            }
            else if (polspnt->ps_type == PT_ABSOFSSYM)
            {
                polspnt->ps_type = PT_ADDRSYM;
                putreld(polspnt, 16);
            }
            else
                badfmt();
            putword(polspnt->ps_value);
            putword(0);
        }
        ++polspnt;
    }
}


//************************************************************
// Function: polstraddrl - Process store long address operator
// Returned: Nothing
//************************************************************

void polstraddrl(void)

{
	if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
		underflow();
	else
	{
		if (polspnt->ps_type == PT_ABSOLUTE)
		{
			putlong(polspnt->ps_value);
			putword(polspnt->ps_select);
		}
		else
		{
			if (polspnt->ps_type == PT_ABSOFSMS)
			{
				if (polspnt->ps_pnt.m->md_flag & MA_ADDR)
				{
					polspnt->ps_type = PT_SELMS;
					polspnt->ps_pnt.s = polspnt->ps_pnt.m->md_sgd;
					offset += 4;
					putreld(polspnt, 16);
					offset -= 4;
				}
				else
				{
                    polspnt->ps_type = PT_ADDRMS;
                    putreld(polspnt, 32);
                }
            }
            else if (polspnt->ps_type == PT_ABSOFSSYM)
            {
                polspnt->ps_type = PT_ADDRSYM;
                putreld(polspnt, 32);
            }
            else
                badfmt();
            putlong(polspnt->ps_value);
            putword(0);
        }
        ++polspnt;
    }
}


//*********************************************************
// Function: polstub - Process store unsigned byte operator
// Returned: Nothing
//*********************************************************

void polstrub(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE || // Check for truncation if should
                (polspnt->ps_type == PT_ABSOFSMS &&
                (polspnt->ps_pnt.m->md_flag & MA_ADDR)))
        {
            if ((polspnt->ps_value & 0xFFFFFF00L) &&
                    (polspnt->ps_value & 0xFFFFFF80L) != 0xFFFFFF80L)
                truncate(8);
        }
        else
            putreld(polspnt, 8);
        putbyte(polspnt->ps_value);		// Store byte
        ++polspnt;
    }
}


//*********************************************************
// Function: polstuw - Process store unsigned word operator
// Returned: Nothing
//*********************************************************

void polstruw(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE || // Check for truncation if should
                (polspnt->ps_type == PT_ABSOFSMS &&
                (polspnt->ps_pnt.m->md_flag & MA_ADDR)))
        {
            if ((polspnt->ps_value & 0xFFFF0000L) &&
                    (polspnt->ps_value & 0xFFFF8000L) != 0xFFFF8000L)
                truncate(16);
        }
        else
            putreld(polspnt, 16);
        putword(polspnt->ps_value);		// Store word
        ++polspnt;
    }
}


//*******************************************************
// Function: polstsb - Process store signed byte operator
// Returned: Nothing
//*******************************************************

void polstrsb(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE ||
                (polspnt->ps_type == PT_ABSOFSMS &&
                (polspnt->ps_pnt.m->md_flag & MA_ADDR)))
        {
            if (polspnt->ps_value & 0x80) // Check for truncation
            {
                if ((polspnt->ps_value & 0xFFFFFF00L) != 0xFFFFFF00L)
                    truncate(8);
            }
            else
            {
                if (polspnt->ps_value & 0xFFFFFF00L)
                    truncate(8);
            }
        }
        else
            putreld(polspnt, 8);
        putbyte(polspnt->ps_value);		// Store byte
        ++polspnt;
    }
}

//*******************************************************
// Function: polstsw - Process store signed word operator
// Returned: Nothing
//*******************************************************

void polstrsw(void)

{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
        if (polspnt->ps_type == PT_ABSOLUTE ||
                (polspnt->ps_type == PT_ABSOFSMS &&
                (polspnt->ps_pnt.m->md_flag & MA_ADDR)))
        {
            if (polspnt->ps_value & 0x8000) // Check for truncation
            {
                if ((polspnt->ps_value & 0xFFFF0000L) != 0xFFFF0000L)
                    truncate(16);
            }
            else
            {
                if (polspnt->ps_value & 0xFFFF0000L)
                    truncate(16);
            }
        }
        else
            putreld(polspnt, 16);
        putword(polspnt->ps_value);		// Store word
        ++polspnt;
    }
}


//***********************************************
// Function: polstl - Process store long operator
// Returned: Nothing
//***********************************************

void polstrl(void)
{
    if (polspnt >= (polstk + POLSSIZE))	// Check for stack underflow
        underflow();
    else
    {
///		printf("### STORE LONG %d: %08X flags=%08X\n", polspnt->ps_type,
///				polspnt->ps_value, polspnt->ps_pnt.m->md_flag);

        if (polspnt->ps_type != PT_ABSOLUTE &&
                (polspnt->ps_type != PT_ABSOFSMS ||
                (polspnt->ps_pnt.m->md_flag & MA_ADDR) == 0))
		{
///			printf("### calling putreld\n");

			putreld(polspnt, 32);
		}
///		else
///			printf("### did not call putreld\n");

	     putlong(polspnt->ps_value);
        ++polspnt;
    }
}


//********************************************************
// Function: polpav - Process push absolute value operator
// Returned: Nothing
//********************************************************

void polpav(
    int header)

{
    polpush(getitem(header), 0, PT_ABSOLUTE, NULL); // Push value onto the stack
}


//*****************************************************************
// Function: polpar - Process push absolute relative value operator
// Returned: Nothing
//*****************************************************************

void polpar(
    int header)

{
    polpush(getitem(header) - (offset - curpsd->pd_offset + curpsd->pd_base),
            0, ((curmsd->md_flag & MA_ADDR) ? PT_RELATIVE : PT_RELOFSMS),
            (MD *)getpsd(0));
}


//***************************************************************************
// Function: polrac - Process push relocated value for current psect operator
// Returned: Nothing
//***************************************************************************

void polrac(
    int header)

{
	long value;
	long select;
	int  type;

    checkcur();
	value = getitem(header) + curpsd->pd_base;
	if (curmsd == NULL)
	{
		select = 0;
		type = PT_ABSOLUTE;
	}
	else
	{
		select = curmsd->md_sgd->sd_select;
		type = ((curmsd->md_flag & MA_ADDR) && select != 0) ? PT_ABSOLUTE :
				PT_ABSOFSMS;			// Value is absolute here if address
	}									//   of msect is specified

///	printf("### polrac: type=%d addr=%04X:%08X base=%08X flags=%08X\n", type,
///			select, value, curpsd->pd_base, curmsd->md_flag);

    polpush(value, select, type, curmsd);
}


//***********************************************************************
// Function: polrra - Process push relocated value for any psect operator
// Returned: Nothing
//***********************************************************************

void polraa(
    int header)

{
    long value;
    PD  *psd;

    value = getitem(header);
    psd = getpsd(getbyte());
    polpush(value + psd->pd_base, psd->pd_msd->md_sgd->sd_select,
            (psd->pd_msd->md_flag & MA_ADDR) ? PT_ABSOLUTE : PT_ABSOFSMS,
            psd->pd_msd);
}


//*****************************************************************
// Function: polrra - Process push relocated relative value for any
//		psect operator
// Returned: Nothing
//*****************************************************************

void polrra(
    int header)

{
    long value;
    PD  *psd;
    SD  *sgd;

    value = getitem(header);
    psd = getpsd(getbyte());
    sgd = psd->pd_msd->md_sgd;
/*
    if (sgd != cursgd && (sgd->sd_select != cursgd->sd_select ||
            sgd->sd_select == 0))		// In same segment?
    {
        diffspace(NULL);					  // No - complain (value pushed is
		polpush(value, 0, PT_RELATIVE, NULL); //   nonsense in this case!)
    }
    else
*/
//////qqqqqqq
        polpush(value + psd->pd_base - (offset - curpsd->pd_offset +
				curpsd->pd_base), psd->pd_msd->md_sgd->sd_select,
                (((psd->pd_msd == curmsd) || ((psd->pd_msd->md_flag&MA_ADDR) &&
                (curmsd->md_flag & MA_ADDR))) ? PT_ABSOLUTE : PT_RELOFSMS),
                psd->pd_msd);
}


//********************************************************
// Function: polexan - Process push symbol offset operator
// Returned: Nothing
//********************************************************

void polexan(
    int header)

{
    long value;

    value = getitem(header);
    symnum();
    symval += value;
    if (sympnt->sy_flag & SYF_UNDEF)
        polpush(0, 0, PT_ABSOLUTE, NULL);
    else if (sympnt->sy_flag & SYF_IMPORT)
	{
///		printf("### sym: %s\n", sympnt->sy_name + 1);

        polpush(symval, 0, PT_ABSOFSSYM, (void *)sympnt);
	}
	else
        polpush(symval, ((sympnt->sy_flag & SYF_ABS) ? sympnt->sy_sel :
				symmsd->md_sgd->sd_select), (((symmsd == NULL) ||
				((symmsd->md_flag & MA_ADDR) &&
				(symmsd->md_sgd->sd_select != 0))) ? PT_ABSOLUTE : PT_ABSOFSMS),
				(void *)symmsd);
}


//*******************************************************************
// Function: polexrn - Process push symbol offset (relative) operator
// Returned: Nothing
//*******************************************************************

void polexrn(
    int header)

{
    long value;
    SD  *sgd;

    value = getitem(header);
    symnum();
    symval += value;
    if (sympnt->sy_flag & SYF_UNDEF)
        polpush(0, 0, PT_RELATIVE, NULL);
    else if (sympnt->sy_flag & SYF_IMPORT) // Imported symbol?
	{
///		printf("### sym rel: %s\n", sympnt->sy_name + 1);

        polpush(symval, 0, PT_RELOFSSYM, sympnt); // Yes
	}
    else				// No
    {
        if (symmsd != NULL)
        {
            sgd = symmsd->md_sgd;
/*
            if (sgd != cursgd && (sgd->sd_select != cursgd->sd_select ||
                    sgd->sd_select == 0)) // In same segment?
            {

                if (!(sympnt->sy_flag & SYF_UNDEF)) // No
                    diffspace(sympnt);	// Complain unless undefined
                polpush(symval, 0, PT_RELATIVE, NULL); // Push a value we can
            }					       //   handle!
            else
////////qqqqqqqq
*/
                polpush(symval - (offset - curpsd->pd_offset + curpsd->pd_base), 0,
                        (symmsd == curmsd || (symmsd->md_addr != 0 &&
                        curmsd->md_addr != 0)) ?
                        PT_ABSOLUTE: PT_RELOFSMS, symmsd);
        }
        else
        {
            char bufr[100];

            sprintf(bufr, "Unsupported relative reference to absolute "
                    "symbol\n         %s", sympnt->sy_name+1);
            fail(bufr);
        }
    }
}


//*****************************************************************
// Function: twointern - Process internal symbol definition section
// Returned: Nothing
//*****************************************************************

static void twointern(void)

{
    SY  *sym;
    MD  *msd;
    long newvalue;
    int  newpsn;
    char flag;

    if (!mltcnt && !opsymsz && !symdd)	// Do we need to do this?
		return;							// No
    while (seccnt)
    {
		flag = getbyte();				// Get flag byte
		newvalue = getitem(flag);		// Get the value
        if (flag & SYF_ADDR)			// Address?
            newpsn = (flag & SYF_ABS) ?	// Yes - get psect number or selector
                    (uint)getword() : (uint)getnumber();
        else							// Not address - if not indicated as
        {								//   ABS eat the extra byte which old
            if (!(flag & SYF_ABS))		//   versions of XMAC inserted!
                getbyte();
            newpsn = 0;
        }
		getsym();						// Get symbol
		symbuf[0] = SYM_SYM;
		if ((sym=looksym()) != NULL)	// Find it in the symbol table
		{
			if (sym->sy_flag & SYF_MULT) // Multiply defined?
				multdef(sym, newvalue, newpsn); // Yes - complain
			if ((opsymsz || symdd) && !(sym->sy_flag&SYF_INSYMT))
            {							// Loading symbol table?
				sym->sy_flag |= SYF_INSYMT; // Yes - load this symbol if its
											//   not already loaded
                msd = sym->sy_psd->pd_msd;
			}
		}
        else
        {
            char buf[80];

            sprintf(buf, "Internal error: symbol \"%s\" not found during "
                    "pass 2\n", symbuf+1);
            fail(buf);
        }
    }
}


//*******************************************************
// Function: twostradr - Process starting address section
// Returned: Nothing
//*******************************************************

static void twostradr(void)

{
	if (multstr)
	{
        multaddr("starting");
		return;
	}
}


//******************************************************
// Function: twodebug - Process debugger address section
// Returned: Nothing
//******************************************************

static void twodebug(void)

{
	if (multdbg)
	{
		multaddr("debugger");
		return;
	}
}


//***********************************************************
// Function: twostack - Process initial stack address section
// Returned: Nothing
//***********************************************************

static void twostack(void)

{
    if (multstk)
	{
        multaddr("stack");
		return;
	}
}


//**************************************************************
// Function: segpush - Push segment selector on the polish stack
// Returned: Nothing
//**************************************************************

void segpush(
	SD *sgd)

{
    if (sgd->sd_select)
        polpush((long)(sgd->sd_select), 0, PT_ABSOLUTE, NULL);
    else
        polpush(0, 0, PT_SELMS, (MD *)sgd);
}


//***************************************************
// Function: polpush - Push value on the polish stack
// Returned: Nothing
//***************************************************

void polpush(
    long  value,
    long  select,
    int   type,
    void *pnt)

{
    if (polspnt <= polstk)				// Check for stack overflow
		overflow();
    else
    {
        --polspnt;
        polspnt->ps_value = value;
        polspnt->ps_select = select;
        polspnt->ps_type = type;
        polspnt->ps_pnt.m = pnt;
    }
}


//************************************************
// Function: symname - Get name of external symbol
// Returned: Nothing
//************************************************

void symname(void)

{
    getsym();							// Get symbol
    symbuf[0] = SYM_SYM;
    if ((sympnt=looksym()) == NULL)
        fail("Cannot find data for symbol during pass 2");
    if (!(sympnt->sy_flag & SYF_UNDEF))
    {									// Look it up in the symbol table
		symval += sympnt->sy_val.v;		// Use value if defined
        symmsd = sympnt->sy_psd->pd_msd;
    }
    else
    {
		varwmsg(sympnt, "Undefined", "referenced"); // Complain if undefined
        symmsd = NULL;
    }
    return;
}


//*************************************************
// Function: symnum - Get number of external symbol
// Returned: Nothing
//*************************************************

void symnum(void)

{
    int symn;

    symn = (sectypx < MSSC_MIN) ? getnumber() : getmsnum();
    if (symn > curmod->mb_symnummax)
    {
        char buf[60];

        sprintf(buf, "Illegal symbol number %d", symn);
        fail(buf);
    }
    sympnt = curmod->mb_symnumtbl[symn - 1];
    if (!(sympnt->sy_flag & SYF_UNDEF))
    {
        if (sympnt->sy_flag & SYF_IMPORT)
        {
            symval = 0;
            sympsd = NULL;
            symmsd = NULL;
        }
        else
        {
            symval = sympnt->sy_val.v;
            sympsd = sympnt->sy_psd;
            symmsd = sympsd->pd_msd;
        }
    }
    else
    {
		varwmsg(sympnt, "Undefined", "referenced"); // Complain if undefined
        symval = 0;
        sympsd = NULL;
        symmsd = NULL;
    }
}


//***********************************************
// Function: checkcur - Ensure have current psect
// Returned: Nothing
//***********************************************

void checkcur(void)
{
    if (curpsd == NULL)
        fail("Indeterminate psect reference");
}


//******************************************
// Function: putreld - Store relocation item
// Returned: Nothing
//******************************************

void putreld(
    PSTK *item,
    int   size)
{
    RD  *rld;
	char buf[60];

    rld = (RD *)getspace(sizeof(RD));	// Allocate relocation data block
	rld->rd_magic = 'RLD*';
    *curmsd->md_rdlast = rld;
    curmsd->md_rdlast = &rld->rd_next;	// Link to end of the relocation list
    rld->rd_next = NULL;
    rld->rd_loc = offset - curmsd->md_offset;
    switch (item->ps_type)
    {
     case PT_ABSOLUTE:
        rld->rd_type = (size == 32) ? (RT_32ABSOFS|RT_NONE) :
                ((size == 16) ? (RT_16ABSOFS|RT_NONE) : (RT_8ABSOFS|RT_NONE));
        rld->rd_data.n = 0;
        break;

     case PT_RELATIVE:
        rld->rd_type = (size == 32) ? (RT_32RELOFS|RT_NONE) :
                ((size == 16) ? (RT_16RELOFS|RT_NONE) : (RT_8RELOFS|RT_NONE));
        rld->rd_data.n = 0;
        break;

     case PT_SELMS:
        rld->rd_type = RT_SELECTOR|RT_SEGMENT;
        rld->rd_data.n = item->ps_pnt.s->sd_num;
        break;

     case PT_ABSOFSMS:
        rld->rd_type = (size == 32) ? (RT_32ABSOFS|RT_MSECT) :
                ((size == 16) ? (RT_16ABSOFS|RT_MSECT) : (RT_8ABSOFS|RT_MSECT));
        goto notsym;

     case PT_RELOFSMS:
        rld->rd_type = (size == 32) ? (RT_32RELOFS|RT_MSECT) :
                ((size == 16) ? (RT_16RELOFS|RT_MSECT) : (RT_8RELOFS|RT_MSECT));
        goto notsym;

     case PT_ADDRMS:
        rld->rd_type = (size == 32) ? (RT_32ADDR|RT_MSECT) :
                (RT_16ADDR|RT_MSECT);
     notsym:
        rld->rd_data.n = item->ps_pnt.m->md_num;        
        break;

     case PT_SELSYM:
        rld->rd_type = RT_SELECTOR|RT_SYMBOL;
        goto havesym;

     case PT_ABSOFSSYM:
        rld->rd_type = (size == 32) ? (RT_32ABSOFS|RT_SYMBOL) :
                ((size == 16) ? (RT_16ABSOFS|RT_SYMBOL) :
				(RT_8ABSOFS|RT_SYMBOL));
        goto havesym;

     case PT_RELOFSSYM:
        rld->rd_type = (size == 32) ? (RT_32RELOFS|RT_SYMBOL) :
                ((size == 16) ? (RT_16RELOFS|RT_SYMBOL) :
				(RT_8RELOFS|RT_SYMBOL));
        goto havesym;

     case PT_ADDRSYM:
        rld->rd_type = (size == 32) ? (RT_32ADDR|RT_SYMBOL) :
                (RT_16ADDR|RT_SYMBOL);
     havesym:
        rld->rd_data.s = item->ps_pnt.i;
        item->ps_pnt.i->sy_flag |= SYF_USED;
        ++importnum;
        break;

     default:
		sprintf(buf, "Illegal relocatable item type (%d) (internal error)",
			item->ps_type);
		fail(buf);
    }
}


//**********************************************************
// Function: ms2mtheadr - Process MS module header (MTHEADR)
// Returned: Nothing
//**********************************************************

static void ms2mtheadr(void)
{
    getmssym();							// Get module name
    strmov(modname, symbuf+1);			// Save module name
}


//****************************************************
// Function: ms2mcoment - Process MS comment (MCOMENT)
// Returned: Nothing
//****************************************************

static void ms2mcoment(void)
{
}


//**************************************************************
// Function: ms2mmodend - Process MS 16-bit module end (MMODEND)
// Returned: Nothing
//**************************************************************

static void ms2mmodend(void)

{
    uchar mtype;

    mtype = getbyte();					// Get module type
    if (mtype & 0x80)
        fail("Start address not valid in MS format OBJ file");
    twoeom();
}


//************************************************************
// Function: ms2mtypdef - Process MS type definition (MTYPDEF)
// Returned: Nothing
//************************************************************

static void ms2mtypdef(void)

{
}


//**************************************************************
// Function: ms2mpubdef - Process MS public definition (MPUBDEF)
// Returned: Nothing
//**************************************************************

static void ms2mpubdef(void)

{
    SY  *sym;
    long newoffset;
    int  newseg;
    int  newgrp;

    if (!mltcnt && !opsymsz && !symdd)	// Do we need to do this?
		return;							// No
    newgrp = getmsnum();				// Yes - get group number
    newseg = getmsnum();				// Get "segment" number
    if (newseg == 0 && newgrp == 0)
    {
        if(getword() != 0)
            fail("MS public symbol value is absolute address (unsupported)");
    }
    while (seccnt > 1)
    {
        getmssym();						// Get symbol name
		symbuf[0] = SYM_SYM;
        newoffset = (sectypx == MSSC_PUBDEF) ? getword() : getlong();
        getbyte();						// Discard the type index
		if ((sym=(sectypx < MSSC_LOCPUB) ? looksym() : looklocsym()) != NULL)
        {								// Find it in the symbol table

			if (sym->sy_flag & SYF_MULT) // Multiply defined?
			multdef(sym, newoffset, newseg); // Yes - complain
		}
        else
        {
            char buf[80];

            sprintf(buf, "Internal error: MS symbol \"%s\" not found during "
                    "pass 2\n", symbuf+1);
            fail(buf);
        }
    }
}


//**********************************************************
// Function: ms2mlocsym - Process MS local symbols (MLOCSYM)
// Returned: Nothing
//**********************************************************

static void ms2mlocsym(void)

{
}


//****************************************************************
// Function: ms2msegdef - Process MS segment definitions (MSEGDEF)
// Returned: Nothing
//****************************************************************

static void ms2msegdef(void)

{
    PD  *psd;
    long size;

    if (++psnumcnt > curmod->mb_psectnummax)
        fail("More MS segments encountered in pass 2 than in pass 1");
    psd = *psnumpnt++;

    if (!(psd->pd_flag & PA_OVER))
    {
        if ((getbyte() & 0xE0) == 0)	// Absolute segment?
        {
            getword();					// Yes - discard selector
            getbyte();					// Skip unused byte
        }

        size = (sectypx == MSSC_SEGDEF) ? getword() : getlong();

        if (psd->pd_modulus > 1)
            size = ((size + psd->pd_modulus - 1)/psd->pd_modulus) *
					psd->pd_modulus;
    }
}

///long ffaddr1;
///long xxxx;

//***************************************************************
// Function: ms2mfixupp - Process MS fix-up (MFIXUPP or (MFIX386)
// Returned: Nothing
//***************************************************************

// The only fixup type supported in this version is the reference to the
//   FLAT pseudo-group, however, we actually treat it as a type 4 fixup!
//   This allows linking of msects with unspecified addresses that are
//   resolved at run time. (This is mainly used when linking LKEs as the
//   addresses are resolved by LKELOAD when the LKE is loaded.) This works
//   because of the way we map between MS "segments" and "groups" and our
//   psects, msects, and segments.

static void ms2mfixupp(void)
{
    SY   *tarsym;
    SD   *tarsgd;
    MD   *tarmsd;
    PD   *tarpsd;
	MD   *frmmsd;
    int   locat;
    uchar fixdat;
    uchar header;
    int   type;
    int   fmethod;
    int   size;
    int   bytes;
	int   psnum;
    ulong tarvalue;
    int   boffset;
    PSTK  item;
	char  buf[80];

	if (junked)
		return;

    while (seccnt > 1)
    {
		if (((header = getbyte()) & 0x80) == 0) // Get header byte
			fail("MS fix-up thread definition not implemented yet");
										// Must be fix-up data
		if (msdispatch != msdoledata && msdispatch != msdolidata)
			fail("MS MFIXUPP record not preceeded by MLEDATA or MLIDATA");
		locat = getbyte() | (header << 8); // Get the locat value

///		printf("### fixup header: %s %X %03X\n", (header & 0x40) ? "ABS" :
///				"REL", (locat >> 10) & 0x0F, locat & 0x3FF);

		boffset = locat & 0x3FF;		// Get offset in data buffer
		fixdat = getbyte();				// Get the fixdat byte
		if (fixdat & 0x80)				// Is frame is specified by a thread?
			fail("MS fix-up threads not implemented yet");

		// Frame is specified explicitly

		type = (locat >> 10) & 0x0F;
		fmethod = (fixdat >> 4) & 0x7;
		if (fmethod != 1)
		{
			sprintf(buf, "Unsupported MS fixup frame method %d", fmethod);
			fail(buf);
		}

		frmmsd = getmsgrp();			// Get the group
		if (strcmp("FLAT_m", frmmsd->md_sym->sy_name + 1) != 0)
		{
			sprintf(buf, "Unsupported MS fixup frame group %s",
					frmmsd->md_sym->sy_name + 1);
			fail(buf);
		}

		// Dispatch on the target type

		switch (fixdat & 0x3)
		{
		 case 0:						// Target is specified as MS segment
			psnum = getmsnum();			//   number
			tarpsd = getpsd(psnum);
			tarmsd = tarpsd->pd_msd;
			tarsgd = tarmsd->md_sgd;
			tarsym = NULL;
			tarvalue = tarpsd->pd_base;

///			printf("### SEG FIXUP: |%s| %X\n", tarpsd->pd_sym->sy_name + 1,
///					tarvalue);

			break;

		 case 1:						// Target is specified as group number
			tarmsd = getmsgrp();
			tarsgd = (symmsd != NULL) ? symmsd->md_sgd : NULL;
			tarpsd = NULL;
			tarsym = NULL;
			tarvalue = tarmsd->md_addr;
			break;

		 case 2:						// Target is specified as symbol number
			symnum();
			tarmsd = symmsd;
			tarsym = sympnt;
			tarvalue = tarsym->sy_val.v;

///			printf("### sym for fixup: %s\n", tarsym->sy_name + 1);

			if (symmsd == NULL)
			{
///				printf("### No msect for sym\n");

				tarsgd = NULL;
			}
			else
			{
				tarsgd = symmsd->md_sgd;
///				tarisaddr = ((symmsd->md_flag & MA_ADDR) != 0);

///				printf("### sym sel = %04X\n", tarsgd->sd_select);
			}
			break;

		 default:
			fail("Illegal MS fixup target method 3");
		}
		if ((fixdat & 0x04) == 0)		// Is a displacement value present?
			tarvalue += (((sectypx == MSSC_FIXUPP) ? getword() : getlong()));

		//Here with everything we need - now apply it

		switch (type)					// Dispatch on the fixup type
		{
		 case 0:						// 8-bit value
			if (boffset >= mscount)
				fail(badoffset);
			size = 8;
			bytes = 1;
			goto valuecom;

		 case 2:						// Selector
			msfixsel(boffset, tarsym, tarsgd);
			break;

		 case 3:						// 16-bit address
			msfixsel(boffset + 2, tarsym, tarsgd);
			goto valuecom;

		 case 11:						// 32-bit address
			msfixsel(boffset + 4, tarsym, tarsgd);
			goto valuecom;

		 case 1:						// 16-bit offset
		 case 5:
			if (boffset > (mscount-2))
				fail(badoffset);
			size = 16;
			bytes = 2;
			goto valuecom;

		 case 9:						// 32-bit offset
		 case 13:
			if (boffset > (mscount-4))
				fail(badoffset);
			size = 32;
			bytes = 4;
		 valuecom:

///			if (tarsym != NULL && strcmp(tarsym->sy_name + 1, "initpci") == 0)
///				INT3;

			offset = msoffset + boffset;
			if (tarsym != NULL && (tarsym->sy_flag & SYF_IMPORT))
			{
				// Here if have an imported symbol - Thus must always be
				//   defered until run-time.

				item.ps_pnt.i = tarsym;
				if (locat & 0x4000)
				{
					item.ps_type = PT_ABSOFSSYM;
					putreld(&item, size);
					break;				// No immediate fix-up
				}
				else
				{
					item.ps_type = PT_RELOFSSYM;
					putreld(&item, size);
					tarvalue = -bytes;	// We need to account for the fact that
				}						//   an MS fixup is relative to the END
			}							//   of the value so do an immediate
										//   fix-up with the size of the value.
			else
			{
				// Here if not an imported symbol - This may be a external
				//   symbol, an MS segment (psect), or an MS group (msect).

				if ((locat & 0x4000) == 0) // Relative fixup?
				{
					// Here for a relative fixup

					if (tarmsd == curmsd || ((tarmsd == NULL ||
							tarmsd->md_addr != 0) && curmsd->md_addr != 0))
					{
						// Here if have same msect or if both msects have
						//   known address - This can be fully resolved here.

						tarvalue -= (msoffset - curmsd->md_offset + boffset +
								curmsd->md_addr + bytes);
					}
					else
					{
						// Here if at least one msect does not have an
						//   address yet - Must defer this.

						if (tarmsd != NULL)	// Will be NULL for an undefined
						{					//   symbol
							item.ps_type = PT_RELOFSMS;
							item.ps_pnt.m = tarmsd;
							tarvalue -= bytes;
							putreld(&item, size);
						}
					}
				}
				else
				{
					// Here if the fix-up is not relative - We can do it
					//   directly as long as the target value is fully
					//   defined.

					if (tarmsd != NULL && tarmsd->md_addr == 0)
					{
						// Here if the target msect is not known - We must
						//   defer this.

						item.ps_type = PT_ABSOFSMS;
						item.ps_pnt.m = tarmsd;
						putreld(&item, size);
					}
				}
			}

			// Apply the fix-up

			if (size == 32)
				*(ulong *)(msbuffer + boffset) += (ulong)tarvalue;
			else if (size == 16)
				*(ushort *)((msbuffer + boffset)) += (ushort)tarvalue;
			else
				*(char *)((msbuffer + boffset)) += (char)tarvalue;
			break;

		 default:
			sprintf(buf, "Illegal MS fix-up type %d", type);
			fail(buf);
		}
	}
}



//************************************************
// Function: msfixsel - Fix up a MS selector value
// Returned: Nothing
//************************************************

// NOTE: We do NOT support fixup for selectors defined at run time, only
//       for symbol values!

static void msfixsel(
	char boffset,
	SY  *sym,
	SD  *seg)
{
    PSTK  item;

	if (boffset > (mscount - 2))
		fail(badoffset);
	if (sym != NULL && sym->sy_flag & SYF_IMPORT) // Imported symbol?
	{
		offset = msoffset + boffset;	// Yes - must defer this
		item.ps_type = PT_SELSYM;
		item.ps_pnt.i = sym;

///		printf("### putreld::: B\n");

		putreld(&item, 0);
	}
	else
		*(ushort *)((msbuffer + boffset)) += seg->sd_select;
}




//************************************************************
// Function: ms2mledata - Process MS data (MLEDATA or MLED386)
// Returned: Nothing
//************************************************************

static void ms2mlxdata(void)

{
    uchar *pnt;
	char   buf[80];

    curpsd = getpsd(getmsnum());
	if (curpsd->pd_tsize == 0)			// Is the size 0?
	{
		junked = TRUE;
		return;							// Yes - must be one we are ignoring!
	}
	junked = FALSE;
    if (seccnt > 1025)
    {
        sprintf(buf, "MS data record larger than 1024 bytes (%d)", seccnt-1);
        fail(buf);
    }
    curmsd = curpsd->pd_msd;
    cursgd = curmsd->md_sgd;
    msoffset = ((sectypx == MSSC_LEDATA) ? getword() : getlong()) +
            curpsd->pd_offset;
    mscount = seccnt - 1;

///	printf("### MS data %d at 0x%08X\n", seccnt, curmsd->md_addr + msoffset);

    pnt = msbuffer;
    while (seccnt > 1)
        *pnt++ = getbyte();
    msdispatch = (sectypx < MSSC_LIDATA) ? msdoledata : msdolidata;
}


//****************************************************
// Function: msdoledata - Copy MS data to the RUN file
// Returned: Nothing
//****************************************************

void msdoledata(void)

{
    uchar *pnt;

    offset = msoffset;
    pnt = msbuffer;
    while (--mscount >= 0)
        putbyte(*pnt++);
    msdispatch = (void (*)(void))nullfunc;
}


//****************************************************
// Function: msdolidata - Copy MS data to the RUN file
// Returned: Nothing
//****************************************************

void msdolidata(void)

{
    offset = msoffset;					// Set offset for storing data
    lid2block(msbuffer);				// Process the outer data block
    msdispatch = (void (*)(void))nullfunc;
}


//******************************************************
// Function: lid2block - Process LIDATA data block field
// Returned: Pointer to next byte in record
//******************************************************

uchar *lid2block(
    uchar *pnt1)

{
    union
    {   uchar  *c;
        ushort *s;
        ulong  *l;
    } pnt2;
    int    reptcnt;
    int    blkcnt;
    int    bytecnt;

    pnt2.c = pnt1;
    reptcnt = (sectypx == MSSC_LEDATA) ? *pnt2.s++ : *pnt2.l++;
    blkcnt = *pnt2.s++;
    while (--reptcnt >= 0)
    {
        pnt1 = pnt2.c;
        if (blkcnt == 0)
        {
            bytecnt = *pnt1++;
            while (--bytecnt >= 0)
                putbyte(*pnt1++);
        }
        else
        {
            while (--blkcnt >= 0)
                pnt1 = lid2block(pnt1);
        }
    }
    return (pnt1);
}


//***********************************************************
// Function: ms2mcomdef - Process MS communal names (MCOMDEF)
// Returned: Nothing
//***********************************************************

static void ms2mcomdef(void)

{
    int type;
    SY *sym;

    if (!mltcnt && !opsymsz && !symdd)	// Do we need to do this?
		return;							// No
    while (seccnt >= 5)					// Yes
    {
        getmssym();						// Collect the symbol name
        getmsnum();						// Get and discard the type index
        type = getbyte();				// Get the data type
        if (type == 0x61)
        {
            getmsxnum();
            getmsxnum();
        }
        else if (type == 0x62)
            getmsxnum();
        else
        {
            char buf[80];

            sprintf(buf, "Illegal type value (0x02.2X) in MS communal "
                    "definition", type);
            fail(buf);
        }
		symbuf[0] = SYM_SYM;
		if ((sym=(sectypx == MSSC_COMDEF) ? looksym() : looklocsym()) != NULL)
        {								// Find it in the symbol table
			if (sym->sy_flag & SYF_MULT) // Multiply defined?
				multdef(sym, 0, 0);		// Yes - complain
		}
        else
        {
            char buf[80];

            sprintf(buf, "Internal error: MS symbol \"%s\" not found during "
                    "pass 2\n", symbuf+1);
            fail(buf);
        }
    }
}


//**********************************************************************
// Function: ms2mcomdat - Process MS initialized communal data (MCOMDAT)
// Returned: Nothing
//**********************************************************************

static void ms2mcomdat(void)
{
    int    flags;
    int    select;
    int    namenum;
    uchar *pnt;
    char  *name;
    SY    *sym;

    flags = getbyte();					// Get flag byte
    select = getbyte();					// Get attribute byte
    getbyte();							// Get align byte
    msoffset = (sectypx == MSSC_COMDAT) ? getword() : getlong();
    getmsnum();							// Get and discard type index
    if ((select & 0x0F) == 0)			// If necessary, get and discard
    {									//   the group and segment indexes
        getbyte();
        getbyte();
    }
    if ((namenum = getmsnum()) > curmod->mb_lnamesmax)
    {									// Get public name index
        char buf[60];

        sprintf(buf, "Illegal MS public name index %d", namenum);
        fail(buf);
    }
    name = curmod->mb_lnamestbl[namenum-1];
    strmov(symbuf+1, name+1);
    symsize = name[0] + 1;
    symbuf[0] = SYM_SYM;				// Indicate MS group name

    if (seccnt > 1025)
    {
        char buf[80];

        sprintf(buf, "MS communal data record larger than 1024 bytes (%d)",
                seccnt-1);
        fail(buf);
    }
    if ((sym=(flags & 0x04) ? looklocsym() : looksym()) == NULL)
    {									// Find symbol table entry
        char buf[100];

        sprintf(buf, "MS public name %0.30s\n         not found in symbol "
                "table during pass 2", name+1);
        fail(buf);
    }
    curpsd = sym->sy_psd;
    curmsd = curpsd->pd_msd;
    cursgd = curmsd->md_sgd;
    msoffset += curpsd->pd_offset;
    mscount = seccnt - 1;
    pnt = msbuffer;
    while (seccnt > 1)
        *pnt++ = getbyte();
    msdispatch = (flags & 0x02) ? msdolidata : msdoledata;
}


//*****************************************************
// Function: getmsgrp - Get pointer to MSD for MS group
// Returned: Pointer to MSD
//*****************************************************

MD *getmsgrp()
{
	uint grpnum;
	char buf[80];

    if ((grpnum = (uint)getmsnum()) == 0 ||
            grpnum > (uint)(curmod->mb_groupnummax))
    {
///		INT3;

        sprintf(buf, "Illegal MS group number %d", grpnum);
        fail(buf);
    }
    return (curmod->mb_groupnumtbl[grpnum-1]);
}
