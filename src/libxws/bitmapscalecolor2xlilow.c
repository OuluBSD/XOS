#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"
#include "swcscale.h"

// The following code is derived from the Leptonica graphics library. It
//   has been modified significantly.

//====================================================================
//  Copyright (C) 2001 Leptonica.  All rights reserved.
//  This software is distributed in the hope that it will be
//  useful, but with NO WARRANTY OF ANY KIND.
//  No author or distributor accepts responsibility to anyone for the
//  consequences of using this software, or for whether it serves any
//  particular purpose or works at all, unless he or she says so in
//  writing.  Everyone is granted permission to copy, modify and
//  redistribute this source code, for commercial or non-commercial
//  purposes, with the following restrictions: (1) the origin of this
//  source code must not be misrepresented; (2) modified versions must
//  be plainly marked as such; and (3) this notice may not be removed
//  or altered from any source or modified source distribution.
//====================================================================

// We don't really expect to use this so we will remove it to eliminate
//   a source of errors!  We may want to put it back later, so leave
//   source here.

/*------------------------------------------------------------------*
 *                2x linear interpolated color scaling              *
 *------------------------------------------------------------------*/
/*!
 *  scaleColor2xLILow()
 *
 *  This is a special case of 2x expansion by linear
 *  interpolation.  Each src pixel contains 4 dest pixels.
 *  The 4 dest pixels in src pixel 1 are numbered at
 *  their UL corners.  The 4 dest pixels in src pixel 1
 *  are related to that src pixel and its 3 neighboring
 *  src pixels as follows:
 *
 *             1-----2-----|-----|-----|
 *             |     |     |     |     |
 *             |     |     |     |     |
 *  src 1 -->  3-----4-----|     |     |  <-- src 2
 *             |     |     |     |     |
 *             |     |     |     |     |
 *             |-----|-----|-----|-----|
 *             |     |     |     |     |
 *             |     |     |     |     |
 *  src 3 -->  |     |     |     |     |  <-- src 4
 *             |     |     |     |     |
 *             |     |     |     |     |
 *             |-----|-----|-----|-----|
 *
 *           dest      src
 *           ----      ---
 *           dp1    =  sp1
 *           dp2    =  (sp1 + sp2) / 2
 *           dp3    =  (sp1 + sp3) / 2
 *           dp4    =  (sp1 + sp2 + sp3 + sp4) / 4
 *
 *  We iterate over the src pixels, and unroll the calculation
 *  for each set of 4 dest pixels corresponding to that src
 *  pixel, caching pixels for the next src pixel whenever possible.
 *  The method is exactly analogous to the one we use for
 *  scaleGray2xLILow() and its line version.
 *
 *  P3 speed is about 5 x 10^7 dst pixels/sec/GHz
 */

/* static */ void swcImage::scalecolor2xlilow(
	ulong  *datad,
	long    wd,
	ulong  *datas,
	long    ws,
	long    hs)

{
	l_int32  i, hsm;
	ulong   *lines, *lined;

    hsm = hs - 1;

	// We're taking 2 src and 2 dest lines at a time, and for each src line,
	//   we're computing 2 dest lines. Call these 2 dest lines: destline1 and
	//   destline2. The first src line is used for destline 1. On all but the
	//   last src line, both src lines are  used in the linear interpolation
	//   for destline2. On the last src line, both destline1 and destline2 are
	//   computed using only that src line (because there isn't a lower src
	//   line).

	// Iterate over all but the last src line

    for (i = 0; i < hsm; i++)
	{
		lines = datas + i * ws;
		lined = datad + 2 * i * wd;
		scaleColor2xLILineLow((uint *)lined, wd, (uint *)lines, ws, 0);
		if (xws_SchedReq > 0)
			xws_SchedReq--;
    }
    
    // Last src line

    lines = datas + hsm * ws;
    lined = datad + 2 * hsm * wd;
    scaleColor2xLILineLow((uint *)lined, wd, (uint *)lines, ws, 1);
    return;
}


/*!
 *  scaleColor2xLILineLow()
 *
 *      Input:  lined   (ptr to top destline, to be made from current src line)
 *              wpld
 *              lines   (ptr to current src line)
 *              ws
 *              wpls
 *              lastlineflag  (1 if last src line; 0 otherwise)
 *      Return: void
 *
 *  *** Warning: explicit assumption about RGB component ordering ***
 */

void
scaleColor2xLILineLow(l_uint32  *lined,
                      l_int32    wd,
	 	      l_uint32  *lines,
	 	      l_int32    ws,
		      l_int32    lastlineflag)
{
l_int32    j, jd, wsm;
l_int32    rval1, rval2, rval3, rval4, gval1, gval2, gval3, gval4;
l_int32    bval1, bval2, bval3, bval4;
l_uint32   pixels1, pixels2, pixels3, pixels4, pixel;
l_uint32  *linesp, *linedp;

    wsm = ws - 1;

    if (lastlineflag == 0) {
        linesp = lines + ws;
        linedp = lined + wd;
        pixels1 = *lines;
	pixels3 = *linesp;

	    /* initialize with v(2) and v(4) */
	rval2 = pixels1 >> 24;
	gval2 = (pixels1 >> 16) & 0xff;
	bval2 = (pixels1 >> 8) & 0xff;
	rval4 = pixels3 >> 24;
	gval4 = (pixels3 >> 16) & 0xff;
	bval4 = (pixels3 >> 8) & 0xff;

	for (j = 0, jd = 0; j < wsm; j++, jd += 2) {
	        /* shift in previous src values */
	    rval1 = rval2;
	    gval1 = gval2;
	    bval1 = bval2;
	    rval3 = rval4;
	    gval3 = gval4;
	    bval3 = bval4;
	        /* get new src values */
	    pixels2 = *(lines + j + 1);
	    pixels4 = *(linesp + j + 1);
	    rval2 = pixels2 >> 24;
	    gval2 = (pixels2 >> 16) & 0xff;
	    bval2 = (pixels2 >> 8) & 0xff;
	    rval4 = pixels4 >> 24;
	    gval4 = (pixels4 >> 16) & 0xff;
	    bval4 = (pixels4 >> 8) & 0xff;
	        /* save dest values */
	    pixel = (rval1 << 24 | gval1 << 16 | bval1 << 8);
	    *(lined + jd) = pixel;                               /* pix 1 */
	    pixel = (((rval1 + rval2) << 23) & 0xff000000 |
		     ((gval1 + gval2) << 15) & 0x00ff0000 |
		     ((bval1 + bval2) << 7) & 0x0000ff00);
	    *(lined + jd + 1) = pixel;                           /* pix 2 */
	    pixel = (((rval1 + rval3) << 23) & 0xff000000 |
		     ((gval1 + gval3) << 15) & 0x00ff0000 |
		     ((bval1 + bval3) << 7) & 0x0000ff00);
	    *(linedp + jd) = pixel;                              /* pix 3 */
	    pixel = (((rval1 + rval2 + rval3 + rval4) << 22) & 0xff000000 | 
		     ((gval1 + gval2 + gval3 + gval4) << 14) & 0x00ff0000 |
		     ((bval1 + bval2 + bval3 + bval4) << 6) & 0x0000ff00);
	    *(linedp + jd + 1) = pixel;                          /* pix 4 */
        }  
	    /* last src pixel on line */
	rval1 = rval2;
	gval1 = gval2;
	bval1 = bval2;
	rval3 = rval4;
	gval3 = gval4;
	bval3 = bval4;
	pixel = (rval1 << 24 | gval1 << 16 | bval1 << 8);
	*(lined + 2 * wsm) = pixel;                        /* pix 1 */
	*(lined + 2 * wsm + 1) = pixel;                    /* pix 2 */
	pixel = (((rval1 + rval3) << 23) & 0xff000000 |
		 ((gval1 + gval3) << 15) & 0x00ff0000 |
		 ((bval1 + bval3) << 7) & 0x0000ff00);
	*(linedp + 2 * wsm) = pixel;                       /* pix 3 */
	*(linedp + 2 * wsm + 1) = pixel;                   /* pix 4 */
    }
    else {   /* last row of src pixels: lastlineflag == 1 */
	linedp = lined + wd;
	pixels2 = *lines;
	rval2 = pixels2 >> 24;
	gval2 = (pixels2 >> 16) & 0xff;
	bval2 = (pixels2 >> 8) & 0xff;
	for (j = 0, jd = 0; j < wsm; j++, jd += 2) {
	    rval1 = rval2;
	    gval1 = gval2;
	    bval1 = bval2;
	    pixels2 = *(lines + j + 1);
	    rval2 = pixels2 >> 24;
	    gval2 = (pixels2 >> 16) & 0xff;
	    bval2 = (pixels2 >> 8) & 0xff;
	    pixel = (rval1 << 24 | gval1 << 16 | bval1 << 8);
	    *(lined + jd) = pixel;                            /* pix 1 */
	    *(linedp + jd) = pixel;                           /* pix 2 */
	    pixel = (((rval1 + rval2) << 23) & 0xff000000 |
		     ((gval1 + gval2) << 15) & 0x00ff0000 |
		     ((bval1 + bval2) << 7) & 0x0000ff00);
	    *(lined + jd + 1) = pixel;                        /* pix 3 */
	    *(linedp + jd + 1) = pixel;                       /* pix 4 */
	}  
	rval1 = rval2;
	gval1 = gval2;
	bval1 = bval2;
	pixel = (rval1 << 24 | gval1 << 16 | bval1 << 8);
	*(lined + 2 * wsm) = pixel;                           /* pix 1 */
	*(lined + 2 * wsm + 1) = pixel;                       /* pix 2 */
	*(linedp + 2 * wsm) = pixel;                          /* pix 3 */
	*(linedp + 2 * wsm + 1) = pixel;                      /* pix 4 */
    }
	
    return;
}
