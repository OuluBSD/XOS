/*--------------------------------------------------------------------------*
 * session.c
 * Command to create a session on a terminal
 *
 * Written by: John R. Goltz
 *
 * Edit History:
 *-------------------------------------------------------------------------*/

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

// Command format:
//	 session {/grp=grpname} {/usr=usrname} trmname {progname {arguments {....}}}
// Everything is passed directly to init

// This program must be linked with _mainalt which does not parse the command
//   line into separate items!

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <xcstring.h>

char  prgname[] = "SESSION";
long  ipmhandle;
int   exitsts;
char  mbufr[256];

char sysinit[] = "SYS^INIT";

struct
{   BYTE4PARM  timeout;
    LNGSTRPARM dstname;
    char       end;
} outparm =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT   , 10*ST_SECOND},
    {PAR_SET|REP_STR , 0, IOPAR_MSGRMTADDRS, sysinit, 8}
};

struct
{   BYTE4PARM timeout;
    char      end;
} inpparm =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT, 10*ST_SECOND}
};

void mainalt(
    img_data *argp)
{
    char *argpnt;
	char *pnt;
    char *symmsg;
    long  rtn;
    char  chr;

    argpnt = argp->cmdtail;
    while (((chr=*argpnt) != '\0') && !isspace(chr)) // Advance to first
        ++argpnt;								     //   whitespace character
    while (((chr=*argpnt) != '\0') && isspace(chr)) // Skip whitespace
        ++argpnt;
    if (chr == '\0')
    {
        fputs("? SESSION: Command error, correct usage is:\n"
///				"             SESSION {/grp=grpnm} {/usr=usrnm} trmnm {prognm "
				"             SESSION trmnm {prognm "
				"{args {...}}}", stderr);
        exit(1);
    }
	if ((ipmhandle=svcIoOpen(XO_IN|XO_OUT, "IPM:", NULL)) < 0)
		femsg2(prgname, "Cannot open message device IPM:", ipmhandle, NULL);

	pnt = argpnt;
	while (((chr=*pnt) != '\0') && !isspace(chr)) // Advance to next
		++pnt;									  //   whitespace character
	while (((chr=*pnt) != '\0') && isspace(chr)) // Skip whitespace
		++pnt;
	if (chr == 0)
		strmov(pnt, " XOSCMD:xshell.run");
    symmsg = argpnt - 1;				// Get address of our message
    *symmsg = MT_SESNREQ;				// Store message type
    if ((rtn=svcIoOutBlockP(ipmhandle, symmsg, strlen(symmsg),
			(char *)&outparm)) < 0)
        femsg2(prgname, "Error sending message to INIT", rtn, NULL);
    for (;;)
    {
        if ((rtn=svcIoInBlockP(ipmhandle, mbufr, sizeof(mbufr),
				(char *)&inpparm)) < 0)
            femsg2(prgname, "Error receiving response from INIT", rtn, NULL);
        if (mbufr[0] > 7)				// Valid message type?
        {
            fprintf(stderr, "? SESSION: Illegal message received, type = %d\n",
                    mbufr[0]);
            exit(1);
        }
        mbufr[(int)rtn] = '\0';			// Put null at end of message
        if ((mbufr[0] & 3) == 3)
        {
            exitsts |= 1;
            fprintf(stderr, "%s\n", &mbufr[1]);
        }
        else
            printf("%s\n", &mbufr[1]);
        if (mbufr[0] > 3)				// Are we finished?
            exit (exitsts);
    }
}
