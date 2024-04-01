//**************************
// LOGIN - XOS login utility
//**************************
// Written by John Goltz
//**************************

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
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <utility.h>
#include <xos.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xossignal.h>
#include <xosmsg.h>
#include <xostime.h>
#include <xossysp.h>
#include <xosudf.h>
#include <xosvfyuser.h>

#define RETURN1 (URQ1_HISTORY|URQ1_USEREXP|URQ1_PSWDEXP|URQ1_AVLCPU|\
    URQ1_INLCPU|URQ1_AVLMEM|URQ1_INLMEM|URQ1_AVLPRIV|URQ1_INLPRIV|\
    URQ1_HOMEDIR|URQ1_PROGRAM|URQ1_USERDESC|URQ1_USERNAME|URQ1_PASSWORD)
#define RETURN2 (URQ2_AVLSECT|URQ2_INLSECT)


VUDATA vd;

char namebufr[64];
char pswdbufr[64];
char syspassword[64];

struct
{	LNGSTRCHAR password;
	TEXT8CHAR  access;
	char       end;
} pwchar =
{	{PAR_GET|REP_STR , 0, "PASSWORD", syspassword, 64, 64},

	{PAR_GET|REP_TEXT, 4, "ACCESS"}  

};

struct
{	BYTE4PARM cinpmode;
	uchar     end;
} echooff =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMCINPMODE, TIM_ECHO},
};

struct
{	BYTE4PARM sinpmode;
	uchar     end;
} echoon =
{   {PAR_SET|REP_HEXV, 4, IOPAR_TRMSINPMODE, TIM_ECHO},
};

QAB runqab =
{   RFNC_WAIT|RFNC_RUN,			// func    - Function
    0,							// status  - Returned status
    0,							// error   - Error code
    0,							// amount  - Process ID
    0,							// handle  - Device handle
    0,							// vector  - Vector for interrupt
    0, 0, 0,					//         - Reserved
    XR_ALLDEV|XR_SAMEPROC,		// option  - Options or command
    0,							// count   - Count
    NULL,						// buffer1 - Pointer to file spec
    NULL,						// buffer2 - Unused
    NULL						// parm    - Pointer to parameter area
};


void terminate(void);


// Main program

void main(void)
{
	long  rtnval;
    int   retry;
    char *pnt;
    char  chr;
	uchar vuser;
	char  buffer[1000];

	// If we have a system password, verify it

	if ((rtnval = svcIoDevChar(5, (char *)&pwchar)) < 0)
	{
		fputs("? Error obtaining system password - access not allowed\n",
				stdout);

		printf("### %d\n", rtnval);

		terminate();
	}
	vuser = FALSE;
	pnt = syspassword;
	while ((chr = *pnt) != 0)
	{
		if (chr == '#')
		{
			vuser = TRUE;
			*pnt = 0;
			break;
		}
		pnt++;
	}
	fputs("\x1B[2J", stdout);			// Clear the screen
	if (syspassword[0] != 0)
    {
		retry = 3;
		for (;;)
		{
		    svcIoInBlockP(5, NULL, 0, (char *)&echooff);
			fputs("# ", stdout);
			fgets(pswdbufr, 50, stdin);
		    svcIoInBlockP(5, NULL, 0, (char *)&echoon);
			pnt = pswdbufr;
			while ((chr = *pnt) != '\0' && chr != '\n')
				pnt++;
			*pnt = '\0';
			if (strcmp(pswdbufr, syspassword) == 0)
				break;

			if (--retry <= 0)
			{
				fputs("\n? Too many failures - disconnecting\n", stdout);
				terminate();
			}
			fputs("\n? Incorrect - try again\n", stdout);
			svcSchSuspend(NULL, 5*ST_SECOND);
		}
		fputc('\n', stdout);
	}

	// Get and verify the user name and password

	if (vuser)
	{
		retry = 3;
		for (;;)
		{
			fputs("User name: ", stdout);
			fgets(namebufr, 50, stdin);
		    svcIoInBlockP(5, NULL, 0, (char *)&echooff);
			fputs("Password: ", stdout);
			fgets(pswdbufr, 50, stdin);
		    svcIoInBlockP(5, NULL, 0, (char *)&echoon);
			pnt = namebufr;
			while ((chr = *pnt) != '\0' && chr != '\n')
				pnt++;
			*pnt = '\0';
			pnt = pswdbufr;
			while ((chr = *pnt) != '\0' && chr != '\n')
				pnt++;
			*pnt = '\0';
			pwchar.access.value[4] = 0;
			if (verifyuser(namebufr, pwchar.access.value, RETURN1, RETURN2,
					FALSE, &vd, buffer, sizeof(buffer)) &&
					verifypassword(pswdbufr, pnt - pswdbufr, NULL, NULL, &vd))
				break;
			if (--retry <= 0)
			{
				fputs("\n? Too many failures - disconnecting\n", stdout);
				terminate();
			}
			fputs("\n? Incorrect user name or password - try again\n", stdout);
			svcSchSuspend(NULL, 5*ST_SECOND);
		}
	}
	fputc('\n', stdout);
	runqab.buffer1 = (!vuser || vd.proglen == 0 || vd.progpnt == NULL) ?
			"XOSCMD:xshell.run" : vd.progpnt;
    if ((rtnval = svcIoRun(&runqab)) < 0 ||
			(rtnval = runqab.error) < 0)
   	{
		svcSysErrMsg(rtnval, 3, buffer); // Get error message string
		printf("? LOGIN: Cannot load initial program\n         %s\n", buffer);
	}
	terminate();
}


void terminate(void)
{
	svcSchSuspend(NULL, 2*ST_SECOND);
	exit (1);
}
