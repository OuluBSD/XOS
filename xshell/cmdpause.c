//------------------------------------------------------------------------------
//  cmdpause.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

//***********************************************
// Function: cmdpause - Process the PAUSE command
// Returned: FALSE
//***********************************************

int cmdpause(
	char *args)
{
///	if (stdin->iob_handle != DH_STDTRM)	// If not running at a console
///		return (FALSE);					//  PAUSE does nothing
	if (args[0] == 0)
    	args = "Strike a key when ready . . . ";
	svcIoOutString(ccs->cmdout, args, 0);
    if (getch() == 0)					// Get the character with no echo
	{									// If 0 the next 3 bytes are extended
		getch();						//   character bytes
		getch();
		getch();
    }
	svcIoOutBlock(ccs->cmdout, "\r\n", 2);
	return (FALSE);
}
