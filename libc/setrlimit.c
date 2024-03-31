#include <stdio.h>
#include <time.h>
#include <resource.h>
#include <xoserr.h>
#include <errno.h>

int setrlimit(
	int           resource,
	RLIMIT const *rlim)
{
	resource = resource;
	rlim = rlim;

	errno = -ER_PRIV;
	return (-1);
}
