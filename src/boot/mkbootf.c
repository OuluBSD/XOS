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
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <xcstring.h>
#include <procarg.h>
#include <errno.h>
#include <malloc.h>
#include <xcmalloc.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrun.h>
#include <xoserr.h>
#include <xosrtn.h>
#include <xosxdfs.h>
#include <runread.h>
#include "mkboot.h"

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
//   There must not be and additional segments or msects. If additional code
//   is to be added (such as a debugger, it should be added to the boot_m
//   msect. Since .RUN files do not contain names for segments or msects, we
//   use the starting offset to identify each msect. For the file system
//   msects, we look at the lable in the first 4 bytes of the msect. This
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

// This version of mkboot only installs version 8 bootstraps. It cannot update
//   any bootstrap previous to version 8.0. An older version must be removed
//   with rmboot before installing a version 8 bootstrap. rmboot can remove
//   any version bootstrap.

// Define reserved blocks used. The RBLOCK value is only used when initially
//   installing the bootstrap. When updating, the previously used RBLOCK value
//   is used.

#define RBLKOLDBT  10			// Block number where old boot block is saved
#define RBLKCONFIG 11			// First block number which contains
								//    configuration data (2 blocks)
#define RBLKLEVEL2 13			// First Block which contains the level2 code

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
static MSDATA  *iodata;
static MSDATA   gcodedata;
static MSDATA   gdatadata;

static long  memdev;			// Device handle for output (MEM) file
static long  diskdev;			// Device handle for physical disk device

static BOOTBLK32 oldbootblk;

static struct
{	CFGBLK;
	uchar fill[1024 - sizeof(CFGBLK)];
}  cfgblk = {{"XOS BOOT LOADER"}};

static RUNHEAD  runhead;
static RUNSEG   seghead;
static RUNMSECT mshead;

static uchar haveoldboot;
static uchar fattype;

static struct
{   TEXT8CHAR class;
    TEXT8CHAR fstype;
	TEXT8CHAR unittype;
    BYTE4CHAR partoff;
    char      end;
} diskchar1 =
{   {(PAR_GET|REP_TEXT), 8, "CLASS"},
    {(PAR_GET|REP_TEXT), 8, "FSTYPE"},
	{(PAR_GET|REP_TEXT), 8, "UNITTYPE"},
    {(PAR_GET|REP_DECV), 4, "PARTOFF"}
};

static struct
{   TEXT8CHAR unittype;
    char      end;
} diskchar2 =
{   {(PAR_GET|REP_TEXT), 8, "UNITTYPE"}
};

QAB diskqab =
{
    QFNC_WAIT|QFNC_DEVCHAR,		// open
    0,							// status
    0,							// error
    0,							// amount
    0,							// handle
    0,							// vector
    {0},
    CF_VALUES,					// option
    0,							// count
    NULL,						// buffer1
    (char *)&diskchar1,			// buffer2
    NULL						// parm
};

char bterrmsg[] = "? Boot error - press any key\r\n\0\x55\xAA";

static void   baddevname(int num);
static void   errclose(void);
static void (*fail)(char *str1, long code, char *str2);
static void   menufail(char *text, int num);
static void (*notice)(char *str);
static void   strspec(FILESPEC *dst, char *spec, int num);


//**************************************************
// Function: mkbootf - Write XOS bootstrap to a disk
// Returned: TRUE if normal, FALSE if error
//**************************************************

int mkbootf(
    char     *diskname,			// Name of disk/partition
    char     *disktype,			// Disk type: 0x80 = hard disk, 0 = floppy
								//   (returned)
	char     *fstype,			// File system type (returned)
	long     *maxresrvd,		// Maximum reserved block used (returned)
	char     *bootname,			// File specification for boot.run
    char      fname[12][64],
    char     *defprog,
    char      autoboot,
    int       timeout,
    char      flags,
    mkbfail   failarg,			// void fail(char *str1, long code, char *str2)
    mkbnotice noticearg)		// void notice(char *str)
{
	uchar *cpnt;
	long   rtn;
	int    num;
	int    width;
	int    numpline;
	int    numlines;
	int    numitems;
	int    begin;
	int    line;
	int    pos;
	int    len;
	int    cnt;
	int    rsvneed;
	char   bufr[128];
	uchar  fsbits;

    fail = failarg;
    notice = noticearg;
    memdev = 0;
    diskdev = 0;

	// Open the physical device

    if ((diskdev = svcIoOpen(XO_IN|XO_OUT|XO_PHYS, diskname, NULL)) < 0)
    {
        fail("Error opening physical disk", diskdev, diskname);
        errclose();
        return (FALSE);
    }
    diskqab.handle = diskdev;
    if ((rtn = svcIoQueue(&diskqab)) < 0 || (rtn = diskqab.error) < 0)
    {
        fail("Error obtaining disk class name", rtn, diskname);
        errclose();
        return (FALSE);
    }
    if (strcmp(diskchar1.class.value, "DISK") != 0)
    {
        sprintf(bufr, "Device %s is not a disk", diskname);
        fail(bufr, 0, NULL);
        errclose();
        return (FALSE);
    }
    diskqab.buffer2 = (char *)&diskchar2;
    if ((rtn = svcIoQueue(&diskqab)) < 0 || (rtn = diskqab.error) < 0)
    {
        fail("Error obtaining disk unit type", rtn, diskname);
        errclose();
        return (FALSE);
    }
    *disktype = (stricmp(diskchar2.unittype.value, "HARD") == 0)? 0x80: 0;

    // Read the current boot block

    if ((rtn = svcIoSetPos(diskdev, 0, 0)) < 0)
    {									// Position to read boot block
        fail("Error setting position for reading", rtn, diskname);
        errclose();
        return (FALSE);
    }
    if ((rtn = svcIoInBlock(diskdev, (char *)&oldbootblk, 512)) < 0)
    {
        fail("Error reading boot block", rtn, diskname);
        errclose();
        return (FALSE);
    }

	// Make sure have a valid boot block (This is not an extremely strict
	//   check, mainly it is intended to eliminate disks that have not been
	//   formated.

	if (oldbootblk.label != 0xAA55 || oldbootblk.secsize != 512 ||
			(oldbootblk.jump[0] != 0xEB && oldbootblk.jump[0] != 0xE9))
	{
		fail("Current boot block is not valid", 0, diskname);
        errclose();
		return (FALSE);
	}
    if (strncmp(oldbootblk.errmsg, bterrmsg, sizeof(bterrmsg)) == 0)
	{
		if (oldbootblk.bsversion < 8)	// Can we deal with this version?
		{
			fail("Cannot update bootstrap previous to version 8.0\n"
					"          Remove old bootstrap first", 0, NULL);
			errclose();
			return (FALSE);
		}
		haveoldboot = TRUE;
	}

	// Open the file which contains the bootstrap code

	if (!runreadopen(bootname, &runhead, sizeof(runhead), fail))
	{
		errclose();
		return (FALSE);
	}

	// Read the bootstrap file headers and find and read the msects we need.
	//   We must have the msect containing the level 1 code (which has a base
	//   address  of 0:7C00), the msect containing the common level 2 code
	//   (which has a base address of 0:8200, and the msect containg the IO
	//   routines for the file system (which has a base address of 0:0 - we
	//   identify the file system type by reading the label bytes at the
	//   beginning of the msect).

    if (runhead.numsegs != 1)
    {
        fail("Input file does not contain 1 segment", 0, NULL);
        errclose();
        return (FALSE);
    }
    if (!runreadgetsegheader(&seghead, sizeof(seghead))) // Read the segment
		return (FALSE);									 //   header

	printf("### have the segment header\n");

	cnt = seghead.nummsc;
	while (--cnt >= 0)
	{
	    if (!runreadgetmsectheader(&mshead, sizeof(mshead))) // Read the next
			return (FALSE);									 //   msect header

		printf("### have msect header: %0X\n", mshead.addr);

		if (mshead.addr == 0x7C00)		// Is this the level 1 code?
		{
			if ((level1data.code = runreadmsect(&mshead, 512)) == NULL)
				return (FALSE);
			if (mshead.alloc != 512)
			{
				fail("Level 1 code msect is not 512 bytes long", 0, NULL);
		        errclose();
				return (FALSE);
			}
			if (((BOOTBLK32 *)level1data.code)->jump[0] != 0xEB ||
					((BOOTBLK32 *)level1data.code)->jump[2] != 0x90 ||
					strncmp(((BOOTBLK32 *)level1data.code)->errmsg, bterrmsg,
					sizeof(bterrmsg + 2)) != 0)
			{
				fail("Level 1 code is invalid", 0, NULL);
		        errclose();
				return (FALSE);
			}
			else if (((BOOTBLK32 *)level1data.code)->bsversion < 8)
			{							// Can we deal with this version?
				fail("Cannot install bootstrap previous to version 8.0",
						0, NULL);
				errclose();
				return (FALSE);
			}
			level1data.length = 512;
		}
		else if (mshead.addr == 0x8200)	// Is this the common level 2 code?
		{
			if ((level2data.code = runreadmsect(&mshead, 512)) == NULL)
				return (FALSE);
			level2data.length = mshead.alloc;
		}
		else if (mshead.addr == 0)		// Is this file system dependent code
		{								//   or gecko code or data?
			if ((cpnt = runreadmsect(&mshead, 512)) == NULL)
				return (FALSE);
			if (memcmp(cpnt, "FAT*", 4) == 0)
			{
				fatdata.code = cpnt;
				fatdata.length = mshead.alloc;

				printf("### have FAT*, len = %d\n", fatdata.length);
			}
			else if (memcmp(cpnt, "XFS*", 4) == 0)
			{
				xfsdata.code = cpnt;
				xfsdata.length = mshead.alloc;

				printf("### have XFS*, len = %d\n", xfsdata.length);
			}
			else if (memcmp(cpnt, "GCOD", 4) == 0)
			{
				gcodedata.code = cpnt;
				gcodedata.length = mshead.alloc;

				printf("### have GCOD, len = %d\n", gcodedata.length);
			}
			else if (memcmp(cpnt, "GDAT", 4) == 0)
			{
				gdatadata.code = cpnt;
				gdatadata.length = mshead.alloc;

				printf("### have GDAT, len = %d\n", gdatadata.length);
			}
		}
		else
		{
			fail("Unexpected msect encountered", 0, bootname);
	        errclose();
			return (FALSE);
		}
	}

	printf("### finished reading\n");

    // Here with all headers and msect data read. See if we found everything
	//   we need.

	if (level1data.code == NULL)
	{
		fail("No level 1 code found in", 0, bootname);
        errclose();
		return (FALSE);
	}
	if (level2data.code == NULL)
	{
		fail("No common boot code found in", 0, bootname);
        errclose();
		return (FALSE);
	}

	// See if we have the file system  code for the type of file system that
	//   we have.  This version of MKBOOT can handle FAT and XFS file systems
	//   only.

	if (fatdata.code != NULL)
	{
		if (strcmp(diskchar1.fstype.value, "FAT12") == 0)
		{
			strcpy(fstype, "FAT12");
			fattype = 12;
			fsbits = 0;
			iodata = &fatdata;
		}
		else if (strcmp(diskchar1.fstype.value, "FAT16") == 0 ||
				strcmp(diskchar1.fstype.value, "FAT16H") == 0)
		{
			strcpy(fstype, "FAT16");
			fattype = 16;
			fsbits = BBDRV_FAT16;
			iodata = &fatdata;
		}
		else if (strcmp(diskchar1.fstype.value, "FAT32") == 0)
		{
			strcpy(fstype, "FAT32");
			fattype = 32;
			fsbits = BBDRV_FAT32;
			iodata = &fatdata;
		}
	}
	if (xfsdata.code != NULL && strcmp(diskchar1.fstype.value, "XSF") == 0)
	{
		strcpy(fstype, "XFS");
		fsbits = 0;
		iodata = &xfsdata;
    }
    if (iodata == NULL)
    {
		sprintf(bufr, "No bootstrap IO code is available for the %.8s file "
				"system", diskchar1.fstype.value);
		fail(bufr, 0, NULL);
        errclose();
		return (FALSE);
    }
    runreadclose();						// Close the input file
	if ((flags & MBF_QUIET) == 0)
	{
		if (haveoldboot)
			sprintf(bufr, "Updating XOS %sbootstrap version %d.%d to "
                    "version %d.%d", (flags & MBF_DEBUG) ? "debug " : "",
					oldbootblk.bsversion, oldbootblk.bseditnum,
					((BOOTBLK32 *)level1data.code)->bsversion,
					((BOOTBLK32 *)level1data.code)->bseditnum);
		else
			sprintf(bufr, "Installing XOS %sbootstrap version %d.%d",
					(flags & MBF_DEBUG) ? "debug " : "",
					((BOOTBLK32 *)level1data.code)->bsversion,
					((BOOTBLK32 *)level1data.code)->bseditnum);
		notice(bufr);
	}

	// See if we have enough reserved blocks available

	rsvneed = (level2data.length + 0x0F) & 0xFFFFFFF0;
	cfgblk.filesel = (0x8200 + rsvneed) >> 4;
	rsvneed += ((iodata->length + 0x0F) & 0xFFFFFFF0);
	if (gcodedata.code != NULL)
	{
		cfgblk.gcodesel = (0x8200 + rsvneed) >> 4;
		rsvneed += ((gcodedata.length + 0x0F) & 0xFFFFFFF0);
	}
	if (gdatadata.code != NULL)
	{
		cfgblk.gdatasel = (0x8200 + rsvneed) >> 4;
		rsvneed += ((gdatadata.length + 0x0F) & 0xFFFFFFF0);
	}

	printf("###  file sel: %04X\n", cfgblk.filesel);
	printf("### gcode sel: %04X\n", cfgblk.gcodesel);
	printf("### gdata sel: %04X\n", cfgblk.gdatasel);
	printf("###   max sel: %04X\n", (0x8200 + rsvneed) >> 4);

	rsvneed = (((rsvneed +0x1FF) >> 9) + 3 + RBLKOLDBT);
	*maxresrvd = rsvneed;
	if (rsvneed > oldbootblk.ressec)
	{
		sprintf(bufr, "Not enough reserved sectors available: have %d, need %d",
			oldbootblk.ressec, rsvneed);
		fail(bufr, 0, NULL);
		errclose();
		return (FALSE);
	}

    // Now set up the new boot block.

	if (iodata == &fatdata)
	{
		// Here if have a FAT file system. Copy header part of the boot block
		//   to the new boot block (This will probably copy more than is
		//   needed for FAT16 or FAT12 disks, but this will do no harm since
		//   we don't use the space.)

 		memcpy(level1data.code + 3, ((char *)&oldbootblk) + 3,
				offsetof(BOOTBLK32, xosdrive) - 3);

		// We set the value of drive here in case the current bootstrap was
		//   not written by XOS. We always set 0 as the unit number since there
		//   is no defined mechanism for booting from any other unit. (If a
		//   BIOS supports booting for other disks it normally maps them as
		//   0x80 or 0x00.)

		((BOOTBLK32 *)level1data.code)->xosdrive = fsbits;
		if (stricmp(diskchar1.unittype.value, "HARD") == 0)
		{
			((BOOTBLK32 *)level1data.code)->xosdrive |= BBDRV_HARD;
			((BOOTBLK32 *)level1data.code)->drive = 0x80;
		}
		else
			((BOOTBLK32 *)level1data.code)->drive = 0;
		if (diskchar1.partoff.value != 0)
			((BOOTBLK32 *)level1data.code)->xosdrive |= BBDRV_PART;
		((BOOTBLK32 *)level1data.code)->extsig = 0x29;
		memcpy(((BOOTBLK32 *)level1data.code)->sysid, (fsbits & BBDRV_FAT32) ?
				"FAT32\0" : (fsbits & BBDRV_FAT16) ? "FAT16\0" : "FAT12\0", 8);

		// Store the reserved block values

		((BOOTBLK32 *)level1data.code)->lvl2blk = RBLKCONFIG;
		((BOOTBLK32 *)level1data.code)->lvl2num = rsvneed - 1;
    }
	else
	{
		// Here if have an XSF file system.

		fail("XFS bootstrap not supported yet", 0, NULL);
        errclose();
		return (FALSE);
    }

    // Finish setting up the configuration block

    cfgblk.timeout = autoboot ? 0xFFFF : timeout * 18; // Store timeout value
    strspec(&cfgblk.dftspec, defprog, 0); // Store default program spec
    numitems = 0;
    width = 8;
    numpline = 5;
    num = 0;							// See how much space we need for
	if (fname != NULL)					//   each menu item
	{
	    do
    	{
        	if (fname[num][0])
        	{
            	++numitems;
            	len = strlen(fname[num]);
            	if (len > width)
            	{
                	if (len <= 12)
                	{
                    	width = 12;
                    	numpline = 4;
           	     }
            	    else if (len <= 18)
                	{
                    	width = 18;
      	              numpline = 3;
        	        }
            	    else if (len <= 30)
                	{
                    	width = 30;
            	        numpline = 2;
                	}
        	        else
            	    {
                	    width = 48;
                    	numpline = 1;
                	}
				}
            }
	    } while (++num < 12);
	}
    numlines = (numitems+numpline - 1) / numpline;
    begin = (numpline == 5) ? 9 : 8;
    line = 7;
    pos = begin;
    num = 0;							// Calculate where the menu items
	if (fname != NULL)					//   should be displayed
	{
		do
		{
			if (fname[num][0])
			{
				cfgblk.menu[num].xpos = pos;
				cfgblk.menu[num].ypos = line;
				strspec(&cfgblk.menu[num].spec, fname[num], num + 1);
				if ((pos += (width+6)) > 65)
				{
					++line;
					pos = begin;
				}
			}
		} while (++num < 12);
	}
    if (numlines == 0)
        ++numlines;
    cfgblk.numline = numlines + 2;

	// Write the old boot block if it has not been saved

	if (!haveoldboot)
	{
	    if ((rtn = svcIoSetPos(diskdev, RBLKOLDBT * 512, 0)) < 0)
    	{								// Position to write the block
        	fail("Error setting position for writing boot block", rtn, diskname);
        	errclose();
			return (FALSE);
    	}
    	if ((rtn = svcIoOutBlock(diskdev, (char *)&oldbootblk, 512)) < 0)
    	{
        	fail("Error writing old boot block", rtn, diskname);
        	errclose();
        	return (FALSE);
    	}
	}
	else
	{
	    if ((rtn = svcIoSetPos(diskdev, RBLKOLDBT * 512 + 512, 0)) < 0)
    	{								// Position to write the block
        	fail("Error setting position for writing boot block", rtn, diskname);
        	errclose();
			return (FALSE);
    	}
	}

	// Write the configuration blocks

    if ((rtn = svcIoOutBlock(diskdev, (char *)&cfgblk, 1024)) < 0)
    {
        fail("Error writing configuration block", rtn, diskname);
        errclose();
        return (FALSE);
    }

	// Write the level 2 common boot code

    if ((rtn = svcIoOutBlock(diskdev, level2data.code, (level2data.length +
			0x0F) & 0xFFFFFFF0)) < 0)
    {
        fail("Error writing common boot code blocks", rtn, diskname);
        errclose();
        return (FALSE);
    }

	// Write the file system code

    if ((rtn = svcIoOutBlock(diskdev, iodata->code, (iodata->length + 0x0F) &
			0xFFFFFFF0)) < 0)
    {
        fail("Error writing file system code blocks", rtn, diskname);
        errclose();
        return (FALSE);
    }
	len = ((level2data.length + 0x0F) & 0xFFFFFFF0) +  ((iodata->length +
			0x0F) & 0xFFFFFFF0);
	if (gcodedata.code != NULL)
	{
	    if ((rtn = svcIoOutBlock(diskdev, gcodedata.code,
				(gcodedata.length + 0x0F) & 0xFFFFFFF0)) < 0)
	    {
    	    fail("Error writing debugger code blocks", rtn, diskname);
        	errclose();
        	return (FALSE);
    	}
		len += ((gcodedata.length + 0x0F) & 0xFFFFFFF0);
	}
	if (gdatadata.code != NULL)
	{
	    if ((rtn = svcIoOutBlock(diskdev, gdatadata.code,
				(gdatadata.length + 0x0F) & 0xFFFFFFF0)) < 0)
	    {
    	    fail("Error writing debugger data blocks", rtn, diskname);
        	errclose();
        	return (FALSE);
    	}
		len += ((gdatadata.length + 0x0F) & 0xFFFFFFF0);
	}

	// Zero the rest of the last block

	if ((len = (512 - len) & 0x1FF) > 0)
	{
		memset(&cfgblk, 0, len);
	    if ((rtn = svcIoOutBlock(diskdev, (uchar *)&cfgblk, len)) < 0)
	    {
    	    fail("Error clearing end of last block written", rtn, diskname);
        	errclose();
        	return (FALSE);
    	}
	}

	// Write the modified boot block

    if ((rtn = svcIoSetPos(diskdev, 0, 0)) < 0)
    {									// Position to write the boot block
        fail("Error setting position for writing boot block", rtn, diskname);
        errclose();
        return (FALSE);
    }
    if ((rtn = svcIoOutBlock(diskdev, level1data.code, 512)) < 0)
    {
        fail("Error writing new boot block", rtn, diskname);
        errclose();
        return (FALSE);
    }
	if ((rtn = svcIoClose(diskdev, 0)) < 0)
	{
        fail("Error writing new boot block", rtn, diskname);
        errclose();
        return (FALSE);
    }
    return (TRUE);						// All done!
}


static void strspec(
	FILESPEC *dst,
	char     *spec,
	int       num)
{
	long  value;
	uchar chr;

	dst->dev = 0xFF;
	dst->part = 0xFF;
	if (strchr(spec, ':') != NULL)		// Device or partition specified?
	{
		if ((chr = toupper(*spec++)) == 'D' || chr == 'F' || chr == 'P')
		{
			if (chr != 'P')
			{
				dst->dev = (chr == 'D') ? 0x80 : 0x00;
				value = strtol(spec, &spec, 10);
				if (value > 0x7F)
					baddevname(num);
				dst->dev |= (uchar)value;
				chr = toupper(*spec++);
			}
			if (chr == 'P')
			{
				value = strtol(spec, &spec, 10);
				if (value > 64)
					baddevname(num);
				dst->part = (uchar)value;
				chr = toupper(*spec++);
			}
			if (chr != ':')
				baddevname(num);
		}
	}
	if (strlen(spec) > 59)
		menufail("File specification is too long", num);
	strmov(dst->spec, spec);
}


static void baddevname(num)
{
	menufail("Invalid device or partition name", num);
}


static void menufail(
	char *text,
	int   num)
{
	char buffer[80];

	sprintf(buffer, "%s for /F%d\n", text, num);
	fail(buffer, 0, NULL);
}


//*********************************************************
// Function: errclose - Close all open files after an error
// Returned: Nothing
//*********************************************************

void errclose(void)
{
    if (memdev != 0)
        svcIoClose(memdev, 0);
    if (diskdev != 0)
        svcIoClose(diskdev, 0);
}
