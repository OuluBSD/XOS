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
#include <xoslib.h>
#include <xoserr.h>
#include <ctype.h>
#include <stddef.h>
#include <xosxdisk.h>
#include <xosxffs.h>

#pragma disable_message (120);

static long XOSFNC stronename(FFB *fib, int recur);

extern char ff_name[], ff_f1pnt[];

void XOSFNC xffoncechk(void)
{
	if (((int)ff_name) != offsetof(FFB, name))
		CRASH('FFB1');
	if (((int)ff_f1pnt) != offsetof(FFB, f1pnt))
		CRASH('FFB2');
}


// Here for the get physical unit information class function (CL_PUNITS) for
//   the FFS class to return detailed device information - For the FFS class
//   this consists of the complete path and name of the open file.
//	c{tdaBuffer1} = Address of user's buffer
//	c{tdaCount}   = Buffer size
//	c{tdaDcb}     = Address of DCB
//	long ffsinfo(void);
// Value returned is the length of the string stored or of the string that
//   would be stored if the buffer was long enough (positive) if normal or
//   a negative XOS error code if error.

long XOSFNC ffsinfo(void)
{
	knlTda.ppfilespec = knlTda.buffer1;
	knlTda.pcfilespec = 0;
	knlTda.psfilespec = knlTda.count - 1;
	knlTda.pvfileoptn = XFO_PATH|XFO_FILE;
	return (ffsstorefilespec(((DSKDCB *)knlTda.dcb)->dkbasefib));
}


//********************************************************************
// Function: ffsstorefilespec - Store complete file spec to user space
// Returned: Number of bytes stored if normal or a negative XOS
//           error code if error
//********************************************************************

// Global variables used: (all are updated here)
//	 char *ppfilespec - Buffer pointer
//   long  pcfilespec - Total number of bytes seen (may be larger than the
//                      number of bytes stored)
//   long  psfilespec - Number of bytes available in the buffer

long XOSFNC ffsstorefilespec(
	FFB  *fib)
{
	long rtn;

	if (fib == NULL)
		return (0);
	if ((knlTda.pvfileoptn & XFO_PATH) && (fib->dirfiba != NULL ||
			(knlTda.pvfileoptn & XFO_MARKDIR)))
	{
		if (knlTda.psfilespec > 0 && (rtn = sysLibStrMov(knlTda.ppfilespec,
				"\\")) < 0)
			return (rtn);
		knlTda.ppfilespec++;
		knlTda.pcfilespec++;
		knlTda.psfilespec--;
	}
	stronename(fib, FALSE);
	if ((rtn = sysUSPutByte(knlTda.ppfilespec, 0)) < 0) // Store the final 0
		return (rtn);
	return (knlTda.pcfilespec);
}


//**********************************************
// Function: stronename - Store one file name
// Returned:
// Returned: Number of bytes stored if normal or
//           a XOS negative error code if error
//**********************************************

// This is called recursively to get the complete path in the right order.

static long XOSFNC stronename(
	FFB *fib,
	int  recur)				// TRUE if recured
{
	long rtn;
	long len;
	long amnt;

	if (knlTda.pvfileoptn & XFO_PATH)	// Do we want the path?
	{
		if (fib->dirfiba == NULL)		// Is this the root?
			return (0);					// Yes - finished
		if ((rtn = stronename(fib->dirfiba, TRUE)) < 0) // No - recur
			return (rtn);
	}
	if (recur || knlTda.pvfileoptn & XFO_FILE)
	{
		len = sysLibStrLen(fib->name);		// Get length of the name
		if (fib->name[len - 1] == '.')		// Remove trailing period
			len--;
		if (knlTda.psfilespec > 0)			// Have any room left?
		{
			amnt = (len <= knlTda.psfilespec) ? len : knlTda.psfilespec;
			if ((rtn = sysLibMemCpy(knlTda.ppfilespec, fib->name, amnt)) < 0)
				return (rtn);
			knlTda.ppfilespec += amnt;
			knlTda.psfilespec -= amnt;
		}
		knlTda.pcfilespec += len;
		if ((fib->attrib & XA_DIRECT) && (recur || // Is this a directory that
				knlTda.pvfileoptn & XFO_MARKDIR))  //   we should mark?
		{
			if (knlTda.psfilespec > 0)		// Yes - do so if there is room
			{
				if (knlTda.psfilespec > 0 &&
						(rtn = sysLibStrMov(knlTda.ppfilespec, "\\")) < 0)
					return (rtn);
				knlTda.ppfilespec++;
				knlTda.psfilespec--;
			}
			knlTda.pcfilespec++;
		}
	}
	return (0);
}
