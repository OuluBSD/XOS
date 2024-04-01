/*--------------------------------------------------------------------------*
 * DAYTIME
 * utility to get and set the date and time
 *
 * Written by: Bruce R. Nevins
 *
 * Edit History:
 * 09/04/88(brn) - Created first version
 * 10/28/89(brn) - Removed debug code added standard option handling
 *			and help display
 * 12/04/89(brn) - Make / or - be the option characters
 *		   Add support for procarg dispatch
 * 12/19/89(brn) - Add support for procarg subroutine
 * 06/14/10(jrg) - Major cleanup, almost a rewrite
 *-------------------------------------------------------------------------*/

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

// Command format:
//  DAYTIME
//	Displays current date and time
// or
//  DAYTIME {/A{M}{/Q}{/R}{/S} {DD/MM/YY} {HH:MM:SS}
//	Display or sets the date and or time for the system or realtime clock

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xoserr.h>
#include <xosrtn.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <procarg.h>

#define TIMBFR 40				// Size of the time string buffer
#define MAXSWITNAME 8           // Maximum option name size
#define ENVSWT 20               // Maximum number of environment options

#define uchar  unsigned char
#define ushort unsigned short
#define uint   unsigned int
#define ulong  unsigned long

int     majedt = 4;             // Major edit number
int     minedt = 0;             // Minor edit number

char   monthtbl[48] =
    "DEC\0NOV\0OCT\0SEP\0AUG\0JUL\0JUN\0MAY\0APR\0MAR\0FEB\0JAN";

char   aoption = FALSE;			// TRUE if want AM/PM time display (/12)
char   setsys = FALSE;			// TRUE if /SETSYS specified
char   getrtc = FALSE;			// TRUE if /RTC specified
char   setrtc = FALSE;			// TRUE if /SETRTC specified
char   quiet = FALSE;			// TRUE to suppress info output
char   dateseen = FALSE;		// TRUE if date value specified
char   timeseen = FALSE;		// TRUE if time value specified
char   envname[FILESPCSIZE];	// The environment option name
time_x datetime;				// Date/time value
time_d newdt;					// New date/time value

void  badmonth(void);
void  error(long code, char *msg);
char *getvalue(char   *pnt, ushort *value);
void  help_print(char *help_string, int state, int newline);
int   nonopt(char *arg);
int   opt12(arg_data *);
int   optgetrtc(void);
int   optgetsys(void);
int   optquiet(arg_data *);
int   optsetrtc(void);
int   optsetsys(void);
void  optusage(void);
long  readrtc(time_d *time);

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{   {"?"      , 0, NULL, AF(optusage) , 0},
    {"H"      , 0, NULL, AF(optusage) , 0},
    {"HEL"    , 0, NULL, AF(optusage) , 0},
    {"HELP"   , 0, NULL, AF(optusage) , 0},
    {"12"     , 0, NULL, opt12        , TRUE},
    {"24"     , 0, NULL, opt12        , FALSE},
    {"SYS"    , 0, NULL, AF(optgetsys), 0},
    {"RTC"    , 0, NULL, AF(optgetrtc), 0},
    {"SETSYS" , 0, NULL, AF(optsetsys), 0},
    {"SETRTC" , 0, NULL, AF(optsetrtc), 0},
    {"NOQUIET", 0, NULL, optquiet     , FALSE},
    {"NOQ"    , 0, NULL, optquiet     , FALSE},
    {"QUIET"  , 0, NULL, optquiet     , TRUE},
    {"QUI"    , 0, NULL, optquiet     , TRUE},
    {"Q"      , 0, NULL, optquiet     , TRUE},
    {NULL     , 0, NULL, NULL         , 0}
};

char  prgname[] = "DAYTIME";

int main(
    int   argc,
    char *argv[])
{
    char  daytime[TIMBFR];		// Time and Date string
    long  rtn;
    cchar *foo[2];

    strcpy(envname, "^XOS^");
    strcat(envname, prgname);
    strcat(envname, "^OPT");

    if(svcSysGetEnv(envname, (char const **)&foo[0]) > 0)
    {
		foo[1] = '\0';
		procarg(foo, 0, options, NULL, (int (*)(char *))NULL,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (argc >= 2)
    {
		++argv;
		procarg((cchar **)argv, 0, options, NULL, nonopt,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

	// Here with all arguments processed - now determine exactly what we
	//   need to do:
	//     If /rtc specified, fail if either date or time specified.
	//     If date or time specified and /setrtc, also set /rtc to cause
	//       partial update of the current RTC date/time.
	//     If no date or time specified and /setsys also set /rtc to cause
	//       update from the RTC date/time.


	if (timeseen || dateseen)
	{
	    if (getrtc && (!setrtc || (timeseen && dateseen)))
	    {
    	    fputs("? DAYTIME: Cannot specify date or time with /RTC\n", stderr);
        	exit(1);
    	}
		if (setrtc)
			getrtc = TRUE;
		else
			setsys = TRUE;
	}
	else
	{
		if (setsys)
			getrtc = TRUE;
	}

	// Now do it

	if (!timeseen || !dateseen)			// Do we need the current time?
	{
		if (getrtc)						// Yes - get it
		{
			if ((rtn = readrtc(&datetime.dos)) < 0)
				error(rtn, "Error getting current RTC time");

///			printf("Current RTC: %d/%d/%d\n", datetime.dos.tmx_mon,
///					datetime.dos.tmx_mday, datetime.dos.tmx_year);
		}
		else
		{
			if ((rtn = svcSysDateTime(T_GTDOSDTTM, &datetime.dos)) < 0)
				error(rtn, "Error getting current system date/time");

///			printf("### day: %d\n", datetime.dos.tmx_wday);
///			printf("Current SYS: %d/%d/%d %02d:%02d:%02d.%03d\n",
///					datetime.dos.tmx_mon, datetime.dos.tmx_mday,
///					datetime.dos.tmx_year, datetime.dos.tmx_hour,
///					datetime.dos.tmx_min, datetime.dos.tmx_sec,
///					datetime.dos.tmx_msec);
		}
	}
	if (timeseen)
	{
		datetime.dos.tmx_hour = newdt.tmx_hour;
		datetime.dos.tmx_min = newdt.tmx_min;
		datetime.dos.tmx_sec = newdt.tmx_sec;
		datetime.dos.tmx_msec = 0;
	}
	if (dateseen)
	{
		datetime.dos.tmx_mday = newdt.tmx_mday;
		datetime.dos.tmx_mon = newdt.tmx_mon;
		datetime.dos.tmx_year = newdt.tmx_year;
	}

	// Convert the value we will use to XOS format - this is needed to set the
	//   system time and also verifies that it is a valid value - it is also
	//   needed for the RTC to get the day of the week

	if ((rtn = svcSysDateTime(T_CVDOS2SYS, &datetime)) < 0)
	{
		error(rtn, "Invalid date/time value");
		exit(1);
	}
	if (setsys)
	{
/////
///		printf("%08X %08X\n", (long)(datetime.sys.dt >> 32),
///				(long)datetime.sys.dt);
///		ddt2str(daytime, (aoption)? "%z%T:%m:%s %A on %w, %D-%3n-%L" :
///				"%z%h:%m:%s on %w, %D-%3n-%L", (time_dz *)&datetime.dos);
///		printf("DAYTIME: Setting to (DOS) %s\n", daytime);
///		sdt2str(daytime, (aoption)? "%z%T:%m:%s %A on %w, %D-%3n-%L" :
///				"%z%h:%m:%s on %w, %D-%3n-%L", (time_sz *)&datetime.sys);
///		printf("DAYTIME: Setting to (SYS) %s\n", daytime);
/////
		if ((rtn = svcSysDateTime(T_STSYSDTTM, &datetime.sys)) < 0)
			error(rtn, "Error setting system clock");

		if ((rtn = svcSysDateTime(T_GTDOSDTTM, &datetime.dos)) < 0)
			error(rtn, "Error getting updated system date/time");
		if (!quiet)
		{
			ddt2str(daytime, (aoption)? "%z%T:%m:%s %A on %w, %D-%3n-%L":
					"%z%h:%m:%s on %w, %D-%3n-%L", (time_dz *)&datetime.dos);
			printf("DAYTIME: System date/time set to %s\n", daytime);
		}
	}
	if (setrtc)
	{
		svcSysDateTime(T_CVSYS2DOS, &datetime); // Convert back to DOS format
												//   to get the day of the week
		if ((rtn = svcSysDateTime(T_STRTCDTTM, &datetime.dos)) < 0)
			error(rtn, "Error setting real time clock");
		if ((rtn = readrtc(&datetime.dos)) < 0)
			error(rtn, "Error getting updated RTC date/time");
		if (!quiet)
		{
			ddt2str(daytime, (aoption)? "%z%T:%m:%s %A on %w, %D-%3n-%L" :
					"%z%h:%m:%s on %w, %D-%3n-%L", (time_dz *)&datetime.dos);
            printf("DAYTIME: RTC date/time set to %s\n", daytime);
		}
	}
	if (!setsys && !setrtc)
	{
///		printf("### day: %d\n", datetime.dos.tmx_wday);

		ddt2str(daytime, (aoption)? "%z%T:%m:%s %A on %w, %D-%3n-%L" :
				"%z%h:%m:%s on %w, %D-%3n-%L", (time_dz *)&datetime.dos);
		printf("DAYTIME: %s date/time is %s\n", (getrtc) ? "RTC" : "System",
				daytime);
	}
    return (0);
}


// Function to read real time clock

long readrtc(
   time_d *time)
{
	long rtn;
	int  cnt;

	cnt = 4;
	do
	{
		if ((rtn = svcSysDateTime(T_GTRTCDTTM, time)) > 0 || rtn != ER_BUSY)
			return (rtn);
		svcSchSuspend(NULL, ST_SECOND/10);
	} while (--cnt > 0);
	return (ER_BUSY);
}


// Function to process non-option input

int nonopt(
	char *pntc)
{
	long *pntm;
	char *pntn;
	int   cnt;
	char  month[4];
	char  chr;

    if (strchr(pntc, '-') != NULL)
    {
        pntc = getvalue(pntc, &newdt.tmx_mday) + 1;
		if (newdt.tmx_mday < 1 || newdt.tmx_mday > 31)
    	{
    	    fputs("? DAYTIME: Bad day value\n", stderr);
    	    exit(1);
    	}
        pntn = month;
        cnt = 3;
        do
        {   if ((chr = *pntc++) == '\0')
                badmonth();
            *pntn++ = toupper(chr);
        } while (--cnt > 0);
        if (*pntc++ != '-')
            badmonth();
        *pntn = '\0';
        pntm = (long *)monthtbl;
        newdt.tmx_mon = 12;
        do
        {   if (*((long *)month) == *pntm)
                break;
            pntm++;
        } while (--newdt.tmx_mon > 0);
        if (newdt.tmx_mon == 0)
            badmonth();
        pntc = getvalue(pntc, &newdt.tmx_year);
        if (newdt.tmx_year < 80)
            newdt.tmx_year += 2000;
    	else if (newdt.tmx_year < 100)
    	    newdt.tmx_year += 1900;
    	if (newdt.tmx_year > 3000 || *pntc != '\0')
    	{
            fputs("? DAYTIME: Bad year value\n", stderr);
            exit(1);
    	}
    	dateseen = TRUE;
    }
    else if (strchr(pntc, ':') != NULL)
    {
    	newdt.tmx_hour = atoi(pntc);	/* Get hour value */
    	while (*pntc != ':' && *pntc != '\0')
    	{
    	    if (!isdigit(*pntc++) || newdt.tmx_hour > 23)
    	    {
				fprintf(stderr, "? %s: Bad hour value\n", prgname);
				exit(EXIT_INVSWT);
    	    }
    	}
    	if (*pntc == '\0')
    	{
    	    fprintf(stderr, "? %s: Bad time format\n", prgname);
    	    exit(EXIT_INVSWT);
    	}
    	newdt.tmx_min = atoi(++pntc);	/* Get minute value */
		if (newdt.tmx_min > 59)
    	{
    	    fprintf(stderr, "? %s: Bad minutes value\n",
    	    		prgname);
    	    exit(EXIT_INVSWT);
    	}
    	while (*pntc != ':' && *pntc != '\0')
    	{
    	    if (!isdigit(*pntc))
    	    {
				if (*pntc == 'A' && *(pntc +1) == 'M')
				{
					pntc += 2;	/* Skip the two characters */
					if (newdt.tmx_hour > 12)
					{
						fprintf(stderr, "? %s: Cannot use AM with 24 hour "
								"time\n", prgname);
						exit(EXIT_INVSWT);
					}
				}
				else if (*pntc == 'P' && *(pntc +1) == 'M')
				{
					pntc += 2;	/* Skip the two characters */
					if (newdt.tmx_hour > 12)
					{
						fprintf(stderr, "? %s: Cannot use PM with 24 hour "
								"time\n", prgname);
						exit(EXIT_INVSWT);
					}
					newdt.tmx_hour += 12;
				}
				else
				{
					fprintf(stderr, "? %s: Bad minutes value\n", prgname);
					exit(EXIT_INVSWT);
				}
    	    }
    	    else
    		pntc++;
    	}
    	if (*pntc == '\0')
    	    newdt.tmx_sec = 0;
    	else
    	{
    	    newdt.tmx_sec = atoi(++pntc);
	    if (newdt.tmx_sec > 59)
    	    {
    	    	fprintf(stderr, "? %s: Bad seconds value\n",
    	    			prgname);
    	    	exit(EXIT_INVSWT);
    	    }
    	    while (*pntc != '\0')
    	    {
				if (!isdigit(*pntc))
				{
					if (*pntc == 'A' && *(pntc +1) == 'M')
					{
						pntc += 2;		// Skip the two characters
						if (newdt.tmx_hour > 12)
						{
							fprintf(stderr, "? %s: Cannot use AM with 24 hour "
									"time\n", prgname);
							exit(EXIT_INVSWT);
						}
					}
					else if (*pntc == 'P' && *(pntc +1) == 'M')
					{
						pntc += 2;		// Skip the two characters
						if (newdt.tmx_hour > 12)
						{
							fprintf(stderr, "? %s: Cannot use PM with 24 hour "
									"time\n", prgname);
							exit(EXIT_INVSWT);
						}
						newdt.tmx_hour += 12;
					}
					else
					{
						fprintf(stderr, "? %s: Bad seconds value\n", prgname);
						exit(EXIT_INVSWT);
					}
				}
				else
					pntc++;
    	    }
    	}
    	timeseen = TRUE;
    }
    else
    {
        fprintf(stderr, "? %s: Bad date/time value, %s\n", prgname, pntc);
        exit(EXIT_INVSWT);
    }
    return (TRUE);
}

void badmonth(void)

{
    fputs("? DAYTIME: Bad month name\n", stderr);
    exit(1);
}

char *getvalue(
    char   *pnt,
    ushort *value)
{
    ushort temp;
    char   chr;

    temp = 0;
    while (isdigit((chr = *pnt)))
    {
        temp = (temp * 10) + (chr & 0xF);
        pnt++;
    }
    if (chr != '\0' && chr != '-')
        temp = 0xFFFF;
    *value = temp;
    return (pnt);
}

/*
 * opt12 - function to process -12 option
 */

int opt12(
    arg_data *arg)

{
    aoption = arg->data;
    return (TRUE);
}

/*
 * optusage - Function to print the usage message
 */

void optusage(void)

{
    fprintf(stderr, "\nDAYTIME version %d.%d (%s)\n\n", majedt, minedt,
            __DATE__);
    fputs("DAYTIME {{/}{-}option} {dd-mmm-yy} {hh:mm:ss}\n", stderr);
    fputs("\nOptions:\n", stderr);
    help_print("  /12         - Display time in 12 hour format", aoption, TRUE);
    help_print("  /24         - Display time in 24 hour format", !aoption,
            TRUE);
    help_print("  /HELP or /? - Display this message", FALSE, TRUE);
    help_print("  /{NO}QUIET  - Don't display time and date", quiet, TRUE);
    help_print("  /SYS        - Display XOS system date and time", !getrtc,
			TRUE);
    help_print("  /RTC        - Display real time (CMOS) clock date and time",
		getrtc, TRUE);
    help_print("  /SETSYS     - Set XOS system date and/or time", setsys,
			TRUE);
    help_print("  /SETRTC     - Set real time (CMOS) clock date and/or time",
			setrtc, TRUE);
    fputs("A * shows the option is the current default. Use the DEFAULT "
			"command to \n  change these defaults.\n", stderr);
    fputs("\nTime is specified as hh:mm:ss and dates as dd-mmm-yy.  mmm is the"
            " 3 character\n  name of the month.  The year may be 2 or 4 "
			"digits. If two digits, values\n  between 80 and 99 mean 19yy and "
			"values less than 80 mean 20yy\n", stderr);
    fputs("/SYS and /SETRTC may be used together to set the real time"
            " clock date and\n  time to be the same as the system date and "
			"time\n", stderr);
    fputs("/RTC and /SETSYS may be used together to set the system date and"
            " time to be\n  the same as the real time clock date and time\n",
			stderr);
    exit(0);
}

/*
 * Function to process /GETRTC option
 */

int optgetrtc(void)

{
    getrtc = TRUE;
    return (TRUE);
}

/*
 * Function to process /SETRTC option
 */

int optsetrtc(void)

{
    setrtc = TRUE;
    return (TRUE);
}

/*
 * Function to process /GETSYS option
 */

int optgetsys(void)

{
    getrtc = FALSE;
    return (TRUE);
}

/*
 * Function to process /SETSYS option
 */

int optsetsys(void)

{
    setsys = TRUE;
    return (TRUE);
}

/*
 * Function to proces /QUIET or /NOQUIET option
 */

int optquiet(arg_data *arg)

{
    quiet = arg->data;
    return (TRUE);
}

/*
 * Function to report fatal error
 */

void error(code, msg)
long  code;
char *msg;

{
    char buffer[100];

    svcSysErrMsg(code, 3, buffer);	/* Get error message string */
    fprintf(stderr, "? DAYTIME: %s\n           %s\n", msg, buffer);
    exit(1);
}

/*
 * help_print - Function to print help option entries
 */

void help_print(char *help_string, int state, int newline)

{
    char str_buf[132];

    strcpy(str_buf, help_string);
    if (state)
        strcat(str_buf, " *");
    if (newline)
        fprintf(stderr, "%s\n", str_buf);
    else
        fprintf(stderr, "%-38s", str_buf);
}
