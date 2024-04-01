// Unix compatable gmtime function

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct tm *gmtime(
	time_t const *time)

{
	time = time;

	fputs("\n?\!!!!!!!!!!! called gmtime !!!!!!!!!!\n", stderr);
	exit (1);

	return (0);
}
