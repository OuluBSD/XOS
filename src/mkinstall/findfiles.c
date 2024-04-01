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


int chkexcluded(char *name, FILB *xlist);

static void findfiles2(char *base, FILB **filehead, FILB *xcld1, FILB *xcld2,
		llong *cdt);


//*******************************************************************
// Function: findfiles - Find all of the files to put in the XID file
// Returned: Nothing (does not return if error)
//*******************************************************************

// This function expands the data base created by readins by searching the
//   disk where a wildcard was specified and adjusting the data base
//   accordingly.

void findfiles(void)
{
	DIRB *dir;
	SUBB *sub;
	FILB *gblx;
	char *bpnt;
	char *epnt;
	char  bname[600];

	dir = dirhead;
	while (dir != NULL)
	{
		gblx = dir->gblxhead;
		bpnt = strmov(bname, dir->srcname);
		*bpnt++ = '\\';
		*bpnt = 0;
		findfiles2(bname, &dir->filehead, gblx, dir->xcldhead, &dir->cdt);

///		char xxtxt[100];
///		sdt2str(xxtxt, "ZZZ %Z %h:%m:%s %D-%3n-%l", (time_sz *)&dir->cdt);
///		printf("### storing: %08X %08X %s %s\n", 
///				(long)(dir->cdt >> 32), (long)dir->cdt, xxtxt, dir->tgtname);

		sub = dir->subhead;
		while (sub != NULL)
		{
			epnt = strmov(bpnt, sub->srcname);
			*epnt++ = '\\';
			*epnt = 0;
			findfiles2(bname, &sub->filehead, gblx, sub->xcldhead, &sub->cdt);
			sub = sub->next;
		}
		dir = dir->next;
	}
}


static struct
{	BYTE4PARM  filoptn;
	LNGSTRPARM filspec;
	BYTE8PARM  cdt;
	char       end;
} dsparms =
{	{PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_FILE},
	{PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, 300, 300},
	{PAR_GET|REP_HEXV, 8, IOPAR_MDATE}
};

static struct
{	BYTE8PARM cdt;
	char      end;
} odparms =
{	{PAR_GET|REP_HEXV, 8, IOPAR_MDATE}
};

static void findfiles2(
	char  *base,
	FILB **filehead,
	FILB  *xcld1,
	FILB  *xcld2,
	llong *pcdt)
{
	FILB  *file;
	FILB **pnt2file;
	FILB  *listhead;
	FILB  *list;
	FILB **pnt2list;
	FILB  *fpnt;
	char  *pnt;
	char  *npnt;
	long   dirhndl;
	long   rtn;
	int    len;
	char   chr;
	char   buffer[300];

///	printf("### in findfiles2: base: |%s|\n", base);

	// Open the directory - we need to do this even if we don't need to
	//   search it to get its creation date/time.

	if ((dirhndl = svcIoOpen(XO_ODFS, base, &odparms)) < 0)
		errormsg(dirhndl, "!? Error opening directory %s for search", base);
	*pcdt = odparms.cdt.value;

	// Now go though the list of file items. Search the directory for each one
	//   which contains any wildcards and replace the item with everything
	//   that matched that we don't already have. Note that we keep a separate
	//   list of the files we find so we don't search it looking for duplicates!
	//   This list replaces the wildcard item when we have finished searching.

	dsparms.filspec.buffer = buffer;	
	pnt2file = filehead;
	while ((file = *pnt2file) != NULL)
	{
		pnt = file->srcname;
		while ((chr = *pnt++) != 0 && chr != '*' && chr != '?')
			;
		if (chr != 0)
		{
///			printf("### searching for |%s%s|\n", base, file->srcname);

			if ((rtn = svcIoSetPos(dirhndl, 0, 0)) < 0)
				errormsg(rtn, "!? Error resetting directory position when "
						"searching %s", inpbufr);
			listhead = NULL;
			pnt2list = &listhead;
			while (TRUE)
			{
///				printf("### searching for |%s|\n", file->srcname);

				if ((rtn = sysIoNextFile(dirhndl, file->srcname,
						&dsparms)) < 0)
				{
					if (rtn != ER_FILNF)
						errormsg(rtn, "!? Error searching directory %s",
								inpbufr);
					break;
				}

///				char xxtxt[100];
///				sdt2str(xxtxt, "%Z %h:%m:%s %D-%3n-%l", (time_sz *)&dsparms.cdt.value);
///				printf("### found file: %s 0x%08X %08X %s\n", xxtxt,
///						(long)(dsparms.cdt.value >> 32), (long)dsparms.cdt.value,
///						xidbufr);

				// Here with a file. See if we already know about this one.
				//   Check for an excluded file first since we expect this
				//   list to be short.

///				printf("### found file: |%s|\n", dsparms.filspec.buffer);

				if (chkexcluded(dsparms.filspec.buffer, xcld1) ||
						chkexcluded(dsparms.filspec.buffer, xcld2))
					continue;

				fpnt = *filehead;
				while (fpnt != NULL)
				{
					pnt = npnt = fpnt->srcname;
					while ((chr = *pnt++) != 0)
						if (chr == '\\' || chr == ':')
							npnt = pnt;
					if (stricmp(npnt, dsparms.filspec.buffer) == 0)
						break;
					fpnt = fpnt->next;
				}
				if (fpnt != NULL)
					continue;

				// Here with a file to add to our list

				len = strlen(dsparms.filspec.buffer);
				list = allocmem(offsetof(DIRB, srcname) + len + 1);
				list->flag = file->flag | IT_FILE;
				list->linenum = file->linenum;
				memcpy(list->srcname, dsparms.filspec.buffer, len + 1);
				list->tgtname = list->srcname;
				list->next = NULL;
				*pnt2list = list;
				pnt2list = &list->next;
			}

			// Here with the directory search for this item complete

			if (listhead == NULL)
			{
				*pnt2file = file->next;
				continue;
			}
			else
			{
				list->next = file->next;
				*pnt2file = listhead;
				pnt2file = &list->next;
			}
		}
		else							// If not a wildcard specification
			pnt2file = &file->next;
	}
	svcIoClose(dirhndl, 0);
}


int chkexcluded(
	char *name,
	FILB *xlist)
{
	while (xlist != NULL)
	{
		if (sysLibWildCmp8(0, xlist->srcname, name) == 0)
			return (TRUE);
		xlist = xlist->next;
	}
	return (FALSE);
}
