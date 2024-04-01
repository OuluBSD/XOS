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


//***********************************************
// Function: _makepath - Construct a file path
//				specification from its components
// Returned: Nothing
//***********************************************

void _makepath(
	char       *path,
	const char *drive,
	const char *dir,
	const char *fname,
	const char *ext)
{
	int  left;
	int  len;
	char chr;

	left = _MAX_PATH - 2;
	if (drive != NULL && drive[0] != 0)
	{
		len = strlen(drive);
		if ((left -= len) <= 0)
		{
			*path = 0;
			return;
		}
		path = strmov(path, drive);
		if (drive[len - 1] != ':')
		{
			*path++ = ':';
			left--;
		}
	}
	if (dir != NULL && dir[0] != 0)
	{
		len = strlen(dir);
		if ((left -= len) <= 0)
		{
			*path = 0;
			return;
		}
		while ((chr = *dir) != 0)
		{
			dir++;
			if (chr == '/')
				chr = '\\';
			*path++ = chr;
		}
		if (dir[-1] != '/' && dir[-1] != '\\')
		{
			*path++ = '\\';
			left--;
		}
	}
	len = strlen(fname);
	if ((left -= len) <= 0)
	{
		*path = 0;
		return;
	}
	path = strmov(path, fname);
	if (ext != NULL && ext[0] != 0)
	{
		len = strlen(ext);
		if ((left -= len) <= 0)
		{
			*path = 0;
			return;
		}
		if (ext[0] != '.')
			*path++ = '.';
		strmov(path, ext);
	}
}
