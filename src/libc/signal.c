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

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <xos.h>
#include <errno.h>
#include <signal.h>
#include <xossignal.h>
#include <string.h>
#include <xossvc.h>


// Define structure for the generic signal stack data

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
} SIGDATA;

static void sigexit(void);
static void sigfpuerr(void);
static void sigillins(void);
static void sigint(void);
static void sigmemflt(void);
static void sigpterm(void);
static void sigbreak(void);

// Signal table

typedef struct
{	uchar  vect1;
	uchar  vect2;
	char   xxx[2];
	void (*xfunc)(void);
	void (*ufunc)(int num);
} ST;



ST sigtbl[] =
{	{VECT_EXIT  , 0           , 0, 0, sigexit},		// SIGABRT  = 1
	{VECT_FPUERR, 0           , 0, 0, sigfpuerr},	// SIGFPE   = 2
	{VECT_ILLINS, 0           , 0, 0, sigillins},	// SIGILL   = 3
	{VECT_CNTC  , 0           , 0, 0, sigint},		// SIGINT   = 4
	{VECT_SEGNP , VECT_PAGEFLT, 0, 0, sigmemflt},	// SIGSEGV  = 5
	{VECT_PTERM , 0           , 0, 0, sigpterm},	// SIGTERM  = 6
	{VECT_CNTC  , 0           , 0, 0, sigbreak}		// SIGBREAK = 7
};


//******************************************************
// Function: signal - Set up signal handling Unix style
// Returned: Address of previous function for the signal
//******************************************************

__sig_func *signal(
	int         sig,
	__sig_func *func)
{
	__sig_func *pfunc;

	if (sig < 1 || sig > 7)
	{
		errno = -ER_VALUE;
		return (SIG_ERR);
	}
	pfunc = sigtbl[sig - 1].ufunc;
	if (func == SIG_DFL)				// Want default action?
	{
		setvector(sigtbl[sig - 1].vect1, 0, NULL);
		if (sigtbl[sig - 1].vect2 != 0)
			setvector(sigtbl[sig - 1].vect2, 0, NULL);
		sigtbl[sig - 1].ufunc = NULL;
	}
	else
	{
		sigtbl[sig - 1].ufunc = func;
		if (func == SIG_IGN)			// Want to ignore the signal?
			func = NULL;
		setvector(sigtbl[sig - 1].vect1, 4, sigtbl[sig - 1].xfunc);
		if (sigtbl[sig - 1].vect2 != 0)
			setvector(sigtbl[sig - 1].vect2, 4, sigtbl[sig - 1].xfunc);
	}
	return (pfunc);
}


// Following are internal signal handling stubs

//******************
// Function: sigexit
// Returned: Nothing
//******************

static void sigexit(void)
{
	if (sigtbl[SIGABRT - 1].ufunc != NULL)
		(sigtbl[SIGABRT - 1].ufunc)(SIGABRT);
}


//********************
// Function: sigfpuerr
// Returned: Nothing
//********************

static void sigfpuerr(void)
{
	if (sigtbl[SIGFPE - 1].ufunc != NULL)
		(sigtbl[SIGFPE - 1].ufunc)(SIGFPE);
}


//********************
// Function: sigillins
// Returned: Nothing
//********************

static void sigillins(void)
{
	if (sigtbl[SIGILL - 1].ufunc != NULL)
		(sigtbl[SIGILL - 1].ufunc)(SIGILL);
}


//******************
// Function: sigint
// Returned: Nothing
//******************

static void sigint(void)
{
	if (sigtbl[SIGINT - 1].ufunc != NULL)
		(sigtbl[SIGINT - 1].ufunc)(SIGINT);
}


//*******************
// Function: sigbreak
// Returned: Nothing
//*******************

static void sigbreak(void)
{
	if (sigtbl[SIGBREAK - 1].ufunc != NULL)
		(sigtbl[SIGBREAK - 1].ufunc)(SIGBREAK);
}


//********************
// Function: sigmemflt
// Returned: Nothing
//********************

static void sigmemflt(void)
{
	if (sigtbl[SIGSEGV - 1].ufunc != NULL)
		(sigtbl[SIGSEGV - 1].ufunc)(SIGSEGV);
}


//*******************
// Function: sigpterm
// Returned: Nothing
//*******************

static void sigpterm(void)
{
	if (sigtbl[SIGTERM - 1].ufunc != NULL)
		(sigtbl[SIGTERM - 1].ufunc)(SIGTERM);
}
