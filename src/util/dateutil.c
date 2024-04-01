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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
///#include <xoserr.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>


// This program converts between external date-time values and system
//   date-time values (and the reverse) and displays the result. Command
//   format is:
//       dateutil day-month-year
//   or
//       dateutil sysdtlow sysdthigh
//   The day and year values are decimal, the month value is the 3-letter
//   month name. The system date-time values are specified and displayed in
//   hex.

time_x dt;
char   prgname[] = "DATEUTIL";
char   montbl[] = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec";

void badcmd(void);


void main(
    int   argc,
    char *argv[])
{
	char *pnt;
	char *tbl;
	int   cnt;
	long  rtn;
	long  day;
	long  year;

	if (argc != 2 && argc != 3)
		badcmd();
	if (argc == 2)
	{
		day = strtol(argv[1], &pnt, 10);
		if (pnt[0] != '-' || pnt[4] != '-' || day < 1 || day > 31)
			badcmd();
		pnt++;
		tbl = montbl;
		cnt = 12;
		do
		{
			if (strnicmp(tbl, pnt, 3) == 0)
			{
				year = strtol(pnt + 4, &pnt, 10);
				if (pnt[0] != 0)
					badcmd();
				dt.dos.tmx_mday = (ushort)day;
				dt.dos.tmx_mon = (ushort)(((tbl - montbl) / 4) + 1);
				dt.dos.tmx_year = (ushort)year;
				if ((rtn = svcSysDateTime(T_CVDOS2SYS, &dt)) < 0)
					femsg2(prgname, "Error converting to system date-time",
							rtn, NULL);
				printf("System date-time: %08X %08X\n", ((long *)&dt.sys.dt)[0],
						((long *)&dt.sys.dt)[1]);
				exit (0);
			}
			tbl += 4;
		} while (--cnt > 0);
		badcmd();
	}
	((long *)&dt.sys.dt)[0] = strtol(argv[1], &pnt, 16);
	if (pnt[0] != 0)
		badcmd();
	((long *)&dt.sys.dt)[1] = strtol(argv[2], &pnt, 16);
	if (pnt[0] != 0)
		badcmd();

	if ((rtn = svcSysDateTime(T_CVSYS2DOS, &dt)) < 0)
		femsg2(prgname, "Error converting from system date-time", rtn, NULL);
	printf("Date-time: %d-%s-%d %d:%02d:%02d.%03d\n", dt.dos.tmx_mday,
			&montbl[dt.dos.tmx_mon - 1], dt.dos.tmx_year, dt.dos.tmx_hour,
			dt.dos.tmx_min, dt.dos.tmx_sec, dt.dos.tmx_msec);
	exit (0);
}


void badcmd(void)
{
	fputs("? DATEUTIL: Command error, usage is:\n"
			"                dateutil day-mon-year\n"
			"            or\n"
			"                dateutil sysdtlow sysdthigh\n", stderr);
	exit(1);
}
