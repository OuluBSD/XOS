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

#include "dmpinstall.h"

char    *xidbufr;
char    *outbufr;
///char    *chkbufr;
z_stream stream;
long     amnt;
long     outhndl;
long     xidhndl;
///long     chkhndl;
int      seqnum;
int      zflag = Z_NO_FLUSH;
long     itemnum;
///long     chkoffset;
long     diffcnt;
ulong    crcvalue = 0xFFFFFFFF;
long     totalsize;
long     totalitems;
FILE    *ctlfile;
char    *bootspec;
char     prgname[] = "DMPINSTALL";
char    *xidspec;
///char    *chkspec;

XIDHEAD  xidhead;
ITEMHEAD itemhead;

long getinput(char *bufr, long size);

static char *setupfile(char *arg, char *ext);


void main(
	int   argc,
	char *argv[])
{
	long rtn;
	long amnt;
	long offset;
	char ttxt[32];

	if (argc != 2)
	{
		fputs("? DMPINSTALL: Command error, usage is:\n"
				"               rdinstall xidfile\n", stderr);
		exit(1);
	}

	// Allocate our buffers

	if ((outbufr = malloc(OUTBUFRSZ)) == NULL ||
			(xidbufr = malloc(XIDBUFRSZ)) == NULL ||
			(argc == 3 && (chkbufr = malloc(OUTBUFRSZ)) == NULL))
		fail(-errno, "Error allocating buffer");

	// Set up the file specs

	xidspec = setupfile(argv[1], ".xid");

	// Open the files

	if ((xidhndl = svcIoOpen(XO_IN, xidspec, NULL)) < 0)
		fail(xidhndl, "Error opening XID file %s", xidspec);

	// Read the file header

	if ((rtn = svcIoInBlock(xidhndl, (char *)&xidhead, sizeof(XIDHEAD))) < 0)
		fail(rtn, "Error reading XID file header");

	if (*(long *)xidhead.label != 'FDIX' && *(long *)xidhead.label != 'FSIX' )
		fail(0, "Incorrect label value in XID file header");	
	if (xidhead.hlen < 12)
		fail(0, "XID file header is too short");
	if (xidhead.fmtver != 1)
		fail(0, "XID format version is not 1");
	if (xidhead.hlen > 12 && (rtn = svcIoSetPos(xidhndl, xidhead.hlen + 8,
			0)) < 0)
		fail(rtn, "Error setting position in XID file");

	// Fill the XID buffer

	if ((rtn = svcIoInBlock(xidhndl, xidbufr, XIDBUFRSZ)) <= 0)
	{
		if (rtn == 0)
			rtn = ER_EOF;
		fail(rtn, "Error reading the XID file");
	}
	stream.avail_in = rtn;
	stream.next_in = xidbufr;

	// Initialize zlib

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
	if ((rtn = inflateInit(&stream)) != Z_OK)
		fail((ushort)rtn | 0x80000000, "Error initializing zlib");

	// When get here we are ready to start reading the compressed items

	while (TRUE)
	{
		itemnum++;

		offset = totalsize;
		getinput((char *)&itemhead, sizeof(itemhead));
		if (itemhead.sync != 0xC183)
			fail(0, "Incorrect sync value for item %d", itemnum);
		if (itemhead.seqnum != (char)seqnum)
			printf(    "Incorrect sequence number - should be %02X\n",
					(char)seqnum);
		if (itemhead.namelen == 0xFFFF)
			break;
		getinput(outbufr, itemhead.namelen);

///		if ((itemhead.flag & 0x07) == 2)
///			INT3;
		
		sdt2str(ttxt, "%Z%h:%m:%s %D-%3n-%l", (time_sz *)&itemhead.cdt);
		printf("%,12d %4d %02X %c%c %s %,10d %.*s\n", offset, itemnum,
				itemhead.seqnum, ((itemhead.flag & IF_APP) ? "BFAS" :
				(itemhead.flag & IF_USR) ? "BFUS" : "BFOS")[itemhead.flag &
				0x03], " dib"[itemhead.flag >> 6], ttxt, itemhead.filelen,
				itemhead.namelen, outbufr);
		seqnum++;
		do
		{
			if ((amnt = itemhead.filelen) > OUTBUFRSZ)
				amnt = OUTBUFRSZ;
			getinput(outbufr, amnt);
			itemhead.filelen -= amnt;
		} while (itemhead.filelen > 0);

	}

	// Finished - display the results

	fputs("\n", stdout);
	if (argc == 3)
	{
		printf("Differences: %,d\n", diffcnt);
		printf(" Chk offset: %,d\n", chkoffset);
	}
	if (crcvalue == xidhead.crcvalue)
		printf("        CRC: 0x%08X OK\n", crcvalue);
	else
		printf("        CRC: 0x%08X should be 0x%08X\n", crcvalue,
				xidhead.crcvalue);
	if (totalsize == xidhead.totalsize)
		printf(" Total size: %,d OK\n", totalsize);
	else
		printf(" Total size: %,d should be %,d\n", totalsize,
				xidhead.totalsize);
	if (itemnum == xidhead.totalitems)
		printf("Total items: %,d OK\n", itemnum);
	else
		printf("Total items: %,d should be %,d\n", itemnum, xidhead.totalitems);
	exit(0);
}


void *allocmem(
	int len)
{
	void *pnt;

	if ((pnt = malloc(len)) == NULL)
		fail(-errno, "Error allocating memory");
	return (pnt);
}




long getinput(
	char *bufr,
	long  size)
{
	long  amnt;
	long  rtn;
	ulong crc;

	stream.next_out = bufr;
	stream.avail_out = size;

	amnt = 0;
	while (size > 0)
	{
///		if (*more)
		{
			if (stream.avail_in == 0)
			{
				if ((rtn = svcIoInBlock(xidhndl, xidbufr, XIDBUFRSZ)) < 0)
				{
					if (rtn != ER_EOF)
						fail(rtn, "Error reading XID file");
///					*more = FALSE;
					rtn = 0;
				}
		    	stream.next_in = xidbufr;
		   		stream.avail_in = rtn;
			}
		}
		if ((rtn = inflate(&stream, Z_NO_FLUSH)) < 0)
			fail(((ushort)rtn) | 0x80000000, "Error expanding XID data");

		amnt += (size - stream.avail_out);
		size -= (size - stream.avail_out);
		if (rtn == Z_STREAM_END)
			break;
	}
	totalsize += amnt;

	if (chkhndl > 0)
	{
		char *pnt1;
		char *pnt2;

		if ((rtn = svcIoInBlock(chkhndl, chkbufr, amnt)) < 0)
			fail(rtn, "Error reading check file");
		pnt1 = bufr;
		pnt2 = chkbufr;
		rtn = amnt;
		while (--rtn >= 0)
		{
			if (*pnt1 != *pnt2)
			{
				printf("(%,d %02X %02X) ", chkoffset, *pnt1, *pnt2);
				diffcnt++;
			}
			pnt1++;
			pnt2++;
			chkoffset++;
		}
	}

	rtn = amnt;
	crc = crcvalue;
    while (--rtn >= 0)
        crc = crctable[(crc ^ *bufr++) & 0xFF] ^ (crc >> 8);
	crcvalue = crc;
	return (amnt);
}


static char *setupfile(
	char *arg,
	char *ext)
{
	char *spec;
	char *pnt;
	int   len;
	char  chr;

	len = strlen(arg);
	spec = malloc(len + 5);
	strmov(spec, arg);
	pnt = arg + len;
	while ((chr = *--pnt) != '.')
	{
		if (pnt <= arg || (chr = *pnt) == '\\' | chr == ':')
		{
			strmov(spec + len, ext);
			break;
		}
	}
	return (spec);
}


static char *tbl[] =
{	"Version error",			// -6
	"Buffer error",				// -5
	"Memory error",				// -4
	"Data error",				// -3
	"Stream error",				// -2
	NULL,						// -1
	"No error indicated",		//  0
	"Unexpected stream end",	//  1
	"Need Dictionary"			//  2
};


void fail(
	long code,
	char *fmt, ...)
{
	va_list pi;
	char    text[600];

   	va_start(pi, fmt);
   	vsprintf(text, fmt, pi);
	if (code == 0)						// Is there an error code?
	{
		fprintf(stderr, "? %s %s\n", prgname, text); // No - just display the
		exit(1);									 //   message and die
	}
	if ((code & 0xFFFF0000) != 0x80000000) // Is this a zlib error code?
		femsg2(prgname, text, code, NULL); // No
	code = (short)code;					// Yes
	if (code == -1)						// Really an errno value?
		femsg2(prgname, text, -errno, NULL); // Yes
	if (code < -6 || code > 2)			// No - it's really a zlib code
		fprintf(stderr, "? CHKINSTALL: %s\n              Unknown zlib error "
				"code %d\n", text, code);
	else
		fprintf(stderr, "? CHKINSTALL: %s\n              zlib error: %s\n",
			text, tbl[code + 6]);
	exit(1);
}
