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


void doresponse(void)
{
	char text[100];

	if (debugout)
	{
		sprintf(text, "Have response %02X, length = %d\r\n", rsptype, rsplen);
		putstr(text);
	}
	switch (rsptype & 0x1F)
	{
	 case DBREQ_NOOP:
		havenoop = TRUE;
		putstr("NoOp received\r\n");
		break;

	 case DBREQ_STATE:
		rspstate();
		break;

	 case DBREQ_LKELIST:
		rsplkelist();
		break;

	 case DBREQ_RDMEM:
		rsprdmem();
		break;

	 case DBREQ_WRTMEM:
		rspwrtmem();
		break;

	 case DBREQ_RDREG:
		goto unknwn;

	 case DBREQ_WRTREG:
		rspwrtreg();
		break;

	 case DBREQ_RDIO:
		rsprdio();
		break;

	 case DBREQ_WRTIO:
		begincmd();
		break;

	 case DBREQ_BPCTRL:
		rspbpctrl();
		break;

	 case DBREQ_BPLIST:
		rspbplist();
		break;

	 case DBREQ_WPCTRL:
		rspwpctrl();
		break;

	 case DBREQ_WPLIST:
		rspwplist();
		break;

	 case DBREQ_XCTRL:
		rspxctrl();
		break;

	 case DBREQ_GETVA:
		rspgetva();
		break;

	 case DBREQ_GETPA:
		rspgetpa();
		break;

	 case DBREQ_RDPCI:
		rsprdpci();
		break;

	 case DBREQ_WRTPCI:
		begincmd();
		break;

	 case DBREQ_ERROR:
		rsperror();
		break;

	 default:
	 unknwn:
		if (hvgdrsp)
		{
			ensureleft();
			sprintf(text, "\1? Unknown response (%02X) discarded\2\r\n",
					rsptype);
			putstr(text);
			begincmd();
		}
		return;
	}
	hvgdrsp = TRUE;
}



void unexpected(
	int type)
{
	char text[100];

	ensureleft();
	sprintf(text, "\1? Unexpected response (%02X) discarded\2\r\n", type);
	putstr(text);
	begincmd();
}
