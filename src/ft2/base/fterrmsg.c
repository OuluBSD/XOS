#include <stdio.h>
#include <stdlib.h>
#include <ft2build.h>

#undef __FTERRORS_H__

#define FT_NOERRORDEF_(e,v,s)

#define FT_ERRORDEF_(e,v,s){v, s},

///#define FT_ERROR_START_LIST {

///#define FT_ERROR_END_LIST {0, 0}};

static struct errtbl_
{	int   code;
	char *text;
} errtbl[] =
{
#include FT_ERROR_DEFINITIONS_H
	{0}
};


char *FT_ErrMsg(
	int code)

{
	struct errtbl_ *etp;
	static char text[32];

	etp = errtbl;
	do
	{
		if (code == etp->code)
			return (etp->text);
	} while ((++etp)->text != 0);
	sprintf(text, "Unknown error code %d", code);
	return (text);
}
