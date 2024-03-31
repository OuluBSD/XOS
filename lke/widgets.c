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
#include <string.h>
#include <xosstr.h>
#include <xos.h>
#include <xoserrmsg.h>
#include <xossvc.h>


// Define structure for path block items

typedef struct
{	char node;
	char dev : 4;
	char inx : 4;
	char type: 4;
	char pos : 4;
	char end : 1;
	char voli: 1;
	char volo: 1;
	char eapd: 1;
	char colr: 4;
} WDB;


char *labels[] =
{	"DAC",			// 0 - Audio output (DAC)
	"ADC",			// 1 - Audio input (ADC)
	"MIX",			// 2 - Audio mixer
	"SEL",			// 3 - Audio selector
	"PIN",			// 4 - Pin complex (jack)
	"PWR",			// 5 - Power control
	"VOL",			// 6 - Volume knob
	"BEP",			// 7 - Beep generator
	"0x8",			// 8 - Invalid
	"0x9",			// 9 - Invalid
	"0xA",			// A - Invalid
	"0xB",			// B - Invalid
	"0xC",			// C - Invalid
	"0xD",			// D - Invalid
	"0xE",			// E - Invalid
	"VEN"			// F - Vendor defined
};

char *devices[] = 
{	"Line out",		// 0
	"Speaker",		// 1
	"HP out",		// 2
	"CD",			// 3
	"SPDIF out",	// 4
	"Digital out",	// 5
	"Modem line",	// 6
	"Modem handset",// 7
	"Line in",		// 8
	"Aux",			// 9
	"Mic in",		// A
	"Telephony",	// B
	"SPDIF in",		// C
	"Digital in",	// D
	"0xE",			// E
	"Other"			// F
};

char *colors[] =
{	"Black",		// 1
	"Grey",			// 2
	"Blue",			// 3
	"Green",		// 4
	"Red",			// 5
	"Orange",		// 6
	"Yellow",		// 7
	"Purple",		// 8
	"Pink",			// 9
	"Color=0xA",	// A
	"Color=0xB",	// B
	"Color=0xC",	// C
	"Color=0xD",	// D
	"White",		// E
	"Other color"	// F
};

char *positions[] =
{	"Rear",			// 1
	"Front",		// 2
	"Left",			// 3
	"Right",		// 4
	"Top",			// 5
	"Bottom",		// 6
	"Special-7",	// 7
	"Speclal-9",	// 8
	"Special=0",	// 9
	"Pos=0xA",		// A
	"Pos=0xB",		// B
	"Pos=0xC",		// C
	"Pos=0xD",		// D
	"Pos=0xE",		// E
	"Pos=0xF"		// F
};

#define CWT_PIN 4

_Packed struct
{	LNGSTRCHAR wd;
	char       end;
} widchars =
{	PAR_GET|REP_DATAS, 0, "WIDGETS", NULL, 512};

char prgname[] = "WIDGETS";

void main(
    int   argc,
    char *argv[])
{
	WDB   widdata[512/4];
	WDB  *wpnt;
	char *cpnt;
	long  dev;
	long  rtn;
	int   color;
	int   position;
	int   device;
	char  buffer[100];
	char  devname[20];
	char  ntxt[12];
	char  first;

    if (argc > 2)
    {
        fputs("? Command error, usage is:\n"
              "    WIDGETS snddev\n", stderr);
        exit(1);
    }
	cpnt = strnmov(devname, (argc == 2) ? argv[1] : "SND0:", 16);
	if (cpnt[-1] != ':')
	{
		cpnt[0] = ':';
		cpnt[1] = 0;
	}
    if ((dev = svcIoOpen(0, devname, NULL)) < 0)
        errormsg(dev, "Error opening sound device");
	widchars.wd.buffer = (char *)widdata;
    if ((rtn = svcIoDevChar(dev, &widchars)) < 0)
        errormsg(rtn, "!Error getting widget data");
	if (widchars.wd.strlen >= 512)
		errormsg(0x80000000, "!Too many widgets");

	color = 0;
	position = 0;
	wpnt = widdata;
	device = -1;
	while (widchars.wd.strlen > 0)
	{
		first = TRUE;
		cpnt = strmov(buffer, "  ");
		do
		{
			if (cpnt > (buffer + 79))
			{
				fputs(buffer, stdout);
				cpnt = strmov(buffer, "      ");
			}

			if (wpnt->type == CWT_PIN)
			{
				color = wpnt->colr;
				position = wpnt->pos;
				device = wpnt->dev;
			}

			if (!first)
				sprintf(ntxt, "%X ", wpnt->inx);
			else
			{
				first = FALSE;
				ntxt[0] = 0;
			}
			cpnt += sprintf(cpnt, "%s%s(%02X)%s%s", ntxt, labels[wpnt->type],
					wpnt->node, (wpnt->volo) ? "o" : (wpnt->voli) ?
						((wpnt->type == CWT_PIN) ? "b" : "i") : " ",
					(wpnt->end) ? "" : "-->");
			wpnt++;
			widchars.wd.strlen -= 4;
		} while (!(wpnt - 1)->end);

		cpnt += sprintf(cpnt, " %s", (wpnt->eapd) ? "A" : " ");

		if (device >= 0)
			cpnt += sprintf(cpnt, " %s,", devices[device]);

		if (color == 0 && position == 0)
		{
			cpnt[0] = '\n';
			cpnt[1] = 0;
		}
		else
		{
			*cpnt++ = ' ';
			if (color != 0)
				cpnt += sprintf(cpnt, "%s%s", colors[color - 1],
						(position == 0) ? "" : ", ");
			if (position != 0)
				sprintf(cpnt, "%s\n", positions[position - 1]);
			else
			{
				cpnt[0] = '\n';
				cpnt[1] = 0;
			}
		}

		fputs(buffer, stdout);
	}
	exit(0);
}
