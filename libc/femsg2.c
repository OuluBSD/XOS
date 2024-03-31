// ++++
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xosstr.h>
#include <xoserr.h>
#include <xos.h>
#include <xossvc.h>
#include <dirscan.h>


//************************************************
// Function: femsg2 - Displays fatal error message
// Returned: Never returns
//************************************************

void femsg2(
	char *progname,
	char *msg,
	long  code,
	char *name)
{
	char *pnt;
	char  bufr1[256];
	char  bufr2[272];

	svcIoOutBlock(DH_STDERR, bufr2, sprintf(bufr2, "\n? %s: %s\n",
			progname, msg));
	bufr1[0] = 0;
	pnt = bufr1;
	if (code != 0)
		pnt += svcSysErrMsg(code, 0x03, bufr1);

	if (name != NULL && name[0] != 0)
	{
		if (bufr1[0] != 0)
			*pnt++ = ' ';
		strmov(pnt, name);
	}
	if (bufr1[0] != 0)
		svcIoOutBlock(DH_STDERR, bufr2, sprintf(bufr2, "%*s%s\n",
				strlen(progname) + 4, "", bufr1));
	exit(1);
}
