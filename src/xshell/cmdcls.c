//------------------------------------------------------------------------------
//  cmdcls.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

//*******************************************
// Function: cmdcls - Process the CLS command
// Returned: FALSE
//*******************************************

int cmdcls(
	char *args)
{
	args = args;

    svcIoOutString(ccs->cmdout, "\033[2J\033[0;100r", 0);
	return (FALSE);
}
