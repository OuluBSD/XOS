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

#include <xxws.h>


typedef struct
{	XWSTB *tb;
	XWSTL *tl;
	char  *pnt;
	int    cnt;
} GTLC;

static int gettlc(GTLC *gtlc);

//*******************************************************************
// Function: xwsfindtextboxbystr - Find a text box given its contents
// Returned: Address of the TB
//*******************************************************************

//  Value is address of the text box if it was found or NULL otherwise. ptb
//  receives the address of the previous TB if the requested TB was found. If
//  the requested TB is the first, this is NULL. If the requested TB was not
//  found it is the address of the TB before where the requested TB would be
//  inserted or NULL if it would be the first TB.

//If lines in the text box were terminated with a \n character, the string
//  should contain that character. If lines were word wrapped, no \n character
//  should be included. All formating bytes are ignored in both the string and
//  the text boxes searched.

///// NEED TO REWRITE THIS !!!!!

XWSTB *xwsfindtextboxbystr(
	XWSWIN *win,
	char   *str,
	long    len,
	XWSTB **ptb)

{
	XWSTB *prevtb;
	XWSTB *tb;
	GTLC   gtlc1;
	GTLC   gtlc2;
	int    diff;
	int    chr1;
	int    chr2;

	prevtb = NULL;
	tb= win->tbhead;	
	while (tb != NULL && tb->inx < ((XWSLIST *)win)->alphamin)
	{
		prevtb = tb;
		tb = tb->next;
	}
	while (tb != NULL && tb->inx < ((XWSLIST *)win)->alphamax)
	{
		gtlc1.tl = NULL;
		gtlc1.cnt = len;
		gtlc1.pnt = str;
		gtlc2.tl = tb->tlhead;
		gtlc2.cnt = gtlc2.tl->count;
		gtlc2.pnt = gtlc2.tl->text;

		diff = 0;

		while (TRUE)
		{
			chr1 = gettlc(&gtlc1);
			chr2 = gettlc(&gtlc2);
			if (chr1 < 0 && chr2 < 0)
			{
				*ptb = prevtb;
				return (tb);
			}
			if (chr1 != chr2)
			{
				if (chr1 > chr2)
					break;
				*ptb = prevtb;
				return (NULL);
			}
		}
		prevtb = tb;
		tb = tb->next;
	}
	*ptb = prevtb;
	return (NULL);
}


static int gettlc(
	GTLC *gtlc)

{
	int chr;
	int fnc;
	int len;

	while (TRUE)
	{
		while (gtlc->cnt > 0)
		{
			gtlc->cnt--;
			if ((chr = *gtlc->pnt++) >= 0x10 && chr <= 0x12)
			{							// Formatting function?
				if (gtlc->cnt-- < 0)	// Must have at least 1 byte following
					return (-1);
				fnc = *gtlc->pnt++;		// Get the function byte
				if (chr == 0x10)
					continue;
				if (chr == 0x11)		// Short function?
					len = (gtlc->tl == NULL) ?
							((fnc & 0x03) + 1) : // If not stored
							((fnc == 0x1C || fnc == 0x20) ? 16 : 4);
										// If string has been stored
				else
				{						// If long function
					if (gtlc->tl == NULL) // Stored?
					{
						if (gtlc->cnt-- < 0) // Must have at least 1 more byte
							return (-1);
						if ((len = *gtlc->pnt++) & 0x80) // Get first count byte
						{
							if (gtlc->cnt-- < 0) // If have 2 byte count
								return (-1);
							len = ((len & 0x7F) << 8) + (uchar)*gtlc->pnt++;
						}
					}
					else
					{
						if ((gtlc->cnt -= 2) < 0) // Stored string - count is
							return (-1);		  //   always 2 bytes
						len = *(ushort *)gtlc->pnt;
						gtlc->pnt += 2;
					}
				}
				if ((gtlc->cnt -= len) < 0)
					return (-1);
				gtlc->pnt += len;
			}
			else
				return (chr);
		}
		if (gtlc->tl == NULL)
			break;
		else
		{
			if (gtlc->cnt == 0)
			{
				if (gtlc->tl->fmt.bits & TLB_EOL)
				{
					gtlc->cnt--;
					return ('\n');
				}
				if (gtlc->tl->fmt.bits & TLB_EOP)
				{
					gtlc->cnt--;
					return ('\v');
				}
			}
			if ((gtlc->tl = gtlc->tl->next) != NULL)
			{
				gtlc->cnt = gtlc->tl->count;
				gtlc->pnt = gtlc->tl->text;
			}
			else
				break;
		}
	}
	return (-1);
}
