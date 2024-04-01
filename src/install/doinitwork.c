#include "install.h"


static int dodiskinit(char *diskname);


//*****************************************************************
// Function: doinitwork - Work thread function to initialize a disk
// Returned: Nothing
//*****************************************************************

// This function is called in the work thread as a result of a dowork(FUNC_INIT)
//   call.

void doinitwork(void)
{
	time_s curdt;
	char  *pnt;
	int    rtn;
	char   ttxt[32];
	char   diskname[20];

	if (logfile == NULL &&
			(logfile = fopen("XOSACT:install.log", "a+")) == NULL)
		puterror(-errno, "Error opening XOSACT:install.log, this sequence "
				"will not be logged");
	if (tarbase[0] == 0)
	{
		putline(COL_ERROR, "There is no disk available");
		return;
	}
	pnt = strmov(diskname, tarbase);
	if (pnt[-1] == ':')
		pnt[-1] = 0;
	rtn = dodiskinit(diskname);
	svcSysDateTime(1, (time_sz *)&curdt);
	sdt2str(ttxt, "%z%H:%m:%s on %D-%3n-%l", (time_sz *)&curdt);
	putline((rtn) ? COL_GOOD : COL_FAIL, "Initialization of %s: %s complete "
			"at %s", diskname, (rtn) ? "is" : "did not", ttxt);
	if (continueinst)
	{
		didinit = TRUE;
		doinstwork();
	}
	else
	{
		xwsWinDestroy(btncancel);
		btncancel = NULL;
		if (logfile != NULL)
		{
			fclose(logfile);
			logfile = NULL;
		}
		state = STATE_DONE;
		disprescan();
	}
}



static int dodiskinit(
	char *diskname)
{
	time_s curdt;
	MBR    mbr;
	char   disk[24];
	char   text[64];

	strmov(strmov(disk, diskname), ":");
	svcSysDateTime(1, (time_sz *)&curdt);
	sdt2str(text,"%z%H:%m:%s on %D-%3n-%l", (time_sz *)&curdt);
	putline(COL_NOSEL, "Initialization of disk %s started at %s", disk, text);
	if (!diskutilopen(disk, puterror))
		return (FALSE);
	if (!diskutilinitmbr())
	{
		diskutilclose();
		return (FALSE);
	}
	if (!diskutilfinish())
		return (FALSE);
	putline(COL_NOSEL, "Master boot record (MBR) written");

	// Here with the disk initialized with no partitions. Now create a
	//   partition containing up to 20GB.

	if (!diskutilopen(disk, puterror))
		return (FALSE);
	if (!diskutilmakepart(&mbr, 12, TRUE, 0, 41943040))
		return (FALSE);
	if (!diskutilfinish())
		return (FALSE);
	putline(COL_NOSEL, "Partition created");

	// Format the partition as FAT32

	strmov(strmov(text, diskname), "P1:");
	if (!fmtfat(text, FS_FAT32, 32, 64, NULL))
		return (FALSE);
	putline(COL_NOSEL, "Partition formatted");
	strmov(strmov(tarprtn, diskname), "P1:");
	tarbase[0] = 0;						// Indicate now have valid partition
	return (TRUE);
}


//***************************************************
// Function: formatstatus - Called by the disk format
//				function to report progress
// Returned: Nothing
//***************************************************

void formatstatus(
	char *text,
	long  value)
{
	static long lastval;

	if (value >= 0)
	{
		putline(COL_NOSEL | TEMP, "Writing %s: %,d", text, value);
		lastval = value;
	}
	else
		putline(COL_NOSEL, "Writing %s: %,d - Complete", text, lastval);
}


void diskiniterror(
	long  code,
	char *msg)
{
	if (showdierror)
		puterror(code, msg);
}


void formaterror(
	long  code,
	char *msg)
{
	puterror(code, msg);
}
