/******************************************/
/*                                        */
/* INITDATA.C - Data definitions for INIT */
/*                                        */
/******************************************/
/*                                        */
/* Written by John Goltz                  */
/*                                        */
/******************************************/

// ++++
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xossignal.h>
#include <xos.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xostime.h>
#include <xossysp.h>
#include "init.h"

int     mincnt = 60;
ulong   initpid;				// Process ID for INIT
long    conhndl;
long    logdev;
long    logdate;
time_sz bgndt;
long    rtnval;
char    dttmbfr[40];			// Buffer for date/time string

char welcome[50];

RATEDATA ratedata1[] =			// Auto-baud data for type 1
{   {0xFF, 0x00,  110, 1200},	// 110 baud
    {0xFF, 0x00,  150, 1200},	// 150 baud
    {0xFF, 0x00,  300,  600},	// 300 baud
    {0xFF, 0x00, 1200,  200},	// 1200 baud
    {0xFF, 0x00, 2400,  100},	// 2400 baud
};
int numrates1 = (sizeof(ratedata1))/(sizeof(RATEDATA));

RATEDATA ratedata2[] =			// Auto-baud data for type 2
{   {0xFF, 0x00,  300, 600},	// 300 baud
    {0xFF, 0x80,  600, 300},	// 600 baud
    {0xFF, 0x78, 1200, 200},	// 1200 baud
    {0xED, 0x8C, 1800, 100},	// 1800 baud
    {0xFF, 0xE6, 2400, 100},	// 2400 baud
    {0xDC, 0x18, 3600, 100},	// 3600 baud
    {0xFF, 0x0D, 4800,   0},	// 4800 baud
    {0xF0, 0xF0, 9600,   0}		// 9600 baud
};
int numrates2 = (sizeof(ratedata2))/(sizeof(RATEDATA));

uchar logcommit = FALSE;

char prgname[]  = "INIT";

char sysname[34];
char startupdone = FALSE;		// TRUE if startup stuff complete

char ipmerrmsg[] = "? INIT: Cannot open message device IPM:SYS^INIT\r\n"
		"        %s\r\n";
///char ipmstarterr[] = "? INIT: Cannot start message input on IPM:SYS^INIT\r\n"
///		"        %s\r\n";
///char starterr[] = "? INIT: Cannot execute XOSCFG:\\STARTUP.BAT\r\n"
///		"        %s\r\n";

struct cfgchar cfgchar =
{   {(PAR_GET|REP_STR ), 0, "SYSNAME" , sysname, 34, 0},
    {(PAR_GET|REP_DECV), 4, "TOTALMEM", 0}
};

struct statechar statechar =
{   {(PAR_SET|REP_HEXV), 4, "STATE", 1},
    {(PAR_GET|REP_TEXT), 4, "INITIAL", 0}
};

char systemspec[] = "SYSTEM:";

QAB cfgqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// func    - Function
    0,							// status  - Returned status
    0,							// error   - Error code
    0,							// amount  - Process ID
    0,							// handle  - Device handle
    0,							// vector  - Vector for interrupt
    0, 0, 0,					//         - Reserved
    CF_VALUES,					// option  - Options or command
    0,							// count   - Count
    systemspec, 				// buffer1 - Pointer to file spec
    (char *)&cfgchar,			// buffer2 - Unused
    NULL						// parm    - Pointer to parameter area
};
