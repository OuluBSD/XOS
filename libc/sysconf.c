#include <stdio.h>


static int sctbl[] =
{	65536,			// _SC_ARG_MAX     = 1
	1024,			// _SC_CHILD_MAX   = 2
	50,				// _SC_CLK_TCK     = 3
	0,				// _SC_JOB_CONTROL = 4
	0,				// _SC_NGROUPS_MAX = 5
	1024,			// _SC_OPEN_MAX    = 6
	0,				// _SC_SAVED_IDS   = 7
	1024,			// _SC_STREAM_MAX  = 8
	0,				// _SC_TZNAME_MAX  = 9
	0				// _SC_VERSION     = 10
}; 


int sysconf(
	int name)

{
	return ((name < 1 || name > 10) ? -1 : sctbl[name - 1]);
}
