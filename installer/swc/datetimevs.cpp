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
///#include "mms.h"
///#include "winnetwork.h"

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

static char *spnt;
static char  names[] = "JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";

static ulong litconv = 0x31B5D43B;
static long  lit3600000000 = 3600000000;
static long  lit3600000 = 3600000;
static long  lit146097 = 146097;
static long  lit60000 = 60000;
static long  lit36524 = 36524;
static long  lit10000 = 10000;
static long  lit1461 = 1461;
static long  lit1000 = 1000;
static long  lit400 = 400;
static long  lit366 = 366;
static long  lit365 = 365;
static long  lit240 = 240;
static long  lit100 = 100;
static long  lit60 = 60;
static long  lit31 = 31;
static long  lit24 = 24;
static long  lit4 = 4;
static long  litTICKPERDAY_LO = TICKPERDAY_LO;
static long  litTICKPERDAY_HI = TICKPERDAY_HI;

static long get2dig(char *str);
static int  getchr(void);
static long getdate(struct tm *dt, char *str, int yropt, int offset);
static long getdec(int stopper);
static long gettime(struct tm *dt, char *str);
static long getyear(int offset);

#pragma warning(disable: 4035)		// Turn off the "no return value" warning



void swcDateTime::ConvToDos(
	struct tm *tm)

{
	ConvTimeToDos(tm);
	ConvDateToDos(tm);
}


void swcDateTime::ConvTimeToDos(
	struct tm *tm)

{
	_asm
	{
		mov		ECX, this
		mov		EAX, [ECX]				// First convert to 2/3-hour periods
		mov		EDX, 4[ECX]				//    since 1-jan-0001 (which fits in 32
		div		lit3600000000			//    bits, also we can always do this
		xor		EDX, EDX				//    divide without overflows)
		div		lit240					// Now convert to days since 1-Jan-0001
		imul	ECX, EAX, TICKPERDAY_HI	// Convert days back to ticks (this
		mul		litTICKPERDAY_LO		//   gives us the value for midnight)
		add		EDX, ECX				// Subtract to get ticks since midnight
		mov		ECX, this				//   which is the time only value
		sub		EAX, [ECX]
		sbb		EDX, 4[ECX]
		neg		EDX
		neg		EAX
		sbb		EDX, 0
		div		lit10000				// Get millseconds since midnight
		xor		EDX, EDX
		div		lit3600000				// Get hours
		mov		ECX, tm
		mov		[ECX].tm_hour, EAX		// Store hours
		mov		EAX, EDX
		xor		EDX, EDX
		div		lit60000				// Get minutes
		mov		[ECX].tm_min, EAX		// Store minutes
		mov		EAX, EDX
		xor		EDX, EDX
		div		lit1000					// Get seconds and milliseconds
		mov		[ECX].tm_sec, EAX		// Store seconds
	}

}


void swcDateTime::ConvDateToDos(
	struct tm *tm)

{
	__asm
	{
		mov		ECX, this
		mov		EAX, [ECX]
		mov		EDX, [ECX + 4]
		div		lit3600000000		// First convert to 6-minute periods
									//   since 1-Jan-0001 (which fits in 32
									//   bits, also we can always do this
		xor		EDX, EDX			//   divide without overflows)
		div		lit240				// Convert to days
		add		EAX, 366			// Adjust for the missing year 0000
		xor		EBX, EBX
		xor		EDX, EDX			// Get number of 400-year groups since
		div		lit146097			//   year 
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
		mov		[EBX].tm_mday, EDX
///		inc		EAX
		mov		[EBX].tm_mon, EAX
		sub		EDI, 1900			// Change to offset from 1980
		mov		[EBX].tm_year, EDI
		pop		[EBX].tm_yday
	}
}


void swcDateTime::ConvFromDos(struct tm *tm)

{
	swcDateTime dtc;

	dtc.ConvDateFromDos(tm);
	ConvTimeFromDos(tm);
	dt += dtc.dt;
}


void swcDateTime::ConvTimeFromDos(struct tm *tm)

{
	ulong millisec;

	millisec = ((((tm->tm_hour * 60) + tm->tm_min) * 60) + tm->tm_sec) * 1000;
	dt = ((llong)millisec) * 10000;
}


void swcDateTime::ConvDateFromDos(struct tm *tm)

{
	__asm
	{
		mov		EBX, tm
		xor		ECX, ECX			// Assume not leap year
		mov		ESI, [EBX].tm_mon	// Get month
		inc		ESI					// We thought it was 1 based!
		mov		EAX, [EBX].tm_year	// Get year (offset from 0000)
		mov		EBX, [EBX].tm_mday	// Get day in month
		add		EAX, 1900
		xor		EDX, EDX			// Get number of 400-year groups
		div		lit400
		imul	EDI, EAX, 146097	// Get days to start of group
		mov		EAX, EDX			// Get years within 400-year group
		cmp		EAX, 100			// In first 100-year group?
		jb		lbl204				// Yes
		add		EDI, 36525			// No
		sub		EAX, 100
		xor		EDX, EDX			// Get number of 100-year groups
		div		lit100
		imul	EAX, 36524			// Get days to start of 100-year group
		add		EDI, EAX
		mov		EAX, EDX			// Year within the 100-year group
		cmp		EAX, 4				// In first 4-year group?
		jb		lbl208				//Yes
		add		EDI, 1460			// No - compensate for first 4-year group
		sub		EAX, 4				//  which does not have a leap-year
lbl204:	xor		EDX, EDX
		div		lit4					// Get number of 4-year groups
		imul	eax, 1461			// Get days to start of 4-year group
		add		EDI, EAX
		mov		EAX, EDX
		test	EAX, EAX			// First year in 4-year group?
		jne		lbl206				// No
		inc		ECX					// Yes - indicate leap-year
		jmp		lbl210

// Here if not first year in a 4-year group

lbl206:	add		EDI, 366
		dec		EAX
lbl208:	imul	EAX, 365			// Get days to start of year
		add		EDI, EAX
lbl210:	test	ESI, ESI			// Month too small?
		jne		lbl212				// No
		inc		ESI					// Yes - assume JAN!
lbl212:	cmp		ESI, 12				// Month too large?
		jbe		lbl214				// No
		mov		SI, 12				// Yes - assume DEC!
lbl214:	movzx	EAX, lengthtbl[ECX-2][ESI*2] // Get maximum length of the month
		test	EBX, EBX			// Day too small?
		jne		lbl216				// No
		inc		EBX					// Yes - assume 1
lbl216:	cmp		EBX, EAX			// Day too large?
		jbe		lbl218				// No
		mov		EBX, EAX			// Yes - assume maximum
lbl218:	shl		ECX, 2
		add		EDI, days[ECX-8][ESI*8] // Get days to start of month
		add		EDI, EBX			// Get total days
		dec		EDI
		mov		EAX, EDI

// When get here, c{EAX} = days since 1-Jan-0000 - Now change to ticks
//   since 1-Jan-0001

		sub		EAX, 366
		jg		lbl220
		xor		EAX, EAX
lbl220:	imul	ECX, EAX, 201
		mul		litTICKPERDAY_LO
		add		EDX, ECX
		mov		ecx, this
		mov		[ECX], EAX
		mov		[ECX+4], EDX
	}
}


void swcDateTime::ConvToExternal(
	time_nz *nz)

{
	*(llong *)nz = sll(dt);
	nz->tzone = sw(tzone);
	nz->dlst = sw(dlst);
}


void swcDateTime::ConvFromExternal(
	time_nz *nz)

{
	if (nz->high & 0xFF)
	{
		dt = sll(*(llong *)nz);
		tzone = sw(nz->tzone);
		dlst = sw(nz->dlst);
	}
	else
		ConvFromOldFmt(sl(nz->low), sl(nz->high), nz->tzone, nz->dlst);
}


long swcDateTime::ConvToDays()

{
	return ((long)(dt/TICKPERDAY));
}


void swcDateTime::ConvFromDays(long days)

{
	dt = days * TICKPERDAY;
	tzone = (short)0x8000;
	dlst = 0;
}


llong swcDateTime::Diff(
	swcDateTime *a,
	swcDateTime *b)

{
	return (a->dt - b->dt);
}


void swcDateTime::TimeOnly(
	swcDateTime *dt)

{
	*this = *dt;
	TimeOnly();
}


void swcDateTime::GetLocal()

{
	SYSTEMTIME  stime;
	struct tm   tm;
	swcDateTime dtx;

	::GetLocalTime(&stime);
	tm.tm_year = stime.wYear - 1900;
	tm.tm_mon = stime.wMonth - 1;
	tm.tm_mday = stime.wDay;
	tm.tm_hour = stime.wHour;
	tm.tm_min = stime.wMinute;
	tm.tm_sec = stime.wSecond;
	dtx.ConvDateFromDos(&tm);
	ConvTimeFromDos(&tm);
	dt += dtx.dt + stime.wMilliseconds * 10000;
	tzone = (short)0x8000;
	dlst = 0;
}

///union
///{   FILETIME ft;
///    llong    lt;
///} filetime;

void swcDateTime::GetLocalTime()
{
	SYSTEMTIME stime;
	struct tm  tm;

	::GetLocalTime(&stime);

///    SystemTimeToFileTime(&stime, (FILETIME *)&dt);

///    GetSystemTimeAsFileTime(&filetime.ft);

	tm.tm_hour = stime.wHour;
	tm.tm_min = stime.wMinute;
	tm.tm_sec = stime.wSecond;
	ConvTimeFromDos(&tm);
///    ConvDateFromDos(&tm);
    dt += (stime.wMilliseconds * 10000);

}


/* static */ long swcDateTime::GetLocalDate()

{
	SYSTEMTIME  stime;
	struct tm   tm;
	swcDateTime dt;

	::GetLocalTime(&stime);
	tm.tm_year = stime.wYear - 1900;
	tm.tm_mon = stime.wMonth - 1;
	tm.tm_mday = stime.wDay;
	dt.ConvDateFromDos(&tm);
	return (dt.ConvToDays());
}


int swcDateTime::SetCurrent()

{
	SYSTEMTIME stime;
	struct tm  tm;

	ConvToDos(&tm);
	stime.wYear = (ushort)(tm.tm_year + 1900);
	stime.wMonth = (ushort)(tm.tm_mon + 1);
	stime.wDay = (uchar)(tm.tm_mday);
	stime.wHour = (uchar)(tm.tm_hour);
	stime.wMinute = (uchar)(tm.tm_min);
	stime.wSecond = (uchar)(tm.tm_sec);
	return (SetLocalTime(&stime));
}


void swcDateTime::ConvFromOldFmt(
	long  lowval,
	long  highval,
	int   tzoneval,
	int   dlstval)

{
	static long lit500 = 500;
	static long litTICKPERDAY_LO = TICKPERDAY_LO;

	__asm
	{
		mov		EAX, 0x65CE0300			// Convert fractrional days to 50usec
		mul		lowval					//   intervals (this is the smallest
		mov		EAX, EDX				//   interval that will fit in 32 bits!)
		mul		lit500					// Convert to 100ns ticks
		mov		ECX, this				// Store tick count in result
		mov		[ECX], EAX
		mov		4[ECX], EDX
		mov		EAX, highval			// Get days since 1-Jan-1600
		add		EAX, 0x8E956			// Change to days since 1-Jan-0001
		imul	ECX, EAX, TICKPERDAY_HI	// Convert to ticks
		mul		litTICKPERDAY_LO
		ADD		EDX, ECX
		MOV		ECX, this				// Add into result
		ADD		[ECX], EAX
		ADC		4[ECX], EDX
	}
	tzone = (short)tzoneval;
	dlst = (short)dlstval;
}


//*****************************************************************************
// Function: swcConvTimeDos2Xos - Convert a DOS time value to an XOS time value
// Returned: XOS time value
//*****************************************************************************

/*
void swcConvTimeDos2Xos(
	time_s    *time,
	struct tm *tm)

{
	ulong millisec;

	millisec = ((((tm->tm_hour * 60) + tm->tm_min) * 60) + tm->tm_sec) * 1000;
	time->dt = millisec * 10000;
}
*/

//*****************************************************************************
// Function: xwcConvTimeXos2Dos - Convert an XOS time value to a DOS time value
// Returned: Nothing
//*****************************************************************************

/*
void swcConvTimeXos2Dos(
	struct tm *tmp,
	time_s    *time)

{
	_asm
	{
		mov		ecx, time
		mov		EAX, [ECX]
		mov		EDX, 4[ECX]
		cmp		EDX, TICKPERDAY_HI
		jb		lbl012
		ja		lbl010
		cmp		EAX, TICKPERDAY_LO
		jb		lbl012
lbl010:	mov		EAX, TICKPERDAY_LO-1
		mov		EDX, TICKPERDAY_HI
lbl012:	push	EDX						// First convert to 2/3-hour periods
		push	EAX						//    since 1-jan-0001 (which fits in 32
		div		lit3600000000			//    bits, also we can always do this
		xor		EDX, EDX				//    divide without overflows)
		div		lit240					// Now convert to days since 1-Jan-0001
		imul	ECX, EAX, TICKPERDAY_HI	// Convert days back to ticks (this
		mul		litTICKPERDAY_LO		//   gives us the value for midnight)
		add		EDX, ECX				// Subtract to get ticks since midnight
		pop		ECX						//   which is the time only value
		sub		EAX, ECX
		pop		ECX
		sbb		EDX, ECX
		neg		EDX
		neg		EAX
		sbb		EDX, 0
		div		lit10000				// Get millseconds since midnight
		xor		EDX, EDX
		div		lit3600000				// Get hours
		mov		ECX, tmp
		mov		WORD PTR 6[ECX], AX		// Store hours
		mov		EAX, EDX
		xor		EDX, EDX
		div		lit60000				// Get minutes
		mov		WORD PTR 4[ECX], AX		// Store minutes
		mov		EAX, EDX
		xor		EDX, EDX
		div		lit1000					// Get seconds and milliseconds
		mov		WORD PTR 2[ECX], AX		// Store seconds
		mov		WORD PTR 0[ECX], DX		// Store milliseconds
	}
}
*/

//*****************************************************
// Function: swcConvDateXos2Dos - Convert an XOS format
//					date to a DOS format date
// Returned: Nothing
//*****************************************************

/*
void swcConvDateXos2Dos(
	struct tm *tm,
	time_s    *xosdate)

{
	__asm
	{
		mov		ECX, xosdate
		mov		EAX, [ECX]
		mov		EDX, [ECX + 4]
		cmp		EDX, 201
		ja		lbl082
		jb		lbl084
		cmp		eax, 711573504
		jb		lbl084
lbl082:	mov		edx, 201
		mov		eax, 711573504
lbl084:	div		lit3600000000		// First convert to 2/3-hour periods
									//   since 1-Jan-0001 (which fits in 32
									//   bits, also we can always do this
		xor		EDX, EDX			//   divide without overflows)
		div		lit240				// Convert to ticks
		add		EAX, 366			// Adjust for the messing year 0000
		xor		EBX, EBX
		xor		EDX, EDX			// Get number of 400-year groups since
		div		lit146097			//   year 
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
		mov		[EBX].tm_mday, EDX
///		inc		EAX
		mov		[EBX].tm_mon, EAX
		sub		EDI, 300			// Change to offset from 1980
		mov		[EBX].tm_year, EDI
		pop		[EBX].tm_yday
	}
}
*/

//****************************************************
// Function: swcConvDateDos2Xos - Convert a DOS format
//					date to an XOS format date
// Returned: Nothing
//****************************************************

/*
void swcConvDateDos2Xos(
	time_s    *date,
	struct tm *tm)

{
	__asm
	{
		mov		EBX, tm
		xor		ECX, ECX			// Assume not leap year
		mov		ESI, [EBX].tm_mon	// Get month
		inc		ESI					// We thought it was 1 based!
		mov		EAX, [EBX].tm_year	// Get year (offset from 0000)
		mov		EBX, [EBX].tm_mday	// Get day in month
		add		EAX, 300
		xor		EDX, EDX			// Get number of 400-year groups
		div		lit400
		imul	EDI, EAX, 146097	// Get days to start of group
		mov		EAX, EDX			// Get years within 400-year group
		cmp		EAX, 100			// In first 100-year group?
		jb		lbl204				// Yes
		add		EDI, 36525			// No
		sub		EAX, 100
		xor		EDX, EDX			// Get number of 100-year groups
		div		lit100
		imul	EAX, 36524			// Get days to start of 100-year group
		add		EDI, EAX
		mov		EAX, EDX			// Year within the 100-year group
		cmp		EAX, 4				// In first 4-year group?
		jb		lbl208				//Yes
		add		EDI, 1460			// No - compensate for first 4-year group
		sub		EAX, 4				//  which does not have a leap-year
lbl204:	xor		EDX, EDX
		div		lit4					// Get number of 4-year groups
		imul	eax, 1461			// Get days to start of 4-year group
		add		EDI, EAX
		mov		EAX, EDX
		test	EAX, EAX			// First year in 4-year group?
		jne		lbl206				// No
		inc		ECX					// Yes - indicate leap-year
		jmp		lbl210

// Here if not first year in a 4-year group

lbl206:	add		EDI, 366
		dec		EAX
lbl208:	imul	EAX, 365			// Get days to start of year
		add		EDI, EAX
lbl210:	test	ESI, ESI			// Month too small?
		jne		lbl212				// No
		inc		ESI					// Yes - assume JAN!
lbl212:	cmp		ESI, 12				// Month too large?
		jbe		lbl214				// No
		mov		SI, 12				// Yes - assume DEC!
lbl214:	movzx	EAX, lengthtbl[ECX-2][ESI*2] // Get maximum length of the month
		test	EBX, EBX			// Day too small?
		jne		lbl216				// No
		inc		EBX					// Yes - assume 1
lbl216:	cmp		EBX, EAX			// Day too large?
		jbe		lbl218				// No
		mov		EBX, EAX			// Yes - assume maximum
lbl218:	shl		ECX, 2
		add		EDI, days[ECX-8][ESI*8] // Get days to start of month
		add		EDI, EBX			// Get total days
		dec		EDI
		mov		EAX, EDI

// When get here, c{EAX} = days since 1-Jan-0000 - Now change to ticks
//   since 1-Jan-0001

		sub		EAX, 366
		jg		lbl220
		xor		EAX, EAX
lbl220:	imul	ECX, EAX, 201
		mul		litTICKPERDAY_LO
		add		EDX, ECX
		mov		ecx, time
		mov		[ECX], EAX
		mov		[ECX+4], EDX
	}
}
*/

#pragma warning(default: 4035)		// Turn off the "no return value" warning
