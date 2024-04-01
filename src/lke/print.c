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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xosstr.h>
#include <xos.h>
#include <xoserrmsg.h>
#include <xossvc.h>
#include <xoserr.h>
#include <errno.h>
#include <xosthreads.h>
#include <xosxws.h>
#include <xosxwsx.h>


typedef struct
{	long   label;				// Label (PTR#)
	long   dev;					// Printer device handle
	long   width;				// Current page width
	long   seedsz;				// Size of the seed row buffer
	uchar *seedrow;				// Seed row buffer (1 bit per pixel)
	uchar *currow;				// Current row buffer (1 bit per pixel)
	uchar *outrow;				// Row output buffer (mode 9 compression)
	uchar *outbfr;				// Printer output buffer


} PTR;




// NOTES:

//   $&bnW is used to send PML data (PML appears to use 1284.4 on EP3)
//   $&onW is the "Driver configure" command:
//               1st byte: Device ID
//               2nd byte: Function
//       additional bytes: Function arguments
//     This appears to set various parameters for specific printers.
//   $*bnW is the "new" configuration set up command (n = 20):
//     Offset  Size  Description
//        0      1   Format (= 6)
//        1      1   1F (KRGB ID)
//        2      2   Component count (= 2), MSB first
//                   First component:
//        4      2   Horizontal resolution, MSB first
//        6      2   Vertical resolution, MSB first
//        8      1   Compression method (= 9)
//        9      1   Orientation (= 0, pixel major mode)
//       10      1   Bits per component (= 1)
//       11      1   Planes per component (= 1)
//                   Second component:
//       12      2   Horizontal resolution, MSB first
//       14      2   Vertical resolution, MSB first
//       16      1   Compression method (= 10)
//       17      1   Orientation (=1, pixel major mode)
//       18      1   Bits per component (= 32)
//       19      1   Planes per component (= 1)
//     This specifies two "planes". The first is a 1-bit per pixel monochrome
//       plane which uses mode 9 compression. The second is a 32-bit per
//       pixel color plane which uses mode 10 compression.

// Compression modes: (Some of the HP printers require specific compression
//   modes!)
//   Mode 9: Replacement Delta Row Encoding
//     Command byte formats:

//       +---------------+  Uncompressed:
//       |0|O O O O|R R R|    O = Offset count
//       +---------------+    R = Replacement count

//       +---------------+  Compressed (run length encoded):
//       |1|O O|R R R R R|    O = Offset count
//       +---------------+    R = Replacement count

//   Mode 10: Modified replacement Delta Row Encoding (this appears to be
//              mode 9 extended for single plane color data)

//     ??????????????????????

// Current only the monochorme plane with mode 9 compression is supported!!!

#define OUTBFRSZ 2048			// Size of the printer output buffer


char *bfr;
char *pnt;
int   left;
int   amnt;
long  rtn;
long  ptr;

char prgname[] = "PT";

char *putline(char *pnt, int x1, int x2);


char buffer[20000];


char jobbgn[] =
	"\x1B""E"
	"\x1B%-12345X@PJL ENTER LANGUAGE=PCL3GUI\r\n"
	"\x1B""E"
///	"\x1B&b27WPML \x04\x00\x04\x01\x01\x02\x08\x10\x0E\x01\x15"
///			"200610250022\x00"
///	"\x1B&b16WPML \x04\x00\x06\x01\x04\x01\x04\x01\x06\x08\x01\x00"
	"\x1B*r4800SB"
	"\x1B&l0M"

	"\x1B*o5W\x0D\x03\x00\x00\x01"

	"\x1B&l1H"
	"\x1B&l2A"
	"\x1B&l0E"
	"\x1B&l0L"
	"\x1B&u600D"
	"\x1B*o0M"
	"\x1B&l-2H"
	"\x1B&l0O"

	"\x1B&o5W\x0E\x03\x00\x00\x00"

	"\x1B&o7W\x08\x09\x00\x00\x07\x00\x01"

	"\x1B*g20W\x06\x1F\x00\x02\x02\x58\x02\x58\x09\x00"
			 "\x01\x01\x02\x58\x02\x58\x0A\x01\x20\x01";

char jobend[] =	"\x1B""E\x1B%-12345X@PJL EOJ\r\n\x1B""E";



long xwsdomonorow(PTR *ptr, ushort *rowpnt);
long xwsprinterout(PTR *ptr, uchar *buffer, long len);



#define ROWLEN (600*8/8)

/*
uchar seedrow[ROWLEN];		// Seek row (1 bit/pixel)
uchar thisrow[ROWLEN];
uchar genrow[ROWLEN + (ROWLEN+254)/255 + 2];




long  width;				// Display width (pixels)


long   inx;
long   max;
long   bgn;
long   lenm1;
long   cnt;
long   offset;
long   bytecnt;
long   prv;
uchar *genpnt;
uchar *bytepnt;

int    pixcnt;

uchar  byteval;
uchar  pixbit;
uchar  seednotwhite;		// TRUE if seedrow does not contain all white (0)
							//   pixels
*/


//**********************************************************
// Function: xwsPrintBegin - Begin a print job
// Returned: 0 if normal or negative XOS error code if error
//**********************************************************

long xwsPrintBegin(
	char *prtname,
	PTR **pptr)

{
	PTR *ptr;

	if ((ptr = (PTR *)xwsmalloc(sizeof(PTR))) == NULL)
		return (-errno);

	if ((rtn = svcIoOpen(XO_OUT, prtname, NULL)) < 0)
	{
		xwsfree(ptr);
		return (rtn);
	}
	ptr->dev = rtn;
	ptr->width = 0;						// Indicate no row buffers allocated
	ptr->seedrow = NULL;
	ptr->currow = NULL;
	ptr->outrow = NULL;
	if ((ptr->outbfr = (uchar *)xwsmalloc(OUTBFRSZ)) == NULL)
	{
		ptrclean(ptr);
		return (-errno);
	}
	if ((rtn = xwsprintoutput(ptr, jobbgn, sizeof(jobbgn))) < 0 ||
			(rtn = xwsprintflush(ptr)) < 0)
	{
		ptrclean(ptr);
		return (rtn);
	}
	*pptr = ptr;
	return (0);
}


//**********************************************************
// Function: xwsPrintEnd - End a print job
// Returned: 0 if normal or negative XOS error code if error
//**********************************************************

long xwsPrintEnd(
	char *ptr)

{
	if ((rtn = xwsprintoutput(ptr, jobend, sizeof(jobend))) >= 0)
		rtn = xwsprintflush(ptr);
	ptrclean(ptr);
	return (rtn);
}


//******************************************************************
// Function: xwsPrintPage - Print one page from an XWS memory region
// Returned: 0 if normal or negative XOS error code if error
//******************************************************************

long xwsPrintPage(
	PTR    *ptr,
	ushort *buffer,				// Pixel buffer (16-bits per pixel)
	long    widtharg,			// Page width in pixels
	long    heightarg)			// Page height in poxels

{
	static char bgnpage[] = "\x1B*r0A\x1B*b0Y";
	static char endpage[] = "\x1B*rC\x0C";

	int  whitecnt;
	int  len;
	char cmdtxt[64];

	memset(ptr->seedrow, 0, ptr->seedsz);
	ptr->width = widtharg;
	whitecnt = 0;
	if ((rtn = xwsprinterout(ptr, bgnpage, sizeof(bgnpage))) < 0)
		return (rtn);
	while (--heightarg >= 0)
	{
		if ((len = xwsdomonorow(ptr, buffer)) < 0)
			whitecnt++;
		else
		{
			if (whitecnt > 0)
			{
				rtn = sprintf(cmdtxt, "%dy", whitecnt);
				if ((rtn = xwsprinterout(ptr, cmdtxt, rtn)) < 0)
					return (rtn);
				whitecnt = 0;
			}
			rtn = sprintf(cmdtxt, (heightarg > 0) ? "%dw" : "%dW", len);
			if (len > 0)
			{
				if ((rtn = xwsprinterout(ptr, cmdtxt, rtn)) < 0 ||
						(rtn = xwsprinterout(ptr, ptr->outrow, len)) < 0)
					return (rtn);
			}
		}
		buffer += widtharg;
	}
	if (whitecnt > 0)
	{
		if ((rtn = xwsprinterout(ptr, "0Y", 2)) < 0)
			return (rtn);
	}
	if ((rtn = xwsprinterout(ptr, endpage, sizeof(endpage))) < 0)
		return (rtn);					// Leave raster mode, print the page
	return (0);
}


//*********************************************************
// Function: xwsdomonorow - Generate data for one pixel row
// Returned: Length of the generated output (0 if there are
//				no differences, -1 if new row is all white
//*********************************************************


long xwsdomonorow(
	PTR    *ptr,
	ushort *rowpnt)

{
	uchar *genpnt;
	uchar *bytepnt;

	long   bytecnt;
	long   lenm1;
	long   pixcnt;
	long   inx;
	long   max;
	long   bgn;
	long   offset;
	long   prv;
	long   cnt;

	uchar  seednotwhite;
	uchar  byteval;
	uchar  pixbit;

	genpnt = ptr->outrow;

	// First convert the input row to a 1-bit per pixel row in currow (same
	//   format as the seed row buffer)

	bytepnt = ptr->currow;
	bytecnt = ptr->width/8;
	seednotwhite = 0;
	while (--bytecnt >= 0)
	{
		byteval = 0;					// Construct 1 byte of pixel values
		pixcnt = 0;
		pixbit = 0x80;
		do
		{
			if (*rowpnt++ == 0)
				byteval |= pixbit;
			pixbit >>= 1;
		} while (--pixcnt > 0);
		*bytepnt++ = byteval;
		seednotwhite |= byteval;
	}
	if (seednotwhite == 0)				// Is the entire new row white?
	{
		memset(ptr->seedrow, 0, ptr->seedsz); // Yes - this is easy!
		return (-1);
	}

	// Here with the new row constructed in currow. Now compare it to the
	//   seed row, generate any required output, and update the seed row.

	inx = 0;
	max = ptr->width/8;

	while (inx < max)
	{
		if (ptr->currow[inx] != ptr->seedrow[inx])
		{
			// Here with a difference between the new row and the seed row.
			//   Find the end of the difference. Note that we require two
			//   matching bytes to end the difference. The actual amount
			//   that gives maximum is not necessarily two but depends on
			//   the length of the next difference region. Its very complex
			//   to calulate this and it does not really make much difference
			//   so always using 2 is reasonable.

			bgn = ++inx;
			while (inx < (max - 1) && (ptr->currow[inx] != ptr->seedrow[inx] ||
					ptr->currow[inx+1] != ptr->seedrow[inx + 1]))
				inx++;

			lenm1 = inx - bgn - 1;

			// Note that this version does not use RLE encoding!

			// Generate output for this difference region

			offset = bgn - prv;

			// Store the command byte

			*genpnt++ = (((offset > 15) ? 15 : offset) << 3) +
					((lenm1 >= 7) ? 7 : lenm1);

			// Store any excess offset bytes

			if ((offset -= 15) >= 0)
			{
				while (offset >= 255)
				{
					*genpnt++ = 255;
					offset -= 255;
				}
				*genpnt++ = offset;
			}

			// Store any excess count bytes

			cnt = lenm1;
			if ((lenm1 -= 7) >= 0)
			{
				while (lenm1 >= 255)
				{
					*genpnt++ = 255;
					lenm1 -= 255;
				}
				*genpnt++ = lenm1;
			}

			// Store the data bytes and update seedrow

			do
			{
				byteval = ptr->currow[bgn];
				*genpnt++ = byteval;
				ptr->seedrow[bgn++] = byteval;
			} while (--cnt >= 0);
			prv = inx;					// Remember where this region ended


		}
		inx++;
	}
	return (genpnt - ptr->outrow);
}


//**********************************************************
// Function: xwsprinterout - Output data to the printer
// Returned: 0 if normal or negative XOS error code if error
//**********************************************************

long xwsprinterout(
	PTR   *ptr,
	uchar *buffer,
	long   len)

{
	ptr = ptr;
	buffer = buffer;
	len = len;




	return (0);
}
