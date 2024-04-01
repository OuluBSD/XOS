/************************************************/
/*           Run-time library for C68K          */
/* Copyright (c) 1985 by Saguaro Software, Ltd. */
/************************************************/

#include <STDIO.H>	/* Include C parameter definations */
#include <STDLIB.H>


char *strncpyn(
	char       *dst,
    const char *src,
	int         len)

{
	const char *pnt;
	char        chr;

	pnt = src;
    while (--len >= 0 && (chr = *pnt++) != 0)
		*dst++ = chr;
	*dst = 0;
    return ((char *)src);
}
