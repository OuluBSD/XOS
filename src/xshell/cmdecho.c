//------------------------------------------------------------------------------
//  cmdecho.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

//*********************************************
// Function: cmdecho - Process the ECHO command
// Returned: FALSE
//*********************************************

int cmdecho(
	char *args)
{
	int len;

	len = strlen(args);

	if (len == 3 && stricmp(args, "OFF") == 0)
		ccs->echo = FALSE;
	else if (len == 2 && stricmp(args, "ON") == 0)
		ccs->echo = TRUE;
	else
	{
		args[len] = '\r';
		args[len + 1] = '\n';
		svcIoOutBlock(ccs->cmdout, args, len + 2);
	}
	return (FALSE);
}
