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


//*****************************************************
// Function: setvbuf - Set buffer mode, address or size
// Returned: 0 if normal or EOF if error
//*****************************************************

// If address is not null, if specifies the buffer to use. In this case size
//   must be > 0. mode must always be specified. If size is > 0, it specifies
//   the buffer size (applies to either a buffer specified here or to an
//   allocated buffer if the bufr argument is NULL).

int setvbuf(
	FILE  *file,				// File pointer
	char  *bufr,				// Buffer address
	int    mode,				// Buffer mode (_IOFBF, _IOLBF, or _IONBF)
	size_t size)				// Buffer size
{
	if (file->_buffer != NULL)	// Already have a buffer?
	{
		errno = -ER_FUNCS;		// Yes - can't do this
		return (EOF);
	}
	file->_flag &= ~(_IOFBF|_IOLBF|_IONBF);
	file->_flag |= ((_IOFBF|_IOLBF|_IONBF) & mode);
	file->_buffer = bufr;
	file->_bufsize = size;
	return (0);
}
