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

#include "xosmkinst.h"
#include "zlib.h"

static uchar   *filebufr;
static uchar    needxis;
static int      zflag = Z_NO_FLUSH;

static void zlibfail(int code, char *fmt, ...);

static char appdir[] = "?:\\xosapp";
static char datadir[] = "?:\\xosdata";

static char *setupfile(char *arg, char *ext);




//**********************************************************************
// Function: copyitems - Copy items from the XID file to the target disk
// Returned: true if normal, false if error
//**********************************************************************

// This function does all of the work after the target disk is initialized
//   and formated.

int copyitems()
{
	char  *baseend;
	char  *dirend;
	HANDLE fhndl;
	ulong  size;
	ulong  len;
	ulong  amnt;
	int   itype;
	uchar   haveapp;
	uchar   havedata;
	uchar   haveos;
	char  namebufr[600];

	// When get here we are ready to start reading the compressed items

	needxis = false;
	haveapp = false;
	havedata = false;
	haveos = false;
	namebufr[0] = drvltr;
	namebufr[1] = ':';
	namebufr[2] = '\\';
	while (TRUE)
	{
		if (!readitemhead())
			return (false);
		itype = itemhead.h.flag & IF_TYPE;
		if (itype == IT_END)
			break;
		if (itemhead.h.flag & IF_ISET)	// Is this item to be added to an
										//   install set?
			needxis = true;				// Yes
		if (itemhead.h.flag & IF_COPY)	// Is this item to be copied to the
		{								//   boot disk?
			if (itemhead.h.namelen > 198) // Yes - is the name too long?
			{
				setmsg("? Item name is too long: %s", itemhead.name);
				return (false);
			}
			if (itype < IT_FILE )		// Is this a directory name?
			{							// Yes
				if (itype == IT_SBDIR)	// Is this a sub-directory?
				{
					*baseend = '\\';
					dirend = strmov(baseend + 1, itemhead.name);
				}
				else					// If a root directory
				{
					dirend = baseend = strmov(namebufr + 3, itemhead.name);
					if (itemhead.h.filelen != 0 && !skipdata())
						return (FALSE);
				}
				setmsg("Creating directory %s", namebufr);
				if (!createdir(namebufr))
					return (false);
			}
			else						// If this is a file
			{
				if (itemhead.h.namelen > 198)
				{
					setmsg("? File name is too long: %s", itemhead.name);
					return (false);
				}
				*dirend = '\\';
				strmov(dirend + 1, itemhead.name);
				setmsg("Copying file %s", namebufr);
				if ((fhndl = CreateFile(namebufr, GENERIC_WRITE, 0, NULL,
						CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0)) ==
						INVALID_HANDLE_VALUE)
				{
					seterrmsg("Error creating file %s", namebufr);
					return (false);
				}
				itemhead.h.cdt -= (0x701CE1722770000 - tzone);
				if (!SetFileTime(fhndl, (FILETIME *)&itemhead.h.cdt,
						(FILETIME *)&itemhead.h.cdt,
						(FILETIME *)&itemhead.h.cdt))
				{
					seterrmsg("Error setting date and time for file %s",
							namebufr);
					return (false);
				}
				while (itemhead.h.filelen != 0)
				{
					if ((amnt = itemhead.h.filelen) > DSKBUFRSZ)
						amnt = DSKBUFRSZ;
					if ((len = readxid(dskbufr, amnt)) != amnt)
					{
						seterrmsg("? Read from the XID file was "
								"incomplete: %s", namebufr);
						CloseHandle(fhndl);
						return (false);
					}
					if (!WriteFile(fhndl, dskbufr, amnt, &size, NULL))
					{
						seterrmsg("Error writing file %s", namebufr);
						CloseHandle(fhndl);
						return (false);
					}
					if (size != amnt)
					{
						seterrmsg("Incomplete output to file %s",
								namebufr);
						CloseHandle(fhndl);
						return (false);
					}
					itemhead.h.filelen -= amnt;
				}
				if (!CloseHandle(fhndl))
				{
					seterrmsg("Error closing file %s", namebufr);
					return (false);
				}
			}
		}
		else							// If item is not to be copied to
		{								//   boot disk - just skip it for
			if (!skipdata())			//   now
				return (false);
		}
	}
	return (finishxid());
}


//****************************************************
// Function: makeinstallsets - Create the install sets
// Returned: true if OK, false if error
//****************************************************

int makeinstallsets()
{
	ulong  len;
	ulong  amnt;
	int   itype;
	uchar   haveapp;
	uchar   haveos;
	char    xisname[256];

	if (!needxis)						// Do we need to make any install
										//   sets?
		return (true);					// No - nothing needed here!
	haveapp = false;
	haveos = false;
	xisname[0] = drvltr;
	strmov(xisname + 1, ":\\xosinst-");
	while (TRUE)
	{
		if (!readitemhead())
			return (false);
		itype = itemhead.h.flag & IF_TYPE;
		if (itype == IT_END)
			break;
		if (itemhead.h.flag & IF_ISET)	// Is this item to be copied to an
		{								//   install set?
			if (itype < IT_RTDIR)		// Yes - is this the start of an
			{							//   install set?
				if (itemhead.h.namelen > 95) // Yes
				{
					setmsg("? Install set name is too long");
					return (false);
				}
				if (xisname[11] != 0 && !finishxis()) // Finish up current
					return (false);					  //   install set if one
													  //   is pending
				if (readxid((uchar *)xisname + 11, itemhead.h.filelen) < 0)
					return (FALSE);
				memcpy(xisname + 11 + itemhead.h.filelen, ".xis", 5);
				itemhead.h.filelen = 0;
				setmsg("Creating install set %s", xisname);
				if (!startxis(xisname, itemhead.h.cdt))
					return (false);
			}
			if (xisname[11] == 0)		// Invalid sequence?
			{
				setmsg("? Invalid item sequence in the XID file");
				return (false);
			}
			setmsg("Storing item %s", itemhead.name);

			// Copy the item header

			itemhead.h.seqnum = (char)xishead.totalitems++;
			itemhead.h.flag &= 0x0F;
			if (!writexis((uchar *)&itemhead, sizeof(ITEMHEAD) +
					itemhead.h.namelen))
				return (false);

			// Copy the item data

			while (itemhead.h.filelen != 0)
			{
				if ((amnt = itemhead.h.filelen) > DSKBUFRSZ)
					amnt = DSKBUFRSZ;
				if ((len = readxid(dskbufr, amnt)) != amnt)
				{
					seterrmsg("? Read from the XID file was "
							"incomplete: %s", xisname);
					return (false);
				}
				if (!writexis(dskbufr, amnt))
					return (false);
				itemhead.h.filelen -= amnt;
			}
		}
		else
			if (!skipdata())
				return (false);
	}
	return (finishxis() && finishxid());
}
