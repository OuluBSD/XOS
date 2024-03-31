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
#include <xosfibers.h>
#include "jpeglib.h"

// This module contains routines for loading JPEG image files.


//**********************************************************************
// Function: xwsbitmaploadjpeg - Create a BITMAP object from a JPEG file
// Returned: Address of a BITMAP object if successful or NULL if an
//				error occured
//**********************************************************************

// This function uses the Pascal calling sequence

// When the caller is finished with the BITMAP object, it should be 
//   deallocated by calling xwsBitmapFree.

long xwsbitmaploadjpeg(
	FILE       *infile,			// Input file stream
	XWSWIN     *win,			// WIN object
	long        flags,			// Flag bits
	XWSBITMAP **bm)

{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr         jerr;

	XWSBITMAP *bitmap;
	ushort    *pixpnt;
	uchar     *wbufr;
	uchar     *linepnt;
	long      *jpegpnt;
	uchar     *linearray[10];
	long       pixel;
	long       rtn;
	int        linecnt;
	int        pixcnt;

	win = win;
	flags = flags;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);		// Read the header data
	jpeg_start_decompress(&cinfo);		// Process the header data

	// Create the unfilled bitmap. Also allocate our working buffer which
	//   jpeg_read_scanlines will fill.

	if ((rtn = xwsbitmapnew(cinfo.output_width, cinfo.output_height,
			&bitmap)) < 0)
	{
		fclose(infile);
		return (rtn);
	}
	if ((wbufr = xwsMalloc(cinfo.output_width * 40)) == NULL)
	{
		xwsFree(bitmap);
		fclose(infile);
		return (ER_NEMA);
	}
	linecnt = 0;						// Initialize the line pointer array
	linepnt = wbufr;
	do
	{
		linearray[linecnt] = linepnt;
		linepnt += (cinfo.output_width * 4);
	} while (++linecnt < 10);

	// ADD CHECK HERE FOR DEFERED LOADING!!!

	pixpnt = (ushort *)(bitmap->bitmap);
	while (cinfo.output_scanline < cinfo.output_height)
	{
		linecnt = jpeg_read_scanlines(&cinfo, linearray, 10);
		linepnt = wbufr;
		while (--linecnt >= 0)
		{
			jpegpnt = (long *)linepnt;
			pixcnt = cinfo.output_width;
			while (--pixcnt >= 0)
			{
				pixel = *jpegpnt++;
				*pixpnt++ = ((pixel >> 19) & 0x001F) | ((pixel >> 5) & 0x07E0) |
						((pixel << 8) & 0xF800);
			}
			linepnt += (cinfo.output_width * 4);
		}
		if (xws_SchedReq > 0)
			fbrCtlRelease();
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
	xwsFree(wbufr);
	bitmap->status |= BMS_DONE;			// Indicate that its fully loaded
	*bm = bitmap;
	return (0);
}
