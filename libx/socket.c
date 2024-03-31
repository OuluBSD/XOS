// Berkley sockets routines for XOS

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <xos.h>
#include <xossvc.h>
#include <socket.h>


SOCKET accept(
	SOCKET s,
	struct sockaddr *addr,
	int *addrlen)

{
	s = s;
	addr = addr;
	addrlen = addrlen;

	fputs("\n!!!!!!! accept was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}

int bind(
	SOCKET soc,
	const struct sockaddr *addr,
	int namelen)

{
	soc = soc;
	addr = addr;
	namelen = namelen;

	fputs("\n!!!!!!! accept was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


int closesocket(
	long soc)

{
	soc = soc;

	fputs("\n!!!!!!! closesocket was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


int connect(
	SOCKET soc,
	const struct sockaddr *name,
	int namelen)

{
	soc = soc;
	name = name;
	namelen = namelen;

	fputs("\n!!!!!!! connect was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


struct hostent *gethostbyaddr(
	const char *addr,
	int len,
	int type)

{
	addr = addr;
	len = len;
	type = type;

	fputs("\n!!!!!!! gethostbyaddr was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


struct  hostent *gethostbyname(
	const char *name)

{
	name = name;

	fputs("\n!!!!!!! gethostbyname was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


struct servent *getservbyname(
	const char *name,
	const char *proto)

{
	name = name;
	proto = proto;

	fputs("\n!!!!!!! getservbyname was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


struct servent *getservbyport(
	int port,
	const char *proto)

{
	port = port;
	proto = proto;

	fputs("\n!!!!!!! getservbyport was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


int getsockopt(
	SOCKET soc,
	int level,
	int optname,
	char *optval,
	int *optlen)

{
	soc = soc;
	level = level;
	optname = optname;
	optval = optval;
	optlen = optlen;

	fputs("\n!!!!!!! getsockopt was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


u_long htonl(
	u_long hostlong)

{
	hostlong = hostlong;

	fputs("\n!!!!!!! htonl was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


u_short htons(
	u_short hostshort)

{
	hostshort = hostshort;

	fputs("\n!!!!!!! htons was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


int ioctlsocket(
	SOCKET soc,
	long cmd,
	u_long *argp)

{
	soc = soc;
	cmd = cmd;
	argp = argp;

	fputs("\n!!!!!!! ioctlsocket was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


int listen(
	SOCKET soc,
	int backlog)

{
	soc = soc;
	backlog = backlog;

	fputs("\n!!!!!!! listen was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


u_long ntohl(
	u_long netlong)

{
	netlong = netlong;

	fputs("\n!!!!!!! ntohl was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


u_short ntohs(
	u_short netshort)

{
	netshort = netshort;

	fputs("\n!!!!!!! ntohs was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


int recv(
	SOCKET soc,
	char *bufr,
	int size)

{
	long rtn;

	if ((rtn = svcIoInBlock(soc, bufr, size)) < 0)
	{
		errno = -rtn;
		return (-1);
	}
	return (rtn);
}


int recvfrom(
	SOCKET soc,
	char *buf,
	int len,
	int flags,
	struct sockaddr *from,
	int *fromlen)

{
	soc = soc;
	buf = buf;
	len = len;
	flags = flags;
	from = from;
	fromlen = fromlen;

	fputs("\n!!!!!!! recvfrom was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


int send(
	SOCKET soc,
	const char *bufr,
	int size)

{
	long rtn;

	if ((rtn = svcIoOutBlock(soc, (char *)bufr, size)) < 0)
	{
		errno = -rtn;
		return (-1);
	}
	return (rtn);
}


int sendto(
	SOCKET soc,
	const char *buf,
	int len,
	int flags,
	const struct sockaddr *to,
	int tolen)

{
	soc = soc;
	buf = buf;
	len = len;
	flags = flags;
	to = to;
	tolen = tolen;

	fputs("\n!!!!!!! sendto was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


int setsockopt(
	SOCKET soc,
	int level,
	int optname,
	const char *optval,
	int optlen)

{
	soc = soc;
	level = level;
	optname = optname;
	optval = optval;
	optlen = optlen;

	fputs("\n!!!!!!! getsockopt was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


u_long shutdown(
	long soc,
	int mode)

{
	soc = soc;
	mode = mode;

	fputs("\n!!!!!!! shutdown was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}


SOCKET socket(
	int af,
	int type,
	int protocol)

{
	af = af;
	type = type;
	protocol = protocol;

	fputs("\n!!!!!!! socket was called!!!!!!!\n", stderr);
	exit(1);

	return (0);
}
