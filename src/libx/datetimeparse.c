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
#include <xostime.h>
#include <xosstr.h>

//=========================================//
//                                         //
// Functions to parse date and time values //
//                                         //
//=========================================//

// Local Function prototypes

static long getvalue(char **ppnt);



//****************************************************************//
// Function: dtparse - Parse string containing both date and time //
// Returned: date/time return value                               //
//****************************************************************//

char *dtparse(
	char   *bufr,
	time_s *dt)
{




	return (NULL);
}


//***********************************************************//
// Function: dtparsetime - Parse string containing only time //
// Returned: Pointer to character after time value if value  //
//             is valid, NULL if invalid value               //
//***********************************************************//

// Valid time formats is:
//   hh:mm{:ss}{ }{A|AM|P|PM}

char *dtparsetime(
	char   *bufr,
	time_d *dtd)
{
	long hour;
	long minute;
	long second;
	char chr;

	hour = getvalue(&bufr);
	if (*bufr != ':')
		return (NULL);
	bufr++;
	minute = getvalue(&bufr);
	second = (*bufr == ':') ? (bufr++, getvalue(&bufr)) : 0;
	while (isspace(*bufr))
		bufr++;
	if ((chr = toupper(*bufr)) != 0)
	{
		if (chr == 'A')
		{
			bufr++;
			if (bufr[0] != 0 && toupper(bufr[0]) == 'M')
				bufr++;
		}
		else if (chr == 'P')
		{
			bufr++;
			if (bufr[0] != 0 && toupper(bufr[0]) != 'M')
				bufr++;
			hour += 12;
		}
	}
	if (hour > 23 || minute > 59 || second > 59)
		return (NULL);
	dtd->tmx_hour = hour;
	dtd->tmx_min = minute;
	dtd->tmx_sec = second;
	dtd->tmx_msec = 0;
	return (bufr);
}


//*******************************************************//
// Function: dtparse - Parse string containing only date //
// Returned: Pointer to character after date value if    //
//             value is valid, NULL if invalid value     //
//*******************************************************//

char *dtparsedate(
	char   *bufr,
	time_d *dt)
{
	static long mtbl[12] = {'DEC', 'NOV', 'OCT', 'SEP', 'AUG', 'JUL',
							'JUN', 'MAY', 'APR', 'MAR', 'FEB', 'JAN'};

	long  *mpnt;
	char  *pnt;
	union
	{	long l;
		char c[4];
	}      name;
	time_x dtx;

	// First determine which format we have.

	pnt = bufr;
	while (*pnt != 0)
	{
		switch (*pnt)
		{
		 case '-':						// If dd-mmm-yyyy
			dtx.dos.tmx_mday = getvalue(&bufr);
			if (bufr[0] != '-' || bufr[4] != '-')
				return (NULL);
			name.c[0] = toupper(bufr[1]);
			name.c[1] = toupper(bufr[2]);
			name.c[2] = toupper(bufr[3]);
			name.c[3] = 0;
			mpnt = mtbl;
			dtx.dos.tmx_mon = 12;
			while (*mpnt != name.l)
			{
				if (dtx.dos.tmx_mon == 0)
					return (NULL);
				dtx.dos.tmx_mon--;
				mpnt++;
			}
			bufr += 5;
			goto dtcom;

		 case '/':						// If mm/dd/yyyy
			dtx.dos.tmx_mon = getvalue(&bufr);
			if (bufr[0] != '/')
				return (NULL);
			bufr++;
			dtx.dos.tmx_mday = getvalue(&bufr);
			if (bufr[0] != '/')
				return (NULL);
			bufr++;
			goto dtcom;

		 case '.':						// If dd.mm.yyyy
			dtx.dos.tmx_mday = getvalue(&bufr);
			if (bufr[0] != '.')
				return (NULL);
			bufr++;
			dtx.dos.tmx_mon = getvalue(&bufr);
			if (bufr[0] != '.')
				return (NULL);
			bufr++;
		 dtcom:
			if ((dtx.dos.tmx_year = getvalue(&bufr)) < 100)
				dtx.dos.tmx_year += 2000;
			if (dtx.dos.tmx_mday < 1 || dtx.dos.tmx_mday > 31 ||
				dtx.dos.tmx_mon < 1 || dtx.dos.tmx_mon > 12)
				return (NULL);
			return (bufr);
		}
		pnt++;
	}
	return (NULL);
}


//====================================//
// All functions below this are local //
//====================================//



static long getvalue(
	char **ppnt)
{
	char *pnt;
	long  value;
	long  chr;

	pnt = *ppnt;
	value = 0;
	while (isspace(*pnt))
		pnt++;
	while (isdigit((chr = *pnt)))
	{
		pnt++;
		value = value * 10 + (chr & 0x0F);
	}
	*ppnt = pnt;
	return (value);
}


