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

#include <xos.h>
#include <xosx.h>
#include <xossvc.h>
#include <xoslib.h>
#include <xoserr.h>
#include <ctype.h>


typedef struct
{	char label[6];
	char text[1];
} ERRMSG;

#define ERRMAX ((long)&errmax)
extern char    errmax;
extern ERRMSG *errtbl[];


//*******************************************************
// Function: svcLibErrMsg - Generate error message string
// Returned: Length of string generated if normal or
//				negative XOS error code if error
//*******************************************************

XOSCALL sysLibErrMsg(
	long  code,				// Error code value
	long  bits,				// Format bits
							//   Bit 1:  Include error name in braces
							//   Bit 0:  Include error text
							//   If no bits are set, a null string is
							//   generated!
	char *bufr)				// Address of bufr to receive message (must be
							//   at least 80 bytes long)
{
	ERRMSG *epnt;
	char   *tpnt;
	long    inx;

	inx = -code;
	tpnt = bufr;
	if (inx > ERRMAX || (epnt = errtbl[inx]) == 0)
	{
		if (bits & 0x02)
		{
			tpnt += sysLibStrMov(tpnt, "{UNKNW}");
			if (bits & 0x01)
				*tpnt++ = ' ';
		}
		if (bits & 0x01)
			tpnt += sysLibSPrintF(tpnt, "Unknown error code %d", code);
	}
	else
	{
		if (bits & 0x02)
		{
			tpnt += sysLibSPrintF(tpnt, "{%s}", epnt->label);
			if (bits & 0x01)
				*tpnt++ = ' ';
		}
		if (bits & 0x01)
			tpnt += sysLibStrMov(tpnt, epnt->text);
	}
	return (tpnt - bufr);
}
