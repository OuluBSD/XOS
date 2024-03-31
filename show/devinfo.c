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

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSINFO.H>
#include <XOSSTR.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>

#include "SHOW.H"

extern char prgname[];

static int clsmax = 5;
static int devmax = 6;
static int typmax = 4;

int devinfo(void)
{
    devinfo_data *blk;
    devinfo_data *bpnt;
    char         *pnt;
	int           left;
	int           len;
    char          name[256];
    char          buffer[400];

    ++validcount;

    blk = (devinfo_data *)getinfo(GSI_DEV);
    name[255] = 0;

	// Determine the required field widths

	left = infocnt;
	bpnt = blk;
    while ((left -= sizeof(devinfo_data)) >= 0)
    {
		if ((len = strnlen(bpnt->cname, 16)) > clsmax)
			clsmax = len;
		if ((len = strnlen(bpnt->dname, 16)) > devmax)
			devmax = len;
		if ((len = strnlen(bpnt->tname, 8)) > typmax)
			typmax = len;
		bpnt++;
	}
    printf("Class%*sDevice%*sType%*s   Sesn  Open Out In  Device\n"
			"Name%*sName%*sName%*s   Proc  Cnt  Cnt Cnt Information\n",
			clsmax - 4, "", devmax - 5, "", typmax - 3, "", clsmax - 3, "",
			devmax - 3, "", typmax - 3, "");
    while ((infocnt -= sizeof(devinfo_data)) >= 0)
    {
        pnt = buffer + sprintf(buffer, "%-*.*s", clsmax, clsmax, blk->cname);
        if (blk->dname[0] != '\0')
        {
            pnt += sprintf(pnt, " %-*.*s %-*.*s ", devmax, devmax, blk->dname,
					typmax, typmax, blk->tname);
            if (blk->spid != 0)
                pnt += sprintf(pnt, "%5d.%-3d", blk->spid >> 16,
						(ushort)(blk->spid));
            else
				pnt = strmov(pnt, "         ");
			pnt += sprintf(pnt, "%4d %3d %3d", blk->ocount, blk->outcnt,
		    blk->incnt);
            name[0] = 0;
            svcSysGetInfo(GSI_DEV, blk->dcbid, name, 255);
            if (name[0] != 0)
			{
				*pnt++ = ' ';
				pnt = strmov(pnt, name);
			}
        }
		*pnt++ = '\n';
		*pnt = 0;
		fputs(buffer, stdout);
        ++blk;
    }
    return (TRUE);
}
