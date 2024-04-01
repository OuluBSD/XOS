#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <process.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"


swcThread::swcThread(
	unsigned (__stdcall *address)(void *),
	void  *args,
	long   stksize)

{
	thrdhndl = _beginthreadex(NULL, stksize, address, args, 0, &thrdid);

}

///	uint  thrdid;
///	ulong thrdhndl;


