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
#include <giflib.h>

// This module contains routines for loading GIF image files/

static int interlacedoffset[] = {0, 4, 2, 1};
static int interlacedjumps[] = {8, 8, 4, 2};


//********************************************************************
// Function: xwsbitmaploadgif - Create a BITMAP object from a GIF file
// Returned: Address of a BITMAP object if successful or NULL if an
//				error occured
//********************************************************************

// This function uses the Pascal calling sequence

// When the caller is finished with the BITMAP object, it should be 
//   deallocated by calling xwsBitmapFree.

long xwsbitmaploadgif(
	FILE       *infile,			// Input file stream
	XWSWIN     *win,			// WIN object
	long        flags,			// Flag bits
	XWSBITMAP **bm)

{
	XWSBITMAP    *bitmap;
	GifFileType  *gifobj;
	GifByteType  *extension;
	long         *gcmap;
	uchar        *linepnt;
	ushort       *pixpnt;
	uchar        *wbufr;
	GifRecordType rectype;
	long          pixel;
	int           extcode;
	int           bsize;
	int           cnt;
	int           width;
	int           height;
	int           row;
	int           col;
	int           i;
	int           j;

	win = win;
	flags = flags;

	if ((gifobj = DGifBeginFile(infile)) == NULL)
	{
		fclose(infile);
		return (ER_ERROR);
	}

	cnt = gifobj->SColorMap->ColorCount;
	gcmap = (long *)(gifobj->SColorMap->Colors);
	while (--cnt >= 0)					// Convert the color map to our
	{									//   internal format
		pixel = *gcmap;
		*gcmap++ = ((pixel >> 19) & 0x001F) | ((pixel >> 5) & 0x07E0) |
				((pixel << 8) & 0xF800);
	}
	gcmap = (long *)(gifobj->SColorMap->Colors);

	// Determine the size of our BITMAP object and allocate its memory.
	//   Also allocate our working buffer which jpeg_read_scanlines will
	//   fill.

	bsize = gifobj->SWidth * gifobj->SHeight * 2;
	if ((bitmap = (XWSBITMAP *)xwsMalloc(bsize + offsetof(XWSBITMAP,
			bitmap))) == NULL)
	{
		fclose(infile);
		return (ER_NEMA);
	}
	if ((wbufr = xwsMalloc(gifobj->SWidth)) == NULL)
	{
		xwsFree(bitmap);
		fclose(infile);
		return (ER_NEMA);
	}

	// Set up the BITMAP object header

	bitmap->label = 'BM##';
///	bitmap->fnot = NULL;
	bitmap->status = 0;
	bitmap->drw.label = 'DRW#';
	bitmap->drw.buffer = bitmap->bitmap;
	bitmap->drw.width = gifobj->SWidth;
	bitmap->drw.pixelspan = gifobj->SWidth;
	bitmap->drw.bytespan = gifobj->SWidth * 2;
	bitmap->drw.height = gifobj->SHeight;
	bitmap->drw.bufsize = bsize;
	bitmap->drw.funcdsp = &xwsmemdsp;

	// ADD CHECK HERE FOR DEFERED LOADING!!!

	do
	{
		if (DGifGetRecordType(gifobj, &rectype) == GIF_ERROR)
		{
			free(bitmap);
			free(wbufr);
			fclose(infile);
			return (ER_IDFMT);
		}
		switch (rectype)
		{
		 case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(gifobj) == GIF_ERROR)
			{
				free(bitmap);
				free(wbufr);
				fclose(infile);
				return (ER_IDFMT);
			}
			row = gifobj->Image.Top;
			col = gifobj->Image.Left;
			width = gifobj->Image.Width;
			height = gifobj->Image.Height;
			pixpnt = ((ushort *)(bitmap->bitmap)) + row * width + col;
			if (gifobj->Image.Interlace)
			{
				// Here if interleaved image

				for (i = 0; i < 4; i++)
				{
					for (j = row + interlacedoffset[i]; j < row + height;
							j += interlacedjumps[i])
					{
						if (DGifGetLine(gifobj, wbufr, width) == GIF_ERROR)
						{
							free(bitmap);
							free(wbufr);
							fclose(infile);
							return (ER_IDFMT);
						}
					}
				}
			}
			else
			{
				for (i = 0; i < height; i++)
				{
					if ((DGifGetLine(gifobj, wbufr, width)) == GIF_ERROR)
					{
						free(bitmap);
						free(wbufr);
						fclose(infile);
						return (ER_IDFMT);
					}
					cnt = width;
					linepnt = wbufr;
					while (--cnt >= 0)
						*pixpnt++ = gcmap[*linepnt++];

				}
			}
			break;

		 case EXTENSION_RECORD_TYPE:
			if (DGifGetExtension(gifobj, &extcode, &extension) == GIF_ERROR)
			{
				free(bitmap);
				free(wbufr);
				fclose(infile);
				return (ER_IDFMT);
			}
			while (extension != NULL)
			{
				if (DGifGetExtensionNext(gifobj, &extension) == GIF_ERROR)
				{
					free(bitmap);
					free(wbufr);
					fclose(infile);
					return (ER_IDFMT);
				}
			}
			break;

		 case TERMINATE_RECORD_TYPE:
			break;

		 default:
			break;
		}
	} while (rectype != TERMINATE_RECORD_TYPE);

	free(wbufr);
	fclose(infile);
	*bm = bitmap;
	return (0);
}
