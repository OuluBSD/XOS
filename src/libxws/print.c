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

#include <xosxxws.h>

uchar  *genrow;
uchar  *currow;
uchar  *monseedrow;
uchar  *colseedrow;

///long   xxxx;

XWSPTR *thdptr;


// These routines generate output for printers which support PCL raster
//   output using mode 9 or mode 10 compression. This includes PCL3GUI.
//   The output generated does not include the necessary PJL header and
//   trailer elements. It is assumed that this output will be written to
//   a file which will later be sent to the printer by PTRSRV or the
//   equilivalent.

//===========================================================================
//
// Following is a summary of the "unpublished" information we have collected
//   about the PCI3GUI protocol
//
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
//
//===========================================================================
//
// Compression modes: (Some of the HP printers require specific compression
//   modes, generally mode 9 or mode 10, which are almost completely
//   undocumented!)
//
//   Counts: All count fields can be extended by specifying the maximum value
//     that fits in the fields. Each extention is an 8-bit byte. The count
//     can be extended indefinately by specifying 255 in each byte. The total
//     count is the sum of all of the count bytes. The last count extension
//     byte must be less than 255. It may be 0. Thus a count of 1000 would
//     be encoded (assuming the initial count field is 3 bits) as values of
//     7, 255, 255, 255, and 228. A count of 235 would be encoded as 7, 255,
//     and 0. Note that in some cases the value encoded is 1 or 2 bytes less
//     than the intended count (see below).
//
//===========================================================================
//
//   Mode 9: Replacement delta row encoding for 1 bit per pixel planes
//
//     Command byte formats:
//
//       +---------------+  Uncompressed:
//       |0|C C C C|R R R|    C = Number of bytes to copy from seed row
//       +---------------+    R = Number of bytes - 1 to copy from data
//
//       +---------------+  Compressed (run length encoded):
//       |1|C C|R R R R R|    O = Number of bytes to copy from seed row
//       +---------------+    R = Number of times - 2 to repeat the single
//                                  data byte. (The HP documents imply this
//                                  should 1 less than the count. This is
//                                  wrong!)
//     All count expansion bytes immediately follow the command byte, first
//       any expansion bytes for the copy count followed by any expansion
//       bytes for the replacement count. These are followed by the data
//       byte(s).
//
//===========================================================================
//
//   Mode 10: Modified replacement Delta Row Encoding (this appears to be
//              mode 9 extended for single plane color data although there
//              are some additional unobvious differences)
//
//     Pixel formats:
//
//       +---------------+---------------+  Short delta pixel value
//       |1|R R R R R|G G G G G|B B B B B|    R = Red difference
//       +---------------+---------------+    G = Green difference
//                                            B = Blue difference / 2
//
//       +---------------+---------------+---------------+ Long pixel value
//       |0|R R R R R R R R|G G G G G G G G|B B B B B B B|   R = Red value
//       +---------------+---------------+---------------+   G = Green value
//                                                           B = Blue value / 2
//
//     Command byte formats:
//
//       +---------------+  Uncompressed:
//       |0|S S|C C|R R R|    S = Pixel source
//       +---------------+    C = Number of bytes to copy from seed row
//                            R = Number of bytes - 1 to copy from data
//
//       +---------------+  Compressed (run length encoded):
//       |1|S S|C C|R R R|    S = Pixel source
//       +---------------+    C = Number of bytes to copy from seed row
//                            R = Number of times - 2 to repeat pixel
//       Pixel source values:
//         00 = New pixel
//         01 = West pixel
//         10 = Northeast pixel
//         11 = Cached pixel (last new pixel specified)
//
//     When the count of the list of pixel values in the uncompressed format
//     is extended, the extension bytes are distributed thourghout the pixel
//     values. Each field is followed by the number of bytes it specifies
//     followed by the next count extention byte, etc. The maximum count of
//     7 from the command byte is considered to represent 8 pixels. Thus if
//     there are any extension bytes, there will always be 8 pixel values
//     before the first count extension byte. Additional extension bytes
//     are seperated by 255 bytes.
//
//     When a literal pixel value is specified for the RLE format, the pixel
//     value follows the copy count extension bytes but PRECEEDS the repeat
//     count extension bytes. This is different from mode 9.
//
//===========================================================================

// This version supports both monochrome and color modes. It does not support
//   mixed monochrome and color. It does use RLE compression but its use of
//   compression in color mode is not very agressive. This needs to be
//   improved.

#define OUTBFRSZ 2048			// Size of the printer output buffer

static long   docolorrow(XWSPTR *ptr, ushort *rowpnt);
static long   domonorow(XWSPTR *ptr, ushort *rowpnt);
static void   printclean(XWSPTR *ptr);
static long   printflush(XWSPTR *ptr);
static long   printout(XWSPTR *ptr, uchar *buffer, long len);
static uchar *storecolorcmd(XWSPTR *ptr, uchar *genpnt, ushort *rowpnt, int bgn,
		int cpycnt, int repcnt);
static uchar *storecolorrlecmd(XWSPTR *ptr, uchar *genpnt, int pixel, int bgn,
		int cpycnt, int repcnt);
static uchar *storemonocmd(XWSPTR *ptr, uchar *genpnt, int bgn, int cpycnt,
		int repcnt);
static uchar *storemonorlecmd(XWSPTR *ptr, uchar *genpnt, int bgn, int cpycnt,
		int repcnt);
static uchar *storepixel(uchar *genpnt, int pixel);

// Sequence for printing a document:
//	1. Call xwsPrintBegin to set up the printer (this creates a PTR object)
//  2. Call xwsPrintPage for each page of the document
//  3. Call xwsPrintEnd to terminate the print job (this gives up the PTR
//       object)
// Note that any error return from xwsPrintPage terminates the print job and
//   frees up the PTR object. DO NOT call xwsPrintEnd in this case.

char ptrlang[24];

struct
{	LNGSTRCHAR ptrcmd;
	char       end;
} langchar =
{	{PAR_GET|REP_STR , 0, "PTRLANG", ptrlang, sizeof(ptrlang) - 2,
			sizeof(ptrlang) - 2}
};


//**********************************************************
// Function: xwsPrintBegin - Begin a print job
// Returned: 0 if normal or negative XOS error code if error
//**********************************************************

long xwsPrintBegin(
	char    *prtname,			// Name of spooled printer device
	char    *filespec,			// Name of output file or NULL if output is
								//   to the spooled printer
    char    *jobname,			// Job name
	int      cpycnt,			// Number of copies
	int      mode,				// Print mode:
								//   0 = Monochrome
								//   1 = Color
								//   2 = Mixed
	XWSPTR **pptr)

{
	typedef struct
	{	char flabel[4];
		long type;
		long length;
		char jobname[48];
	}    FHEAD;
	static FHEAD fhead = {"PTR@", PTRRECTYPE_HEADER};
	static char  pcl3guibgn[] =

///		"\x1B&b27WPML \x04\x00\x04\x01\x01\x02\x08\x10\x0E\x01\x15"
///				"200610250022\x00"
///		"\x1B&b16WPML \x04\x00\x06\x01\x04\x01\x04\x01\x06\x08\x01\x00"
///		"\x1B*r4800SB"
///		"\x1B&l0M"
///		"\x1B*o5W\x0D\x03\x00\x00\x01"
///		"\x1B&l1H"
///		"\x1B&l2A"
///		"\x1B&l0E"
///		"\x1B&l0L"
///		"\x1B*o0M"
///		"\x1B&l-2H"
///		"\x1B&l0O"
///		"\x1B&o5W\x0E\x03\x00\x00\x00"
///		"\x1B&o7W\x08\x09\x00\x00\x07\x00\x01"

		"\x1B*g20W\x06\x1F\x00\x02"
			"\x02\x58\x02\x58\x09\x00\x01\x01"  // Component 1
			"\x02\x58\x02\x58\x0A\x01\x20\x01"; // Component 2
	static char pclbgn[] =
		"\x1B*t600R"			// Resolution = 600 DPI
		"\x1B&l0L"				// Disable perf skip
		"\x1B""9"				// Clear horizontal margins
		"\x1B&l0E"				// Top margin = 0
		"\x1B*b9M";				// Compression mode = 9

	XWSPTR *ptr;
	char   *bgnstr;
	int     bgnlen;
	long    rtn;
	long    hndl;

	if ((ptr = (XWSPTR *)xwsMalloc(sizeof(XWSPTR))) == NULL)
		return (-errno);
	memset(ptr, 0, sizeof(XWSPTR));

	// Get the PTRLANG value from the spooled printer device

	if ((rtn = svcIoOpen(XO_RAW, prtname, NULL)) < 0)
	{
		xwsFree(ptr);
		return (rtn);
	}
	hndl = rtn;
	rtn = svcIoDevChar(hndl, (char *)&langchar);
	svcIoClose(hndl, 0);
	if (rtn < 0)
	{
		xwsFree(ptr);
		return (rtn);
	}

	// Open the output device (either the spooled printer or a file)

	if ((rtn = svcIoOpen(XO_OUT|XO_CREATE|XO_TRUNCA, (filespec != NULL) ?
			filespec : prtname, NULL)) < 0)
	{
		xwsFree(ptr);
		return (rtn);
	}
	ptr->hndl = rtn;
	thdptr = ptr;


///	if ((xxxx = svcIoOpen(XO_OUT|XO_CREATE|XO_TRUNCA, "file.xxx", NULL)) < 0)
///		printf("### Could not open file.xxx\n");


	if (strncmp(ptrlang, "GUI", 3) == 0)
	{
		// If get here we have a PCL3GUI printer (probably HP). We use the
		//   $*g20W command to set the configuration. We support mono data
		//   using mode 9 compression and color data useing mode 10. This
		//   version does not mix mono and color data.

		bgnstr = pcl3guibgn;
		bgnlen = sizeof(pcl3guibgn) - 1;
	}
	else if (strncmp(ptrlang, "PCL", 3) == 0)
	{
		// If get here we have a PCL or HBP (Brother) printer. We use the
		//   $t#R command to set resolution and $*b#M to set compression
		//   mode. Currently all we support in this case is mono mode using
		//   mode 9 compression.

		bgnstr = pclbgn;
		bgnlen = sizeof(pclbgn) - 1;
	}
	else
	{
		printclean(ptr);
		return (ER_UNSPL);
	}
	ptr->cmode = (ptrlang[4] == 'C' || ptrlang[5] == 'C');
	ptr->mmode = (ptrlang[4] == 'M' || ptrlang[5] == 'M');

	if ((mode == PTRMODE_BOTH && !(ptr->cmode && ptr->mmode)) ||
			(mode == PTRMODE_COLOR && !ptr->cmode) || !ptr->mmode)
	{
		printclean(ptr);
		return (ER_PMDNA);
	}
	ptr->mode = mode;
	ptr->width = 0;						// Indicate no row buffers allocated
	ptr->monseedrow = NULL;
	ptr->currow = NULL;
	ptr->genrow = NULL;
	ptr->monseedrow = NULL;
	ptr->colseedrow = NULL;
	if ((ptr->outbfr = (uchar *)xwsMalloc(OUTBFRSZ * 2)) == NULL)
	{
		printclean(ptr);
		return (-errno);
	}
	ptr->outpnt = ptr->outbfr;
	ptr->outcnt = OUTBFRSZ;
	ptr->bfrpos = 0;
	rtn = sprintf(fhead.jobname, "%.32s\n%d\n", jobname, cpycnt);
	fhead.length = rtn + bgnlen;
	if ((rtn = printout(ptr, (uchar *)&fhead, offsetof(FHEAD, jobname) +
			rtn)) < 0 || (rtn = printout(ptr, (uchar *)bgnstr, bgnlen)) < 0)
		return (rtn);
	*pptr = ptr;
	return (0);
}


//**********************************************************
// Function: xwsPrintEnd - End a print job
// Returned: 0 if normal or negative XOS error code if error
//**********************************************************

long xwsPrintEnd(
	XWSPTR *ptr)
{
	long rtn;

	rtn = printflush(ptr);
	printclean(ptr);
	return (rtn);
}


//******************************************************************
// Function: xwsPrintPage - Print one page from an XWS memory region
// Returned: 0 if normal or negative XOS error code if error
//******************************************************************

long xwsPrintPage(
	XWSPTR *ptr,
	XWSWIN *win)
{
	struct
	{	BYTE4PARM pos;
		uchar     end;
	} posparm =
	{    {PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS}
	};
	static uchar bgnpage[] =
		"\x1B*b%dM"				// PCL3GUI doesn't need this but it doesn't hurt
		"\x1B*r1A"				// This MUST be *r1A not *r0A (at least for the
		"\x1B*b0y";				//   Deskjet 5300 series)
	static uchar endpage[] =	// Just end raster mode and output a formfeed
		"\x1B*rC\x0C";
	static uchar header[] = {0xF0, 0xF0, 0x0F, 0x0C, 0, 0, 0, 0};

	ushort *buffer;
	long    rtn;
	long    heightcnt;
	long    whitecnt;
	long    len;
	char    cmdtxt[64];

	if (ptr->width != win->width)		// Has the width changed?
	{
		if (ptr->width != 0)			// Yes - first time?
		{
			if (ptr->monseedrow != NULL)  // No - give up the current row
			{							  //   buffers (Set them to NULL here
				xwsFree(ptr->monseedrow); //   so printclean will work right if
				ptr->monseedrow = NULL;   //   if there is an error allocating
			}							  //   the new buffers.
			if (ptr->colseedrow != NULL)
			{
				xwsFree(ptr->colseedrow);
				ptr->colseedrow = NULL;
			}
			xwsFree(ptr->currow);
			ptr->currow = NULL;
			xwsFree(ptr->genrow);
			ptr->genrow = NULL;
		}
		if (ptr->mode != PTRMODE_COLOR)
		{
			ptr->monseedsz = len = (win->width + 7)/8;
			if ((ptr->monseedrow = (uchar *)xwsMalloc(len)) == NULL)
			{
				printclean(ptr);
				return (-errno);
			}
		}
		if (ptr->mode != PTRMODE_MONO)
		{
			ptr->colseedsz = len = win->width * 2;
			if ((ptr->colseedrow = (ushort *)xwsMalloc(ptr->colseedsz)) == NULL)
			{
				printclean(ptr);
				return (-errno);
			}
		}
		if ((ptr->currow = (uchar *)xwsMalloc(len)) == NULL ||
				(ptr->genrow = (uchar *)xwsMalloc(len + (len + 254)/255 + 8))
				== NULL)
		{
			printclean(ptr);
			return (-errno);
		}
		genrow = ptr->genrow;
		currow = ptr->currow;
		monseedrow = ptr->monseedrow;

		ptr->width = win->width;
	}
	xwsdodraw(win);						// Make sure the window buffer is
										//   current
	whitecnt = 0;

	// Here to output the page header. This would be rather complex to handle
	//   if the 4-byte count value is split between buffers. We take the easy
	//   way out and simply output a header for a 0 length page (which will
	//   be ignored by PTRSRV - a real blank page will output the page prefix
	//   and suffix).

	if (ptr->outcnt < 8 && ptr->outcnt > 4 &&
			(rtn = printout(ptr, header, 8)) < 0)
		return (rtn);

///	printf("### page begin: 0x%X\n", ptr->bfrpos + (ptr->outpnt - ptr->outbfr));

	if ((rtn = printout(ptr, header, 8)) < 0)
		return (-errno);
	ptr->lenpnt = ptr->outpnt - 4;
	ptr->lenpos = ptr->bfrpos + (ptr->outpnt - ptr->outbfr) - 4;
	ptr->reclen = 0;

	// Now output the page prefix code

	rtn = sprintf(cmdtxt, (char *)bgnpage, (ptr->mode == PTRMODE_COLOR) ?
			10 : 9);
	if ((rtn = printout(ptr, (uchar *)cmdtxt, rtn)) < 0)
		return (rtn);

	// Followed by the page data

	buffer = (ushort *)(win->drw->buffer);
	heightcnt = win->height;
	if (ptr->mode == PTRMODE_COLOR)
	{
		memsetlong(ptr->colseedrow, 0xFFFFFFFF, ptr->colseedsz);
		while (--heightcnt >= 0)
		{
			if ((len = docolorrow(ptr, buffer)) < 0)
				whitecnt++;
			else
			{
				if (whitecnt > 0)
				{
					rtn = sprintf(cmdtxt, "%dy", whitecnt);
					if ((rtn = printout(ptr, (uchar *)cmdtxt, rtn)) < 0)
						return (rtn);
					whitecnt = 0;
				}
				if (len >= 0)
				{
					rtn = sprintf(cmdtxt, (heightcnt > 0) ? "0v%dw" : "0v%dW",
							len);
					if ((rtn = printout(ptr, (uchar *)cmdtxt, rtn)) < 0)
						return (rtn);
					if (len > 0 && (rtn = printout(ptr, ptr->genrow, len)) < 0)
						return (rtn);
				}
			}
			buffer += win->width;
		}
	}
	else
	{
		memsetlong(ptr->monseedrow, 0, ptr->monseedsz);
		while (--heightcnt >= 0)
		{
			if ((len = domonorow(ptr, buffer)) < 0)
				whitecnt++;
			else
			{
				if (whitecnt > 0)
				{
					rtn = sprintf(cmdtxt, "%dy", whitecnt);
					if ((rtn = printout(ptr, (uchar *)cmdtxt, rtn)) < 0)
						return (rtn);
					whitecnt = 0;
				}
				if (len >= 0)
				{
					rtn = sprintf(cmdtxt, (heightcnt > 0) ? "%dw" : "%dW", len);
					if ((rtn = printout(ptr, (uchar *)cmdtxt, rtn)) < 0)
						return (rtn);
					if (len > 0 && (rtn = printout(ptr, ptr->genrow, len)) < 0)
						return (rtn);
				}
			}
			buffer += win->width;
		}
	}
	if (whitecnt > 0)
	{
		if ((rtn = printout(ptr, (uchar *)"0Y", 2)) < 0)
			return (rtn);
	}

	// Output the page suffix code

	if ((rtn = printout(ptr, endpage, sizeof(endpage) - 1)) < 0)
		return (rtn);					// Leave raster mode, print the page

	// Finally, fill in the record length value. If the bytes are still in
	//   the buffer we just fill then in. If they have been written out we
	//   write the value directly to the file.

	if (ptr->lenpnt != NULL)
		*(long *)(ptr->lenpnt) = ptr->reclen;
	else
	{
		posparm.pos.value = ptr->lenpos;

///		printf("### setpos to 0x%X\n", posparm.pos.value);

		if ((rtn = svcIoOutBlockP(ptr->hndl, (char *)&ptr->reclen, 4,
				(char *)&posparm)) < 0)
		{
			printclean(ptr);
			return (rtn);
		}
		posparm.pos.value = ptr->bfrpos;

///		printf("### setpos to 0x%X\n", posparm.pos.value);

		if ((rtn = svcIoOutBlockP(ptr->hndl, NULL, 0, (char *)&posparm)) < 0)
		{
			printclean(ptr);
			return (rtn);
		}
	}
	return (0);
}


//*********************************************************
// Function: domonorow - Generate monochrome (plane 0) data
//				for one pixel row
// Returned: Length of the generated output (0 if there are
//				no differences, -1 if new row is all white
//*********************************************************

static long domonorow(
	XWSPTR *ptr,
	ushort *rowpnt)

{
	uchar *genpnt;
	uchar *bytepnt;

	long   bytecnt;
///	long   lenm1;
	long   pixcnt;
	long   inx;
	long   max;
	long   bgn;
///	long   offset;
	long   prv;
///	long   cnt;
	long   rleinx;
	long   rlebgn;
	long   rlecnt;
	uchar  rlebyte;
	uchar  seednotwhite;
	uchar  byteval;
	uchar  pixbit;

	genpnt = ptr->genrow;

	// First convert the input row to a 1-bit per pixel row in currow (same
	//   format as the seed row buffer)

	bytepnt = ptr->currow;
	bytecnt = ptr->width/8;
	seednotwhite = 0;
	while (--bytecnt >= 0)
	{
		byteval = 0;					// Construct 1 byte of pixel values
		pixcnt = 8;
		pixbit = 0x80;
		do
		{
			if (*rowpnt++ != 0xFFFF)
				byteval |= pixbit;
			pixbit >>= 1;
		} while (--pixcnt > 0);
		*bytepnt++ = byteval;
		seednotwhite |= byteval;
	}
	if (seednotwhite == 0)				// Is the entire new row white?
	{
		memsetlong(ptr->monseedrow, 0, ptr->monseedsz); // Yes - this is easy!
		return (-1);
	}

	// Here with the new row constructed in currow. Now compare it to the
	//   seed row, generate any required output, and update the seed row.

	inx = 0;
	max = ptr->width/8;
	prv = 0;
	while (inx < max)
	{
		if (ptr->currow[inx] != ptr->monseedrow[inx])
		{
			// Here with a difference between the new row and the seed row.
			//   Find the end of the difference. Note that we require two
			//   matching bytes to end the difference. The actual amount
			//   that gives minimum size is not necessarily two but depends
			//   on the length of the next difference region. Its very complex
			//   to calulate this and it does not really make much difference
			//   so always using 2 is reasonable.

			bgn = inx++;
			while (inx < (max - 1) &&
					(ptr->currow[inx] != ptr->monseedrow[inx] ||
					ptr->currow[inx+1] != ptr->monseedrow[inx + 1]))
				inx++;

			// Generate output for this difference region
			//   prv = Index of last byte output + 1
			//   bgn = Index of first byte in difference region
			//   inx = Index of first byte after difference region

			rleinx = bgn;
			do
			{
				// See if there is an RLE run here and see how long it is
				//   rlebgn = Index of first byte in run
				//   rleinx = Current index in run
				//   rlecnt = Length of run

				rlecnt = 1;
				rlebgn = rleinx;
				rlebyte = ptr->currow[rleinx++];
				while (rleinx <= inx && rlebyte == ptr->currow[rleinx])
				{
					rleinx++;
					rlecnt++;
				}

				// See if its useful to output an RLE run here. If the run
				//   is at the end of a difference region it will require
				//   1 additional header byte so its usually helpful to 
				//   use RLE for 3 or more bytes. There may be some funny
				//   special cases because of the way counts are encoded
				//   where a 3 byte run will not save any space, but it
				//   should never hurt. If the run is not at the end of a
				//   difference region it will require 2 additional header
				//   bytes so use only use RLE in this case if the run is
				//   at least 4 bytes.

				if (rlecnt >= ((rleinx < inx) ? 4 : 3))
				{
					// Here if have a run long enough to bother with.
					//   (Generating a run of 4 or more characters will
					//   (almost always) reduce the size of the output.
					//   Generating an RLE run will insert 1 or 2 more
					//   header bytes)

					if (rlebgn > bgn)
					{
						// If there was data before the run output it now

						genpnt = storemonocmd(ptr, genpnt, bgn, bgn - prv,
								rlebgn - bgn);
						prv = bgn = rlebgn;

					}

					// Output the RLE run

					genpnt = storemonorlecmd(ptr, genpnt, bgn, bgn - prv,
							rlecnt);
					prv = bgn = rleinx;
				}
			} while (rleinx < inx);

			if (bgn < inx)
			{
				// If there was data after the last RLE run or if there were
				//   no RLE runs output the data now

				genpnt = storemonocmd(ptr, genpnt, bgn, bgn - prv, inx - bgn);
				prv = inx;
			}
		}
		inx++;
	}
	return (genpnt - ptr->genrow);		// Return the length of the generated
}										//   data


//*************************************************************
// Function: storemonocmd - Output a monochrome non-RLE command
// Returned: Updated generated data pointer
//*************************************************************

static uchar *storemonocmd(
	XWSPTR *ptr,
	uchar  *genpnt,
	int     bgn,
	int     cpycnt,
	int     repcnt)

{
	int   cnt;
	uchar byteval;

	// Store the command byte

	cnt = repcnt - 1;
	*genpnt++ = (((cpycnt > 15) ? 15 : cpycnt) << 3) + ((cnt >= 7) ? 7 : cnt);

	// Store any excess copy count bytes

	if ((cpycnt -= 15) >= 0)
	{
		while (cpycnt >= 255)
		{
			*genpnt++ = 255;
			cpycnt -= 255;
		}
		*genpnt++ = cpycnt;
	}

	// Store any excess replacement count bytes

	if ((cnt -= 7) >= 0)
	{
		while (cnt >= 255)
		{
			*genpnt++ = 255;
			cnt -= 255;
		}
		*genpnt++ = cnt;
	}

	// Store the data bytes and update seedrow

	do
	{
		byteval = ptr->currow[bgn];
		*genpnt++ = byteval;
		ptr->monseedrow[bgn++] = byteval;
	} while (--repcnt > 0);
	return (genpnt);
}


// NOTE: The following function really should be in line since each is only
//       called once. It is implemented here as a function only for better
//       readability.

//***********************************************************
// Function: storemonorlecmd - Store a monochrome RLE command
// Returned: Updated generated data pointer
//***********************************************************


static uchar *storemonorlecmd(
	XWSPTR *ptr,
	uchar  *genpnt,
	int     bgn,
	int     cpycnt,
	int     repcnt)

{
	int   cnt;
	uchar byteval;

	// Store the command byte

	cnt = repcnt - 2;
	*genpnt++ = 0x80 + (((cpycnt > 3) ? 3 : cpycnt) << 5) +
			((cnt >= 31) ? 31 : cnt);

	// Store any excess copy count bytes

	if ((cpycnt -= 3) >= 0)
	{
		while (cpycnt >= 255)
		{
			*genpnt++ = 255;
			cpycnt -= 255;
		}
		*genpnt++ = cpycnt;
	}

	// Store any excess replacement count bytes

	if ((cnt -= 31) >= 0)
	{
		while (cnt >= 255)
		{
			*genpnt++ = 255;
			cnt -= 255;
		}
		*genpnt++ = cnt;
	}

	// Store the data byte and update seedrow

	byteval = ptr->currow[bgn];
	*genpnt++ = byteval;
	do
	{
		ptr->monseedrow[bgn++] = byteval;
	} while (--repcnt > 0);
	return (genpnt);
}


//*********************************************************
// Function: docolorrow - Generate color (plane 1) data for
//				one pixel row
// Returned: Length of the generated output (0 if there are
//				no differences, -1 if new row is all white
//*********************************************************

// The seed row buffer uses the same format as the drawing context buffer.
//   Thus there is no need for a current row buffer; comparasions are made
//   directly to the drawing buffer.

static long docolorrow(
	XWSPTR *ptr,
	ushort *rowpnt)

{
	uchar *genpnt;
	long   inx;
	long   max;
	long   bgn;
	long   prv;
	long   rleinx;
	long   rlebgn;
	long   rlecnt;
	ushort rlebyte;
	ushort pixel;
	ushort whiteval;

	genpnt = ptr->genrow;

	whiteval = 0xFFFF;
	inx = 0;
	max = ptr->width;
	prv = 0;
	while (inx < max)
	{
		pixel = rowpnt[inx];
		whiteval &= pixel;
		if (pixel != ptr->colseedrow[inx])
		{
			// Here with a difference between the new row and the seed row.
			//   Find the end of the difference. Note that we require two
			//   matching bytes to end the difference. The actual amount
			//   that gives minimum size is not necessarily two but depends
			//   on the length of the next difference region. Its very complex
			//   to calulate this and it does not really make much difference
			//   so always using 2 is reasonable.

			bgn = inx++;
			while (inx < (max - 1))
			{
				pixel = rowpnt[inx];
				whiteval &= pixel;
				if (pixel == ptr->colseedrow[inx] &&
						rowpnt[inx+1] == ptr->colseedrow[inx + 1])
					break;
				inx++;
			}

			// Generate output for this difference region
			//   prv = Index of last byte output + 1
			//   bgn = Index of first byte in difference region
			//   inx = Index of first byte after difference region

			rleinx = bgn;
			do
			{
				// See if there is an RLE run here and see how long it is
				//   rlebgn = Index of first byte in run
				//   rleinx = Current index in run
				//   rlecnt = Length of run

				rlecnt = 1;
				rlebgn = rleinx;
				rlebyte = rowpnt[rleinx++];
				while (rleinx <= inx && rlebyte == rowpnt[rleinx])
				{
					rleinx++;
					rlecnt++;
				}

				// See if its useful to output an RLE run here. If the run
				//   is at the end of a difference region it will require
				//   1 additional header byte so it's usually helpful to 
				//   use RLE for 3 or more bytes. There may be some funny
				//   special cases because of the way counts are encoded
				//   where a 3 byte run will not save any space, but it
				//   should never hurt. If the run is not at the end of a
				//   difference region it will require 2 additional header
				//   bytes so we only use RLE in this case if the run is
				//   at least 4 bytes.

				if (rlecnt >= ((rleinx < inx) ? 4 : 3))
				{
					// Here if have a run long enough to bother with.

					if (rlebgn > bgn)
					{
						// If there was data before the run output it now

						genpnt = storecolorcmd(ptr, genpnt, rowpnt, bgn,
								bgn - prv, rlebgn - bgn);
						prv = bgn = rlebgn;

					}

					// Output the RLE run

					genpnt = storecolorrlecmd(ptr, genpnt, rowpnt[bgn], bgn,
							bgn - prv, rlecnt);
					prv = bgn = rleinx;
				}
			} while (rleinx < inx);

			if (bgn < inx)
			{
				// If there was data after the last RLE run or if there were
				//   no RLE runs output the data now

				genpnt = storecolorcmd(ptr, genpnt, rowpnt, bgn, bgn - prv,
						inx - bgn);
				prv = inx;
			}
		}
		inx++;
	}
	return ((whiteval == 0xFFFF) ? -1 : (genpnt - ptr->genrow));
										// Return the length of the generated
}										//   data or -1 if line is completely
										//   white


//*********************************************************
// Function: storecolorcmd - Output a color non-RLE command
// Returned: Updated generated data pointer
//*********************************************************

static uchar *storecolorcmd(
	XWSPTR *ptr,
	uchar  *genpnt,
	ushort *rowpnt,
	int     bgn,
	int     cpycnt,
	int     repcnt)

{
	int cnt;
	int pixel;

	// Store the command byte

	cnt = repcnt - 1;
	*genpnt++ = (((cpycnt > 3) ? 3 : cpycnt) << 3) + ((cnt >= 7) ? 7 : cnt);

	// Store any excess copy count bytes

	if ((cpycnt -= 3) >= 0)
	{
		while (cpycnt >= 255)
		{
			*genpnt++ = 255;
			cpycnt -= 255;
		}
		*genpnt++ = cpycnt;
	}

	// Unlike mode 9, mode 10 distributes the excess replacement count
	//   bytes throughout  the data. A count byte is output when the count
	//   from the privious byte has been completely used.

	cnt =  8;
	while (--repcnt >= 0)
	{
		pixel = rowpnt[bgn];
		ptr->colseedrow[bgn++] = pixel;	// Update the seed row
		genpnt = storepixel(genpnt, pixel); // Store pixel in the output
		if (--cnt <= 0)					// Need an count extension byte?
		{
			*genpnt++ = (repcnt >= 255) ? 255 : repcnt; // Yes
			cnt = 255;
		}
	}
	return (genpnt);
}


//*******************************************************
// Function: storecolorrlecmd - Store a color RLE command
// Returned: Updated generated data pointer
//*******************************************************


static uchar *storecolorrlecmd(
	XWSPTR *ptr,
	uchar  *genpnt,
	int     pixel,
	int     bgn,
	int     cpycnt,
	int     repcnt)

{
	int   cnt;

	// Store the command byte

	cnt = repcnt - 2;
	*genpnt++ = 0x80 + (((cpycnt >= 3) ? 3 : cpycnt) << 3) +
			((cnt >= 7) ? 7 : cnt);

	// Store any excess copy count bytes

	if ((cpycnt -= 3) >= 0)
	{
		while (cpycnt >= 255)
		{
			*genpnt++ = 255;
			cpycnt -= 255;
		}
		*genpnt++ = cpycnt;
	}

	// Store the data byte and update seedrow

	genpnt = storepixel(genpnt, pixel);	// Convert pixel to printer format
	do									//   and store it in the output line
	{
		ptr->colseedrow[bgn++] = pixel;
	} while (--repcnt > 0);

	// Store any excess replacement count bytes

	if ((cnt -= 7) >= 0)
	{
		while (cnt >= 255)
		{
			*genpnt++ = 255;
			cnt -= 255;
		}
		*genpnt++ = cnt;
	}
	return (genpnt);
}


//****************************************************
// Function: storepixel - Convert pixel value from XWS
//				internal format to printer format and
//				store it in the printer line buffer
// Returend: Updated genpnt value
//****************************************************

static uchar *storepixel(
	uchar *genpnt,
	int     pixel)

{
	long value;
	int  red;
	int  green;
	int  blue;

	// First get the color values and check for maximum values so we get a
	//   pure white

	if ((red = ((pixel >> 8) & 0xF8)) == 0xF8)
		red = 0xFF;
	if ((green = ((pixel >> 3) & 0xFC)) == 0xFC)
		green = 0xFF;
	if ((blue = ((pixel << 2) & 0x7C)) == 0x7C) // Blue is only 7 bits
		blue = 0x7F;

	// Combine the colors into a 32-bit value shifted right 1 bit to get
	//   the colors in the funny positions the printer expects

	value = (red << 15) + (green << 7) + blue;

	// Finally store the pixel values

	genpnt[0] = (uchar)(value >> 16);
	genpnt[1] = (uchar)(value >> 8);
	genpnt[2] = (uchar)value;
	return (genpnt + 3);
}


/*
void ptrststhread(void)
	

{
	long rtn;
	char stsbufr[256];

	printf("in ptrststhread, hndl = %d\n", thdptr->hndl);

	while (TRUE)
	{


		if ((rtn = thdIoInBlock(thdptr->hndl, stsbufr,
				sizeof(stsbufr) - 1)) < 0)
		{
			BREAK();
		}	
		if (rtn > 0)
		{
			stsbufr[rtn] = 0;
			printf("STS: %s", stsbufr);
		}
		else
			thdCtlSuspendT(NULL, 0, ST_SECOND/4);

	}
}
*/


//**********************************************************
// Function: printout - Output data to the printer
// Returned: 0 if normal or negative XOS error code if error
//**********************************************************

static long printout(
	XWSPTR *ptr,
	uchar  *buffer,
	long    len)
{
	long amnt;
	long rtn;

	ptr->reclen += len;
	while (len > 0)
	{
		if (ptr->outcnt <= 0)
		{
			if ((rtn = fbrIoOutBlock(ptr->hndl, (char *)ptr->outbfr,
					OUTBFRSZ)) < 0)
			{
				printclean(ptr);
				return (rtn);
			}

///			fbrIoOutBlock(xxxx, ptr->outbfr, OUTBFRSZ);

			ptr->lenpnt = NULL;
			ptr->bfrpos += OUTBFRSZ;
			ptr->outpnt = ptr->outbfr;
			ptr->outcnt = OUTBFRSZ;
		}
		if ((amnt = len) > ptr->outcnt)
			amnt = ptr->outcnt;
		memcpy(ptr->outpnt, buffer, amnt);
		ptr->outpnt += amnt;
		ptr->outcnt -= amnt;
		buffer += amnt;
		len -= amnt;
	}
	return (0);
}


static long printflush(
	XWSPTR *ptr)
{
	long amnt;
	long rtn;

	if ((amnt = (OUTBFRSZ - ptr->outcnt)) > 0)
	{
		if ((rtn = fbrIoOutBlock(ptr->hndl, (char *)ptr->outbfr, amnt)) < 0)
		{
			printclean(ptr);
			return (rtn);
		}

///		fbrIoOutBlock(xxxx, ptr->outbfr, amnt);

		ptr->outpnt = ptr->outbfr;
		ptr->outcnt = OUTBFRSZ;
	}
	return (0);
}


static void printclean(
	XWSPTR *ptr)
{
	if (ptr->hndl > 0)
		svcIoClose(ptr->hndl, 0);

///	if (xxxx > 0)
///	{
///		svcIoClose(xxxx, 0);
///		xxxx = 0;
///	}

	if (ptr->monseedrow != NULL)
		xwsFree(ptr->monseedrow);
	if (ptr->colseedrow != NULL)
		xwsFree(ptr->colseedrow);
	if (ptr->currow != NULL)
		xwsFree(ptr->currow);
	if (ptr->genrow != NULL)
		xwsFree(ptr->genrow);
	if (ptr->outbfr != NULL)
		xwsFree(ptr->outbfr);
	xwsFree(ptr);
}
