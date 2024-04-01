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

#include "xosmkinst.h"
#include "zlib.h"

static z_stream xidstream;
static XIDHEAD  xidhead;
static uchar   *xidbufr;
static ulong    itemnum;
static ulong    crcvalue;
static ulong    totalsize;
static ulong    totalitems;


//***************************************************
// Function: startxid - Set up to access the XID file
// Returned: true if normal, false if error
//***************************************************

int startxid()
{
	int  rtn;

	// Allocate our buffer if necessary

	if (xidbufr == NULL && (xidbufr = (uchar *)malloc(XIDBUFRSZ)) == NULL)
	{
		setmsg("? Error allocating XID buffer");
		return (false);
	}

	// Open the XID file

	if ((xidhndl = CreateFile(xidspec, GENERIC_READ, 0, NULL, OPEN_EXISTING,
			0, 0)) == INVALID_HANDLE_VALUE)
	{
		seterrmsg("Error opening XID file %s", xidspec);
		return (false);
	}

	// Read the XID file header

	if (!ReadFile(xidhndl, (char *)&xidhead, sizeof(XIDHEAD),
			(ulong *)&xidstream.avail_in, NULL))
	{
		seterrmsg("Error reading XID file header");
		return (false);
	}
	if (xidstream.avail_in != sizeof(XIDHEAD))
	{
		setmsg("? EOF while reading XID file header");
		return (false);
	}
	if (*(long *)xidhead.label != 'FDIX')
	{
		setmsg("? Incorrect label value in XID file header");	
		return (false);
	}
	if (xidhead.hlen < 20)
	{
		setmsg("? XID file header is too short");
		return (false);
	}
	if (xidhead.fmtver != 2)
	{
		setmsg("? XID format version is not 2");
		return (false);
	}
	if (xidhead.hlen > 20 && (SetFilePointer(xidhndl, xidhead.hlen + 8,
			NULL, 0)) == 0xFFFFFFFF)
	{
		seterrmsg("Error setting position in XID file");
		return (false);
	}
	itemnum = 0;
	totalsize = 0;
	crcvalue = 0xFFFFFFFF;
	amount = xidhead.totalsize;
	curadone = 0;
	adone = 0;
	setmsg("");

	// Fill the XID buffer

	if (!ReadFile(xidhndl, (char *)xidbufr, XIDBUFRSZ,
			(ulong *)&xidstream.avail_in, NULL))
	{
		seterrmsg("Error reading the XID file");
		return (false);
	}
	if (xidstream.avail_in < 100)
	{
		setmsg("? Unexpected EOF while reading XID file");
		return (false);
	}
	xidstream.next_in = (uchar *)xidbufr;

	// Initialize zlib

    xidstream.zalloc = (alloc_func)0;
    xidstream.zfree = (free_func)0;
    xidstream.opaque = (voidpf)0;
	if ((rtn = inflateInit(&xidstream)) != Z_OK)
		zlibfail(rtn, "Error initializing zlib");
	return (true);
}


//*************************************************
// Function: finishxid = Finish reading an XID file
// Returned: true if OK, false if error
//*************************************************

int finishxid()
{
	int rtn;

	if (crcvalue != xidhead.crcvalue)
	{
		setmsg("XID CRC value is incorrect");
		return (false);
	}
	if (totalsize != xidhead.totalsize)
	{
		setmsg("XID total size value is incorrect");
		return (false);
	}
	if (itemnum != xidhead.totalitems)
	{
		setmsg("XID item count is incorrect");
		return (false);
	}
	if ((rtn = inflateEnd(&xidstream)) < 0)
	{
		zlibfail(rtn, "Error terminating inflate");
		return (false);
	}
	return (true);
}


int skipdata()
{
	long amnt;

	while (itemhead.h.filelen != 0)
	{
		if ((amnt = itemhead.h.filelen) > DSKBUFRSZ)
			amnt = DSKBUFRSZ;
		if (readxid(dskbufr, amnt) < 0)
			return (false);
		itemhead.h.filelen -= amnt;
	}
	return (true);
}


//****************************************************************
// Function: readitemhead - Read next item header for the XID file
// Returned: true if normal, false if error
//****************************************************************

int readitemhead()
{
	long rtn;

	// Read the fixed part of the item header

	if ((rtn = readxid((uchar *)&itemhead, sizeof(ITEMHEAD))) < 0)
		return (false);
	if (rtn != sizeof(ITEMHEAD))
	{
		setmsg("? Invalid length when reading XID item header");
		return (false);
	}
	if (itemhead.h.sync != 0xC183)
	{
		setmsg("? Incorrect sync value in XID item header");
		return (false);
	}
	if (itemhead.h.seqnum != (uchar)itemnum)
	{
		setmsg("? Incorrect sequence number in XID item header");
		return (false);
	}
	itemnum++;

	// If not EOF marker, read the item name if there is one

	if (itemhead.h.namelen != 0xFFFF && itemhead.h.namelen != 0)
	{
		if (itemhead.h.namelen > 299)
		{
			setmsg("? XID item name is too long");
			return (false);
		}
		if ((rtn = readxid((uchar *)&itemhead.name, itemhead.h.namelen)) < 0)
			return (false);
		itemhead.name[itemhead.h.namelen] = 0;
	}
	else
		itemhead.name[0] = 0;
	return (true);
}


long readxid(
	uchar *bufr,
	long   size)
{
	long  amnt;
	long  rtn;
	ulong crc;

	xidstream.next_out = bufr;
	xidstream.avail_out = size;

	amnt = 0;
	while (size > 0)
	{
		if (xidstream.avail_in == 0)
		{
			if (!ReadFile(xidhndl, xidbufr, XIDBUFRSZ, (ulong *)&rtn, NULL))
			{
				seterrmsg("Error reading the XID file");
				return (-1);
			}
		    xidstream.next_in = xidbufr;
		   	xidstream.avail_in = rtn;
		}
		if ((rtn = inflate(&xidstream, Z_NO_FLUSH)) < 0)
		{
			zlibfail(rtn, "Error expanding XID data");
			return (-1);
		}
		amnt += (size - xidstream.avail_out);
		size -= (size - xidstream.avail_out);
		if (rtn == Z_STREAM_END)
			break;
	}
	totalsize += amnt;
	adone = totalsize;

	if (amnt == 0)
	{
		setmsg("? Unexpected EOF when reading the XID file");
		return (-1);
	}
	rtn = amnt;
	crc = crcvalue;
    while (--rtn >= 0)
        crc = crctable[(crc ^ *bufr++) & 0xFF] ^ (crc >> 8);
	crcvalue = crc;
	return (amnt);
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


void zlibfail(
	int   code,
	char *fmt, ...)
{
	va_list pi;
	char    text1[600];
	char    text2[600];

   	va_start(pi, fmt);
   	vsprintf(text1, fmt, pi);
	if (code == -1)						// Really an errno value?
		sprintf(text2, "? %s: zlib error: %s", strerror(errno)); // Yes
	else if (code < -6 || code > 2)		// No - it's really a zlib code
		sprintf(text2, "? %s: zlib error %d", text1, code);
	else
		sprintf(text2, "? %s zlib error: %s", text1, tbl[code + 6]);
	setmsg(text2);	
}
