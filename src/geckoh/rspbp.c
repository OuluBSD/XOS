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


void rspbpctrl(void)
{
	ensureleft();
	if (rsplen < 1)
		putstr("\1? Response is too short\2");
	else if (rspbufr.bpctrl.func & 0x80)
	{
		switch (rspbufr.bpctrl.func & 0x7F)
		{
		 case 1:
			if (rsplen < 5 || rspbufr.bpctrl.addr == 0 ||
					rspbufr.bpctrl.addr == 0xFFFFFFFF)
				putstr("\1? Invalid address\2");
			else
				putstr("\1? Too many breakpoints set\2");
			break;

		 case 2:
			putstr("\1? Breakpoint is not set\2");
			break;

		 case 3:
			putstr("\1? No breakpoints are set\2");
			break;
		}
	}
	begincmd();
}



void rspbplist(void)
{
	char   *tpnt;
	BPITEM *ipnt;
	int     cnt;
	char    text[400];

	ensureleft();
	cnt = rsplen / sizeof(BPITEM);
	if (cnt == 0)
		putstr("No breakpoints are set");
	else
	{
		ipnt = rspbufr.bplist.item;
		do
		{
			tpnt = fmtvalue(text, (VAL *)&ipnt->addr);
			if (ipnt->count != 0)
				tpnt += sprintf(tpnt, "  (count = %d)", ipnt->count);
			strmov(tpnt, "\r\n");
			putstr(text);
			ipnt++;
		} while (--cnt > 0);

	}
	begincmd();
}
