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
#include <xoslib.h>
#include <xoserr.h>
#include <ctype.h>

static long dorun(QAB *qab, char *epnt, char *extlist[], int ext);


//********************************************************************
// Function: sysIoRunCmd - Run a program trying each device from the
//				definition of an environment string and each extension
//				from a list provided by the caller.
// Returned: 0 if a program was run or 1 if a .BAT file was found or
//				a negative XOS error code if error (All errors are
//				indicated in the value returned. qab->error will also
//				be set for some, but not all, errors.)
//******************************************************************

// NOTE: The value of qab->buffer1 may be changed during execution of the
//       function. It is restored before this function returns. This should
//       have no effect on the caller unless a signal is taken while this
//       function is executing and the signal code looks at this value.

XOSCALL sysIoRunCmd(
	QAB   *qab,				// Address of svcIoRun QAB
	char   spec[],			// Buffer to receive full file specification used
							//   (Must be at least MAXSPEC bytes in length.)
	cchar *dirlist,			// List of directories to look in
	char  *extlist[])		// Pointer to the extension list
{
	char const *epnt;
	char *pnt;
	char *obufr1;
	int   ext;
	int   len;
	int   cnt;
	int   rtn;
	char  bufr[300];
	char  chr;

	obufr1 = qab->buffer1;				// Save his buffer1 value

	// See if an extension was specified and if it was .BAT

	ext = ((rtn = sysLibStrRChr(obufr1, '.')) < 0) ? 0 :
			(sysLibStrICmp(obufr1 + rtn, ".BAT") == 0) ? 1 : 2;
	qab->buffer1 = bufr;

	// See if a device or directory was specified and simply use what was
	//   specified if so

	pnt = obufr1;
	while ((chr = *pnt++) != 0 && chr != ':' && chr != '\\')
		;
	if (chr != 0)
	{
		pnt = bufr + sysLibStrMov(bufr, obufr1);
		rtn = dorun(qab, pnt, extlist, ext);
	}
	else
	{
		// If no device or directory specified try each device and directory
		//   in the directory list. The directory list may be specified
		//   directly or by specifying the name of an environment string.

		if (dirlist[0] != '%')
		{
			if (svcSysGetEnv(dirlist, &epnt) <= 0)
				epnt = "Z:;XOSPRG:;XOSCMD:;XOSDEV:";
		}
		else
			epnt = dirlist + 1;
		len = sysLibStrLen(obufr1);
		while (*epnt != 0)				// Copy next directory specification
		{								//   to our buffer
			pnt = bufr;
			cnt = sizeof(bufr) - 2 - len;
			while ((chr = *epnt) != 0)
			{
				epnt++;
				if (chr == ';' || chr == ',')
					break;
				if (--cnt > 0)
					*pnt++ = chr;
			}
			if (pnt != bufr && pnt[-1] != ':' && pnt[-1] != '\\')
				*pnt++ = '\\';

			pnt += sysLibStrMov(pnt, obufr1); // Add the command name

			if ((rtn = dorun(qab, pnt, extlist, ext)) != ER_FILNF &&
					rtn != ER_DIRNF && rtn != ER_NSDEV)
				break;
		}
	}
	if (spec != NULL)
		sysLibStrMov(spec, qab->buffer1);
	qab->buffer1 = obufr1;				// Restore his buffer1 value
	return (rtn);
}


//***********************************************************************
// Function: dorun - Do run function for each entry in the extension list
// Returned: 0 if a program was run or 1 if a .BAT file was found or
//				a negative XOS error code if error
//***********************************************************************

static long dorun(
	QAB *qab,				// Address of the svcIoRun QAB
	char *epnt,				// Address of byte following spec
	char *extlist[],		// Address of the extension list
	int   ext)				// 0 if no extension, 1 if extension is .BAT, 2 if
							//   other extension specified
{
	char **xtpnt;
	char  *pnt;
	long   rtn;

///	char   debug[128];

	if (ext > 0)						// If extension specified
	{
		if (ext == 1)					// Was .BAT specified?
		{
			if ((rtn = sysIoDevParm(0, qab->buffer1, NULL)) >= 0) // Yes
				rtn = 1;
		}
		else
		{
			if ((rtn = svcIoRun(qab)) >= 0)
				rtn = qab->error;			
		}
		return (rtn);
	}

	// Otherwise try each extinsion in order

	xtpnt = extlist;
	while ((pnt = *xtpnt) != NULL)
	{
		sysLibStrMov(epnt, pnt);

///		sysLibSPrintF(debug, "### trying %s, ext = %d\n", qab->buffer1, ext);
///		sysIoOutString(DH_STDTRM, debug, 0);

		if (sysLibStrICmp(pnt, ".BAT") == 0)
		{
			if ((rtn = sysIoDevParm(0, qab->buffer1, NULL)) >= 0)
				rtn = 1;
		}
		else
		{
			if ((rtn = svcIoRun(qab)) >= 0)
				rtn = qab->error;
		}
		if (rtn != ER_FILNF && rtn != ER_DIRNF && rtn != ER_NSDEV)
			return (rtn);
		xtpnt++;
	}
	return (ER_FILNF);
}
