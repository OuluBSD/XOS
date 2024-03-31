//------------------------------------------------------------------------------
//  cmdhistory.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

//***************************************************
// Function: cmdhistory - Process the HISTORY command
// Returned: FALSE
//***************************************************

int cmdhistory(
	char *cmd)
{
	HIST *hpnt;
	long  num;
	char  buffer[100];

	if ((cmd = getonearg(cmd)) == NULL)
		return (FALSE);
	if (!getonenum(cmd, &num))
		return (FALSE);
	if (num != 0)
	{
		if (num > 500)
			num = 500;
		if (num < 10)
			num = 10;
		histmax = num;
		num = sprintf(buffer, "Command history limit set to %d\n", num);
		svcIoOutBlock(ccs->cmdout, buffer, num);
		return (FALSE);
	}
	if ((hpnt = histfirstblk) == NULL)
	{
		cmnderror(0, "History list is empty");
		return (FALSE);
	}
	num = sprintf(buffer, "Command history: (%d)\n", histmax);
	svcIoOutBlock(ccs->cmdout, buffer, num);
	do
	{
		num = sprintf(buffer, "%6d: %.68s\n", hpnt->num, hpnt->txt);
		svcIoOutBlock(ccs->cmdout, buffer, num);
	} while((hpnt = hpnt->next) != NULL);
	return (FALSE);
}
