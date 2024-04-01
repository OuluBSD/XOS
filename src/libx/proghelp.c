//-------------------------------------------------------------------------*
// PROGHELP.C
// Programmer's help utilities for XOS
//
// Written by: SA Ortmann
//
// Edit History:
// 12/09/94(sao) - Initial development
// 02/03/95(brn) - Clean up formatting and change default colors
// 02/06/95(sao) - Modified screen & kbd handling to support ANSI TRMs
// 02/23/95(sao) - Expanded display area, added 'more' indicators, prevent
//					scrolling into last page, added 'no color' (gpc_ANSIClr)
//					support, added support for variable length header area.
// 02/28/95(sao) - Added support for both an option and keyword table.
//					Fixed reported build date
// 05/15/95(sao) - Modified keyboard commands, eliminate WS support, Ctrl-C
//					breaks.
// 18May95 (fpj) - Changed name from progasst to proghelp.
// 08/28/96(brn) - Fix getTrmParms to set default values if no display
//-------------------------------------------------------------------------*

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
#include <ctype.h>
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xostrm.h>
#include <progarg.h>

// This is a somewhat quick and dirty conversion of the XOS 3.x.x version.
//   It DOES NOT support the following features what were supported in the
//   previous version:
//   1. Highlighting of default options
//   2. Scrolling for long displays. This should be OK for all programs when
//      using a 60 line display. If not, the top part of the display will be
//      scrolled off! (This really should be fixed! - Some of the logic for
//      scrolling is there, but nowhere near all of it.)

// Local variables

static PROGINFO *pib = NULL;

static int linenum = 1;

typedef struct line__ LINE;
struct line__
{	LINE  *next;
	ushort num;
	ushort len;
	char   text[];
};

static LINE  *linehead;
static LINE **lineprev = &linehead;

// Define values returned by getkey

#define KEY_IGNORE   0
#define KEY_EXIT     1
#define KEY_LINEUP   2
#define KEY_LINEDOWN 3
#define KEY_PAGEUP   4
#define KEY_PAGEDOWN 5
#define KEY_TOP      6
#define KEY_BOTTOM   7


static void setcolors(COLDATA *cb);
static void genlines(char *text, int pos, char *prefix);
static int  getkey(void);
static void storeline(char *text, int len);


//*********************************************//
// Function: opthelp - Display the help screen //
// Returned: Nothing                           //
//*********************************************//

int descpos;

void opthelp(void)
{
	ARGSPEC *tblpnt;
	SUBOPTS *subpnt;
	LINE    *lpnt;
	int      len;
	char     buffer[128];

    getTrmParms();						// Get the screen height and width
	getHelpClr();
    setcolors(&pib->hdr_color);

    // Setup header

	printf("\x1B[M\x1B[2J");				// Clear the screen
    len = sprintf(buffer, "Help on %s", pib->prgname);
	printf("%*s\x1B[7m  %s  \x1B[27m\n\n", (76 - len) / 2, "", buffer);
	if (pib->example != NULL && pib->example[0] != 0)
	{
		snprintf(buffer, 80, "%s %s", pib->prgname, pib->example);
		printf("%.80s\n", buffer);
	}
    snprintf(buffer, 80, "  version %d.%d [%s] %s", pib->majedt,
			pib->minedt, pib->build, pib->copymsg);
	printf("%.80s", buffer);

    printf("\x1B[%d;33H^C or Q to quit\x1B[6;1H", pib->screen_height);

	// Construct lines for the program description

	genlines(pib->desc, 0, "");
	storeline("", 0);
	storeline("OPTIONS:", 8);

	// Scan the argument tables and find the longest name so we know where
	//   to put the descriptions

	descpos = 0;
	if ((tblpnt = pib->opttbl) != NULL)
	{
		while (tblpnt->name != NULL)
		{
			if ((len = strlen(tblpnt->name) + 5) > descpos)
				descpos = len;

			if ((tblpnt->flags & ASF_XSVAL) &&
					(subpnt = tblpnt->svalues.s) != NULL)
			{
				while (subpnt->option != NULL)
				{
					if ((len = strlen(subpnt->option) + 5) > descpos)
						descpos = len;
					subpnt++;
				}
			}
			tblpnt++;
		}
	}

	// Generate text from the argument tables

	if ((tblpnt = pib->opttbl) != NULL)
	{
		while (tblpnt->name != NULL)
		{
			sprintf(buffer, "%s%s", (tblpnt->flags & (ASF_NEGATE | ASF_FLAG |
					ASF_BOOL)) ? "{NO}" : "    ", tblpnt->name);
			genlines(tblpnt->help_str, descpos, buffer);

			if ((tblpnt->flags & ASF_XSVAL) &&
					(subpnt = tblpnt->svalues.s) != NULL)
			{
				while (subpnt->option != NULL)
				{
					sprintf(buffer, "      %s", subpnt->option);
					genlines(subpnt->desc, descpos + 2, buffer);
					subpnt++;
				}
			}
			tblpnt++;
		}
	}

	// Display the scrollable (someday) lines

	lpnt = linehead;
	while (lpnt != NULL)
	{
		printf("%s\n", lpnt->text);
		lpnt = lpnt->next;
	}

	while (TRUE)
	{
		switch (getkey())
		{
		 case KEY_EXIT:
			printf("\x1B[M\x1B[2J");	// Clear the screen
			if (pib->console)
			    svcTrmAttrib(DH_STDTRM, 0x83, &pib->old_color);
			exit(0);					// And leave

		 case KEY_LINEUP:

		 case KEY_LINEDOWN:

		 case KEY_PAGEUP:

		 case KEY_PAGEDOWN:

		 case KEY_TOP:

		 case KEY_BOTTOM:
			break;
		}
	}
}


static void genlines(
	char *text,
	int   pos,
	char *prefix)
{
	char *pnt;
	char *bgn;
	char *bpnt;
	int   cnt;
	int   max;
	int   len;
	char  chr;
	char  buffer[128];

	if ((pnt = text) != NULL)
	{
		max = 80 - pos;
		while (TRUE)
		{
			while ((chr = *pnt) != 0 && isspace(chr))
				pnt++;
			bgn = pnt;
			bpnt = NULL;
			cnt = 0;
			while (cnt < max && (chr = *pnt) != 0)
			{
				pnt++;
				if (isspace(chr))
					bpnt = pnt;
				if (chr == '\n')
					break;
				cnt++;
			}
			if (cnt != 0 || chr == '\n')
			{
				if (chr != 0 && chr != '\n')
				{
					pnt = bpnt;
					cnt = pnt - bgn - 1;
				}

				len = sprintf(buffer, "%-*s%.*s", pos, prefix, cnt, bgn);
				storeline(buffer, len);
				if (prefix[0] != 0)
				{
					prefix = "";
					pos += 4;
				}
			}
			else
				break;
		}
	}
	if (prefix[0] != 0)
		storeline(prefix, strlen(prefix));
}


static void storeline(
	char *text,
	int   len)
{
	LINE *line;

	if ((line = malloc(sizeof(LINE) + len + 1)) == NULL)
	{
		fprintf(stderr, "? Cannot allocate memory for help text\n");
		exit(1);	
	}
	line->len = len;
	line->num = linenum++;
	memcpy(line->text, text, len);
	line->text[len] = 0;
	line->next = NULL;
	*lineprev = line;
	lineprev = &line->next;
}


static int getkey(void)
{
	static struct
	{   BYTE4PARM modec;
	    BYTE4PARM modes;
	    char      end;
	} inpparm =
	{	{(PAR_SET|REP_HEXV), 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
		{(PAR_SET|REP_HEXV), 4, IOPAR_TRMSINPMODE, TIM_IMAGE}
	};

	int key;
	int scancd;
	int bits1;
	int bits2;

	if ((key = svcIoInSingleP(DH_STDTRM, &inpparm)) != 0) // ASCII key?
	{
		if (key == 0x1B)
		{
			// ESC handling for ANSI serial terminals goes here!


			return (KEY_IGNORE);
		}
		return ((key == 3 || key == 'q' || key == 'Q') ? KEY_EXIT : KEY_IGNORE);
	}

	// Here if have a non-ASCII keyboard value - get the rest of the key
	//   value and status

	scancd = svcIoInSingleP(DH_STDTRM, &inpparm);
	bits1 = svcIoInSingleP(DH_STDTRM, &inpparm);
	bits2 = svcIoInSingleP(DH_STDTRM, &inpparm);

	// Special handling goes here!

	return (KEY_IGNORE);
}




// getTrmParms:  gets the 'console' value and screen size, saves in pib
// Returns TRUE - done
//		   FALSE - pure defaults (some problem)

int getTrmParms(void)
{
	static struct
	{   BYTE4PARM  devsts;
		char       end;
	} prmprm =
	{   {(PAR_GET|REP_HEXV), 4, IOPAR_DEVSTS, 0}
	};

    TRMMODES data;

	pib->console = FALSE;

    // Get the screen height and width

	if (svcTrmDspMode(DH_STDTRM, DM_RTNDATA, &data) >= 0)
	{
		pib->screen_width = data.dm_columns;
		pib->screen_height = data.dm_rows;
		pib->handle = DH_STDTRM;
	}
	else
	{
		pib->handle = 0;
		pib->screen_width = 80;
		pib->screen_height = 25;
		pib->page=0;
		return (FALSE);
    }

	// Make sure we really have a console!

	if (svcIoOutBlockP(DH_STDTRM, NULL, 0, &prmprm) < 0)
		return (FALSE);
    if (prmprm.devsts.value & DS_CONTROL)
	{
	    svcTrmAttrib(DH_STDTRM, 0x41,&pib->old_color);
		pib->console = TRUE;
	}
    return (TRUE);
};


// getHelpClr: gets help screen colors and saves in pib
//	Attempts to read program specific color set
//		If not, attempts to read global color set
//			If not, sets hardcoded color scheme
// Returns	TRUE - program specific or global color scheme
//			FALSE - hardcoded color scheme
// NOTE:  both foreground and background colors must be in the range 0-8

int getHelpClr(void)

{
	char const *bpnt;
    long  h_fgc;
	long  h_bgc;
	long  b_fgc;
	long  b_bgc;
    int   rv;
    char  envstr[41];
    char  clrbuf[41];

	rv = TRUE;
    h_fgc = -1;
	h_bgc = -1;
	b_fgc = -1;
	b_bgc = -1;
	clrbuf[40] = 0;
    sprintf(envstr, "^XOS^%s^HCLR",pib->prgname);
    if (svcSysGetEnv(envstr, &bpnt) > 0)
    {
		strncpy(clrbuf, bpnt, 40);
		h_fgc = atol(strtok(clrbuf, ",")) % 9;
		h_bgc = atol(strtok(NULL, ",")) % 8;
		b_fgc = atol(strtok(NULL, ",")) % 9;
		b_bgc = atol(strtok(NULL, ",")) % 8;
    }
    else
    {
		if (svcSysGetEnv("^XOS^ALL^HCLR", &bpnt) > 0)
		{
			strncpy(clrbuf, bpnt, 40);
			h_fgc = atol(strtok(clrbuf, ",")) % 9;
			h_bgc = atol(strtok(NULL, ",")) % 8;
			b_fgc = atol(strtok(NULL, ",")) % 9;
			b_bgc = atol(strtok(NULL, ",")) % 8;
        }
    }

    // If there was a problem (some color not set or fg/bg pair that are eq)

    if (h_fgc == -1 || h_bgc == -1 || b_fgc == -1 || b_bgc == -1 ||
			h_fgc == h_bgc || b_fgc == b_bgc)
    {
		h_fgc = 1;	// header blue on
		h_bgc = 7;	// White
		b_fgc = 2;	// body green on
		b_bgc = 0;	// black
		rv = FALSE;
    }
    pib->hdr_color.fgc = h_fgc;
    pib->hdr_color.bgc = h_bgc;
    pib->hdr_color.fgf = h_fgc;
    pib->hdr_color.bgf = h_bgc;
    pib->hdr_color.atr = 0;
    pib->bdy_color.fgc = b_fgc;
    pib->bdy_color.bgc = b_bgc;
    pib->bdy_color.fgf = b_fgc;
    pib->bdy_color.bgf = b_bgc;
    pib->bdy_color.atr = 0;
    return (rv);
};


// setcolors: Changes current color pallete

static void setcolors(COLDATA *cb)
{
	static long ansi_conv[8] = {0,4,2,6,1,5,3,7};

	if (cb->fgc != cb->bgc)
		printf("%c[%d;%dm" , 30 + ansi_conv[cb->fgc], 40 + ansi_conv[cb->bgc]);
}


void reg_pib(
	PROGINFO *user_pib)
{
    pib = user_pib;
    pib->scb.head = NULL;
    pib->scb.buffer = NULL;
	pib->kwdtbl = NULL;
	pib->opttbl = NULL;
};
