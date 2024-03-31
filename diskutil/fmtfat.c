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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcstring.h>
#include <malloc.h>
#include <xcmalloc.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrun.h>
#include <xostime.h>
#include <xoserr.h>
#include <xosrtn.h>
#include <xosdisk.h>
#include <xosxdfs.h>
#include <xosthreads.h>
#include "fmtfuncs.h"

// The FAT partition boot block is formated as follows:
//   Name Offset Size Description
//   db_jump       0x00  3  Jump instruction to start of bootstrap code
//   db_oemname    0x03  8  OEM name and version
//   db_secsize    0x0B  2  Sector size in bytes
//   db_secpcls    0x0D  1  Sectors per cluster
//   db_ressec     0x0E  2  Total number of reserved sectors (includes the
//                            boot block)
//   db_numfats    0x10  1  Number of FATs
//   db_rdirent    0x11  2  Number of root directory entries
//   db_sectors    0x13  2  Total number of sectors on disk (16 bit value)
//   db_media      0x15  1  Media description byte
//   db_secpfat    0x16  2  Number of sectors per FAT table
//   db_secptrk    0x18  2  Number of sectors per track
//   db_heads      0x1A  2  Number of heads
//   db_hidsec     0x1C  4  Number of hidden sectors
//   db_ttlsec     0x20  4  Total number of sectors on disk (32 bit value)
// Following items are the extended boot infomation area for FAT12/FAT16 disks
//   db_drive      0x24  1  BIOS drive number (0 or 0x80)
//   db_extsig     0x26  1  Extended boot record signature = 0x29
//   db_sernum     0x27  4  Disk serial number (random value)
//   db_label      0x2B 11  Volume label (not used)
//   db_sysid      0x36  8  System ID (FAT12, FAT16, or FAT32)
// Following items are only present on partitions with 32-bit FATs
//   db_32secpfat  0x24  4  Sectors per FAT (32 bit value)
//   db_32extflags 0x28  2  Flags
//   db_32version  0x2A  2  File system version
//   db_32rootcls  0x2C  4  Number of first cluster in root directory
//   db_32fsinfo   0x30  2  Sector number of the file system intormation sector
//   db_32backboot 0x32  2  Sector number of the backup boot sector (0FFFFh if
//                            no backup boot sector)
// Following items are the extended boot infomation area for FAT32 disks
//   db_32drive    0x40  1  BIOS drive number (0 or 0x80)
//   db_32extsig   0x42  1  Extended boot record signature = 0x29
//   db_32sernum   0x43  4  Disk serial number (random value)
//   db_32label    0x47 11  Volume label (not used)
//   db_32sysid    0x52  8  System ID (FAT12, FAT16, or FAT32)

// This function will set up FAT-12, FAT-16 or FAT-32 partitions.

// The boot block written here contains code to display the message "Not a
//   system disk" if the disk is booted.

extern uchar bootblk[512];

long fatsize;
long  clusters;

#define btblk ((BOOTBLK *)bootblk)

#define BFRBLKS 32		// Number of blocks in our buffer

static struct
{	TEXT8PARM class;
	uchar     end;
} diskparm =
{	{PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "DISK"}
};

static QAB diskqab =
{   QFNC_WAIT | QFNC_SPECIAL,	// open
    0,							// status
    0,							// error
    0,							// amount
    0,							// handle
    0,							// vector
    {0},
    DSF_DISMOUNT,				// option
    0,							// count
    NULL,						// buffer1
    NULL,						// buffer2
    &diskparm					// parm
};


static int writeblks(uchar *buffer, long nmbr, long cnt, char *text);
static int writefat(uchar *buffer, char *text);
static int writeonegrp(uchar *buffer, long nmbr, char *text);


int fmtfat(
	char *diskname,
	int   fstypearg,
	int   clussize,
	int   reserved,
	int (*fmtready)(char *diskname, char *basename, long blocks, long clussize,
			long clusters, long reserved, int lbamode, int fstype,
			long  fatsize, long  rootsize))
{
	time_sz curdt;
	long    rtn;
	uchar   buffer[BFRBLKS*512];

	if (!fmtsetupdisk(diskname))
		return (FALSE);
	if ((blocks + partofs) > 0x0FFFFFFF)
	{
		fmterror(ER_DK2LG, "Disk partition is too large");
		return (FALSE);
	}
	fstype = fstypearg;

	// If no file structure type was specified, decide which one to use based
	//   on the size of the disk

	if (fstype == 0)
		fstype = (blocks < 16000) ? FS_FAT12 : (blocks < 512000) ? FS_FAT16 :
				FS_FAT32;
	switch (fstype)
	{
	 case FS_FAT12:
		prtntype = PRTN_FAT12;
		if (reserved < 0)
			reserved = 0;
		if (rootsize < 0)
			rootsize = 224;
		goto chkroot;

	 case FS_FAT16:
		prtntype = (lbamode) ? PRTN_FAT16X : PRTN_FAT16;
		if (clussize <= 0)
			clussize = (blocks < 32000) ? 2 : (blocks < 66000) ? 4 :
					(blocks < 160000) ? 8 : (blocks < 320000) ? 16 : 32;
		clusters = blocks/clussize;
		fatsize = (clusters + 255)/256;
		if (reserved < 0)
			reserved = 34;
		if (rootsize < 0)
			rootsize = 448;
	 chkroot:
		if (rootsize < 32)
			rootsize = 32;
		if (reserved <= 0)
			reserved = 1;
		rootsize &= ~0x1F;
		break;

	 case FS_FAT32:
		prtntype = (lbamode) ? PRTN_FAT32X : PRTN_FAT32;
		if (clussize <= 0)
			clussize = (blocks < 2000000) ? 4 : (blocks < 16000000) ? 8 :
					(blocks < 120000000) ? 16 : (blocks < 512000000) ? 32 : 64;
		clusters = blocks/clussize;
		fatsize = (clusters + 127)/128;
		if (reserved <= 0)
			reserved = 32;
		rootsize = 0;
		break;
	}
	if (fmtready != NULL)
	{
		if (!fmtready(diskname, basename, blocks, clussize, clusters, reserved,
				lbamode, fstype, fatsize, rootsize))
			return (TRUE);
	}

	// Following items are the same for all FAT formats

	btblk->secpcls = clussize;			// Sectors per cluster
	btblk->ressec = reserved;			// Total number of reserved sectors
	btblk->numfats = 2;					// Number of FATs
	btblk->rdirent = rootsize;			// Number of root directory entries
	btblk->sectors = (ushort)((blocks & 0xFFFF0000) ? 0 : blocks);
										// Total number of sectors on disk
										//   (16-bit value)
	btblk->media = 0xF8;				// Media description byte
	btblk->secpfat = (ushort)((fstype == FS_FAT32) ? 0 : fatsize);
										// Number of sectors per FAT table
										//   (16-bit value)
	btblk->secptrk = numsects;			// Number of sectors per track
	btblk->heads = numheads;			// Number of heads
	btblk->hidsec = partofs;			// Number of hidden sectors (This is
										//   actually the offset of the
										//   partition on the disk!!)
	btblk->ttlsec = blocks;				// Total number of sectors on disk
										//   (32 bit value)

	svcSysDateTime(1, &curdt);
	rtn = (long)(curdt.dt >> 20);
	if (fstype != FS_FAT32)
	{
		// Following items are the EBPB for FAT12/FAT16 disks

		btblk->drive = 0x80;			// BIOS drive number (0 or 0x80)
		btblk->extsig = 0x29;			// EBPB signature
		btblk->sernum = rtn;			// Disk serial number (random value)
		memcpy(btblk->vollbl, "           ", 11); // Volume label
		memcpy(btblk->sysid, (fstype == FS_FAT12) ? "FAT12   " : "FAT16   ", 8);
	}									// Sytem ID
	else
	{
		// Following items are unique to FAT32 disks

		((BOOTBLK32 *)btblk)->ttlsecpfat = fatsize; // Sectors per FAT (32-bit
													//   value)
		((BOOTBLK32 *)btblk)->extflags = 0; // Flags
		((BOOTBLK32 *)btblk)->rootcls = 2; // Number of first cluster in root
										  //   directory
		((BOOTBLK32 *)btblk)->fsinfo = 0; // Sector number of the file system
										 //   intormation sector
		((BOOTBLK32 *)btblk)->backboot = 0xFFFF; // Sector number of the backup
												 //   boot sector (0xFFFF if
												 //   none)

		// Following items are the EBPB for FAT32 disks

		((BOOTBLK32 *)btblk)->drive = 0x80; // BIOS drive number (0 or 0x80)
		((BOOTBLK32 *)btblk)->extsig = 0x29; // EBPB signature
		((BOOTBLK32 *)btblk)->sernum = rtn; // Disk serial number
		memcpy(((BOOTBLK32 *)btblk)->vollbl, "           FAT32   ", 19);
										// Volume label and Sytem ID
	}

	// Write the boot block to block 0 of the partition

	if ((rtn = svcIoSetPos(parthndl, 0, 0)) < 0)
	{
		fmterror(rtn, "Error setting IO position for boot block write");
		return (FALSE);
	}
	if (!writeblks(bootblk, 1, 0, "boot block"))
		return (FALSE);

	// Write the reserved blocks

	memset(buffer, 0, sizeof(buffer));
	if (!writeblks(buffer, reserved - 1, 0, "reserved blocks"))
		return (FALSE);

	// Write the FAT blocks

	if (!writefat(buffer, "first FAT") || // Write the first FAT
			!writefat(buffer, "second FAT")) // Write the second FAT
		return (FALSE);

	// Write the root directory

	if (!writeblks(buffer, clussize, 0, "root directory"))
		return (FALSE);

	// Finally, set the partition type in the MBR

	if (!fmtsetparttype())
		return (FALSE);

	// Close the partition to force everything out

	if (!fmtfinish())
		return (FALSE);

	// Reopen the partition so we can dismount it

	if ((parthndl = thdIoOpen(XO_RAW|XO_NOMOUNT, diskname, NULL)) < 0)
	{
		fmterror(parthndl, "Error reopening disk partition");
		return (FALSE);
	}
	diskqab.handle = parthndl;
    if ((rtn = thdIoFunc(&diskqab)) < 0 || (rtn = diskqab.error) < 0)
	{
		fmterror(rtn, "Error dismounting disk partition");
		return (FALSE);
	}
	thdIoClose(parthndl, 0);
	if ((rtn = thdIoDevParm(XO_RAW, diskname, NULL)) < 0)
	{
		fmterror(rtn, "Error remounting disk partition");
		return (FALSE);
	}
	return (TRUE);
}


static int writefat(
	uchar *buffer,
	char  *text)
{
	switch (fstype)
	{
	 case FS_FAT12:
		buffer[0] = 0xF0;				// Store prefix data (24 bits)
		buffer[1] = 0xFF;		
		buffer[2] = 0xFF;		
		break;

	 case FS_FAT16:
		((long *)buffer)[0] = 0xFFFFFFF8; // Store prefix data (32 bits)
		break;

	 case FS_FAT32:
		((long *)buffer)[0] = 0x0FFFFFF8; // Store prefix data (64 bits)
		((long *)buffer)[1] = 0x0FFFFFFF;
		((long *)buffer)[2] = 0x0FFFFFF8; // Store EOF indicator for root
		break;							  //   directory
	}
	if (!writeonegrp(buffer, 1, text))
		return (FALSE);
	((long *)buffer)[0] = 0;
	((long *)buffer)[1] = 0;
	((long *)buffer)[2] = 0;
	return (writeblks(buffer, fatsize - 1, 1, text));	
}


static int writeblks(
	uchar *buffer,
	long   nmbr,
	long   cnt,
	char  *text)
{
	long amnt;

	while (nmbr > 0)
	{
		formatstatus(text, cnt);
		if ((amnt = nmbr) > BFRBLKS)
			amnt = BFRBLKS;
		if (!writeonegrp(buffer, amnt, text))
			return (FALSE);
		nmbr -= amnt;
		cnt += amnt;
	}
	formatstatus(text, cnt);
	formatstatus(text, -1);
	return (TRUE);
}


static int writeonegrp(
	uchar *buffer,
	long   nmbr,
	char  *text)
{
	long rtn;

	if ((rtn = thdIoOutBlock(parthndl, buffer, nmbr * 512)) < 0)
	{
		fmtfinish();
		fmterror(rtn, "Error writing %s", text);
		return (FALSE);
	}
	return (TRUE);
}
