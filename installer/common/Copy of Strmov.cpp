/************************************************/
/*           Run-time library for C68K          */
/* Copyright (c) 1985 by Saguaro Software, Ltd. */
/************************************************/

#include <STDIO.H>	/* Include C parameter definations */
#include <STDLIB.H>

/*
 * Function to copy second string into first string & point to end
 */

char *strmov(
    char *a,
    const char *b)

{
    while ((*a = *b++) != '\0')		/* Copy 2nd string into 1st */
        ++a;
    return (a);
}
