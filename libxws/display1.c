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

#include <xosxxws.h>


//********************************************************************
// Function: xwsDrawTextCentered - Display a text string centered at a
//				specified position using the specified font and color
// Returned: The new current text X position
//********************************************************************

// This function uses the Pascal calling sequence

long XOSFNC xwsDrawTextCentered(
	XWSWIN *win,
	char *text,
	long  length,
	long  xpos,
	long  ypos,
	XWSFNT *font,
	long  color)
{
	if ((length & 0x80000000) == 0)		// Should this be scalled?
	{
		xpos = (xpos + win->xsfr) / win->xsfd; // Yes - scale the values
		ypos = (ypos + win->ysfr) / win->ysfd;
	}
	else
		length &= ~0x80000000;
	xpos -= (xwsfontgetstrlengthns(font, text, length) / 2);
	return (xwsDrawText(win, text, length | 0x80000000, xpos, ypos, font,
			color));
}


//*************************************************************
// Function: xwsDrawText - Display a text string at a specified
//				position using the specified font and color
// Returned: The new current text X position
//*************************************************************

// This function uses the Pascal calling sequence

long XOSFNC xwsDrawText(
	XWSWIN *win,
	char *text,
	long  length,
	long  xpos,
	long  ypos,
	XWSFNT *font,
	long  color)
{
	long chr;
	long chr2;
	long chr3;
	long chr4;

	if (length & 0x7F000000)
		xwsFail(0, "Invalid count in xwsDrawText");
	if ((length & 0x80000000) == 0)
	{
		xpos = (xpos + win->xsfr) / win->xsfd;
		ypos = (ypos + win->ysfr) / win->ysfd;
	}
	else
		length &= ~0x80000000;
	xpos += win->xorg;
	ypos += win->yorg;
	while (--length >= 0) /// && (chr = *text++) != 0)
	{
		chr = *text++;
		if (chr & 0x80)					// Unicode multibyte character?
		{
			if ((chr & 0xE0) == 0xC0)	// 2-byte character?
			{
				if (--length < 0)
					break;

				if (((chr2 = *text++) & 0xC0) != 0x80)
					continue;
				chr = (chr2 & 0x3F) | ((chr & 0x1F) << 6);
			}
			else if ((chr & 0xF0) == 0xE0) // 3-byte character?
			{
				if ((length -= 2) < 0)
					break;
				if (((chr2 = *text++) & 0xC0) != 0x80)
					continue;
				if (((chr3 = *text++) & 0xC0) != 0x80)
					continue;
				chr = (chr3 & 0x3F) | ((chr2 & 0x3F) << 6) |
						((chr & 0x0F) << 12);
			}
			else if ((chr & 0xF8) == 0xF0) // 4-byte character?
			{
				if ((length -= 3) < 0)
					break;
				if (((chr2 = *text++) & 0xC0) != 0x80)
					continue;
				if (((chr3 = *text++) & 0xC0) != 0x80)
					continue;
				if (((chr4 = *text++) & 0xC0) != 0x80)
					continue;
				chr = (chr4 & 0x3F) | ((chr3 & 0x3F) << 6) |
						((chr2 & 0x3F) << 6) | ((chr & 0x07) << 18);
			}
			else						// If invalid sequence - ignore it!
				continue;
		}
		xpos += xwsdispchar(win, font, chr, xpos, ypos, color);
	}
	return (xpos - win->xorg);
}
