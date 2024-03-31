// This program automatically installs the console device.  Ths initial
//   version simply installs the standard VGA and keyboard drivers.  A
//   future version will probably support other console devices and will
//   also load the graphics extension driver that matchs the display.

// This program is only intented to be run at startup time.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <progarg.h>
#include <xoserr.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <xosstr.h>
#include <xossinfo.h>
#include "lkeload.h"

#define VERSION 1
#define EDITNO  0

extern char    lkename[];
extern LKEARGS lkeargs;

struct
{   LNGSTRCHAR result;
    uchar      end;
} lkechars =
{   {PAR_GET|REP_STR, 0, "RESULT", NULL, 1024, 1024}
};

long data[10];
char trmclass[] = "TRM:";
char trmname[] = "TRM0";
char kbdclass[] = "KBD:";

typedef struct
{   LNGSTRCHAR data;
    char       end;
} CHKCHARS;

CHKCHARS chkchars =
{   {PAR_GET|REP_DATAS, 0, "VALUE", (char *)data, sizeof(data), sizeof(data)}
};

struct 
{   BYTE1CHAR unit;
    TEXT4CHAR type;
    TEXT8CHAR adapter;
    BYTE1CHAR screen;
    char      end;
} vgachars =
{   {PAR_SET|REP_HEXV, 1, "UNIT"   , 0},
    {PAR_SET|REP_TEXT, 4, "TYPE"   , "VGAA"},
    {PAR_SET|REP_TEXT, 8, "ADAPTER", "COLOR"},
    {PAR_SET|REP_HEXV, 1, "SCREEN" , 1},
};

struct
{	BYTE1CHAR  unit;
	TEXT4CHAR  type;
	LNGSTRCHAR trm;
	char       end;
} kbcchars =
{	{PAR_SET|REP_HEXV, 1, "UNIT"   , 0},
	{PAR_SET|REP_TEXT, 4, "TYPE"   , "KBCA"},
	{PAR_SET|REP_STR , 0, "DISPLAY", trmname, 4, 4}
};

QAB vgaaddqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// func     - Function
    0,							// status   - Returned status
    0,							// error    - Error code
    0,							// amount   - Amount
    0,							// handle   - Device handle
    0,							// vector   - Vector for interrupt
    {0},
    CF_ADDUNIT,					// option   - Options or command
    0,							// count    - Count
    trmclass,					// buffer1  - Pointer to file spec
    (char *)&vgachars,			// buffer2  - Pointer to characteristics
    NULL						// parm     - Pointer to parameters
};

QAB kbcaddqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// func     - Function
    0,							// status   - Returned status
    0,							// error    - Error code
    0,							// amount   - Amount
    0,							// handle   - Device handle
    0,							// vector   - Vector for interrupt
    {0},
    CF_ADDUNIT,					// option   - Options or command
    0,							// count    - Count
    kbdclass,					// buffer1  - Pointer to file spec
    (char *)&kbcchars,			// buffer2  - Pointer to characteristics
    NULL						// parm     - Pointer to parameters
};

char  prgname[] = "CONINST";

long lkeload(int quiet, char *name, CHKCHARS *chars);


int main(void)
{
    int rtn;

    if (lkeload(0x01, "vgachk.lke", &chkchars) < 0)
		return (1);
    if (data[0] == 0)
    {
		fputs("? CONINST: No console adapter found\n", stdout);
		return (1);
    }
    if (lkeload(0, "conlnk.lke", NULL) < 0)
		return (1);
    if (lkeload(0, "kbdcls.lke", NULL) < 0)
		return (1);
    if (lkeload(0, "kbdconadev.lke", NULL) < 0)
		return (1);
    if (lkeload(0, "vgaadev.lke", NULL) < 0)
		return (1);
    if (data[0] == 0 && data[4] == 1)
		strcpy(vgachars.adapter.value, "MONO");
    if ((rtn = svcIoQueue(&vgaaddqab)) < 0 || (rtn = vgaaddqab.error) < 0)
		femsg2(prgname, "Error adding console display device", rtn, NULL);
	fputs("CONINST: Device unit TRM0: added\n", stdout);
    if ((rtn = svcIoQueue(&kbcaddqab)) < 0 || (rtn = kbcaddqab.error) < 0)
		femsg2(prgname, "Error adding console keyboard device", rtn, NULL);
	fputs("CONINST: Device unit KBD0: added\n", stdout);
    return (0);
}


//************************************************
// Function: lkeload - Load an LKE
// Returned: LKE return value (positive) if normal
//           or a negative error code if error
//************************************************

long lkeload(
    int       quiet,
    char     *name,
    CHKCHARS *chars)
{
	long rtn;

    if ((rtn = lkeloadf(quiet | 0x04, name, (LKECHAR *)chars)) < 0 &&
			rtn != ER_LKEAL)
		return (rtn);
	return (0);
}


//*************************************************
// Function: message - Display message for lkeloadf
// Returned: Nothing
//*************************************************

void  message(
    int level,
    char *text)
{
    level = level;

    fputs(text, stdout);
}
