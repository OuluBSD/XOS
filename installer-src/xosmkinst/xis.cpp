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

static ITEMHEAD lastitem = {0xC183, 0, IT_END};
static z_stream xisstream;
static long     crcvalue;
static long     totalsize;
static uchar   *xisbufr;
static int      zflag;
static char    *xisfilename;

//**************************************************
// Function: startxis - Set up to create an XIS file
// Returned: true if normal, false if error
//**************************************************

int startxis(
	char *name,
	llong cdt)
{
	int   rtn;
	ulong size;

	// Allocate our buffer if necessary

	if (xisbufr == NULL && (xisbufr = (uchar *)malloc(XISBUFRSZ)) == NULL)
	{
		setmsg("? Error allocating XIS buffer");
		return (false);
	}

	// Create the XIS file

	if ((xishndl = CreateFile(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		seterrmsg("Error creating install set %s", xisname);
		return (false);
	}

	// Store the XIS file header (this is not compressed). The length and
	//   checksum values will be filled in at the end.

	xishead.cdt = cdt;
	xishead.totalsize = 0;
	xishead.totalitems = 0;
	xishead.crcvalue = 0xFFFFFFFF;
	if (!WriteFile(xishndl, (char *)&xishead, sizeof(XIDHEAD), &size, NULL))
	{
		seterrmsg("Error writing XIS file %s", name);
		return (false);
	}
	if (size != sizeof(XIDHEAD))
	{
		seterrmsg("Incomplete output to XIS file %s", name);
		return (false);
	}

	// Initialize zlib

	zflag = Z_NO_FLUSH;
	xisstream.zalloc = (alloc_func)0;
	xisstream.zfree = (free_func)0;
	xisstream.opaque = (voidpf)0;
	if ((rtn = deflateInit(&xisstream, 6)) != Z_OK)
		zlibfail(rtn, "Error initializing zlib deflate");
	xisstream.next_out = xisbufr;
	xisstream.avail_out = XISBUFRSZ;
	xisfilename = name;
	return (true);
}


//**********************************************************
// Function: writexis - Write compressed data to an XIS file
// Returned: trueif OK, false if error
//**********************************************************

long writexis(
	uchar *bufr,
	long   size)
{
	long  rtn;
	ulong wsize;
	ulong crc;

	xisstream.next_in = bufr;
	xisstream.avail_in = size;
	do
	{
		if (xisstream.avail_out == 0)
		{
			if (!WriteFile(xishndl, xisbufr, XISBUFRSZ, &wsize, NULL))
			{
				seterrmsg("Error writing install set %s", xisfilename);
				return (false);
			}
			xisstream.next_out = xisbufr;
			xisstream.avail_out = XISBUFRSZ;
		}
		if ((rtn = deflate(&xisstream, zflag)) < 0)
		{
			zlibfail(rtn, "Error storing item in XIS file %s", xisfilename);
			return (false);
		}
	} while (xisstream.avail_in != 0 || (zflag == Z_FINISH &&
			rtn != Z_STREAM_END));
	xishead.totalsize += size;
	crc = xishead.crcvalue;
    while (--size >= 0)
        crc = crctable[(crc ^ *bufr++) & 0xFF] ^ (crc >> 8);
	xishead.crcvalue = crc;
	return (true);
}


//****************************************************
// Function: finishxis - Finish generating an XIS file
// Returned: true if OK, false if error
//****************************************************

int finishxis()
{
	ulong wsize;

	// Store the EOF item

	lastitem.seqnum = (char)xishead.totalitems++;
	zflag = Z_FINISH;
	if (!writexis((uchar *)&lastitem, sizeof(ITEMHEAD)))
		return (false);

	// Finish writing compressed data

	if ((XISBUFRSZ - xisstream.avail_out) > 0 && !WriteFile(xishndl, xisbufr,
			XISBUFRSZ - xisstream.avail_out, &wsize, NULL))
		seterrmsg("Error finishing output to XIS file %s", xisfilename);

	// Update the file header

	if (SetFilePointer(xishndl, offsetof(XIDHEAD, totalsize), NULL, FILE_BEGIN) < 0)
	{
		seterrmsg("Error setting to update XIS header in %s", xisfilename);
		return (false);
	}
	if (!WriteFile(xishndl, (char *)&xishead.totalsize, 12, &wsize, NULL))
	{
		seterrmsg("Error writing install set %s", xisfilename);
		return (false);
	}
	if (!CloseHandle(xishndl))
	{
		seterrmsg("Error closing install set %s", xisfilename);
		return (false);
	}
	xishndl = (HANDLE)0xFFFFFFFF;
	return (true);
}
