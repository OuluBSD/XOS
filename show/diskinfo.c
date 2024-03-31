//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <xos.h>
#include <xossinfo.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <xcmalloc.h>
#include <xosstr.h>
#include "show.h"

extern char prgname[];
char  *unitnames;

struct dichar
{   TEXT16CHAR dosname;
    TEXT16CHAR volname;
    TEXT4CHAR  unittype;
    TEXT4CHAR  remove;
    TEXT4CHAR  msensor;
    TEXT8CHAR  fstype;
    BYTE4CHAR  cblksz;
    BYTE4CHAR  clssz;
    BYTE4CHAR  clusters;
    BYTE4CHAR  avail;
    BYTE4CHAR  cheads;
    BYTE4CHAR  csects;
    BYTE4CHAR  ccylns;
    BYTE1CHAR  partn;
    char       end;
} dichar =
{   {PAR_GET|REP_TEXT, 16, "DOSNAME"},
    {PAR_GET|REP_TEXT, 16, "VOLNAME"},
    {PAR_GET|REP_TEXT,  4, "UNITTYPE"},
    {PAR_GET|REP_TEXT,  4, "REMOVE"},
    {PAR_GET|REP_TEXT,  4, "MSENSOR"},
    {PAR_GET|REP_TEXT,  8, "FSTYPE"},
    {PAR_GET|REP_DECV,  4, "CBLKSZ"},
    {PAR_GET|REP_DECV,  4, "CLSSZ"},
    {PAR_GET|REP_DECV,  4, "CLUSTERS"},
    {PAR_GET|REP_DECV,  4, "AVAIL"},
    {PAR_GET|REP_DECV,  4, "CHEADS"},
    {PAR_GET|REP_DECV,  4, "CSECTS"},
    {PAR_GET|REP_DECV,  4, "CCYLNS"},
    {PAR_GET|REP_HEXV,  1, "PARTN"}
};

char diskcls[] = "DISK:";

QAB diqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// func
    0,							// status
    0,							// error
    0,							// amount
    -1,							// handle
    0,							// vector
    {0},
    CF_PUNITS,					// option
    0,							// count
    &diskcls,					// buffer1
    NULL,						// buffer2
    NULL						// parm
};


static int getval(int chr, char **pnt);
static void showerror(long code);
static int strxcmp(char *s1, char *s2, int len);


int diskinfo(void)
{
    long     rtn;
    long     avail;
    long     total;
    long     cnt;
    int      numunits;
    char    *ptr;
    char     temp[12];
    char     partnc;

    ++validcount;
    if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.error) < 0)
        femsg(prgname, rtn, NULL);

    numunits = diqab.count = diqab.amount;
    diqab.buffer2 = unitnames = getspace(diqab.count * 8);
    if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.error) < 0)
        femsg(prgname, rtn, NULL);

    // Sort the list of disk names

    partnc = FALSE;
    do
    {
        partnc = FALSE;
        cnt = numunits;
        ptr = unitnames;
        while (-- cnt > 0)
        {
            if ((ptr[0] != ptr[8]) ? ((ptr[0] != 'F' && ptr[8] == 'F') ? TRUE :
					(ptr[0] != 'F' && ptr[0] > ptr[8])) :
					(strxcmp(ptr+1, ptr+9, 7) > 0))
			{
				strnmov(temp, ptr, 8);
                strnmov(ptr, ptr+8, 8);
                strnmov(ptr+8, temp, 8);
                partnc = TRUE;
            }
            ptr += 8;
        }
    } while (partnc);

    fputs("Disk DOS Unit F M File    Ptn  Blk Clst       Total          Free\n"
          "name ltr type R S system  Tbl size size          KB            KB  "
            "Hd Sct   Cyl\n", stdout);

    diqab.func = QFNC_WAIT|QFNC_DEVCHAR;
    diqab.option = XO_NOMOUNT|XO_PHYS|CF_VALUES;
    diqab.buffer1 = NULL;
    diqab.buffer2 = (char *)&dichar;
    diqab.parm = NULL;

    while (--numunits >= 0)
    {
        printf("%-6.6s", unitnames);
		strmov(strnmov(temp, unitnames, 8), ":");

        if ((diqab.handle = svcIoOpen(XO_PHYS|XO_NOMOUNT, temp, NULL)) < 0)
			showerror(diqab.handle);
		else
		{
			rtn = svcIoQueue(&diqab);
			svcIoClose(diqab.handle, 0);
			if (rtn < 0 || (rtn = diqab.error) < 0)
				showerror(rtn);
			else
			{
				printf(" %1.1s ", dichar.dosname.value);
				if (dichar.partn.value == (char)0xFF)
					dichar.partn.value = 0;
				partnc = dichar.partn.value & 0x7F;
				printf("%-4.4s %s %-8.8s %c%c%5ld",
						dichar.unittype.value,
            		    (dichar.remove.value[0] == 'N') ? "F  " :
                			((dichar.msensor.value[0] == 'N') ? "R N" : "R Y"),
						(stricmp(dichar.fstype.value, "NOTMNTD") == 0 ||
							stricmp(dichar.fstype.value, "NONE") == 0) ? "" :
							dichar.fstype.value,
						(dichar.partn.value & 0x80) ? 'E' : ' ',
						(partnc) ? partnc+'0' : ' ',
						dichar.cblksz.value);
		        if (stricmp(dichar.fstype.value, "NOTMNTD") == 0)
					fputs("    Not mounted                ", stdout);
				else if (stricmp(dichar.fstype.value, "NONE") == 0)
					fputs("    No file structure          ", stdout);
    	    	else
        		{
            		total = dichar.clusters.value * dichar.clssz.value;
            		avail = dichar.avail.value * dichar.clssz.value;
           	 	printf("%3d%,12d.%c%,12d.%c", dichar.clssz.value, total/2,
            		    (total & 1)? '5': '0', avail/2, (avail & 1)? '5': '0');
        		}
        		printf("%4d%4d%6d\n", dichar.cheads.value, dichar.csects.value,
            		    dichar.ccylns.value);
			}
		}
		unitnames += 8;
    }
    return (TRUE);
}


static void showerror(
	long code)
{
	char text[100];

	svcSysErrMsg(code, 0x03, text);	
	printf("? %s\n", text);
}


//************************************************************
// Function: strxcmp - Compare two disk names so that variable
//				length numeric parts compare as expected
// Return: 1 if s1 > s2, 0 if s1 == s2, -1 if s1 < s2
//************************************************************

static int strxcmp(
	char *s1,
	char *s2,
	int   len)
{
	int  d1;
	int  d2;
	char c1;
	char c2;

	while (--len >= 0)
	{
		c1 = *s1++;
		c2 = *s2++;
		if (c1 == c2)
		{
			if (c1 == 0)
				return (0);
		}
		else
		{
			if (c1 == 0)
				return ((c2 == 0) ? 0 : -1);
			if (c2 == 0)
				return (1);
			d1 = isdigit(c1);
			d2 = isdigit(c2);
			if (d1 ^ d2)
				return ((d1) ? 1 : -1);
			else
			{
				if (d1)
					return ((getval(c1, &s1) > getval(c2, &s2)) ? 1 : -1);
				else
					return ((c1 < c2)? 1 : -1);
			}
		}
	}
	return (0);
}


//***************************************************
// Function: getval - Get decimal value from a string
//				and advance pointer past the value
// Returned: Value
//***************************************************

static int getval(
	int    chr,
	char **pnt)

{
	int value;

	value = chr & 0x0F;
	while ((chr = **pnt) != 0 && isdigit(chr))
	{
		(*pnt)++;
		value = value * 10 + (chr & 0x0F);
	}
	return (value);
}
