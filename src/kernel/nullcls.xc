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

#include <xos.h>
#include <xosx.h>
#include <xossvc.h>
#include <xoserr.h>
#include <ctype.h>
#include <stddef.h>

static DEVCHKFNC nulldevchk;

static long XOSFNC nullopen(char *spec);
static long XOSFNC nullinpblk(void);
static long XOSFNC nulloutblk(void);

// SVC dispatch table for NULL device

static QFDISP nullqfdisp =
{	NULL,				// Mount device
	NULL,				// Clear DCB
	NULL,				// Open additional
	nullopen,			// Open
	NULL,				// Find next file
	NULL,				// Delete
	NULL,				// Rename
	nullinpblk,			// Input block
	nulloutblk,			// Output block
	NULL,				// Reserved
	NULL,				// Special functions
	NULL,				// Get label
	NULL,				// Commit data
	NULL,				// Device info
	NULL				// Verify changed disk
};

static DCB *nulldcb;

static CCB nullccb =
{	'CCB*',				// label
	NULL,				// next    - Address of next CCB
	"NULL",				// name    - Name of this class
	0xFFFFFFFF,			// npfxmsk - Name prefix part mask
	'NULL',				// npfxval - Name prefix part value
	nulldevchk,			// devchk  - Address of device check routine
	NULL,				// fdsp    - Address of class func disp table
	NULL				// clschar - Address of class char table
};


//*******************************************************************
// Function: nullinit - Initializaton routine for the NULL device
// Returned:  0 if no match, 1 if found a match, 2 if need to restart
//            search, or a negative XOS error code if error.
//*******************************************************************

#pragma code_seg (o_CODE, o_CODE);

long XOSFNC nullinit(void)
{
	long rtn;

	if ((rtn = sysIoNewClass(&nullccb)) < 0 ||
			(rtn = sysIoMakeDcb(DS_DIN|DS_DOUT|DS_MLTUSER, sizeof(DCB),
			&nullqfdisp, &nulldcb)) < 0 ||
			(rtn = sysIoLinkDcb(nulldcb, &nullccb)) < 0)
		return (rtn);
	*(long *)nulldcb->name = 'NULL';
	return (0);
}

#pragma code_seg ();

#pragma data_seg (_TBL, o_CODE);

long XOSFNC (*initpnt)(void) = nullinit;

#pragma data_seg ();


//*******************************************************************
// Function: nulldevchk - Device check routine for the NULL device
// Returned:  0 if no match, 1 if found a match, 2 if need to restart
//            search, or a negative XOS error code if error.
//*******************************************************************

static long XOSFNC nulldevchk(
	char  name[16],
	char *path,
	long  rtndcb)
{
	if (name[4] == 0)
	{
		if (rtndcb)
			knlTda.dcb = nulldcb;
		return (1);
	}
	return (0);
}

//*************************************************************
// Function: ipmopen - Open routine for NULL device
// Returned:  0 if normal or a negative XOS error code if error
//*************************************************************

static long XOSFNC nullopen(
	char *spec)
{
	long rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&knlIoMinParams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		return (rtn);
	knlTda.amount = 1;
	return (0);
}


//***********************************************************
// Function: nullinpblk - Input block routine for NULL device
// Returned:  Amount input (always 0) if normal or a negative
//			  XOS error code if error.
//***********************************************************

static long XOSFNC	nullinpblk(void)
{
	long rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&knlIoMinParams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		return (rtn);
	if (knlTda.count != 0)
		knlTda.error = ER_EOF;
	knlTda.status |= QSTS_DONE;
	return (0);
}


//************************************************************
// Function: nulloutblk - Output block routine for NULL device
// Returned:  Amount "output" if normal or a negative XOS
//			  error code if error.
//************************************************************

static long XOSFNC nulloutblk(void)
{
	long rtn;

	if (knlTda.parm != NULL && (rtn = sysIoProcDevParamNew(&knlIoMinParams,
			&knlIoMinDDParams, NULL, 0)) < 0)
		return (rtn);
	knlTda.amount = knlTda.count;		// Always say everyting output
	knlTda.status |= QSTS_DONE;
	return (0);
}
