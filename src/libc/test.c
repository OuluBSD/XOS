#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <xoserr.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>


void vasub(char *fmt, ...);


int main( )
{
	vasub("%d %d %d", 1, 2, 3);
	return (0);
}


void vasub(
	char *fmt, ...)
{
	va_list ap;
	char    buffer[100];

	va_start(ap, fmt);

	vsprintf(buffer, fmt, ap);
}
