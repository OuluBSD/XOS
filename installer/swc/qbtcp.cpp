




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
			return ((MMSER_COMMERR << 16) + (ER_NILAD & 0xFFFF));
		hostipa = *(ulong *)(host->h_addr);
	}
	if ((soc = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return ((MMSER_COMMERR << 16) + (ER_NNPA & 0xFFFF));
	socaddr.sin_family = AF_INET;
	socaddr.sin_port = 0;
	socaddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(soc, (SOCKADDR *)&socaddr, sizeof(socaddr)) == SOCKET_ERROR)
	{
		closesocket(soc);
		soc = 0;
		return ((MMSER_COMMERR << 16) + (ushort)ER_NNPA);
	}
	socaddr.sin_port = htons((short)(oblk->len));
	socaddr.sin_addr.s_addr = hostipa;
	if (connect(soc, (SOCKADDR *)&socaddr, sizeof(socaddr)) == SOCKET_ERROR)
	{
		code = xlatewinsocerror(WSAGetLastError());
		closesocket(soc);
		soc = 0;
		return ((MMSER_COMMERR << 16) + (ushort)code);
	}
	if ((tcpinthd = (HANDLE)_beginthreadex(NULL, 0, tcpinthread, this, 0,
			&(tcpintid))) == NULL)
	{
		closesocket(soc);
		soc = 0;
		return ((MMSER_SYSTEM << 16) + (ushort)ER_TMTHD);
	}
	connected = true;
	noopcnt = KACNT;
	return (0);
}









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
				if (!PostThreadMessage(basetid, WM_USER + 100, (uint)ER_NDRTL,
						(long)arg))
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
		if (!PostThreadMessage(basetid, WM_USER + 100, size, (long)arg))
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
			return ((MMSER_COMMERR << 16) + (ushort)xlatewinsocerror(
					WSAGetLastError()));
		len -= rtn;
		pnt += len;
	}
	if (oblk->result & SENDFLG_DATA)
	{
		givememory(oblk->data.p);
		oblk->data.p = NULL;
	}





static long xlatewinsocerror(
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
		return (ER_NAINU);

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
