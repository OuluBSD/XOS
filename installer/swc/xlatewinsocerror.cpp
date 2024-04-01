#include "xoserr.h"
#include <winsock.h>


long xlatewinsocerror(
	int wscode)

{
	switch (wscode)
	{
	 case WSAEMSGSIZE:
		return (ER_NDRTL);

	 case WSAEPROTOTYPE:
		return (ER_NILPC);

	 case WSAENOPROTOOPT:
	 case WSAEPROTONOSUPPORT:
	 case WSAESOCKTNOSUPPORT:
	 case WSAEOPNOTSUPP:
	 case WSAEPFNOSUPPORT:
	 case WSAEAFNOSUPPORT:
		return (ER_NIYT);

	 case WSAEADDRINUSE:
		return (ER_NANAV);

	 case WSAEADDRNOTAVAIL:
		return (ER_NANAV);

	 case WSAENETDOWN:
	 case WSAENETUNREACH:
		return (ER_NLKNA);

	 case WSAECONNABORTED:
		return (ER_ABORT);

	 case WSAENETRESET:
	 case WSAESHUTDOWN:
	 case WSAECONNRESET:
		return (ER_NCLST);

	 case WSAECONNREFUSED:
		return (ER_NCRFS);

	 case WSAENOBUFS:
		return (ER_NOBUF);

	 case WSAEISCONN:
		return (ER_NACON);

	 case WSAENOTCONN:
		return (ER_NNCON);

	 case WSAETIMEDOUT:
		return (ER_NTTIM);

	 case WSAENAMETOOLONG:
		return (ER_BDSPC);

	 case WSAEHOSTDOWN:
		return (ER_NHSNA);

	 case WSAEHOSTUNREACH:
		return (ER_NAUNR);

	 case WSAEDISCON:
		return (ER_NCCLR);

	 case WSAENOTSOCK:
		return (ER_NCLST);
	}
	return (ER_ERROR);
}
