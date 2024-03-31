#include "install.h"

static struct
{	BYTE1CHAR prtntype;
	TEXT4CHAR remove;
	BYTE4CHAR size;
	char      end;
} diskchars =
{	{PAR_GET|REP_DECV, 1, "PRTNTYPE"},
	{PAR_GET|REP_TEXT, 4, "REMOVE"},
	{PAR_GET|REP_DECV, 4, "CBLOCKS"}
};

// This function indicates the current state of the system as follows:
//   c{tarprtn} = Name of usable FAT32 partition (null if no usable partition)
//   c{tarbase} = Name of usable base unit (null if no usable base unit)
//   c(havexos) = TRUE if XOS installed on partition indicated by tarptrn

//*************************************************************
// Function: getstate - Obtains the current state of the system
// Returned: Nothing
//*************************************************************

void getstate(void)
{
	VERSN versn;
	char *dpnt;
	int   dcnt;
	long  hndl;
	long  rtn;
	char  name[48];

	// Determine the current state of the machine.

	// First determine which disk and partition to use. We determine this
	//   as follows:
	//   1) Find the first usable FAT32 file system in a partition of 1GB or
	//      more.
	//   2) If can't find a usable FAT32 file system, use the first disk which
	//      is 1GB more. (We will reinitialize the disk and create a FAT
	//      file system.)

	// We know that disk names are of the format Dn, Dnn DnPn.., or DnnP...
	//   Thus any name which is 3 characters of shorter is a base disk and
	//   any name that is longer than 3 characters is a partiton. We also
	//   know that the names in the list of units are 0 filled to 8 characters
	//   so all we have to do is check the 4th character. If it is 0 it is a
	//   base unit.

	// First try to read the MBR - the only error that we can recover from
	//   here is an invalid MBR format

	tarbase[0] = 0;
	havexos = FALSE;
	dpnt = unitnames;
	dcnt = numunits;
	while (--dcnt >= 0)
	{
		if (dpnt[3] != 0)				// Is this a partition?
		{
			sprintf(tarprtn, "%.8s:", dpnt); // Yes - construct the name
			if ((hndl = fbrIoOpen(XO_PHYS, tarprtn, NULL)) > 0)
			{
				rtn = svcIoDevChar(hndl, &diskchars);
				fbrIoClose(hndl, 0);
				if (rtn >= 0)
				{
					if ((diskchars.prtntype.value == PRTN_FAT32 ||
							diskchars.prtntype.value == PRTN_FAT32X) &&
							diskchars.remove.value[0] == 'N' &&
							diskchars.size.value >= 2000000)
						break;
				}
			}
		}
		dpnt += 8;
	}
	if (dcnt >= 0)						// Did we find a partition to use?
	{									// Yes
		// Here if have a FAT32 file system. Now determine if XOS appears to
		//   be currently installed. We do this by looking for the file
		//   \xossys\xos.run and verifing it's version number. If this file
		//   is present and the version information and creation date is
		//   reasonable, we assume XOS has been installed.

		strmov(strmov(name, tarprtn), "\\xossys\\xos.run");
		if (getrunver(&versn, name, "", 0xFFFFFFFF, TRUE))
			havexos = TRUE;
		putline(COL_GOOD, "Programs will be installed on %s%s", tarprtn,
				(havexos) ? "" : ", XOS must be installed first");
		rtn = isdigit(tarprtn[2]) ? 3 : 2; // Construct the name of the
		strnmov(tarbase, tarprtn, rtn);	   //   corresponding base partition
		tarbase[rtn] = ':';
		tarbase[rtn + 1] = 0;
		return;
	}

	// Here if did not find a usable partition - Now try to find a base unit
	//   which we can initialize.

	tarprtn[0] = 0;
	dpnt = unitnames;
	dcnt = numunits;
	while (--dcnt >= 0)
	{
		if (dpnt[3] == 0)				// Is this a partition?
		{
			sprintf(tarbase, "%.8s:", dpnt); // No - construct the name
			if ((hndl = fbrIoOpen(XO_PHYS, tarbase, NULL)) > 0)
			{
				rtn = svcIoDevChar(hndl, &diskchars);
				fbrIoClose(hndl, 0);
				if (rtn >= 0)
				{
					if (diskchars.remove.value[0] == 'N' &&
							diskchars.size.value >= 2000000)
						break;
				}
			}
		}
		dpnt += 8;
	}
	if (dcnt >= 0)						// Did we find a base unit to use?
	{
		putline(COL_GOOD, "Programs will be installed on %.*sP1, disk %s must "
				"be initialized first", strlen(tarbase) - 1, tarbase, tarbase);
		return;							// Yes
	}
	tarbase[0] = 0;
	putline(COL_GOOD, "No disk or partition was found on which to install "
			"programs");
}
