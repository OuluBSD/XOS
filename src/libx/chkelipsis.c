//==============================================================//
// Module: chkelipsis - Check for elipsis in file specification //
//==============================================================//

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

#include <stdio.h>
#include <string.h>
#include <xcstring.h>
#include <ctype.h>


//***************************************************************
// Function: chkelipsis - Check for elipsis in file specification
// Returned: TRUE (1) if valid elipsis, 0 otherwise
//***************************************************************

// If a valid elipsis is found it is removed.

int chkelipsis(
	char *spec)
{
	char *pnt;
	char  chr;

	if ((pnt = strrchr(spec, '\\')) != NULL && (pnt == (spec + 4) ||
			(chr = pnt[-4]) == '\\' || chr == ':') && strncmp(pnt - 3,
			"...", 3) == 0)
	{
		strmov(pnt - 3, pnt);
		return (TRUE);
	}
	return (FALSE);
}
