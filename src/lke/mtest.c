#include "stdio.h"
#include "xos.h"
#include "xossvc.h"
#include "xossignal.h"
#include "xoserrmsg.h"


typedef struct
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intEIP;
    long  intCS;
    long  intEFR;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    short intcnt;
    short intnum;
	long  userdata;
	long  buttons;
	long  xval;
	long  yval;
	long  wval;
	long  timelow;
	long  timehigh;
} SIGDATA;

long xval;
long yval;
long wval;
char mname[20];
char prgname[] = "MTEST";

struct
{	BYTE4PARM vect;
	uchar     end;
} mouseparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_SIGVECT1, 50}
};


void mousesignal(SIGDATA sigdata);


void main()


{
	long hndl;
	long rtn;

	// Open the console mouse device

	if ((hndl = svcIoOpen(XO_IN, "MOUSE:", NULL)) < 0)
		femsg2(prgname, "Error opening MOUSE:", hndl, NULL);

	// Set the mouse signal vector

	if ((rtn = setvector(50, 0x04, mousesignal)) < 0)
		femsg2(prgname, "Error setting mouse signal vector", rtn, NULL);

	// Set the mouse vector number

	if ((rtn = svcIoInBlockP(hndl, NULL, 0, &mouseparms)) < 0)
		femsg2(prgname, "Error setting mouse signal number", rtn, NULL);

	fputs("\x1B[2J", stdtrm);

	svcSchSetLevel(0);
	while (TRUE)
		svcSchSuspend(NULL, -1);
}



void mousesignal(
	SIGDATA sigdata)

{
	if ((xval += sigdata.xval) < 0)
		xval = 0;
	else if (xval > 799)
		xval = 799;
	if ((yval += sigdata.yval) < 0)
		yval = 0;
	else if (yval > 599)
		yval = 599;
	wval += sigdata.wval;

	svcTrmCurPos(DH_STDTRM, -1, 0, 0);

	printf("  %s%s%s%s%s%s%s%s %6d %6d %6d %6d %6d %6d   %08X %08X\n",

			(sigdata.buttons & 0x40) ? "8" : " ",
			(sigdata.buttons & 0x40) ? "7" : " ",
			(sigdata.buttons & 0x20) ? "6" : " ",
			(sigdata.buttons & 0x10) ? "5" : " ",
			(sigdata.buttons & 0x08) ? "4" : " ",
			(sigdata.buttons & 0x04) ? "M" : " ",
			(sigdata.buttons & 0x02) ? "R" : " ",
			(sigdata.buttons & 0x01) ? "L" : " ", xval, yval, wval,
			sigdata.xval, sigdata.yval, sigdata.wval, sigdata.timehigh,
			sigdata.timelow);

	svcTrmCurPos(DH_STDTRM, -1, xval/10, yval/10);
}
