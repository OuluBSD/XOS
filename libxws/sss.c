#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <xosstr.h>
#include <xostrm.h>
#include <xosthreads.h>
#include <xosxws.h>
#include <xosxxws.h>

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

long xwsbitmapscalesmoothlow(
	XWSDRW *drw,
	ushort *datad,
    long    wd,
    long    hd,
    ushort *datas,
    long    ws,
    long    hs,
    long    size)
{
	long    i, j, m, n, xstart;
	long    rval, gval, bval;
	ushort *srow, *scol;
	ushort *lines, *lined, *ppixel;
	long    pixel;
	float   wratio, hratio, norm;

	drw = drw;

	/* clear dest */
    memset(datad, 0, 2 * wd * hd);
    
	/* Each dest pixel at (j,i) is computed as the average
	   of size^2 corresponding src pixels.
	   We store the UL corner location of the square of
	   src pixels that correspond to dest pixel (j,i).
	   The are labelled by the arrays srow[i] and scol[j]. */

    if ((srow = (ushort *)calloc(hd, sizeof(ushort))) == NULL)
		return (-errno);
    if ((scol = (ushort *)calloc(wd, sizeof(ushort))) == NULL)
		return (-errno);

    norm = (float)(1. / (float)(size * size));
    wratio = (float)ws / (float)wd;
    hratio = (float)hs / (float)hd;

    for (i = 0; i < hd; i++)
		if ((srow[i] = (long)(hratio * i + 0.5)) > (hs - size))
			srow[i] = hs - size;

    for (j = 0; j < wd; j++)
		if ((scol[j] = (long)(wratio * j + 0.5)) > (ws - size))
			scol[j] = ws - size;

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
			rval = (long)((float)rval * norm);
			gval = (long)((float)gval * norm);
			bval = (long)((float)bval * norm);
			*(lined + j) = rval << 16 | gval << 8 | bval;
			if (xws_SchedReq > 0)
				thdCtlRelease();
		}
	}
    free(srow);
    free(scol);
    return (0);
}
