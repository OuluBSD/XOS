#include "geckoh.h"

long  debug;
uchar usbinpbuf[16];

QAB debugqab =
{	QFNC_INBLOCK,		// func    = 0  - Function
	0,					// status  = 2  - Returned status
	0,					// error   = 4  - Error code
	0,					// amount  = 8  - Amount transfered
	0,					// handle  = 12 - Device handle
	VECT_USBINP,		// vector  = 16 - Vector for interrupt
	{0},
	0,					// option  = 20 - Option bits
	16,					// count   = 24 - Amount to transfer
	usbinpbuf,			// buffer1 = 28 - Pointer to data buffer
	0,					// buffer2 = 32
	0					// parm    = 36 - Pointer to parameter list
};


struct
{	TEXT8CHAR class;
	TEXT4CHAR type;
	TEXT8CHAR speed;
	uchar     end;
} charchar =
{	{PAR_GET|REP_TEXT, 8, "CLASS"},
	{PAR_GET|REP_TEXT, 4, "TYPE"},
	{PAR_GET|REP_TEXT, 8, "USBSPEED"}
};


// The debug device appears to be capable of buffering two packets and there
//   appears to be no way to clear its buffer other than for the target to
//   read the buffered packets. Since we don't know what the state of the
//   device is when we start, we initially send two NOOP requests and quietly
//   ignore invalid responses until we see a valid response. We also send a
//   request for register values if we have not received the values (which
//   might happen while we were sending the NOOPs).

int usbinit(void)
{
	long rtn;

	ensureleft();
	putstr("Initializing USB debug device\r\n");
	if (debug > 0)
		svcIoClose(debug, 0);
	if ((debug = svcIoOpen(XO_IN|XO_OUT, "DEBUG0:", NULL)) < 0)
	{
		ioerror(debug, "Cannot open the USB debug device DEBUG0:");
		return (FALSE);
	}
	debugqab.handle = debug;
	if ((rtn = svcIoDevChar(debug, &charchar)) < 0)
	{
		ioerror(rtn, "Error getting characteristics for DEBUG0:");
		return (FALSE);
	}
	if (strcmp(charchar.class.value, "BULK") != 0 ||
			strncmp(charchar.type.value, "USBA", 4) != 0)
	{
		ioerror(0, "DEBUG0: is not a USB bulk transfer device");
		return (FALSE);
	}

#if 0
	if (strcmp(charchar.speed.value, "HIGH") != 0)
	{
		ioerror(0, "DEBUG0: is not a high speed USB device");
		return (FALSE);
	}
#endif

	if ((rtn = svcIoQueue(&debugqab)) < 0 || // Start debug input
			(rtn = debugqab.error) < 0)
	{
		ioerror(rtn, "Error starting input on the USB debug device "
				"DEBUG0:");
		return (FALSE);
	}
	return (TRUE);
}


int sendrequest(
	int type,
	int len)
{
	union
	{	long  l[2];
		uchar c[8];
	} buffer;
	uchar *bpnt;
	uchar *dpnt;
	long   rtn;
	int    bleft;
	uchar  seqnum;

	if (debug <= 0)
	{
		if (!usbinit())
			return (-1);
		svcSchSuspend(NULL, 0, 400 * ST_MILLISEC);
	}
	buffer.l[0] = type | 0x80;
	buffer.l[1] = 0;
	seqnum = 0;
	buffer.c[1] = (uchar)len;
	bpnt = buffer.c + 2;
	bleft = 6;
	if (len > 0xFF)
	{
		buffer.c[2] = (uchar)(len >> 8);
		buffer.c[0] |= 0x40;
		bpnt++;
		bleft--;
	}
	*(long *)(reqbufr.c + len) = 0xFFFFFFFF;
	*(long *)(reqbufr.c + 4 + len) = 0xFFFFFFFF;
	dpnt = reqbufr.c;
	while (TRUE)
	{
		len -= bleft;
		while (--bleft >= 0)
			*bpnt++ = *dpnt++;
		if (debugout)
		{
			if (curchar != 0)
				putstr("\r\n");
			putstr("REQ: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
					buffer.c[0], buffer.c[1], buffer.c[2], buffer.c[3],
					buffer.c[4], buffer.c[5], buffer.c[6], buffer.c[7]);
		}
		if ((rtn = svcIoOutBlock(debug, buffer.c, 8)) < 0)
		{
			ioerror(rtn, "Error sending packet to the debug device");
			return (rtn);
		}
		if (len <= 0)
			return (0);
		buffer.l[0] = seqnum++;
		bpnt = buffer.c + 1;
		bleft = 7;
	}
}


void ioerror(
	long  code,
	char *msg)
{
	char text[100];

	ensureleft();
	putstr("\1? %s", msg);
	if (code != 0)
	{
		svcSysErrMsg(code, 0x03, text);
		putstr("\r\n  %s", text);
	}
	putstr("\2\r\n");
}


void usbinpsig(void)
{
	uchar *dpnt;
	int    dcnt;
	int    type;

	if (debugqab.error != 0)			// Error?
	{
		svcIoClose(debug, 0);			// Yes - close the device
		debug = 0;						// Reset the input state
		rspleft = 0;
		rspseq = 0;
		ioerror(debugqab.error, "Error reading input from target");
		return;
	}
	if (debugqab.amount != 8)
		putstr("Input from target is wrong length\r\n");
	else
	{
		if (debugout)
		{
			if (curchar != 0)
				putstr("\r\n");
			putstr("RSP: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
					usbinpbuf[0], usbinpbuf[1], usbinpbuf[2], usbinpbuf[3],
					usbinpbuf[4], usbinpbuf[5], usbinpbuf[6], usbinpbuf[7]);
		}
		if (usbinpbuf[0] & 0x80)		// Does this start a message?
		{								// Yes
			type = usbinpbuf[0] & 0x3F;
			if (hvgdrsp && rspleft > 0)
				putstr("\1? Partial message (0x%02X) discarded\2\r\n", rsptype);
			rsptype = type;
			rspleft = usbinpbuf[1];
			rspseq = 0;
			rsppnt = rspbufr.c;
			dpnt = usbinpbuf + 2;
			dcnt = 6;
			rsplen = usbinpbuf[1];
			if (usbinpbuf[0] & 0x40)
			{
				rsplen |= (usbinpbuf[2] << 8);
				dpnt++;
				dcnt--;
			}
			if (hvgdrsp && rsplen > sizeof(rspbufr))
			{
				putstr("\1? Message (%02X) is too long (%d), discarded\r\2\n",
						rsptype, rsplen);
				rspleft = -2;
			}
			else
			{
				rspleft = rsplen;
				if (dcnt > rspleft)
					dcnt = rspleft;
				rspleft -= dcnt;
				while (--dcnt >= 0)
					*rsppnt++ = *dpnt++;
				if (rspleft == 0)
					doresponse();
			}
		}
		else							// If a continuation of a message
		{
			if (rspleft <= 0)			// Expecting start of a message?
			{
				if (hvgdrsp && rspleft != -2)
				{
					putstr("\1? Out of sequence packet (%d) discarded\2",
							usbinpbuf[0]);
					rsptype = -2;
				}
			}
			else if (hvgdrsp && usbinpbuf[0] != rspseq)
			{
				putstr("\1? Out of sequence packet (%d) discarded, current "
						"message (%02X) discarded\2\r\n", usbinpbuf[0],
						rsptype);
				rsptype = -2;
			}
			else
			{
				rspseq++;
				rspseq &= 0x7F;
				dpnt = usbinpbuf + 1;
				if ((dcnt = 7) > rspleft)
					dcnt = rspleft;
				rspleft -= dcnt;
				while (--dcnt >= 0)
					*rsppnt++ = *dpnt++;
				if (rspleft == 0)
					doresponse();
			}
		}
	}
	svcIoQueue(&debugqab);
}
