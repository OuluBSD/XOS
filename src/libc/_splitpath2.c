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
#include <io.h>
#include <xos.h>
#include <errno.h>
#include <string.h>
#include <xossvc.h>


//********************************************************
// Function: _splitpath2 - Split a file specification
//		into its component parts
// Returned:
//********************************************************

int _splitpath2(
	const char *inp,
	char       *outp,
	char      **drive,
	char      **dir,
	char      **fname,
	char      **ext)
{
	char *pnt;
	int   len;
	int   left;

	left = _MAX_PATH2;
	if (drive != NULL)
		*drive = outp;
	if ((pnt = strchr(inp, ':')) != NULL) // Have a device name?
	{
		len = pnt - inp + 1;			// Yes
		if ((left -= len) <= 0)
			return (FALSE);
		memcpy(outp, inp, len);			// Copy it to the buffer
		outp += len;
		inp += len;
	}
	*outp++ = 0;
	if (dir != NULL)
		*dir = outp;
	if ((pnt = strrchr(inp, '\\')) != NULL) // Have a directory name?
	{
		len = pnt - inp + 1;			// Yes
		if ((left -= len) <= 0)
			return (FALSE);
		memcpy(outp, inp, len);			// Copy it to the buffer
		outp += len;
		inp += len;
	}
	*outp++ = 0;
	if (fname != NULL)
		*fname = outp;
	if (*inp != 0)						// Have anything else?
	{
		if ((pnt = strrchr(inp, '.')) == NULL) // Yes - have an extension?
		{
			len = strlen(inp);			// No
			if ((left -= len) <= 0)
				return (FALSE);
			memcpy(outp, inp, len);		// Copy the name
			outp += len;
			inp += len;
			*outp++ = 0;
			if (ext != NULL)
				*ext = outp;
		}
		else							// Here if have an extension
		{
			len = pnt - inp ;
			if ((left -= len) <= 0)
				return (FALSE);
			memcpy(outp, inp, len);		// Copy the name
			outp += len;
			inp += (len + 1);
			*outp++ = 0;
			if (ext != NULL)
				*ext = outp;
			len = strlen(inp);
			if ((left -= len) <= 0)
				return (FALSE);
			memcpy(outp, inp, len);		// Copy the extension
			outp += len;
		}
	}
	else								// If no name or extension
	{
		if (ext != NULL)
			*ext = outp;
	}
	*outp++ = 0;
	return (TRUE);
}
