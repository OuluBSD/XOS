// ++++
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

#include "geckoh.h"


int getname(
	int   wildok,
	char *atom,
	int   size)
{
	char *pnt;
	char  chr;

	pnt = atom;
	chr = nxtnbc();
	do
	{
		if (chr == 0 || isspace(chr) || chr == ',')
		{
			*pnt = 0;
			hldchar = stopper = chr;
			return (pnt - atom);
		}
		if (isalpha(chr) || isdigit(chr) || chr == '_' || chr == '$' ||
				(wildok && (chr == '?' || chr == '*')))
			*pnt++ = chr;
		else
		{
			putstr("\r\n\1? Invalid symbol name\2\r\n");
			return (-1);
		}
		chr = nxtchar();
	} while (--size > 1);
	putstr("\r\n\1? Symbol name is too long\2\r\n");
	return (-1);
}


int nxtnbc(void)
{
    int chr;

    do
    {
		chr = nxtchar();				// Get next input character
    } while (chr != '\n' && isspace(chr));
    return (chr);
}


int nxtnb0(
	int chr)

{
    while (chr != '\n' && isspace(chr))
		chr = nxtchar();
    return (chr);
}


int nxtchar(void)

{
	int chr;

	if (hldchar != 0)
	{
		chr = hldchar;
		hldchar = 0;
	}
	else if ((chr = *cmdpnt) != 0)
		cmdpnt++;
	return (chr);
}
