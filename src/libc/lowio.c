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


static struct
{	BYTE4PARM pos;
	uchar     end;
} parms =
{	{PAR_SET|REP_DECV, 4, IOPAR_ABSPOS}
};

//*****************************************************************
// Function: __nextblk - Internal function to advance to next block
//				of file - called for both input and output
// Returned: 0 if normal or EOF if error (errno is set)
//*****************************************************************

// This function is called whenever an attempt is made to read a byte past
//   what is in the buffer or to write a byte beyond the end of the buffer.
//   It is also called on the first attempt to read or write a byte after
//   a file is opened and after the offset in the file has been changed by
//   calling fseek.

int XOSFNC __nextblk(
	FILE *file,
	int   writing)
{
	long bfrofs;
	long filpos;
	long amnt;

	if (file->_buffer == NULL)			// Do we need to allocate a buffer?
	{
		if ((file->_buffer = (char *)malloc(BUFSIZ)) == NULL)
			return (EOF);
		file->_bufsize = BUFSIZ;
		file->_flag |= _ALLOC;			// Remember have allocated the buffer
	}
	if (file->_flag & _NEWPOS)			// Has the file offset been changed?
	{
		// Here if the offset in the file has been changed. We also get here
		//   on the first attempt to read or write the file. Note that when
		//   we get here we know that the value of _length matches the actual
		//   length of the file (it was updated by fseek if necessary).

		file->_flag &= ~_NEWPOS;
		if ((file->_flag & _APPEND) && writing)
			file->_newpos = file->_length;
		bfrofs = file->_newpos % file->_bufsize;
		filpos = file->_newpos - bfrofs;
		if (file->_newpos > file->_length) // Are we extending the file?
		{
			if (!writing)				// Yes - are we writing?
				return (EOF);			// No - indicate EOF
			if (filpos != file->_position) // Are we at the right block now?	
			{							// No
				if ((file->_position + file->_bufsize) > file->_length)
				{						// Are we at the last block now?
					// Here extending the file, the current block is not the
					//   target block, and the current block is the last block
					//   in the file. We must clear anything in the block not
					//   yet written;

					if ((amnt = file->_length % file->_bufsize) != 0)
					{
						memset(file->_buffer + amnt, 0, file->_bufsize - amnt);
						file->_flag |= _DIRTY;
					}

					// Must set _length here so __writeblk will do the right
					//   thing - it will be set to the final value below.

					file->_length = file->_position + file->_bufsize;
				}
				if ((file->_flag & _DIRTY) && __writeblk(file) < 0)
					return (EOF);
				file->_position = filpos;

				// The current block will be the new last block in the file.
				//   zero out the bytes up to the EOF.

				if (bfrofs != 0)
				{
					memset(file->_buffer, 0, bfrofs);
					file->_flag |= _DIRTY;
				}
			}
			else
			{
				// Here if extending the file and we have the correct target
				//   block in the buffer. Thus we know that everything we need
				//   to do is local to this block. All we need to zero out the
				//  area between the current and the new EOF.

				amnt = file->_newpos - file->_length;
				memset(file->_buffer + (file->_length % file->_bufsize),
						0, amnt);
				file->_flag |= _DIRTY;		// Clear the new space
			}
			file->_length = file->_newpos;
		}
		else
		{
			// Here if not extending the file

			if (filpos != file->_position) // Are we at the right block now?	
			{							 // No
				if ((file->_flag & _DIRTY) && __writeblk(file) < 0)
					return (EOF);		// Write out the current block if
										//   necessary
				file->_position = filpos;
				if (file->_position >= file->_length)
				{
					if (!writing)
						return (EOF);
				}
				else if (__readblk(file) < 0) // Read the new current block
					return (EOF);
			}
		}
		file->_position = filpos;
		file->_ptr = file->_buffer + bfrofs;
		file->_cnt = file->_bufsize - bfrofs;
		amnt = file->_length - file->_newpos;
		if (file->_cnt > amnt)
			file->_cnt = amnt;
	}
	else
	{
		// Here if the offset in the file has not been changed. This means
		//   we got here because of an attempt to advance beyond the end of
		//   the buffer.

		if (!writing && (file->_position + file->_bufsize) >= file->_length)
										// Are we attempting to read beyond
										//   the EOF?
			return (EOF);				// Yes
		if ((file->_flag & _DIRTY) && __writeblk(file) < 0)
										// Write current block if need to
			return (EOF);				// If error
		if ((file->_position += file->_bufsize) >= file->_length)
		{
			if (!writing)
				return (EOF);
			amnt = 0;
		}
		else if ((amnt = __readblk(file)) < 0)
			return (EOF);
		file->_cnt = amnt;
		file->_ptr = file->_buffer;
	}
	return (0);
}


//*****************************************************************
// Function: __readblk - Internal function to fill buffer from file
// Returned: Amount read if normal or EOF if error (errno is set)
//*****************************************************************

int XOSFNC __readblk(
	FILE *file)
{
	long amnt;

	parms.pos.value = file->_position;
	if ((amnt = svcIoInBlockP(file->_handle, file->_buffer, file->_bufsize,
			(char *)&parms)) <= 0)
	{
		if (amnt != ER_EOF)				// EOF returns "EOF" but does NOT set
			errno = -amnt;				//   errno. Errors also return "EOF"
		return (EOF);					//   AND set errno.
	}
	file->_flag &= ~_DIRTY;
	return (amnt);
}


//**********************************************************
// Function: __writeblk - Internal function to output buffer
// Returned: 0 if normal or EOF if error (errno is set)
//**********************************************************

// If the device cannot seek, the amount written is always determined by _ptr.
//   Otherwise it is determined by _ptr if it is the last block in the file.
//   If it is not the last block, the entire block is always written.

int XOSFNC __writeblk(
	FILE *file)
{
	long amnt;
	long rtn;

	if (file->_flag & _CANSEEK)
	{
		if ((file->_position + file->_bufsize) > file->_length)
		{
			amnt = file->_position + (file->_ptr - file->_buffer);
			if (file->_length < amnt)
				file->_length = amnt;
			amnt = file->_length - file->_position;
		}
		else
			amnt = file->_bufsize;
		parms.pos.value = file->_position;
		rtn = svcIoOutBlockP(file->_handle, file->_buffer, amnt,
				(char *)&parms);
		if (file->_length < (file->_position + amnt))
			file->_length = file->_position + amnt;
	}
	else
	{
		amnt = file->_ptr - file->_buffer;
		rtn = svcIoOutBlock(file->_handle, file->_buffer, amnt);
		file->_length += amnt;
	}
	if (rtn < 0)
	{
		errno = -rtn;
		return (EOF);
	}
	file->_flag &= ~_DIRTY;
	return (0);
}


static struct
{	BYTE4PARM devsts;
	BYTE4PARM length;
	BYTE4PARM pos;
	uchar     end;
} fparms =
{	{PAR_GET|REP_HEXV, 4, IOPAR_DEVSTS},
	{PAR_GET|REP_DECV, 4, IOPAR_LENGTH},
	{PAR_GET|REP_DECV, 4, IOPAR_ABSPOS}
};

//***************************************************
// Function: __fsetup - Set up an IO stream for use
// Returned: Address of the streams IOB if OK or NULL
//				if error (errno is set)
//***************************************************

// The _handle, _next, and _prev items in the FILE must be set before calling
//   _fsetup. All other items in FILE are initialized here. The current
//   in a file is preserved and initial IO will be done at that position if
//   it is not changed with fseek.

// It is assumed that the caller has cleared the FILE structure. The _cnt
//   item is left at 0, which causes any attempt to read from the file to call
//   __nextblk which will read the initial block. The _ptr item is initialized
//   to 0xF0000000 which causes any attempt to write to the file to call
//   __nextblk which will read the initial block if necessary. The _NEWPOS
//   flag bit is set and the _newpos item is set to the current position in
//   the file. The _length item is set to the actual length of the file.

FILE XOSFNC *__fsetup(
	FILE *file)
{
	long rtn;

	file->_ungotten = 0xFFFFFFFF;		// No ungotten character
	file->_ptr = (char *)0xF0000000;
	if ((rtn = svcIoInBlockP(file->_handle, NULL, 0, (char *)&fparms)) < 0)
	{
		errno = -rtn;
		return (NULL);
	}
	if (fparms.devsts.value & (DS_DOUT|DS_QOUT)) // Open for output?
		file->_flag |= _WRITE;			// Yes
	if (fparms.devsts.value & (DS_DIN|DS_QIN)) // Open for input?
		file->_flag |= _READ;			// Yes
	if (fparms.devsts.value & DS_UNBFRD) // Unbuffered device?
		file->_flag |= _IONBF;			// Yes
	else
	{
		file->_flag |= _NEWPOS;			// No - initicate need to set position
		if (fparms.devsts.value & (DS_FILE|DS_RANDOM)) // can device seek?
			file->_flag |= (_CANSEEK|_READ); // Yes - remember that, also
											 //   device which seek must be
											 //   open for reading as well as
											 //   writing
		file->_newpos = fparms.pos.value;
		file->_length = fparms.length.value;
		file->_position = 0xFFFFFFFF;	// This forces reading of the initial
	}									//   block
	return (file);
}
