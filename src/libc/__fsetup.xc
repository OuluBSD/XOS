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
{	BYTE4PARM devsts;
	BYTE4PARM length;
	BYTE4PARM pos;
} parms =
{	{PAR_GET|REP_HEXV, 4, IOPAR_DEVSTS},
	{PAR_GET|REP_DECV, 4, IOPAR_LENGTH},
	{PAR_GET|REP_DECV, 4, IOPAR_ABSPOS}
};

//***************************************************
// Function: _fsetup - Set up an IO stream for use
// Returned: Address of the streams IOB if OK or NULL
//				if error (errno is set)
//***************************************************

// The _handle, _next, and _prev items in the FILE must be set before calling
//   _fsetup. All other items in FILE are initialized here. The current
//   in a file is preserved and initial IO will be done at that position if
//   it is not changed with fseek.

// The _EMPTY flag bit is set. The _cnt item is initialized to 0, which causes
//   any attempt to read from the file to call __nextblk which will read the
//   initial block. The _ptr item is initialized to 0xF0000000 which causes
//   any attempt to write to the file to call __nextblk which will read the
//   initial block is necessary. The _offset item is set to the current
//   position in the file, not to the beginning of the current buffer since
//   we don't know the buffer size yet. The _length item is set to the actual
//   length of the file.

FILE __pascal *__fsetup(
	FILE *file)
{
	long rtn;

	file->_ungotten = 0xFFFFFFFF;		// No ungotten character
	file->_ptr = (uchar *)0xF0000000;
	if ((rtn = svcIoInBlockP(file->_handle, NULL, 0, &parms)) < 0)
	{
		errno = -rtn;
		return (NULL);
	}
	if (parms.devsts.value & DS_UNBFRD)	// Unbuffered device?
		file->_flag |= _IONBF;			// Yes
	else
		file->_flag |= _INITIAL;		// Initicate buffer not set up yet
	if (parms.devsts.value & (DS_FILE|DS_RANDOM)) // File structured device?
	{
		file->_flag |= _FILEDEV;
		file->_length = parms.length.value;
		file->_offset = parms.pos.value;
	}
	return (file);
}
