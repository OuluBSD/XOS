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


//***********************************************
// Function: feof - Check for EOF on an IO stream
// Returned: TRUE if at EOF, FALSE if no at EOF
//***********************************************

int feof(
	FILE *file)
{
	long amnt;

	if (file->_flag & _NEWPOS)
		return (file->_newpos >= file->_length);
	if ((file->_flag & _WRITE) &&
			(file->_position + file->_bufsize) > file->_length)
	{
		// Here if writing and the file position has not been changed and we
		//   have the last block of the file in the buffer. See if the file
		//   has been extended and update the file length if so.

		if ((amnt = file->_position + (file->_ptr - file->_buffer)) >
				file->_length)			// Has the file been extended?
			file->_length = amnt;		// Yes - update the length
	}
	return ((file->_position + file->_bufsize - file->_cnt) >= file->_length);
}
