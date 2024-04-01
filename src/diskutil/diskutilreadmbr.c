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
{	BYTE4PARM pos;
	uchar     end;
} posparm =
{	{PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS}
};


//*****************************************
// Function: diskutilreadmbr - Read the MBR
// Returned: TRUE if normal, FALSE if error
//*****************************************

// The diskutil_error function is called before an error is returned.
// The diskname, parthndl, clussize, clusters, blocks, groupsize values
//   are set by this function.

int diskutilreadmbr(
	MBR *mbr)
{
	long   rtn;

	// Read the MBR

	posparm.pos.value = 0;
	if ((rtn = svcIoInBlockP(diskutil_hndl, (char *)mbr, 512, &posparm)) < 0)
	{
		diskutil_error(rtn, "Error reading the MBR");		
		return (FALSE);
	}

	// Here with the MBR

	if (mbr->endlbl != 0xAA55)
	{
		diskutil_error(ER_IVMBR, "Incorrect MBR signature");
		return (FALSE);
	}
	return (TRUE);
}
