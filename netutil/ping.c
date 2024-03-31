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
#include <TIME.H>
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSTIME.H>
#include <XOSERRMSG.H>
#include <XOSSTR.H>
#include <XOSNET.H>

#define VERSION 4
#define EDITNO  0


long  timeintv;

char  netdev[520] = "ICMP0:";
char  prgname[] = "PING";

char dstaddr[512];
char message[512] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int  msglen = 52;


char  phyname[20];

_Packed struct
{	TEXT8PARM  clss;
	BYTE4PARM  filoptn;
	LNGSTRPARM filspec;
    char       end;
} opnparms =
{	{PAR_SET|REP_TEXT, 8, IOPAR_CLASS   , "ICMP"},
	{PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_XOSDEV},
    {PAR_GET|REP_STR , 0, IOPAR_FILESPEC, phyname, sizeof(phyname),
			sizeof(phyname)}
};

_Packed struct
{	BYTE8PARM to;
    char      end;
} inparms =
{	{PAR_SET|REP_HEXV, 8, IOPAR_TIMEOUT, 20 * ST_SECOND}
};


/*
_Packed struct
{	LNGSTRPARM addrstr;
	BYTE4PARM  ipaddr;
} addrparms =
{	{PAR_SET|REP_STR , 0, IOPAR_NETRMTNAME, dstaddr, sizeof(dstaddr)},
	{PAR_GET|REP_HEXV, 5, IOPAR_NETRMTNETAS}
};
*/


int  havearg(char *);
int  havetime(arg_data *);
void help(void);

#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{   {"?"   , 0       , NULL, AF(help)   , 0},
    {"HELP", 0       , NULL, AF(help)   , 0},
    {"H"   , 0       , NULL, AF(help)   , 0},
    {"T"   , ASF_NVAL, NULL,    havetime, 0},
    {"TIM" , ASF_NVAL, NULL,    havetime, 0},
    {"TIME", ASF_NVAL, NULL,    havetime, 0},
    {NULL  , 0       , NULL, AF(NULL)   , 0}
};


void main(
	int   argc,
	char *argv[])
{
    time_s time1;
    time_s time2;
    llong  diff;
	long   nethndl;
	long   rtn;
	char   tmtxt[32];
	char   buffer[512];

    if (--argc > 0)
    {
        argv++;
        procarg((cchar **)argv, 0, options, NULL, havearg,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }
    if ((nethndl = svcIoOpen(0, netdev, &opnparms)) < 0)
		errormsg(nethndl, "!Error opening network device %s", netdev);

	printf("Using ICMP device %s:\n", phyname);

///	if ((rtn = sysIoInBlockP(nethndl, NULL, 0, &addrparms)) < 0)
///		errormsg(0x80000000, "!Error resolving destination address");

/*
	printf("Pinging %s:%d.%d.%d.%d::\n", netdev,
			((char *)addrparms.ipaddr.value)[0],
			((char *)addrparms.ipaddr.value)[1],
			((char *)addrparms.ipaddr.value)[2],
			((char *)addrparms.ipaddr.value)[3]);
*/

    for (;;)
    {
        svcSysDateTime(11, &time1);

		if ((rtn = sysIoOutBlock(nethndl, message, msglen)) < 0)
			errormsg(rtn, "!Error sending echo request");

		if ((rtn = sysIoInBlockP(nethndl, buffer, sizeof(buffer),
				&inparms)) < 0)
			errormsg(rtn, "!Error receiving echo response");

        svcSysDateTime(11, &time2);

		if ((diff = time2.dt - time1.dt) > (10 * ST_MINUTE))
			strmov(tmtxt, "> 10 min!");
		else
		{
			*((long *)&diff) /= (ST_MILLISEC/100);
			sprintf(tmtxt, "%,d.%02d ms", (long)diff / 100, ((long)diff) % 100);
		}

        printf("Response received in %s\n", tmtxt);
        if (timeintv != 0)
            svcSchSuspend(NULL, timeintv);
        else
            exit(0);
    }
}

int havearg(
    char *arg)

{
	int   len;
    char *pnt;

	// See if a device name was specified by looking for a single colon. If
	//   no device, use ICMP0: (which is already stored at the beginning of
	//   netdev).

	pnt = ((pnt = strchr(arg, ':')) == NULL || pnt[1] == ':') ? (netdev + 6) :
		netdev;
	if ((len = strlen(arg)) > (sizeof(netdev) - 7))
	{
		fputs("? PING: Destination address is too long\n", stderr);
		exit(1);
	}
	memcpy(pnt, arg, len);
	return (TRUE);
}


int  havetime(arg_data *arg)

{
    timeintv = arg->val.n * ST_SECOND;
    return (TRUE);
}

void help(void)

{
    fprintf(stderr, "PING - version %d.%d\n\n", VERSION, EDITNO);
    fputs("A single argument of the form n.n.n.n which specifies the IP "
          "address of the\n  node to ping must be given\n\n"
          "The following options may also be specified:\n"
	  "  /HELP or /?  Display this help message\n"
          "  /TIME=time   Specify repeat rate in seconds\n", stderr);
    exit(0);
}
