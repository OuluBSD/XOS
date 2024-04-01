// ++++
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

#include "geckoh.h"


void rsplkelist(void)
{
	union
	{	uchar   *c;
		LKEITEM *i;
	}      pnt;
	uchar *rsptop;
	int    len;
	char   text[200];
	char   name[20];

	rsptop = rspbufr.c + rsplen;
	ensureleft();

	sprintf(text, "LKELIST: req=%d, top=%d\r\n", rspbufr.lkelist.number,
			rspbufr.lkelist.lketop);
	putstr(text);

	pnt.i = rspbufr.lkelist.data;
	while (pnt.c < rsptop)
	{
		if ((len = pnt.i->namelen) > 16)
			len = 16;
		strncpy(name, pnt.i->name, len);
		name[len] = 0;
		addsymtbl(name, pnt.i->codeaddr, pnt.i->dataaddr, pnt.i->number);
		lastlkenum = pnt.i->number;
		pnt.c += (offsetof(LKEITEM, name) + pnt.i->namelen);
	}
	if (showentry())
		begincmd();
}
