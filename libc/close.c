#include <stdio.h>
#include <xos.h>
#include <xossvc.h>
#include <errno.h>


//***********************************************
// Function: close - Unix compatable close
// Returned: 0 or -1 if error (errno is also set)
//***********************************************

int close(
	long  hndl)

{
	long rtn;

	if ((rtn = svcIoClose(hndl, 0)) < 0)
	{
		errno = - rtn;
		return (-1);
	}
	return (rtn);
}
