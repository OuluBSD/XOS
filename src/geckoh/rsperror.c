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

static char *nametbl[] =
{	"NOOP",				// 0x00
	"STATE",			// 0x01
	"LKELIST",			// 0x02
	"RDMEM",			// 0x03
	"WRTMEM",			// 0x04
	"RDREG",			// 0x05
	"WRTREG",			// 0x06
	"RDIO",				// 0x07
	"WRTIO",			// 0x08
	"BPCTRL",			// 0x09
	"BPLIST",			// 0x0A
	"WPCTRL",			// 0x0B
	"WPLIST",			// 0x0C
	"XCTRL"				// 0x0D
};


void rsperror(void)
{
	int   errf;
	char  text[100];

	if (rsplen < sizeof(ERRORRP))
	{
		ensureleft();
		putstr("\1? ERROR response is too short (%d)\2\r\n", rsplen);
		return;
	}
	errf = rspbufr.error.func & 0x1F;
	if (errf <= 0x0D)
		strcpy(text, nametbl[errf]);
	else
		sprintf(text, "#%d", errf);
	ensureleft();
	putstr("\1? ERROR response received for %srequest %s\r\n",
			(rspbufr.error.func & 0x20) ? "unsolicited " : "", text);
	if (rspbufr.error.code < 0)
	{
		svcSysErrMsg(rspbufr.error.code, 0x03, text);
		putstr("  %s\r\n", text);
	}
	if (rspbufr.error.detail != 0)
		putstr("  Detail: %08\r\n", rspbufr.error.detail);
	putstr("\2");
	begincmd();
}
