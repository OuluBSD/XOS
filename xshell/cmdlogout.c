//------------------------------------------------------------------------------
//  cmdlogout.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

//*************************************************
// Function: cmdlogout - Process the LOGOUT command
// Returned: FALSE
//*************************************************

int cmdlogout(
	char *args)
{
	args = args;

	cmnderror(0, "The LOGOUT command is not implimented yet");
	return (FALSE);
}
