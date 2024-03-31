//------------------------------------------------------------------------------
//  cmdver.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

//*******************************************
// Function: cmdver - Process the VER command
// Returned: FALSE
//*******************************************

int cmdver(
	char *args)
{
	union
	{	struct xos_ver
		{	ulong edit:16;
			ulong minor:8;
			ulong major:8;
		}     x_version;
		ulong x_long;
	}    a;
	int  len;
	char buffer[80];

	args = args;

	len = sprintf(buffer, "%s version %d.%d (%s)\r\n", prgname, majversion,
			minversion, builddate);
	svcIoOutBlock(ccs->cmdout, buffer, len);

	a.x_long = getsyschar("XOSVER");	/* Get XOS version */
	len = sprintf(buffer, "XOS version %d.%d.%d", a.x_version.major,
			a.x_version.minor, a.x_version.edit);
	svcIoOutBlock(ccs->cmdout, buffer, len);

	a.x_long = getsyschar("DOSVER");	/* Get DOS emulator version */
	len = sprintf(buffer, " with DOS version %d.%d emulation\r\n",
			a.x_version.major, a.x_version.minor);
	svcIoOutBlock(ccs->cmdout, buffer, len);
	return (FALSE);
}

