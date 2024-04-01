#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <process.h>
#include <winsock.h>
#include "xosstuff.h"
#include "xosqdes.h"
#include "xoserr.h"
#include "random.h"
#include "swcwindows.h"
#include "swcclient16.h"

#include "crtdbg.h"

#define SENDFLG_ENCRYPT 0x01
#define SENDFLG_DATA    0x02

static int xxx = 0;

static int clientreg = false;
static int msginpavl;
static int msgoutdon;

struct _ring
{	uchar  type;
	uchar  msgval;
	WPARAM value;
};

static _ring  ring[1024];
static _ring *rngpnt = ring;

uint   swcClient16::basetid = 0;

long xlatewinsocerror(int wscode);


static void putinring(
	int    type,
	int    msgval,
	WPARAM value)

{
	rngpnt->type = (uchar)type;
	rngpnt->msgval = (uchar)msgval;
	rngpnt->value = value;
	if (++rngpnt >= (ring + 1024))
		rngpnt = ring;
}


// Messages sent to the network thread: (messages sent to the main thread
//   use the swcRespxxxx functions)
//  WM_USER + 102:	Sent to TCP output thread to request a new connection
//					lparam:	Address of output block
//  WM_USER + 103:	Sent to TCP output thread to request output
//					lparam: Address of output block
//  WM_USER + 104:	Sent to TCP output thread to request disconnect
//  WM_USER + 105:	Sent to TCP input thread to indicate that previous input
//					message has been processed

//***********************************************************
// Function: readyinpavlghc - Called from main thread message
//				loop when input is available
// Returned: Nothing
//***********************************************************

/* static */ void swcClient16::readyinpavlthd(
	MSG  *msg,
	void *pntr)

{
	((swcClient16 *)pntr)->readyinpavl(msg);
}


void swcClient16::readyinpavl(
	MSG *msg)

{
	putinring(3, msg->message, msg->wParam);
	if (closed)
		*((long *)&(msg->wParam)) = ER_BDDVH;
	rcvfunc(this, rcvbfr, msg->wParam, rcvarg);
}


//********************************************************
// Function: readyoutdon - Called from main thread message
//				loop when output is complete
// Returned: Nothing
//********************************************************

/* static */ void swcClient16::readyoutdonthd(
	MSG  *msg,
	void *pntr)

{
	((swcClient16 *)pntr)->readyoutdon(msg);
}


void swcClient16::readyoutdon(
	MSG *msg)

{
///	void (*functmp)(uchar *bufr, int len, void *arg);

	putinring(4, msg->message, msg->wParam);
	(((OBLK *)(msg->wParam))->donefunc)(this, ((OBLK *)(msg->wParam))->result,
			((OBLK *)(msg->wParam))->arg);
	givememory((void *)(msg->wParam));
}

//**************************************************************
// Function: swcClient16 - Constructor for the swcClient16 class
// Returned: Nothing
//**************************************************************

// The arguments passed to this function must not be modified until
//   "donfunc" is called.

swcClient16::swcClient16(
	uchar *rcvbfrarg,
	int    rcvlenarg,
	void (*errorfuncarg)(swcClient16 *wnw, long code, void *errorarg),
	void  *errorargarg)

{
	rcvbfr = rcvbfrarg;
	rcvlen = rcvlenarg - 6;
	errorfunc = errorfuncarg;
	errorarg = errorargarg;
	rcvfunc = NULL;
	if (basetid == 0)
		basetid = GetCurrentThreadId();
	errcode = 0;
	startdone = false;
	connected = false;
	loggedin = false;
	closed = false;
	inputfin = false;
	nooptmr = noopcnt = 0;
	if (!clientreg)
	{
		msginpavl = swcRespRegFunc(swcClient16::readyinpavlthd, this);
		msgoutdon = swcRespRegFunc(swcClient16::readyoutdonthd, this);
		clientreg = true;
	}
	starthndl = CreateEvent(NULL, TRUE, FALSE, NULL);
	if ((tcpoutthd = (HANDLE)_beginthreadex(NULL, 0, tcpoutthread, this, 0,
			&tcpouttid)) == NULL)
	{
		printf("Thread create failed!\n");
	}
	while (!startdone)
		WaitForSingleObject(starthndl, 10000);
	CloseHandle(starthndl);
}


swcClient16::~swcClient16()

{
	basetid = 0;
}


void swcClient16::OpenConnection(
	char  *netaddr,
	long   port,
	void (*donefuncarg)(swcClient16 *wnw, long result, void *arg),
	void  *argarg)

{
	OBLK *obpnt;

	obpnt = getoblk(0);
	obpnt->wnw = this;
	obpnt->donefunc = donefuncarg;
	obpnt->arg = argarg;
	obpnt->result = (long)netaddr;
	obpnt->len = port;
	putinring(0, 102, 0);
	closed = false;
	if (!PostThreadMessage(tcpouttid, WM_USER + 102, 0, (LPARAM)obpnt))
		xxx = 1;
}


void swcClient16::CloseConnection(
	void      (*donefuncarg)(swcClient16 *wnw, long result, void *arg),
	void      *argarg)

{
	OBLK *obpnt;

	connected = FALSE;
	noopcnt = nooptmr = 0;
	obpnt = getoblk(0);
	obpnt->wnw = this;
	obpnt->donefunc = donefuncarg;
	obpnt->arg = argarg;
	putinring(0, 104, 0);
	if (!PostThreadMessage(tcpouttid, WM_USER + 104, 0, (LPARAM)obpnt))
		xxx = 2;
}


//************************************************************
// Function: AcceptRreceive - Accept previous input message and
//				specify function to process next message
// Returned: Nothing
//************************************************************

void swcClient16::AcceptReceive(
	void (*rcvfuncarg)(swcClient16 *wnw, uchar *bufr, int len, void *arg),
	void  *rcvargarg)

{
	if (rcvfuncarg != NULL)
		rcvfunc = rcvfuncarg;
	rcvarg = rcvargarg;
	putinring(0, 105, 0);
	if (!PostThreadMessage(tcpintid, WM_USER + 105, 0, 0))
		xxx = 3;
}

//****************************************************************
// Function: setreceive - Specify function to process next message
// Returned: Nothing
//****************************************************************

void swcClient16::SetReceive(
	void (*rcvfuncarg)(swcClient16 *wnw, uchar *bufr, int len, void *arg),
	void  *rcvargarg)

{
	rcvfunc = rcvfuncarg;
	rcvarg = rcvargarg;
}

//************************************
// Functions: SendMsg - Send a message
// Returned: Nothing
//************************************

// This function returns immediately. Output is queued and the caller is
//   free to modify the message buffer as soon as this function returns.
//   If "donefunc" is not null, this function will be called when output
//   is complete.

// This function allocates a buffer and copies the message to be output.
//   While this is not very efficient, it does make using this function
//   much easier, since the caller can immediately reuse the buffer, even
//   though output is not complete. Given that this function will not
//   generally be used for a large amount of data, this seems like a good
//   trade-off.

void swcClient16::SendMsg(
	uchar *msg,
	int    len,
	int    encrypt,
	void (*donefunc)(swcClient16 *wnw, long result, void *arg),
	void  *argarg)

{
	OBLK *obpnt;

	noopcnt = KACNT;
	if ((obpnt = getoblk((len + 21) & ~0x07)) == NULL)
	{
		donefunc(this, ER_NEMA, argarg);
		return;
	}
	obpnt->wnw = this;

	if (donefunc == NULL)
		putinring(0xFF, 0xFE, (WPARAM)donefunc);

	obpnt->donefunc = donefunc;
	obpnt->arg = argarg;
	obpnt->result = (encrypt) ? SENDFLG_ENCRYPT : 0;
	obpnt->len = len;
	memcpy(obpnt->data.c, msg, len + 4);
	putinring(0, 103, (WPARAM)donefunc);
	if (!PostThreadMessage(tcpouttid, WM_USER + 103, 0, (LPARAM)obpnt))
		xxx = 4;
}


//********************************
// Functions: SendData - Send data
// Returned: Nothing
//********************************

// This function returns immediately. Output is queued and the caller must
//   not modify the buffer. The buffer must have been allocated as a char
//   array with new. It will be given up after output is complete.

void swcClient16::SendData(
	uchar *msg,
	int    len,
	int    encrypt,
	void (*donefunc)(swcClient16 *wnw, long result, void *arg),
	void  *argarg)

{
	OBLK *obpnt;

	noopcnt = KACNT;
	if ((obpnt = getoblk(sizeof(oblk))) == NULL)
	{
		donefunc(this, ER_NEMA, argarg);
		return;
	}
	obpnt->wnw = this;

///	if (donefunc == NULL)
///		putinring(0xFF, 0xFE, (WPARAM)donefunc);

	obpnt->donefunc = donefunc;
	obpnt->arg = argarg;
	obpnt->result = encrypt;
	obpnt->result = (encrypt) ? SENDFLG_ENCRYPT|SENDFLG_DATA : SENDFLG_DATA;
	obpnt->len = len;
	obpnt->data.p = msg;
	putinring(0, 103, (WPARAM)donefunc);
	if (!PostThreadMessage(tcpouttid, WM_USER + 103, 0, (LPARAM)obpnt))
		xxx = 4;
}



// Everything from here on is private!

//**********************************************************************
// Function: tcpinthread - Main thread function for the TCP input thread
// Returned: Thread exit status
//**********************************************************************

uint __stdcall swcClient16::tcpinthread(
	void *arg)

{
	MSG  msg;
	long size;
	long len;
	long rtn;
	int  crypt;

	while (TRUE)
	{
		if (((swcClient16 *)arg)->closed)
		{
///			__asm int 3;

			return (1);
		}

		// Input a message

		if ((rtn = getdata(((swcClient16 *)arg)->soc,
				((swcClient16 *)arg)->rcvbfr, 4)) < 0)
		{
			if (rtn == -1)
				rtn = xlatewinsocerror(WSAGetLastError());
			if (!(((swcClient16 *)arg)->closed))
			{
				putinring(0, 100, rtn);
				if (!swcRespSend(msginpavl, rtn, (long)arg))
					xxx = 5;
			}
			((swcClient16 *)arg)->inputfin = true;

			Sleep(10000);

///			__asm int 3;

			return (1);					// Terminate the input thread
		}


		size = sw(*(short *)(((swcClient16 *)arg)->rcvbfr));
		if (size & 0x8000)				// Is the message encrypted?
		{
			len = ((size & 0x7FFF) << 3) - 2;
			crypt = true;
		}
		else
		{
			len = size;
			crypt = false;
		}
		if (len > ((swcClient16 *)arg)->rcvlen)
		{
			if (!(((swcClient16 *)arg)->closed))
			{
				putinring(0, 100, (uint)ER_NDRTL);
				if (!swcRespSend(msginpavl, ER_NDRTL, (long)arg))
					xxx = 6;
			}

///			__asm int 3;

			return (1);
		}
		if (len > 0)
		{
			if ((rtn = getdata(((swcClient16 *)arg)->soc,
					((swcClient16 *)arg)->rcvbfr + 4, len)) < 0)
			{
				if (rtn == -1)
					rtn = xlatewinsocerror(WSAGetLastError());
				if (!(((swcClient16 *)arg)->closed))
				{
					putinring(0, 100, rtn);
					if (!PostThreadMessage(basetid, WM_USER + 100, rtn,
							(long)arg))
						xxx = 7;
				}

///				__asm int 3;

				return (1);
			}
		}
		if (crypt)
		{
			qdesDecBlock(((swcClient16 *)arg)->rcvbfr + 4, len + 2,  
					&(((swcClient16 *)arg)->instate)); // Decrypt the message
			size = len - 8 + (((((swcClient16 *)arg)->rcvbfr)[len + 5]) & 0x07);
		}
		putinring(0, 100, size);
		if (!swcRespSend(msginpavl, size, (long)arg))
			xxx = 8;

		// Wait for the acknowledgement

		while (GetMessage(&msg, NULL, 0, 0))
		{
			putinring(1, msg.message, msg.wParam);
			if (msg.message == (WM_USER + 105))
				break;
		}
	}
}

//**********************************************************************
// Function: ignorereceive - Receive function called when ignoring input
// Returned: Nothing
//**********************************************************************
/*
void swcClient16::ignorereceive(
	swcClient16 *wnw,
	uchar      *bufr,
	int         len,
	void       *arg)

{
	NOTUSED(bufr);
	NOTUSED(len);
	NOTUSED(arg);

	putinring(0, 105, 0);
	if (!PostThreadMessage(wnw->tcpintid, WM_USER + 105, 0, 0))
		xxx = 9;
}
*/

//************************************************************************
// Function: tcpoutthread - Main thread function for the TCP output thread
// Returned: Thread exit status
//************************************************************************

// This thread does all TCP output. It also handles establishing the TCP
//   connection for a new session with the central system.

uint __stdcall swcClient16::tcpoutthread(
	void *arg)

{
	MSG     msg;
	WSAData wsadata;

	if (!wsstarted)
	{
		WSAStartup(0x0101, &wsadata);
		wsstarted = true;
	}
	PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	((swcClient16 *)arg)->startdone = TRUE;
	SetEvent(((swcClient16 *)arg)->starthndl);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		putinring(2, msg.message, msg.wParam);
		if (((swcClient16 *)arg)->closed) // Is connection closed now?
		{
			givememory((OBLK *)(msg.lParam));
			continue;
		}
		switch (msg.message)
		{
		 case WM_USER + 102:			// Connect to HOST
			((OBLK *)(msg.lParam))->result = ((OBLK *)(msg.lParam))->wnw->
					doconnect((OBLK *)(msg.lParam));
			goto outdone;

		 case WM_USER + 103:			// Send message
			((OBLK *)(msg.lParam))->result = dosendmsg((OBLK *)(msg.lParam));
			goto outdone;

		 case WM_USER + 104:			// Disconnect
			((OBLK *)(msg.lParam))->result = ((OBLK *)(msg.lParam))->wnw->
					dodisconnect((OBLK *)(msg.lParam));
		 outdone:
			if (((OBLK *)(msg.lParam))->donefunc != NULL)
			{
				putinring(0, 101, (WPARAM)(msg.lParam));
				if (!swcRespSend(msgoutdon, msg.lParam, (long)arg))
					xxx = 10;
			}
			else
			{
				putinring(0xFF, 0xFF, (WPARAM)(msg.lParam));
				givememory((OBLK *)(msg.lParam));
			}
///			if (((swcClient16 *)arg)->closed) // Is connection closed now?
///			{							// Yes
///				if (((swcClient16 *)arg)->inputfin)
///					PostThreadMessage(basetid, WM_USER + 100, (long)ER_NCLST,
///							(long)arg);
///				return (1);				// Terminate the output thread
///			}
			break;
		}
	}
	return (0);
}

long swcClient16::doconnect(
	OBLK *oblk)

{
	SOCKADDR_IN socaddr;
	HOSTENT    *host;
	long        hostipa;
	long        code;

	closed = false;
	if ((hostipa = inet_addr((char *)(oblk->result))) == INADDR_NONE)
	{
		if ((host = gethostbyname((char *)(oblk->result))) == NULL)
			return (ER_NILAD);
		hostipa = *(ulong *)(host->h_addr);
	}
	if ((soc = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return (ER_NNPA);
	socaddr.sin_family = AF_INET;
	socaddr.sin_port = 0;
	socaddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(soc, (SOCKADDR *)&socaddr, sizeof(socaddr)) == SOCKET_ERROR)
	{
		closesocket(soc);
		soc = 0;
		return (ER_NNPA);
	}
	socaddr.sin_port = htons((short)(oblk->len));
	socaddr.sin_addr.s_addr = hostipa;
	if (connect(soc, (SOCKADDR *)&socaddr, sizeof(socaddr)) == SOCKET_ERROR)
	{
		code = xlatewinsocerror(WSAGetLastError());
		closesocket(soc);
		soc = 0;
		return (code);
	}
	if ((tcpinthd = (HANDLE)_beginthreadex(NULL, 0, tcpinthread, this, 0,
			&(tcpintid))) == NULL)
	{
		closesocket(soc);
		soc = 0;
		return (ER_TMTHD);
	}
	connected = true;
	noopcnt = KACNT;
	return (0);
}

long swcClient16::dodisconnect(
	OBLK *oblk)

{
	oblk = oblk;

	connected = false;
	closed = true;
	if (soc > 0)
	{
		closesocket(soc);
		soc = 0;
	}
	return (0);
}

//*************************************************************
// Functions: dosendmsg - Send a message
// Returned: 0 if normal, negative combined error code if error
//*************************************************************

long swcClient16::dosendmsg(
	OBLK *oblk)

{
	swcClient16 *wnw;
	char       *pnt;
	long        rtn;
	int         rem;
	int         ttl;
	int         len;

	wnw = oblk->wnw;
	len = oblk->len;
///	oblk->data.c[1] = (uchar)0x81;

	pnt = (char *)((oblk->result & SENDFLG_DATA) ? oblk->data.p : oblk->data.c);
	if (oblk->result & SENDFLG_ENCRYPT)	// Should we encypt this message?
	{									// Yes
										// First determine how many bytes are
		rem = (len + 2) % 8;			//   in the last encryption block and
		ttl = (len + 10) & ~0x07;		//   where it starts
		getrandombytes(oblk->data.c - 4 + ttl + rem, 8 - rem);
										// Fill the part of the last block we
										//   don't need with random bytes
		oblk->data.c[ttl + 3] &= 0xF8;	// Put in the exact length
		oblk->data.c[ttl + 3] |= (uchar)rem;
		qdesEncBlock(oblk->data.c + 4, ttl, &(wnw->outstate));
										// Encrypt the message
		*(long *)pnt = sl(0x80000000 | (ttl >> 3)); // Store length
		len = ttl + 2;					// Get total length
	}
	else
	{
		*(ushort *)pnt = sw(len);
		len += 4;
	}
	while (len > 0)
	{
		if ((rtn = send(wnw->soc, pnt, len, 0)) == SOCKET_ERROR)
			return (xlatewinsocerror(WSAGetLastError()));
		len -= rtn;
		pnt += len;
	}
	if (oblk->result & SENDFLG_DATA)
	{
		givememory(oblk->data.p);
		oblk->data.p = NULL;
	}
	return (0);
}

//********************************************************
// Function: getdata - Gets specified number of bytes from
//				the TCP connection
// Returned: Amount input if normal, -1 if error
//********************************************************

int swcClient16::getdata(
	SOCKET soc,
    uchar *bufr,
	int    len)

{
	long rtn;

	while (len > 0)
	{
		if ((rtn = recv(soc, (char *)bufr, len, 0)) == SOCKET_ERROR)
			return (-1);
		if (rtn == 0)
			return (ER_NCCLR);
		len -= rtn;
		bufr += rtn;
	}
	return (len);
}


//*********************************************
// Function: getoblk - Allocate an output block
// Returned: Address of output block allocated
//*********************************************

OBLK *swcClient16::getoblk(
	int size)

{
    _ASSERTE(_CrtCheckMemory());


	return ((OBLK *)getmemory(offsetof(OBLK, data) + size));
}
