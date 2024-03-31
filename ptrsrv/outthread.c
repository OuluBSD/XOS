//--------------------------------------------------------------------------*
// outthread.c
// PCL print server output thread
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
#include <string.h>
#include <time.h>
#include <errno.h>
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
#include <xosxws.h>
#include "ptrsrv.h"


static char hdrfmt[] =				// This format creates the print job
	"\x1B""E"						//   header
	"\x1B%%-12345X@PJL \n"
	"@PJL USTATUS JOB = ON\n"
	"@PJL USTATUS PAGE = ON\n"
	"@PJL JOB NAME = \"%s\"\n"
	"@PJL ENTER LANGUAGE = %s\n"
	"\x1B""E";

static char endfmt[] =				// This format creates the print job
	"\x1B""E"						//   trailer
	"\x1B%%-12345X@PJL\r\n"
	"@PJL EOJ NAME = \"%s\"\r\n"
	"\x1B""E";


static long printfile(IDB *idb, long hndl);


void outthread(void)

{
	IDB  *idb;
	QENT *qent;
	long  rtn;
	long  hndl;
	char  filespec[256];

	idb = cmdidb;
    strcpy(idb->ptrdev, ptrdev);
    strcpy(idb->splname, splname);

	debugprint("outthread: cmdidb = %X", cmdidb);

	idb->ptrhndl = cmdptrhndl;
	threadready = TRUE;
	thdCtlWake(cmdtdb);
	while (TRUE)
	{
		while (idb->qhead == NULL)
			thdCtlSuspend(NULL, -1);

		qent = idb->qhead;
		sprintf(filespec, "XOSSPL:\\%s\\%08X.SPL", idb->splname, qent->seqnum);

		debugprint("outthread: opening file %s", filespec);

		if ((hndl = svcIoOpen(XO_IN, filespec, NULL)) < 0)
			logerror(hndl, "Error opening spooled file %s", filespec);
		else
		{
			debugprint("outthread: printing file %s", filespec);

			printfile(idb, hndl);
			svcIoClose(hndl, 0);
		}
		if ((rtn = svcIoDelete(0, filespec, NULL)) < 0)
			logerror(rtn, "Error deleting spooled file %s", filespec);
		else
			debugprint("outthread: File deleted");
		idb->qhead = qent->next;
		free(qent);
	}
}


static long printfile(
	IDB *idb,
	long hndl)
{
	struct
	{	long prefix;
		long type;
		long length;
	}     header;
	char *pnt;
	char *cpypnt;
	long  rtn;
	long  cpycnt;
	long  amnt;
	char  chr;

	debugprint("in printfile");

	// Read the file prefix and the header for the first record (which should
	//   be a header record)

	if ((rtn = svcIoInBlock(hndl, (char *)&header, sizeof(header))) !=
			sizeof(header))
	{
		if (rtn >= 0)
			rtn = ER_ICMIO;
		return (rtn);
	}
	if (header.prefix != '@RTP')
	{
		logerror(ER_ERROR, "Incorrect file prefix");
		return (ER_ERROR);
	}
	if (header.type != PTRRECTYPE_HEADER)
	{
		logerror(ER_ERROR, "Type of first record is incorrect");
		return (ER_ERROR);
	}
	if (header.length > 1022)
	{
		logerror(ER_REC2L, "Header record is too long");
		return (ER_REC2L);
	}

	// Read the header record

	if ((rtn = svcIoInBlock(hndl, idb->outbfr + 1024, header.length)) !=
			header.length)
	{
		if (rtn >= 0)
			rtn = ER_ICMIO;
		return (rtn);
	}

	// The first line in the header record is the job name

	pnt = idb->outbfr + 1024;
	cpypnt = idb->jobname;
	while (--header.length >= 0 && (chr = *pnt++) != 0 && chr != '\n')
	{
		if (cpypnt < (cpypnt + sizeof(idb->jobname) - 1))
			*cpypnt++ = chr;
	}
	*cpypnt++ = 0;

	// The second line contains the copy count

	cpypnt = pnt;
	while (--header.length >= 0 && (chr = *pnt++) != 0 && chr != '\n')
		;
	if ((cpycnt = atoi(cpypnt)) < 1)
		cpycnt = 1;

	debugprint("Print job: %s, copies: %d", idb->jobname, cpycnt);

	// Construct the first part of the print job header that we generate

	rtn = sprintf(idb->outbfr, hdrfmt, idb->jobname,
			(strncmp(idb->ptrlang, "GUI", 3) == 0) ? "PCL3GUI" : "PCL");

	// Add the remainder of the header record to finish the print job header
	//   and send it to the printer

	memcpy(idb->outbfr + rtn, pnt, header.length);

	debugprint("Initial output: %d", rtn + header.length);

	if ((rtn = thdIoOutBlock(idb->ptrhndl, idb->outbfr, rtn +
			header.length)) < 0)
	{
		logerror(rtn, "Error writing job header to printer for job %s",
				idb->jobname);
		return (rtn);
	}

	// Copy the data records directly to the printer

	// NOTE: Whether or not to buffer input is a tradeoff between the cost of
	//       copying the data an extra time vs the cost of system calls to read
	//       the short headers. Considering that we expect to see records that
	//       are signficantly longer than our buffer, it seems better to not
	//       buffer input. Output is buffered. Input is done directly into a
	//       header structure for the header record and directly into the
	//       output buffer for data records.

	while ((rtn = svcIoInBlock(hndl, (char *)&header.type, 8)) == 8)
	{
		if (header.type != PTRRECTYPE_DATA)
		{
			logerror(ER_ERROR, "Data record type is incorrect");
			return (ER_ERROR);
		}

		debugprint("Have data record: %d", header.length);

		while (header.length > 0)
		{
			amnt = (header.length < OUTBFRSZ) ? header.length : OUTBFRSZ;
			if ((rtn = svcIoInBlock(hndl, idb->outbfr, amnt)) != amnt)
			{
				if (rtn >= 0)
					rtn = ER_ICMIO;
				logerror(rtn, "Error reading data for job: %s", idb->jobname);
				return (rtn);
			}

			debugprint("Data output: %d\n", amnt);

			if ((rtn = thdIoOutBlock(idb->ptrhndl, idb->outbfr, amnt)) < 0)
			{
				logerror(rtn, "Error writing data for job: %s", idb->jobname);
				return (rtn);
			}
			header.length -= amnt;
		}
	}
	if (rtn != 0 && rtn != ER_EOF)
	{
		if (rtn >= 0)
			rtn = ER_ICMIO;
		logerror(rtn, "Error reading record header for job: %s", idb->jobname);
		return (rtn);
	}

	// Finished reading the file - now construct and output the job trailer

	amnt = sprintf(idb->outbfr, endfmt, idb->jobname, idb->ptrlang);

	if ((rtn = thdIoOutBlock(idb->ptrhndl, idb->outbfr, amnt)) < 0)
	{
		logerror(rtn, "Error writing job trailer for job: %s", idb->jobname);
		return (rtn);
	}
	return (0);
}
