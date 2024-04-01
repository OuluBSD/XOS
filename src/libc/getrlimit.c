#include <stdio.h>
#include <time.h>
#include <resource.h>

int getrlimit(
	int     resource,
	RLIMIT *rlim)

{
	resource = resource;

	rlim->rlim_cur = 0;					// Current (soft) limit
	rlim->rlim_max = 0;					// Maximum value for rlim_cur
	return (0);
}
