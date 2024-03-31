//------------------------------------------------------------------------------
//  cmddelay.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

//***********************************************
// Function: cmddelay - Process the DELAY command
// Returned: FALSE
//***********************************************

int cmddelay(
	char *args)
{
	long amount;

	if (getonenum(args, &amount))
	{
		if (amount > 100000)
			amount = 100000;
		svcSchSuspend(NULL, amount * ST_MILLISEC); 
	}
	return (FALSE);
}
