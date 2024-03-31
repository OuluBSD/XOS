//===================================================
// CLIENT.C - MMS inventory file server client thread
// Written by John Goltz
//===================================================

#include "xfpsrv.h"


static struct
{   byte4_parm  timeout;
    char        end;
} inparms =
{   {PAR_SET|REP_DECV, 4, IOPAR_TIMEOUT, -1}
};


static void funcbad(CDB *cdb);
static void getinput(CDB *cdb, char *bufr, int length);



//********************************************************************
// Function: clientthread - Main thread function for the client thread
// Returned: Never returns
//********************************************************************

void clientthread(
    CDB *cdb,
    long slot,
    long hndl,
	long ipaddr,
	long port)

{
	static void (*functbl[])(CDB *cdb, uchar *msg, int len) =
	{	funcbad,			//                  = 0
		funcconnect,		// XFPCF_CONNECT    = 1
		funcpassowrd,		// XFPCF_PASSWORD   = 2
		funcdisconnect,		// XFPCF_DISCONNECT = 3
		funcsession,		// XFPCF_SESSION    = 4
		funckeepalive,		// XFPCF_KEEPALIVE  = 5
		funcbad,			//                  = 6
		funcbad,			//                  = 7
		funcopen,			// XFPCF_OPEN       = 8
		funcparm,			// XFPCF_PARM       = 9
		funcclose,			// XFPCF_CLOSE      = 10
		funccommit,			// XFPCF_COMMIT     = 11
		funcdelete,			// XFPCF_DELETE     = 12
		funcrename,			// XFPCF_RENAME     = 13
		funcdata,			// XFPCF_DATA       = 14
		funcdatareq			// XFPCF_DATAREQ    = 15
	};

    int msglen;

///	printf("### Starting client, slot = %d, CDB = %X\n", slot, cdb);

	clientcnt++;
    cdb->slot = slot;
    cdb->tcphndl = hndl;
	cdb->ipaddr.n = ipaddr;
	cdb->port = port;
    while (TRUE)
    {
		getinput(cdb, cdb->buffer, sizeof(XFPHEAD));
		msglen = sw(((XFPHEAD *)(cdb->buffer))->length);
		if (msglen < 0)
			clientfail(cdb, 0, "Encryption is not yet supported");
		if (msglen > sizeof(cdb->buffer))
		{
			sprintf(cdb->buffer, "Client message is too long (%d)", msglen);
			clientfail(cdb, 0, cdb->buffer);
		}
		getinput(cdb, cdb->buffer + sizeof(XFPHEAD), msglen);
		if (srvDebugStream != NULL)
			debugout(cdb, "msg", "hd2=0x%02X, hd3=0x%02X, len=%d",
					cdb->buffer[2], cdb->buffer[3], msglen);
		(functbl[((XFPHEAD *)(cdb->buffer))->func & 0x0F])(cdb,
				cdb->buffer + 4, msglen);
    }
}


static void funcbad(
	CDB *cdb)

{
	cdb = cdb;
}


//*************************************************************
// Function: getinput - Get input bytes from the TCP connection
// Returned: Nothing (does not return if error)
//*************************************************************

static void getinput(
    CDB  *cdb,
	char *bufr,
    int   length)

{
    long rtn;

    if ((rtn = thdIoInBlockP(cdb->tcphndl, bufr, length, &inparms)) < 0)
    {
		if (rtn == ER_NCCLR)
			clientdone(cdb);
		clientfail(cdb, rtn, "Error receiving client message");
    }
}


//*****************************************************************
// Function: eatinput - Discard input bytes from the TCP connection
// Returned: Nothing (does not return if error)
//*****************************************************************

void eatinput(
    CDB *cdb,
    int  length)

{
	long  amount;
    long  rtn;
	uchar junk[400];

	while (length > 0)
	{
		if ((amount = length) > 400)
			amount = 400;

	    if ((rtn = thdIoInBlockP(cdb->tcphndl, junk, amount, &inparms)) < 0)
    	{
			if (rtn == ER_NCCLR)
				clientdone(cdb);
			clientfail(cdb, rtn, "Error while discarding client data");
		}
		length -= amount;
	}
}


//********************************************
// Function: notify - Notify everyone of error
// Returend: Nothing
//********************************************

// Sends error response to the user and logs the error
/*
void notify(
	CDB  *cdb,
	int   type,
	long  code,
	char *fmt, ...)

{
	va_list pi;
	long    xoscode;
	char    text1[200];
	char    text2[300];
	char    text3[16];
	char    textmms[100];
	char    textxos[100];

    va_start(pi, fmt);
    vsprintf(text1, fmt, &va_arg(pi, long));
	xoscode = mmserrmsg(code, strmov(textmms, "\n        "));
	if (xoscode != 0)
		svcSysErrMsg(xoscode, 0x03, strmov(textxos, "\n        "));
	if (cdb != NULL)
		sprintf(text3, "(%d) ", cdb->slot);
	else
		text3[0] = 0;
	svcSysLog(text2, sprintf(text2, "xxxxINVSRV  %s%s%s%s", text3, text1,
			(textmms[9] != 0) ? textmms : "", (xoscode != 0) ? textxos : ""));
	if (type != 0)
		errorrsp(cdb, type, code, text1);
	if (debug)
		printf("? INVSRV: %s\n", text2 + 12);
}
*/


//*****************************************
// Function: errorrsp - Send error response
// Returned: Nothing
//*****************************************

/*
void errorrsp(
	CDB  *cdb,
	int   type,
    long  code,
	char *fmt, ...)

{
	va_list pi;
	struct errmsg
	{	long    msglength;
		uchar   msgtype;
		uchar   msgflags;
		ushort  mmscode;
		ushort	xoscode;
		char	text[160];
	}       errmsg;
	int     len;
	char    text[200];

	if (fmt != NULL)
	{
    	va_start(pi, fmt);
    	if ((len = vsprintf(text, fmt, &va_arg(pi, long))) > 160)
			len = 160;
	}
	else
		len = 0;
	errmsg.mmscode = sw(code >> 16);
	errmsg.xoscode = sw((ushort)code);
	if (len > 0)
		memcpy(errmsg.text, text, len);
	sendrsp(cdb, type, 0x00, (uchar *)&errmsg, len + 4);
}
*/


//*******************************************************
// Function: sendnormrsp - Send normal response to client
// Returned: Nothing (does not return if error)
//*******************************************************

void sendnormrsp(
    CDB   *cdb,
	long   error,
	long   amount,
	uchar *parm)

{
	uchar *xpnt;
	uchar *tpnt;
	long   rtn;
	int    len;

	if (srvDebugStream != NULL)
		debugout(cdb, "rsp", "code=%d, amount=%d, %s", error, amount,
				(parm != NULL) ? "have parameters" : "no parameters");
	cdb->buffer[2] = XFPSF_RESPONSE;	// NEED SEQ NUM HERE!!!!
	if (error < 0)
	{
		cdb->buffer[3] = 0x90 | cdb->channel;
		*(ulong *)(cdb->buffer + 4) = sl(error);
		xpnt = cdb->buffer + 8;
	}
	else
	{
		cdb->buffer[3] = 0x10 | cdb->channel;
		xpnt = cdb->buffer + 4;
	}
	xpnt = putvalue(xpnt, amount);
	if (parm != NULL)
	{
		if ((tpnt = storeparms(cdb, xpnt, parm)) == NULL)
		{
			if (cdb->diskqab.qab_error >= 0)
			{
				*(ulong *)(cdb->buffer + 4) = SL(ER_NPERR);
				xpnt = putvalue(cdb->buffer + 8, amount);
			}
		}
		else
			xpnt = tpnt;
	}
	len = xpnt - cdb->buffer;
	*(ushort *)(cdb->buffer) = sw(len - 4);
	if ((rtn = thdIoOutBlock(cdb->tcphndl, cdb->buffer, len)) < 0)
		clientfail(cdb, rtn, "Error sending client response");
}


//***************************************************
// Function: clientdone - Terminate the client thread
// Returned: Never returns
//***************************************************


extern long moveamnt;
extern long movecnt;

void clientdone(
    CDB *cdb)

{
	int chnl;

///	printf("### terminating client, CDB = %x\n", cdb);

	chnl = 0;							// Close any open local files
	do
	{
		if (cdb->hndltbl[chnl] != 0)
		{
			thdIoClose(cdb->hndltbl[chnl], 0);
			cdb->hndltbl[chnl] = 0;
		}
	} while (++chnl < NUMHNDL);
    if (cdb->tcphndl != 0)				// Close the client connection
    {
		thdIoClose(cdb->tcphndl, 0);
		cdb->tcphndl = 0;
    }

///	notify(cdb, 0, 0, "### giving up client slot %d", cdb->slot);

    clienttbl[cdb->slot] = (CDB *)clientfree;
    clientfree = cdb->slot;
	clientcnt--;
    thdCtlTerminate();
}


//*******************************************************************
// Function: clientfail - Log fatal error and terminate client thread
// Returned: Never returns
//*******************************************************************

void clientfail(
    CDB  *cdb,
    long  code,
	char *fmt, ...)

{
	va_list pi;
	char    text[300];

	code = code;

	if (fmt != NULL)
	{
    	va_start(pi, fmt);
    	vsprintf(text, fmt, &va_arg(pi, long));
	}
	else
		text[0] = 0;

///	printf("### !! clientfail !! code = %d msg: %s\n", code, text);

///qqqqqqqqqqqqqq
///	logerror(code, text);
    clientdone(cdb);
}


void debugdump(
	CDB   *cdb,
	char  *lbl,
	uchar *data,
	int    len)

{
	uchar *pnt1;
	uchar *pnt2;
	int    cnt;
	int    offset;
	char   text1[64];
	char   text2[24];
	uchar  chr;

	offset = 0;
	while (len > 0)
	{
		pnt1 = text1;
		pnt2 = text2;
		cnt = 16;
		do
		{
			pnt1 += sprintf(pnt1, " %02X", chr = *data++);
			if (chr <' ' || chr == 0xFF)
				chr = '.';
			*pnt2++ = chr;
			len--;
		} while (--cnt > 0 && len > 0);
		*pnt2 = 0;
		debugout(cdb, lbl, "%03X>%s%*s%|%s|", offset, text1, 3 * cnt + 1, "",
				text2);
		offset += 16;
		lbl = NULL;
	}
}


void debugout(
	CDB  *cdb,
	char *lbl,
	char *fmt, ...)
{
	va_list pi;
	char    text[256];

    va_start(pi, fmt);
    vsprintf(text, fmt, &va_arg(pi, long));

	if (lbl != NULL)
		fprintf(srvDebugStream, "%s %02X: %s\n", lbl, cdb->slot, text);
	else
		fprintf(srvDebugStream, "        %s\n", text);
}
