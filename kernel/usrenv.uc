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

#include <xos.h>
#include <xosx.h>
#include <xossvc.h>
#include <xoslib.h>
#include <xoserr.h>
#include <stddef.h>
#include <ctype.h>

// Environment strings are implimented completely in user space. The are stored
//   in the same msect used to pass arguments to a process. 

// Environment strings can be defined at the current process level, at the
//   session level (assuming that the session process handles the VECT_SETENV
//   signal) and at system level.

// Environment strings are stored in the high user memory (HUM) area immediately
//   above the program arguments. Each environment string has the following
//   format:
// Offset Size Description
//   0      2  Sync value = 0xA33A
//   2      2  Length of the environment string (does not include the final
//               0 byte or any fill bytes)
//   4      N  The string (name=value) (0 terminated, may be followed by up to
//               3 additional 0 bytes if needed to long align the next item)
// Each item starts on a long boundry. The final item is followed by a 4-byte
//   entry consisting of a sync value and a count of 0.

// These functions are NOT thread or signal safe. If they are called in a
//   thread environment or at signal level, the caller must ensure that they
//   will not be re-entered by a different thread or at a different signal
//   level. This is true even in a non-preemptive thread environment for
//   non-local requests since in this case rescheduling will occur.

static int XOSFNC scanenv(const char *name, const ENVSTR **env);


//*********************************************************
// Function: sysEnvGetAll - Returns all environment strings
// Returned: Total length of the string returned if normal
//				or a negative XOS error code if error
//*********************************************************

// The environment strings are tightly stored in the caller's buffer with
//   a single 0 byte terminating each string. Each string has the format
//   name=string. An additional 0 bytes terminates the list. The length value
//   returned includes the 0 bytes that terminate each string but does not
//   include the final extra 0 byte.

XOSCALL sysEnvGetAll(
	char *buffer,		// Address of buffer (may be NULL to just
						//   return length needed)
	long  length)		// Length of the buffer
{
	union
	{	ENVSTR *e;
		char   *c;
	}    epnt;
	long total;

	epnt.e = humArea.envaddr;
	total = 1;
	while (TRUE)
	{
		if (epnt.e->sync != ENVSYNC)
			return (ER_BDENV);
		if (epnt.e->length == 0)
			break;
		total += (epnt.e->length + 1);
		if (buffer != NULL)
		{
			if ((length -= (epnt.e->length + 1)) <= 0)
				return (ER_BFRTS);
			buffer += (sysLibStrMov(buffer, epnt.e->str) + 1);
		}			
		epnt.c += ((epnt.e->length + 8) & 0xFFFFFFFC);
	}
	if (buffer != NULL)
		*buffer = 0;
	return (total);
}


//*************************************************************
// Function: sysEnvGet - returns a specified environment string
// Returned: Total length of the string returned if normal or a
//				negative XOS error code if error
//*************************************************************

XOSCALL sysEnvGet(
	const char  *name,	// Name to search for (no wild-card characters)
	const char **pstr)	// Address of pointer to receive address of
						//   the environment string
{
	const ENVSTR *epnt;
	long    rtn;

	if ((rtn = scanenv(name, &epnt)) < 0)
		return (rtn);
	if (rtn == 0)
		return (ER_NTDEF);
	*pstr = &epnt->str;
	return (epnt->length);
}


//*********************************************************
// Function: sysEnvFind - Returns the specified environment
//				string - wild-card searchs are allowed
// Returned: Total length of the string returned if normal
//				a negative XOS error code if error
//*********************************************************

XOSCALL sysEnvFind(
	cchar  *name,		// Address of name to search for (may contain
						//   wild-card characters)
	cchar **found)		// Address of pointer to receive address of the
						//   environment string - This pointer also specifies
						//   where to start searching. Shoud be set to NULL
						//   for a new search. The returned pointer points to
						//   the beginning of the entire envionment string
						//   (name=string).
{
	(void)name;
	(void)found;

	return (ER_NTDEF);
}


//************************************************************
// Function: sysEnvDefine - Defines an environment string
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

XOSCALL sysEnvDefine(
	long   level,		// 0 = This process
						// 1 = Session process (session process must
						//     be able to process a VECT_SETENV signal)
						// 2 = System level
	cchar *str)			// Address of environment string (name=string)
						//   string to delete the environment string)
{
	ENVSTR *epnt;		// Address of the environment string in HUM
	cchar  *define;
	char   *newavl;		// New value for humArea.avladdr
	char   *cend;		// Current address of first byte above the string (This
						//   will be the address of the end marker if the
						//   string is not currently defined.)
	char   *pnt;
	long    rtn;
	int     nlen;		// Length of the name of the environment string
	int     clen;		// Current length of the environment string
	int     dlen;		// Length of the new definition string
	int     tlen;		// Total length of the entironment string including
						//   the header, the final 0 and any padding needed
	int     mlen;		// Amount to move above the string
	int     mdist;		// Distance to move the area above the string (+ means
						//   move up, - means move down)

	if (level != 0)
	{





	}

	define = ((nlen = sysLibStrChr(str, '=')) < 0) ? NULL : (str + nlen + 1);
	if ((rtn = scanenv(str, (const ENVSTR **)&epnt)) < 0)
		return (rtn);

	if (define == NULL)
	{
		if (nlen == 0)
			return (ER_NTDEF);
		tlen = 0;
		clen = (epnt->length + 8) & 0xFFFFFFFC;
	}
	else
	{
		dlen = sysLibStrLen(define);
		if (rtn == 0)					// Is the string defined now?
		{
			epnt = (ENVSTR *)((char *)humArea.logaddr - 4);
			clen = 0;					// No - put it at the end of the
		}
		else
			clen = (epnt->length + 8) & 0xFFFFFFFC;
		tlen = (nlen + dlen + 10) & 0xFFFFFFFC;
	}
	mlen = ((long)humArea.avladdr) - ((long)epnt) - clen;
	mdist = tlen - clen;
	cend = (char *)epnt + clen;
	newavl = humArea.avladdr + mdist;
	if (mdist > 0)						// Increasing size of the HUM?
	{
		if (newavl > humArea.topaddr)	// Need to get more memory?
		{
			if ((rtn = svcMemChange(&humArea, PG_READ|PG_WRITE,
					newavl - (char *)&humArea)) < 0)
				return (rtn);
			humArea.topaddr = (char *)&humArea + rtn;
		}
		sysLibMemRCpyLong(cend + mdist, cend, mlen >> 2); // Increase space for
	}													  //   string
	else if (mdist < 0)					// Reducing size of the HUM?
	{
///		INT3;

		if (newavl < (humArea.topaddr - 0x1000))
										// Should we reduce the size?
		{								// Yes (We don't reduce it usless we
										//   can give up more than 1 page to
										//   prevent thrashing on small changes)
			if ((rtn = svcMemChange(&humArea, PG_READ|PG_WRITE,
					newavl - (char *)&humArea)) < 0)
				return (rtn);
			humArea.topaddr = (char *)&humArea + rtn;
		}
		sysLibMemCpyLong((long *)(cend + mdist), (long *)cend, mlen >> 2);
	}									// Reduce space for string
	if (tlen > 0)
	{
		epnt->sync = ENVSYNC;
		epnt->length = nlen + dlen + 1;
		pnt = epnt->str + sysLibMemCpy(epnt->str, str, nlen); // Store name
		*pnt++ = '=';
		pnt += (sysLibStrMov(pnt, define) + 1); // Store definition
		while (((long)pnt & 0x03) != 0)	// Clear the padding bytes
			*pnt++ = 0;
	}
	humArea.logaddr += mdist;
	humArea.pathaddr += mdist;
	humArea.avladdr += mdist;
	return (0);
}


//********************************************************
// Function: scanenv - Search for environment string
// Returned: Length of name part if found, 0 if not found,
//				or a negative XOS error code if error
//********************************************************

static int XOSFNC scanenv(
	const char    *name,
	const ENVSTR **env)
{
	union
	{	char   *c;
		ENVSTR *e;
	}     epnt;
	char *cpnt;
	int   len;
	char  chr;

	epnt.e = (ENVSTR *)(humArea.envaddr);
	while (epnt.e->sync == ENVSYNC)
	{
		if (epnt.e->length == 0)
			return (0);
		cpnt = epnt.e->str;
		while ((chr = *cpnt) != 0 && chr != '=')
			cpnt++;
		if (chr != 0)
		{
			len = cpnt - epnt.e->str;
			if (sysLibStrNCmp(name, epnt.e->str, len) == 0 &&
					(name[len] == 0 || name[len] == '='))
			{
				*env = epnt.e;
				return (len);
			}
		}
		epnt.c += ((epnt.e->length + 8) & 0xFFFFFFFC);
	}
	return (ER_BDENV);					// Should never get here!
}
