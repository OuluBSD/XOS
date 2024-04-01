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
///#include "msg.h"
#include "swcwindows.h"
#include "swcServer.h"

// The swcServer class encapcelates most of the logic required to implement a
//   newwork server using multiple client threads. To use the swcServer class
//   a derived class must be created from it. A single instance of this class
//   must be created by calling the default constructor. Also a derived class
//   must be created from the swcServerClient class. An instance of this class
//   will be instanciated for each connection to the server. The class derived
//   from swcServer must at least implement the virtual member function
//   clientftry. This function must create, initialize, and return an instance
//   of the class derived from swcServerClient. The class derived from
//   swcServerCLient must at least implement the virtual member function
//   clientfunc. This function is called whenever a new client connects in
//   the context of that client's thread. It must not return until the client
//   connection is to be cleared. This class should also define any per client
//   data needed by the server.

//   The server is started by calling the Start class function specifying the
//   socket to listen on, and the the thread limit. Data is exchanged with the
//   client using the ReceiveData and SendData member functions. The client
//   connection is closed when the clientfunc function returns.


uchar started = false;

swcServer::swcServer()

{
}


long swcServer::Start(
	long   portarg,
	long   qlimitarg,
	long   tlimitarg,
	void (*errorcbarg)(long code, char *msg))

{
///	SOCKADDR_IN socaddr;
	WSAData     wsadata;

	port = portarg;
	qlimit = qlimitarg;
	tlimit = tlimitarg;
	errorcb = errorcbarg;

	if (!started)
	{
		WSAStartup(0x0101, &wsadata);
		started = true;
	}

	if ((listenthd = (HANDLE)_beginthreadex(NULL, 0, listenthread, this, 0,
			&listentid)) == NULL)
		return (-1);
	return (0);
}


long swcServerClient::ReceiveData(
	uchar *buffer,
	long   len)

{
	long rtn;

	if ((rtn = recv(soc, (char *)buffer, len, 0)) != len)
		return ((rtn == SOCKET_ERROR) ? xlatewinsocerror(WSAGetLastError()) :
				(rtn > 0) ? ER_ICMIO : ER_NCCLR);
	return (len);
}


long swcServerClient::SendData(
	uchar *buffer,
	long   len)

{
	long rtn;

	if ((rtn = send(soc, (char *)buffer, len, 0)) == SOCKET_ERROR)
		return (xlatewinsocerror(WSAGetLastError()));
	return (0);
}


//***************************************************************************
// Function: listenthread - Main thread function for the server listen thread
// Returned: Thread exit status
//***************************************************************************

// This thread loops listening for connections. When a connection is received,
//   a client thread is created which then calls the specified client function.

/* static */ uint __stdcall swcServer::listenthread(
	void *arg)

{
	((swcServer *)arg)->listenfnc();
	return (0);
}


void swcServer::listenfnc()

{
	SOCKADDR_IN socaddr;
	SOCKET      csoc;
	swcServerClient
	           *clnt;
	int         len;
	int         errcd;

	if ((soc = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return;
	socaddr.sin_family = AF_INET;
	socaddr.sin_port = htons((ushort)port);
	socaddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(soc, (SOCKADDR *)&socaddr, sizeof(socaddr)) == SOCKET_ERROR)
	{
		errcd = WSAGetLastError();
		closesocket(soc);
		soc = 0;
		return;
	}

	if (listen(soc, SOMAXCONN) == SOCKET_ERROR)
	{
		errcd = WSAGetLastError();
		closesocket(soc);
		soc = 0;
		return;
	}
	while (true)
	{
		socaddr.sin_family = AF_INET;
		len = sizeof(SOCKADDR_IN);
		if ((csoc = accept(soc, (SOCKADDR *)&socaddr, &len)) == INVALID_SOCKET)
		{
			errcd = WSAGetLastError();
		}
		clnt = this->clientftry();
		clnt->srvr = this;
		clnt->soc = csoc;
		clnt->addr = socaddr;
		if ((clnt->clntthd = (HANDLE)_beginthreadex(NULL, 0, clientthread, clnt, 0,
			&clnt->clnttid)) == NULL)
		{

		}
	}
}


//******************************************************
// Function: clientreq - Send request to a client thread
// Returned: true if OK, false if error
//******************************************************

/* static */ int swcServerClient::ClientReq(
	long   inx,
	ulong  tid,
	uchar *par1,
	long   par2)

{
	if (!PostThreadMessage(tid, WM_USER + 100, (WPARAM)par1, (LPARAM)par2))
	{
		if (par2 > 0)
			free(par1);
		ClientError(inx, ER_ERROR, "Error posting request to client thread");
		return (false);;
	}
	return (true);
}


//***************************************************************************
// Function: clientthread - Main thread function for the server client thread
// Returned: Thread exit status
//***************************************************************************

// This thread first call ClientBgn (which must be overriden by the derived
//   class. It then loops waiting for a thread message. When a message is
//   received the ClientMsg function (which must be overridden by the derived
//   class) is called.

/* static */ uint __stdcall swcServer::clientthread(
	void *arg)

{
	return (((swcServerClient *)arg)->clientthread());
}


uint swcServerClient::clientthread()

{
	MSG  msg;
	uint rtn;

	// First create the client read thread

	if ((readthd = (HANDLE)_beginthreadex(NULL, 0, readthread, this, 0,
			&readtid)) == NULL)
	{

	}
	ClientBgn();						// Call the overriden initialization
	rtn = 0;							//   function
	while (GetMessage(&msg, NULL, 0, 0))
		if (msg.message == (WM_USER + 100) &&
				(rtn = ClientMsg((uchar *)msg.wParam, msg.lParam)) != 0)
			break;
	ClientEnd();
	return (rtn);
}


//******************************************************************************
// Function: readthread - Main thread function for the server client read thread
// Returned: Thread exit status
//******************************************************************************

// This function loops with a blocking TCP read. When an input message is
//   received it is passed to the client thread. This is implemented in a
//   separate thread to allow external requests to be handled by the client
//   thread.


/* static */ uint __stdcall swcServerClient::readthread(
	void *arg)

{
	return (((swcServerClient *)arg)->readthread());
}

#define BFRSZ 4000

uint swcServerClient::readthread()

{
	struct
	{	ushort len;
		uchar  type;
		uchar  flags;
	}      header;
	long   rtn;
	long   len;
	uchar *bufr;

	while (true)
	{
		if ((rtn = ReceiveData((uchar *)&header, 4)) < 0)
		{
			ClientError(clntinx, rtn, "Error reading message header from client");
			break;
		}
		len = sw(header.len);
		if (len > BFRSZ)
		{
			ClientError(clntinx, ER_NDRTL, "Client message is too long (%d)", len);
			break;
		}
		if (header.flags != 0)
		{
			ClientError(clntinx, ER_NPERR, "Invalid flag value in message from "
					"client");
			break;
		}
		if ((bufr = (uchar *)malloc(len + 6)) == NULL)
		{
			ClientError(clntinx, ER_NEMA, "Cannot allocate memory for client "
					"message");
			break;
		}
		bufr[0] = header.type;
		bufr[1] = header.flags;
		if (len > 0 && (rtn = ReceiveData(bufr + 2, len)) < 0)
		{
			ClientError(clntinx, rtn, "Error reading message data from client");
			break;
		}
		if (!PostThreadMessage(clnttid, WM_USER + 100, (WPARAM)bufr,
				(LPARAM)len))
		{
			free(bufr);
			ClientError(clntinx, ER_ERROR, "Error posting message to client "
					"thread");
			break;
		}
	}
	PostThreadMessage(clnttid, WM_USER + 100, (WPARAM)0, (LPARAM)-1);
	return (0);
}
