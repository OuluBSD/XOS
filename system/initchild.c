/*****************************************************************/
/*                                                               */
/* INITCHILD.C - Functions dealing with child processes for INIT */
/*                                                               */
/*****************************************************************/
/*                                                               */
/* Written by John Goltz                                         */
/*                                                               */
/*****************************************************************/

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
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <exit.h>
#include <xos.h>
#include <xossvc.h>
#include <xossignal.h>
#include <xostrm.h>
#include <xosmsg.h>
#include <xostime.h>
#include "init.h"

static char sesntext[160] = "----SESNTERM----Session ";

#define SESNTEXT (sesntext + 24)

// Function to service child died interrupt

void childgone(
    struct intdata intdata)

{
    char *str;
    long  code;
	int   len;
    long  status;

///	if (!startupdone)
///		return;

	code = intdata.term >> 24;
    status = intdata.term & 0x00FFFFFFL;
    switch (code)
    {
     case TC_EXIT:						// Process terminated with exit
        len = sprintf(SESNTEXT, "terminated; status = %06.6X", status);
        break;

     case TC_KILL:						// Process killed
        len = sprintf(SESNTEXT, "killed; status = %06.6X", status);
        break;

     case TC_PIPE:						// Process terminated because pipe
										//   had no more input
        len = sprintf(SESNTEXT, "terminated, Pipe out of input data: Status = "
                "%06.6X", status);
        break;

     case TC_BDUSTK:					// Bad user stack address
        len = sprintf(SESNTEXT, "terminated, Illegal user stack pointer;"
                " CS:EIP = %04.4X:%08.8X, EFR = %08.8X"
                " SS:ESP = %04.4X:%08.8X", intdata.pCS&0xFFFF, intdata.pEIP,
                intdata.pEFR, intdata.pseg&0xFFFF, intdata.pos);
        break;

     case TC_NOMEM:						// No memory available
        len = sprintf(SESNTEXT, "terminated, No memory available;"
                " CS:EIP = %04.4X:%08.8X, EFR = %08.8X",
                intdata.pCS&0xFFFF, intdata.pEIP, intdata.pEFR);
        break;

     case TC_UNIMOP:					// Unimplemented operation
        len = sprintf(SESNTEXT, "terminated, Unimplemented operation;"
                " CS:EIP = %04.4X:%08.8X, EFR = %08.8X"
                " Data = %02.2X, %02.2X, %02.2X, %02.2X",
                intdata.pCS&0xFFFF, intdata.pEIP, intdata.pEFR,
                intdata.data&0xFF, (intdata.data>>8)&0xFF,
                (intdata.data>>16)&0xFF, (intdata.data>>24)&0xFF);
        break;

	 case TC_BDCRIT:					// Bad CRIT structure
        len = sprintf(SESNTEXT, "terminated, Invalid CRIT structure;"
                " CS:EIP = %04.4X:%08.8X, EFR = %08.8X"
                " SS:ESP = %04.4X:%08.8X", intdata.pCS&0xFFFF, intdata.pEIP,
                intdata.pEFR, intdata.pseg&0xFFFF, intdata.pos);
		break;

     case TC_NOVECT:					// Uninitialized user vector
        switch ((int)status)
        {
         case VECT_ILLINS:				// Illegal instruction
            str = "Illegal instruction";
            goto novect;

         case VECT_DIVERR:				// Divide error
            str = "Divide error";
            goto novect;

         case VECT_DEBUG:				// Debug trap
            str = "Debug trap";
            goto novect;

         case VECT_BRKPNT:				// Breakpoint interrupt
            str = "Breakpoint interrupt";
            goto novect;

         case VECT_FPPNAVL:				// Coprocessor not available
            str = "Coprocessor not available";
            goto novect;

         case VECT_FPPSOVR:				// Coprocessor error
            str = "Coprocessor error";
            goto novect;

         case VECT_BOUND:				// BOUND instruction
            str = "BOUND instruction trap";
            goto novect;

         case VECT_INTO:				// INTO instruction
            str = "INTO instruction trap";
            goto novect;

         case VECT_PTERM:				// Process terminated
            str = "Process terminated signal";
            goto novect;

         case VECT_CNTC:				// Control-C
            str = "Control-C interrupt";
         novect:
            len = sprintf(SESNTEXT, "terminated, %s; CS:EIP = %04.4X:%08.8X,"
                    " EFR = %08.8X", str, intdata.pCS&0xFFFF,
                    intdata.pEIP, intdata.pEFR);
            break;

         case VECT_PROT:				// Protection fault
            str = "Protection fault";
            goto addrerr;

         case VECT_PAGEFLT:				// Page fault

            str = "Page fault";
            goto addrerr;

         case VECT_SEGNP:				// Segment not present
            str = "Segment not present";
		 addrerr:
            len = sprintf(SESNTEXT, "terminated, %s; CS:EIP = %04.4X:%08.8X,"
                    " EFR = %08.8X, Memory = %04.4X:%08.8X", str,
                    intdata.pCS&0xFFFF, intdata.pEIP, intdata.pEFR,
                    intdata.pseg, intdata.pos);
            break;

         default:
            len = sprintf(SESNTEXT, "terminated, Interrupt for uninitialized "
                    "vector 0x%X; CS:EIP = %04.4X:%08.8X, EFR = %08.8X",
                    status, intdata.pCS&0xFFFF, intdata.pEIP, intdata.pEFR);
            break;
        }
        break;

     default:
        len = sprintf(SESNTEXT, "terminated, Unknown termination type = "
				"0x%02.2X, status = 0x%06.6X", code, status);
        break;
    }
	*(long *)&sesntext[12] = intdata.pid;
	svcSysLog(sesntext, len + (SESNTEXT - sesntext));
}
