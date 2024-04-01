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

static long    clussize;
static long    clusters;

// The boot block written here contains code to display the message "Not a
//   system disk" if the disk is booted.

long  fatsize;

static int writefat(char *text);

int fmtfat32()
{
	ulong size;

	// We assigned the cluster size as follows:
	//           size <= 100MB = 4 blocks
	//   100MB < size <= 1GB   = 8 blocks
	//   1GB   < size <= 2GB   = 16 blocks
	//   2GB   < size <= 4GB   = 32 blocks
	//   4GB   < size          = 64 blocks

	setmsg("Writing boot block on physical disk %d", devnum);

	clussize = ((ulong)devsize < 200000) ? 4 : ((ulong)devsize < 2000000) ? 8 :
			((ulong)devsize < 4000000) ? 16 : ((ulong)devsize < 80000000) ?
			32 : 64;
	clusters = ((ulong)devsize)/clussize;
	fatsize = (clusters + 127)/128;

	memcpy(dskbufr, bootblk, 512);
	((BOOTBLK32 *)dskbufr)->secpcls = (uchar)clussize; // Sectors per cluster

	((BOOTBLK32 *)dskbufr)->ressec = RESERVED; // Total number of reserved
											   //   sectors
	((BOOTBLK32 *)dskbufr)->numfats = 2; // Number of FATs
	((BOOTBLK32 *)dskbufr)->rdirent = 0; // Number of root directory entries
	((BOOTBLK32 *)dskbufr)->sectors = (ushort)(((ulong)devsize & 0xFFFF0000) ?
			 0 : devsize);				// Total number of sectors on disk
										//   (16-bit value)
	((BOOTBLK32 *)dskbufr)->media = 0xF8; // Media description byte
	((BOOTBLK32 *)dskbufr)->secpfat = /* (fatsize < 0xFFFF) ? fatsize : */ 0;
										// Number of sectors per FAT table
										//   (16-bit value)
	((BOOTBLK32 *)dskbufr)->secptrk = 63; // Number of sectors per track
	((BOOTBLK32 *)dskbufr)->heads = 255; // Number of heads
	((BOOTBLK32 *)dskbufr)->hidsec = 63; // Number of hidden sectors (This is
										//   actually the offset of the
										//   partition on the disk!!)
	((BOOTBLK32 *)dskbufr)->ttlsec = (ulong)devsize; // Total number of sectors
													 //   on disk(32 bit value)
	((BOOTBLK32 *)dskbufr)->ttlsecpfat = fatsize; // Sectors per FAT (32-bit
												  //   value)
	((BOOTBLK32 *)dskbufr)->extflags = 0; // Flags
	((BOOTBLK32 *)dskbufr)->rootcls = 2; // Number of first cluster in root
										//   directory
	((BOOTBLK32 *)dskbufr)->fsinfo = 0;	// Sector number of the file system
										//   intormation sector

	// Write the boot block to block 0 of the partition

	if (SetFilePointer(phyhndl, 63 * 512, NULL, FILE_BEGIN) < 0)
	{
		setmsg("? Error setting position for boot block");
		return (false);
	}
	if (!WriteFile(phyhndl, dskbufr, 512, &size, NULL))
	{
		seterrmsg("Error writing boot block");
		return (false);
	}

	// Write the reserved blocks (The disk buffer is the size of the extra
	//   reserved blocks.)

	setmsg("Writing reserved blocks on physical disk %d", devnum);
	memset(dskbufr, 0, DSKBUFRSZ);
	if (!WriteFile(phyhndl, dskbufr, DSKBUFRSZ, &size, NULL))
	{
		seterrmsg("Error writing reserved blocks");
		return (false);
	}

	// Write the FAT blocks

	setmsg("Writing first FAT block on physical disk %d", devnum);
	if (!writefat("first"))				// Write the first FAT
		return (false);
	setmsg("Writing second FAT block on physical disk %d", devnum);
	if (!writefat("second"))			// Write the second FAT
		return (false);

	// Write the root directory

	setmsg("Writing root directory on physical disk %d", devnum);
	if (!WriteFile(phyhndl, dskbufr, clussize * 512, &size, NULL))
	{
		seterrmsg("Error writing home directory");
		return (false);
	}
	setmsg("Format of partition on physical disk %d is complete", devnum);
	return (true);
}


static int writefat(
	char  *text)
{
	ulong size;
	long  left;
	long  amnt;

	((long *)dskbufr)[0] = 0x0FFFFFF8; // Store prefix data (64 bits)
	((long *)dskbufr)[1] = 0x0FFFFFFF;
	((long *)dskbufr)[2] = 0x0FFFFFF8; // Store EOF indicator for root
	if (!WriteFile(phyhndl, dskbufr, 512, &size, NULL))
	{
		seterrmsg("Error writing %s FAT", text);
		return (false);
	}
	((long *)dskbufr)[0] = 0;
	((long *)dskbufr)[1] = 0;
	((long *)dskbufr)[2] = 0;
	left = fatsize - 1;
	while (left > 0)
	{
		if ((amnt = left) > DSKBUFRNM)
			amnt = DSKBUFRNM;
		if (!WriteFile(phyhndl, dskbufr, amnt * 512, &size, NULL))
		{
			seterrmsg("Error writing %s FAT", text);
			return (false);
		}
		left -= amnt;
	}
	return (true);
}
