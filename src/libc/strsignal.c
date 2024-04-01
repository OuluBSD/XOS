#include <stdio.h>

char *strsignal(
	int signum)

{
	static char bufr[20];

	sprintf(bufr, "Signal #%d", signum);
	return (bufr);
}
