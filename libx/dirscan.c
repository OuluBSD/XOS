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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <xosstr.h>
#include <xoserr.h>
#include <xos.h>
#include <xossvc.h>
#include <dirscan.h>
#include <heapsort.h>

static DIRPARMS dirinit =
{	// Values for opn

	{	{PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_XOSDEV|XFO_DOSDEV|XFO_VOLUME|
				XFO_NODENAME|XFO_RXOSDEV|XFO_RDOSDEV|XFO_RVOLUME|XFO_PATH|
				XFO_FILE},
		{PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, FILENAME_MAX, FILENAME_MAX},
		{PAR_SET|REP_HEXV, 2, IOPAR_SRCATTR, XA_DIRECT},
		{PAR_GET|REP_HEXV, 2, IOPAR_FILEATTR},
		{PAR_GET|REP_HEXV, 4, IOPAR_DEVSTS}
	},

	// Values for pos

	{	{PAR_SET|REP_HEXV, 4, IOPAR_ABSPOS},
		{PAR_GET|REP_HEXV, 4, IOPAR_DEVSTS},
	},

	// values for sch

	{	{PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_FILE},
		{PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, FILENAME_MAX, FILENAME_MAX},
		{PAR_SET|REP_HEXV, 2, IOPAR_SRCATTR, XA_DIRECT}
	}
};


///static const char noelipmsg[] = "Device does not support ... searches";

static int   _dirscan(char *filespec, DIRSCANDATA *dsd, int recur);
static int   compdir(DIRNAME *a, DIRNAME *b, DIRSCANDATA *dsd);
static int   doelip(char *filename, char *elip, long hndl, DIRSCANDATA *dsd);
static void *dsgetmem(size_t size, DIRSCANDATA *dsd);
static int   level;

//**************************************************
// Function: dirscan - Scans directory and processes
//		all files in directory
// Returned: TRUE if normal, FALSE if error
//**************************************************

int dirscan(
    char        *filespec,
    DIRSCANDATA *dsd,
	int          opts)
{
    int rtn;

    // First initialize our main data structure, allocate buffer, etc.

	dsd->pathname[0] = 0;
    dsd->pathnamelen = 0;
	dsd->filename[0] = 0;
    dsd->filenamelen = 0;
///	dsd->newnamespec = NULL;

    // Initialize the parameter lists we need

	dsd->parmlist->filspec.buffer = dsd->filename;
    dsd->parmlist->filspec.bfrlen = FILEPART_MAX;
	*(DIRPARMS *)(&dsd->opn) = dirinit;
	dsd->opn.filoptn.value |= (dsd->parmlist->filoptn.value & XFO_MARKDIR);
    dsd->opn.filspec.buffer = dsd->pathname;
    dsd->sch.filspec.buffer = dsd->parmlist->filspec.buffer;
	dsd->sch.srcattr.value |= (dsd->parmlist->srcattr.value & (XA_HIDDEN|
			XA_SYSTEM));
	dsd->needwcc = ((opts & (DSO_WILDNAME|DSO_WILDEXT)) != 0);

    // Do the work

    rtn = _dirscan(filespec, dsd, opts);
    return (rtn);
}


//***************************************************
// Function: _dirscan - Internal function for dirscan
// Returned: TRUE if normal, FALSE if error
//***************************************************

static int _dirscan(
	char        *filespec,
    DIRSCANDATA *dsd,
    int          opts)
{
	DIRNAME *next;
	DIRNAME *thisdir;
	DIRNAME *lastdir;
	DIRNAME *firstdir;
	char    *pathend;
	char    *filepnt;
	char    *pnt;
    long     rtn;
	long     dirhndl;
    int      size;
	char     chr;
	char     bufr[600];

    dsd->parmlist->filspec.buffer[0] = '\0'; // In case we don't get a string
 
///	printf("### Opening dir: |%s|\n", filespec);

	if ((dirhndl = svcIoOpen(XO_ODFS, filespec, &dsd->opn)) < 0)
	{									// Open the directory
///		printf("### open returned %d\n", dirhndl);

		dsd->errormsg(filespec, dirhndl); // If error, display message
		return (dirhndl);
	}

///	printf("### Dir open: |%s|\n", dsd->opn.filspec.buffer);

	dsd->pathnamelen = dsd->opn.filspec.strlen;
	dsd->filename[0] = 0;
	dsd->filenamelen = -1;
	dsd->attr = dsd->opn.filattr.value;
	dsd->dirlvl = level;
	dsd->error = 0;
	if (!dsd->func(dsd))				// Call the caller's function
		return (FALSE);
	if (dsd->needwcc)					// Need to check for wild cards?
	{
		pnt = filepnt = filespec;		// Yes - find the start of the name
		while ((chr = *pnt++) != 0)		//   part
		{
			if (chr == ':' || chr == '\\' || chr == '/')
				filepnt = pnt;
		}
		if (filepnt[0] == 0)			// Null file name?
		{
			*filepnt++ = '*';
			*filepnt = 0;
		}
		if (dsd->opn.devsts.value & DS_NAMEEXT) // Does the FS use name and ext?
		{
			while ((chr = *filepnt++) != 0 && chr != '.') // Find start of ext
				;
			if (chr == 0)				// Did we see a period?
			{
				filepnt[-1] = '.';		// No - add one
				filepnt[0] = 0;
				if (opts & DSO_WILDEXT && *filepnt == 0) // Need wild ext?
				{
					*filepnt++ = '*';	// Yes
					*filepnt = 0;
				}
			}
		}
	}
	dsd->parmlist->filspec.buffer = dsd->filename;
	while (TRUE)
	{
		// Process the files in this directory

///		printf("### Looking for: |%s|\n", filespec);

		if ((rtn = sysIoNextFile(dirhndl, filespec, dsd->parmlist)) < 0)
		{
///			printf("### nextfile returned %d\n", rtn);

			if (rtn == ER_FILNF)		// Normal end of scan?
				break;					// Yes

			dsd->error = rtn;

			if (dsd->parmlist->filspec.strlen == 0)
			{
				strmov(dsd->parmlist->filspec.buffer, "<""???"">");
				dsd->parmlist->filspec.strlen = 5;
			}

///			dsd->errormsg(filespec, rtn);
///			return (FALSE);
		}
		else
			dsd->error = 0;

///		printf("### found: %3d |%s|\n", dsd->parmlist->filspec.strlen,
///				dsd->parmlist->filspec.buffer);

		dsd->filenamelen = dsd->parmlist->filspec.strlen;
		dsd->attr = dsd->parmlist->filattr.value;
		dsd->dirlvl = level;
		if (!dsd->func(dsd))			// Call the caller's function
			return (FALSE);
	}

///	printf("### out of file loop!\n");

    if (opts & DSO_RECUR)
	{
		// Set to start of directory

		if ((rtn = sysIoInBlockP(dirhndl, NULL, 0, &dsd->pos)) < 0)
	    {
    	    dsd->errormsg("Error resetting position in directory", rtn);
        	return (FALSE);
    	}

		// Scan the directory again and collect a list of the directories it
		//   contains.

		lastdir = (struct dirname *)&firstdir;
		firstdir = NULL;
		do
		{
			dsd->parmlist->filspec.buffer[0] = '\0';
			if ((rtn = sysIoNextFile(dirhndl, "*.*", &dsd->sch)) == ER_FILNF)
				break;

			if ((size = dsd->sch.filspec.strlen) == 0)
			{
				rtn = ER_DRFER;
				strmov(dsd->sch.filspec.buffer, "<""???"">\\");
				size = 6;
			}
			if (dsd->sch.filspec.buffer[size - 1] != '\\')
			{
				dsd->sch.filspec.buffer[size] = '\\';
				dsd->sch.filspec.buffer[size + 1] = 0;
				size++;
			}
			if ((thisdir = (struct dirname *)dsgetmem(offsetof(DIRNAME, name) +
					1 + size, dsd)) == NULL)
				return (FALSE);
			thisdir->error = rtn;
			memcpy(thisdir->name, dsd->sch.filspec.buffer, size);
			thisdir->name[size] = 0;
			lastdir->next = thisdir;
			lastdir = thisdir;
			thisdir->next = NULL;
		} while (rtn >= 0);

		svcIoClose(dirhndl, 0);

///		printf("### Directory list is complete\n");

	    // Now we have constructed a list of directories to search

		if (firstdir != NULL)			// Did we find any directries?
		{								// Yes
			if (dsd->sort != DSSORT_NONE) // Should we sort the list?
				firstdir = heapsort(firstdir, // Yes
						(int (*)(void *a, void *b, void *dsd))compdir, dsd);

			// Now we have a sorted list of all subdirectories.

			pnt = filepnt = filespec;
			while ((chr = *pnt++) != 0)
			{
				if (chr == '\\' || chr == '/' || chr == ':')
					filepnt = pnt;
			}
			pathend = strmov(bufr, dsd->pathname);
			do							// Loop for each directory
			{
				next = firstdir->next;
				free(firstdir);
				if (++level > 40)
				{
					dsd->errormsg(dsd->pathname, ER_DIRTD);
					return(FALSE);
				}

///				*pathend = 0;
///				printf("### bufr: |%s| dir |%s| file |%s|\n", bufr,
///						firstdir->name, filepnt);

				if (firstdir->error >= 0)
				{
///qqqqqqqqqqqqqqqq
					sprintf(pathend, "%s%s", firstdir->name, filepnt);

///					printf("### calling _dirscan %d |%s|\n", level, bufr);

					if (!_dirscan(bufr, dsd, TRUE))
					{

///						printf("### backup from _dirscan - FALSE\n");

						level--;
						return (FALSE);
					}

///					printf("### back from _dirscan - TRUE\n");
				}
				else
				{
///					printf("### bufr: |%s| name: |%s|\n", bufr, firstdir->name);


					strmov(pathend, firstdir->name);

					dsd->pathnamelen = strmov(dsd->pathname, bufr) -
							dsd->pathname;

					dsd->filename[0] = 0;
					dsd->filenamelen = -1;
					dsd->attr = dsd->opn.filattr.value;
					dsd->dirlvl = level;
					dsd->error = firstdir->error;
					if (!dsd->func(dsd)) // Call the caller's function
						return (FALSE);
				}
				level--;
			} while ((firstdir = next) != NULL);
		}
	}
	else
	    svcIoClose(dirhndl, 0);
    return (TRUE);
}


//**************************************************
// Function: compdir - called by heapsort to compare
//		two directory names
// Returned: Negative if a < b
//	     Zero if a == b
//           Positive if a > b
//**************************************************

static int compdir(
    DIRNAME     *a,
    DIRNAME     *b,
    DIRSCANDATA *dsd)
{
    return ((dsd->sort == DSSORT_ASCEN)?
            stricmp(a->name, b->name): stricmp(b->name, a->name));
}


//****************************************************
// Function: dsgetmem - Get memory with malloc, return
//		with error if out of memory
// Returned: Pointer to memory obtained
//****************************************************

static void *dsgetmem(
    size_t       size,
    DIRSCANDATA *dsd)

{
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL)
    {
        dsd->errormsg("Not enough memory available", 0);
        return (NULL);
    }

//  if (maxmem < _malloc_amount)
//      maxmem = _malloc_amount;

    return (ptr);
}
