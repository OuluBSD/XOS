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

#pragma pack (push)
#pragma pack (1)
typedef struct
{	uchar boot;
	uchar bgnhead;
	uchar bgnsect;
	uchar bgncyln;
	uchar type;
	uchar endhead;
	uchar endsect;
	uchar endcyln;
	ulong begin;
	ulong length;
} PTENT;

typedef struct
{	char   btcd[440];
	long   diskid;
	char   fill[2];
	PTENT  part[4];
	ushort label;
} MBR;
#pragma pack (pop)

//***********************************************************************************
// Function: initmbr - Initialize MBR to create single partition that covers the disk
// Returned: true if OK, false if failed
//***********************************************************************************

int initmbr()
{
	swcDateTime dt;
	ulong size;
	int   cylns;

	memcpy(dskbufr, mbrblk, 512);
	dt.GetLocal();						// Get current date and time
	((MBR *)dskbufr)->diskid = (long)(dt.dt >> 20); // Get a disk ID from the dt

	// We always set up the disk with 255 heads and 63 sectors. We do not
	//   these numbers to ever be used for anything!

	if (devsize > 16000000)
		devsize = 16000000;
///	blocks = (long)(devsize);			// Get number of sectors
	cylns = (long)devsize / (63 * 255);
	if (cylns > 1024)
		cylns = 1024;

	// Construct the partition table entry. The partition always begins with
	//   the first sector of a track (1) and ends with the last sector of a
	//   track (sects). It begins with the first or second track (0 or 1)
	//   and ends with the last track (heads - 1).

	((MBR *)dskbufr)->part[0].boot = 0x80;
	((MBR *)dskbufr)->part[0].bgnhead = 1;
	((MBR *)dskbufr)->part[0].bgnsect = 1;
	((MBR *)dskbufr)->part[0].bgncyln = 0;
	((MBR *)dskbufr)->part[0].type = 12;
	((MBR *)dskbufr)->part[0].endhead = 254;
	((MBR *)dskbufr)->part[0].endsect = (uchar)(63 + (((cylns - 1) >> 2) & 0xC0));
	((MBR *)dskbufr)->part[0].endcyln = (uchar)(cylns - 1);
	((MBR *)dskbufr)->part[0].begin = 63;
	((MBR *)dskbufr)->part[0].length = (long)devsize - 63;

	// Write out the master boot record

	if (SetFilePointer(phyhndl, 0, NULL, FILE_BEGIN) < 0)
	{
		setmsg("? Error setting position for MBR");
		return (false);
	}
	if (!WriteFile(phyhndl, dskbufr, 512, &size, NULL))
	{
		seterrmsg("Error writing MBR");
		return (false);
	}
	return (true);
}


