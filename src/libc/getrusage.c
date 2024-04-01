#include <stdio.h>
#include <string.h>
#include <time.h>
#include <resource.h>

int getrusage(
	int     who,
	RUSAGE *usage)

{
	who = who;

	memset(usage, 0, sizeof(RUSAGE));
	return (0);
}
