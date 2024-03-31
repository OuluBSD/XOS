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

#include "geckoh.h"

static int membad(void);
static int dumpmemory(void);
static int dumpinst(void);
static int dispdata(void);
static int dispinst(void);
static int displist(void);
static int dumplst1(void);
static int dumplst2(void);
static int dispprev(void);
static int opendata(void);
static int datacommon(void);
static int dumpcommon(void);

int (*dmemtbl[])(void) =
{	membad,				//               = 0x00
	dumpmemory,			// DMEM_DUMPMEM  = 0x10
	dumpinst,			// DMEM_DUMPINST = 0x20
	dispdata,			// DMEM_DISPDATA = 0x30
	dispinst,			// DMEM_DISPINST = 0x40
	dispprev,			// DMEM_DISPPREV = 0x50
	opendata,			// DMEM_OPENDATA = 0x60
	displist,			// DMEM_DISPLIST = 0x70
	dumplst1,			// DMEM_DUMPLST1 = 0x80
	dumplst2,			// DMEM_DUMPLST2 = 0x90
	membad,				//               = 0xA0
	membad,				//               = 0xB0
	membad,				//               = 0xC0
	membad,				//               = 0xD0
	membad,				//               = 0xE0
	membad				//               = 0xF0
};

char chrtbl[] = {"\\[{|????"};
char sizetbl[] = {1, 2, 4, 1, 1, 1, 1, 1};
long masktbl[] = {0xFF, 0xFFFF, 0xFFFFFFFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void rsprdmem(void)
{
///	char text[100];

	if (rsplen < offsetof(RDMEMRP, data))
	{
		putstr("\n\r\1? RDMEM response is too short\2\n\r");
		return;
	}

///	sprintf(text, "\r\nlen=%d, size=%02X, addr=%04X:%08X\r\n", rsplen,
///			rspbufr.rdmem.size, rspbufr.rdmem.sel, rspbufr.rdmem.addr);
///	putstr(text);

///	BREAK();

	if ((cmdstate = (dmemtbl[(rspbufr.rdmem.size >> 4) & 0x0F])()) == CMD_CMD)
		begincmd();
}


static int membad(void)
{
	putstr("\n\r\1? Invalid RDMEM response\2\n\r");
	return (CMD_CMD);
}



static int dumpmemory(void)
{
	dumpcommon();
	return (CMD_CMD);
}


//************************************************
// Function: dumplst1 - Dump a linked list, part 1
// Returned: TRUE if OK, FALSE if error
//************************************************

static int dumplst1(void)
{
	long length;

	if (!linechk(dumplst1))
		return (CMD_PAUSED);
	if (!dumpcommon())					// Dump this item
		return (CMD_CMD);
	length = rsplen - offsetof(RDMEMRP, data);
	if (length >= (listoffset + 4))
	{
		reqbufr.rdmem.size = rspbufr.rdmem.size;
		if ((reqbufr.rdmem.addr = *(long *)(rspbufr.rdmem.data +
				listoffset)) == 0)
			return (CMD_CMD);
		reqbufr.rdmem.count = listcount;
		putstr("\n");
	}
	else
	{
		reqbufr.rdmem.size = DMEM_DUMPLST2 | DMEM_LONG;
		reqbufr.rdmem.count = 1;
		reqbufr.rdmem.addr = rspbufr.rdmem.addr + listoffset;
	}
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


//************************************************
// Function: dumplst1 - Dump a linked list, part 2
// Returned: TRUE if OK, FALSE if error
//************************************************

// This function is called after fetching the address of the next item
//   in the list. This is only used if the address was not included in
//   the values fetched to be displayed.

static int dumplst2(void)
{
	reqbufr.rdmem.size = listsize;
	if ((reqbufr.rdmem.addr = *(long *)rspbufr.rdmem.data) == 0)
		return (CMD_CMD);
	reqbufr.rdmem.count = listcount;
	putstr("\n");
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


static int dumpcommon(void)
{
	union
	{	uchar  *c;
		ushort *s;
		ulong  *l;
	}      dpnt;
	uchar *tpnt;
	uchar *apnt;
	long   address;
	long   rcvd;
	int    size;
	int    left;
	int    num;
	int    cnt;
	int    length;
	char   text[100];
///	char   chr;

	ensureleft();
	size = rspbufr.rdmem.size & 0x0F;
	address = rspbufr.rdmem.addr;
	length = rsplen - offsetof(RDMEMRP, data);
	dpnt.c = rspbufr.rdmem.data;
	left = length;
	while (left > 0)
	{
		tpnt = text;
		tpnt += sprintf(tpnt, "%08X>", address);
		if ((num = 16) > left)
			num = left;
		cnt = num;
		apnt = dpnt.c;
		do
		{
			switch(size)
			{
			 case DMEM_BYTE:
				tpnt += sprintf(tpnt, " %02X", *dpnt.c++);
				cnt--;
				break;

			 case DMEM_WORD:
				tpnt += sprintf(tpnt, " %04X", *dpnt.s++);
				cnt -= 2;
				break;

			 case DMEM_LONG:
				tpnt += sprintf(tpnt, " %08X", *dpnt.l++);
				cnt -= 4;
				break;
			}
		} while (cnt > 0);
		*tpnt++ = 0xB3;
		do
		{
			*tpnt++ = *apnt++;
		} while (--num > 0);
		*tpnt++ = 0xB3;
		putraw(text, tpnt - text);
		putstr("\r\n");
		address += 16;
		left -= 16;
	}
	rcvd = rsplen - offsetof(RDMEMRP, data);
	lstaddr.val = address;
	if (rcvd < (rspbufr.rdmem.count * size))
	{
		ensureleft();
		putstr("\1? Memory access failed at %08X\2\r\n", rspbufr.rdmem.addr +
				rcvd);
		return (FALSE);
	}
	return (TRUE);
}


static int dumpinst(void)
{


	return (CMD_CMD);
}


static int dispdata(void)
{
	datacommon();
	putstr("\r\n");
	return (CMD_CMD);
}


static int opendata(void)
{
	if (!datacommon())
		return (CMD_CMD);
	putstr(" %%");
	cmdpnt = cmdline;
	return (CMD_MOPEN);
}


//*******************************************
// Function: displist - Display a linked list
// Returned: New command state
//*******************************************

static int displist(void)
{
	if (!datacommon())					// Display this link
		return (CMD_CMD);
	putstr("\n");
	if (*(long *)rspbufr.rdmem.data == 0)
		return (CMD_CMD);
	reqbufr.rdmem.count = 1;
	reqbufr.rdmem.size = DMEM_DISPLIST | DMEM_LONG;
	listaddr.val = *(long *)rspbufr.rdmem.data;
	reqbufr.rdmem.addr = listaddr.val + listoffset;
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


static int datacommon(void)
{
	char addr[200];
	char text[400];

	rspbufr.rdmem.size &= 0x07;
	if ((rsplen - offsetof(RDMEMRP, data)) < sizetbl[rspbufr.rdmem.size])
	{
		ensureleft();
		fmtnumeric(addr, rspbufr.rdmem.addr);
		putstr("\1? Cannot access memory at %s\2", addr);
		return (FALSE);
	}
	fmtnumeric(text, *(long *)rspbufr.rdmem.data & masktbl[rspbufr.rdmem.size]);
	if (!suspaddr)
	{
		fmtnumeric(addr, rspbufr.rdmem.addr);
		putstr("\r\4%s%c\t%s", addr, chrtbl[rspbufr.rdmem.size], text);
		curaddr.val = rspbufr.rdmem.addr;
		curszx = rspbufr.rdmem.size;
		cursize = sizetbl[curszx];
	}
	else
	{
		suspaddr = FALSE;
		putstr("\t%s", text);
	}
	return (TRUE);
}


static int dispinst(void)
{
	int   avl;
	char  addr[128];
	char  text[400];

	avl = rsplen - offsetof(RDMEMRP, data);
	cursize = instdisp(rspbufr.rdmem.type, rspbufr.rdmem.data, text);
	*txtpnt = 0;
	if (cursize <= avl)
	{
		if (!suspaddr)
		{
			curcmdinx = CCX_EI;
			curaddr.val = rspbufr.rdmem.addr;
			fmtnumeric(addr, rspbufr.rdmem.addr);
			putstr("\r\4%s!\t%s", addr, text);
		}
		else
		{
			suspaddr = FALSE;
			putstr("\t%s", text);
		}
	}
	else
	{
		rspbufr.rdmem.addr += avl;
		fmtnumeric(addr, rspbufr.rdmem.addr);
		ensureleft();
		putstr("\1? Cannot access memory at %s\2", addr);
	}
	return (CMD_CMD);
}


static int dispprev(void)
{
	uchar *ipnt;
	char  *pnt;
	uchar *target;
	int    avl;
	char   addr[400];
	char   text[400];

	avl = rsplen - offsetof(RDMEMRP, data);
	if (avl < rspbufr.rdmem.count)
	{
		rspbufr.rdmem.addr += avl;
		fmtnumeric(addr, rspbufr.rdmem.addr);
		ensureleft();
		putstr("\1? Cannot access memory at %s\2", addr);
	}
	else
	{
		ipnt = rspbufr.rdmem.data;
		target = ipnt + rspbufr.rdmem.count;
		do
		{
			if ((cursize = instdisp(rspbufr.rdmem.type, ipnt, text)) == 0)
				cursize++;
			if (debugout)
			{
				*txtpnt = 0;
				putstr("=> %08X (%d) %s\r\n", ipnt, cursize, text);
			}
			ipnt += cursize;
		} while (ipnt < target);
		if (ipnt == target)
		{
			curaddr.val = rspbufr.rdmem.addr + rspbufr.rdmem.count - cursize;
			pnt = fmtnumeric(addr, curaddr.val);
			putstr("\r\4%s!\t%s", addr, text);
		}
		else
		{
			curcmdinx = CCX_EB;
			curaddr.val = rspbufr.rdmem.addr + rspbufr.rdmem.count - 1;
			cursize = 1;
			fmtnumeric(addr, curaddr.val);
			fmtnumeric(text, rspbufr.rdmem.data[rspbufr.rdmem.count - 1]);
			putstr("\r\4%s\\\t%s", addr, text);
		}
	}
	return (CMD_CMD);
}


void rspwrtmem(void)
{
	if (cmdstate != CMD_WWTO)
		unexpected(DBREQ_WRTMEM);
	else
		begincmd();
}
