//------------------------------------------------------------------------------
//
//  cmdcd.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"


struct
{   BYTE4CHAR syschar;
    char      end;
} sysparm =
{   {(PAR_GET|REP_HEXV), 4}
};

ulong getsyschar(
    char *str)

{
    strncpy(sysparm.syschar.name, str, 8);
	*(long *)&sysparm.syschar.value = 0;
    svcIoClsChar("SYSTEM:", (char *)&sysparm);
    return ((ulong)(sysparm.syschar.value));
}


//****************************************************************
// Function: cmnderror - Report general error as returned from XOS
// Returned: Nothing
//****************************************************************

void cmnderror(
	long  code,
	char *txt)
{
	char *fmt;
	int   len;
    char  buffer[200];

	fmt = "\n? XSHELL: %s";
	if (txt == NULL)
		txt = "";
	if (txt[0] == '\n')
		txt++;
	else
		fmt++;
    len = sprintf(buffer, fmt, txt);
	if (code < 0)
	{
		if (txt[0] != 0)
		{
			strmov(buffer + len, "\n           ");
			len += 11;
		}
		len += svcSysErrMsg(code, 3, buffer + len);
	}
	strmov(buffer + len, "\r\n");
	svcIoOutBlock(ccs->cmderr, buffer, len + 2);
}


char *getonearg(
	char *args)
{
	char *pnt;
	char  chr;

	pnt = args;
	while ((chr = *pnt) != 0 && !isspace(chr))
		pnt++;
	while ((chr = *pnt) != 0 && isspace(chr))
		pnt++;
	return ((chr != 0) ? NULL : args);
}


int getonenum(
	char *cmd,
	long *valp)
{
	*valp = strtol(cmd, &cmd, 10);
	if (*cmd != 0)
	{
		cmnderror(0, "Invalid number specified");
		return (FALSE);
	}
	return (TRUE);	
}



long pathfunc(
	long  options,
	char *name,
	char *buffer,
	int   length)
{
	long rtn;
	long len;

	if ((rtn = svcIoPath(options, name, buffer, length)) >= 0)
	{
		if (gcp_dosdrive && buffer[1] != ':')
		{
			if ((len = svcIoGetDosDevice(buffer, buffer, length)) > 0)
				rtn = len;
		}
	}
	return (rtn);
}
