#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <process.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "SFThread.h"

SFThread::SFThread(
	unsigned int (__stdcall *address)(void *),
	void  *argsarg,
	long   stksize)

{
	args = argsarg;
	thrdhndl = _beginthreadex(NULL, stksize, address, this, 0, &thrdid);
}


