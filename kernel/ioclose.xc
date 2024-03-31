//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <xos.h>
#include <xosx.h>
///#include <xossvc.h>
#include <xoslib.h>
#include <xoserr.h>
#include <ctype.h>
#include <stddef.h>


static long XOSFNC cancelhandle(long func, DCB *dcb, long hndl, TDA *reqtda);
static void XOSFNC cancelwait(long hndl);


//******************************************************//
// Function: closeall - close all devices for a process //
// Returned: Nothing                                    //
//******************************************************//

// This is called when terminating the base thread for a process. It is
//   called after all other user threads have been terminated and after all
//   active IO for this thread has been canceled.

void XOSFNC closeall(void)
{
	int hndl;

	while (TRUE)
	{
		hndl = 1;
		do
		{
			if ((knlTda.dcb = knlPda.hndltbl[hndl].dcb) != NULL) // Is this slot
			{													 //   in use?
				if ((((long)knlTda.dcb) & 0x80000000) == 0) // Yes - is it a
				{											//   real DCB?
					// Either it was just allocated or is being transfered. In
					//   either case we can just discard it!

					knlPda.hndltbl[hndl].dcb = NULL;
					knlPda.hndltbl[hndl].cmd = 0;
					continue;
				}

				// Here with the address of a real DCB - Set up to call the
				//   close routine for this device. Since we are specifying
				//   direct IO this is a blocking call which does not return
				//   until the close is complete.

				knlTda.cmd = knlPda.hndltbl[hndl].cmd;
				knlTda.handle = hndl;
				knlTda.qab = NULL;
				knlTda.status = 0;
				knlTda.error = 0;
				knlTda.option = 0;
				knlTda.parm = NULL;
				knlTda.func = QFNC_DIO|QFNC_CLOSE;
				close1();
			}
		} while (++hndl <= knlPda.hndlnum);

		// Here with all handle table devices closed - Now close any held
		//   devices. We do this by unholding the held devices and placing
		//   them in the handle table and continuing to close devices.

		if (knlPda.hdlblist == NULL)	// Finished if no held devices
			break;
		while (knlPda.hdlblist != NULL)
		{
			CRASH('HLDD');				// WRITE THIS SOON!!!
		}
	}
	knxLogGiveAllPaths();				// Give up all device paths
}


//****************************************************************//
// Function: closefunc - QFNC_CLOSE routine - Close device handle //
// Returned: 0 if no immediate error or a negative XOS error code //
//****************************************************************//

// The only error returned directly is ER_BDDVH. For all other errors, the
//   error code is stored in knlPda.error and 0 is returned.

// The close function may be queued or direct for all devices, however queued
//   calls are only useful to allow signals during the close operation. Queued
//   closes do not wait for any active operation to complete but immediately
//   cancel or abort any active operations for the handle and then immediately
//   close the handle. If the device is open on more than one handle only IO
//   operations for this handle are canceled or aborted. IO operations for
//   other handles are not affected. Direct close calls are valid if queued
//   operations for the handle are active. In this case, all IO operations
//   for the handle, including any needed to cancel current operations, are
//   done in the user thread and are not interruptable. 

// Assuming that the handle is valid, this call ALWAYS completely frees up
//   the handle, even when errors are reported. Errors are possible when IO
//   is needed to close the device, for example, to update a file's directory
//   entry when closing a file. 

// NOTE: A device driver MUST NOT do IO when closing a handle unless there
//       are no other handles open on the device. When closing the last handle
//       a device driver may set up an IO frame and do IO. In this case there
//       will never be an IO frame set up when the sd_close routine is called.

long XOSFNC closefunc(void)
{
	long rtn;

	if ((rtn = sysIoGetDcb()) < 0)
		return (rtn);

	if ((knlTda.func & QFNC_DIO) == 0)	// Want to do this directly?
	{
		knlTda.routine = close1;		// No - start an IO thread
		sysIoBeginThread(sysIoFinishQueue);
	}
	else
	{
		close1();
	}
	return (0);
}


//****************************************//
// Function: close1 - Close device handle //
// Returned: 0                            //
//****************************************//

// For all errors, the error code is stored in knlPda.error.

void XOSFNC close1(void)
{
	TDA *tda;
	SLB *slb;

	TOFORK;

	knlPda.hndltbl[knlTda.handle].cmd |= XO_NOQUE; // Disable queuing for this
												   //   handle
	if ((tda = knlTda.dcb->outtda) == knlTda.addr && // Cancel output if
			tda->handle == knlTda.handle)			   //   need to
		sysIoCancel(CAN_ALL, tda, knlTda.handle);
	if ((tda = knlTda.dcb->inptda) == knlTda.addr && // Cancel input if
			tda->handle == knlTda.handle)			   //   need to
		sysIoCancel(CAN_ALL, tda, knlTda.handle);

	// Here with the device idle on this handle. The device may be active
	//   on one or two different handle if it is open on one or more other
	//   handles.

	if (knlTda.dcb->opencnt == 0)		// Is the device open?
		CRASH('DNOC');					// [Device Not Open when Closing]
	if (knlTda.dcb->opencnt == 1)		// Is this the last close?
	{									// Yes
		if (((long)knlTda.dcb->inptda) | ((long)knlTda.dcb->outtda) != 0)
			CRASH('DIAC');				// [Device IS Active for Close]

		knlTda.dcb->outtda = knlTda.addr;	  // Set up an output frame in case
		knlTda.frame = offsetof(DCB, outtda); //   the device needs it
	}

	FROMFORK;

	if (knlTda.dcb->qfdisp->close != NULL) // Call the device driver for close
		knlTda.dcb->qfdisp->close();	   //   if necesary

	if (knlPda.hndltbl[knlTda.handle].dcb != knlTda.dcb) // Debug - make sure
		CRASH('BHTE');									 //   handle table is

	TOFORK;
														 //   right
	if (knlPda.hndltbl[knlTda.handle].waitcnt != 0) // Is anyone doing a
													//   special wait for this?
		sysSchEndWaitAll(((ulong)knlTda.dcb) >> 6); // Yes - wake him up

	knlPda.hndltbl[knlTda.handle].dcb = NULL; // Clear the handle table entry
	knlPda.hndltbl[knlTda.handle].cmd = 0;

	knlTda.dcb->outtda = NULL;			// Destroy the output frame but leave
	knlTda.frame = 0;					//   the DCB address in knlTda.dcb for
										//   cleardcb
	if (knlTda.dcb->clsslb != NULL)		// Do we need a close signal?
	{
		// Here if need a close signal. The XMB pointed to by dcb_clsslb is
		//   set up to be used as an SLB when calling sysIoReqSignal. The item
		//   count and the 4 data items are stored in the correct locations.
		//   The vector is stored in SLB.vector. The PID is stored in the
		//   SLB.gdata item.

		slb = knlTda.dcb->clsslb;
		knlTda.dcb->clsslb = NULL;
		sysSchReqSignal(slb->vector, NULL, 0, slb->gdata, 0x80000000, slb);
	}

	FROMFORK;

	cleardcb();							// Give up the DCB if necessary
	knlTda.dcb = NULL;					// Clear the reference to the DCB
	knlTda.status |= QSTS_DONE;
}


//************************************//
// Function: cleardcb - Give up a DCB //
// Returned: Nothing                  //
//************************************//

void XOSFNC cleardcb(void)
{
	if (knlTda.dcb->clsslb != NULL)		// Still have a close SLB?
	{
		sysMemGiveXmb(knlTda.dcb->clsslb); // Yes - give it up
		knlTda.dcb->clsslb = NULL;
	}
	if (--knlTda.dcb->opencnt == 0 && knlTda.dcb->hdlb == NULL)
	{									// Reduce the open count - Is it still
										//   in use?
		knlTda.dcb->sespda = NULL;
		knlTda.dcb->outtda = NULL;
		knlTda.dcb->inptda = NULL;
		if (knlTda.dcb->qfdisp->cleardcb != NULL)
			knlTda.dcb->qfdisp->cleardcb();
		knlTda.dcb = NULL;
		knlTda.frame = 0;
	}
}


//*************************************************//
// Function: psvcIoCancel - SVC to cancel/abort IO //
// Returned: Number of transfers canceled or a     //
//           negative XOS error code if error      //
//*************************************************//

// Currently active operations are aborted. This will terminate the
//   operation as quickly as possible and cause it to return an error code
//   of ER_ABORT. Queued operations that are not yet active are canceled.
//   They are terminated immediately with an error code of ER_CANCL.
//   Termination signals for aborted and canceled operations for a handle
//   will be delivered in the order the operations were queued. The order of
//   delivery for termination signals for different handles is not defined.

// This SVC must be used with care in a multi-thread environment. It can be
//   used to cancel IO being done in any thread (user or IO) by any user
//   thread. However, there are no interlocks to prevent another thread from
//   issueing a new IO request for a handle immediately after a request for
//   it is canceled. Such situations must be handled by user code. Also,
//   open and close operations CANNOT be canceled. Devices being opened or
//   closed are ignored. Also, critical IO operations (mostly IO operations
//   for local mass storage devices) are not canceled until they reach a
//   "safe" state. Interruptable direct IO cannot be canceled unless that
//   IO operation is active. Such transfers are not active while waiting.
//   Canceling of such an io transfer can be emulated by sending a signal
//   to the user process and having the signal routine modify one of the
//   arguments of the svcIoQueue call to cause the call to fail when it is
//   re-done after the signal is dismissed. This requires that the svcIoQueue
//   be executed directly by the user code, not by one of the sysIo library
//   functions.

//	long XOSSVC svcIoCancel(
//		long func,		// Function bits:
//						//   CAN_WAIT   = 0x8000 - Wait until complete
//						//   CAN_ALL    = 0x4000 - Cancel all requests for
//						//                           handle
//						//   CAN_NOSIG  = 0x0010 - Suppress IO done signals
//						//   CAN_OUTPUT = 0x0002 - Cancel output
//						//   CAN_INPUT  = 0x0001 - Cancel input
//		long hndl,		// Device handle
//		long tid);		// Thread ID

long XOSFNC psvcIoCancel(void)
{

	return (ER_NIYT);
}


//*******************************************************//
// Function: sysIoCancel - cancel or abort IO operations //
// Returned: The number of requests canceled or aborted  //
//           or a negative XOS error code if error       //
//*******************************************************//

// must be called from main program level
//   raised to fork level  - the current queued IO operation (if any) is aborted
//   and all queued requests are canceled

// This function will cancel any specified queued IO request and will abort
//   any non-critical active specified IO request. Critical IO is any IO which
//   may damage system integraty if not allowed to go to completion. This mainly
//   consists of transfers involving local mass storage devices. Such transfers
//   are allowed to run to completion. Transfers involving remote system are
//   never critical.

// A transfer is specified by a user thread (TDA address), a device handle,
//   and transfer direction (for full duplex devices). If a transfer direction
//   is not specified, output only is assumed for full-duplex devices. This
//   parameter is ignored for half-duplex devices. (If the thread or handle
//   is not  specified, all transfers that match the ones that are specified
//   are canceled.

// IO can only be canceled for the same process. Direct IO can be canceled from
//   a different user thread. Queued IO can be canceled from any user thread in
//   the process.

// NOTE: This version DOES not support multiple queued IO requests. Queued IO,
//       without multiple requests (simple non-blocking IO), IS supported.
//       (Multiple queued IO requests are NOT supported at all in this version
//       of XOS!)
	
long XOSFNC sysIoCancel(
 	long func, 			// Function bits (see comments for psvcIoCancel)
	TDA *tda,			// Address of TDA
	long hndl)			// Device handle
{
	DCB *dcb;
	long val;

	val = 0;
	if (hndl == 0)
	{
		// Here if no handle was specified - We terminate all IO operations
		//   for the specified TDA or for all TDAs if none specified.

		hndl = 1;
		do
		{
			if ((dcb = knlPda.hndltbl[hndl].dcb) != NULL) // Is this slot in
														  //   use?
				val += cancelhandle(func, dcb, hndl, tda);
		} while (++hndl <= knlPda.hndlnum);
	}
	else
	{
		// Here if a handle was specified - We terminate IO operations for the
		//   specified handle that match the specified TDA (if any).

		if ((dcb = knlPda.hndltbl[hndl].dcb) != NULL) // Is this slot in use?
			val += cancelhandle(func, dcb, hndl, tda);
	}
	return (val);
}



static long XOSFNC cancelhandle(
	long func,
	DCB *dcb,
	long hndl,
	TDA *reqtda)
{
	TDA   *tda;
	long   val;
	ushort seqnum;

	if ((((long)dcb) & 0x80000000) == 0) // Yes - is it a real DCB?
	{
		// Either it was just allocated or is being transfered. In
		//   either case we can just discard it!

		knlPda.hndltbl[hndl].dcb = NULL;
		knlPda.hndltbl[hndl].cmd = 0;
		return (0);
	}

	// Here with the address of a real DCB

	if (knlPda.hndltbl[hndl].cmd & (XO_NOQUE|XO_OPNCLS))
	{
		cancelwait(hndl);
		return (0);
	}	
	val = 0;
	seqnum = dcb->inpseq;
	while ((dcb->dsp & DS_DUPLEX) && (func & CAN_INPUT) &&
			(tda = dcb->inptda) != NULL && (reqtda == NULL ||
			reqtda == ((tda->type == TDATYPE_IO) ? tda->usertda : tda)))
	{
		if ((knlTda.qsts1 & XF1_ABTINP) == 0)
		{
			val++;
			knlTda.qsts1 |= XF1_ABTINP; // Indicate aborting input
			if (func & CAN_ALL)	// Want to cancel everything?
				tda->qsts1 |= XF1_CANINP;
			if (func & CAN_NOSIG) // Want to supress signals?
				tda->qsts1 |= XF1_NOINPSIG;
			if ((dcb->dsp & DS_NOABORT) == 0) // Can we stop it?
				sysIoResumeThread(tda, ER_ABORT); // Yes
		}
		if (func & CAN_WAIT)
		{
			cancelwait(hndl);
			if (dcb != knlPda.hndltbl[hndl].dcb || dcb->inpseq != seqnum)
				break;
		}
		else
			break;
	}
	seqnum = dcb->outseq;
	while (((dcb->dsp & DS_DUPLEX) == 0 || (func & CAN_OUTPUT)) &&
			(tda = dcb->outtda) != NULL && (reqtda == NULL ||
			reqtda == ((tda->type == TDATYPE_IO) ? tda->usertda : tda)))
	{
		if ((knlTda.qsts1 & XF1_ABTOUT) == 0)
		{
			val++;
			knlTda.qsts1 |= XF1_ABTOUT; // Indicate aborting
			if (func & CAN_ALL)	// Want to cancel everything
				tda->qsts1 |= XF1_CANOUT;
			if (func & CAN_NOSIG) // Want to supress signals?
				tda->qsts1 |= XF1_NOOUTSIG;
			if ((dcb->dsp & DS_NOABORT) == 0) // Can we stop it?
				sysIoResumeThread(tda, ER_ABORT); // Yes
		}
		if (func & CAN_WAIT)
		{
			cancelwait(hndl);
			if (dcb != knlPda.hndltbl[hndl].dcb || dcb->outseq != seqnum)
				break;
		}
		else
			break;
	}
	return (val);
}


//**************************************************************//
// Function: cancelwait - Wait until a handle has been canceled //
// Returned: Nothing                                            //
//**************************************************************//

// This waits using a wait index of the address of the DCB divided by 64.
//   This is somewhat difficult because there are several way in which the
//   state of the handle and/or device can change while we are waiting:
//   1. (usual case) The IO operation being canceled has completed and the
//      device is now idle.
//   2. The wake up is spurious and the handle is still active.
//   3. Same as 1 plus a new IO operation is now active.
//   4. Same as 1 plus the handle has been or is being closed.
//   5. Same as 1 plus the handle has been closed and a new device is being
//      opened or is open on the handle.
//   The code calling this function handles most of this, mainly by checking
//   the address of the DCB and the operation serial number in the DCB. Note
//   that the waitcnt we modify here is only modified in this routine. It is
//   never changed by any other code that modifies the handle table entries.

static void XOSFNC cancelwait(
	long hndl)
{
	knlPda.hndltbl[hndl].waitcnt++;
	sysSchSetUpToWait(((ulong)knlPda.hndltbl[hndl].dcb) >> 6, -1, THDS_DW2);
	SCHEDULE();
	TOFORK;
	if (knlPda.hndltbl[hndl].waitcnt == 0)
		CRASH('BDWC');					// [BaD Wait Count]
	knlPda.hndltbl[hndl].waitcnt--;
}
