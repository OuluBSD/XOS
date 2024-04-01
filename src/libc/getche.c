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
#include <xostrm.h>
#include <errno.h>
#include <string.h>
#include <xossvc.h>


struct
{	BYTE4PARM getmode;
	BYTE4PARM clrmode;
	BYTE4PARM setmode;
	uchar     end;
} parm1 =
{	{PAR_GET|REP_HEXV, 4, IOPAR_TRMCINPMODE},
	{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
	{PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_IMAGE|TIM_ECHO}
};
struct
{	BYTE4PARM clrmode;
	BYTE4PARM setmode;
	uchar     end;
} parm2 =
{	{PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, 0xFFFFFFFF},
	{PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE}
};


//******************************************************************
// Function: getche -  Input single character from console with echo
// Returned: Character value or 0 special character prefix or EOF if
//				error (errno is set)
//******************************************************************

int getche(void)
{
	int rtn;

	if ((rtn = svcIoInSingle(DH_STDTRM)) < 0) // Get a character
	{
		errno = -rtn;
		return (EOF);
	}
	return (rtn);
}
