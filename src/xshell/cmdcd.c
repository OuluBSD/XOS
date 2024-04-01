//------------------------------------------------------------------------------
//  cmdcd.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

// The CD command sets or displays the path associated with a device.
//   If no argument is specified, the path for the current device is displayed.
//   If only a device name is specified (last character is a colon), the path
//     for that device is displayed. 
//   If only a path is specified (no colon in the string) the path for the
//     current device is set. Nothing is displayed.
//   If both a device and path is specified, the path for the specified device
//     is set. Nothing is displayed.
// The path is sticky across batch files. If it is changed in a batch file,
//   that change is still in affect after the batch file terminates.

//*****************************************
// Function: cmdcd - Process the CD command
// Returned: FALSE
//*****************************************

int cmdcd(
	char *args)
{
    char *pnt;
	char *endpnt;
	long  opt;
	long  rtn;
	char  buffer[300];
	char  text[360];

	if ((args = getonearg(args)) == NULL)
		return (FALSE);
	if (args[0] == 0)					// No arguments?
		args = "Z:";					// Yes - display path for current device
	if ((pnt = strchr(args, ':')) == NULL) // Path only (no colon)?
	{
		// We know there are at least 3 bytes available before the start of
		//   the path string ("CD ") so we cheat and put "Z:" before the
		//   string!

		args[-2] = 'Z';
		args[-1] = ':';
		args -= 2;
	}
	endpnt = strnmov(buffer, args, 298);
	buffer[298] = 0;
	opt = POPT_GETPATH | POPT_GETDEV;
	if (pnt == NULL || pnt[1] != 0)		// Was the colon the last character?
	{									// No - setting the path
		if (endpnt[-1] != '\\')
			*endpnt++ = '\\';
		endpnt[0] = 'x';
		endpnt[1] = 0;
		if ((rtn = svcIoDevParm(XO_ODFS, buffer, NULL)) < 0)
		{
			cmnderror(rtn, "New path is not valid");
			return (FALSE);
		}
		endpnt[0] = 0;
		opt |= POPT_SETPATH;
	}
	if ((rtn = pathfunc(opt,  buffer, buffer, 298)) < 0)
		cmnderror(rtn, (opt & POPT_SETPATH) ? "Error setting path for device" :
				"Error getting path for device");
	else
	{
		if (rtn > 296)
		{
			buffer[296] = '+';
			buffer[287] = 0;
		}
		pnt = strchr(buffer, ':') + 1;
		rtn = sprintf(text, "Default path for %.*s is%s %s\r\n", pnt - buffer,
				buffer, (opt & POPT_SETPATH) ? " now set to" : "", pnt);
		svcIoOutBlock(ccs->cmdout, text, rtn);
	}
	return (FALSE);
}
