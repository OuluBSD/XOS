// This class implements a general purpose interface to a SignalFire radio
//   network using an RS-232 serial interface connected to a SignalFire
//   gateway node. It is intended to be used with programs written using
//   the SWC Windows environment.

#if !defined SFGINTERFACE_H
#define SFGINTERFACE_H

#include "SFGInterface.h"

class SFGInterfaceSWC : public SFGInterface

{public:
	SFGInterfaceSWC(void (*notify)(int func, pkt *bufr, int size, void *arg),
			void *arg);
	void notifyfnc(int func, pkt *bufr, int sz, void *arg1);

	int notifymsg;

///	int SetComm(int port, int speed);
///	int Start();
///	int Stop();
///	int Send(long node, unsigned char *data, int size);
///	int GetRoute(long node, long *route);
///	int GetStatus(long node, status *sts);

///	serinthd  *inthread;
///	seroutthd *outthread;
///	int        port;
///	int        speed;
///	char       started;

};

#endif