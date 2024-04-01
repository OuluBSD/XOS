#include <stdio.h>
#include <xos.h>
#include <xostime.h>
#include <uniutil.h>
#include <xossvc.h>


//********************************************
// Function: __xosdt2unixdt - Convert XOS date
//				and time to Unix date and time
// Returned: Unix date and time value
//********************************************

time_t __xosdt2unixdt(
	time_s *ts)

{
	time_x	tmx;

	tmx.sys = *ts;
    svcSysDateTime(T_CVSYS2DOS, &tmx);

/// THIS IS WRONG - IT NEEDS THE YEAR OFFSET!!!!

	return (tmx.dos.tmx_hour * 3600 +
			tmx.dos.tmx_min * 60 + tmx.dos.tmx_sec);
}
