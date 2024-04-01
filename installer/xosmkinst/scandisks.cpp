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


#define IOCTL_DISK_GET_DRIVE_GEOMETRY_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0028, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*typedef struct _DISK_GEOMETRY_EX
{	DISK_GEOMETRY Geometry;
	__int64       DiskSize;
	BYTE          Data[256];
} DISK_GEOMETRY_EX, *PDISK_GEOMETRY_EX;
*/

static DEVITEM *devlist;

static DISK_GEOMETRY_EX geomex;

static void scandisks();
static void dodevice(uint dnum, long mask);


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
		scandisks();
		WaitForSingleObject(waithndl, INFINITE);
		ResetEvent(waithndl);
	}
	return (0);
}


//***********************************************
// Function: scandisks - Find all removable disks
// Returned: Nothing (does not return if error
//***********************************************

static void scandisks()
{
	STORAGE_DEVICE_NUMBER
	       devnumber;
	HANDLE hndl;
	int    rtn;
	int    dltr;
	ulong  size;
	long   drives;
	long   dmask;
    char   dname[32];

	// Scan the DOS drives and remember the removable disks. Also get the size
	//   of each removable disk. This code builds a linked list of DEVITEM
	//   structures, one for each removable disk. This list includes drives
	//   that do not have media mounted.

	drives = GetLogicalDrives();		// Get the DOS drive letter mask
	dltr = 'A';
	dmask = 0x01;
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
	swcRespSend(targetnum, 0, (long)devlist);
	devlist = NULL;


}


static void dodevice(
	uint dnum,
	long mask)
{
	HANDLE    hndl;
	DEVITEM **ppnt;
	DEVITEM  *dpnt;
	int       rtn;
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
			memset(&geomex.DiskSize, 0, sizeof(geomex.DiskSize));

			if ((rtn = DeviceIoControl(hndl, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
					NULL, 0, &geomex, sizeof(geomex), &size, NULL)) > 0 &&
					geomex.DiskSize.QuadPart > 0)
					dpnt->length = geomex.DiskSize.QuadPart;
			else
				dpnt->length = -1;
			CloseHandle(hndl);
		}
	}
	if (dpnt->length > 0)
		dpnt->mask |= mask;
}


