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

#include "mkinstall.h"


typedef struct
{	ITEMHEAD;
	char   name[300];
} ITEMHEADX;

static char *itemtbl[] =
{	"bootstrap",		// IT_BOOT   = 0
	"file",				// IT_FILE   = 1
	"directory",		// IT_DIR    = 2
	"sub-directory"		// IT_SUBDIR = 3
};

static struct
{	BYTE4PARM length;
	BYTE8PARM cdt;
	char      end;
} oparms =
{	{PAR_GET|REP_HEXV, 4, IOPAR_LENGTH},
	{PAR_GET|REP_HEXV, 8, IOPAR_MDATE}
};

static ITEMHEADX itemhead;


static void storefiles(char *base, FILB *file);


//***************************************************
// Function: storeitems - Store items to the XID file
// Returned: Nothing (does not return if error)
//***************************************************

void storeitems(void)
{
	DIRB *dir;
	SUBB *sub;
	char *pnt;
	char  name[600];

	hviset = FALSE;
	dir = dirhead;
	while (dir != NULL)
	{
		printf("\n  Storing files for %s\\ |", dir->tgtname);
		lastspin = xidhead.totalsize;
		spininx = 0;

		// Store item for a top level directory

		if ((dir->flag & IF_TYPE) < IT_RTDIR)
			hviset = TRUE;
		store1item("", dir->tgtname, dir->flag, dir->cdt, dir);

		// Store items for files in the top level directory

		storefiles(dir->srcname, dir->filehead);

		// Store items for sub-directories

		pnt = strmov(name, dir->srcname);
		*pnt++ = '\\';
		sub = dir->subhead;
		while(sub != NULL)
		{
			store1item("", sub->tgtname, sub->flag, sub->cdt, NULL);
			strmov(pnt, sub->srcname);
			storefiles(name, sub->filehead);
			sub = sub->next;
		}
		dir = dir->next;
		fputs("\bDone", stdout);
	}
}


//***********************************************************************
// Function: storefiles - Store all files in a directory or sub-directory
// Returned: Nothing (does not return if error)
//***********************************************************************

static void storefiles(
	char *base,
	FILB *file)
{
	char *pnt;
	char  name[600];

	pnt = strmov(name, base);
	*pnt++ = '\\';
	while (file != NULL)
	{
		strmov(pnt, file->srcname);
		store1item(name, file->tgtname, file->flag, 0, NULL);
		file = file->next;
	}
}


//******************************************************
// Function: store1item - Store one item in the XID file
// Returned: Nothing (does not return if error
//******************************************************

void store1item(
	char *srcname,
	char *tgtname,
	int   flags,
	llong cdt,
	DIRB *dir)
{
	FILB *file;
	char *pnt;
	char *npnt;
	long  hndl;
	long  ziphndl;
	long  rtn;
	long  amnt;
	long  majver;
	long  minver;
	long  editnum;
	int   itype;
	int   pos;
	char  chr;
	char  hvrun;
	char  hvdigit;

	xidhead.totalitems++;

///	printf("### store1item: %02X |%s| |%s|\n", flags, srcname, tgtname);

	itype = flags & IF_TYPE;
	if (itype >= IT_FILE || itype == IT_BSTRP)
	{
		if ((hndl = svcIoOpen(XO_IN, srcname, &oparms)) < 0)
			fail(hndl, "Error opening file %s", srcname);
		itemhead.filelen = oparms.length.value;
		itemhead.cdt = oparms.cdt.value;
	}
	else if (itype < IT_RTDIR && (flags & IF_ISET))
	{
		// Here if storing the start of an install set and it will be used
		//   to create an actual install set (rather than only being installed
		//   on the created boot disk, the data part of the item contains a
		//   string that specifies (almost) the full name of the install set.
		//   This has the following format:
		//
		//       x-vendor-program-majver-minver-editnum
		//
		//   "x" is the type of the install set which is one of "o" (OS), "a"
		//   (application), or "d" (data). "vendor is the final element of the
		//   target directory name if the primary file is executable or is the
		//   first text atom from the primary file if it is a text file.
		//   "program" is the name of  the primary file without its extension.
		//   "majver" is the decimal major version number, "minver" is the
		//   decimal minor version number and "editnum" is the decimal edit
		//   number. "-edit" may be omitted if the install set does not use an
		//   edit number. The version number values are taken from the primary
		//   file. If that file is a .run file, the values are taken from the
		//   version number field in the file's header. If it is a .txt file,
		//   the version number is taken from the first line in the file which
		//   must be in this format.  The of the install set is the modification
		//   date of the primary file. The part after the | is the name of the
		//   install set file without the "xosinst-" prefix and the ".xis"
		//   extension.

		// Open the primary file

		if ((file = dir->filehead) == NULL)
			fail(0, "Internal error: No primary file for install set");
		if ((pnt = strrchr(file->tgtname, '.')) == NULL)
			fail(0, "Internal error: Primary file has no extension");
		pos = pnt - file->tgtname;
		hvrun = FALSE;
		if (stricmp(".run", pnt) == 0)
			hvrun = TRUE;
		else if (stricmp(".txt", pnt) != 0)
			fail(hndl, "Invalid extension for primary file for install set %s",
					dir->tgtname);
		sprintf(inpbufr + 600, "%s\\%s", dir->srcname, file->tgtname);

///		printf("### opening primary file: |%s|\n", inpbufr + 600);

		if ((hndl = svcIoOpen(XO_IN, inpbufr + 600, &oparms)) < 0)
			fail(hndl, "Error getting version number for %s", inpbufr + 600);
		if ((rtn = svcIoInBlock(hndl, inpbufr + 1000, 256)) < 0)
			fail(rtn, "Error getting version number for %s", inpbufr + 600);

		// Get the install set's date which is the modified date of the
		//   primary file

		itemhead.cdt = oparms.cdt.value;

		// Insert the install set type

		npnt = strmov(inpbufr, (itype == IT_OSISET) ? "o-" :
				(itype == IT_APISET) ? "a-" : "d-");

		// Insert the vendor name

		if (hvrun)						// If we have an executable primary
		{								//   file, the vendor name is the last
										//   element of the target name unless
										//   this is an OS install set. In
										//   this case it is the name of the
										//   primary file.
			if (itype == IT_OSISET)
				npnt = strnmov(npnt, file->tgtname, pos);
			else
			{
				if ((pnt = strrchr(dir->tgtname, '\\')) == NULL)
					pnt = dir->tgtname;
				else
					pnt++;
				npnt = strmov(npnt, pnt);
			}
		}
		else							// If we have a text file as the primary
		{								//   file the vendor name is taken from
										//   the first line of that file
			if ((pnt = strchr(inpbufr + 1000, '|')) == NULL)
				fail(0, "Invalid syntax in text file for install set %s",
						dir->tgtname);
			*pnt++ = 0;
			npnt = strmov(npnt, inpbufr + 1000);
		}
		*npnt++ = '-';

		// Insert the program name - If not an OS install set this is just the
		//   target name for the primary file. If an OS install set it is the
		//   last element of the directry target name.

		if (itype == IT_OSISET)
		{
			if ((pnt = strrchr(dir->tgtname, '\\')) == NULL)
				pnt = dir->tgtname;
			else
				pnt++;
			npnt = strmov(npnt, pnt);
		}
		else
			npnt = strnmov(npnt, file->tgtname, pos);
		*npnt++ = '-';

		// Insert the version number from the install set's primary file

		if (hvrun)
		{
			// Here if the primary file is an executable file - Get the version
			//   number from its header.

			if ((pos = (inpbufr + 1000)[5]) == 0) // Get position of the version
												  //   number
				fail(0, "No version number in executable file");
			majver = *(ushort *)(inpbufr + 1000 + pos);
			minver = *(ushort *)(inpbufr + 1002 + pos);
			editnum = *(long *)(inpbufr + 1004 + pos);
			npnt += sprintf(npnt, "%d-%d-%d", majver, minver, editnum);
		}
		else
		{
			// Here if the primary file is a text file - Get the version number
			//   from the first line.

			pos = 0;
			hvdigit = FALSE;
			while ((chr = *pnt++) != 0 && chr != '\n' && chr != '\r' &&
					!isspace(chr))
			{
				if (chr == '-')
				{
					if (!hvdigit)
						fail(0, "Invalid syntax in version number in text "
								"file for install set %s", dir->tgtname);
					pos++;
					hvdigit = FALSE;
				}
				else if (isdigit(chr))
					hvdigit = TRUE;
				else
					fail(0, "Invalid character in version number in text file "
							"for install set %s", dir->tgtname);
				*npnt++ = chr;
			}
			if (!hvdigit || (pos != 1 && pos != 2))
				fail(0, "Invalid syntax in version number in text file for "
						"install set %s", dir->tgtname);
		}
		itemhead.filelen = npnt - inpbufr;
	}
	else
	{
		itemhead.filelen = 0;
		itemhead.cdt = cdt;
	}
	itemhead.sync = 0xC183;
	itemhead.seqnum = (char)seqnum++;
	itemhead.flag = flags;
	itemhead.namelen = (ushort)strlen(tgtname);
	memcpy(itemhead.name, tgtname, itemhead.namelen);

	if ((rtn = putinxid((char *)&itemhead, itemhead.namelen +
			offsetof(ITEMHEADX, name))) < 0)
		fail(rtn, "Error storing header for %s item %s in the XID file",
				itemtbl[itype], tgtname);

///	printf("### cdt = %08X %08X %s\n", (long)(itemhead.cdt >> 32),
///			(long)itemhead.cdt, tgtname);


	if (zippath[0] != 0)				// Are we createing "ZIP" directories?
	{
		if (itype >= IT_OSISET && itype <= IT_RTDIR)
		{
			rtn = ((pnt = strnchr(itemhead.name, '|', itemhead.namelen)) !=
					NULL) ? (pnt - itemhead.name) : itemhead.namelen;
			zipsbdir = strnmov(zipdir, itemhead.name, rtn);
			*zipsbdir++ = '\\';
			*zipsbdir = 0;
			zipfile = zipsbdir;

///			printf("### zipdir = |%s|\n", zippath);

			if ((rtn = svcIoDevParm(XO_CREATE|XO_ODFS, zippath, NULL)) < 0
					&& rtn != ER_DIREX)
				fail(rtn, "Error createing \"ZIP\" directory %s", zippath);
		}
		else if (itype == IT_SBDIR)
		{
			zipfile = strnmov(zipsbdir, itemhead.name, itemhead.namelen);
			*zipfile++ = '\\';
			*zipfile = 0;

///			printf("### zipsubdir = |%s|\n", zippath);

			if ((rtn = svcIoDevParm(XO_CREATE|XO_ODFS, zippath, NULL)) < 0
					&& rtn != ER_DIREX)
				fail(rtn, "Error createing \"ZIP\" directory %s", zippath);
		}
		else if (itype == IT_FILE)
		{
			strnmov(zipfile, itemhead.name, itemhead.namelen);
			zipfile[itemhead.namelen] = 0;

///			printf("### zipfile = |%s|\n", zippath);

			if ((ziphndl = svcIoOpen(XO_CREATE|XO_TRUNCA|XO_IN|XO_OUT, zippath,
						NULL)) < 0)
				fail(ziphndl, "Error createing \"ZIP\" file %s", zippath);
		}
	}
	if (itype >= IT_FILE || itype == IT_BSTRP)
	{
		while (itemhead.filelen > 0)
		{
			if ((amnt = itemhead.filelen) > INBUFRSZ)
				amnt = INBUFRSZ;
			if ((rtn = svcIoInBlock(hndl, inpbufr, amnt)) != amnt)
			{
				if (rtn >= 0)
					rtn = ER_ICMIO;
				fail(rtn, "Error reading %s item %s", itemtbl[itype], srcname);
			}
			if ((rtn = putinxid(inpbufr, amnt)) < 0)
				fail(rtn, "Error storing data for %s item %s in the XID file",
						itemtbl[itype], srcname);
			if (zippath[0] != 0 && itype == IT_FILE &&
					(rtn = svcIoOutBlock(ziphndl, inpbufr, amnt)) < 0)
				fail(rtn, "Error writing \"ZIP\" data file %s", zippath);
			itemhead.filelen -= amnt;
		}
		svcIoClose(hndl, 0);
		if (zippath[0] != 0 && itype == IT_FILE &&
				(rtn = svcIoClose(ziphndl, 0)) < 0)
			fail(rtn, "Error closing \"ZIP\" data file %s", zippath);
	}
	else if (itype < IT_RTDIR)
	{
		if ((rtn = putinxid(inpbufr, itemhead.filelen)) < 0)
			fail(rtn, "Error storing data for %s item %s in the XID file",
					itemtbl[itype], srcname);
	}
}
