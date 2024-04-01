// This class implements a general purpose interface to a SignalFire radio
//   network using an RS-232 serial interface connected to a SignalFire
//   gateway node.

#if !defined __SFGINTERFACEINC
#define __SFGINTERFACEINC 1

#define MAXROUTE  10			// Maximum number of nodes in a route
#define MAXPKTLEN 200			// Maximum total packet length

// Define values for the func argument for the notify function

#define SFGI_NOTIFY_PKTIN      0
#define SFGI_NOTIFY_BADLENGTH  1
#define SFGI_NOTIFY_BADDATALEN 2
#define SFGI_NOTIFY_BADROUTE   3
#define SFGI_NOTIFY_OUTERROR   4
#define SFGI_NOTIFY_COMNOTAVL  5
#define SFGI_NOTIFY_COMBADRATE 6
#define SFGI_NOTIFY_COMERROR   7

// Define function values for the notify (callback) function

#define SFGI_INPKT   0
#define SFGI_INERR   1
#define SFGI_BADDATA 2

class SFGInterface

{public:
	struct status
	{	long xxxx;
	};

	struct pkt
	{	uchar  type;
		uchar  ttl;
		uchar  routes;
		uchar  xxx;
		ushort flags;
		ushort yyy;
		long   addr;
		long   msgid;
        long   dlen;
		uchar  data[1];
	};

	struct routeinfo
	{	long  addr;
		char  rssi;
		uchar xxx[3];
	};
	struct routedata
	{   int       num;
    	routeinfo route[MAXROUTE];
	};

// Define values for state

#define RT_STA_NEW     0
#define RT_STA_INVALID 1
#define RT_STA_OK      3

	struct rtblk
	{	rtblk    *next;
		long      addr;
		uchar     num;
		ushort    seqnum;
		uchar     state;
		uchar     xxx;
		routeinfo route[MAXROUTE];
	};

	class serinthd : public SFThread
	{public:
		serinthd(unsigned int (__stdcall *address)(void *), void  *argsarg,
				long stksize);
		unsigned int threadfnc();

		SFGInterface *iface;
	};

	class seroutthd : public SFThread
	{public:
		seroutthd(unsigned int (__stdcall *address)(void *), void  *argsarg,
				long stksize);
		unsigned int threadfnc();

		SFGInterface *iface;
	};

	SFGInterface(void (*notify)(int func, pkt *bfr, int sz, void *arg),
            void *argarg);
	int  SetComm(int port, int speed);
	int  Start();
	int  Stop();
	int  Send(int type, long addr, int flags, int ttl, uchar *data, int size,
            routedata *rused);
	int  GetRoute(long addr, routedata *route);
    int  GetNextMsgID(long node);
	int  GetStatus(long node, status *sts);
	void storeroute(long address, sfp_route *route, int num);

	serinthd  *inthread;
	seroutthd *outthread;
	rtblk     *routehead;
	int        port;
	int        speed;
	char       started;
    uchar      fakeseqnum;
	void      *arg;
	void      *child;
	void     (*usrnotify)(int func, pkt *bufr, int sz, void *arg);
	void     (*donotify)(int func, pkt *bufr, int sz, void *arg1,
            void *arg2);
};

#endif
