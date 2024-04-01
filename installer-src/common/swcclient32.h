#pragma pack(push, 1)

#define KACNT 20

typedef struct oblk OBLK;

extern uchar  wsstarted;

class swcClient32
{public:
	swcClient32(uchar *rcvbfrarg, int rcvlenarg,
			void (*errorfuncarg)(swcClient32 *wnw, long code, void *errorarg),
			void  *errorargarg);
	~swcClient32();
	void acceptreceive(void (*rcvfuncarg)(swcClient32 *wnw, uchar *bufr,
			int len, void *arg), void *arg = NULL);
	void openconnection(char *netaddr, long port,
			void (*donefuncarg)(swcClient32 *wnw, long result, void *arg),
			void *argarg);
	void closeconnection(void (*donefuncarg)(swcClient32 *wnw, long result,
			void *arg), void *arg = NULL);
	void readyinpavl(MSG *msg);
	void readyoutdon(MSG *msg);
	void sendmsg(uchar *msg, int len, int encrypt,
			void (*donefunc)(swcClient32 *wnw, long result, void *arg),
			void *arg = NULL);
	void senddata(uchar *msg, int len, int encrypt,
			void (*donefunc)(swcClient32 *wnw, long result, void *arg),
			void *arg = NULL);
	void setreceive(void (*rcvfuncarg)(swcClient32 *wnw, uchar *bufr, int len,
			void *arg), void *arg = NULL);

	static void initnetwork();
	static void readyinpavlthd(MSG *msg, void *pntr);
	static void readyoutdonthd(MSG *msg, void *pntr);

	QDESSTATE instate;
	QDESSTATE outstate;
	HANDLE    starthndl;
	SOCKET    soc;
	long      errcode;
	int       unlen;
	int       noopcnt;
	int       nooptmr;
	uchar     connected;
	uchar     startdone;
	uchar     loggedin;
	char      username[32];
	uchar     inputfin;
	uchar     closed;
	static uint   basetid;

 protected:
	long  doconnect(OBLK *oblk);
	long  dodisconnect(OBLK *oblk);

	static long  dosendmsg(OBLK *oblk);
	static int   getdata(SOCKET soc, uchar *bufr, int len);
	static OBLK *getoblk(int size);
	static void  ignorereceive(swcClient32 *wnw, uchar *bufr, int len,
			void *arg);
	static uint  __stdcall tcpinthread(void *arg);
	static uint  __stdcall tcpoutthread(void *arg);

	void (*errorfunc)(swcClient32 *wnw, long len, void *arg);
	void  *errorarg;
	void (*rcvfunc)(swcClient32 *wnw, uchar *bufr, int len, void *arg);
	void  *rcvarg;
	uchar *rcvbfr;		// Receive buffer
	int    rcvlen;		// Size of receive buffer
	HANDLE tcpinthd;
	HANDLE tcpoutthd;
	uint   tcpintid;
	uint   tcpouttid;

};





struct oblk
{	swcClient32
	      *wnw;
	long   result;
	long   len;
	void (*donefunc)(swcClient32 *wnw, long result, void *arg);
	void  *arg;
	union
	{	uchar *p;
		uchar c[1];
	}      data;
};


#pragma pack(pop)
