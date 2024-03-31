#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <errmsg.h>
#include <xos.h>
#include <xossignal.h>
#include <xossvc.h>
#include <xossnd.h>
#include <xosswap.h>
#include "soundplay.h"

long busy;

char  prgname[] = "WPLAY";

///void sounderror(long code, char *msg);

void main(
    int   argc,
    char *argv[])
{
	if (argc != 3)
	{
		fputs("? SPLAY: Command error, usage is:\n"
				"             SPLAY filespec volume\n", stderr);
		exit(1);
	}
	busy = 1;
	if (soundplay(argv[1], atol(argv[2])) < 0)
		exit (1);
	while (busy)
		svcSchSuspend(&busy, -1);
	printf("All done\n");
	exit (0);
}
