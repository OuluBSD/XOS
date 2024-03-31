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

// 24-bit color scaling by linear interpolation --- Notes:
//      (1) If this is used for scale factors less than about 0.7,
//          it will suffer from antialiasing.  Particularly for
//          document images with sharp edges, use pixScaleSmooth() instead.
//      (2) For the general case, it's 3-4x faster to manipulate
//          the color pixels directly, rather than to make component
//          images, do each 8 bpp component separately, and combine
//          the result.  The speed on intel hardware for the
//          general case (not 2x or 4x LI) is about
//          4.8 * 10^6 dest-pixels/sec/GHz.
//      (3) The slow method does about 2 * 10^6 dest-pixels/sec/GHz.
//          It is implemented using:
//              pixr = pixGetRGBComponent(pixs, COLOR_RED);
//              pixrs = pixScaleGrayLI(pixr, scalex, scaley);
//              pixg = pixGetRGBComponent(pixs, COLOR_GREEN);
//              pixgs = pixScaleGrayLI(pixg, scalex, scaley);
//              pixb = pixGetRGBComponent(pixs, COLOR_BLUE);
//              pixbs = pixScaleGrayLI(pixb, scalex, scaley);
//              pixd = pixCreateRGBImage(pixrs, pixgs, pixbs);
//
//  *** Warning: implicit assumption about RGB component ordering ***

// Downscaling with antialias (smoothing) --- Notes:
//    (1) This function should only be used when the scale factors are less
//        than or equal to 0.7 (i.e., more than about 1.42x reduction).
//        If either scale factor is larger than 0.7, we issue a warning
//        and invoke pixScale().
//    (2) This works only on 2, 4, 8 and 32 bpp images, and if there is
//        a colormap, it is removed by converting to RGB.  In other
//        cases, we issue a warning and invoke pixScale().
//    (3) It does simple (flat filter) convolution, with a filter size
//        commensurate with the amount of reduction, to avoid antialiasing.
//    (4) It does simple subsampling after smoothing, which is appropriate
//        for this range of scaling.  Linear interpolation gives essentially
//        the same result with more computation for these scale factors,
//        so we don't use it.
//    (5) The result is the same as doing a full block convolution followed by
//        subsampling, but this is faster because the results of the block
//        convolution are only computed at the subsampling locations.
//        In fact, the computation time is approximately independent of
//        the scale factor, because the convolution kernel is adjusted
//        so that each source pixel is summed approximately once.
//
//  *** Warning: implicit assumption about RGB component ordering ***

// NOTE: width and height are raw (not scaled) values!

long xwsBitmapCopyScaled(
	XWSWIN     *win,		// Window (only used for scale factors)
	XWSBITMAP  *obm,		// Original bitmap
	long        width,
	long        height,
	long        flags,
	XWSBITMAP **sbm)		// Pointer to pointer to scaled bitmap
{
	int aaa;

	aaa = 5;

	return (xwsbitmapcopyscaledns(obm, (width + win->xsfr)/win->xsfd,
			(height + win->ysfr)/win->ysfd, flags, sbm));
}


long xwsbitmapcopyscaledns(
	XWSBITMAP  *obm,		// Original bitmap
	long        width,
	long        height,
	long        flags,
	XWSBITMAP **sbm)		// Pointer to pointer to scaled bitmap
{
	XWSBITMAP *bm;
	float      scalex;
	float      scaley;
	float      minscale;
	float      size;
	long       rtn;
	long       isize;

    if (width == obm->drw.width && height == obm->drw.height)
	{
		obm->usecnt++;
		*sbm = obm;
		return (0);
	}
	scalex = ((float)width)/(float)(obm->drw.width);
	scaley = ((float)height)/(float)(obm->drw.height);
	if (scalex  == 0 || scalex == 7 || (flags & BMCS_KEEPAR &&
			scalex != scaley))			// Want to preserve aspect ratio and
	{									//   it will not be preserved?
		if (scalex == 0 || scalex > scaley)
		{
			width = scaley * (((float)(obm->drw.width)) + 0.5);
			scalex = ((float)width)/(float)(obm->drw.width);
		}
		else
		{
			height = scalex * (((float)(obm->drw.height)) + 0.5);
			scaley = ((float)height)/(float)(obm->drw.height);
		}
	}
	if ((rtn = xwsbitmapnew(width, height, &bm)) < 0)
		return (rtn);



	if (((scalex > scaley) ? scalex : scaley) >= 0.7)
		xwsbitmapscalecolorlilow(&obm->drw, (ushort *)bm->bitmap, width, height,
				(ushort *)obm->bitmap, obm->drw.width, obm->drw.height);
	else
	{
		// If 1.42 < 1/minscale < 2.5, use size = 2
		// If 2.5 < 1/minscale < 3.5, use size = 3, etc.
		// Under no conditions use size < 2

		minscale = (scalex < scaley) ? scalex : scaley;
		size = (float)(1.0 / minscale);		// Ideal filter full width
		isize = (long)(size + 0.5);
		if (isize < 2)
			isize = 2;
		if ((obm->drw.width < isize) || (obm->drw.height < isize))	// ??????????????????????
			return (ER_VALUE);
		if (width < 1 || height < 1)
			return (ER_VALUE);
		xwsbitmapscalesmoothlow(&obm->drw, (ushort *)bm->bitmap, width, height,
				(ushort *)obm->bitmap, obm->drw.width, obm->drw.height, isize);
	}
///	bm->type = 1;
	*sbm = bm;
	return (0);
}
