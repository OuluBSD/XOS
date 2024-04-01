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
#include <ctype.h>
#include <io.h>
#include <xos.h>
#include <errno.h>
#include <string.h>
#include <xossvc.h>

//*********************************************************************
// Function: __fopen - Associate an IO stream with a device/file handle
// Returned:  A FILE pointer if OK or NULL if error (errno is set)
//*********************************************************************

FILE pascal *__fopen(
	long  handle,
	int   append)
{
	FILE *file;

	if ((file = __filefree) != NULL)	// Anything on the FILE free list?
		__filefree = file->_next;		// Yes - grap it
	else								// No - create a new FILE
	{
		if ((file = (FILE *)pmalloc(sizeof(FILE))) == NULL)
		{
			svcIoClose(handle, 0);		// Can't get memory - close the file
			return (NULL);				// Return failure (pmalloc set errno)
		}
	}

	// Here with a new FILE

	memset(file, 0, sizeof(FILE));		// Clear the FILE
	file->_handle = handle;				// Store the device handle

	if (__filehead != NULL)
		__filehead->_prev = file;		
	file->_next = __filehead;			// Put this FILE on the active list
	__filehead = file;
	if (__fsetup(file) == NULL)			// Finish setting up the FILE
	{
		__givefile(file);				// Error - give up the FILE (this also
		return (NULL);					//   closes the file)
	}
	if (append)
		file->_flag |= _APPEND;
	return (file);
}
