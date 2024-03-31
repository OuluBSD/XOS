//============================================
// cmdretry.c
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

static long indent;


static void retrylevel(DDB *ddbtbl[], char *bufr);


void cmdretry(
    char *cmd,
	int   len)
{
	char bufr1[256];

	(void)cmd;
	(void)len;

	sfwResponse(0, SFWRSP_SEND|SFWRSP_PREFIX, "%d.%d.%d - Retrying devices:",
			srvmajver, srvminver, srveditnum);
	retrylevel(&roottbl, bufr1);
	sfwResponse(0, SFWRSP_SEND|SFWRSP_FINAL, "Done");
}


static void retrylevel(
	DDB  *ddbtbl[],
	char *bufr)
{
	DDB  *ddb;
	HDB  *hdb;
	IFD  *ifd;
	long  sdata[5];
	long  rtn;
	int   port;
	char  etxt[16];

	indent = 0;
	port = 1;
	do
	{
		if ((ddb = ddbtbl[port]) != NULL)
		{
			if ((hdb = ddb->owner.hdb) != NULL)
			{
				if (ddb->error < 0)		// Device level error?
				{
					svcSysErrMsg(ddb->error, 0x02, etxt);
					sprintf(bufr, "Retrying USB device %08X (Error was %s)",
							ddb->gport, etxt);
					sfwResponse(ddb->error, SFWRSP_SEND, bufr);
					debugprint(bufr);
					sdata[0] = (long)ddb->rootcdb;
					sdata[1] = ddb->gport;
					sdata[2] = ddb->seqnum;
					sdata[3] = -1;
					sdata[4] = 0;
					if ((rtn = svcSchReqSignal(ddb->rootcdb->vector + 2, sdata,
							5, -1)) < 0)
					{
						sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND, "Error "
								"requesting retry signal for %08X", ddb->gport);
						debugprint("Error requesting retry signal for %08X: "
								"%d", ddb->gport, rtn);
					}
				}
				else
				{
					ifd = ddb->fifd;
					while (ifd != NULL)
					{
						if (ifd->error < 0)
						{
							svcSysErrMsg(ifd->error, 0x02, etxt);
							sprintf(bufr, "Retrying interface %08X-%d (Error "
									"was %s) %d DDB:%08X IFD:%08X", ddb->gport,
									ifd->number, etxt, ifd->seqnum, ddb, ifd);
							sfwResponse(0, SFWRSP_SEND, bufr);
							debugprint(bufr);

							sdata[0] = (long)ddb->rootcdb;
							sdata[1] = ddb->gport;
							sdata[2] = ddb->seqnum;
							sdata[3] = ifd->number;
							sdata[4] = ifd->seqnum;
							if ((rtn = svcSchReqSignal(ddb->rootcdb->vector + 2,
									sdata, 5, -1)) < 0)
							{
								sfwResponse(rtn, SFWRSP_LOG|SFWRSP_SEND,
										"Error requesting retry signal for "
										"%08X", ddb->gport);
								debugprint("Error requesting retry signal for "
										"%08X: %d", ddb->gport, rtn);
							}
						}
						ifd = ifd->next;
					}
				}
			}
			else if (ddb->hdb != NULL)
			{
				indent += 2;
				retrylevel(ddb->hdb->ddbtbl, bufr);
				indent -= 2;
			}
		}
	} while (++port <= 15);
}
