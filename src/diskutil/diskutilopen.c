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
#include <stdarg.h>
#include <string.h>
#include <xcstring.h>
#include <ctype.h>
#include <xos.h>
#include <xoserr.h>
#include <xossvc.h>
#include <xosdisk.h>
#include "diskutilfuncs.h"

long   diskutil_hndl;
long   diskutil_blocks = -1;
long   diskutil_numheads;
long   diskutil_numsects;
long   diskutil_numcylns;
char   diskutil_lbamode;
void (*diskutil_error)(si32, char *, ...);

static char physname[32];

static struct
{   BYTE4PARM  optn;
    LNGSTRPARM spec;
    TEXT8PARM  class;
    char       end;
} opnparms =
{  {PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_PHYDEV},
   {PAR_GET|REP_STR , 0, IOPAR_FILESPEC, physname, sizeof(physname),
        sizeof(physname)},
   {PAR_GET|REP_TEXT, 8, IOPAR_CLASS}
};

static struct
{   BYTE4CHAR  blocks;
    BYTE4CHAR  numheads;
    BYTE4CHAR  numsects;
    BYTE4CHAR  numcylns;
	TEXT4CHAR  lbamode;
	BYTE4CHAR  partinx;
    char       end;
} diskchars =
{	{PAR_GET|REP_DECV, 4, "IBLOCKS"},
	{PAR_GET|REP_DECV, 4, "IHEADS"},
	{PAR_GET|REP_DECV, 4, "ISECTS"},
	{PAR_GET|REP_DECV, 4, "ICYLNS"},
	{PAR_GET|REP_TEXT, 4, "LBAMODE"},
	{PAR_GET|REP_HEXV, 4, "PARTN"},
};

char diskutil_name[64];


int diskutilopen(
	char *name,
	void (*errfunc)(si32, char *, ...))
{
	long rtn;

	diskutil_error = errfunc;
	strncpy(diskutil_name, name, sizeof(diskutil_name) - 1);
	if ((diskutil_hndl = svcIoOpen(XO_IN|XO_OUT|XO_PHYS, diskutil_name,
			&opnparms)) < 0)
	{
		diskutil_error(diskutil_hndl, "Error opening disk %s", diskutil_name);
		return (FALSE);
	}
	if (strcmp(opnparms.class.value, "DISK") != 0)
	{
		svcIoClose(diskutil_hndl, 0);
		diskutil_error(ER_NTDSK, "Device %s is not a disk", diskutil_name);
		return (FALSE);
	}

	// Determine disk characteristics

	if ((rtn = svcIoDevChar(diskutil_hndl, &diskchars)) < 0)
	{
		svcIoClose(diskutil_hndl, 0);
		diskutil_error(rtn, "Error getting disk characteristics for %s",
				diskutil_name);
		return (FALSE);
	}

	if ((uchar)diskchars.partinx.value != 0)
	{
		svcIoClose(diskutil_hndl, 0);
		diskutil_error(0, "Device %s is not a physical disk", diskutil_name);
		return (FALSE);
	}
	diskutil_blocks = diskchars.blocks.value;
	diskutil_numheads = diskchars.numheads.value;
	diskutil_numsects = diskchars.numsects.value;
	diskutil_numcylns = diskchars.numcylns.value;
	diskutil_lbamode = (diskchars.lbamode.value[0] == 'Y');
	return (TRUE);
}
