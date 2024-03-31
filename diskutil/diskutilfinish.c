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


int diskutilfinish(void)
{
	long rtn;

	// Close the disk/partition to force everything out

	if (!diskutilclose())
		return (FALSE);

	// Reopen the disk/partition so we can dismount it

	if ((diskutil_hndl = svcIoOpen(XO_RAW|XO_NOMOUNT, diskutil_name, NULL)) < 0)
	{
		diskutil_error(diskutil_hndl, "Error reopening disk %s", diskutil_name);
		return (FALSE);
	}
	diskqab.handle = diskutil_hndl;
    if ((rtn = svcIoQueue(&diskqab)) < 0 || (rtn = diskqab.error) < 0)
	{
		diskutil_error(rtn, "Error dismounting disk");
		return (FALSE);
	}
	svcIoClose(diskutil_hndl, 0);
	if ((rtn = svcIoDevParm(XO_RAW, diskutil_name, NULL)) < 0)
	{
		diskutil_error(rtn, "Error remounting disk %s", diskutil_name);
		return (FALSE);
	}
	return (TRUE);
}
