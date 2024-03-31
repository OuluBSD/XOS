#include <stdio.h>
#include <xos.h>
#include <xossvc.h>
#include <errno.h>


//*********************************************************
// Function: read - Unix compatable read
// Returned: Amount read or -1 if error (errno is also set)
//*********************************************************

int read(
	long  hndl,
	char *buffer,
	int   count)

{
	long rtn;

	if ((rtn = svcIoInBlock(hndl, buffer, count)) < 0)
	{
		if (rtn == ER_EOF)
			return (0);
		errno = rtn;
		return (EOF);
	}
	return (rtn);
}
