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

// This module contains routines for loading PNG image files/

//********************************************************************
// Function: xwsbitmaploadpng - Create a BITMAP object from a PNG file
// Returned: Address of a BITMAP object if successful or NULL if an
//				error occured
//********************************************************************

// This function uses the Pascal calling sequence

// When the caller is finished with the BITMAP object, it should be 
//   deallocated by calling xwsBitmapFree.

long xwsbitmaploadpng(
	FILE       *infile,			// Input file stream
	XWSWIN     *win,			// WIN object
	long        flags,			// Flag bits
	XWSBITMAP **bm)

{
	win = win;
	flags = flags;
	bm = bm;

	fclose(infile);
	return (ER_IDFMT);
}
