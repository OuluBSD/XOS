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

//*****************************************************
// Function: fopen - ASNI C function to open a file
// Returned: Pointer to the FILE structure for the open
//				file or NULL if error (errno is set)
//*****************************************************

FILE *fopen(
	char const *name,
	char const *mode)
{
	long  handle;
	long  bits;
	int   append;
	int   chr;

	bits = 0;
	append = 0;
	while ((chr = toupper(*mode++)) != 0)
	{
		switch (chr)
		{
		 case 'R':
			bits |= XO_IN;
			if (*mode == '+')
			{
				bits |= XO_OUT;
				mode++;
			}
			break;

		 case 'W':
			bits |= XO_CREATE|XO_TRUNCA|XO_OUT|XO_IN;
			if (*mode == '+')
			{
				bits |= XO_IN;
				mode++;
			}
			break;

		 case 'A':
			append = TRUE;
			bits |= XO_CREATE|XO_OUT|XO_IN;
			if (*mode == '+')
			{
				bits |= XO_IN;
				mode++;
			}
			break;

		 case 't':						// Unsupported letter allowed for
		 case 'b':						//   compatability
		 case 'c':
		 case 'n':
			break;
		}
	}
	if ((handle = svcIoOpen(bits, name, NULL)) < 0)
	{
		errno = -handle;
		return (NULL);
	}
	return (__fopen(handle, append));
}
