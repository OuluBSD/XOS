#include "xosstuff.h"

class swcServer
{public:
	swcServer();
	void listenfnc();
	uint clientthread();

	long Start(long port, long qlimit, long tlimit, void (*errorcb)(long code,
			char *msg));
	long   port;
	long   qlimit;
	long   tlimit;
	void (*errorcb)(long code, char *msg);

	virtual class swcServerClient *clientftry()=0;
	static uint __stdcall listenthread(void *arg);
	static uint __stdcall clientthread(void *arg);

	SOCKET soc;
	long   clientcnt;
	HANDLE listenthd;
	uint   listentid;

};

class swcServerClient
{public:
	long ReceiveData(uchar *buffer, long len);
	long SendData(uchar *buffer, long len);

	virtual uint ClientBgn()=0;
	virtual uint ClientMsg(uchar *pnt, long len) = 0;
	virtual void ClientEnd()=0;

	uint  clientthread();
	uint  readthread();

	static int swcServerClient::ClientReq(long inx, ulong tid, uchar *par1,
			long par2);
	static uint __stdcall readthread(void *arg);
	static void ClientError(long clntinx, long code, char *fmt, ...);

	HANDLE      clntthd;
	uint        clnttid;
	HANDLE      readthd;
	uint        readtid;
	SOCKADDR_IN addr;
	SOCKET      soc;
	swcServer  *srvr;
	swcServerClient *next;
	swcServerClient *prev;
	long        clntinx;
};
