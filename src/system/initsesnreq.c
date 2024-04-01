/************************************************************/
/*                                                          */
/* initsesnreq.c - Session request message handler for INIT */
/*                                                          */
/************************************************************/
/*                                                          */
/* Written by John Goltz                                    */
/*                                                          */
/************************************************************/

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
#include <procarg.h>
#include "init.h"

char grpname[18];
char usrname[18];
char trmname[18];


static void badoption(void);


//*******************************************************
// Function: sesnreq - Handle the session request message
// Returned: Nothing
//*******************************************************

// This message is sent by the a user program (usually session) to request that
//   a session be created on a terminal. The data, starting with the second
//   byte in the message is simply the command line from session, which has
//   the following format:
//		/grp=group /usr=user trmname progname arg1 arg2 ...
//   The /grp and /usr options are optional and can appear in any order as long
//   as they are before the progname argument. Progname is also optional. If
//   not present, "nshell" is used. There may be 0 or more argN arguments.

// If this message is received before startup is complete, it is queued and
//   processed when startup is complete.

void sessionreq(
	uchar *msg,				// Address of message
	int    len)				// Length of message
{
	SESNREQ *spnt;
	char    *next;
	char    *tok;
	char    *tail;
	char    *pnt;
	long     rtn;
	uchar    hvgrp;

	msg[len] = 0;
	msg[len + 1] = 0;
	tail = NULL;
	trmname[0] = 0;
	strcpy(grpname, "user");
	strcpy(usrname, "user");
	next = msg;
	while ((tok = strtok(next, " \t")) != NULL)
	{
		next = NULL;
		if (tok[0] == '/')
		{
			if ((hvgrp = strnicmp(tok + 1, "grp", 3)) == 0 ||
					strnicmp(tok + 1, "usr", 3) == 0)
			{
				if (tok[4] != 0 && tok[4] != '=')
				{
					badoption();
					return;
				}
				if (tok[4] == 0)
				{
					if ((tok = strtok(NULL, " \t")) == NULL || tok[0] != '=')
					{
						badoption();
						return;
					}
					tok++;
				}
				if (tok[4] == 0)
				{
					if ((tok = strtok(NULL, " \t")) == NULL)
					{
						badoption();
						return;
					}
				}
				if ((len = strlen(tok)) > 16)
				{
					badoption();
					return;
				}
				strmov((hvgrp) ? grpname : usrname, tok);
			}
		}
		else
		{
			if (trmname[0] == 0)
			{
				if ((len = strlen(tok)) > 16)
				{
					response("\7? ", "Terminal name is too long");
					return;
				}
				pnt = strmov(trmname, tok);
				if (pnt[-1] != ':')
				{
					pnt[0] = ':';
					pnt[1] = 0;
				}
			}
			else
			{
				tail = tok;
				tok += strlen(tok);
				if (tok[1] != 0)
					tok[0] = ' ';
				break;
			}
		}
	}
	if (tail == NULL)
	{
		response("\7 ", "No program specified");
		return;
	}
	if (!startdone)
	{
		if ((spnt = malloc(strlen(tail) + sizeof(SESNREQ))) != NULL)
		{
			strmov(spnt->grp, grpname);
			strmov(spnt->usr, usrname);
			strmov(spnt->trm, trmname);
			strmov(spnt->cmd, tail);
			spnt->next = NULL;
			*sesnppnt = spnt;
			sprintf(msg + 128, "Session request for %s has been queued",
					trmname);
			response("\5", msg + 128);
		}
	}
	else
	{
		if ((rtn = makesession(trmname, tail, grpname, usrname)) < 0)
		{
	        svcSysErrMsg(rtn, 3, msg + 128); // Get error message string
    	    response("\7? ", msg + 128);	// And complain
		}
		else
		{
			sprintf(msg + 128, "Session has been created on %s", trmname);
			response("\5", msg + 128);
		}
	}
}


static void badoption(void)
{
	response("\7? ", "Invalid option specified");
}
