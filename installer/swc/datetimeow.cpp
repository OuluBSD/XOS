#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "xosstuff.h"
#include "swcwindows.h"
#include "xosqdes.h"
#include "winnetwork.h"

uchar correct[] =
{	1,  1,  4,  4,  5,  5,  6,  6,  6,  7,  7,  8,
	1,  1,  3,  3,  4,  4,  5,  5,  5,  6,  6,  7
};
	
uchar mlength[] =
{	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

// Table which gives days in month

char lengthtbl[] =
{	31, 31,			// Jan
	28, 29,			// Feb
	31, 31,			// Mar
	30, 30,			// Apr
	31, 31,			// May
	30, 30,			// Jun
	31, 31,			// Jul
	31, 31,			// Aug
	30, 30,			// Sep
	31, 31,			// Oct
	30, 30,			// Nov
	31, 31,			// Dec
};

// Table which gives days to start of month

ulong days[] =
{	0  , 0,			// Jan
	31 , 31,		// Feb
	59 , 60,		// Mar
	90 , 91,		// Apr
	120, 121,		// May
	151, 152,		// Jun
	181, 182,		// Jul
	212, 213,		// Aug
	243, 244,		// Sep
	273, 274,		// Oct
	304, 305,		// Nov
	334, 335,		// Dec
};


static ulong litconv = 0x31B5D43B;
static long  lit146097 = 146097;
static long  lit36524 = 36524;
static long  lit1461 = 1461;
static long  lit400 = 400;
static long  lit366 = 366;
static long  lit365 = 365;
static long  lit100 = 100;
static long  lit60 = 60;
static long  lit31 = 31;
static long  lit24 = 24;
static long  lit4 = 4;


//*****************************************************************************
// Function: swcConvTimeDos2Xos - Convert a DOS time value to an XOS time value
// Returned: XOS time value
//*****************************************************************************

extern ulong _convt2x(long val);
#pragma aux _convt2x = \
"		mul 	litconv" \
"		shld	EDX, EAX, 8" \
		value [EDX] \
		parm   [EAX];

ulong swcConvTimeDos2Xos(
	struct tm *tm)

{
	return (_convt2x(((((tm->tm_hour * 60) + tm->tm_min) * 60) + tm->tm_sec) *
			1000));
}


//*****************************************************************************
// Function: swcConvTimeXos2Dos - Convert an XOS time value to a DOS time value
// Returned: XOS time value
//*****************************************************************************

void swcConvTimeXos2Dos(
	struct tm *tmp,
	ulong  time)

{
	_asm
	{
		mov		EBX, tmp
		mov		EAX, time
		add		EAX, 25			// Round to nearest millsecond
		jnc		t2d01
		xor		EAX, EAX
		dec		EAX
t2d01:	mul		lit24
		mov		[EBX+8], EDX	// tm_hour
		mul		lit60
		mov		[EBX+4], EDX	// tm_min
		mul		lit60
		mov		[EBX], EDX		// tm_sec
	}
}


//*******************************************************************************
// Function: swcConvDateXos2Dos - Convert an XOS format date to a DOS format date
// Returned: Nothing
//*******************************************************************************

void swcConvDateXos2Dos(
	struct tm *tm,
	ulong  xosdate)

{
	__asm
	{
		mov		EAX, xosdate
		xor		EBX, EBX
		xor		EDX, EDX			// Get number of 400-year groups since
		div		lit146097			//   1600
		imul	EDI, EAX, 400		// Save number of years
		mov		EAX, EDX			// In first 100-year group in the 400-year
		cmp		EAX, 36525			//   group?
		jl		lbl104				// Yes
		dec		EAX					// No - calculate 100 year group
		xor		EDX, EDX
		div		lit36524
		imul	EAX, 100			// Get years to start of 100 year group
		add		EDI, EAX			// Add it in
		mov		EAX, EDX			// Get day in 100 year group
		cmp		EAX, 1460			// In first 4 year group?
		jb		lbl110				// Yes
		sub		EAX, 1460			// No - Adjust for non-leap century
		add		EDI, 4
lbl104:	xor		EDX, EDX			// Calculate 4 year group
		div		lit1461
		shl		EAX, 2				// Get years to start of 4 year group
		add		EDI, EAX
		mov		EAX, EDX
		cmp		EAX, 366			// In first year of 4 year group?
		jl		lbl106				// Yes
		dec		EAX					// No - adjust for leap year
		jmp		lbl110
	
lbl106:	mov		BL, 12
		xor		EDX, EDX
		div		lit366
		jmp		lbl112

lbl110:	xor		EDX, EDX
		div		lit365				// Get day in year
lbl112:	add		EDI, EAX			// Get total years
		mov		EAX, EDX
		push	EAX					// Save day in year
		xor		EDX, EDX			// Get guess for month
		div		lit31				// c{EAX} = month, c{EDX} = day
		add		DL, correct[EBX+EAX] // Correct the day
		cmp		DL, mlength[EBX+EAX] // Have we gone into the next month?
		jbe		lbl114				// No
		sub		DL, mlength[EBX+EAX] // Yes - adjust the day
		inc		EAX					// And adjust the month

// When get here:
//		c{EAX} = Month
//		c{EDX} = Day of month (0 - 11)
//		c{EDI} = Year (offset from 1600)

lbl114:	mov		ebx, tm
		mov		[EBX+12], EDX		// tm_mday
///		inc		EAX
		mov		[EBX+16], EAX		// tm_mon
		sub		EDI, 300			// Change to offset from 1980
		mov		[EBX+20], EDI		// tm_year
		pop		[EBX+28]			// tm_yday
	}
}

//*******************************************************************************
// Function: swcConvDateDos2Xos - Convert a DOS format date to an XOS format date
// Returned: Nothing
//*******************************************************************************

extern ulong _convd2x(long month, long day, long year);
#pragma aux _convd2x = \
"		xor		ECX, ECX"			/* Assume not leap year */ \
"		inc		ESI"				/* We thought it was 1 based! */ \
"		add		EAX, 300" \
"		xor		EDX, EDX"			/* Get number of 400-year groups */ \
"		div		lit400" \
"		imul	EDI, EAX, 146097"	/* Get days to start of group */ \
"		mov		EAX, EDX"			/* Get years within 400-year group */ \
"		cmp		EAX, 100"			/* In first 100-year group? */ \
"		jb		lb204"				/* Yes */ \
"		add		EDI, 36525"			/* No */ \
"		sub		EAX, 100" \
"		xor		EDX, EDX" 			/* Get number of 100-year groups */ \
"		div		lit100" \
"		imul	EAX, 36524"			/* Get days to start of 100-year group */ \
"		add		EDI, EAX" \
"		mov		EAX, EDX"			/* Year within the 100-year group */ \
"		cmp		EAX, 4"				/* In first 4-year group? */ \
"		jb		lb208"				/* Yes */ \
"		add		EDI, 1460"			/* No - compensate for first 4-year group */ \
"		sub		EAX, 4"				/*   which does not have a leap-year */ \
"lb204:	xor		EDX, EDX" \
"		div		lit4"				/* Get number of 4-year groups */ \
"		imul	eax, 1461"			/* Get days to start of 4-year group */ \
"		add		EDI, EAX" \
"		mov		EAX, EDX" \
"		test	EAX, EAX"			/* First year in 4-year group? */ \
"		jne		lb206"				/* No */ \
"		inc		ECX"				/* Yes - indicate leap-year */ \
"		jmp		lb210" \
\
/* Here if not first year in a 4-year group */ \
\
"lb206:	add		EDI, 366" \
"		dec		EAX" \
"lb208:	imul	EAX, 365"			/* Get days to start of year */ \
"		add		EDI, EAX" \
"lb210:	test	ESI, ESI"			/* Month too small? */ \
"		jne		lb212"				/* No */ \
"		inc		ESI"				/* Yes - assume JAN! */ \
"lb212:	cmp		ESI, 12"			/* Month too large? */ \
"		jbe		lb214"				/* No */ \
"		mov		SI, 12"				/* Yes - assume DEC! */ \
"lb214:	movzx	EAX, lengthtbl[ECX-2][ESI*2]" /* Get maximum length of the month */ \
"		test	EBX, EBX"			/* Day too small? */ \
"		jne		lb216"				/* No */ \
"		inc		EBX"				/* Yes - assume 1 */ \
"lb216:	cmp		EBX, EAX"			/* Day too large? */ \
"		jbe		lb218"				/* No */ \
"		mov		EBX, EAX"			/* Yes - assume maximum */ \
"lb218:	shl		ECX, 2" \
"		add		EDI, days[ECX-8][ESI*8]" /* Get days to start of month */ \
"		add		EDI, EBX"			/* Get total days */ \
"		dec		EDI" \ 
"		mov		EAX, EDI" \
		parm  [ESI] [EBX] [EAX] \
		value [EAX] \
		modify [ECX EDX EBX];

ulong swcConvDateDos2Xos(
	struct tm *tm)

{
	return (_convd2x(tm->tm_mon, tm->tm_mday, tm->tm_year));
}

