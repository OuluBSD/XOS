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
// Function: fgetc - Input single character from stream
// Returned: EOF if error (errno is set) or character
//				(0 filled to 32 bits) if normal
//*****************************************************

// We do not currently support an in-line version of this (getc) so getc
//   is defined to be the same as fgetc.

int fgetc(
	FILE *file)
{
	long rtn;

	if (file->_ungotten != 0xFFFFFFFF)
	{
		rtn = file->_ungotten;
		file->_ungotten = 0xFFFFFFFF;
		return (rtn);
	}
	if (file->_cnt <= 0)
	{
		if (file->_flag & _IONBF)		// Unbuffered device?
		{
			if ((rtn = svcIoInSingle(file->_handle)) < 0)
			{
				errno = -rtn;
				return (EOF);
			}
			return (rtn);
		}
		if (__nextblk(file, FALSE) < 0)
			return (EOF);
	}
	file->_cnt--;
	return (*file->_ptr++);	
}
