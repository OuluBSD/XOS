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


#ifndef ALLHEADERS_H_INCLUDED
#define ALLHEADERS_H_INCLUDED


#define _LITTLE_ENDIAN_

typedef long l_intptr_t;
typedef unsigned long l_uintptr_t;

/*--------------------------------------------------------------------*
 *                          Built-in types                            *
 *--------------------------------------------------------------------*/
typedef signed char		l_int8;
typedef unsigned char      	l_uint8;
typedef short                   l_int16;
typedef unsigned short          l_uint16;
typedef int                     l_int32;
typedef unsigned int            l_uint32;
typedef float			l_float32;
typedef double			l_float64;



/*------------------------------------------------------------------------*
 *                            Standard macros                             *
 *------------------------------------------------------------------------*/
#ifndef MIN
#define MIN(x,y)      (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y)      (((x) > (y)) ? (x) : (y))
#endif

#ifndef ABS
#define ABS(x)        (((x) < 0) ? (-1 * (x)) : (x))
#endif

#ifndef SIGN
#define SIGN(x)       (((x) < 0) ? -1 : 1)
#endif




    /* imaging */

#define  GET_DATA_BIT(pdata, n) \
    ((*((pdata) + ((n) >> 5)) >> (31 - ((n) & 31))) & 1)

#define  SET_DATA_BIT(pdata, n) \
    (*((pdata) + ((n) >> 5)) |= (0x80000000 >> ((n) & 31)))

#define  CLEAR_DATA_BIT(pdata, n) \
    (*((pdata) + ((n) >> 5)) &= ~(0x80000000 >> ((n) & 31)))


    /*---------  2 bit access ---------*/
#define  GET_DATA_DIBIT(pdata, n)             getDataDibit(pdata, n)
#define  SET_DATA_DIBIT(pdata, n, val)        setDataDibit(pdata, n, val)
#define  CLEAR_DATA_DIBIT(pdata, n)           clearDataDibit(pdata, n)


    /*---------  4 bit access ---------*/
#define  GET_DATA_QBIT(pdata, n)              getDataQbit(pdata, n)
#define  SET_DATA_QBIT(pdata, n, val)         setDataQbit(pdata, n, val)
#define  CLEAR_DATA_QBIT(pdata, n)            clearDataQbit(pdata, n)


    /*---------  8 bit access ---------*/
#ifdef  _BIG_ENDIAN_
#define  GET_DATA_BYTE(pdata, n) \
             (*((l_uint8 *)(pdata) + (n)))
#else  /* _LITTLE_ENDIAN_ */
#define  GET_DATA_BYTE(pdata, n) \
             (*(l_uint8 *)((l_uintptr_t)((l_uint8 *)(pdata) + (n)) ^ 3))
#endif  /* _BIG_ENDIAN_ */

#ifdef  _BIG_ENDIAN_
#define  SET_DATA_BYTE(pdata, n, val) \
             (*((l_uint8 *)(pdata) + (n)) = (val))
#else  /* _LITTLE_ENDIAN_ */
#define  SET_DATA_BYTE(pdata, n, val) \
             (*(l_uint8 *)((l_uintptr_t)((l_uint8 *)(pdata) + (n)) ^ 3) = (val))
#endif  /* _BIG_ENDIAN_ */


    /*---------  16 bit access ---------*/
#ifdef  _BIG_ENDIAN_
#define  GET_DATA_TWO_BYTES(pdata, n) \
             (*((l_uint16 *)(pdata) + (n)))
#else  /* _LITTLE_ENDIAN_ */
#define  GET_DATA_TWO_BYTES(pdata, n) \
             (*(l_uint16 *)((l_uintptr_t)((l_uint16 *)(pdata) + (n)) ^ 2))
#endif  /* _BIG_ENDIAN_ */

#ifdef  _BIG_ENDIAN_
#define  SET_DATA_TWO_BYTES(pdata, n, val) \
             (*((l_uint16 *)(pdata) + (n)) = (val))
#else  /* _LITTLE_ENDIAN_ */
#define  SET_DATA_TWO_BYTES(pdata, n, val) \
             (*(l_uint16 *)((l_uintptr_t)((l_uint16 *)(pdata) + (n)) ^ 2) = (val))
#endif  /* _BIG_ENDIAN_ */




/*------------------------------------------------------------------------*
 *                           Basic Pix                                    *
 *------------------------------------------------------------------------*/
struct Pix
{
	long  type;
	BITMAPINFOHEADER bmih;

///    l_uint32             w;           /* width in pixels                  */
///    l_uint32             h;           /* height in pixels                 */
///    l_uint32             d;           /* depth in bits                    */

///    l_uint32             wpl;         /* 32-bit words/line                */
///    l_uint32             xres;        /* image res (ppi) in x direction   */
				      /* (use 0 if unknown)               */
///    l_uint32             yres;        /* image res (ppi) in y direction   */
				      /* (use 0 if unknown)               */
///    char                *text;        /* text string associated with pix  */
///    struct PixColormap  *colormap;    /* colormap (may be null)           */
    l_uint32            *data;        /* the image data                   */
    l_uint32             usecnt;    /* reference count (1 if no clones) */
};
typedef struct Pix PIX;


/*-----------------------------------------------------------------------*
 *                            Colors for 32 bpp                          *
 *-----------------------------------------------------------------------*/
/* Note: colors are used in 32 bpp images.  The 4th byte can be
 * used for blending, but is not used here.  */
///enum {
///    COLOR_RED = 0,
///    COLOR_GREEN = 1,
///    COLOR_BLUE = 2
///};


///#ifdef __cplusplus
///extern "C" {
///#endif /* __cplusplus */


///extern l_int32 scaleBySamplingLow ( l_uint32 *datad, l_int32 wd, l_int32 hd,
///		l_uint32 *datas, l_int32 ws, l_int32 hs, l_int32 d);

extern l_int32 scaleSmoothLow ( l_uint32 *datad, l_int32 wd, l_int32 hd,
		l_uint32 *datas, l_int32 ws, l_int32 hs, l_int32 size);

extern void scaleColor2xLILineLow (l_uint32 *lined, l_int32 wd,
		l_uint32 *lines, l_int32 ws, l_int32 lastlineflag);


extern PIX * pixCopy ( PIX *pixd, PIX *pixs );
extern PIX * pixClone ( PIX *pixs );
extern void pixDestroy ( PIX **ppix );

extern l_int32 pixGetWidth ( PIX *pix );
extern l_int32 pixSetWidth ( PIX *pix, l_int32 width );
extern l_int32 pixGetHeight ( PIX *pix );
extern l_int32 pixSetHeight ( PIX *pix, l_int32 height );
extern l_int32 pixGetDepth ( PIX *pix );
extern l_int32 pixSetDepth ( PIX *pix, l_int32 depth );
extern l_int32 pixGetWpl ( PIX *pix );
extern l_int32 pixSetWpl ( PIX *pix, l_int32 wpl );
extern l_int32 pixGetRefcount ( PIX *pix );
extern l_int32 pixChangeRefcount ( PIX *pix, l_int32 delta );

extern PIX * pixScale ( PIX *pixs, l_float32 scalex, l_float32 scaley );
extern PIX * pixScaleColorLI ( PIX *pixs, l_float32 scalex, l_float32 scaley );
extern PIX * pixScaleColor2xLI ( PIX *pixs );

extern PIX * pixScaleSmooth ( PIX *pix, l_float32 scalex, l_float32 scaley );

extern l_uint32 * pixGetData ( PIX *pix );
extern l_int32 pixSetData ( PIX *pix, l_uint32 *data );

extern PIX * pixCreate(l_int32 width, l_int32 height, l_int32 depth );
extern l_int32 pixCopyResolution(PIX *pixd, PIX *pixs );
extern l_int32 pixScaleResolution(PIX *pix, l_float32 xscale, l_float32 yscale);

extern l_uint32 pixGetXRes(PIX *pix);
extern l_uint32 pixGetYRes(PIX *pix);
extern l_int32 pixSetXRes(PIX *pix, l_uint32 res);
extern l_int32 pixSetYRes(PIX *pix, l_uint32 res);

extern l_int32 getDataDibit ( l_uint32 *line, l_int32 n );
extern void setDataDibit ( l_uint32 *line, l_int32 n, l_int32 val );

extern l_int32 pixSizesEqual ( PIX *pix1, PIX *pix2 );

extern l_int32 getDataQbit ( l_uint32 *line, l_int32 n );
extern void setDataQbit ( l_uint32 *line, l_int32 n, l_int32 val );


///#ifdef __cplusplus
///}
///#endif /* __cplusplus */


#endif /* ALLHEADERS_H_INCLUDED */


