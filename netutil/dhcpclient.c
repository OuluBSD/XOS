#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <procarg.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xosswap.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <xosnet.h>
#include "dhcpclient.h"

#define VERSION 1
#define EDITNO  0

long  udphndl;
long  ipshndl;

char  phyname[32];

int   dnamelen;
uchar dname[260];
char  ipaddr[20];

long  dhcpsrvip;

struct 
{	int   dhcpmsgtype;
	long  subnetmask;
	long  dhcpsrvip;
	long  routerip;
	long  dnssrvip;
	long  timersrvip;
	long  namesrvip;
	long  logsrvip;
	long  cookiesrvip;
	long  lprsrvip;
	long  timeoffset;
	long  leasetime;
	long  renewtime;
	long  rebindtime;
} vals;

void doconfig(void);
int  getrsp(int reqtype, long reqipaddr);

void hvsubnetmask(uchar *data);
void hvtimeoffset(uchar *data);
void hvrouter(uchar *data);
void hvtimesrv(uchar *data);
void hvnamesrv(uchar *data);
void hvdnssrv(uchar *data);
void hvlogsrv(uchar *data);
void hvcookiesrv(uchar *data);
void hvlprsrv(uchar *data);
void hvdomainname(uchar *data);
void hvleasetime(uchar *data);
void hvoverload(uchar *data);
void hvdhcpmsgtp(uchar *data);
void hvdhcpsrv(uchar *data);
void hvmessage(uchar *data);
void hvrenewtime(uchar *data);
void hvrebindtime(uchar *data);
void hvunknown(uchar *data);
void hvclientid(uchar *data);
long parsemsg(int len);


typedef struct
{	void (*func)(uchar *data);
	int    size;
} OPTENT;

OPTENT opttbl[] =
{	{hvsubnetmask, 0x84},	// OPT_SUBNETMASK  = 1
	{hvtimeoffset, 0x82},	// OPT_TIMEOFFSET  = 2
	{hvrouter    , 0x84},	// OPT_ROUTER      = 3
	{hvtimesrv   , 0x84},	// OPT_TIMESRV     = 4
	{hvnamesrv   , 0x84},	// OPT_NAMESRV     = 5
	{hvdnssrv    , 0x84},	// OPT_DNSSRV      = 6
	{hvlogsrv    , 0x84},	// OPT_LOGSRV      = 7
	{hvcookiesrv , 0x84},	// OPT_COOKIESRV   = 8
	{hvlprsrv    , 0x84},	// OPT_LPRSRV      = 9
	{hvunknown   , 0x00},	//                 = 10
	{hvunknown   , 0x00},	//                 = 11
	{hvunknown   , 0x00},	//                 = 12
	{hvunknown   , 0x00},	//                 = 13
	{hvunknown   , 0x00},	//                 = 14
	{hvdomainname, 0x01},	// OPT_DOMAINNAME  = 15
	{hvunknown   , 0x00},	//                 = 16
	{hvunknown   , 0x00},	//                 = 17
	{hvunknown   , 0x00},	//                 = 18
	{hvunknown   , 0x00},	//                 = 19
	{hvunknown   , 0x00},	//                 = 20
	{hvunknown   , 0x00},	//                 = 21
	{hvunknown   , 0x00},	//                 = 22
	{hvunknown   , 0x00},	//                 = 23
	{hvunknown   , 0x00},	//                 = 24
	{hvunknown   , 0x00},	//                 = 25
	{hvunknown   , 0x00},	//                 = 26
	{hvunknown   , 0x00},	//                 = 27
	{hvunknown   , 0x00},	//                 = 28
	{hvunknown   , 0x00},	//                 = 29
	{hvunknown   , 0x00},	//                 = 30
	{hvunknown   , 0x00},	//                 = 31
	{hvunknown   , 0x00},	//                 = 32
	{hvunknown   , 0x00},	//                 = 33
	{hvunknown   , 0x00},	//                 = 34
	{hvunknown   , 0x00},	//                 = 35
	{hvunknown   , 0x00},	//                 = 36
	{hvunknown   , 0x00},	//                 = 37
	{hvunknown   , 0x00},	//                 = 38
	{hvunknown   , 0x00},	//                 = 39
	{hvunknown   , 0x00},	//                 = 40
	{hvunknown   , 0x00},	//                 = 41
	{hvunknown   , 0x00},	//                 = 42
	{hvunknown   , 0x00},	//                 = 43
	{hvunknown   , 0x00},	//                 = 44
	{hvunknown   , 0x00},	//                 = 45
	{hvunknown   , 0x00},	//                 = 46
	{hvunknown   , 0x00},	//                 = 47
	{hvunknown   , 0x00},	//                 = 48
	{hvunknown   , 0x00},	//                 = 49
	{hvunknown   , 0x00},	//                 = 50
	{hvleasetime , 0x00},	// OPT_LEASETIME   = 51
	{hvoverload  , 0x00},	// OPT_OVERLOAD    = 52
	{hvdhcpmsgtp , 0x81},	// OPT_DHCPMSGTYPE = 53
	{hvdhcpsrv   , 0x84},	// OPT_DHCPSRV     = 54
	{hvunknown   , 0x00},	//                 = 55
	{hvmessage   , 0x01},	// OPT_MESSAGE     = 56
	{hvunknown   , 0x82},	// OPT_MAXSIZE     = 57
	{hvrenewtime , 0x84},	// OPT_RENEWTIME   = 58
	{hvrebindtime, 0x84},	// OPT_REBINDTIME  = 59
	{hvunknown   , 0x00},	//                 = 60
	{hvclientid  , 0x02},	// OPT_CLIENTID    = 61
};
#define MAXOPT (sizeof(opttbl)/sizeof(OPTENT))


struct
{	DHCPFXD;
	char options[20];
} req =
{	{	OP_BOOT_REQUEST,	// op
		1,					// htype
		6,					// hlen
		0,					// hops
		0,					// xid
		0,					// secs
		0x80, 				// flags
		0,					// ciaddr
		0,					// yiaddr
		0,					// siaddr
		0,					// giaddr
		{0},				// chaddr
		{0},				// sname
		{0},				// file
		0x63538263,			// magic
	}
};


struct
{	DHCPFXD;
	char options[400];
} rsp;


uchar reqlist[] = {OPT_REQLIST, 4, OPT_SUBNETMASK, OPT_DOMAINNAME, OPT_ROUTER,
			OPT_DNSSRV};


struct
{   text4_parm  class;
    byte4_parm  filoptn;
    lngstr_parm filspec;
	byte4_parm  ipaddr;
	byte4_parm  port;
	byte8_parm  hwaddr;
    char        end;
} openparms =
{   {PAR_SET|REP_TEXT, 4, IOPAR_CLASS  , "UDP"},
    {PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_NOPREFIX|XFO_XOSNAME},
    {PAR_GET|REP_STR , 0, IOPAR_FILSPEC, phyname, 0, sizeof(phyname), 0},
	{PAR_GET|REP_HEXV, 4, IOPAR_NETLCLNETA},
	{PAR_SET|REP_HEXV, 4, IOPAR_NETLCLPORT, UDPP_BOOTPC},
	{PAR_GET|REP_HEXV, 8, IOPAR_NETLCLHWA}
};

struct
{	byte4_parm  port;
	byte4_parm  addr;
	char        end;
} sendparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_NETRMTPORTS, UDPP_BOOTPS},
	{PAR_SET|REP_HEXV, 4, IOPAR_NETRMTNETAS, 0xFFFFFFFF}
};


struct
{	byte8_parm  timeout;
	char        end;
} recvparms =
{	{PAR_SET|REP_HEXV, 8, IOPAR_TIMEOUT}
};

struct
{	text16_char  ipsdev;
	char        end;
} udpchars =
{	{PAR_GET|REP_TEXT, 16, "IPSDEV"}
};

struct
{	text4_char  addr;
	text4_char  parm;
	char        end;
} dhcpchars =
{	{PAR_GET|REP_TEXT, 4, "DHCPADDR"},
	{PAR_GET|REP_TEXT, 4, "DHCPPARM"}
};

struct
{	byte4_char ipaddr;
	uchar      end;
} set1chars =
{	{PAR_SET|REP_HEXV, 4, "IPADDR"}
};

struct
{	byte4_char subnet;
	byte4_char router;
	uchar      end;
} set2chars =
{	{PAR_SET|REP_HEXV, 4, "SUBMASK"},
	{PAR_SET|REP_HEXV, 4, "DRT1ADDR"}
};

uchar dhcpaddr;
uchar dhcpparm;

/*
#define AF(func) (int (*)(arg_data *))func

arg_spec options[] =
{   {"A"    , 0, NULL, AF(afunc)     , 0},
    {"CNAME", 0, NULL, AF(cnamefunc) , 0},
    {"C"    , 0, NULL, AF(cnamefunc) , 0},
    {"MX"   , 0, NULL, AF(mxfunc)    , 0},
    {"M"    , 0, NULL, AF(mxfunc)    , 0},
    {"PTR"  , 0, NULL, AF(ptrfunc)   , 0},
    {"P"    , 0, NULL, AF(ptrfunc)   , 0},
    {"?"    , 0, NULL, AF(help)      , 0},
    {"HELP" , 0, NULL, AF(help)      , 0},
    {"H"    , 0, NULL, AF(help)      , 0},
    {NULL   , 0, NULL, AF(NULL)      , 0}
};
*/

char prgname[] = "DHCPCLIENT";

main(argc, argv)
int   argc;
char *argv[];

{
	time_s curdt;
	long   rtn;
	int    retry;
	char   text[20];

	argc = argc;
	argv = argv;

///	if (--argc > 0)
///	{
///		argv++;
///		procarg(argv, 0, options, NULL, havename,
///				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
///	}


	svcSysDateTime(T_GTSYSDTTM, &curdt);
	req.xid = (curdt.high << 24) | (curdt.low >> 8);


    if ((udphndl = svcIoOpen(XO_IN|XO_OUT, "UDP0:", &openparms)) < 0)
		femsg2(prgname, "Error opening UDP device", udphndl, NULL);

	// Find the underlying IPS device and get the DHCP characteristics

	if ((rtn = svcIoDevChar(udphndl, &udpchars)) < 0)
		femsg2(prgname, "Error getting underlying IPS device", rtn, NULL);
	sprintf(text, "%.15s:", udpchars.ipsdev.value);
    if ((ipshndl = svcIoOpen(XO_IN|XO_OUT, text, NULL)) < 0)
		femsg2(prgname, "Error opening underlying IPS device", udphndl, NULL);
	if ((rtn = svcIoDevChar(ipshndl, &dhcpchars)) < 0)
		femsg2(prgname, "Error getting DHCP charicterists", rtn, NULL);

///	svcIoClose(ipshndl, 0);

	dhcpaddr = (dhcpchars.addr.value[0] == 'Y');
	dhcpparm = (dhcpchars.parm.value[0] == 'Y');
	printf("   DHCP used to get IP address: %s\n", (dhcpaddr) ? "YES" : "NO");
	printf("DHCP used to get IP parameters: %s\n", (dhcpparm) ? "YES" : "NO");

	if (!dhcpaddr && !dhcpparm)
	{
		printf("DHCP is not being used\n");
		exit(0);
	}

	*(long *)(req.chaddr + 0) = openparms.hwaddr.value[0];
	*(long *)(req.chaddr + 4) = openparms.hwaddr.value[1];
	printf("\n   Our Hardware Addr: (1) %02X-%02X-%02X-%02X-%02X-%02X\n",
			req.chaddr[0], req.chaddr[1], req.chaddr[2],
			req.chaddr[3], req.chaddr[4], req.chaddr[5]);
	printf("  Initial IP Address: %d.%d.%d.%d\n",
			((uchar *)&openparms.ipaddr.value)[0],
			((uchar *)&openparms.ipaddr.value)[1],
			((uchar *)&openparms.ipaddr.value)[2],
			((uchar *)&openparms.ipaddr.value)[3]);

	req.ciaddr = openparms.ipaddr.value;

	if (openparms.ipaddr.value != 0)	// If we have an IP address now try
	{									//   a REQUEST or INFORM first
		do
		{
			if ((rtn = getrsp((dhcpaddr) ? DHCPREQUEST : DHCPINFORM,
					openparms.ipaddr.value)) < 0)
				femsg2(prgname, "Error sending DHCPREQUEST to server", rtn,
						NULL);
			if (vals.dhcpmsgtype == DHCPACK)
			{
				doconfig();
				exit(0);
			}
		} while (--retry >= 0);
	}		

	// If can cannot get information about a static IP addres or cannot renew
	//   the lease for a dynamic IP addres.

	if (!dhcpaddr)
	{
		printf((openparms.ipaddr.value) ?
				"? Can not get information about current IP address\n" :
				"? DHCP not being used to get IP address and no IP address "
						"specified\n");
		exit(1);
	}
	retry = 3;
	do
	{
		if ((rtn = getrsp(DHCPDISCOVER, 0)) < 0)
			femsg2(prgname, "Error sending DHCPDISCOVER to server", rtn, NULL);
		if (vals.dhcpmsgtype == DHCPOFFER)
		{
			// Here with an offered IP address

			dhcpsrvip = vals.dhcpsrvip;
			if ((rtn = getrsp(DHCPREQUEST, rsp.yiaddr)) < 0)
				femsg2(prgname, "Error sending DHCPREQUEST to server", rtn,
						NULL);
			if (vals.dhcpmsgtype == DHCPACK)
			{
				doconfig();
				exit (0);
			}
		}
	} while (--retry >= 0);

	printf("? Did not get an address!\n");

    exit (0);
}


//************************************************
// Function: doconfig - Configure our IP interface
// Returned: Nothing
//************************************************

void doconfig(void)
{
	long rtn;

	if (dhcpaddr && openparms.ipaddr.value != rsp.yiaddr)
	{
		set1chars.ipaddr.value = rsp.yiaddr;
		if ((rtn = svcIoDevChar(ipshndl, &set1chars)) < 0)
			femsg2(prgname, "Error changing IP address", rtn, NULL);
	}
	set2chars.subnet.value = vals.subnetmask;
	set2chars.router.value = vals.routerip;
	if ((rtn = svcIoDevChar(ipshndl, &set2chars)) < 0)
		femsg2(prgname, "Error changing subnet mask or router address", rtn,
				NULL);
}



int getrsp(
	int    reqtype,
	long   ipaddr)
{
	uchar *dpnt;
	uchar *spnt;
	long   rtn;
	long   slen;
	long   rlen;
	int    cnt;
	int    retry;
	int    delay;
	int    rercv;

	// Construct the request message

	req.xid = sl(sl(req.xid) + 1);
	dpnt = req.options;

	*dpnt++ = OPT_DHCPMSGTYPE;
	*dpnt++ = 1;
	*dpnt++ = reqtype;

	*dpnt++ = OPT_CLIENTID;
	*dpnt++ = 7;
	*dpnt++ = 1;

	spnt = (uchar *)(openparms.hwaddr.value);
	cnt = 6;
	do
	{
		*dpnt++ = *spnt++;
	} while (--cnt > 0);

	if (ipaddr != 0)
	{
		*dpnt++ = OPT_REQIPADDR;
		*dpnt++ = 4;
		*(long *)dpnt = ipaddr;
		dpnt += 4;
	}

	memcpy(dpnt, reqlist, sizeof(reqlist));
	dpnt += sizeof(reqlist);

	if (dhcpsrvip != 0)
	{
		*dpnt++ = OPT_SERVERID;
		*dpnt++ = 4;
		*(long *)dpnt = dhcpsrvip;
		dpnt += 4;
	}
	*dpnt++ = OPT_END;

	slen = ((dpnt - (uchar *)&req) + 3) &~0x03;
	retry = 3;
	delay = 4;
	do
	{
		printf("\nSending request %d\n", reqtype);
		if ((rtn = svcIoOutBlockP(udphndl, (char *)&req, slen, &sendparms)) < 0)
			femsg2(prgname, "Error sending DHCP request", rtn, NULL);

		rercv = 3;
		do
		{
			longmul(recvparms.timeout.value, ST_SECOND, delay);
			if ((rlen = svcIoInBlockP(udphndl, (char *)&rsp, sizeof(rsp),
					&recvparms)) < 0)
			{
				printf("Receive error %d\n", rlen);

				if (rlen != ER_NORSP)
					return (rlen);
				delay *= 2;

				printf("Timeout set to %d\n", delay);

				break;
			}
			if (rlen < (sizeof(DHCPFXD) + 4))
			{
				printf("? Response is too short (%d)\n", rlen);
				continue;
			}
			if (rsp.magic != 0x63538263)
			{
				printf("? Magic cookie value is wrong: %0x8X\n", rsp.magic);
				continue;
			}
			if ((rtn = parsemsg(rlen)) < 0)
				continue;
			if ((vals.dhcpmsgtype == DHCPOFFER || vals.dhcpmsgtype == DHCPACK ||
						vals.dhcpmsgtype == DHCPNAK) &&
					(dhcpsrvip == 0 || dhcpsrvip == vals.dhcpsrvip))
				return (rlen);
		} while (--rercv >= 0);
	} while (--retry >= 0);
	return (ER_NORSP);
}

long parsemsg(
	int len)
{
	uchar *dpnt;
	long   size;
	long   cnt;
	long   opt;

	printf("     Response Length: %d bytes\n", len);
	printf("Client Hardware Addr: (%d) %02X-%02X-%02X-%02X-%02X-%02X\n",
			rsp.htype, rsp.chaddr[0], rsp.chaddr[1], rsp.chaddr[2],
			rsp.chaddr[3], rsp.chaddr[4], rsp.chaddr[5]);
	printf("    Assigned IP addr: %d.%d.%d.%d\n", ((uchar *)&rsp.yiaddr)[0],
			((uchar *)&rsp.yiaddr)[1], ((uchar *)&rsp.yiaddr)[2],
			((uchar *)&rsp.yiaddr)[3]);

	memset(&vals, 0, sizeof(vals));
	dpnt = rsp.options;
	cnt = len - sizeof(DHCPFXD);
	while (--cnt > 0)
	{
		if ((opt = *dpnt++) == 0)
			printf("PAD\n");
		else if (opt == 255)
		{
			printf("                 END\n");
			break;
		}
		else
		{
			if (--cnt < 0)
			{
				printf("? Bad option: Extends beyond end of datagram\n");
				return (ER_DATER);
			}
			len = *dpnt++;
			if ((cnt -= len) < 0)
			{
				printf("? Bad option: Extends beyond end of datagram\n");
				return (ER_DATER);
			}


			if (opt > MAXOPT)
				hvunknown(dpnt - 2);
			else
			{
				size = opttbl[opt - 1].size;
				if (size & 0x80)
				{
					if (len == 0 || (len % (size & 0x7F)) != 0)
					{
						printf("? Invalid option length %d for option %d\n",
								len, opt);
						return (ER_DATER);
					}
				}
				else
				{
					if (len < size)
					{
						printf("? Invalid option length %d for option %d\n",
								len, opt);
						return (ER_DATER);
					}
				}
				(opttbl[opt - 1].func)(dpnt - 2);
			}
			dpnt += len;
		}
	}
	return (0);
}



void hvsubnetmask(
	uchar *data)
{
	vals.subnetmask = *(long *)(data + 2);
	printf("         Subnet mask: %d.%d.%d.%d\n", data[2], data[3], data[4],
			data[5]);
}


void hvtimeoffset(
	uchar *data)
{
	vals.timeoffset = sl(*(long *)(data + 2));
	printf("         Time Offset: %d\n", vals.timeoffset);
}


void hvrouter(
	uchar *data)
{
	vals.routerip = *(long *)(data + 2);
	printf("           Router IP: %d.%d.%d.%d\n", data[2], data[3], data[4],
			data[5]);
}


void hvtimesrv(
	uchar *data)
{
	vals.timersrvip = *(long *)(data + 2);
	printf("     Timer Server IP: %d.%d.%d.%d\n", data[2], data[3], data[4],
			data[5]);
}


void hvnamesrv(
	uchar *data)
{
	vals.namesrvip = *(long *)(data + 2);
	printf("      Name Server IP: %d.%d.%d.%d\n", data[2], data[3], data[4],
			data[5]);
}


void hvdnssrv(
	uchar *data)
{
	vals.dnssrvip = *(long *)(data + 2);
	printf("       DNS Server IP: %d.%d.%d.%d\n", data[2], data[3], data[4],
			data[5]);
}


void hvlogsrv(
	uchar *data)
{
	vals.logsrvip = *(long *)(data + 2);
	printf("       Log Server IP: %d.%d.%d.%d\n", data[2], data[3], data[4],
			data[5]);
}


void hvcookiesrv(
	uchar *data)
{
	vals.cookiesrvip = *(long *)(data + 2);
	printf("    Cookie Server IP: %d.%d.%d.%d\n", data[2], data[3], data[4],
			data[5]);
}


void hvlprsrv(
	uchar *data)
{
	vals.lprsrvip = *(long *)(data + 2);
	printf("       LPR Server IP: %d.%d.%d.%d\n", data[2], data[3], data[4],
			data[5]);
}


void hvdomainname(
	uchar *data)
{
	printf("         Domain Name: %.*s\n", data[1], data + 2);
}


void hvleasetime(
	uchar *data)
{
	vals.leasetime = sl(*(long *)(data + 2));
	printf("          Lease Time: %d\n", vals.leasetime);
}


void hvoverload(
	uchar *data)
{
	printf("  Overload indicated: %d\n", data[2]);
}


void hvdhcpmsgtp(
	uchar *data)
{
	static char *msgtbl[] =
	{	"",					// 0
		" DHCPDISCOVER",	// 1
		" DHCPOFFER",		// 2
		" DHCPREQUEST",		// 3
		" DHCPDECLINE",		// 4
		" DHCPACK",			// 5
		" DHCPNAK",			// 6
		" DHCPRELEASE",		// 7
		" DHCPINFORM"		// 8
	};

	int val;

	vals.dhcpmsgtype = data[2];
	if ((val = vals.dhcpmsgtype) > 8)
		val = 0;
	printf("       DHCP Msg Type: %d%s\n", vals.dhcpmsgtype, msgtbl[val]);
}


void hvdhcpsrv(
	uchar *data)
{
	vals.dhcpsrvip = *(long *)(data + 2);
	printf("      DHCP Server IP: %d.%d.%d.%d\n", data[2], data[3], data[4],
			data[5]);
}


void hvmessage(
	uchar *data)
{
	printf("             Message: %.*s\n", data[1], data + 2);
}


void hvrenewtime(
	uchar *data)
{
	vals.renewtime = sl(*(long *)(data + 2));
	printf("   Renewal (T1) Time: %d\n", vals.renewtime);
}


void hvrebindtime(
	uchar *data)
{
	vals.rebindtime = sl(*(long *)(data + 2));
	printf(" Rebinding (T2) Time: %d\n", vals.rebindtime);
}


void hvclientid(
	uchar *data)
{
	data = data;
}


void hvunknown(
	uchar *data)
{
	char  *tpnt;
	uchar *dpnt;
	int    cnt;
	char   text[128];

	tpnt = text + sprintf(text, "      Unknown Option: %d (%d)", data[0],
			data[1]);
	dpnt = data + 2;
	cnt = data[1];
	while (--cnt >= 0)
		tpnt += sprintf(tpnt, " %02X", *dpnt++);
	printf("%s\n", text);
}
