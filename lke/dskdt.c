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
#include <string.h>
#include <xcstring.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.h>

// This program creates a file with the specified createion date and time

char prgname[] = "DSKDT";

struct
{   BYTE8PARM cdt;
	BYTE8PARM mdt;
    char      end;
} opnparms =
{	{PAR_SET|REP_DECV, 8, IOPAR_CDATE, 0},
	{PAR_SET|REP_DECV, 8, IOPAR_MDATE, 0}
};

time_x cdt;


void main(
    int   argc,
    char *argv[])

{
    long rtn;
    long hndl;
	long len;
	char bufr[256];

    if (argc != 7)
    {
        fputs("? Command error, usage is:\n"
              "    DSKDT filename day month year hours minutes\n", stderr);
        exit(1);
    }
	memset(&cdt, 0, sizeof(cdt));
	cdt.dos.tmx_min = atol(argv[6]);
	cdt.dos.tmx_hour = atol(argv[5]);
	cdt.dos.tmx_year = atol(argv[4]);
	cdt.dos.tmx_mon = atol(argv[3]);
	cdt.dos.tmx_mday = atol(argv[2]);
	svcSysDateTime(T_CVDOS2SYS, &cdt);
	opnparms.cdt.value = cdt.sys.dt;
	opnparms.mdt.value = cdt.sys.dt;
	len = sdt2str(bufr, "%ZCreation and modify date-time should be %h:%m:%s "
			"%D-%3n-%l\n", (time_sz *)&cdt.sys);
	fputs(bufr, stdout);
    if ((hndl = svcIoOpen(XO_CREATE|XO_TRUNCA|XO_OUT, argv[1], &opnparms)) < 0)
        errormsg(hndl, "!Error opening file %s", argv[1]);
	if ((rtn = svcIoOutBlock(hndl, bufr, len)) < 0)
        errormsg(hndl, "!Error writing to file %s", argv[1]);
	if ((rtn = svcIoClose(hndl, 0)) < 0)
        errormsg(hndl, "!Error closing file %s", argv[1]);
}
