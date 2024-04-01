#include "common.h"


void strtokinit(
	strtokdata &data,
	char       *bufr)

{
	data.pnt = bufr;
}


char *strtoknext(
	strtokdata &data,
	char       *tkns)

{
	char *pnt;
	char *bgn;
	char *tp;
	char  chr;
	char  tok;

	if ((pnt = data.pnt) == NULL)
		return (NULL);
	while (true)
	{
		chr = *pnt;
		tp = tkns;
		while ((tok = *tp++) != 0)
		{
			if (chr == tok)
			{
				*pnt = 0;
				data.pnt = pnt + 1;
				data.token = tok;
				return (pnt);
			}
		}
		if (!isspace(chr))
			break;
		pnt++;
	}
	bgn = pnt;

	while ((chr = *pnt) != 0 && chr != '\n')
	{
		if (chr == '"')
		{
			pnt++;
			while ((chr = *pnt) != 0 && chr != '\n' && chr != '"')
				pnt++;
			if (chr != '"')
				break;
			pnt++;
			continue;
		}
		tp = tkns;
		while ((tok = *tp++) != 0)
		{
			if (chr == tok)
			{
				*pnt = 0;
				data.pnt = pnt + 1;
				data.token = tok;
				return (bgn);
			}
		}
		pnt++;
	}
	*pnt = 0;
	data.pnt = NULL;
	return (bgn);
}
