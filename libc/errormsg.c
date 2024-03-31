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
#include <stdarg.h>
#include <string.h>
#include <xosstr.h>
#include <xoserr.h>
#include <xos.h>
#include <xossvc.h>
#include <dirscan.h>


extern char prgname[];


//**************************************************
// Function: errormsg - Displays fatal error message
// Returned: Never returns
//**************************************************

void errormsg(
	long  code,
	char *fmt, ...)			// Format string, exits after output if first
							//   character is ! (which is not output)
{
	va_list pi;
	char   *pnt;
	char   *progname;
	int     len;
	char    label[64];
	char    text[300];
	char    chr;
	char    doexit;

	progname = prgname;
	doexit = FALSE;
	if (fmt[0] == '!')
	{
		fmt++;
		doexit = TRUE;
	}
   	va_start(pi, fmt);
   	vsnprintf(text, 298, fmt, pi);
	len = snprintf(label, 62, "? %s: ", progname);
	progname = label;
	fmt = text;
	while (TRUE)
	{
		pnt = fmt;
		while ((chr = *pnt) != 0 && chr != '\n')
			pnt++;
		*pnt = 0;
		fprintf(stderr, "%*s%s\n", len, progname, fmt);
		if (chr == 0)
			break;
		fmt = pnt + 1;
		progname = "";
	}
	if (code < 0)
	{
		svcSysErrMsg(code, 0x03, text);
		fprintf(stderr, "%*s%s\n", len, "", text);
	}
	if (doexit)
		exit(1);
}
