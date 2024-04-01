//============================================
// cmdstatus.c
// Written by John Goltz
//============================================

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

#include "usbctl.h"


static long  unit;
static long  address;
static long  port;
static int   indent;


static int   fncstsaddr(arg_data *arg);
static int   fncstsport(arg_data *arg);
///static void showdev(DDB *ddb, char *bufr, long ifnum, char *devlst,
///		char *devtxt, long error);
static void stslevel(DDB *ddbtbl[], char *bufr);


static arg_spec keywordsts[] =
{	{"UNIT"   , ASF_VALREQ|ASF_NVAL , NULL, fncunit, 0},
	{"UNI"    , ASF_VALREQ|ASF_NVAL , NULL, fncunit, 0},
	{"ADDRESS", ASF_VALREQ|ASF_NVAL , NULL, fncstsaddr, 0},
	{"ADD"    , ASF_VALREQ|ASF_NVAL , NULL, fncstsaddr, 0},
	{"PORT"   , ASF_VALREQ|ASF_NVAL , NULL, fncstsport, 0},
	{"POR"    , ASF_VALREQ|ASF_NVAL , NULL, fncstsport, 0},
	{0}
};


//*************************************************
// Function: cmdstatus - Process the STATUS command
// Returned: Nothing
//*************************************************

void cmdstatus(
    char *cmd,
	int   len)
{
///	char *bufr;
///	CDB **tpnt;
///	CDB  *cdb;
///	DDB  *ddb;
///	CFG  *cfg;
///	IFC  *ifc;
///	IFD  *ifd;
///	EPT  *ept;
///	int   cnt;
///	char  text[32];
	char  bufr1[512];

	unit = -1;
	address = -1;
	port = -1;
    cmdpntr[0] = cmd;
    if (!procarg((cchar **)cmdpntr, PAF_INDIRECT|PAF_EATQUOTE, NULL, keywordsts,
			NULL, procargerror, NULL, NULL))
		return;
    sfwResponse(0, SFWRSP_SEND|SFWRSP_PREFIX, "%d.%d.%d USB status:", srvmajver,
			srvminver, srveditnum);
	indent = 0;
	stslevel(&roottbl, bufr1);
	sfwResponse(0, SFWRSP_SEND|SFWRSP_FINAL, "Finished");
}


static void stslevel(
	DDB  *ddbtbl[],
	char *bufr)
{
	DDB  *ddb;
	IFD  *ifd;
	int   port;
	int   len;
	char  text[32];
	char  etxt[100];

	port = 1;
	do
	{
		if ((ddb = ddbtbl[port]) != NULL)
		{
			if (ddb->addr != 0)
				sprintf(text, "(%d)", ddb->addr);
			else
				text[0] = 0;
			sfwResponse(0, SFWRSP_SEND, "%08X %*s%d:%s %s", ddb->gport, indent,
					"", ddb->lport, text, (ddb->devtxt == NULL) ? "" :
					ddb->devtxt);
			if (ddb->error < 0)
			{
				svcSysErrMsg(ddb->error, 0x03, bufr + sprintf(bufr,
						STR_MT_INTRMDMSG"%*s", indent + 11, ""));
				sfwResponse(0, SFWRSP_SEND, bufr);
			}
			ifd = ddb->fifd;
			while (ifd != NULL)
			{
				if (ifd->error < 0 && ifd->error != ER_USBBC)
					svcSysErrMsg(ifd->error, 0x03, etxt);
				else
					etxt[0] = 0;
				sprintf(bufr, STR_MT_INTRMDMSG"%*sIF%d: %s %s",
						indent + 11, "", ifd->number,
						(ifd->devlst == NULL) ? "" : ifd->devlst,
						(ifd->devtxt == NULL) ? "" : ifd->devtxt);
				if (etxt[0] != 0)
				{
					if (ifd->devlst == NULL && ifd->devlst == NULL)
					{
						len = strlen(bufr);
						strmov(bufr + len - 1, etxt);
					}
					else
					{
						sfwResponse(0, SFWRSP_SEND, bufr);
						sprintf(bufr, STR_MT_INTRMDMSG"%*s%s", indent + 16,
								"", etxt);
					}
				}
				sfwResponse(0, SFWRSP_SEND, bufr);
				ifd = ifd->next;
			}
			if (ddb->hdb != NULL)
			{
				indent += 2;
				stslevel(&ddb->cdb->ddbtbl, bufr);
				indent -= 2;
			}
		}
	} while (++port <= 15);
}


/*
static void showdev(
	DDB  *ddb,
	char *bufr,
	long  ifnum,
	char *devlst,
	char *devtxt,
	long  error)
{
	char *pnt;
	int   cnt;
	char  text[32];

	if (ddb->addr != 0)
		sprintf(text, "%d-%d", ddb->addr, ifnum);
	else
		text[0] = 0;
	sprintf(bufr, STR_MT_INTRMDMSG"%08X %*s%d:%s %s %s", ddb->gport, indent, "",
			ddb->lport, text, devlst, devtxt);
	srvCmdResponse(bufr, srvMsgDst);
	if (error < 0)
	{
		pnt = strmov(bufr, STR_MT_INTRMDMSG);
		cnt = indent + 11;
		do
		{
			*pnt++ = ' ';
		} while (--cnt > 0);
		svcSysErrMsg(error, 0x03, pnt);
		srvCmdResponse(bufr, srvMsgDst);
	}
}
*/

//**********************************************************************
// Function: fncstsaddr - Process ADDRESS keyword for the STATUS command
// Returned: TRUE if OK, FALSE if error
//**********************************************************************

static int fncstsaddr(
    arg_data *arg)
{
    address = arg->val.n;
    return (TRUE);
}


//*******************************************************************
// Function: fncstsport - Process PORT keyword for the STATUS command
// Returned: TRUE if OK, FALSE if error
//*******************************************************************

static int fncstsport(
    arg_data *arg)
{
    port = arg->val.n;
    return (TRUE);
}
