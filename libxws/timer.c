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

#include <xosxxws.h>

// Timer processing is somewhat complex. We want to be able to call general
//   XWS function from timer functions, which means these functions must
//   execute in a fiber, NOT at signal level.

// XWS timer functions are called from the draw thread.

#define TIMERNUM 12			// Maximum number of active timers

typedef struct
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    long  intEIP;
    long  intCS;
    long  intEFR;
    short intcnt;
    short intnum;
    long  hndl;
    long  data;
} TIDATA;

typedef struct _timerent TIMERENT;
struct _timerent
{	TIMERENT *next;
	long      hndl;
	int       repeat;
	XWSTIMER *func;
	long      data;
};

static TIMERENT   timertbl[TIMERNUM];

static TIMERENT  *timerhead;
static TIMERENT **timertail;
static TIMERENT  *timerfree;


static void timersignal(TIDATA tid);


//******************************************
// Function: xwstimerinit Tinitialize timers
// Returned: Nothing
//******************************************

void xwstimerinit(void)
{
	TIMERENT *pnt;
	long      rtn;
	int       cnt;

	if ((rtn = setvector(xwsvectbase + XWS_VECT_TIMER, 1, timersignal)) < 0)
		xwsFail(rtn, "Error setting timer vector");
	pnt = timertbl;
	cnt = TIMERNUM - 1;
	do
	{
		pnt->next = pnt + 1;
		pnt->hndl = 0;
		pnt++;
	} while (--cnt > 0);
	pnt->next = NULL;
	pnt->hndl = 0;
	timerfree = timertbl;
	timerhead = NULL;
	timertail = &timerhead;
}


//************************************************
// Function: xwsTimerStart - Start a timer
// Returned: Timer handle (positive) if successful
//				or a negative error code if error
//************************************************

// This function uses the Pascal calling sequence

long xwsTimerStart(
	XWSTIMER *func,		// Timer function
	long      data,		// Data value passed to the timer function
	llong     time)		// Time interval (bit 63 set if repeated)
{
	TIMERENT *pnt;
	long      rtn;

	if ((pnt = timerfree) == NULL)
		return (ER_TMALM);
	if ((rtn = svcSchSetAlarm(xwsvectbase + XWS_VECT_TIMER, pnt - timertbl + 1,
			time)) < 0)
		return (rtn);
	timerfree = pnt->next;
	pnt->hndl = rtn + (((long)(time >> 32)) & 0x80000000);
	pnt->repeat = (int)(((ullong)time) >> 63);
	pnt->func = func;
	pnt->data = data;
	pnt->next = (TIMERENT *)-1;
	return (pnt - timertbl + 1);
}


//************************************************************
// Function: xwsTimerStop - Stop a timer
// Returned: 0 if successful or a negative error code if error 
//************************************************************

// This function uses the Pascal calling sequence

long xwsTimerStop(
	long hndl)
{
	TIMERENT *pnt;
	long      rtn;

	if (hndl < 1 || hndl > TIMERNUM)
		return (ER_VALUE);
	pnt = timertbl + hndl - 1;
	if (pnt->hndl == 0)
		return (ER_NACT);
	if (pnt->next == (TIMERENT *)-1)
		rtn = svcSchClrAlarm(hndl);
	pnt->hndl = 0;
	pnt->next = timerfree;
	timerfree = pnt;
	return (rtn);
}


void xwsdotimer(void)
{
	TIMERENT *pnt;

	while (timerhead != NULL)
	{
		pnt = timerhead;
		if ((timerhead = pnt->next) == NULL)
			timertail = &timerhead;
		if (pnt->func != NULL)
			(pnt->func)(pnt->hndl, pnt->data);
		if (!pnt->repeat && pnt->hndl != 0)
		{
			pnt->hndl = 0;
			pnt->next = timerfree;
			timerfree = pnt;
		}
		else
			pnt->next = (TIMERENT *)-1;
	}
}



//**********************************************
// Function: timersignal - Timer signal function
// Returned: Nothing
//**********************************************

static void timersignal(
	TIDATA tid)
{
	TIMERENT *pnt;

	if (tid.data >= 1 && tid.data <= TIMERNUM)
	{
		pnt = timertbl + tid.data - 1;
		if (pnt->hndl == tid.hndl && pnt->next == (TIMERENT *)-1)
		{
			pnt->next = NULL;
			*timertail = pnt;
			timertail = &pnt->next;
			if (timerhead == NULL)
				timerhead = pnt;
			fbrCtlWake((long)xwsdrawtdb);
		}
	}
}
