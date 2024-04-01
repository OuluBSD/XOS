#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

imgblk::imgblk()

{
	data = NULL;
}

imgblk::~imgblk()

{
	if (data != NULL)
		delete [] data;
}
