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

//******************************************************
// Function: fclose - ASNI C function to close a file
// Returned: 0 if OK or -1 (EOF) if error (errno is set)
//******************************************************

int fclose(
	FILE *file)
{
	long code;
	long rtn;

	if (file == NULL)
		return (0);
	code = 0;
	if (file->_handle > 0 && (file->_flag & _WRITE) &&
			(file->_flag & _DIRTY))
	{
		if ((file->_position + file->_bufsize) > file->_length)
		{								// At last block?
										// Yes
			if ((rtn = file->_position + (file->_ptr - file->_buffer)) >
					file->_length)		// Get the current written length (Which
				file->_length = rtn;	//   may be greater than file->_length
		}								//   if something has been written.)
		code = __writeblk(file);
	}
	rtn = __givefile(file);				// Do the rest of this
	if (code == 0 && rtn < 0)
	{
		errno = -rtn;
		code = -1;
	}
	return (code);
}


//*************************************************************
// Function: __givefile - Closes the device and gives up a FILE
// Returned: 0 if OK, negative XOS error code if error (errno
//				is NOT set)
//*************************************************************

// This is an internal function which is called by fclose and by fopen on
//   errors after the device is open.

long XOSFNC __givefile(
	FILE *file)
{
	FILE *next;
	FILE *prev;
	long  rtn;

	rtn = svcIoClose(file->_handle, 0);	// Close the device/file
	file->_handle = -1;

	// Give up the buffer if we allocated it

	if ((file->_flag & _ALLOC) && file->_buffer != NULL)
	{
		free(file->_buffer);
		file->_buffer = NULL;			// Just to be safe!
	}

	// Give up the FILE (IOB) if it was allocated

	if (file->_next != (FILE *)0xFFFFFFFF) // Is this an allocated FILE?
	{
		next = file->_next;				// Remove the FILE from the FILE list
		if ((prev = file->_prev) != NULL)
			prev->_next = next;
		else
			__filehead = next;
		if (next != NULL)
			next->_prev = prev;
		file->_next = __filefree;		// Put the FILE on the free list
		__filefree = file;
	}
	return (rtn);
}
