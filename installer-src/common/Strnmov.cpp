/************************************************/
/*           Run-time library for C68K          */
/* Copyright (c) 1985 by Saguaro Software, Ltd. */
/************************************************/

#include <stdio.h>	/* include c parameter definations */
#include <stdlib.h>
#include "xcstring.h"

/*
 * Function to copy second string into first string & point to end
 */

char *strnmov(
	char *a,
	const char *b,
	int   len)

{
    while (--len >= 0)
    {
	if ((*a = *b++) != '\0')	/* Copy 2nd string into 1st */
	    ++a;
	else
	    break;
    }
    return (a);
}
