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


#define IOCTL_DISK_UPDATE_PROPERTIES CTL_CODE(IOCTL_DISK_BASE, 0x0050, METHOD_BUFFERED, FILE_ANY_ACCESS)

static void cleanup();


void install()
{
	if ((installthd = (HANDLE)_beginthreadex(NULL, 0, installthread, 0, 0,
			&installid)) == NULL)
	{
		setmsg("? Thread create failed");
		return;
	}
}

int rtn;

//**********************************************************************
// Function: installthread - Main thread function for the install thread
// Returned: Thread exit status
//**********************************************************************

// This function does most of the work setting up a bootable structure on the
//   disk and copying the distribution files to it.
// Arguments are taken from the following global variables:
//		devnum   = Windows physical disk number
//      devbits  = Bit mask for DOS letter associated with the disk
//		devdize  = Total size of the disk (bytes)
//		filespec = Full file spec for the install data file

uint __stdcall installthread(
	void *arg)
{
	STORAGE_DEVICE_NUMBER
	        devnumber;
	HANDLE *hpnt;
	long    drives;
	ulong   size;
	long    bits;
	long    mask;
	char    dname[32];
	int     rtn;

	setmsg("\nInitializing");

	if (dskbufr == NULL)				// Allocate the disk buffer if we
	{									//   don't have one  (This must be
										//   512-byte aligned.)
		dskbufr = (uchar *)malloc(DSKBUFRSZ + 512);
		dskbufr = (uchar *)(((long)dskbufr + 0x01FF) & 0xFFFFFE00);
	}
	memset(hndltbl, 0xFF, sizeof(hndltbl));
	setmsg("Opening physical disk %d", devnum);
	sprintf(dname, "\\\\.\\PHYSICALDRIVE%d", devnum);
	if ((phyhndl = CreateFile(dname, GENERIC_READ|GENERIC_WRITE, 0,
			NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, 0)) ==
			INVALID_HANDLE_VALUE)
	{
		setmsg("? Error opening physical disk unit %d", devnum);
		cleanup();
		return (0);
	}
	setmsg("Locking physical disk %d", devnum);

	// Now we must open, lock, and dismount each volume (partition) on the
	//   physical disk. This is some almost totally undocumented magic that
	//   is needed to insure exclusive access to the physical disk! Note that
	//   all of the handles used here must be kept open as long as we want
	//   exclusive access.

	bits = devbits;
	drvltr = 'A';
	mask = 0x01;
	hpnt = hndltbl;
	while (bits != 0)
	{
		if (bits & mask)
		{
			sprintf(dname, "\\\\.\\%c:", drvltr);
			if ((*hpnt = CreateFile(dname, GENERIC_READ|GENERIC_WRITE,
					FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
					0, 0)) == INVALID_HANDLE_VALUE)
			{
				seterrmsg("? Error opening partition %c:", drvltr);
				cleanup();
				return (0);
			}
			if ((rtn = DeviceIoControl(*hpnt, FSCTL_LOCK_VOLUME,
					NULL, 0, NULL, 0, &size, NULL)) == 0)
			{
				seterrmsg("Error locking partition %c:", drvltr);
				cleanup();
				return (0);
			}
			if ((rtn = DeviceIoControl(*hpnt, FSCTL_DISMOUNT_VOLUME,
					NULL, 0, NULL, 0, &size, NULL)) == 0)
			{
				seterrmsg("Error dismounting partition %c:", drvltr);
				cleanup();
				return (0);
			}
			hpnt++;
			bits &= ~mask;
		}
		drvltr++;
		mask += mask;
	}
///	*hpnt = (HANDLE)0xFFFFFFFF;

	// If get here, we should have exclusive access to the physical disk.
	//   Before we modify the disk, verify that we have a valid XID file.
	//   This also reads the first item header and verifies that it is a
	//   bootstrap item.

	if (!startxid())
	{
		cleanup();
		return (0);
	}

	//   XID file looks OK. Rewrite the MBR to indicate a single FAT32
	//     partition which covers the disk.

	setmsg("Initializing MBR (master boot record) on physical disk %d",
			devnum);
	if (!initmbr())
	{
		cleanup();
		return (0);
	}

	// Set up the partition with a FAT32 file system.

	if (!fmtfat32())
	{
		cleanup();
		return (0);
	}

	// Write the XOS bootstrap to the partition.

	if (!mkboot())
	{
		cleanup();
		return (0);
	}

	// Close all of the handles for partitions that were on this disk.

	setmsg("Doing clean-up after initializing the disk");
	hpnt = hndltbl;
	while (*hpnt != (HANDLE)0xFFFFFFFF)
	{
		CloseHandle(*hpnt);
		*hpnt++ = (HANDLE)0xFFFFFFFF;
	}

	// Tell Windows to reread the disk's partition table

	if ((rtn = DeviceIoControl(phyhndl, IOCTL_DISK_UPDATE_PROPERTIES,
			NULL, 0, NULL, 0, &size, NULL)) == 0)
	{
		seterrmsg("Error dismounting physical disk %d:", devnum);
		cleanup();
		return (0);
	}
	CloseHandle(phyhndl);				// Close the handle for the physical
	phyhndl = (HANDLE)0xFFFFFFFF;		//   device

	// Here with the partition completely set up and known to Windows. Windows
	//   may have (but probably has not) changed the drive letter assignment
	//   for our disk. To be safe, we scan the drive letters again and see
	//   which one how corresponds to our partition. Note that we now know
	//   there is exactly one partition on our physical disk. The drive number
	//   can not have changed since we still have it open.

	drives = GetLogicalDrives();		// Get current DOS drive letter mask
	drvltr = 'A';
	mask = 0x01;
	while (drives != 0)
	{
		if (drives & mask)
		{
			sprintf(dname, "%c:", drvltr);
			if (GetDriveType(dname) == DRIVE_REMOVABLE) // Is it a removable
			{											//  "disk"?
				sprintf(dname, "\\\\.\\%c:", drvltr); // Yes - open the raw
													  //   partition
				if ((hndltbl[0] = CreateFile(dname, 0, FILE_SHARE_READ|
						FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0)) !=
						INVALID_HANDLE_VALUE)
				{
					// Here with a partition on a removable disk open as a
					//   raw partition - Get the physical disk number.

					rtn = DeviceIoControl(hndltbl[0],
							IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
							&devnumber, sizeof(devnumber), &size, NULL);
					CloseHandle(hndltbl[0]);
					hndltbl[0] = (HANDLE)0xFFFFFFFF;
					if (rtn > 0 && devnumber.DeviceType == FILE_DEVICE_DISK &&
							devnumber.DeviceNumber == devnum) // Is it our's?
						break;			// Yes
				}
			}
			drives &= ~mask;
		}
		drvltr++;
		mask += mask;
	}
	if (drives == 0)					// Did we find it?
	{
		setmsg("Could not find volume after format");
		cleanup();
		return (0);
	}

	// Here with our reformatted volume. We are now ready to copy files to
	//   the volume

	setmsg("\nInstalling files on the boot disk");
	if (!copyitems())
	{
		cleanup();
		return (0);
	}
	CloseHandle(xidhndl);				// Close the XID file
	xidhndl = (HANDLE)0xFFFFFFFF;

	// Here with a complete bootable system on the disk. Now copy the install
	//   sets (XIS files) from the XID file. (These are not really copied but
	//   are constructed "on-the-fly" from the data in the XID file.)

	setmsg("\nCopying install sets to the boot disk");
	if (!startxid())
	{
		cleanup();
		return (0);
	}
	if (!makeinstallsets())
	{
		cleanup();
		return(0);
	}
	CloseHandle(xidhndl);				// Close the XID file for the last time
	xidhndl = (HANDLE)0xFFFFFFFF;

	// Here with everything copied to our disk - now open the partition a
	//   final time and dismount it so it can be removed safely.

	sprintf(dname, "\\\\.\\%c:", drvltr);
	if ((hndltbl[0] = CreateFile(dname, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0)) ==
			INVALID_HANDLE_VALUE)
	{
		seterrmsg("Error opening partition for dismount");
		return (0);
	}
	*(long *)dskbufr = 0;
	rtn = DeviceIoControl(hndltbl[0], IOCTL_STORAGE_MEDIA_REMOVAL, dskbufr,
			4, NULL, 0, &size, NULL);
	rtn = DeviceIoControl(hndltbl[0], IOCTL_STORAGE_EJECT_MEDIA, NULL,
			0, &devnumber, sizeof(devnumber), &size, NULL);
	CloseHandle(hndltbl[0]);
	hndltbl[0] = (HANDLE)0xFFFFFFFF;
	if (!rtn)
		seterrmsg("Error dismounting partition");
	else
		setmsg("Finished - The target disk may be removed safely now\n");
	SetEvent(waithndl);
	return (0);
}


int createdir(
	char *spec)
{
	char *pnt;
	char  name[200];
	char  finished;

	do
	{
		finished = true;
		strmov(name, spec);
		while (!CreateDirectory(name, NULL))
		{
			if (GetLastError() != ERROR_PATH_NOT_FOUND || (pnt =
					strrchr(name, '\\')) == NULL)
			{
				seterrmsg("? Error creating directory %s", spec);
				return (false);
			}
			finished = false;
			*pnt = 0;
		}
	} while (!finished);
	return (true);
}


void seterrmsg(
	char *fmt, ...)
{
	va_list pi;
	char   *pnt;
	int     len;
	char    text[300];

    va_start(pi, fmt);
	pnt = strmov(text, "? ");
   	if ((len = _vsnprintf(pnt, 200, fmt, pi)) < 0)
		len = 198;
	pnt += len;
	pnt = strmov(pnt, "\n   ");
	if ((rtn = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|
			FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), pnt, 298 -
			(pnt - text), NULL)) <= 0)
		*pnt = 0;
	else
	{
		if ((pnt = strpbrk(pnt, "\r\n")) != NULL)
			*pnt = 0;
	}
	len = pnt - text;
	pnt = (char *)malloc(len + 1);
	memcpy(pnt, text, len + 1);
	swcRespSend(stsnum, 0, (long)pnt);
}


int eeeee;


void setmsg(
	char *fmt, ...)
{
	va_list pi;
	char   *pnt;
	int     len;
	char    text[256];

    va_start(pi, fmt);
   	if ((len = _vsnprintf(text, 254, fmt, pi)) < 0)
	{
		text[254] = 0;
		len = 254;
	}

	if (fmt[0] == '?')
		eeeee++;

	pnt = (char *)malloc(len + 1);
	memcpy(pnt, text, len + 1);
///	if (message[0] == '?')
///	{
///		cleanup();
///		hverror = true;
///		stopreq = true;
///		finished = true;
///	}
	swcRespSend(stsnum, 0, (long)pnt);
}


//*******************************************************************
// Function: cleanup - Close all open disk handles before terminating
// Returned: Nothing
//*******************************************************************

void cleanup()
{
	HANDLE *hpnt;

	if (phyhndl != (HANDLE)0xFFFFFFFF)
	{
		CloseHandle(phyhndl);
		phyhndl = (HANDLE)0xFFFFFFFF;
	}
	hpnt = hndltbl;
	while (*hpnt != (HANDLE)0xFFFFFFFF)
	{
		CloseHandle(*hpnt);
		*hpnt = (HANDLE)0xFFFFFFFF;
		hpnt++;
	}
	if (xidhndl != (HANDLE)0xFFFFFFFF)
	{
		CloseHandle(xidhndl);
		xidhndl = (HANDLE)0xFFFFFFFF;
	}
}
