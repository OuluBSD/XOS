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


static char *fmtval(char *str, long value);
static char *symmov(char *str, SYM *sym, SYM *mod);


char *fmtnumeric(
	char *pnt,
	long  value)
{
	VAL val;

	val.val = value;
	val.seg = 0;
	return (fmtvalue(pnt, &val));
}


char *fmtvalue(
	char *pnt,
	VAL  *value)
{
	SYM  *sym;
	SYM  *mod;
	VAL   rmv;
	ulong offset;
	int   sel;
	char  chr;

	if (value->seg == 0xFFFF)
	{
		rmaddr = TRUE;
		pnt += sprintf(pnt, "0x%X;", ((ulong)value->val) >> 16);
		rmv.val = (ushort)value->val;
		rmv.seg = 0;
		value = &rmv;
	}
	rmaddr = FALSE;
	sel = value->seg & 0xFFFC;
	if (sel != 0 && sel != GS_UDATA && sel != GS_UCODE && sel != GS_XDATA &&
			sel != GS_XCODE)
		pnt += sprintf(pnt, "0x%X:", value->seg);
	switch (curmode.omode)
	{
	 case OM_SYMBOLIC:
		if ((sym = findbestsym(value->val, &mod)) == NULL ||
				(offset = value->val - sym->value) >= 0x1000)
			goto numeric;
		pnt = symmov(pnt, sym, mod);
		if (offset != 0)
		{
			*pnt++ = '+';
			pnt = fmtval(pnt, offset);
		}
		break;
	 case OM_NUMERIC:
	 numeric:
		pnt = fmtval(pnt, value->val);
		break;
	 case OM_CHAR:
		do
		{
			if ((chr = value->val & 0x7F) >= ' ')
				*pnt++ = chr;
			else
				pnt += sprintf(pnt, "<%02X>", (uchar)value->val);
			value->val >>= 8;
		} while (value->val != 0);
		*pnt = 0;
		break;
	}
	return (pnt);
}


static char *symmov(
	char *pnt,
	SYM  *sym,
	SYM  *module)
{
	pnt = strnmov(pnt, sym->name, sym->length);
	while (TRUE)
	{
		if (sym->flags & SF_GBL)
		{
			*pnt++ = '#';
			if (!showgmn || module == curmodule)
				break;
		}
		else
		{
			if (module == curmodule)
				break;
			*pnt++ = '@';
		}
		pnt = strnmov(pnt, module->name, module->length);
		break;
	}
	*pnt = 0;
	return (pnt);
}


static char *fmtval(
	char *pnt,
	long  value)
{
	switch (curmode.oradix)
	{
	 case 16:
		if (((ulong)value) > 9)
		{
			*pnt++ = '0';
			*pnt++ = 'x';
		}
		return (pnt + sprintf(pnt, "%X", value));

	 case 10:
		return (pnt + sprintf(pnt, "%d", value));

	 case 8:
		if (((ulong)value) > 7)
			*pnt++ = '0';
		return (pnt + sprintf(pnt, "%O", value));

	 default:
		if (((ulong)value) > 1)
		{
			*pnt++ = '0';
			*pnt++ = 'b';
		}
		return (pnt + sprintf(pnt, "%B", value));
	}
}
