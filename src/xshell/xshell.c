//------------------------------------------------------------------------------
// xshell.c - Command decoder for XOS
// 
// Written by John R. Goltz and Bruce R. Nevins
// 
// Edit History:
// -------------
//------------------------------------------------------------------------------*

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

#include "xshell.h"

// Main program

long  childflag;

CDATA childdata;

CMDST *ccs;

char  cmdbfr[CMDMAX];

QAB inpqab =
{   QFNC_WAIT|QFNC_DIO|QFNC_INBLOCK,
						// open
    0,					// status
    0,					// error
    0,					// amount
    0,					// handle
    0,					// vector
    {0},
	0,					// option
    sizeof(cmdbfr),		// count
    cmdbfr,				// buffer1
    NULL,				// buffer2
    NULL				// parm
};

int   majversion = 4;
int   minversion = 1;
char *builddate = __DATE__;

char *applist;

uchar flags;

uchar batch = FALSE;
uchar echo;
uchar xosarg = FALSE;

char prgname[] = "XSHELL";


static int cmdlvl2(int len);


void mainalt(
    img_data *args)

{
	CMDST cmdst;
	char const *epnt;
	char *pnt;
	int   len;
	char  chr;
	char  single;
	char  noini;

    setvector(VECT_HNGUP, 4, hungupsignal); // Setup to take hung-up signal
    setvector(VECT_PTERM, 4, childsignal); // Setup vector for chil process
										   //   terminated signal
    setvector(VECT_CNTC, 4, cntlcsignal); // Setup cntl-C signal vector

///	resetscrn();						// Set screen to known state

	global_parameter(TRUE);				// Set up global parameters
    svcSchSetLevel(0);					// Enable software interrupts
	flags &= ~FLG_HOLDCC;
	cmdst.cmdout = cmdst.cmderr = DH_STDOUT;
	cmdst.prev = NULL;
	cmdst.cmdin = stdin;
	cmdst.datain = stdin;
	cmdst.batch = FALSE;
	cmdst.echo = TRUE;
	cmdst.args = NULL;
	ccs = &cmdst;
	findapps();							// Find all of the application
										//   directories

	// See if we have an initial command specified

	single = FALSE;
	if (args->cmdtailsz > 0)
	{
		pnt = args->cmdtail;			// Get the start of the first argument
		while ((chr = *pnt++) != 0 && !isspace(chr))
			;
		while ((chr = *pnt) != 0 && isspace(chr))
			pnt++;
	}
	else
		pnt = "";
	single = *pnt;						// Assume single if have an argument
	noini = FALSE;						// Assume want to execut the shellini
										//   file
	while (pnt[0] != 0)					// Handle any special prefix character
	{									//   that are present
		if (*pnt == '+')
			single = FALSE;
		else if (*pnt == '!')
			noini = TRUE;
		else
			break;

		pnt++;
	}
	if (!noini)							// Execute the "shellini" batch file
	{									//   if we should
	    len = strnmov(cmdbfr, (svcSysGetEnv("SHELLINI", &epnt) > 0) ? epnt :
				"HOME:shellini.bat", CMDMAX - 1) - cmdbfr;
		cmdbfr[CMDMAX - 1] = 0;
		cmdlvl2(len);
	}
	if (*pnt != 0)						// If had an argument, execute it as
	{									//   if typed in
		len = strnmov(cmdbfr, pnt, CMDMAX - 1) - cmdbfr;
		cmdbfr[CMDMAX -1] = 0;
		single |= cmdlvl2(len);
	}
	if (!single)						// If not doing a single command, do
		cmdlevel();						//   commands until asked to terminate
	exit(0);
}


//**************************************
// Function: cmdlevel - Process commands
// Returned: Nothing
//**************************************

// This is called recusively to process batch files.

void cmdlevel(void)
{
	int  len;
	char chr;

///	svcIoOutBlock(ccs->cmderr, "### LEVEL++\r\n", 13);

	len = strlen(cmdbfr);
	while (TRUE)
	{
		if (ccs->batch)
		{
			if (flags & FLG_HAVECC)
				break;
			if (fgets(cmdbfr, sizeof(cmdbfr) - 2, ccs->cmdin) == NULL)
			{
				if ((len = -errno) >= 0)
					len = ER_EOF;
				break;
			}
			if (flags & FLG_HAVECC)
				break;
			len = strlen(cmdbfr);
		}
		else
		{
			showprompt();
			while (TRUE)
			{
				cmdbfr[0] = 0;
				inpqab.func = QFNC_WAIT|QFNC_DIO|QFNC_INBLOCK;
				inpqab.handle = ccs->cmdin->_handle;
				flags &= ~FLG_HOLDCC;
				if (!(flags & FLG_HAVECC))
				{
///					len = svcIoInBlock(inpqab.handle, cmdbfr, sizeof(cmdbfr));


					if ((len = svcIoQueue(&inpqab)) >= 0)
						len = inpqab.amount;

				}
				flags |= FLG_HOLDCC;
				if (flags & FLG_HAVECC)
					showprompt();
				else
				{
					if (len < 0)
					{
						svcIoOutBlock(ccs->cmderr, "\r\n", 2);
						cmnderror(len, "Error reading command input");
						break;
					}
					if (len < 4 || cmdbfr[len - 4] != 0)
						break;
					else
					{
						if ((chr = cmdbfr[len - 3]) == ISC_UPARW ||
								chr == ISC_KPUPARW)
							prevfromhistory();
						else if (chr == ISC_DNARW || chr == ISC_KPDNARW)
							nextfromhistory();
						else
						{
							svcIoOutBlock(ccs->cmderr, "\r\n", 2);
							cmnderror(0, "\nUnreconized special key");
							showprompt();
						}
					}
				}
			}
		}
		if (len != 0 && cmdlvl2(len))
			break;
	}
///	svcIoOutBlock(ccs->cmderr, "### LEVEL--\r\n", 13);
	ccs = ccs->prev;
}


static int cmdlvl2(
    int len)
{
	char chr;

	while (len >= 1 && ((chr = cmdbfr[len - 1]) == '\r' ||
			chr == '\n'))
		len--;
	if (ccs->batch && ccs->echo && cmdbfr[0] != '@')
	{
		showprompt();
		cmdbfr[len] = '\n';
		svcIoOutBlock(ccs->cmdout, cmdbfr, len + 1);
	}
	cmdbfr[len] = 0;
	len = docmd(cmdbfr);
	flags &= ~FLG_HOLDCC;
	return (len);
}
