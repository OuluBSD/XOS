
typedef struct
{	uchar  op;
	uchar  htype;
	uchar  hlen;
	uchar  hops;
	long   xid;
	ushort secs;
	ushort flags;
	long   ciaddr;
	long   yiaddr;
	long   siaddr;
	long   giaddr;
	uchar  chaddr[16];
	char   sname[64];
	char   file[128];
	long   magic;
} DHCPFXD;

// Define values for op in DHCPMSG

#define OP_BOOT_REQUEST 1
#define OP_BOOT_REPLY   2

// Define option types

#define OPT_PAD         0
#define OPT_END         255
#define OPT_SUBNETMASK  1
#define OPT_TIMEOFFSET  2
#define OPT_ROUTER      3
#define OPT_TIMESRV     4
#define OPT_NAMESRV     5
#define OPT_DNSSRV      6
#define OPT_LOGSRV      7
#define OPT_COOKIESRV   8
#define OPT_LPRSRV      9
#define OPT_DOMAINNAME  15

#define OPT_REQIPADDR   50
#define OPT_LEASETIME   51
#define OPT_OVERLOAD    52
#define OPT_DHCPMSGTYPE 53
#define OPT_SERVERID    54
#define OPT_REQLIST     55
#define OPT_MESSAGE     56
#define OPT_MAXSIZE     57
#define OPT_RENEWTIME   58
#define OPT_REBINDTIME  59
#define OPT_CLIENTID    61

// Define values for the OPT_DHCPMSGTYPE option

#define DHCPDISCOVER 1
#define DHCPOFFER    2
#define DHCPREQUEST  3
#define DHCPDECLINE  4
#define DHCPACK      5
#define DHCPNAK      6
#define DHCPRELEASE  7
#define DHCPINFORM   8
