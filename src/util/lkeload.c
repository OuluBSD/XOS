//-----------------------------------------------------------------------
// LKELOAD.C
// Program to load LKE
// 
// Written by: John R. Goltz
//
// Edit History:
// ------------
// v?.? 08/20/92(brn)
//	Add comment header.
// v?.? 05/12/94(brn)
//	Fix spelling in command abbreviations.
// v3.3 26-Nov-94
//	Changed to use 6 character LKE symbol prefix
// v3.4 27-Nov-94
//	Seperated into LKELOAD.C and LKELOADF.C to allow the lkeloadf function
//	to be called by other programs (mainly INSTALL)
// v3.7 28-Sep-10
//	Changed to use varialble length (1 to 8 character) LKE symbol prefix
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

// Command format:
//   LKELOAD {/options} lkename {char1=value1 {char2=value2 {...}}}

// Even though this program is written in C, it does NOT use ANY of the
//   standard C run-time routines.  It is linked with _mainmin, which is an
//   absolutely minimum startup routine which does NO device set up of any
//   kind.  This is necessary so that it can be run during initial system
//   startup before the shared C run-time library is available.

#include <ctype.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xosstr.h>
#include "lkeload.h"

#define VERSION 4
#define EDITNO  1

typedef _Packed struct lkecharbfr_ LKECHARBFR;
_Packed struct lkecharbfr_
{	LKECHARBFR *next;
	union
	{	LKECHAR p;
		char    s[1];
	} b;
};

LKECHARBFR *lkecharbfr;

LKECHAR *addbfr;
LKECHAR *lkechar;

LKECHARBFR *firstchar;
LKECHARBFR *lastchar = (LKECHARBFR *)(&firstchar);

char   lkename[64];

char  *args;

// Following are local functions

void fail(char *fmt, ...);
#pragma aux fail aborts;
void getatom(void);
void helphave(void);
void noquiethave(void);
void quiethave(void);

LNGSTRCHAR result = {PAR_GET|REP_STR, 0, "RESULT", NULL, 1022};

#define AF(func) (int (*)(arg_data *))func

struct argspec
{   char  *name;
    void (*func)(void);
} options[] =
{   {"HELP"   , helphave},
    {"HEL"    , helphave},
    {"H"      , helphave},
    {"?"      , helphave},
    {"NOQUIET", noquiethave},
    {"NOQ",     noquiethave},
    {"QUIET"  , quiethave},
    {"QUI"    , quiethave},
    {"Q"      , quiethave},
};
#define NUMOPTS (sizeof(options)/sizeof(struct argspec))

char   atom[68];
char   value[68];
char   quiet;			// TRUE if no output wanted
char   haveerror = FALSE;
char   copymsg[] = "";
char   prgname[] = "LKELOAD";


//**********************************
// Function: mainmin - Main program
// Returned: 0 if normal, 1 if error
//**********************************

int mainmin(                            // Main program entry
    char **argp)
{
    char  *pnt1;
    char  *pnt2;
    int    cnt;
    int    size;
    char   chr;
    char   buffer[1024];

    args = argp[2];
    while ((chr = *args) != 0 && isspace(chr)) // Skip leading whitespace
        args++;
    while ((chr = *args) != 0 && !isspace(chr)) // Skip first atom
        args++;
    for (;;)
    {
        while ((chr = *args) != 0 && isspace(chr)) // Skip leading whitespace
            args++;
        if (chr == 0)
            break;
        if (chr == '/' || chr == '-')	// Is this atom an option?
        {
            struct argspec *opnt;

            args++;						// Skip the / or -
            getatom();					// Collect the option
            if (value[0] != 0)
                fail("Unexpected value for option \"%s\"", atom);
            opnt = options;
            cnt = NUMOPTS;
            do
            {
                if (strcmp(atom, opnt->name) == 0)
                {
                    (opnt->func)();
                    break;
                }
                opnt++;
            } while (--cnt > 0);
            if (cnt == 0)
                fail("Illegal option \"%s\"", atom);
        }
        else							// If atom is not an option
        {
            getatom();					// Collect the atom
            if (lkename[0] == '\0')		// Was the LKE name specified yet?
            {							// No - this is the LKE name
                char *pntn;
                char *pnta;
                int   cnt;
                char  chr;

                pnta = atom;
                pntn = lkename;
                cnt = 48;
                while ((chr = *pnta++) != '\0')
                {
                    if (--cnt < 0)
                        fail("LKE name \"%s\" is too long", atom);
                    if (chr == '=' || chr == ':' || chr == '\\' || chr == '/')
                        fail("Illegal LKE name \"%s\"", atom);
                    *pntn++ = chr;
                }
                strmov(pntn, ".LKE");
                continue;
            }

            if (value[0] == 0)			// Have a value?
            {
                value[0] = '0';			// No - assume 0!
                value[1] = 0;
            }
            if (strlen(atom) > 8)
                fail("Characteristic name \"%s\" is too long", atom);
            if ((lkecharbfr = (LKECHARBFR *)sbrk(64)) == (LKECHARBFR *)-1)
				fail("Cannot allocate memory for characteristic");
            lastchar->next = lkecharbfr;
            lastchar = lkecharbfr;
            strncpy(lkecharbfr->b.p.name, atom, 8); // Copy characteristic name

            if (isdigit(value[0]))		// Is the value numeric?
            {
                char *spnt;				// Yes
                ulong numval;
                ulong radix;

                spnt = value;
                radix = (value[0] != '0')? 10:
                    (spnt++, (toupper(value[1]) != 'X')? 8: (spnt++, 16));
                numval = 0;

                while ((chr = *spnt++) != 0 && isxdigit(chr))
                {
                    if (chr > '9')
                        chr += 9;
                    chr &= 0xF;
                    numval = numval * radix + chr;
                }
                if (chr != 0)
                    fail("Illegal numeric value \"%s\" for characteristic "
							"\"%s\"", value, atom);
                lkecharbfr->b.p.type = PAR_SET|REP_DECV;
                lkecharbfr->b.p.val.num = numval;
                lkecharbfr->b.p.length = 4;
            }
            else
            {
                int vsize;

                lkecharbfr->b.p.type = PAR_SET|REP_TEXT;
                vsize = strlen(value);
                if (vsize > 32)		// Is our value too long?
                    fail("String value for characteristic \"%s\" is too long",
							atom);
                lkecharbfr->b.p.length = vsize;
                strcpy(lkecharbfr->b.p.val.text, value);
            }
        }
    }

    //================================================
    // Here with the command line completely processed
    //================================================

    if (lkename[7] == '\0')				// Was an LKE specified?
        fail("No LKE specified");		// No - fail

    //=====================================================================
    //  When we get here, we have a list of characteristics buffers pointed
    //    to by firstchar - first we copy these into a single buffer
    //=====================================================================

    lkecharbfr = (LKECHARBFR *)(&firstchar);
    size = 0;							// First see how much space we need
    while ((lkecharbfr = lkecharbfr->next) != NULL)
        size += lkecharbfr->b.p.length + 10;

    if ((addbfr = (LKECHAR *)sbrk(size + 8 + sizeof(result))) ==
			(LKECHAR *)-1)
		fail("Cannot allocate memory");
    lkecharbfr = (LKECHARBFR *)(&firstchar);
    result.buffer = buffer;
    memcpy(addbfr, &result, sizeof(result));
    pnt1 = (char *)addbfr + sizeof(result);
    while ((lkecharbfr = lkecharbfr->next) != NULL)
    {
        cnt = lkecharbfr->b.p.length + 10;
        pnt2 = (char *)&(lkecharbfr->b.p);
        do
        {   *pnt1++ = *pnt2++;
        } while (--cnt > 0);
    }
    *pnt1 = 0;

    //=====================================================================
    // Here with the characteristics list (if any) set up, now we are ready
    //   to load the LKE
    //=====================================================================

	if (lkeloadf(quiet, lkename, addbfr) >= 0)
	{
		if (result.buffer[0] != 0)
		{
			strcat(result.buffer, "\n");
			message(LKEML_INFO, result.buffer);
		}
		return (0);
	}
	return (1);
}


//**********************************************
// Function: getatom - Collect next command atom
// Returned: Nothing
//**********************************************

void getatom(void)
{
    int   cnt;
    char *apnt;
    char *vpnt;
    char  chr;

    value[0] = 0;
    cnt  = 64;
    apnt = atom;
    while ((chr = *args) != 0 && chr != '/')
    {
        if (--cnt < 0)
            fail("Command item \"%s...\" is too long", atom);
        if (chr == '=' || isspace(chr))
        {
            while ((chr = *args) != 0 && isspace(chr))
                args++;
            if (chr != '=')
                break;
            args++;
            vpnt = value;
            cnt = 64;
            while ((chr = *args) != 0 && isspace(chr))
                args++;
            while ((chr = *args) != 0 && chr != '/' && !isspace(chr))
            {
                if (--cnt < 0)
                    fail("Value for command item \"%s\" is too long", atom);
                args++;
                *vpnt++ = chr;
            }
            *vpnt = 0;
            break;
        }
        args++;
        *apnt++ = toupper(chr);
    }
    *apnt = 0;
}


//**************************************************
// Function: quiethave - Process /Q or /QUIET option
// Returned: TRUE, does not return if error
//**************************************************

void quiethave(void)

{
    quiet = TRUE;
}

//********************************************************
// Function: noquiethave - Process /NOQ or /NOQUIET option
// Returned: TRUE, does not return if error
//********************************************************

void noquiethave(void)
{
    quiet = FALSE;
}


//**************************************************
// Function: helpprint - Print string for helphave
// Returned: Nothing
//**************************************************

void helpprint(
    char *helpstr,
    int   state)
{
	char *pnt;
	char  text[200];

	pnt = strmov(text, helpstr);
    if (state)
        pnt = strmov(pnt, " *");
    *pnt++ = '\n';
	*pnt = 0;
    message(LKEML_INFO, text);
}


//************************************************
// Function: helphave - Process /H or /HELP option
// Returned: TRUE, does not return if error
//************************************************

void helphave(void)
{
	char text[200];

	sprintf(text, "\nLKELOAD %d.%d (%s) %s\n\nLKELOAD {/option} lkename "
			"{char=value {...}}\n\n", VERSION, EDITNO, __DATE__, copymsg);
    message(LKEML_INFO, text);
    helpprint(" HELP or ?    - This message", FALSE);
    helpprint(" {NO}QUIET    - Don't display normal status messages", quiet);

    message(LKEML_FININFO, "\nA * shows this option is the current default.\n"
			"All options may be abbreviated to 1 or 3 letters where "
			"possible.\n");
    svcSchExit(0);
}


//************************************
// Function: fail - Report fatal error
// Returned: Never returns
//************************************

void fail(
	char *fmt, ...)
{
	va_list pi;
	char   *pnt;
	char    text[300];

    va_start(pi, fmt);
	pnt = strmov(text, "? LKELOAD: ");
    pnt += vsprintf(pnt, fmt, pi);
	*pnt++ = '\n';
	*pnt = 0;
    message(LKEML_FINERROR, text);
    svcSchExit(1);
}


//*********************************************
// Function: message - Output the output buffer
// Returned: Nothing
//*********************************************

void message(
    int   level,
    char *text)

{
	char buffer[24];

    svcIoOutString(STDTRM, text, 0);
	if ((level & 0x01) == 0)			// Error indication?
		haveerror = TRUE;				// Yes - remember that
	if (level >= 3 && haveerror)		// Final response and have seen error?
	{
		svcIoOutString(STDTRM, "Press enter to continue", 0); // Yes
		svcIoInBlock(STDTRM, buffer, sizeof(buffer));
		if ((level = svcTrmCurPos(STDTRM, 0, -1, -1)) < 0 ||
				((uchar)level) != 0)
			svcIoOutBlock(STDTRM, "\r\n", 2);
	}
}
