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
#include "jpeglib.h"

// This module contains a function for creating a BITMAP object from any
//   supported image file.

static uchar strjpeg[] = {0xFF, 0xD8, 0xFF};
static uchar strbmp[] = {'B', 'M'};
static uchar strpng[] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
///static uchar strgif[] = {'G', 'I', 'F'};


//********************************************************************
// Function: xwsBitmapLoad - Create a BITMAP object from an image file
// Returned: Address of a BITMAP object if successful or NULL if an
//				error occured
//********************************************************************

// This function uses the Pascal calling sequence

// When the caller is finished with the BITMAP object, it should be 
//   deallocated by calling xwsBitmapFree.

long xwsBitmapLoad(
	char       *filename,		// File specification
	XWSWIN     *win,			// WIN object
	long        flags,			// Flag bits
	XWSBITMAP **bm)
{
	FILE *infile;
	char bufr[8];

	if ((infile = fopen(filename, "rb")) == NULL ||
			fread(bufr, 1, 8, infile) != 8 ||
			fseek(infile, 0, 0) < 0)
	{
		if (errno == 0)
			errno = -ER_EOF;
		return (-errno);
	}	
	if (memcmp(bufr, strjpeg, 3) == 0)
		return (xwsbitmaploadjpeg(infile, win, flags, bm));
	else if (memcmp(bufr, strbmp, 2) == 0)
		return (xwsbitmaploadbmp(infile, win, flags, bm));

	else if (memcmp(bufr, strpng, 8) == 0)
		return (xwsbitmaploadpng(infile, win, flags, bm));

///	else if (memcmp(bufr, strgif, 3) == 0)
///		return (xwsbitmaploadgif(infile, win, flags, bm));

	fclose(infile);
	return (ER_IDFMT);
}
