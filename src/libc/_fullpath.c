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
#include <xcstring.h>
#include <xossvc.h>

//***************************************************************
// Function: _fullpath - Return a full path given a relative path
// Returned: Address of the full path string
//***************************************************************

char *_fullpath(
	char  *abspath,
	cchar *relpath,
	size_t maxlen)
{
	cchar *spnt;
	char  *dpnt;
	long   rtn;
	int    len;
	char   dev[20];
	char   chr;

	if ((spnt = strchr(relpath, ':')) == NULL)
	{
		strcpy(dev, "Z:");
		spnt = relpath;
	}
	else
	{
		spnt++;
		if ((len = spnt - relpath) > 17)
		{
			errno = -ER_VALUE;
			return (NULL);
		}
		memcpy(dev, relpath, len);
		dev[len] = 0;
	}
	if ((len = svcIoPath((*spnt != '\\') ? (POPT_GETDEV|POPT_GETPATH) :
			POPT_GETDEV, dev, abspath, maxlen)) < 0)
	{
		errno = -len;
		return (NULL);
	}
	if (_clib_dosdrive && abspath[1] != ':') // Do we need to try and find a
	{										 //   DOS type device name?
										// Yes
		if ((rtn = svcIoGetDosDevice(abspath, abspath, maxlen)) < 0)
		{
			errno = -rtn;
			return (NULL);
		}
		if (rtn != 0)					// Did we find a DOS type name?
			len = rtn;					// Yes - get the right length value
	}

	// When get here abspath contains the definition of the current device
	//   and also the path if the given spec did not begin with \. len
	//   contains the length of the string. We now copy relpath after this
	//   removing any . or .. directory names as we go.

	dpnt = abspath + len;
	if (dpnt[-1] != '\\' && *spnt != '\\') // Do we need \ before relpath?
		*dpnt++ = '\\';					// Yes

	// Scan the string and remove any . or .. names

	while ((chr = *spnt++) != 0)
	{
		if (chr == '\\' && spnt[0] == '.') // Start of directory name that
										   //   begins with .?
		{
			if (spnt[1] == 0)			// At end of string?
				break;					// Yes - just ignore it
			if (spnt[1] == '\\')		// Is the name .?
			{
				spnt++;					// Yes - just skip it
				continue;
			}
			if (spnt[1] == '.' && (spnt[2] == '\\' || spnt[2] == 0))
			{							// Is the directory name ..?
				do						// Yes - remove the previous directory
				{						//   name if there is one
					if ((chr = *--dpnt) == ':')
					{					// Did we get back to the device name?
						errno = -ER_BDSPC; // Yes - fail
						return (NULL);
					}
				} while (chr != '\\');
				spnt += 2;
			}
		}
		else
		{
			if (dpnt >= (abspath + maxlen - 1))
			{
				errno = -ER_DATTR;
				return (NULL);
			}
			*dpnt++ = chr;
		}
	}
	*dpnt = 0;
	return (abspath);
}
