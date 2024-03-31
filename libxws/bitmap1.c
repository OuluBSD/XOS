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

// These two functions display an image in a window. All of the details of
//   loading and drawing the image are handled. If scaling is not specified
//   and the image is smaller than the specified area any area outside the
//   image is transparent. If the image is larger it is clipped. When scaling
//   is specified, the aspect ratio is always preserved. The image is scaled
//   (up or down) to the largest size that will completely fit in the given
//   area.


long xwsWinBitmapFromFile(
	XWSWIN *win,
	long    index,
	long    xpos,
	long    ypos,
	long    width,
	long    height,
	long    attr,
	char   *filename)

{
	XWSBITMAP *bm;
	long       rtn;

	if ((rtn = xwsBitmapLoad(filename, win, 0, &bm)) < 0)
		return (rtn);
	return (xwsWinBitmapFromBM(win, index, xpos, ypos, width, height,
			attr, bm));
}


long xwsWinBitmapRemove(
	XWSWIN *win,
	long    index)

{
	XWSIB **ppnt;
	XWSIB  *ipnt;

	if (index == -1)
	{
		while (win->ibhead != NULL)
			xwsbitmapgiveib(win, &win->ibhead);
		return (0);
	}
	ppnt = &win->ibhead;
	while ((ipnt = *ppnt) != NULL && ipnt->inx < index)
		ppnt = &ipnt->next;
	if (ipnt != NULL && ipnt->inx == index)
	{
		xwsbitmapgiveib(win, ppnt);
		return (0);
	}
	return (ER_NTDEF);
}


long xwsWinBitmapFromBM(
	XWSWIN    *win,
	long       index,
	long       xpos,
	long       ypos,
	long       width,
	long       height,
	long       flags,
	XWSBITMAP *bm)
{
	XWSIB    **ppnt;
	XWSIB     *ib;
	XWSIB     *ipnt;
	XWSRGN    *newrgn;
	XWSBITMAP *sbm;
	long       rtn;

	if ((ib = (XWSIB *)malloc(sizeof(XWSIB))) == NULL)
		return (-errno);
	ib->label = 'IB##';
	ib->inx = index;
	ib->xpos = (xpos + win->xsfr)/win->xsfd;
	ib->ypos = (ypos + win->ysfr)/win->ysfd;
	width = (width != 0) ? ((width + win->xsfr)/win->xsfd) : bm->drw.width;
	height = (height != 0) ? ((height + win->ysfr)/win->ysfd) : bm->drw.height;
	if (width != bm->drw.width || height != bm->drw.height)
	{
		if ((rtn = xwsbitmapcopyscaledns(bm, width, height, flags, &sbm)) < 0)
			return (rtn);

		xwsBitmapDestroy(bm);
		bm = sbm;
	}
	ib->bm = bm;
	ppnt = &win->ibhead;
	while ((ipnt = *ppnt) != NULL && ipnt->inx < index)
		ppnt = &ipnt->next;

	newrgn = xwsrgncreaterectns(ib->xpos, ib->ypos, ib->bm->drw.width,
			ib->bm->drw.height);

	/// NEED TO INVALIDATE WITHOUT ERASING!!!!

	if (ipnt != NULL && ipnt->inx == index)
	{
		ib->next = ipnt->next;
		xwsbitmapgiveib(win, ppnt);
	}
	else
		ib->next = ipnt;
	*ppnt = ib;
	xwsWinInvalidateRgn(win, newrgn);
	return (0);
}


void xwsbitmapgiveib(
	XWSWIN *win,
	XWSIB **ppnt)

{
	XWSIB  *ipnt;

	ipnt = *ppnt;
	*ppnt = ipnt->next;
	xwsWinInvalidateRgn(win, xwsrgncreaterectns(ipnt->xpos, ipnt->ypos,
			ipnt->bm->drw.width, ipnt->bm->drw.height));
	xwsBitmapDestroy(ipnt->bm);
	free(ipnt);
}


void xwsWinDispBitmap(
	XWSWIN *win)

{
	XWSIB *ib;

	ib = win->ibhead;
	while (ib != NULL)
	{
		xwsDrawBitmap(win, ib->bm, ib->xpos, ib->ypos, 1, 1, 0);
		ib = ib->next;
	}
}



long xwsbitmapnew(
	long        width,
	long        height,
	XWSBITMAP **pbm)
{
	XWSBITMAP *bm;
	long       size;

	size = width * height * 2;
	if ((bm = (XWSBITMAP *)xwsMalloc(size + offsetof(XWSBITMAP,
			bitmap))) == NULL)
		return (-errno);
	bm->label = 'BM##';
	bm->status = 0;
	bm->usecnt = 0;
	bm->drw.label = 'DRW#';
	bm->drw.buffer = bm->bitmap;
	bm->drw.width = width;
	bm->drw.pixelspan = width;
	bm->drw.bytespan = width * 2;
	bm->drw.height = height;
	bm->drw.bufsize = size;
	bm->drw.funcdsp = &xwsmemdsp;
	*pbm = bm;
	return (0);
}
