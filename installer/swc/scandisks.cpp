//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include "xosmkinst.h"


static DEVITEM *devlist;


//**************************************************************************
// Function: scandisksthread - Main thread function for the scandisks thread
// Returned: Thread exit status
//**************************************************************************

// This thread scans the system and identifies all of the USB disks and sets
//   a list of these disks that is used to set the contents of the target
//   disk window. This is a separate thread so it can be invoked from the
//   window function that receives a DISKCHANGE message. When first created
//   it scans the disks and sets up the list contents. It then waits until
//   woken up when a disk change message is received. It then delays for
//   100 millisec to let all of the disk change messages die out and scans
//   the disks again and rebuilds the list.

uint __stdcall scandisksthread(
	void *arg)
{
	while (!terminate)
	{




		WaitForSingleObject(waithndl, INFINITE)
		ResetEvent(waithndl);
	)
	return (0);
}


//***********************************************
// Function: scandisks - Find all removable disks
// Returned: Nothing (does not return if error
//***********************************************

static void scandisks(void)
{
	STORAGE_DEVICE_NUMBER devnumber;
	char   **dlpnt;
	char    *tpnt;
	DEVITEM *dpnt;
	double   length;
	int      dltr;
	ulong    size;
	long     drives;
	long     dmask;
	long     bits;
    char     dname[32];
	char     buffer[256];

	if (lstdev != NULL)
	{
		delete lstdev;
		lstdev = NULL;
	}

	// Scan the DOS drives and remember the removable disks. Also get the size
	//   of each removable disk. This code builds a linked list of DEVITEM
	//   structures, one for each removable disk. This list includes drives
	//   that do not have media mounted.

	drives = GetLogicalDrives();		// Get the DOS drive letter mask
	dltr = 'A';
	dmask = 0x01;
	rmvnum = 0;
	while (drives != 0)
	{
		if (drives & dmask)
		{
			sprintf(dname, "%c:", dltr);
			if (GetDriveType(dname) == DRIVE_REMOVABLE) // Is it a removable
			{											//  "disk"?
				sprintf(dname, "\\\\.\\%c:", dltr); // Yes - open the raw
													//   partition
				if ((hndl = CreateFile(dname, 0, FILE_SHARE_READ|
						FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0)) !=
						INVALID_HANDLE_VALUE)
				{
					// Here with a partition on a removable disk open as a
					//   raw partition - Get the physical disk number.

					if ((rtn = DeviceIoControl(hndl,
							IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
							&devnumber, sizeof(devnumber), &size, NULL)) > 0 &&
							devnumber.DeviceType == FILE_DEVICE_DISK)
						dodevice(devnumber.DeviceNumber, dmask);
					CloseHandle(hndl);
				}
			}
			drives &= ~dmask;
		}
		dltr++;
		dmask += dmask;
	}


	swcRespSend(targetnum, 0, (long)pnt);


	// Construct the strings for the disk select list box - 

	dlpnt = disklist = (char **)malloc((rmvnum + 1) * sizeof(char **));
	memset(disklist, 0, (rmvnum + 1) * sizeof(char **));
	dpnt = devlist;
	while (dpnt != NULL)
	{
		if (dpnt->mask != 0)
		{
			tpnt = buffer + sprintf(buffer, "(%d)", dpnt->dnum);
			dltr = 'A';
			bits = dpnt->mask;
			dmask = 0x01;
			do
			{
				if (bits & dmask)
				{
					tpnt += sprintf(tpnt, " %c:", dltr);
					bits &= ~dmask;
				}
				dltr++;
				dmask += dmask;
			} while (bits != 0);
			length = (double)(dpnt->length / 1000);
			if (length > 1000000.0)
			{
				length /= 1000000.0;
				tpnt += sprintf(tpnt, " %.*fGB", digits(length), length);
			}
			else if (length > 1000.0)
			{
				length /= 1000.0;
				tpnt += sprintf(tpnt, " %.*fMB", digits(length), length);
			}
			else
				tpnt += sprintf(tpnt, " %.3fKB", digits(length), length);

			*dlpnt = (char *)malloc(tpnt - buffer + 1);
			strcpy(*dlpnt, buffer);
			dlpnt++;
		}
		dpnt = dpnt->next;		
	}
	if (disklist[0] != NULL)
	{
		lstdev = new swcList(winmain, 1000, 7200, 12000,
				apprect.bottom - 8200, 1300, FONT_TEXT|1200, 0x000000,
				0xFFFFFF, disklist, 0, SWC_BORDER_IN_LG|SWC_SCROLL_VERT_C|
				SWC_STATE_ENABLE, NULL);
		btnstart->SetEnabled(true);

///		lstdev->SetFocus();

///		dddev->SetTraverse(1, 3);
	}
	else
	{
		winmain->PutText(1000, 7200, 12000, 0xFFFFFF, 0x0000FF,
				TXT_LEFT|FONT_TEXT|1400, "No removable USB\ndisks were found");
		btnstart->SetEnabled(false);
	}
}


static int digits(
	double value)
{
	return ((value < 10) ? 3 : (value < 100) ? 2 : (value < 1000) ? 1 : 0);
}


static void dodevice(
	uint dnum,
	long mask)
{
	HANDLE    hndl;
	DEVITEM **ppnt;
	DEVITEM  *dpnt;
	ulong     size;
    char      dname[32];

	// Find the DEVITEM structure for this disk is we have one or find where
	//   it goes in the list.

	ppnt = &devlist;

	while ((dpnt = *ppnt) != NULL && dnum > dpnt->dnum)
		ppnt = &dpnt->next;
	if (dpnt == NULL || dpnt->dnum != dnum) // Did we find it?
	{									// No - create a new DEVITEM
		dpnt = (DEVITEM *)malloc(sizeof(DEVITEM));
		memset(dpnt, 0, sizeof(DEVITEM));
		dpnt->dnum = dnum;
		dpnt->next = *ppnt;
		*ppnt = dpnt;

		// Get the size of the disk - We need to open the raw physical disk
		//   to do this! Note that the get geometry function will fail if
		//   no media is mounted.

		sprintf(dname, "\\\\.\\PHYSICALDRIVE%d", dnum);
		if ((hndl = CreateFile(dname, 0, FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING, 0, 0)) != INVALID_HANDLE_VALUE)
		{
			geomex.DiskSize = 0;
			if ((rtn = DeviceIoControl(hndl, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
					NULL, 0, &geomex, sizeof(geomex), &size, NULL)) > 0 &&
					geomex.DiskSize > 0)
			{
				dpnt->length = geomex.DiskSize;
				rmvnum++;
			}
			else
				dpnt->length = -1;
			CloseHandle(hndl);
		}
	}
	if (dpnt->length > 0)
		dpnt->mask |= mask;
}


