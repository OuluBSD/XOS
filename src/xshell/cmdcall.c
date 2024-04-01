//------------------------------------------------------------------------------
//  cmdcall.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

//*********************************************
// Function: cmdcall - Process the CALL command
// Returned: FALSE
//*********************************************

int cmdcall(
	char *args)
{
	args = args;

	cmnderror(0, "The CALL command is not implimented yet");
	return (FALSE);
}
