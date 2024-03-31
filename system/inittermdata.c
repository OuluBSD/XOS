/****************************************88888*************/
/*                                                        */
/* inittermreq.c - Terminal data message handler for INIT */
/*                                                        */
/**********************************************************/
/*                                                        */
/* Written by John Goltz                                  */
/*                                                        */
/**********************************************************/

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <xcstring.h>
#include <utility.h>
#include <xos.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xostime.h>
#include <xosmsg.h>
#include <xoserr.h>
#include "init.h"


//**********************************************************
// Function: terminaldata - Handle the terminal data message
// Returned: Nothing
//**********************************************************

// This message is sent by the TRM class driver when input data is received
//   from an idle terminal. Format of the terminal data message:
//  Offset    Use
//     0    Message type (MT_TERMDATA)
//     1    Reason - 0 = data, 1 = carrier detect, 2 = ring detect
//     2    Data byte if reason = 1, unused if reason = 0
//     3    Rate detect type (0 = fixed rate)
//     4    First byte of terminal name (without colon)

void terminaldata(
	uchar *msg,
	int    len)
{
    msgbfr[len] = ':';
    msgbfr[len + 1] = '\0';
    if (msg[1] == 0 && msg[2] != '\r')	// If have data, it must be CR!
		return;
	makesession(&msg[4], NULL, "user", "user");
}
