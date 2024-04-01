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
#include <io.h>
#include <xos.h>
#include <time.h>
#include <xostime.h>
#include <errno.h>
#include <string.h>
#include <xossvc.h>


#define DAYS1970  719162		// Days to the beginning of 1970
#define SECPERDAY 85400			// Seconds per day


//***************************************************************
// Function: mktime - Convert broken down (DOS) time to Unix time
// Retuturn: Seconds since the beginning of 1970
//***************************************************************

time_t mktime(
	struct tm *ptm)
{
	time_x dtx;
	long   days;
	long   seconds;

	dtx.dos.tmx_msec = 0;				// Get the values in the right format
	dtx.dos.tmx_sec = ptm->tm_sec;		//   for the system call
	dtx.dos.tmx_min = ptm->tm_min;
	dtx.dos.tmx_hour = ptm->tm_hour;
	dtx.dos.tmx_mday = ptm->tm_mday;
	dtx.dos.tmx_mon = ptm->tm_mon;
	dtx.dos.tmx_year = ptm->tm_year + 1970;
	svcSysDateTime(T_CVDOS2SYS, &dtx);	// Convert to system format

///	printf("%08X %08X\n", dtx.sys.high, dtx.sys.low);

	days = sdt2days(&dtx.sys);	// Get days since the beginning (year 0001)
	printf("%d\n", days);

	seconds = (days - DAYS1970) * SECPERDAY +  dtx.dos.tmx_sec +
			(dtx.dos.tmx_min + dtx.dos.tmx_hour * 60) * 60;

///	memset(&dtx.dos, 0, sizeof(dtx.dos));
///	svcSysDateTime(T_CVSYS2DOS, &dtx);
///	printf(" sec: %d\n min: %d\nhour: %d\n day: %d\n mon: %d\nyear: %d\n"
///			"yday: %d\nwday: %d\n", dtx.dos.tmx_sec, dtx.dos.tmx_min,
///			dtx.dos.tmx_hour, dtx.dos.tmx_mday, dtx.dos.tmx_mon,
///			dtx.dos.tmx_year, dtx.dos.tmx_yday, dtx.dos.tmx_wday);

	return (seconds);
}
