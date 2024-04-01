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
#include <xcstring.h>
#include <xossvc.h>


//****************************************************************
// Function: _searchenv - Search for a file in list of directories
//				specified by an environment string
// Returned: Nothing
//****************************************************************

void _searchenv(
	const char *name,
	const char *envstr,
	char       *path)
{
	char const *epnt;
	char *ppnt;
	int   elen;
	int   flen;
	char  pbufr[_MAX_PATH];
	char  chr;

	if (svcIoDevParm(0, (char *)name, NULL) >= 0)
	{
		strmov(path, name);
		return;
	}
	if ((elen = svcSysGetEnv(envstr, &epnt)) > 0)
	{
		flen = strlen(name);
		do
		{
			ppnt = pbufr;
			while ((chr = *epnt++) != 0 && chr != ';')
			{
				if (ppnt < (pbufr + _MAX_PATH - 2))
					*ppnt++ = chr;
			}
			if (ppnt[-1] != '\\' && ppnt[-1] != '/' && ppnt[-1] != ':')
				*ppnt++ = '\\';
			if ((ppnt + flen) < (pbufr + _MAX_PATH - 2))
			{
				strmov(ppnt, name);
				if (svcIoDevParm(0, pbufr, NULL) >= 0)
				{
					strmov(path, pbufr);
					return;
				}
			}
		} while (chr != 0);
	}
	path[0] = 0;
}
