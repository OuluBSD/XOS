//------------------------------------------------------------------------------
//  findapps.c Function to find application directories
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

// This is a temporary kludge to find the application directories to search
//   until we get environment strings implimented in this version. This isn't
//   really a bad idea but does have the problem that it provides no way to
//   specify the order in which application directories are searched.


static _Packed struct
{	BYTE4PARM  filoptn;
	LNGSTRPARM filspec;
	BYTE2PARM  srcattr;
	char       end;
} sparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_FILE},
	{PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, 400, 400},
	{PAR_SET|REP_HEXV, 2, IOPAR_SRCATTR, XA_DIRECT}
};

static char ascerr[] = "Error accessing the application directory\n"
		"The application sub-directories will not be searched";
static char scnerr[] = "Error scanning the application directory\n"
		"Some application sub-directories may not be searched";


static void warn(long code, char *msg);


void findapps(void)
{
	char *apppnt;
	long  rtn;
	long  dirhndl;
	long  total;
	int   retry;
	char  buffer[400];

	if (applist != NULL)
	{
		free(applist);
		applist = NULL;
	}
	if ((dirhndl = svcIoOpen(XO_ODFS, "XOSAPP:*.*", NULL)) < 0)
	{									// Open the directory if we can
		if (dirhndl != ER_NSDEV && dirhndl != ER_DIRNF && dirhndl != ER_FILNF)
			warn(dirhndl, ascerr);
		return;
	}
	retry = 3;
	while (TRUE)
	{
		// First scan the XOSAPP: directory and see how much space we need

		sparms.filspec.buffer = buffer;
		total = 1;
		while (TRUE)
		{
			if ((rtn = sysIoNextFile(dirhndl, "*.*", &sparms)) < 0)
			{
				if (rtn != ER_DIRNF && rtn != ER_FILNF)
					warn(dirhndl, scnerr);
				break;
			}
			total += (sparms.filspec.strlen + 10);
		}
		if ((applist = malloc(total)) == NULL)
		{
			warn(-errno, "Error allocating memory for the application "
					"list\nThe application sub-directories will not be "
					"searched");
			return;
		}
		applist[0] = '%';
		apppnt = applist + 1;

		// Scan it again and collect the name

		if ((rtn = sysIoSetPos(dirhndl, 0, 0)) < 0)
		{
			warn(dirhndl, ascerr);
			return;
		}
		while (TRUE)
		{
			if ((rtn = sysIoNextFile(dirhndl, "*.*", &sparms)) < 0)
			{
				if (rtn != ER_DIRNF && rtn != ER_FILNF)
					warn(dirhndl, scnerr);
				break;
			}
			if ((total -= (sparms.filspec.strlen + 8)) < 0)
				break;
			memcpy(apppnt, "XOSAPP:\\", 8);
			memcpy(apppnt + 8, sparms.filspec.buffer, sparms.filspec.strlen);
			apppnt += (sparms.filspec.strlen + 10);
			apppnt[-2] = '\\';
			apppnt[-1] = ';';
		}
		if (total >= 0)
			break;

		// Here if our list was not long enough - Someone must have added an
		//   application since our first scan! (Unlikely, but possible)

		if (--retry < 0)				// Tired enough?
			break;						// Yes - just give him what we have!
		free(applist);					// No - do it all again!
		applist = NULL;
		if ((rtn = sysIoSetPos(dirhndl, 0, 0)) < 0)
		{
			warn(dirhndl, ascerr);
			return;
		}
	}

	// Here with the application list complete - replace the last ; with a 0
	//   to terminate the string

	apppnt[-1] = 0;
}


static void warn(
	long  code,
	char *msg)
{
	errormsg(code, msg);
	cmdpause("Press any key to continue");
}
