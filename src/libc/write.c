#include <stdio.h>
#include <xos.h>
#include <xossvc.h>
#include <errno.h>


//************************************************************
// Function: write - Unix compatable write
// Returned: Amount written or -1 if error (errno is also set)
//************************************************************

int write(
	long  hndl,
	char *buffer,
	int   count)

{
	long rtn;

	if ((rtn = svcIoOutBlock(hndl, buffer, count)) < 0)
	{
		errno = - rtn;
		return (-1);
	}
	return (rtn);
}
