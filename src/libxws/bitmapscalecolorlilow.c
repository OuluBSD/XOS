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

#include <xosxxws.h>

/*------------------------------------------------------------------*
 *            General linear interpolated color scaling             *
 *------------------------------------------------------------------*/
/*!
 *  scaleColorLILow()
 *
 *  We choose to divide each pixel into 16 x 16 sub-pixels.
 *  Linear interpolation is equivalent to finding the 
 *  fractional area (i.e., number of sub-pixels divided
 *  by 256) associated with each of the four nearest src pixels,
 *  and weighting each pixel value by this fractional area.
 *
 *  P3 speed is about 7 x 10^6 dst pixels/sec/GHz
 *
 *  *** Warning: explicit assumption about RGB component ordering ***
 */

	XWSXCOL      pixels1;
	XWSXCOL      pixels2;
	XWSXCOL      pixels3;
	XWSXCOL      pixels4;
	XWSXCOL      pixel;

long XOSFNC xwsbitmapscalecolorlilow(
	XWSDRW *drw,
	ushort *datad,
	long    wd,
	long    hd,
	ushort *datas,
	long    ws,
	long    hs)
{
	XWSCOMCOLOR *comcolor;
	XWSEXPCOLOR *expcolor;
	long         i, j, wm2, hm2;
	long         xpm;		// Location in src image, to 1/16 of a pixel
	long         ypm;
	long         xp;		// Src pixel and pixel fraction coordinates
	long         yp;
	long         xf;
	long         yf;
	long         area00, area01, area10, area11;

	ushort *lines;
	ushort *lined;
	float   scx;
	float   scy;

	comcolor = drw->funcdsp->comcolor;
	expcolor = drw->funcdsp->expcolor;

	// (scx, scy) are scaling factors that are applied to the
	//   dest coords to get the corresponding src coords.
	//   We need them because we iterate over dest pixels
	//   and must find the corresponding set of src pixels.

    scx = (float)(16. * (float)ws / (float)wd);
    scy = (float)(16. * (float)hs / (float)hd);

    wm2 = ws - 2;
    hm2 = hs - 2;

	// Iterate over the destination pixels

    for (i = 0; i < hd; i++)
	{
		ypm = (long)(scy * (float)i + 0.5);
		yp = ypm >> 4;
		yf = ypm & 0x0f;
		lined = datad + i * wd;
		lines = datas + yp * ws;
		for (j = 0; j < wd; j++)
		{
			xpm = (long)(scx * (float)j + 0.5);
			xp = xpm >> 4;
			xf = xpm & 0x0F;

			// If near the edge, just use the src pixel value

			if (xp > wm2 || yp > hm2)
			{
				*(lined + j) = *(lines + xp);
				continue;
			}

			// Do bilinear interpolation.  This is a simple generalization
			//   of the calculation in scaleGrayLILow().

            expcolor(&pixels1, (uchar *)(lines + xp)); 
            expcolor(&pixels2, (uchar *)(lines + xp + 1));
            expcolor(&pixels3, (uchar *)(lines + ws + xp));
            expcolor(&pixels4, (uchar *)(lines + ws + xp + 1));
			area00 = (16 - xf) * (16 - yf);
			area10 = xf * (16 - yf);
			area01 = (16 - xf) * yf;
			area11 = xf * yf;
			pixel.red = (area00 * pixels1.red + area10 * pixels2.red +
					area01 * pixels3.red + area11 * pixels4.red + 128) >> 8;
			pixel.green = (area00 * pixels1.green +  area10 * pixels2.green +
					area01 * pixels3.green + area11 * pixels4.green + 128) >> 8;
			pixel.blue = (area00 * pixels1.blue + area10 * pixels2.blue +
					area01 * pixels3.blue + area11 * pixels4.blue + 128) >> 8;
			comcolor(&pixel, (uchar *)(lined + j));
		}
		if (xws_SchedReq > 0)
			fbrCtlRelease();
    }
    return (0);
}
