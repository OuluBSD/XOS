#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include "xcstring.h"
#include "xosstuff.h"
extern "C" {
#include "jpeglib.h"
#include "png.h"
}
#include "swcwindows.h"


static void dojpeg(struct jpeg_compress_struct *cinfo);


static void user_error_fn(png_structp png_ptr,
		png_const_charp error_msg);
static void user_warning_fn(png_structp png_ptr,
		png_const_charp warning_msg);
static void pngmemread(png_structp png_ptr, png_bytep data, png_size_t length);

swcImage::swcImage()

{
	type = 0;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = 0;
	bmih.biHeight = 0;
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
	data = NULL;
	usecnt = 1;
}


swcImage::swcImage(
	long width,
	long height,
	int  allocdata)

{
	type = 0;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = width;
	bmih.biHeight = height;
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
	data = (allocdata && width != 0 && height != 0) ?
			(new uchar[width * height * 4]) : NULL;
	usecnt = 1;
}


swcImage::swcImage(
	char *fspec)

{
	FILE *file;
	uchar header[12];

	type = SWC_IMAGE_NONE;
	data = NULL;
	usecnt = 1;
	if ((file = fopen(fspec, "rb")) == NULL)
		return;
	if (fread(header, 1, 12, file) != 12)
	{
		fclose(file);
		return;
	}
	fseek(file, 0, SEEK_SET);
	if (*(ushort *)header == 'MB')
		getbmpimage(NULL, -1, file);
	else if (((*(long *)header) & 0xFEFFFFFF) == 0xE0FFD8FF)
		getjpegimage(NULL, -1, file);
	else if (png_sig_cmp(header, 0, 8) == 0)
		getpngimage(NULL, -1, file);
	fclose(file);
}


swcImage::swcImage(
	uchar *bufr,
	int    size)

{
	type = SWC_IMAGE_NONE;
	usecnt = 1;
	data = NULL;
	if (*(ushort *)bufr == 'MB')
		getbmpimage(bufr, size, NULL);
	else if (*(long *)bufr == 0xE0FFD8FF && *(long *)(bufr + 6) == 'FIFJ')
		getjpegimage(bufr, size, NULL);
	else if (png_sig_cmp(bufr, 0, 8) == 0)
		getpngimage(bufr, size, NULL);
}


swcImage::~swcImage()

{
	if (data != NULL)
	{
		delete [] data;
		data = NULL;
	}
}


void swcImage::Use()

{
	usecnt++;
}


void swcImage::Release()

{
	if (--usecnt <= 0)
		delete this;
}


swcImage *swcImage::Copy()

{
	swcImage *rtnimg;

	rtnimg = new swcImage(bmih.biWidth, bmih.biHeight);
	memcpy(rtnimg->data, data, bmih.biWidth * bmih.biHeight * 4);
	rtnimg->type = type;
	return (rtnimg);
}


void swcImage::getbmpimage(
	uchar *bufr,
	int    size,
	FILE  *file)

{
#pragma pack(push, 1)
	struct
	{	BITMAPFILEHEADER bmfh;
		BITMAPINFOHEADER bmih;
	}     header;
#pragma pack(pop)

	if (size == -1)
	{
		if (fread(&header, 1, sizeof(header), file) < 0)
		{
			fclose(file);
			return;
		}
		if (header.bmih.biSize < sizeof(BITMAPINFOHEADER))
		{
			fclose(file);
			return;
		}
		memcpy(&bmih, &header.bmih, sizeof(BITMAPINFOHEADER));
		size = header.bmfh.bfSize - sizeof(BITMAPFILEHEADER);

		data = (uchar *)new char[size];

///		if ((data = (uchar *)getmemory(size)) == NULL)
///	    {
///			fclose(file);
///			return;
///	    }

		if (fread(data, 1, size, file) < 0)
		{
			fclose(file);
			return;
		}
		type = 1;
	}
	else
	{



	}
}


void swcImage::getjpegimage(
	uchar *bufr,
	int    size,
	FILE  *file)

{
	uchar  *dpnt;
	uchar **spnt;
    JSAMPARRAY scanlines;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	long   width;
	int    cnt;

	cinfo.err = jpeg_std_error(&jerr);	// Set up for jpeg decompression
	jpeg_create_decompress(&cinfo);

	if (size == -1)
		jpeg_stdio_src(&cinfo, file);
	else
		jpeg_mem_src(&cinfo, bufr, size);

	jpeg_read_header(&cinfo, TRUE);
	width = ((cinfo.image_width * 4) + 3) & 0xFFFFFFFC;
	data = dpnt = new uchar[width * cinfo.image_height];
	cinfo.out_color_space = JCS_RGB;
	scanlines = (JSAMPARRAY)new long[cinfo.image_height];
	spnt = (uchar **)scanlines + (cinfo.image_height - 1);
	cnt = cinfo.image_height;
	while (--cnt >= 0)
	{
		*spnt-- = (uchar *)dpnt;
		dpnt += width;
	}
	jpeg_start_decompress(&cinfo);		// Decompress the image
	while (cinfo.output_scanline < cinfo.output_height)
		jpeg_read_scanlines(&cinfo, scanlines + cinfo.output_scanline, 10);
	jpeg_finish_decompress(&cinfo);
	delete [] (long *)scanlines;
	bmih.biSize = sizeof(BITMAPINFOHEADER); // Fill in the bmp header
	bmih.biWidth = cinfo.image_width;		//    for windows
	bmih.biHeight = cinfo.image_height;
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0 ;
	bmih.biClrImportant = 0;
	jpeg_destroy_decompress(&cinfo);	// Give up the resources used for
	type = SWC_IMAGE_JPEG;
}


int swcImage::PutJPEG(
	char *filespec,
	int   quality)

{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE  *outfile;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	if ((outfile = fopen(filespec, "wb")) == NULL)
	{
		return (false);
	}
	jpeg_stdio_dest(&cinfo, outfile);
	dojpeg(&cinfo, quality);
	if (fclose(outfile) < 0)
	{
		return (false);
	}
	return (true);
}


uchar *swcImage::PutJPEG(
	int   quality,
	long *sizepntr)

{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	uchar *datapntr;
	long   size;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &datapntr, &size);
	dojpeg(&cinfo, quality);
	*sizepntr = size;
	return (datapntr);
}


long swcImage::PutJPEG(
	int    quality,
	uchar *bufr)

{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	long   size;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_bufr_dest(&cinfo, bufr, &size);
	dojpeg(&cinfo, quality);
	return (size);
}


void swcImage::dojpeg(
	struct jpeg_compress_struct *cinfo,
	int    quality)

{
	JSAMPROW rowpntr[1];				// Pointer to a single row
	int len;							// Physical row width in buffer

	cinfo->image_width = bmih.biWidth;	// Width of image, in pixels
	cinfo->image_height = bmih.biHeight; // Height of image, in pixels
	cinfo->input_components = 4;		// Number of color channels (samples per pixel)
	cinfo->in_color_space = JCS_RGB;	// Color space of source image

	jpeg_set_defaults(cinfo);
	jpeg_set_quality(cinfo, quality, true);
	jpeg_start_compress(cinfo, TRUE);
	len = bmih.biWidth * 4;				// JSAMPLEs per row in image_buffer
	while (cinfo->next_scanline < cinfo->image_height)
	{
	    rowpntr[0] = data + (bmih.biHeight - 1 - cinfo->next_scanline) * len;
	    jpeg_write_scanlines(cinfo, rowpntr, 1);
	}
	jpeg_finish_compress(cinfo);
	jpeg_destroy_compress(cinfo);
}




void swcImage::getpngimage(
	uchar *bufr,
	int    size,
	FILE  *file)

{
	uchar *rowpnt[1];
	ulong width;
	ulong height;
	ulong span;
	long  cnt;
	int  bitdepth;
	int  colortype;
	int interlacetype;
	int compressiontype;
	int filtermethod;
	long errdata;
	png_structp pngptr;
	png_infop   infoptr;

	if ((pngptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			(png_voidp)&errdata, user_error_fn, user_warning_fn)) == NULL)
		return;
	if ((infoptr = png_create_info_struct(pngptr)) == NULL)
	{
		png_destroy_read_struct(&pngptr, (png_infopp)NULL, (png_infopp)NULL);
		return;
	}
	if (size == -1)
		png_init_io(pngptr, file);
	else
		png_set_read_fn(pngptr, bufr, pngmemread);
	png_read_info(pngptr, infoptr);

	png_get_IHDR(pngptr, infoptr, &width, &height, &bitdepth, &colortype,
			&interlacetype, &compressiontype, &filtermethod);

	span = width * 4;
	data = new uchar[span * height];

	ulong *pnt;

	pnt = (ulong *)data;
	cnt = height * width;
	while (--cnt >= 0)
		*pnt++ = 0xC0C0C0;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = width;
	bmih.biHeight = height;
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0 ;
	bmih.biClrImportant = 0;

	if (bitdepth == 16)
		png_set_strip_16(pngptr);

	if (bitdepth < 8)
		png_set_packing(pngptr);

	if (colortype == PNG_COLOR_TYPE_RGB)
		png_set_filler(pngptr, 0, PNG_FILLER_AFTER);

	if (colortype == PNG_COLOR_TYPE_GRAY ||
			colortype == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(pngptr);

	if (colortype == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(pngptr);
		png_set_filler(pngptr, 0, PNG_FILLER_AFTER);
	}

	png_set_bgr(pngptr);

	rowpnt[0] = data + (span * (height - 1));
	cnt = height;
	while (--cnt >= 0)
	{
		png_read_rows(pngptr, rowpnt, NULL, 1);
		rowpnt[0] -= span;
	}
	png_destroy_read_struct(&pngptr, &infoptr, NULL);
	type = 1;
}


static void user_error_fn(png_structp png_ptr,
		png_const_charp error_msg)

{
	exit(1);
}


static void user_warning_fn(png_structp png_ptr,
		png_const_charp warning_msg)

{

}


static void pngmemread(
	png_structp png_ptr,
	png_bytep   data,
	png_size_t length)

{
	memcpy(data, png_ptr->io_ptr, length);
	png_ptr->io_ptr = (void *)(((char *)png_ptr->io_ptr) + length);
}
