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

/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

//------------------------------------------------------------------*
//            General linear interpolated color scaling             *
//------------------------------------------------------------------*/

//  scaleColorLILow()
//
//  We choose to divide each pixel into 16 x 16 sub-pixels.
//  Linear interpolation is equivalent to finding the 
//  fractional area (i.e., number of sub-pixels divided
//  by 256) associated with each of the four nearest src pixels,
//  and weighting each pixel value by this fractional area.
//
//  P3 speed is about 7 x 10^6 dst pixels/sec/GHz
//
//  *** Warning: explicit assumption about RGB component ordering ***

void swcImage::scalecolorlilow(
	ulong *datad,
	long   wd,
	long   hd,
	ulong *datas,
	long   ws,
	long   hs)

{
	l_int32    i, j, wm2, hm2;
	l_int32    xpm, ypm;  /* location in src image, to 1/16 of a pixel */
	l_int32    xp, yp, xf, yf;  /* src pixel and pixel fraction coordinates */
	l_int32    v00r, v01r, v10r, v11r, v00g, v01g, v10g, v11g;
	l_int32    v00b, v01b, v10b, v11b, area00, area01, area10, area11;
	l_uint32   pixels1, pixels2, pixels3, pixels4, pixel;
	ulong     *lines, *lined;
	l_float32  scx, scy;

	/* (scx, scy) are scaling factors that are applied to the
	 * dest coords to get the corresponding src coords.
	 * We need them because we iterate over dest pixels
	 * and must find the corresponding set of src pixels. */

    scx = (float)(16. * (l_float32)ws / (l_float32)wd);
    scy = (float)(16. * (l_float32)hs / (l_float32)hd);

    wm2 = ws - 2;
    hm2 = hs - 2;

	/* iterate over the destination pixels */

    for (i = 0; i < hd; i++)
	{
		ypm = (l_int32)(scy * (l_float32)i + 0.5);
		yp = ypm >> 4;
		yf = ypm & 0x0f;
		lined = datad + i * wd;
		lines = datas + yp * ws;
		for (j = 0; j < wd; j++)
		{
			xpm = (l_int32)(scx * (l_float32)j + 0.5);
			xp = xpm >> 4;
			xf = xpm & 0x0f;

			/* if near the edge, just use the src pixel value */
			if (xp > wm2 || yp > hm2)
			{
				*(lined + j) = *(lines + xp);
				continue;
			}

			/* do bilinear interpolation.  This is a simple
			 * generalization of the calculation in scaleGrayLILow() */

            pixels1 = *(lines + xp);		 
            pixels2 = *(lines + xp + 1);		 
            pixels3 = *(lines + ws + xp);		 
            pixels4 = *(lines + ws + xp + 1);		 
			area00 = (16 - xf) * (16 - yf);
			area10 = xf * (16 - yf);
			area01 = (16 - xf) * yf;
			area11 = xf * yf;

/*			v00r = area00 * (pixels1 >> 24);
			v00g = area00 * ((pixels1 >> 16) & 0xff);
			v00b = area00 * ((pixels1 >> 8) & 0xff);
			v10r = area10 * (pixels2 >> 24);
			v10g = area10 * ((pixels2 >> 16) & 0xff);
			v10b = area10 * ((pixels2 >> 8) & 0xff);
			v01r = area01 * (pixels3 >> 24);
			v01g = area01 * ((pixels3 >> 16) & 0xff);
			v01b = area01 * ((pixels3 >> 8) & 0xff);
			v11r = area11 * (pixels4 >> 24);
			v11g = area11 * ((pixels4 >> 16) & 0xff);
			v11b = area11 * ((pixels4 >> 8) & 0xff);
			pixel = (((v00r + v10r + v01r + v11r + 128) << 16) & 0xff000000) |
	            (((v00g + v10g + v01g + v11g + 128) << 8) & 0x00ff0000) |
	            ((v00b + v10b + v01b + v11b + 128) & 0x0000ff00);
*/

			v00r = area00 * ((pixels1 >> 16) & 0xFF);
			v00g = area00 * ((pixels1 >> 8) & 0xFF);
			v00b = area00 * (pixels1 & 0xFF);
			v10r = area10 * ((pixels2 >> 16) & 0xFF);
			v10g = area10 * ((pixels2 >> 8) & 0xFF);
			v10b = area10 * (pixels2 & 0xFF);
			v01r = area01 * ((pixels3 >> 16) & 0xFF);
			v01g = area01 * ((pixels3 >> 8) & 0xFF);
			v01b = area01 * (pixels3 & 0xFF);
			v11r = area11 * ((pixels4 >> 16) & 0xFF);
			v11g = area11 * ((pixels4 >> 8) & 0xFF);
			v11b = area11 * (pixels4 & 0xFF);
			pixel = (((v00r + v10r + v01r + v11r + 128) << 8) & 0x00FF0000) |
	            ((v00g + v10g + v01g + v11g + 128) & 0x0000FF00) |
	            (((v00b + v10b + v01b + v11b + 128) >> 8) & 0x000000FF);

            *(lined + j) = pixel;
		}
    }
    return;
}

#if 0									// We don't really expect to use this
										//   so we will remove it to eliminate
										//   a source of errors!  We may want
										//   to put it back later, so leave
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

	/* We're taking 2 src and 2 dest lines at a time,
	 * and for each src line, we're computing 2 dest lines.
	 * Call these 2 dest lines:  destline1 and destline2.
	 * The first src line is used for destline 1.
	 * On all but the last src line, both src lines are 
	 * used in the linear interpolation for destline2.
	 * On the last src line, both destline1 and destline2
	 * are computed using only that src line (because there
	 * isn't a lower src line). */

	/* iterate over all but the last src line */
    for (i = 0; i < hsm; i++)
	{
		lines = datas + i * ws;
		lined = datad + 2 * i * wd;
		scaleColor2xLILineLow((uint *)lined, wd, (uint *)lines, ws, 0);
    }
    
    /* last src line */

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
#endif


/*------------------------------------------------------------------*
 *    Color and grayscale downsampling with antialias (smoothing)   *
 *------------------------------------------------------------------*/
/*!
 *  scaleSmoothLow()
 *
 *  Notes:
 *      (1) This function is called on 8 or 32 bpp src and dest images.
 *      (2) size is the full width of the smoothing filter.
 *          It is correlated with the reduction ratio, being the
 *          nearest integer such that size is approximately equal to hs / hd.
 *
 *  *** Warning: explicit assumption about RGB component ordering ***
 */

l_int32 scaleSmoothLow(
	l_uint32  *datad,
    l_int32    wd,
    l_int32    hd,
    l_uint32  *datas,
    l_int32    ws,
    l_int32    hs,
    l_int32    size)

{
	l_int32    i, j, m, n, xstart;
	l_int32    rval, gval, bval;
	l_int32   *srow, *scol;
	l_uint32  *lines, *lined, *ppixel;
	l_uint32   pixel;
	l_float32  wratio, hratio, norm;

	/* clear dest */
    memset((char *)datad, 0, 4 * wd * hd);
    
	/* Each dest pixel at (j,i) is computed as the average
	   of size^2 corresponding src pixels.
	   We store the UL corner location of the square of
	   src pixels that correspond to dest pixel (j,i).
	   The are labelled by the arrays srow[i] and scol[j]. */

    if ((srow = (l_int32 *)calloc(hd, sizeof(l_int32))) == NULL)
		return 1;
    if ((scol = (l_int32 *)calloc(wd, sizeof(l_int32))) == NULL)
		return 1;

    norm = (float)(1. / (l_float32)(size * size));
    wratio = (l_float32)ws / (l_float32)wd;
    hratio = (l_float32)hs / (l_float32)hd;
    for (i = 0; i < hd; i++)
		srow[i] = MIN((l_int32)(hratio * i + 0.5), hs - size);
    for (j = 0; j < wd; j++)
		scol[j] = MIN((l_int32)(wratio * j + 0.5), ws - size);

	/* For each dest pixel, compute average */

	for (i = 0; i < hd; i++)
	{
		lines = datas + srow[i] * ws;
		lined = datad + i * wd;
		for (j = 0; j < wd; j++)
		{
			xstart = scol[j];
			rval = gval = bval = 0;
			for (m = 0; m < size; m++)
			{
				ppixel = lines + m * ws + xstart;
				for (n = 0; n < size; n++)
				{
					pixel = *(ppixel + n);

///					rval += (pixel >> 24);
///					gval += (pixel >> 16 & 0xff);
///					bval += (pixel >> 8 & 0xff);

					rval += ((pixel >> 16) & 0xFF);
					gval += ((pixel >> 8) & 0xFF);
					bval += (pixel & 0xFF);
				}
			}
			rval = (l_int32)((l_float32)rval * norm);
			gval = (l_int32)((l_float32)gval * norm);
			bval = (l_int32)((l_float32)bval * norm);
			*(lined + j) = rval << 16 | gval << 8 | bval;
		}
	}
    free((void *)srow);
    free((void *)scol);
    return 0;
}
