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
#include <xos.h>
#include <xossinfo.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include "show.h"

extern char prgname[];
extern long errno;

static char sysclass[] = "SYSTEM:";
static struct
{   LNGSTRCHAR iotbl;
    char       end;
} iotbllist =
{{  PAR_GET|REP_DATAS, 0, "IOTABLE", NULL, 0, 0}
};

QAB sizeqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// func
    0,							// status
    0,							// error
    0,							// amount
    0,							// handle
    0,							// vector
    {0},
    CF_ONE,						// option
    sizeof(iotbllist),			// count
    sysclass,					// buffer1
	(char *)&iotbllist,			// buffer2
    NULL						// parm
};


int ioinfo(void)
{
    char *pnt;
    long  rtn;
    int   size;
    
    ++validcount;
    while (TRUE)
    {
		if ((rtn = svcIoQueue(&sizeqab)) < 0 || (rtn = sizeqab.error) < 0)
			femsg(prgname, rtn, NULL);

		printf("### length = %d\n", iotbllist.iotbl.bfrlen);

		size = iotbllist.iotbl.bfrlen + 20;
		iotbllist.iotbl.bfrlen = size;
		iotbllist.iotbl.buffer = malloc(size);
		iotbllist.iotbl.desp |= PAR_GET;
		if (((char *)(iotbllist.iotbl.buffer)) == NULL)
			femsg(prgname, -errno, NULL);
		if ((rtn = svcIoClsChar(sysclass, (char *)&iotbllist)) < 0)
			femsg(prgname, rtn, NULL);
		if (iotbllist.iotbl.strlen == (size - 20))
			break;
		free((char *)(iotbllist.iotbl.buffer));
    }
    fputs("Begin   End     Usage\n", stdout);
    size = iotbllist.iotbl.strlen/20;
    pnt = (char *)(iotbllist.iotbl.buffer);
    while (--size >= 0)
    {
		printf("0x%04.4X  0x%04.4X  %-16.16s\n", *(ushort *)(pnt + 16),
				*(ushort *)(pnt + 18), pnt);
		pnt += 20;
    }
    return (TRUE);
}
