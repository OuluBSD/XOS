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


static long  rundev;			// Device handle for input (RUN) file
static long  headpos;

static void (*fail)(char *str1, long code, char *str2);
static char  *runname;

static char  notvalid[] = "File is not a valid stand-alone RUN file";

static int readdata(long  pos, char *buffer, long count);


int runreadopen(
	char    *runnamearg,
	RUNHEAD *runhead,
	int      runheadsz,
	void   (*failarg)(char *str1, long code, char *str2))
{
	runname = runnamearg;
	fail = failarg;
    if ((rundev = svcIoOpen(XO_IN, runname, NULL)) < 0)
    {
        fail("Error opening bootstrap file", rundev, runname);
        runreadclose();
        return (FALSE);
    }
    if (!readdata(0, (char *)(runhead), runheadsz)) // Read the RUN file header
        return (FALSE);
    if (runhead->magic != 0x22D7 || runhead->hdrsize < (runheadsz - 6) ||
            runhead->fmtvrsn != 2 || runhead->imagetype != 1)
    {
        fail(notvalid, 0, runname);		// Make sure valid RUN file
        runreadclose();
        return (FALSE);
    }
	headpos = runhead->hdrsize + 6;		// Set for reading first segment header
	return (TRUE);
}


//************************************************
// Function: runreadmsec - Read an msect
// Returned: Address of allocated block containing
//				msect or NULL if error
//************************************************

// If a round value is specified the size of the allocated block is rounded
//   to modulus this value. The mshead->alloc value is modified to be the
//   size of the block returned.

uchar *runreadmsect(
	RUNMSECT *mshead,
	long      round)			// Modulus to round up size to
{
	uchar *cpnt;

	if (mshead->relsz != 0)
	{
		fail("Relocation required", 0, runname);
        runreadclose();
		return (NULL);
	}
	if (round > 1)
		mshead->alloc = ((mshead->alloc + round - 1) / round) * round;
	if (mshead->datasz > mshead->alloc)
	{
		fail("Invalid msect definition", -errno, runname);
        runreadclose();
		return (NULL);
	}
	if ((cpnt = (uchar *)malloc(mshead->alloc)) == NULL)
	{
		fail("Can't allocate memory for data buffer", -errno, NULL);
        runreadclose();
		return (NULL);
	}
    if (!readdata(mshead->dataos, cpnt, mshead->datasz)) // Read the msect data
		return (NULL);
	if (mshead->alloc > mshead->datasz)	// Clear any excess
		memset(cpnt + mshead->datasz, 0, mshead->alloc - mshead->datasz);
	return (cpnt);
}


int runreadgetsegheader(
	RUNSEG *seghead,
	int     segheadsz)
{
    if (!readdata(headpos, (char *)seghead, segheadsz)) // Read the segment
		return (FALSE);									//   header
    if (seghead->hdrsize < segheadsz || seghead->nummsc == 0)
	{									// Is it long enough and not empty?
		fail(notvalid, 0, runname);		// No - fail
        runreadclose();
		return (FALSE);
    }
	headpos += seghead->hdrsize;		// Yes - advance position to next header
    return (TRUE);
}


int runreadgetmsectheader(
	RUNMSECT *mshead,
	int       msheadsz)
{
    if (!readdata(headpos, (char *)mshead, msheadsz)) // Read the msect header
		return (FALSE);					// Read the msect header
    if (mshead->hdrsize < msheadsz)		// Is it long enough?
    {
		fail(notvalid, 0, runname);		// No - fail
        runreadclose();
		return (FALSE);
    }
	headpos += mshead->hdrsize;			// Yes - advance position to next header
    return (TRUE);
}


//********************************************
// Function: readdata - Read from the RUN file
// Returned: TRUE if OK, FALSE if error
//********************************************

static int readdata(
	long  pos,
    char *buffer,
    long  count)
{
    long rtn;

    if ((rtn = svcIoSetPos(rundev, pos, 0)) < 0)
    {
        fail("Error setting position for reading", rtn, runname);
        runreadclose();
        return (FALSE);
    }
	if (count != 0 && (rtn = svcIoInBlock(rundev, buffer, count)) != count)
    {
		if (rtn < 0)					// Error reported?
			fail("Error reading master bootstrap file", rtn, runname);
		else							// No - report as unexpected EOF
			fail("Unexpected end-of-file on bootstrap file", 0, runname);
		runreadclose();
		return (FALSE);
    }
    return (TRUE);
}


void runreadclose(void)
{
    if (rundev != 0)
	{
        svcIoClose(rundev, 0);
		rundev = 0;
	}
}
