//===============================================
// open.c - Unix compatible open function for XOS
//===============================================

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
#include <fcntl.h>
#include <share.h>
#include <errno.h>
#include <xos.h>
#include <xossvc.h>

static long convbits(long abits, long sbits);


//****************************************
// Function: open - Open or create a file
// Returned: Handle (positive) if OK or -1
//				if error and errno is set
//****************************************

int open(
	char *name,					// File specification
	int   abits,				// Unix file access bits
	int   prot)					// Unix style access mode (ignored for now)
{
	long hndl;

	prot = prot;						// Keep compiler happy

    if ((hndl = svcIoOpen(convbits(abits, SH_DENYNO >> 4), name, NULL)) < 0)
	{
		errno = -hndl;
		return (-1);
	}
	return (hndl);
}


//********************************************************************
// Function: sopen - Open or create a file with specified sharing mode
// Returned: Handle (positive) if OK or -1 if error and errno is set
//********************************************************************

int sopen(
	char *name,					// File specification
	int   abits,				// Unix file access bits
	int   sbits,				// Unix file sharing bits
	int   prot)					// Unix style access protection (ignored)
{
	long hndl;

	prot = prot;						// Keep compiler happy

    if ((hndl = svcIoOpen(convbits(abits, sbits >> 4), name, NULL)) < 0)
	{
		errno = -hndl;
		return (-1);
	}
	return (hndl);
}


// Define table which gives XOS bits corresponding to the value of the
//   low order 2 bits

static uchar lowbits[] =
{	XO_IN,			// 0 - O_RDONLY
	XO_OUT,			// 1 - O_WRONLY
	XO_IN|XO_OUT,	// 2 - O_RDWR
	XO_IN|XO_OUT	// 3 - Invalid (but we treat it like O_RDWR)
};

typedef struct
{	long unix;
	long xos;
} BT;

// Define table which converts Unix access bits to XOS open bits. Only the
//   bits that are meaningfull to the kernel are specified here.

static BT xtbl[] =
{	{O_EXCL     , XO_FAILEX},	// If O_CREAT is set, fail if file exists
	{O_TRUNC    , XO_TRUNCA},	// Truncate file to zero length if it exists
	{O_CREAT    , XO_CREATE},	// Create new file if it does not exist
	{O_APPEND   , XO_FAPPEND}	// Force append to end of file
};

// Define table which converts Unix file sharing bits to XOS open bits. Since
//   these "bits" really encode a value, this is just a simple lookup table.

static uchar stbl[] =
{	0,						// 0x00 - SH_COMPAT - Compatibility mode
	XO_EXREAD|XO_EXWRITE,	// 0x10 - SH_DENYRW - Exclusive access
	XO_EXWRITE,				// 0x20 - SH_DENYWR - Exclusive write access
	XO_EXREAD,				// 0x30 - SH_DENYRD - Exclusive read access
	0,						// 0x40 - SH_DENYNO - No exclusion
	0,						// 0x50 - Invalid
	0,						// 0x60 - Invalid
	0						// 0x70 - Invalid
};

// Internal function to convert Unix style file access bits and file sharing
//   bits flags to XOS open bits.

static long convbits(
	long abits,
	long sbits)
{
	BT  *bpnt;
	long xbits;
	int  cnt;

	xbits = lowbits[abits & 0x03];

	bpnt = xtbl;
	cnt = sizeof(xtbl) / sizeof(BT);
	do
	{
		if (bpnt->unix & abits)
			xbits |= bpnt->xos;
		bpnt++;
	} while (--cnt > 0);
	return (xbits | stbl[sbits & 0x07]);
}
