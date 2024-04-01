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
#include <ctype.h>
#include <xos.h>
#include <xoserr.h>
#include <xossvc.h>
#include <xosdisk.h>
#include "diskutilfuncs.h"


static char badchsmsg[] = "CHS values for non-LBA disk are not valid";

static struct
{	BYTE4PARM pos;
	uchar     end;
} posparm =
{	{PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS}
};


// This is a quick and dirty function that will only create the first
//   partition in an empty partition table! It is intented for use after
//   an empty partition table has been created.
// The position and length of the partiion are specified in terms of LBA
//   block numbers for all disks. If this is not an LBA disk, these numbers
//   are adjusted (begin is rounded up and length is rounded down) to
//   match the disks LBA values. If this is an LBA disk which is small
//   enough to have valid CHS values for the partition, arbitrary CHS values
//   are assigned and the the begin and length values are adjusted to match.
//   If it is an LBA disk which is too large to have valid CHS values, the
//   maximum CHS values are used and the begin and length values are NOT
//   adjusted.

int diskutilmakepart(
	MBR *mbr,
	int  type,
	int  active,
	ui32 begin,
	ui32 length)
{
	PTBL *ppnt;
	long  rtn;
	long  bgnblock;			// Beginning block number
	long  top;
	int   cylnsize;
	int   sects;			// Number of sectors
	int   heads;			// Number of heads
	int   cylns;			// Number of cylinders
	int   bgnhead;			// Beginning head number
	int   bgncyln;			// Beginning cylinder number
	int   endcyln;			// Ending cylinder number
	int   ecval;
	int   temp;
	char  bufr[512];

	if (!diskutilreadmbr(mbr))
		return (FALSE);

	if (mbr->ptbl[0].type != 0)
	{
		diskutil_error(0, "First partition slot is in use");
		return (FALSE);
	}
	heads = diskutil_numheads;			// Assume using the CHS values from
	sects = diskutil_numsects;			//   the hardware
	cylns = diskutil_numcylns;
	if (length == 0)
		length = diskutil_blocks - begin;
	else if (length > (diskutil_blocks - begin))
	{
		diskutil_error(0, "Partition extends past end of the disk");
		return (FALSE);
	}
	top = begin + length;				// Get first block above the partition
	if (begin == 0)						// Can't start at block 0!
	{
		begin = 1;
		length--;
	}
	temp = (sects != 0) ? sects : 63;
	bgnblock = (begin + temp - 1) / temp; // Round up to start of the next track
	if (!diskutil_lbamode)				// LBA disk?
	{
		// Not an LBA disk - make sure the hardware CHS values are usable

		temp = heads * sects * cylns;
		if (temp > diskutil_blocks || heads < 1 || heads > 255 || sects < 1 ||
				sects > 63 || cylns < 1)
			diskutil_error(0, badchsmsg);
		temp = cylns;
		cylns = length / (sects * heads); // Get number of cylinders in the
										  //   partition
		if (cylns > temp || cylns > 1023)
			diskutil_error(0, badchsmsg);
	}
	else
	{
		// If an LBA disk, use the hardware CHS values if we have any,
		//   otherwise generate "reasonable" fake CHS values. We try to keep
		//   the cylinder size as small as we can and still have a valid
		//   top cylinder number. This may not always give the optimum value
		//   due to rounding after this calculation but this should not be
		//   a major problem. The amount of space lost is a small percentage
		//   of the total.

		if (heads == 0 || sects == 0 || cylns == 0)
		{
			sects = 63;
			if ((heads = (begin + length + 64511) / 64512) < 2)
				heads = 2;
			cylns = diskutil_blocks / (sects * heads);
		}
	}

	// Round up the beginning block to the beginning of a cylinder (0,1,C)
	//   unless at the beginning of the disk, in which case round it up to
	//   the beginning of the second head (1,1,0).

	cylnsize = heads * sects;

	if (begin <= sects)
	{
		bgncyln = 0;
		bgnhead = 1;
		begin = sects;
	}
	else
	{
		if ((bgncyln = (begin + cylnsize + 1) / cylnsize) > 1023)
			bgncyln = 1023;
		bgnhead = 0;
		begin = bgncyln * cylnsize;
	}
	if ((ecval = (endcyln = top / cylnsize)) > 1023)
		ecval = 1023;

	// Construct the partition table entry. The partition always begins with
	//   the first sector of a track (1) and ends with the last sector of a
	//   track (sects). It begins with the first or second track (0 or 1)
	//   and ends with the last track (heads - 1).

	ppnt = &mbr->ptbl[0];

	ppnt->boot = (active) ? 0x80 : 0;
	ppnt->bgnhead = bgnhead;
	ppnt->bgnsect = ((bgncyln >> 2) & 0xC0) + 1;
	ppnt->bgncyln = (uchar)bgncyln;

	ppnt->type = type;
	ppnt->endhead = heads - 1;
	ppnt->endsect = ((ecval >> 2) & 0xC0) + sects;
	ppnt->endcyln = (uchar)ecval;

	ppnt->begin = begin;
	ppnt->length = endcyln * cylnsize - begin;

	printf("### Boot: %02X Type: %02X\n", ppnt->boot, ppnt->type);
	printf("### Begin CHS: %d,%d,%d Ending CHS: %d,%d,%d Begin: %,d Length: "
			"%,d\n", ppnt->bgnhead, ppnt->bgnsect & 0x3F,
				ppnt->bgncyln + ((ppnt->bgnsect & 0x0C0) << 2),
				ppnt->endhead, ppnt->endsect & 0x3F,
				ppnt->endcyln + ((ppnt->endsect & 0xC0) << 2), ppnt->begin,
				ppnt->length);

	// Write out the master boot record

	posparm.pos.value = 0;
	if ((rtn = svcIoOutBlockP(diskutil_hndl, (char *)mbr, 512, &posparm)) < 0)
	{
		diskutil_error(rtn, "Error writing the master boot record");
		return (FALSE);
	}

	// Clear the parititons's boot block

	memset(bufr, 0, 512);
	posparm.pos.value = begin * 512;
	if ((rtn = svcIoOutBlockP(diskutil_hndl, bufr, 512, &posparm)) < 0)
	{
		diskutil_error(rtn, "Error writing the partition boot block");
		return (FALSE);
	}
	return (TRUE);
}
