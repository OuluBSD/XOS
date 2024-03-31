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

//*************************************************
// Function: fseek - Set IO position for stream
// Returned: 0 if OK or EOF if error (errno is set)
//*************************************************

// This function does no actual IO. It simply sets the offset in the file
//   were the next IO will be done. It is not an error to set the position
//   beyond the EOF. In this case, an EOF indication will be returned on
//   any following attempt to read from the file. Setting beyond the EOF
//   and then setting back before the EOF before reading will not generate
//   an error.

int fseek(
	FILE *file,					// Stream pointer
	long  position,				// Desired position in file
	int   mode)					// Mode: 0 = Relative to beginning of file
								//       1 = Relative to current position
								//       2 = Relative to end of file
{
	long amnt;

	if ((file->_flag & _CANSEEK) == 0)	// Can the device seek?
	{
		errno = -ER_FUNCM;				// No - can't do this!
		return (EOF);
	}
	file->_ungotten = 0xFFFFFFFF;
	if ((file->_flag & _WRITE) && (file->_flag & _NEWPOS) == 0 &&
			(file->_position + file->_bufsize) > file->_length)
	{
		// Here if writing and the file position has not been changed and we
		//   have the last block of the file in the buffer. See if the file
		//   has been extended and update the file length if so.

		if ((amnt = file->_position + (file->_ptr - file->_buffer)) >
				file->_length)			// Has the file been extended?
			file->_length = amnt;		// Yes - update the length
	}
	switch (mode)						// Get absolute new position in
	{									//   the file based on the mode
	 case 0:
		break;

	 case 1:
		position += ((file->_flag & _NEWPOS) ? file->_newpos :
				(file->_position + file->_bufsize - file->_cnt));
		break;

	 case 2:
		position += file->_length;
		break;

	 default:							// Illegal mode
		errno = -ER_VALUE;
		return (EOF);
	}
	if (position < 0)					// Can't set before the beginning
		position = 0;

///	printf("### fseek: %08X for %d\n", position, file->_handle);

	file->_newpos = position;
	file->_flag |= _NEWPOS;
	file->_cnt = 0;						// This forces all reads to call
										//   __nextblk
	file->_ptr = (uchar *)0xF0000000;	// This forces all writes to call
										//   __nextblk
	return (0);
}
