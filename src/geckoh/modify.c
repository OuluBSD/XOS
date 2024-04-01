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


int modifyreg(void)
{
	RINFO *info;
	VAL    regval;
	char   text[32];

	if ((hldchar = nxtnbc()) == 0 || !lastexpression(&regval))
	{
		cmdpnt = cmdline;
		return (CMD_CMD);
	}
	cmdpnt = cmdline;
	if (curreg->size == 4)
		*(long *) (((uchar *)&regtbl) + curreg->offset) = regval.val;
	else if (curreg->size == 2)
		*(ushort *)(((uchar *)&regtbl) + curreg->offset) = (ushort)regval.val;
	else
		*(((uchar *)&regtbl) + curreg->offset) = (uchar)regval.val;

	info = (RINFO *)(((uchar *)&regtbl) + (curreg->offset & 0xFFFFFFFC));
	if (info->name[2])
		sprintf(text, "%08X", info->value);
	else
		sprintf(text, "%04X", (ushort)(info->value));
	putstrns(info->spos, text);
	reqbufr.wrtreg.reg = curreg->inx;
	reqbufr.wrtreg.value = regval.val;
	if (sendrequest(DBREQ_WRTREG, sizeof(WRTREGRQ)) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}


void rspwrtreg(void)
{
	if (cmdstate != CMD_WWTO || curreg == NULL ||
			curreg->inx != rspbufr.wrtreg.reg)
		unexpected(DBREQ_WRTREG);
	else
	{
		curreg = NULL;
		begincmd();
	}
}



int modifymem(void)
{
	VAL memval;

	if ((hldchar = nxtnbc()) == 0 || !lastexpression(&memval))
	{
		cmdpnt = cmdline;
		return (CMD_CMD);
	}
	cmdpnt = cmdline;
	reqbufr.wrtmem.size = curszx;
	reqbufr.wrtmem.addr = curaddr.val;
	*(VAL *)&reqbufr.wrtmem.data = memval;
	if (sendrequest(DBREQ_WRTMEM, offsetof(WRTMEMRQ, data) + cursize) < 0)
		return (CMD_CMD);
	return (CMD_WWTO);
}
