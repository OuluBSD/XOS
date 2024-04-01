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
#include <ctype.h>
#include <procarg.h>
#include <xos.h>
#include <xossinfo.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xosnet.h>
#include <xoserrmsg.h>
#include <xcmalloc.h>
#include "netshow.h"

extern char prgname[];

char *ttbl[] =
{   "?0",
    "L ",
    "R ",
    "F ",
    "?4",
    "?5",
    "?6",
    "?7",
    "?8",
    "LP",
    "RP",
    "FP",
	"?C",
	"?D",
	"?E",
	"?F"
};

static _Packed struct
{	BYTE4CHAR size;
	char      end;
} rtcharsize =
{   {PAR_GET|REP_DECV, 4, "RTSIZE"}
};

static _Packed struct
{	BYTE4CHAR  use;
	LNGSTRCHAR data;
	char       end;
} rtchardata =
{	{PAR_GET|REP_DECV , 4, "RTUSE"},
	{PAR_GET|REP_DATAS, 0, "RTDATA"}
};

void nsroute(void)
{
    long     dev;
    long     rtn;
    int      cnt;
    int      type;
    IPROUTE *rtp;
    char    *tname;
	long     sec;
	long     min;
    char     tbuf[8];
    char     ipbuf[24];
	char     lubuf[24];
	char     hwbuf[24];

    if (argument[0] != 0)
    {
        if (strchr(argument, ':') == NULL)
            strcat(argument, ":");
    }
    else
        strcpy(argument, "IP0:");
    if ((dev = svcIoOpen(XO_PHYS, argument, NULL)) < 0)
        errormsg(prgname, dev, "!Error opening device %s", argument);
	if ((rtn = svcIoDevChar(dev, &rtcharsize)) < 0) // Get maximum size
		errormsg(prgname, rtn, "!Error get routing cache size");		
    rtchardata.data.bfrlen = (rtcharsize.size.value + 1) *
            sizeof(IPROUTE);
    rtchardata.data.buffer = getspace(rtchardata.data.bfrlen);
	if ((rtn = svcIoDevChar(dev, &rtchardata)) < 0) // Get the data
		errormsg(prgname, rtn, "!Error getting routing cache data");		
    cnt = rtchardata.data.strlen/sizeof(IPROUTE);
    rtp = (IPROUTE *)(rtchardata.data.buffer);
    printf("Routing table for IP network device %s\n", argument);
    fputs("Type IP address      Last used       T2L  HW/Router address\n",
			stdout);
    while (--cnt >= 0)
    {
        type = (int)(rtp->type);
        if (type <= 11)
            tname = ttbl[type];
        else
        {
            sprintf(tbuf, "?%d", type);
            tname = tbuf;
        }

///		printf("  t2l=%d  used=%d\n", rtp->t2l, rtp->used);

		// Generate the IP address string

        sprintf(ipbuf, "%d.%d.%d.%d", rtp->ipaddr.c[0], rtp->ipaddr.c[1],
				rtp->ipaddr.c[2], rtp->ipaddr.c[3]);

		// Generate the last used string

		sec = rtp->used / SPPERSEC;
		min = sec / 60;	
		sprintf(lubuf, "%02d:%02d:%02d.%03d", min / 60, min % 60, sec % 60,
				(rtp->used % SPPERSEC) * 5);

		// Generate the HW/router address string

        if (type == RTT_LOCAL || type == RTT_ROUTER)
            sprintf(hwbuf, "%02X-%02X-%02X-%02X-%02X-%02X", rtp->hwaddr.c[0],
					rtp->hwaddr.c[1], rtp->hwaddr.c[2], rtp->hwaddr.c[3],
					rtp->hwaddr.c[4], rtp->hwaddr.c[5]);
        else if (type == RTT_FOREIGN || type == RTT_FPENDING)
            sprintf(hwbuf, "%d.%d.%d.%d", rtp->hwaddr.c[0], rtp->hwaddr.c[1],
                    rtp->hwaddr.c[2], rtp->hwaddr.c[3]);

		// Put it all together and output it

        printf(" %-4s%-16s%-13s%6d  %s\n", tname, ipbuf, lubuf, rtp->t2l,
				hwbuf);
        rtp++;							// Advance to next item
    }
}
