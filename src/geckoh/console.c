#include "geckoh.h"


TRMMODES trmdata;		// Data block for svcTrmDspMode system call
TRMMODES trmsave;		// Data block for saving initial terminal set-up

long swidth;			// Width of entire screen
;;;long swidx2;			// Width of entire screen times 2
long sheight;			// Height of entire screen
long dheight;			// Height of scrolling display area

long curline;
long curchar;

int  curatr = TXTATR;

int  consta;
int  spcchr;
int  conbits;

char coninpbuf[100];

uchar  conring[120];
uchar *conputer = conring;
uchar *contaker = conring;
int    concnt;


/*
;Define FLAG bit values

FLAG$N  =!4000h		;Nested task flag
FLAG$O  =!0800h		;Overflow flag
FLAG$D  =!0400h		;Direction flag
FLAG$I  =!0200h		;Interrupt flag
FLAG$T  =!0100h		;Trap flag
FLAG$S  =!0080h		;Sign flag
FLAG$Z  =!0040h		;Zero flag
FLAG$A  =!0010h		;Half-carry (auxillary carry) flag bit
FLAG$P  =!0004h		;Parity flag
FLAG$C  =!0001h		;Carry flag bit
FLAG$ALL=!4FD5h		;All flag bits

;Define offsets in the flag symbol table

fst_sym0=!0t		;Symbol for value = 0
fst_sym1=!2t		;Symbol for value = 1
fst_sym2=!4t		;Symbol for value = 2
fst_sym3=!6t		;Symbol for value = 3
fst_sym4=!8t		;Symbol for value = 4
fst_sym5=!10t		;Symbol for value = 5
fst_sym6=!12t		;Symbol for value = 6
fst_sym7=!14t		;Symbol for value = 7
fst_pos =!16t		;Amount to shift field to get value
fst_mask=!18t		;Mask for field
fst_ENSZ=!22t		;Size of table entry
*/

QAB cinpqab =
{	QFNC_QIO|QFNC_INBLOCK,	// func    = 0  - Function
	0,						// status  = 2  - Returned status
	0,						// error   = 4  - Error code
	0,						// amount  = 8  - Amount transfered
	DH_STDTRM,				// handle  = 12 - Device handle
	VECT_CONINP,			// vector  = 16 - Vector for interrupt
	{0},
	0,						// option  = 20 - Option bits
	100,					// count   = 24 - Amount to transfer
	coninpbuf,				// buffer1 = 28 - Pointer to data buffer
	0,						// buffer2 = 32
	0						// parm    = 36 - Pointer to parameter list
};



struct
{	BYTE4PARM clrim;
	BYTE4PARM setim;
	uchar     end;
} trmparm =
{	{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
	{PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_IMAGE},
	0
};


static void borderline(uchar *text, int line);
static void chrlf(void);
static void setcur(void);


void consoleinit(void)
{
	RINFO *rpnt;
	union
	{	uchar  *c;
		ushort *s;
		long   *l;
	}      pnt;
	long   rtn;
	int    cnt;

	if ((rtn = svcTrmDspMode(DH_STDTRM, DM_RTNDATA, &trmdata)) < 0)
		setupfail(rtn, "Cannot setup to use terminal screen");

	memcpy(&trmsave, &trmdata, sizeof(trmdata));

	if ((rtn = svcTrmDspMode(DH_STDTRM, DM_RTNDATA|DM_USEDATA|DM_TEXT,
			&trmdata)) < 0)
		setupfail(rtn, "Cannot setup to use terminal screen");
	swidth = trmdata.dm_columns;
	sheight = trmdata.dm_rows;
	dheight = trmdata.dm_rows - 7;
	if ((rtn = svcTrmMapScrn(DH_STDTRM, scrnbufr, swidth * sheight * 2,
			0)) < 0)
		setupfail(rtn, "Error mapping the console screen buffer");
	if ((rtn = svcIoInBlockP(DH_STDTRM, NULL, 0, &trmparm)) < 0)
		setupfail(rtn, "Error setting terminal parameters");
	memsetlong(scrnbufr, (((TXTATR << 8) + ' ') << 16) + (TXTATR << 8) + ' ',
			swidth * sheight / 2);
	topline();
	borderline(NULL, 5);
	borderline("1\3Help\2 2\3UpdSta\2 4\3ExmEIP\2 5\3ExmPrev\2 6\3ExmSame\2 "
			"7\3ExmNext\2 9\3Trace\2""10\3Go", sheight - 1);
	pnt.l = (long *)(scrnbufr + 158);
	cnt = sheight;
	do
	{
		*pnt.l = VBCHR + (VBCHR << 16);
		pnt.c += 160;
	} while (--cnt > 0);
	*(ushort *)(scrnbufr + 0) = 0xD5 + (BRDATR << 8);
	*(ushort *)(scrnbufr + 158) = 0xB8 + (BRDATR << 8);
	*(ushort *)(scrnbufr + 800) = 0xC6 + (BRDATR << 8);
	*(ushort *)(scrnbufr + 958) = 0xB5 + (BRDATR << 8);
	*(pnt.s - 159)=  0xD4 + (BRDATR << 8);
	*(pnt.s - 80) = 0xBE + (BRDATR << 8);


	cnt = 16;
	rpnt = &regtbl.eax;
	do
	{
		putstrns(rpnt->spos - ((rpnt->name[2]) ? 5 : 4), rpnt->name);
		putchrns(rpnt->spos - 2, ':');
		rpnt++;
	} while (--cnt > 0);
	curatr = TXTATR;
	svcIoQueue(&cinpqab);
}


void topline(void)
{
	char text1[128];
	char text2[64];

	if (target.l != 0)
		sprintf(text2, "GeckoT v%d.%d.%d", target.maj, target.min, target.edit);
	else
		strmov(text2, "Not Connected");

	sprintf(text1, " \3 GeckoR v%d.%d.%d \2 \xAE\xCD\xAF \3 %s \2 ",
			MAJVER, MINVER, EDITNUM, text2);
	borderline(text1, 0);
	curatr = TXTATR;
}


static void borderline(
	uchar *text,
	int    line)
{
	ushort *spnt;
	uchar  *tpnt;
	int     length;
	int     fill;
	int     cnt;
	int     chr;

	length = 0;
	if (text != NULL)
	{
		tpnt = text;
		while ((chr = *tpnt++) != 0)
		{
			if (chr > 3)
				length++;
		}
	}
	curatr = BRDATR;
	cnt = (78 - length) / 2;
	fill = 78 - length - cnt;
	spnt = (ushort *)(scrnbufr + 2 + 160 * line);
	while (--cnt >= 0)
		*spnt++ = HBCHR;
	cnt = length;
	if (length > 0)
	{
		while ((chr = *text++) != 0)
		{
			if (chr <= 3)
				curatr = (chr == 3) ? INVBRD : BRDATR;
			else
				*spnt++ = chr + (curatr << 8);
		}
	}
	while (--fill >= 0)
		*spnt++ = HBCHR;
}



void putraw(
	uchar *str,
	int    len)
{
	while (--len >= 0)
	{	
		*(ushort *)(scrnbufr + 962 + curline * 160 + curchar * 2) = *str++ +
				(curatr << 8);
		curchar++;
	}
	setcur();
}



void putstrns(
	ushort *pos,
	uchar  *str)
{
	int chr;

	while ((chr = *str++) != 0)
	{
		putchrns(pos, chr);
		pos++;
	}
}



void putstr(
	uchar *fmt, ...)
{
	va_list pi;
	int     chr;
///	int     lvl;
	char    text[200];

    va_start(pi, fmt);
    vsprintf(text, fmt, pi);
	fmt = text;
///	lvl = svcSchSetLevel(8);			// Disable all signals
	while ((chr = *fmt++) != 0)
		putchr(chr);
///	if (lvl == 0)
///		svcSchSetLevel(lvl);			// Enable all signals
}



int linechk(
	int (*func)(void))
{
	if (linenum >= -1 && ++linenum >= dheight)
	{
		contfunc = func;
		putstr("\4\3<space> for next screen, L for next line, G for rest, Q "
				"to abandon output \2");
		return (FALSE);
	}
	return (TRUE);
}

/*
int putstrchk(
	uchar *fmt, ...)
{
	va_list pi;
	int     chr;
	char    text[200];

	if (linenum >= -1 && ++linenum >= dheight)
	{
		putstr("\4\3<space> for next screen, L for next line, G for rest, Q "
				"to abandon output \2");
		return (FALSE);
	}
    va_start(pi, fmt);
    vsprintf(text, fmt, &va_arg(pi, long));
	fmt = text;
	while ((chr = *fmt++) != 0)
		putchr(chr);
	return (TRUE);
}
*/


void putchrns(
	ushort *pos,
	int     chr)
{
	*pos = (TXTATR << 8) | chr;
}


void ensureleft()
{
	if (curchar != 0)
		putstr("\r\n");
}



//************************************************************
// Function: putchr - Display a character in the scrolled area
// Returned: Nothing
//************************************************************

void putchr(
	int chr)
{
	switch (chr)
	{
	 case 1:			// Set color for error message
		curatr = ERRATR;
		break;

	 case 2:			// Set color for normal text
		curatr = TXTATR;
		break;

	 case 3:			// Inverse text
		curatr = INVATR;
		break;

	 case 4:			// Clear line
		memsetlong(scrnbufr + 962 + curline * 160,
			(((curatr << 8) + ' ') << 16) + ((curatr << 8) + ' '), 156);
		curchar = 0;
		goto finish;

	 case 5:			// Clear rest of line
		memsetlong(scrnbufr + 962 + curline * 160 + curchar * 2,
			(((curatr << 8) + ' ') << 16) + ((curatr << 8) + ' '),
			156 - curchar * 2);
		break;

	 case 8:			// Backspace character
		if (--curchar < 0)
		{
			if (curline > 0)
			{
				--curline;
				curchar = 77;
			}
			else
				curchar = 0;
		}
		goto finish;

	 case 9:			// HT
		do
		{
			putchr(' ');
		} while ((curchar & 0x07) != 0);
		break;

	 case 0x0A:			// LF
		chrlf();
		goto finish;

	 case 0x0D:			// CR
		curchar = 0;
		goto finish;

	 default:
		if (curchar >= 78)
		{
			curchar = 0;
			chrlf();
		}
		*(ushort *)(scrnbufr + 962 + curline * 160 + curchar * 2) = chr +
				(curatr << 8);
		curchar++;
	 finish:
		setcur();
	}
}


static void chrlf(void)
{
	if (++curline >= dheight)
	{
		memcpy(scrnbufr + 960, scrnbufr + 1120, (dheight - 1) * 160);
		memsetlong(scrnbufr + 802 + dheight * 160,
			(((curatr << 8) + ' ') << 16) + ((curatr << 8) + ' '), 156);
		--curline;
	}
	else
		setcur();
}


static void setcur(void)
{
	svcTrmCurPos(DH_STDTRM, -1, (curchar < 78) ? (curchar + 1) : curchar,
			curline + 6);
}


void clearscreen(void)
{
	int pos;

	curatr = TXTATR;
	pos = 0;
	do
	{
		memsetlong(scrnbufr + 962 + pos * 160, (((TXTATR << 8) + ' ') << 16) +
				((TXTATR << 8) + ' '), 156);
	} while (++pos < dheight);
	curline = 0;
	curchar = 0;
}


// Console input done signal routine

void coninpsig(void)
{
	uchar *pnt;
	int    chr;

	pnt = coninpbuf;
	while (--cinpqab.amount >= 0)
	{
		chr = *pnt++;
		switch (consta)
		{
		 case 0:						// Normal character
			if (chr == 0)				// Prefix byte?
			{
				consta++;				// Yes
				break;
			}
			if (chr == 3)				// ^C?
			{
				svcIoOutString(DH_STDTRM, "\x1B[2J", 0);
				exit(0);
			}
		 hvchar:
			if (chr == 0x1B)
			{
				if (cmdstate != CMD_CMD)
				{
					begincmd();
					break;
				}
			}
			else if (cmdstate != CMD_PAUSED)
			{
				if (concnt < sizeof(conring))
				{
					*conputer++ = chr;
					if (conputer >= (conring + sizeof(conring)))
						conputer = conring;
					concnt++;
				}
				if (cmdstate < CMD_PAUSED)
					echochrs();
			}
			else						// If have paused output
			{
				chr = toupper(chr);
				if (chr == ' ')
					linenum = 2;
				else if (chr == 'L')
					linenum = dheight - 2;
				else if (chr == 'G')
					linenum = -2;
				else if (chr == 'Q')
				{
					putstr("\r\4");
					begincmd();
					break;
				}
				else
					break;
				putstr("\r\4");
				cmdstate = contfunc();
				if (cmdstate == CMD_CMD)
					begincmd();
			}
			break;

		 case 1:						// First byte of special sequence
			spcchr = chr;
			consta++;
			break;

		 case 2:						// Second byte of special sequence
			conbits = chr;
			consta++;
			break;

		 case 3:						// Third byte of special sequence
			if (cmdstate == CMD_CMD)
			{
				cmdline[0] = spcchr | 0x80;
				putchr(0x04);		// Clear the line
				docommand();
			}
			consta = 0;
			chr = CMD_F1;
			goto hvchar;
		}
	}
	svcIoQueue(&cinpqab);
}


void echochrs(void)
{
	uchar chr;

	while (concnt > 0)
	{
		chr = *contaker++;
		if (contaker >= (conring + sizeof(conring)))
			contaker = conring;
		concnt--;		
		if (chr == 0x0D || (chr & 0x80))
		{
			if (chr == 0x0D)
				*cmdpnt = 0;
			else
			{
				cmdline[0] = chr;
				putchr(0x04);		// Clear the line
			}
			switch (cmdstate)
			{
			 case CMD_CMD:
				docommand();
				break;

			 case CMD_ROPEN:
				cmdpnt = cmdline;
				if ((cmdstate = modifyreg()) == CMD_CMD)
					begincmd();
				break;

			 case CMD_MOPEN:
				cmdpnt = cmdline;
				if ((cmdstate = modifymem()) == CMD_CMD)
					begincmd();
				break;
			}
		}
		else if (cmdpnt < (cmdline + sizeof(cmdline) - 2))
		{
			if (chr == 0x08)
			{
				if (cmdpnt > cmdline)
				{
					if (cmdpnt[-1] == 9)
					{
						do
						{
							putchr(0x08);
							putchr(' ');
							putchr(0x08);
						} while (((curchar - 1) & 0x07) != 0);
					}
					else
					{
						putchr(0x08);
						putchr(' ');
						putchr(0x08);
					}
					--cmdpnt;
				}
			}
			else
			{
				*cmdpnt++ = chr;
				putchr(chr);
			}
		}

	}
}
