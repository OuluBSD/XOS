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


char *getcwd(
	char  *buffer,
	size_t len)
{
	long rtn;

	if (buffer == NULL)					// Do we need to allocate a buffer?
	{
		if ((rtn = svcIoPath(POPT_GETDEV|POPT_GETPATH, "Z:", NULL,
				0x7FFFFFFF)) <= 0)
		{
			if (rtn == 0)
				rtn = ER_ERROR;
			errno = -rtn;
			return (NULL);
		}
		len = rtn + 2;
		if ((buffer = (char *)malloc(len)) == NULL)
			return (NULL);
	}
	if ((rtn = svcIoPath(POPT_GETDEV|POPT_GETPATH, "Z:", buffer, len)) >= 0)
	{
		if (_clib_dosdrive && buffer[1] != ':')
		{
			if ((len = svcIoGetDosDevice(buffer, buffer, len)) > 0)
				rtn = len;
		}
	}
	return (buffer);
}
