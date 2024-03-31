#include "install.h"

static int func;


//***************************************************
// Function: dowork - Requests work by the work fiber
// Returned: Nothing
//***************************************************

void dowork(
	int funcarg)
{
	func = funcarg;
	fbrCtlWake(wrkfbr);
}


//**********************************************************
// Function: workfiber - Thread function for the work thread
// Returned: Never returns
//**********************************************************

// This fiber does most of the work for installing the software.

void workfiber(void)

{
	while (TRUE)
	{
		while (func == 0)
			fbrCtlSuspend(NULL, -1);
		switch (func)
		{
		 case FUNC_SCAN:
			doscanwork();
			break;

		 case FUNC_INST:
			doinstwork();
			break;

		 case FUNC_INIT:
			doinitwork();
			break;
		}
		func = 0;
	}
}
