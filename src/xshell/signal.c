//------------------------------------------------------------------------------
//
//  signal.c - Signal functions
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"


//**********************************************************************
// Function: childsignal - Signal function for child process termination
// Returned: Nothing
//**********************************************************************

void childsignal(
	SIGDATA sigdata)
{
	int  len;
	char buffer[180];

/*
	if (sigdata.term != (TC_EXIT << 24) && sigdata.term != (TC_KILL << 24) &&
			sigdata.term != (TC_CTRLC << 24))
	{
		if (flags & FLG_HAVECC)
			svcSchCtlCDone();
		printf("\nProcess terminated:\n"
				"   pid = %8.8lX term = %8.8lX  CS:EIP = %2X:%8.8lX\n"
				"   EFR = %8.8lX data = %8.8lX Address = %2X:%8.8lX\n",
				sigdata.pid, sigdata.term, sigdata.pCS, sigdata.pEIP,
				sigdata.pEFR, sigdata.data, sigdata.pseg, sigdata.poffset);
	}
*/
    if (childdata.pid == sigdata.pid)	// This process?
	{
		childdata.trm = sigdata.term >> 24;
		childdata.status = (sigdata.term & 0x00800000)?
				sigdata.term | 0xFF000000L: sigdata.term & 0x00FFFFFF;
		childdata.pEIP = sigdata.pEIP;
		childdata.pCS = sigdata.pCS & 0xFFFF;
		childdata.pEFR = sigdata.pEFR;
		childdata.offset = sigdata.poffset;
		childdata.segment = sigdata.pseg & 0xFFFF;
		childdata.data = sigdata.data;
		childflag = 0;
	}
	else
    {
		// This should never happen! If it does we display information about
		//   it here, even though this will probably mess up output from
		//   anything that is running!

		if (flags & FLG_HAVECC)
			svcSchCtlCDone();
		svcTrmDspMode(STDTRM, DM_USEBASE, NULL);
		svcTrmFunction(STDTRM, TF_RESET);
		len = sprintf(buffer, "\r\nUnknown process terminated, PID = %u.%u, "
				"Reason = %d, Status = %6.6X\r\n", sigdata.pid >> 16,
				sigdata.pid & 0xFFFF, sigdata.term >> 24,
				sigdata.term & 0xFFFFFF);
		svcIoOutBlock(ccs->cmderr, buffer, len);
	}
}


//****************************************************
// Function: cntrlcsignal - Signal function for ctrl-C
// Returned: Nothing
//****************************************************

// There are 3 cases where we can get a ctrl-C signal:
//   1) Reading command line input. This includes a small time before and
//      after the actual read call is executed. This is indicated by
//      FLG_HOLDCC not being set. In this case if we are not reading command
//      input from a terminal we simply set FLG_HAVECC and dismiss. The
//      command routine will check FLG_HAVECC after reading a command and
//      will terminate all batch levels if it is set. If we are reading
//      from a terminal we set the qab.func value in the QAB used for input
//      to 0 and dismiss. Since terminal input is always done using direct IO
//      this will fail immediately with an ER_IFDEV error when the svcIoQueue
//      SVC is reexecuted allowing FLG_HAVECC to be seen and acted upon
//      immediately. By changing the function value in the QAB we eliminate
//      a race with waiting for IO without having to change the signal level.
//   2) An internal command is being processed. This includes possible
//      execution of critical code (such as malloc) that must not be
//      interrupted. In this case FLG_HOLDCC will be set. Here we simply
//      set FLG_HAVECC and dismiss the signal. The internal command either
//      always completes quickly or will check FLG_HAVECC and terminate early
//      if it is set.
//   3) A child process is being run. In this case childpid will be non-zero.
//      We try to request a CNTC signal for the child. If this works we simply
//      dismiss this signal and let the child to handle this. If it does not
//      work we terminate the process and set FLG_HAVECC. A potential race here
//      is handled with childflag which is used to enable suspending when
//      waiting for the child to terminate.
// Note that this version DOES NOT implment the DOS-like "do you want to
//   terminate the process" question when executing a BAT file. It is simply
//   terminated unconditinally on ^C.

void cntlcsignal(void)
{
	if (childdata.pid != 0)				// Are we running a program now?
	{
		if (svcSchReqSignal(VECT_CNTC, NULL, 0, childdata.pid) < 0)
										// Try to send the child a CNTC signal
			svcSchKill(TC_CTRLC << 24, childdata.pid | 0x80000000);
										// Signal failed - terminate the
										//   child process
	}
	else if ((flags & FLG_HOLDCC) == 0)	// Do we need to hold off the ^C?
		inpqab.func = 0;				// No - ensure that terminal input
										//   will fail
	flags |= FLG_HAVECC;
	svcSchCtlCDone();
	svcIoOutString(DH_STDTRM, "^C\n", 0);
}


//**************************************************************
// Function: hungupsignal - Signal function for terminal hang-up
// Returned: Nothing
//**************************************************************

void hungupsignal(void)
{


}
