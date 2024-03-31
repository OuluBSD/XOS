//============================================
// parsehid,c
// Written by John Goltz
//============================================

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

#include "usbctl.h"


static char *iovalue(long value);


void parsehid(
	uchar *bufr,
	int    left)

{
	long uvalue;
	long svalue;
	int  header;
	int  size;
	int  type;
	int  tag;
	int  indent;
	char text[32];

	indent = 1;
	while (left > 0)
	{
		if ((header = *bufr++) != 0xFE)
		{
			left--;
			if ((size = header & 0x03) == 3)
				size = 4;
			type = (header >> 2) & 0x03;
			tag = header >> 4;

		}
		else
		{
			left -= 3;
			size = *bufr++;
			tag = *bufr++;
		}
		if (left < size)
			break;

		if (size == 0)
			uvalue = svalue = 0;
		else if (size == 1)
		{
			uvalue = *(uchar *)bufr;
			svalue = *(char *)bufr;
		}
		else if (size == 2)
		{
			uvalue = *(ushort *)bufr;
			svalue = *(short *)bufr;
		}
		else
			uvalue = svalue = *(long *)bufr;
		left -= size;
		bufr += size;

		switch (type)
		{
		 case 0:						// MAIN items
			switch (tag)
			{
			 case 0x08:					// Input item
				debugprint("|M:%*sInput (%s)", indent, "", iovalue(uvalue));
				break;

			 case 0x09:					// Output item
				debugprint("|M:%*sOutput (%s)", indent, "", iovalue(uvalue));
				break;

			 case 0x0A:					// Collection item
				if (svalue < 0 || svalue > 2)
					sprintf(text, "0x%02.2X", svalue);
				else
					strcpy(text, (uvalue == 0) ? "Physical" : (uvalue == 1) ?
							"Application" : "Logical");
				debugprint("|M:%*sCollection (%s)", indent, "", text);
				indent += 2;
				break;

			 case 0x0B:					// Feature item
				debugprint("|M:%*sFeature (%s)", indent, "", iovalue(uvalue));
				break;

			 case 0x0C:				// End collection item
				if ((indent -= 2) < 1)
					indent = 1;
				debugprint("|M:%*sEnd collection", indent, "");
				break;

			 default:
				debugprint("|M:%*sTag:%02.2X (%08.8X)", indent, "", tag,
						uvalue);
				break;
			}
			break;

		 case 1:						// GLOBAL items
			switch (tag)
			{
			 case 0x00:					// Usage page
				debugprint("|G:%*sUsage page (%u)", indent, "", uvalue);
				break;

			 case 0x01:					// Logical minimum
				debugprint("|G:%*sLogical minimum (%d)", indent, "", svalue);
				break;

			 case 0x02:					// Logical maximum
				debugprint("|G:%*sLogical maximum (%d)", indent, "", svalue);
				break;

			 case 0x03:					// Physical minimum
				debugprint("|G:%*sPhysical minimum (%d)", indent, "", svalue);
				break;

			 case 0x04:					// Physical maximum
				debugprint("|G:%*sPhysical maximum (%d)", indent, "", svalue);
				break;

			 case 0x05:					// Unit exponent
				debugprint("|G:%*sUnit exponent (%u)", indent, "", uvalue);
				break;

			 case 0x06:					// Unit
				debugprint("|G:%*sUnit (%u)", indent, "", uvalue);
				break;

			 case 0x07:					// Report size
				debugprint("|G:%*sReport size (%u)", indent, "", uvalue);
				break;

			 case 0x08:					// Report ID
				debugprint("|G:%*sReport ID (%u)", indent, "", uvalue);
				break;

			 case 0x09:					// Report count
				debugprint("|G:%*sReport count (%u)", indent, "", uvalue);
				break;

			 case 0x0A:					// Push
				debugprint("|G:%*sPush", indent, "");
				break;

			 case 0x0B:					// Pop
				debugprint("|G:%*sPop", indent, "");
				break;

			 default:
				debugprint("|G:%*sTag:%02.2X (%08.8X)", indent, "", tag,
						uvalue);
				break;
			}
			break;

		 case 2:						// LOCAL items
			switch (tag)
			{
			 case 0x00:					// Usage
				debugprint("|L:%*sUsage (%u)", indent, "", uvalue);
				break;

			 case 0x01:					// Usage minimum
				debugprint("|L:%*sUsage minimum (%d)", indent, "", svalue);
				break;

			 case 0x02:					// Usage maximum
				debugprint("|L:%*sUsage maximum (%d)", indent, "", svalue);
				break;

			 case 0x03:					// Designator index
				debugprint("|L:%*sDesignator index (%d)", indent, "", svalue);
				break;

			 case 0x04:					// Designator minimum
				debugprint("|L:%*sDesignator minimum (%d)", indent, "", svalue);
				break;

			 case 0x05:					// Designator maximum
				debugprint("|L:%*sDesignator maximum (%d)", indent, "", svalue);
				break;

			 case 0x07:					// String index
				debugprint("|L:%*sString index (%u)", indent, "", uvalue);
				break;

			 case 0x08:					// String minimum
				debugprint("|L:%*sString minimum (%u)", indent, "", uvalue);
				break;

			 case 0x09:					// String maximum
				debugprint("|L:%*sString delimiter (%u)", indent, "", uvalue);
				break;

			 case 0x0A:					// Delimiter
				debugprint("|L:%*sDelimiter (%u)", indent, "", uvalue);
				break;

			 default:
				debugprint("|L:%*sTag:%02.2X (%08.8X)", indent, "", tag,
						uvalue);
				break;
			}
			break;

		 default:
				debugprint("|R:%*sTag:%02.2X (%08.8X)", indent, "", tag,
						uvalue);
				break;
			break;
		}

	}
}


static char *iovalue(
	long value)

{
	static char text[128];

	char *pnt;

	pnt = text;
	if (value & 0x0001)
		pnt = strmov(pnt, "Constant, ");
	else
		pnt = strmov(pnt, "Data, ");
	if (value & 0x0002)
		pnt = strmov(pnt, "Variable, ");
	else
		pnt = strmov(pnt, "Array, ");
	if (value & 0x0004)
		pnt = strmov(pnt, "Relative, ");
	else
		pnt = strmov(pnt, "Absolute, ");
	if (value & 0x0008)
		pnt = strmov(pnt, "Wrap, ");
	if (value & 0x0010)
		pnt = strmov(pnt, "Nonlinear, ");
	if (value & 0x0020)
		pnt = strmov(pnt, "No perfered, ");
	if (value & 0x0040)
		pnt = strmov(pnt, "Null state, ");
	if (value & 0x0080)
		pnt = strmov(pnt, "Volatile. ");
	if (value & 0x0100)
		pnt = strmov(pnt, "Buffered bytes, ");
	if (pnt != text)
		pnt -= 2;
	*pnt = 0;
	return (text);
}


char *usagepage[] =
{	"0x00",
	"Desktop",
	"Simulation",
	"VR",
	"Sport",
	"Game",
	"0x06",
	"Keyboard",
	"LEDs",
	"Button",
	"Ordinal",
	"Telephony",
	"Consumer",
	"Digitizer",
	"0x0E",
	"PID page",
	"Unicode",
	"0x11",
	"0x12"
	"0x13",
	"Alphanumeric display"
};

