#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <xcstring.h>
#include <errno.h>
#include <xoserrmsg.h>
#include <xos.h>
#include <xossignal.h>
#include <xosfibers.h>
#include <xossvc.h>
#include <xossnd.h>
#include <xosswap.h>
#include <xossound.h>


#define THRD_BASE  0x40020000	// Base thread

#define VECT_SOUND   58		// Used by the sound functions
#define VECT_STOP    59
#define VECT_THRD    60		// First vector used by the thread routines


long busy;

long volume = 0x7FFF;
long freq = 440;
long time = 1000;
long force = 0;
long stphndl;

char  prgname[] = "WPLAY";
char  initing = TRUE;

void errorfunc(long code, char *msg);
void stopsig(void);

///void sounderror(long code, char *msg);

// NOTE: To get debug output, xossound must be compiled with DEBUG set
//       non-zero.

void main(
    int   argc,
    char *argv[])
{
	char *pnt;
	long  rtn;
	char  cmdline[128];
	char  name[150];
	char  chr;

	// Create a fibers environment

	fbrCtlInitialize(5, VECT_THRD, THRD_BASE + 0x20000 - sizeof(FBRDATA),
			0x18000, 0);

	// Allocate memory

	if ((rtn = svcMemChange((char *)0x40080000 - 0x2000, PG_READ|PG_WRITE,
			0x2000)) < 0)
		errorfunc(rtn, "Error allocating the sound thread stack");
	if ((rtn = svcMemChange((char *)0x400A0000, PG_READ|PG_WRITE, 0x4000)) < 0)
		errorfunc(rtn, "Error allocating the sound buffer");
	if ((rtn = svcMemChange((char *)0x40000000, PG_READ|PG_WRITE, 0x2000)) < 0)
		errorfunc(rtn, "Error allocating the input buffer");
	if ((rtn = setvector(VECT_STOP, 5, stopsig)) < 0)
		errorfunc(rtn, "Error setting stop vector");

	// Initialize the sound stuff

	sndInitalize(
			(long *)0x40080000,	// Top of the sound thread;s stack
			(char *)0x400A0000,	// Address of the sound buffer
			0x2000,				// Size of one buffer segment
			(char *)0x40000000,	// Address of the IO input buffer
			0x2000,				// Size of the IO input buffer
			0,					// Fiber to wake up for notifications
			VECT_SOUND,			// Vector for sound signal
			errorfunc);			// Error function
	initing = FALSE;

	while (TRUE)
	{
		fputs("snd> ", stdtrm);
		fgets(cmdline, sizeof(cmdline), stdin);
		if ((rtn = strlen(cmdline)) == 0)
			continue;
		if (cmdline[rtn - 1] == '\n')
			cmdline[rtn - 1] = 0;
		pnt = cmdline + 1;
		while ((chr = *pnt) != 0 && chr != '\n' && isspace(chr))
			pnt++;
		if (chr == '\n')
			*pnt = 0;
		switch(toupper(cmdline[0]))
		{
		 case 0:
			break;

		 case 'V':
			volume = atol(pnt);
			printf("Volume set to %d\n", volume);
			break;

		 case 'W':
			if (force > 0)
				stphndl = svcSchSetAlarm(VECT_STOP, 0, ((llong)force) * 10000);
			if (strchr(pnt, '.') == NULL)
			{
				strmov(strmov(name, pnt), ".wav");
				pnt = name;
			}
			sndPlay(pnt, volume, 0, 0);
			if (stphndl > 0)
			{
				svcSchClrAlarm(stphndl);
				stphndl = 0;
			}
			break;

		 case 'F':
			freq = atol(pnt);
			printf("Frequency set to %d\n", freq);
			break;

		 case 'D':
			time = atol(pnt);
			printf("Duration set to %d\n", time);
			break;

		 case 'T':
			sndTone(freq, time, volume, 0, 0);
			break;

		 case 'S':
			force = atol(pnt);
			break;

		 case 'X':
		 case 'Q':
			exit(0);

		 default:
			fputs("? Invalid command\n", stderr);
			break;
		}
	}
}


void stopsig(void)
{
	sndStop();
}


void errorfunc(
	long  code,
	char *msg)
{
	errormsg(code, msg);
	if (initing)
		exit(1);

}
