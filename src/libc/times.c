#include <stdio.h>
#include <time.h>
#include <times.h>


clock_t	times(
	TMS *buffer)

{
	buffer->tms_utime = 0;
	buffer->tms_cstime = 0;
	buffer->tms_cutime = 0;
	buffer->tms_stime = 0;
	return (0);
}
