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
#include "xosrun.h"
#include <stddef.h>

// The XOS bootstrap is a partition boot block bootstrap. On a partitioned
//   disk this program only modifies the boot record for the specified
//   partition. It does not modify the MBR.

// The BOOT.RUN file is a stand-alone .RUN file with a somewhat unusual
//   structure as follows:
//	Segment:  boot_s
//	  Msect:  level1_m, Offset=0x7C00
//	  Msect:  level2_m, Offset=0x8200
//	  Msect:  fat_m   , Offset=0, label = FAT*
//	  Msect:  xfs_m   , Offset=0, label = XFS*
//    Msect:  gcode_m , Offset=0, label = GCOD (optional)
//    Msect:  gdata_m , Offset=0, label = GDAT (optional)
//   There must not be any additional segments or msects. If additional code
//   is to be added (such as a debugger, it should be added to the boot_m
//   msect. Since .RUN files do not contain names for segments or msects, we
//   use the starting offset to identify each msect. For the file system
//   msects, we look at the label in the first 4 bytes of the msect. This
//   version of mkboot does not support run-time relocation. All of the msects
//   in the boot.run file must have all addresses and selectors fully defined.
//   The file system msect is copied to the reserved blocks at a 16-byte
//   paragraph boundry. Its selector value is stored in the parameter block
//   and is used when coping this code to the top of memory. Otherwise all
//   slector values are 0.

// The existance of multiple msects with the same offset in a segment means
//   that this file is not loadable in the normal way.  Instead, this program
//   selects the desired msects for the type of file system and uses them to
//   construct the level2 memory image stored in the reserved blocks.

// The bootstrap is implemented in 2 levels. The level 1 bootstrap is the same
//   for all file systems. It is one block in length and simply reads the
//   level 2 bootstrap from the reserved blocks following the boot block.
//   All FAT file system use the common part of the level 2 bootstrap. The
//   remiainder of the level 2 bootstrap consists of the file system IO
//   routines which impliment a simple read-olny file system driver.

// This program copies the selected level 1 bootstrap to the boot block and
//   stores the previous boot block, the bootstrap parameters, and the level
//   2 bootstrap code in the reserved blocks.

// This is a hacked up version of mkbootf.c which is an XOS function that
//   installs a boot strap on an arbitrary disk from a RUN file on disk. This
//   code assumes a valid boot block without any bootstrap code (since we
//   know it was just created). The bootstrap file (which is named BOOT) has
//   been determined to be the first item in the XID file. Its header, but not
//   its contents has been read. We copy the entire file into a buffer and
//   access it from there.

// This code assumes the partition starts at sector 63.

// We could  have combined this with the format step but doing it this way
//   requires fewer changes in the original XOS disk setup code this is taken
//   from.


// Define reserved blocks used.

#define RBLKOLDBT  RBLKBOOT		// Block number where old boot block is saved
#define RBLKCONFIG (RBLKBOOT+1)	// First block number which contains
								//    configuration data (2 blocks)
#define RBLKLEVEL2 (RBLKBOOT+3)	// First Block which contains the level2 code

#pragma pack (push)
#pragma pack (1)

typedef struct
{	uchar *code;				// Address of msect data
	long   length;				// Length of the msect
} MSDATA;

typedef struct
{	uchar dev;
	uchar part;
	char  spec[60];
} FILESPEC;

typedef struct
{	uchar    xpos;
	uchar    ypos;
	FILESPEC spec;
} MENUITEM;

typedef struct
{   char     label[16];			// Label ("XOS BOOT LOADER")
	short    filesel;			// Selector for file system code
	short    gcodesel;			// Selector for gecko code segment
	short    gdatasel;			// Selector for gecko data segment
    short    timeout;			// Time-out value - number of clock ticks (18.2
								//   per second)
    short    numline;			// Number of menu lines
	short    resrvd1[3];
	uchar    resrvd[2];
    FILESPEC dftspec;			// Default file name
	MENUITEM menu[12];			// Menu items
} CFGBLK;

static MSDATA   level1data;
static MSDATA   level2data;
static MSDATA   fatdata;
static MSDATA   xfsdata;
static MSDATA   gcodedata;
static MSDATA   gdatadata;

#define bootblock ((BOOTBLK32 *)dskbufr)
#define oldbootblk ((BOOTBLK32 *)(dskbufr + 512))
#define cfgblk ((CFGBLK *)(dskbufr + 1024))
#define bootbufr ((char *)(dskbufr + 2048))

static RUNHEAD  runhead;
static RUNSEG   seghead;
static RUNMSECT mshead;

static uchar haveoldboot;
static uchar fattype;

char bterrmsg[] = "? Boot error - press any key\r\n\0\x55\xAA";

#pragma pack (pop)

static void   baddevname(int num);
static void   errclose(void);
static void (*fail)(char *str1, long code, char *str2);
static void   menufail(char *text, int num);
static void (*notice)(char *str);
static void   strspec(FILESPEC *dst, char *spec, int num);

static void   bscleanup(void);

//*************************************************
// Function: mkboot - Write XOS bootstrap to a disk
// Returned: true if normal, false if error
//*************************************************

int mkboot()
{
	uchar    *cpnt;
	RUNMSECT *mshead;
	ulong     size;
	long      headpos;
	long      rtn;
	int       cnt;
	int       l2len;
	int       iolen;

	setmsg("Setting up to install bootstrap");
	if (!readitemhead())				// Read the first item header
		return (false);
	if ((itemhead.h.flag & 0x0F) != IT_BSTRP)
	{
		setmsg("? First item is not a bootstrap item");
		return (false);
	}
	if (itemhead.h.filelen > (60 * 512))
	{
		setmsg("? Bootstrap is too long");
		return(false);
	}
	if ((bootimg = (uchar *)malloc(itemhead.h.filelen)) == NULL)
	{
		seterrmsg("Error allocating buffer for bootstrap image");
		return (false);
	}
	if ((rtn = readxid(bootimg, itemhead.h.filelen)) < 0)
	{
		bscleanup();
		return (false);
	}

    // Read the current boot block

	if (SetFilePointer(phyhndl, 63 * 512, NULL, FILE_BEGIN) < 0)
	{
		setmsg("? Error setting position for MBR");
		bscleanup();
		return (false);
	}
	if (!ReadFile(phyhndl, (char *)oldbootblk, 512, &size, NULL))
	{
		seterrmsg("Error reading MBR");
		bscleanup();
		return (false);
	}

	// "Read" the bootstrap file headers and find and read the msects we need.
	//   We must have the msect containing the level 1 code (which has a base
	//   address  of 0:7C00), the msect containing the common level 2 code
	//   (which has a base address of 0:8200, and the msect containg the IO
	//   routines for the file system (which has a base address of 0:0 - we
	//   identify the file system type by reading the label bytes at the
	//   beginning of the msect).

    if (((RUNHEAD *)bootimg)->magic != 0x22D7 ||
			((RUNHEAD *)bootimg)->hdrsize < (sizeof(RUNHEAD) - 6) ||
            ((RUNHEAD *)bootimg)->fmtvrsn != 2 ||
			((RUNHEAD *)bootimg)->imagetype != 1)
    {
		setmsg("? Invalid bootstrap file");	// Make sure valid RUN file
        bscleanup();
        return (false);
    }
	headpos = ((RUNHEAD *)bootimg)->hdrsize + 6; // Set for "reading" first
												 //   segment header
    if (((RUNHEAD *)bootimg)->numsegs != 1)
    {
		setmsg("? Bootstrap file does not contain 1 segment");
        bscleanup();
        return (false);
    }
	if (((RUNSEG *)(bootimg + headpos))->hdrsize < sizeof(RUNSEG))
	{
		setmsg("? Invalid segment header in bootstrap file");
		bscleanup();
		return (false);
	}
	if ((cnt = ((RUNSEG *)(bootimg + headpos))->nummsc) == 0)
	{									// Get number of msects
		setmsg("? No msects in bootstrap file");
		bscleanup();
		return (false);
	}
	headpos += ((RUNSEG *)(bootimg + headpos))->hdrsize;
	while (--cnt >= 0)
	{
		mshead = (RUNMSECT *)(bootimg + headpos);
		if (mshead->hdrsize < sizeof(RUNMSECT) || (mshead->dataos +
				mshead->datasz) > itemhead.h.filelen)
		{
			setmsg("? Invalid msect header in bootstrap file");
			bscleanup();
			return (false);
		}
		headpos += mshead->hdrsize;

		if (mshead->addr == 0x7C00)
		{								// Is this the level 1 code?
			level1data.code = bootimg + mshead->dataos; // Yes
			if (mshead->alloc != 512)
			{
				setmsg("? Level 1 bootstrap code msect is not 512 bytes long");
		        bscleanup();
				return (false);
			}
			if (((BOOTBLK32 *)level1data.code)->jump[0] != 0xEB ||
					((BOOTBLK32 *)level1data.code)->jump[2] != 0x90 ||
					strncmp(((BOOTBLK32 *)level1data.code)->errmsg, bterrmsg,
					sizeof(bterrmsg) + 2) != 0)
			{
				setmsg("? Level 1 bootstrap code is invalid");
		        bscleanup();
				return (false);
			}
			level1data.length = 512;
		}
		else if (mshead->addr == 0x8200) // Is this the common level 2 code?
		{
			level2data.code = bootimg + mshead->dataos;
			level2data.length = mshead->alloc;
		}
		else if (mshead->addr == 0)		// Is this file system dependent code
		{								//   or gecko code or data?
			cpnt = bootimg + mshead->dataos;
			if (memcmp(cpnt, "FAT*", 4) == 0)
			{
				fatdata.code = cpnt;
				fatdata.length = mshead->alloc;
			}
			else if (memcmp(cpnt, "XFS*", 4) == 0)
			{
				xfsdata.code = cpnt;
				xfsdata.length = mshead->alloc;
			}
			else if (memcmp(cpnt, "GCOD", 4) == 0)
			{
				gcodedata.code = cpnt;
				gcodedata.length = mshead->alloc;
			}
			else if (memcmp(cpnt, "GDAT", 4) == 0)
			{
				gdatadata.code = cpnt;
				gdatadata.length = mshead->alloc;
			}
		}
		else
		{
			setmsg("? Unexpected msect encountered in bootstrap");
	        bscleanup();
			return (false);
		}
	}

    // Here with all msect data set up. See if we found everything we need.

	if (level1data.code == NULL)
	{
		setmsg("? No level 1 code found in bootstrap");
        bscleanup();
		return (false);
	}
	if (level2data.code == NULL)
	{
		setmsg("? No common boot code found in bootstrap");
        bscleanup();
		return (false);
	}

	// See if we have the file system code for the FAT32 file system .

    if (fatdata.code == NULL)
    {
		setmsg("? No bootstrap IO code is available for the FAT file system");
        bscleanup();
		return (false);
    }

	// Copy the new level 1 code to the new boot block

	memcpy(bootblock, level1data.code, 512);

	// Copy the level 2 code to the disk buffer

	l2len = (level2data.length + 0x0F) & 0xFFFFFFF0;
	iolen = (fatdata.length + 0x0F) & 0xFFFFFFF0;
	if ((l2len + iolen) > MAXL2LEN)
	{
		setmsg("? Bootstrap is too long");
        bscleanup();
		return (false);
    }
	memcpy(bootbufr, level2data.code, l2len);
	memcpy(bootbufr + l2len, fatdata.code, iolen);

	if ((cnt = (((l2len + iolen) + 0x01FF) & 0xFFFFFE00) - l2len - iolen) != 0)
		memset(bootbufr + l2len + iolen, 0, cnt);

    // Now set up the new boot block. Copy header part of the boot block
	//   to the new boot block 

 	memcpy(((char *)bootblock) + 3, ((char *)oldbootblk) + 3,
			offsetof(BOOTBLK32, code) - 3);
	memcpy(bootblock->vollbl, "XOS_INSTALL", 11);

	// Store the reserved block values

	bootblock->lvl2blk = RBLKCONFIG;
	bootblock->lvl2num = ((l2len + iolen + 0x01FF) >> 9) + 3;

    // Set up the configuration block

	memset(cfgblk + 16, 0, 1024 - 16);
	memcpy(cfgblk->label, "XOS BOOT LOADER", 16);
	cfgblk->filesel = (0x8200 + l2len) >> 4;
    cfgblk->timeout = 10 * 18;		// Store timeout value (10 seconds)
	cfgblk->numline = 3;
	cfgblk->dftspec.dev = 0xFF;
	cfgblk->dftspec.part = 0xFF;
    strcpy(cfgblk->dftspec.spec, "\\xossys\\xos"); // Store default program spec

	// Write the reserved blocks (The blocks in the disk buffer are in the
	//   same order as they are on disk so we just do one big write.)

	setmsg("Writing bootstrap to the disk");
	if (SetFilePointer(phyhndl, (RBLKOLDBT + 63) * 512, NULL, FILE_BEGIN) < 0)
	{
		setmsg("? Error setting position for writing bootstrap");
        bscleanup();
		return (false);
	}
	if (!WriteFile(phyhndl, (char *)oldbootblk, bootblock->lvl2num * 512, &size,
			NULL))
	{
		seterrmsg("Error writing bootstrap to disk");
        bscleanup();
		return (false);
	}

	// Write the modified boot block

	if (SetFilePointer(phyhndl, 63 * 512, NULL, FILE_BEGIN) < 0)
	{
		setmsg("? Error setting position for writing boot block");
        bscleanup();
		return (false);
	}
	if (!WriteFile(phyhndl, (char *)bootblock, 512, &size, NULL))
	{
		seterrmsg("Error writing bootstrap boot block");
        bscleanup();
		return (false);
	}
	free(bootimg);
    return (true);						// All done!
}




//*************************************************
// Function: bscleanus - Clean up before error exit
// Returned: Nothing
//*************************************************

static void bscleanup(void)
{
	free(bootimg);
}
