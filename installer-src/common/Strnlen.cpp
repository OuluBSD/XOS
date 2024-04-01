/************************************************/
/*           Run-time library for C68K          */
/* Copyright (c) 1985 by Saguaro Software, Ltd. */
/************************************************/

#include <STDIO.H>	/* Include C parameter definations */
#include <STDLIB.H>


int strnlen(
    const char *str,
	int         len)

{
	const char *pnt;

	pnt = str;
    while (--len >= 0 && *pnt++ != 0)
        ;
    return (pnt - str);
}
