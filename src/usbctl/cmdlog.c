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


void cmdlog(
    char *cmd,
	int   len)
{
	time_s dt;
	char  *pnt;
	char   bufr[180];

	if (len > 124)
	{
		sfwResponse(ER_BDSPC, SFWRSP_SEND|SFWRSP_PREFIX|SFWRSP_FINAL, 
				"Log file name is too long");
		return;
	}
	svcSysDateTime(T_GTSYSDTTM, &dt);
	sdt2str(bufr, "%H:%m:%s on %D-%3n-%y", (time_sz *)&dt);
	if (debugfile != NULL)
	{
		debugprint("| ");
		debugprint("|***************************************************");
		debugprint("*   Logging terminated at %s   *\n"
				"***************************************************", bufr);
		fclose(debugfile);
		debugfile = NULL;
		debugflg = FALSE;
	}
	if (cmd[0] != 0)
	{
		if ((debugfile = fopen(cmd, "a+")) == NULL)
		{
			sfwResponse(-errno, SFWRSP_SEND|SFWRSP_PREFIX|SFWRSP_FINAL,
					"Cannot open log file %.90s", cmd);
			return;
		}
		debugflg = TRUE;
		strcpy(debugname, cmd);
		if (debugdefer == NULL)
		{
			debugprint("| ");
			debugprint("|************************************************");
			debugprint("*   Logging started at %s   *\n"
					"************************************************\n ",
					bufr);
		}
		else
		{
			pnt = debugdefer;
			debugdefer = NULL;
			if (pnt != NULL)
			{
				if (fwrite(debugbase, 1, pnt - debugbase, debugfile) < 0)
					sfwResponse(-errno, SFWRSP_SEND, "Error writing deferred "
							"entries to log file");
				debugprint("***** End of deferred log entries\n");
			}
			svcMemChange(debugbase, 0, 0);
		}
		sfwResponse(0, SFWRSP_SEND|SFWRSP_PREFIX|SFWRSP_FINAL,
				"Now logging to %s", cmd);
	}
	else
		sfwResponse(0, SFWRSP_SEND|SFWRSP_PREFIX|SFWRSP_FINAL,
				"Logging is now disabled");
}
