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
#include "winnetwork.h"


#define SENDFLG_ENCRYPT 0x01
#define SENDFLG_DATA    0x02

int xxx = 0;

static int clientreg = false;
static int msginpavl;
static int msgoutdon;

struct _ring
{	uchar  type;
	uchar  msgval;
	WPARAM value;
};

_ring  ring[1024];
_ring *rngpnt = ring;

uint   winnetwork::basetid = 0;

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


// Messages used here:
//	WM_USER + 100:	Sent to main thread to request action by that thread:
//					lparam:	1:	Call receive function
//							wparam:	Length of input message
//							2:  Call output complete function
//							wparam:	Address of output block
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

int aaa = 0;

/* static */ void winnetwork::readyinpavlthd(
	MSG  *msg,
	void *pntr)

{
	((winnetwork *)pntr)->readyinpavl(msg);
}


void winnetwork::readyinpavl(
	MSG *msg)

{
	void (*functmp)(winnetwork *wnw, uchar *bufr, int len, void *arg);

	putinring(3, msg->message, msg->wParam);
	functmp = rcvfunc;
	rcvfunc = ignorereceive;
	if (closed || ((long)(msg->wParam)) < 0)
	{
		if (((long)(msg->wParam)) >= 0)
			*((long *)&(msg->wParam)) = ER_BDDVH;
		errorfunc(this, msg->wParam, errorarg);
	}
	else
		functmp(this, rcvbfr, msg->wParam, rcvarg);
}

//********************************************************
// Function: readyoutdon - Called from main thread message
//				loop when output is complete
// Returned: Nothing
//********************************************************

/* static */ void winnetwork::readyoutdonthd(
	MSG  *msg,
	void *pntr)

{
	((winnetwork *)pntr)->readyoutdon(msg);
}


void winnetwork::readyoutdon(
	MSG *msg)

{
///	void (*functmp)(uchar *bufr, int len, void *arg);

	putinring(4, msg->message, msg->wParam);
	(((OBLK *)(msg->wParam))->donefunc)(this, ((OBLK *)(msg->wParam))->result,
			((OBLK *)(msg->wParam))->arg);
	givememory((void *)(msg->wParam));
}

//************************************************************
// Function: winnetwork - Constructor for the winnetwork class
// Returned: Nothing
//************************************************************

// The arguments passed to this function must not be modified until
//   "donfunc" is called.

winnetwork::winnetwork(
	uchar *rcvbfrarg,
	int    rcvlenarg,
	void (*errorfuncarg)(winnetwork *wnw, long code, void *errorarg),
	void  *errorargarg)

{
	rcvbfr = rcvbfrarg;
	rcvlen = rcvlenarg - 6;
	errorfunc = errorfuncarg;
	errorarg = errorargarg;
	rcvfunc = ignorereceive;
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
		msginpavl = swcRespRegFunc(winnetwork::readyinpavlthd, this);
		msgoutdon = swcRespRegFunc(winnetwork::readyoutdonthd, this);
		clientreg = true;
	}
	starthndl = CreateEvent(NULL, TRUE, FALSE, NULL);
	if ((tcpoutthd = (HANDLE)_beginthreadex(NULL, 0, tcpoutthread, this, 0,
			&tcpouttid)) == NULL)
	{
///		printf("Thread create failed!\n");
	}
	while (!startdone)
		WaitForSingleObject(starthndl, 10000);
	CloseHandle(starthndl);
}


winnetwork::~winnetwork()

{


	basetid = 0;
}


void winnetwork::openconnection(
	char  *netaddr,
	long   port,
	void (*donefuncarg)(winnetwork *wnw, long result, void *arg),
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


void winnetwork::closeconnection(
	void      (*donefuncarg)(winnetwork *wnw, long result, void *arg),
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
// Function: acceptreceive - Accept previous input message and
//				specify function to process next message
// Returned: Nothing
//************************************************************

void winnetwork::acceptreceive(
	void (*rcvfuncarg)(winnetwork *wnw, uchar *bufr, int len, void *arg),
	void  *rcvargarg)

{
	rcvfunc = (rcvfuncarg != NULL) ? rcvfuncarg : ignorereceive;
	rcvarg = rcvargarg;
	putinring(0, 105, 0);
	if (!PostThreadMessage(tcpintid, WM_USER + 105, 0, 0))
		xxx = 3;
}

//****************************************************************
// Function: setreceive - Specify function to process next message
// Returned: Nothing
//****************************************************************

void winnetwork::setreceive(
	void (*rcvfuncarg)(winnetwork *wnw, uchar *bufr, int len, void *arg),
	void  *rcvargarg)

{
	rcvfunc = (rcvfuncarg != NULL) ? rcvfuncarg : ignorereceive;
	rcvarg = rcvargarg;
}

//************************************
// Functions: sendmsg - Send a message
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

void winnetwork::sendmsg(
	uchar *msg,
	int    len,
	int    encrypt,
	void (*donefunc)(winnetwork *wnw, long result, void *arg),
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
	memcpy(obpnt->data.c, msg, len + 6);
	putinring(0, 103, (WPARAM)donefunc);
	if (!PostThreadMessage(tcpouttid, WM_USER + 103, 0, (LPARAM)obpnt))
		xxx = 4;
}


//********************************
// Functions: senddata - Send data
// Returned: Nothing
//********************************

// This function returns immediately. Output is queued and the must not
//   modify the buffer. The buffer must have been allocated as a char
//   array with new. It will be given up after output is complete.

void winnetwork::senddata(
	uchar *msg,
	int    len,
	int    encrypt,
	void (*donefunc)(winnetwork *wnw, long result, void *arg),
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

uint __stdcall winnetwork::tcpinthread(
	void *arg)

{
	MSG  msg;
	long size;
	long len;
	long rtn;
	int  crypt;

	while (TRUE)
	{
		if (((winnetwork *)arg)->closed)
		{
///			__asm int 3;

			return (1);
		}

		// Input a message

		if ((rtn = getdata(((winnetwork *)arg)->soc,
				((winnetwork *)arg)->rcvbfr, 6)) < 0)
		{
			if (rtn == -1)
				rtn = xlatewinsocerror(WSAGetLastError());
			if (!(((winnetwork *)arg)->closed))
			{
				putinring(0, 100, rtn);
				if (!PostThreadMessage(basetid, WM_USER + 100, rtn, (long)arg))
					xxx = 5;
			}
			((winnetwork *)arg)->inputfin = true;

///			__asm int 3;

			return (1);					// Terminate the input thread
		}
		size = sl(*(long *)(((winnetwork *)arg)->rcvbfr));
		if (size & 0x80000000)			// Is the message encrypted?
		{
			len = ((size & 0x7FFFFFFF) << 3) - 2;
			crypt = true;
		}
		else
		{
			len = size;
			crypt = false;
		}
		if (len > ((winnetwork *)arg)->rcvlen)
		{
			if (!(((winnetwork *)arg)->closed))
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
			if ((rtn = getdata(((winnetwork *)arg)->soc,
					((winnetwork *)arg)->rcvbfr + 6, len)) < 0)
			{
				if (rtn == -1)
					rtn = xlatewinsocerror(WSAGetLastError());
				if (!(((winnetwork *)arg)->closed))
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
			qdesDecBlock(((winnetwork *)arg)->rcvbfr + 4, len + 2,  
					&(((winnetwork *)arg)->instate)); // Decrypt the message
			size = len - 8 + (((((winnetwork *)arg)->rcvbfr)[len + 5]) & 0x07);
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

void winnetwork::ignorereceive(
	winnetwork *wnw,
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

//************************************************************************
// Function: tcpoutthread - Main thread function for the TCP output thread
// Returned: Thread exit status
//************************************************************************

// This thread does all TCP output. It also handles establishing the TCP
//   connection for a new session with the central system.

uint __stdcall winnetwork::tcpoutthread(
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
	((winnetwork *)arg)->startdone = TRUE;
	SetEvent(((winnetwork *)arg)->starthndl);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		putinring(2, msg.message, msg.wParam);
		if (((winnetwork *)arg)->closed) // Is connection closed now?
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
///			if (((winnetwork *)arg)->closed) // Is connection closed now?
///			{							// Yes
///				if (((winnetwork *)arg)->inputfin)
///					PostThreadMessage(basetid, WM_USER + 100, (long)ER_NCLST,
///							(long)arg);
///				return (1);				// Terminate the output thread
///			}
			break;
		}
	}
	return (0);
}

long winnetwork::doconnect(
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

long winnetwork::dodisconnect(
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

long winnetwork::dosendmsg(
	OBLK *oblk)

{
	winnetwork *wnw;
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
		len = ttl + 4;					// Get total length
	}
	else
	{
		*(long *)pnt = sl(len);
		len += 6;
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

int winnetwork::getdata(
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

OBLK *winnetwork::getoblk(
	int size)

{
	return ((OBLK *)getmemory(offsetof(OBLK, data) + size));
}
