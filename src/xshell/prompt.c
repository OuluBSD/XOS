//------------------------------------------------------------------------------
//
//  prompt.c - Functions to display the command prompt
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"


void showprompt(void)
{
    char const *prmpnt;			// Pointer to prompt string
	char       *bpnt;
    time_s      sysdt;			// System date/time value
    union
    {	struct xos_ver
        {	unsigned edit : 16;
			unsigned minor : 8;
			unsigned major : 8;
        } x_version;
        ulong x_long;
    }      a;
	long   rtn;
	int    bcnt;
	uchar  chr;
    char   buffer[FILESPCSIZE];	// Buffer for current path string

    if (svcSysGetEnv("PROMPT", &prmpnt) <= 0)
			prmpnt = "$_$p$g";

///	printf("### prompt: |%s|\n", prmpnt);

	if (ccs->batch == 0 && (flags & FLG_HAVECC))
	{
		flags &= ~FLG_HAVECC;
		histcurblk = NULL;
		svcSchCtlCDone();
	}

///	printf("### C:%X(%d) F:%X(%d) L:%X(%d)", histcurblk, (histcurblk != NULL) ?
///			histcurblk->num : 0, histfirstblk, histfirstblk->num, histlastblk,
///			histlastblk->num);

	bpnt = buffer;
	bcnt = sizeof(buffer) - 3;
	sysdt.dt = 0;
    while ((chr = *prmpnt++) != '\0')
    {
		if (--bcnt < 0)
			break;
        if (chr != '$')
		{
            *bpnt++ = chr;
			continue;
		}
		switch (tolower(*prmpnt++))
		{
		 case '$':						// Display a $
			*bpnt++ = '$';
			break;

		 case 't':						// Display the time
			if (sysdt.dt == 0)
				svcSysDateTime(T_GTSYSDTTM, &sysdt);
			bpnt = (sysdt.dt == 0) ? strmov(bpnt, "??:??:??") :
					(bpnt + sdt2str(bpnt, "%H:%m:%s", (time_sz *)&sysdt));
			break;

		 case 'd':						// Display the date
			if (sysdt.dt == 0)
				svcSysDateTime(T_GTSYSDTTM, &sysdt);
			bpnt = (sysdt.dt == 0) ? strmov(bpnt, "??:??:??") :
					(bpnt + sdt2str(bpnt, "%3w %D-%3m-%y", (time_sz *)&sysdt));
			break;

		 case 'p':						// Display the current drive and path
			if ((rtn = pathfunc(POPT_GETDEV|POPT_GETPATH, "Z:", bpnt,
					bcnt)) < 0)
			{
				bpnt = strmov(bpnt, "?:\\");
				bcnt -= 3;
			}
			else
			{
				bpnt += rtn;
				bcnt -= rtn;
			}
			break;

		 case 'n':						// Display current drive only
			if ((rtn = pathfunc(POPT_GETDEV, "Z:", bpnt, bcnt)) < 0)
			{
				bpnt = strmov(bpnt, "?:");
				bcnt -= 2;
			}
			else
			{
				bpnt += rtn;
				bcnt -= rtn;
			}
			break;

		 case 'v':						// Display the XOS version number */
			a.x_long = getsyschar("XOSVER");
			bpnt += sprintf(bpnt, "XOS %d.%d.%d", a.x_version.major,
					a.x_version.minor, a.x_version.edit);
			break;

		 case 'c':						// Display )
			*bpnt++ = ')';
			break;

		 case 'f':						// Display (
			*bpnt++ = '(';
			break;

		 case 'g':						// Display >
///			*bpnt++ = '>';
			*bpnt++ = '}';
			break;

		 case 'l':						// Display <
			*bpnt++ = '<';
			break;

		 case 'b':						// Display |
			*bpnt++ = '|';
			break;

		 case 'q':						// Display =
			*bpnt++ = '=';
			break;

		 case 'h':						// Erase previous character
 			bpnt = strmov(bpnt, "\b \b");
			break;

		 case 'e':						// Display an escape
			*bpnt++ = '\x1b';
			break;

		 case '_':						// Display a CR/LF
			*bpnt++ = '\r';
			*bpnt++ = '\n';
			break;
		}
	}
	svcIoOutBlock(ccs->cmderr, buffer, bpnt - buffer);
}
