#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
///#include <process.h>
///#include <dos.h>
///#include <io.h>
#include <time.h>
#include "xcstring.h"
#include "xosstuff.h"
///#include "xoserr.h"
///#include "scaled.h"
#include "swcwindows.h"
///#include "extdropdown.h"
///#include "swcserver.h"
///#include "xosqdes.h"
///#include "fstring.h"
///#include "errorwindow.h"
///#include "cancelling.h"
///#include "swcclient16.h"
///#include "obtaining.h"
///#include "scancodes.h"
#include "datetime.h"

static char *spnt;
static char  names[] = "JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";

static long get2dig(char *str);
static int  getchr(void);
static long getdec(int stopper);
static long getdec(int stopper1, int stopper2);
static long gettime(struct tm *dt, char *str);
static long getyear(int offset);


//******************************************************
// Function: standarddate - Generate string with date in
//					standard format for system
// Returned: Length of string generated
//******************************************************

int standarddate(
	char        *str,			// Buffer for date
	swcDateTime *date,
	int          options)		// STDDATE_SUPPRESS = 0x02 (suppress leading
								//   space)
								// STDDATE_FULLYEAR = 0x01 (display 4 digit
								//   year)
{
	return (displaydate(str, date, 0, options));
}


//*****************************************************
// Function: displaydate - Generate string with date in
//				specified format
// Returned: Length of string generated
//*****************************************************

int displaydate(
	char        *str,			// Buffer for date
	swcDateTime *date,
	int          format,		// Date format
								//   0: System default
								//   1: dd-mmm-yy{yy}
								//   2: mm/dd/yy{yy}
								//   3: mmddyy{yy}
								//   4: dd/mm/yy{yy}
								//   5: ddmmyy{yy}
	int   options)				// STDDATE_SUPPRESS = 0x02 (suppress leading
								//   space)
								// STDDATE_FULLYEAR = 0x01 (display 4 digit
								//   year)

{
	tm  tm;
	int rtn;

	if (*date == 0 || *date >= DATETIME_MAX)
	{
		strcpy(str, "None");
		return (4);
	}
	date->ConvDateToDos(&tm);
	switch (format)
	{
	 default:					// dd-mmm-yy{yy}
		rtn = strftime(str, 16, (options & STDDATE_FULLYEAR) ? "%d-%b-%Y" :
				"%d-%b-%y", &tm);
		if (str[0] == '0')
		{
			if (options & STDDATE_SUPPRESS)
			{
				strcpy(str, str + 1);
				rtn--;
			}
			else
				str[0] = ' ';
		}
		return (rtn);

	 case 2:					// mm/dd/yy{yy}
		return (strftime(str, 16, (options & STDDATE_FULLYEAR) ? "%m/%d/%Y" :
				"%m/%d/%y", &tm));

	 case 3:					// mmddyy{yy}
		return (strftime(str, 16, (options & STDDATE_FULLYEAR) ? "%m%d%Y" :
				"%m%d%y", &tm));

	 case 4:					// dd/mm/yy{yy}
		return (strftime(str, 16, (options & STDDATE_FULLYEAR) ? "%d/%m/%Y" :
				"%d/%m/%y", &tm));

	 case 5:					// ddmmyy{yy}
		return (strftime(str, 16, (options & STDDATE_FULLYEAR) ? "%d%m%Y" :
				"%d%m%y", &tm));
	}
}

//*************************************************
// Function: parsedate - Parse date string
// Returned: Days since 1-1-0001 if OK, -1 if error
//*************************************************

long parsedate(
    char        *str,
	int          yropt,
	int          offset)

{
	swcDateTime dt;
	struct tm   tmx;

	if (!getdate(&tmx, str, yropt, offset))
		return (false);
	dt.ConvDateFromDos(&tmx);
    return (dt.ConvToDays());
}




long getdatetime(
	struct tm *tmx,
	char      *str,
	int        offset)

{
	char *pnt;
	int   len;
	char  chr;
	char  bufr[32];

	while ((chr = *str) != 0 && isspace(*str)) // Find first non-space character
		str++;
	if (chr == 0)
		return (0);
	pnt = str;							// Find first space character
	while ((chr = *pnt) != 0 && chr != ':' && !isspace(*pnt))
		pnt++;
	if (chr == ':')						// If found a colon here we must have
	{									//   only a time value
		if (!gettime(tmx, str))			// Convert the time value
			return (-1);
		return (2);
	}
	if ((len = pnt - str) > 31)
		return (-1);
	memcpy(bufr, str, len);
	bufr[len] = 0;
	if (!getdate(tmx, bufr, false, offset)) // Convert the date value
		return (-1);
	while ((chr = *pnt) != 0 && isspace(*pnt)) // Find first non-space
		pnt++;								   //   character which is the
											   //   start of the time value
	if (chr == 0)						// If we only have a date value
		return (1);
	if (!gettime(tmx, pnt))				// Convert the time value
		return (-1);
	return (3);
}


//***********************************************************************
// Function: parsedatetime - Parse a text string containing date and time
// Returned: -1 if error, 0, if null, 1 if have date only, 2 if have time
//				only, 3 if have both
//***********************************************************************

long parsedatetime(
	swcDateTime *dt,
	char        *str,
	int          offset)

{
	swcDateTime dt2;
	char       *pnt;
	tm          tmx;
	int         len;
	char        chr;
	char        bufr[32];

	while ((chr = *str) != 0 && isspace(*str)) // Find first non-space character
		str++;
	if (chr == 0)
		return (0);
	pnt = str;							// Find first space character
	while ((chr = *pnt) != 0 && chr != ':' && !isspace(*pnt))
		pnt++;
	if (chr == ':')						// If found a colon here we must have
	{									//   only a time value
		if (!gettime(&tmx, str))		// Convert the time value
			return (-1);
		dt->ConvTimeFromDos(&tmx);
		return (2);
	}
	if ((len = pnt - str) > 31)
		return (-1);
	memcpy(bufr, str, len);
	bufr[len] = 0;
	if (!getdate(&tmx, bufr, false, offset)) // Convert the date value
		return (-1);
	dt->ConvDateFromDos(&tmx);
	while ((chr = *pnt) != 0 && isspace(*pnt)) // Find first non-space
		pnt++;								   //   character which is the
											   //   start of the time value
	if (chr == 0)						// If we only have a date value
		return (1);
	if (!gettime(&tmx, pnt))			// Convert the time value
		return (-1);
	dt2.ConvTimeFromDos(&tmx);
	*dt += dt2;
	return (3);
}


//******************************************************
// Function: getdate - Parse a date string
// Returned: true if valid date, false if not valid date
//******************************************************

// Format can be one of the following:

// dd-mmm-yy or dd-mmm-yyyy
//   where: dd   = 1 or 2 digit day of month
//			mmm  = 3 character month name
//			yy   = 2 digit year (50 year window about 2010)
//			yyyy = 4 digit year

// mm/dd/yy or mm/dd/yyyy
//   where:	mm   = 1 or 2 digit month (1 to 12)
//			dd   = 1 or 2 digit day of month
//			yy   = 2 digit year (50 year window about 2010)
//			yyyy = 4 digit year

// mmddyy or mmddyyyy
//   where:	mm   = 2 digit month (01 to 12)
//			dd   = 2 digit day of month
//			yy   = 2 digit year (50 year window about 2010)
//			yyyy = 4 digit year

int getdate(
	struct tm *dt,
	char      *str,
	int        yropt,
	int        offset)

{
	char  *npnt;
	long   value;
	char   atom[4];

	// First try the dd-mmm-yy{yy} format (we reconize this if we find an
	//   initial decimal value <= 31 which is terminated by a '-'

	spnt = str;
	if ((value = getdec('-')) >= 0 && value <= 31)
	{
		memset(dt, 0, sizeof(struct tm));
		dt->tm_mday = value;
		atom[0] = (char)getchr();
		atom[1] = (char)getchr();
		atom[2] = (char)getchr();
		if (getchr() != '-')
		{
			if (!yropt)
				return (false);
			dt->tm_year = 0;
		}
		else if ((dt->tm_year = getyear(offset)) < 0)
			return (false);
		npnt = names;
		value = 0;
		do
		{
			if (strnicmp(atom, npnt, 3) == 0)
				break;
			npnt += 3;
		} while (++value <= 11);
		if (value > 12)
			return (false);
		dt->tm_mon = value;
		return (true);
	}

	// Second try the mm/dd/yy{yy} format - we reconize this by an initial
	//   decimal value > 0 and <= 12 terminated by '/'

	spnt = str;
	if( (value = getdec( '/')) > 0 && value <= 12)
	{
		memset(dt, 0, sizeof(struct tm)); // Collect the 'dd' value
		dt->tm_mon = value - 1;

		str = spnt;
		if( (dt->tm_mday = getdec('/')) <= 0)
		{
			if (yropt)
			{
				spnt = str;
				if ((dt->tm_mday = getdec(0)) <= 0)
					return (false);
				dt->tm_year = 0;
			}
		}
		else if ((dt->tm_year = getyear(offset)) < 0)
				return (false);
		if (dt->tm_mday > 31)
			return (false);
		return (true);
	}

	// Finally try the mmddyy{yy} format - we reconize this if we have a 6 or
	//   8 character string containing only digits

	if ((value = strlen(str)) != 6 && value != 8 && (!yropt || value != 4))
		return (false);
	memset(dt, 0, sizeof(struct tm)); // Collect the 'dd' value
	if ((dt->tm_mon = get2dig(str) - 1) < 0)
		return (false);
	if ((dt->tm_mday = get2dig(str + 2)) < 0)
		return (false);
	spnt = str + 4;
	if (value > 4)
	{
		if ((dt->tm_year = getyear(offset)) < 0)
			return (false);
	}
	else
		dt->tm_year = 0;
	return (true);
}


static long gettime(
	struct tm *dt,
	char      *str)

{
	long value;

	spnt = str;
	if ((value = getdec(':')) < 0 || value > 23)
		return (false);
	dt->tm_hour = value;
	if ((value = getdec(':', 0)) < 0 || value > 59)
		return (false);
	dt->tm_min = value;
	if ((value = getdec(0)) < 0 || value > 59)
		return (false);
	dt->tm_sec = value;
	return (true);
}


//***********************************************************************
// Function: getyear - Get the year in DOS tm format (actual year - 1900)
// Returned: DOS tm year value or -1 if error
//***********************************************************************

static long getyear(
	int offset)

{
	swcDateTime dt;
	struct tm   tm;
	long        value;

	static long thisyear = 0;
	static long thiscentury;
	static long thisyearincen;

	if ((value = getdec(0)) < 0)
		return (-1);
	if (value < 1600)
	{
		if (value > 99)
			return (-1);
		if (thisyear == 0)
		{
			dt.GetLocal();
			dt.ConvDateToDos(&tm);
			thisyear = tm.tm_year + 1900;
			thisyearincen = thisyear % 100;
			thiscentury = thisyear - thisyearincen;
		}
		if ((offset += thisyearincen) >= 100)
		{
			if (value <= (offset - 100))
				value += 100;
		}
		else
		{
			if (value > offset)
				value -= 100;
		}
		value += thiscentury;
	}
	if ((value -= 1900) < 0)
		value = -1;
	return (value);
}


static long getdec(
	int stopper)

{
	long value;
	char chr;

	value = 0;
	while ((chr = (char)getchr()) != 0 && isdigit(chr))
	{
		if ((value = 10 * value + (chr & 0x0F)) > 5000)
			return (-1);
	}
	return ((chr == stopper) ? value : -1);
}


static long getdec(
	int stopper1,
	int stopper2)

{
	long value;
	char chr;

	value = 0;
	while ((chr = (char)getchr()) != 0 && isdigit(chr))
	{
		if ((value = 10 * value + (chr & 0x0F)) > 5000)
			return (-1);
	}
	return ((chr == stopper1 || chr == stopper2) ? value : -1);
}


static long get2dig(
	char *str)

{
	if (!isdigit(str[0]) || !isdigit(str[1]))
		return 0;
	return ((str[0] & 0x0F) * 10 + (str[1] & 0x0F));
}


static int getchr(void)

{
	char chr;

	if ((chr = *spnt) != 0)
		spnt++;
	return (chr);
}


void convolddatetime(
	swcDateTime *dt,
	llong        oldval)

{


}


#pragma warning(default: 4035)		// Turn off the "no return value" warning
