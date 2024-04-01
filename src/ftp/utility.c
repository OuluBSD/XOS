//--------------------------------------------------------------------------*
// UTILITY.C - XOS FTP server utility routines
//
// Written by: John Goltz
//--------------------------------------------------------------------------*

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

#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <xosnet.h>
#include <procarg.h>
#include <xosthreads.h>
#include <xosservert.h>
#include <xosstr.h>
#include "ftpsrv.h"


static struct
{	byte4_parm timeout;
	uchar      end;
}      cmdoutparms =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT, 30 * ST_SECOND}
};


static int get2dig(char **ppnt, ushort *val);


//*********************************************
// Function: xoserror - Return command response
//				which includes XOS error text
// Returned: Nothing
//*********************************************

void xoserror(
	CDB   *cdb,
	long   code,
	char  *txt)

{
	cstr msg;
	char bufr[128];
	char errmsg[84];

	svcSysErrMsg(code, 0x01, errmsg);
	msg.str = bufr;
	msg.len = sprintf(bufr, "%s - %s\r\n", txt, errmsg);
	cmdresponse(cdb, msg);
}


//************************************************
// Function: cmdresponse - Return command response
// Returned: Nothing
//************************************************

void cmdresponse(
	CDB   *cdb,
	cstr   msg)

{
	long rtn;

	if (srvDebugLevel > 0)
		debug(cdb->slot, "RSP", msg.len - 2, msg.str);
	if ((rtn = thdIoOutBlockP(cdb->cmdhndl, msg.str, msg.len,
			&cmdoutparms)) < 0)
	{
		if (rtn != ER_NCCLR && rtn != ER_NCLST && rtn != ER_NORSP &&
				rtn != ER_ABORT && rtn != ER_CANCL)
			logerror(cdb->slot, rtn, "Error writing to client connection");
		cdb->terminate = TRUE;
	}
	if (cdb->terminate)
		terminate(cdb);
}


void tellworkingdir(
	CDB *cdb)

{
	cstr  msg;
	char *pnt;
	char  bufr[600];
	char  spec[512];

	pnt = (cdb->allfiles == 2 || cdb->homdirstr == NULL) ? spec :
			strnmov(spec, cdb->homdirstr, 512);
	if (cdb->curdirstr != NULL)
		pnt = strnmov(pnt, cdb->curdirstr, sizeof(spec) -  2 - (pnt - spec));
	if (pnt == spec)
		*pnt++ = '/';
	*pnt = 0;
	msg.str = bufr;
	msg.len = sprintf(bufr, "200 \"%s\" is the working directory\r\n", spec);
	cmdresponse(cdb, msg);
}


int getbyteval(
	char **ppnt,
	uchar *val,
	int    stopper)

{
	char *pnt;
	long  value;
	char  chr;

	pnt = *ppnt;
	while ((chr = *pnt) != 0 && isspace(chr))
		pnt++;
	value = 0;
	while ((chr = *pnt) != 0 && isdigit(chr))
	{
		pnt++;
		value = value * 10 + (chr & 0x0F);
	}
	pnt++;
	while (chr != 0 && isspace(chr))
	{
		pnt++;
		chr = *pnt;
	}
	*ppnt = pnt;
	*val = (uchar)value;
	return (chr == stopper);
}


void getatom(
	char **ppnt,
	char  *atom,
	int    len)

{
	char *pnt;
	char  chr;

	pnt = *ppnt;

	while ((chr = *pnt) != 0 && isspace(chr))
		pnt++;
	while ((chr = *pnt) != 0 && !isspace(chr))
	{
		pnt++;
		if (--len > 2)
			*atom++ = chr;
	}
	*atom = 0;
	while ((chr = *pnt) != 0 && isspace(chr))
		pnt++;
	*ppnt = pnt;
}


int requireend(
	CDB  *cdb,
	char *pnt)

{
	char  chr;

	while ((chr = *pnt) != 0 && isspace(chr))
		pnt++;
	if (chr != 0)
	{
		cmdresponse(cdb, msg500b);
		return (FALSE);
	}
	return (TRUE);
}


//*******************************************************
// Function: checkpriv - Check for a priviledge
// Returned: TRUE if user has priviledge, FALSE otherwise
//*******************************************************

// Priviledge strings are stored tighly packed (no nulls) with the high
//   order bit set in the last character of each string. There is a final
//   null terminating the data.

int checkpriv(
	char *data,
	char *name)

{
	if (data != NULL)					// Do we have any data?
	{									// Yes
		while (*data != 0)
		{
			while (*data == *name)		// Compare until different or end
			{
				if (*data < 0)			// End of both strings?
					return (TRUE);		// Yes - found it
				data++;					// No - continue
				name++;
			}
			while (*name++ > 0)			// Not a match - advance to end of
				;						//   data string
		}
	}
	return (FALSE);
}


int getdirspec(
	CDB  *cdb,
	char *pnt,
	char *buffer,
	int   length)

{
	int status;
	int len;

	if ((len = getfilespec(cdb, &pnt, buffer, length, &status)) < 0)
		return (-1);
	if (status & 0x02)				// Have any wildcards?
	{								// Yes - fail
		cmdresponse(cdb, msg501a);
		return (-1);
	}
	if (!requireend(cdb, pnt))
		return (-1);
	if (len == 1 && buffer[0] == '/')
			len = 0;

	if (len > 0 && buffer[len - 1] != '/')
	{
		buffer[len] = '/';
		len++;
	}
	buffer[len] = 0;
	return (len);
}


//*******************************************************
// Function: getfilespec - Collect file specification
// Returned: Length of the string returned if normal or
//				-1 if error (error response already sent)
//*******************************************************

int getfilespec(
	CDB   *cdb,
	char **ppnt,
	char  *buffer,
	int    length,
	int   *status)				// Bit 0 = 1 if device specified
								// Bit 1 = 1 if wild card name

{
	char *pnt;
	char *dpnt;
	char *spnt;
	char *bpnt;
	char *begin;
	char *bpath;
	char *name;
	int   len;
	int   sts;
	char  chr;

	pnt = *ppnt;
	sts = 0;

	// First scan enough of the file specification to see if we have a
	//   relative or absolute specification.  It is absolute if it begins
	//   with a \ or if it contains a device specification.  We do not allow
	//   relative specifications with a device because of the conflict between
	//   the FTP and DOS ideas of working directories.  Note that device names
	//   are only allowed when in all-files mode.

	begin = pnt;
	while ((chr = *pnt++) != 0 && chr != '\\' && chr != '/' &&
			chr != ':' && !isspace(chr))
		;

	bpnt = buffer;
	if (chr == ':')						// Do we have a device name?
	{
		len = pnt - begin;				// Get length of device name
		if (cdb->allfiles != 2 || (length -= len) <= 0)
		{
			cmdresponse(cdb, msg501a);
			return (-1);
		}
		memcpy(buffer, begin, len);
		buffer[len] = '/';
		strupr(buffer);
		if ((chr = *pnt) == '\\' || chr == '/') // is next character a / or \?
			pnt++;						// Yes - discard it (we already put
		sts |= 0x01;					//   a / in the buffer)
		begin = pnt;
		bpnt += (len + 1);
	}
	else
		pnt = begin;

	bpath = bpnt;

	if ((chr = *pnt) != '\\' && chr != '/' && !sts)
	{
		// Here if the path is relative - first copy the current path

		if ((length -= cdb->curdirlen) <= 0)
		{
			cmdresponse(cdb, msg501a);
			return (-1);
		}
		memcpy(bpnt, cdb->curdirstr, cdb->curdirlen);
		bpnt += cdb->curdirlen;
	}
	while ((chr = *pnt) != 0 && !isspace(chr))
	{
		if (chr == '\\')
			chr = '/';
		pnt++;
		*bpnt++ = chr;
	}
	*ppnt = pnt;
	*bpnt = 0;

	// We how have the complete path string in the buffer. This string may
	//   contain ".."s which we now must remove.  We also remove any duplicated
	//   "/"s.

	dpnt = spnt = bpath;
	if (*spnt == '/' && cdb->allfiles != 2)
		spnt++;
	while (*spnt != 0)
	{
		if ((chr = *spnt) == '/')		// Skip any leading "/"s
		{
			spnt++;
			*dpnt++ = chr;
			while (*spnt == '/')
				spnt++;
		}
		name = dpnt;
		while ((chr = *spnt) != 0 && chr != '/')
		{
			spnt++;
			*dpnt++ = chr;
		}

		len = dpnt - name;
		if (dpnt == name)				// At end now?
			break;						// Yes

		// Here with a directory name element

		if (name[0] == '.')
		{
			if ((len = dpnt - name) == 2 && name[1] == '.')
			{
				// Have a ".." name element - scan backwards in the buffer to
				//   the beginning of the previous name element, checking for
				//   the beginning of the buffer.

				if ((dpnt -= 4) < bpath)
				{
					cmdresponse(cdb, msg501a);
					return (-1);
				}
				while (dpnt > bpath && *dpnt != '/')
					dpnt--;
				if (*dpnt == '/')
					dpnt++;
				if (*spnt == '/')
					spnt++;
			}
			else if (len == 1)			// if have just ".", just remove it
				dpnt -= 2;
		}
	}
	*dpnt = 0;

	// Finally, we see if there are any wild card characters. These are
	//   allowable only after the final "/".

	spnt = dpnt;

	while (spnt >= buffer && (chr = *spnt--) != '/' && chr != ':')
	{
		if (chr == '*' || chr == '?')
			sts |= 0x02;
	}
	while (spnt >= buffer)
	{
		if ((chr = *spnt--) == '*' || chr == '?')
		{
			cmdresponse(cdb, msg501a);
			return (-1);
		}
	}
	*status = sts;
	return (dpnt - buffer);
}


//******************************************************
// Function: getfiledatetime - Get file date and time
//				for STOR type commands
// Returned: TRUE if OK, FALSE if syntax error (response
//				has been returned)
//******************************************************

// We expect to have a date/time value in the following format:
//    (yyyymmddhhmmssxzhzm)
//   Where:
//     yyyy = year (4 digits)
//     mm   = month (01-12)
//     dd   = day of the month
//     hh   = hours (24-hour time)
//     mm   = minutes
//     ss   = seconds
//     X    = time zone offset sign (+ or -)
//     hz   = time zone offset hours
//     zm   = time zone offset minutes
//   All values must be numeric with leading zeros.  The time zone offset is
//   optional.  If it is completely missing this indicates that the time zone
//   offset is not known.  If only a + is present, this indicates that the time
//   is GMT.  Any variation from this format is reported as a syntax error.

int getfiledatetime(
	CDB   *cdb,
	char **ppnt)

{
	time_x dtx;
	ushort tzh;
	ushort tzm;
	ushort year;
	char  *pnt;
	char   chr;

	pnt = *ppnt;
	cdb->filedt.high = 0;
	while ((chr = *pnt) != 0 && isspace(chr))
		pnt++;
	if (*pnt++ != '(' || !get2dig(&pnt, &year) ||
			!get2dig(&pnt, &(dtx.dos.tmx_year)) ||
			!get2dig(&pnt, &(dtx.dos.tmx_mon)) ||
			!get2dig(&pnt, &(dtx.dos.tmx_mday)) ||
			!get2dig(&pnt, &(dtx.dos.tmx_hour)) ||
			!get2dig(&pnt, &(dtx.dos.tmx_min)) ||
			!get2dig(&pnt, &(dtx.dos.tmx_sec)))
		return (TRUE);
	dtx.dos.tmx_year += (year * 100);
	if ((chr = *pnt++) == '+' || chr == '-')
	{
		if (!get2dig(&pnt, &tzh) || !get2dig(&pnt, &tzm))
			return (TRUE);
	}
	if (*pnt++ != ')')
		return (TRUE);
	svcSysDateTime(T_CVDOS2SYS, &dtx);
	cdb->filedt.high = dtx.sys.high;
	cdb->filedt.low = dtx.sys.low;
	*ppnt = pnt;
	return (TRUE);
}


static int get2dig(
	char  **ppnt,
	ushort *val)

{
	char chr1;
	char chr2;

	chr1 = (*ppnt)[0];
	chr2 = (*ppnt)[1];
	if (!isdigit(chr1) || !isdigit(chr2))
		return (FALSE);
	*val = (chr2 & 0x0F) + (chr1 & 0x0F) * 10;
	(*ppnt) += 2;
	return (TRUE);
}


//***********************************************************************
// Function: datafunc - Send a function request to the client data thread
// Returned: Nothing
//***********************************************************************

void datafunc(
	CDB *cdb,
	int  func)

{
	if (cdb->datafunc != 0)
	{
		logerror(cdb->slot, 0, "Data thread is not idle");
		return;
	}
	cdb->datadone = FALSE;
	cdb->datafunc = func;
	thdCtlWake(cdb->dtdb);
}


//***************************************************************
// Function: datawait - Wait until client data thread is finished
// Returned: Nothing
//***************************************************************

void datawait(
	CDB *cdb)

{
	while (cdb->dtdb != NULL && !(cdb->datadone))
		thdCtlSuspendT(NULL, -1, -1);
	if (cdb->terminate)
		terminate(cdb);
}


//****************************************************************
// Function: dataabort - Abort current client data thread function
// Returned: Nothing
//****************************************************************

void dataabort(
	CDB *cdb)

{
	cdb->dataabort = TRUE;
	if (cdb->pasvhndl > 0)
	{
		thdIoClose(cdb->pasvhndl, 0);
		cdb->pasvhndl = -1;
	}
	if (!(cdb->datadone) && cdb->datahndl > 0)
		canceldevice(cdb->datahndl);
	if (cdb->datacon == DATACON_CONNECTING)
		svcIoCancel(&(((THDDATA *)(cdb->dtdb))->qab), CAN_OPEN);
	datafunc(cdb, DATAFUNC_ABORT);
	datawait(cdb);
}


void canceldevice(
	long hndl)

{
	QAB cqab;

	memset(&cqab, 0, sizeof(cqab));
	cqab.qab_handle = hndl;
	svcIoCancel(&cqab, CAN_ALL|CAN_INPUT|CAN_OUTPUT);
}


//*************************************************
// Function: terminate - Terminate a client session
// Returned: Nothing
//*************************************************

void terminate(
	CDB *cdb)

{
	IDB *idb;
	int  slot;

	slot = cdb->slot;
	if (srvDebugLevel >= 2)
		debug(slot, "SES", 0, "Terminating client session");
	cdb->terminate = FALSE;
	dataabort(cdb);						// Terminate the data thread
	datafunc(cdb, DATAFUNC_QUIT);
	cdb->terminate = FALSE;
	datawait(cdb);
	if (cdb->homdirstr != NULL)			// Give up any allocated strings
		free(cdb->homdirstr);
	if (cdb->curdirstr != NULL)
		free(cdb->curdirstr);
	if (cdb->rnfrstr != NULL)
		free(cdb->rnfrstr);
	thdIoClose(cdb->cmdhndl, 0);		// Close the command connection
	idb = cdb->idb;						// Put this CDB on the CDB free list
	idb->cdbtbl[slot - 1] = (CDB *)(idb->cdbfree);
	idb->cdbfree = slot;
	idb->numclients--;
	if (srvDebugLevel >= 2)
		debug(slot, "SES", 0, "Client session terminated");
	thdCtlTerminate();					// Terminate the command thread
}


//************************************
// Function: debug - Output debug text
// Returned: Nothing
//************************************

void debug(
	int   slot,
	char *label,
	int   len,
	char *fmt, ...)

{
	va_list pi;
	time_sz dt;
	char    time[32];
	char    text[200];


	if (fmt != NULL)
	{
    	va_start(pi, fmt);
    	vsprintf(text, fmt, &va_arg(pi, long));
	}
	else
		text[0] = 0;
	svcSysDateTime(T_GTHRDTTM, &dt);
	sdt2str(time, "%h:%m:%s.%3f", (time_sz *)&dt);
	fprintf(srvDebugStream, "%s %s: %d %.*s\n", time, label, slot, len, text);
}


//*******************************************
// Function: logerror - Log message for error
// Returned: Nothing
//*******************************************

void logerror(
	int   slot,
    long  code,
    char *fmt, ...)

{
	va_list pi;
	char    text[200];
	char    text2[300];
	char    textxos[100];

    va_start(pi, fmt);
    vsprintf(text, fmt, &va_arg(pi, long));

	if (code != 0)
		svcSysErrMsg(code, 0x03, strmov(textxos, "\n        "));
	sprintf(text2, "(%d) %s%s", slot, text, (code != 0) ? textxos : "");
	srvLogSysLog(0, text2);
    if (srvReqName[0] == 0 && srvDebugLevel == 0) // Are we not detatched and
												  //   not debuggint?
		printf("%c %s: %s\n", (code < 0) ? '?' : '%', prgname, text2);
	else if (srvDebugLevel > 0)
		debug(slot, "LOG", 0, text2);
}


//********************************************************************
// Function: xoscode - Convert XOS error code to 5-character text code
// Returned: Pointer to text
//********************************************************************

char *xoscode(
	long code)

{
	static char bufr[16];

	svcSysErrMsg(code, 0x02, bufr);
	return (bufr);
}
