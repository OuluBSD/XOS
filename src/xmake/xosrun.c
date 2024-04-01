//********************************************************************
// XOSRUN.C - C function to run a program as a child process under XOS
//********************************************************************

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

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERRMSG.H>
#include "XMAKE.H"

static char *target_name = NULL; // Name of target currently being built

static char *exttbl[] = {".RUN", ".BAT", NULL};

typedef struct
{   ushort src;
    ushort dst;
    long   cmd;
} DEVITEM;

typedef struct
{   DEVITEM dlstdin;
    DEVITEM dlstdout;
    DEVITEM dlstderr;
    DEVITEM dlstdtrm;
} DEVLIST;

DEVLIST devlist =				// Device list for run
{   {STDIN , STDIN , 0},
    {STDOUT, STDOUT, 0},
    {STDERR, STDERR, 0},
    {STDTRM, STDTRM, 0}
};

_Packed struct runparm 
{	LNGSTRPARM arglist;
	LNGSTRPARM devlist;
    char       end;
} runparm =
{
    {(PAR_SET|REP_STR), 0, IOPAR_RUNCMDTAIL, NULL, 1, 0},
    {(PAR_SET|REP_STR), 0, IOPAR_RUNDEVLIST, (char *)&devlist,
            sizeof(devlist)}
};

QAB runqab =
{   RFNC_WAIT|RFNC_RUN,			// func    - Function
    0,							// status  - Returned status
    0,							// error   - Error code
    0,							// amount  - Process ID
    0,							// handle  - Device handle
    0,							// vector  - Vector for interrupt
    {0},
    XR_CHILDTERM|XR_ACSENV|XR_CHGENV,
								// option  - Options or command
    0,							// count   - Count
    NULL,						// buffer1 - Pointer to file spec
    NULL,						// buffer2 - Unused
    (char *)&runparm			// parm    - Pointer to parameter area
};

void cntlcsrv(void)
{
    svcSchCtlCDone();
    if (target_name != NULL && OPT_precious == FALSE &&
	    svcIoDelete(0, target_name, NULL) >= 0)
	fprintf(stderr, "\n? XMAKE: Removing %s on Ctrl-C abort\n",
		target_name);
    else
	fputs("\n? XMAKE: Ctrl-C abort\n", stderr);
    exit(1);
}

void runprog(
	char     *filespec,
	char     *command,		// Address of command tail (7 bytes before this
							//   string must be available to modify)
	TREENODE *tree)
{
	ulong errcode;
	long  rtn;				// Error code returned by program

///	printf("### filespec: |%s|\n", filespec);
///	printf("### cmd tail: |%s|\n", command);

	if ((OPT_noexec && !OPT_question) || !(OPT_silent || CMD_silent))
	{
		putchar('\t');
		puts(command);
	}
	if (!OPT_noexec)					// We may not want to exec
	{
		target_name = tree->file->fname;
		devlist.dlstdin.dst  = STDIN;
		devlist.dlstdout.dst = STDOUT;
		devlist.dlstderr.dst = STDERR;
		devlist.dlstdtrm.dst = STDTRM;
		runparm.arglist.buffer = command;
		runparm.arglist.bfrlen = runparm.arglist.strlen = strlen(command);
		runqab.buffer1     = filespec;

		if ((rtn = sysIoRunCmd(&runqab, NULL, "PATH", exttbl)) == 1)
		{
			memcpy(command - 7, "xshell", 7); // Insert command name before the
			runparm.arglist.buffer -= 7;	  //   string we were given
			runqab.buffer1 = "XOSCMD:xshell.run";
			rtn = sysIoRun(&runqab);
		}
		if (rtn < 0)
		{
			if (!OPT_precious)			// If we don't want to keep this
			{
				if (svcIoDelete(0, tree->file->fname, NULL) >= 0)
					fprintf(stderr, "XMAKE: Removing %s", tree->file->fname);
			}
			femsg("XMAKE", rtn, command);
		}
		errcode = runqab.amount & 0x0FFFFFF;
		if (runqab.amount & 0x00800000)
			errcode |= 0xFF000000;
		if (errcode != 0)
		{
			if (!OPT_ignore && !CMD_ignore)
			{
				fprintf(stderr, "? XMAKE: %s terminated with error status"
						" %ld\n", command, errcode);
				if (!OPT_precious)	// If we don't want to keep this
				{
					if (svcIoDelete(0, tree->file->fname, NULL) >= 0)
						fprintf(stderr, "XMAKE: Removing %s\n",
								tree->file->fname);
				}
				exit(1);
			}
		}
		target_name = NULL;			// Finished processing this target
    }
}


int cmdcopy(
	char     *dest,			// Pointer to destination for string
	uchar    *source,		// Pointer to source for string
	TREENODE *depend,		// Pointer to current dependency
	int       srcsize,		// Pointer to number of chars in source
	int       destsize)		// Pointer to max number of chars in dest
{
    char mfnc;					// Function for special macros
    char sep;					// Filename separator for special macros
    char c;						// Temporary character
    int  i;						// String counter
    int  dlx;					// Temporary place to keep destsize for loop
    int  dev, path, name, ext;	// Flags for various parts of filename
    int  once; 					// Flag for only first name of list of names
    int  all;					// Flag for all or only out of date filenames
    int  looped;				// Counter for single time around macro loop
    TREENODE *namelist;			// Temporary pointer into dependency list
	char z;
    char *namepnt;				// Temporary pointer into filename
    char *x;					// Very Temporary pointer into filename

    while (*source && *source != CR)
    {
		if (srcsize == 0)
			break;						// If there's nothing left to copy
		if (destsize == 0)
			return (EOF);    			// Return an overflow error

		if (*source < 0x80)				// If not the beginning of a macro
		{
			if (srcsize != -1)			// Check if we're tracking this size
				--srcsize;
			*dest++ = *source++;		// Copy a character
			--destsize;					// Reduce size of available destination
			continue;					// Go around and process next character
		}

		// Here with special macro expansion

		mfnc = *source++ & 0x7f;		// Get macro function
		once = dev = path = name = ext = FALSE;
		while (*source < 0x80)
		{
			if (srcsize != -1)
				--srcsize;
			switch (toupper(*source++))
			{
			 case '1':
				once = TRUE;
				break;

			 case 'D':
				dev  = TRUE;
				break;

			 case 'P':
				path = TRUE;
				break;

			 case 'N':
				name = TRUE;
				break;

			 case 'E':
				ext  = TRUE;
				break;

			 default:
				--source;
				if (*source < ' ')		// If non-printing character
				{
					z = '^';
					*source = *source + '@';
				}
				else
					z = ' ';
				fprintf(stderr, "? XMAKE: Illegal modifier '%c%c' for "
						"internal macro $%c\n", z, *source, mfnc);
				exit(1);
			}
		}
		sep = *source++ & 0x7f;			// Get filename separator char
		if (srcsize != -1)
			--srcsize;
		if (dev == FALSE && path == FALSE && name == FALSE && ext == FALSE)
			dev = path = name = ext = TRUE; // If none specified, copy all
		switch (mfnc)					// Selection actions for each macro
		{
		 case '*':
			namelist = depend->tree_list;
			all = TRUE;
			break;

		 case '@' :
			namelist = depend;
			all = TRUE;
			once = TRUE;
			break;

		 case '?' :
			namelist = depend->tree_list;
			all = FALSE;
			break;
		}
		looped = 0;						// Counter to once around feature
		while (namelist != NULL &&
				(once == FALSE || looped == 0) && destsize > 0)
		{								// Until out of names to process
			dlx = destsize;				// Store this for later
			if (all == FALSE && depend->file->fdatetime >
					namelist->file->fdatetime)
			{
				namelist = namelist->next; // Advance to next node
				continue;				// And try again
			}
			namepnt = namelist->file->fname; // Get ptr to name
			if (inchr(namepnt, ':') && dev == TRUE) // If device name
			{
				while ((c = *namepnt++) != ':' && destsize > 0 && c != 0)
				{
					*dest++ = c;		// Copy device name
					--destsize;
				}
				*dest++ = c;			// Copy trailing colon
				--destsize;
			}
			else if (dev != TRUE && inchr(namepnt, ':') && *namepnt != 0)
				while (*namepnt++ != ':') // Otherwise ignore
					;
			if ((inchr(namepnt, '/') || inchr(namepnt, '\\')) &&
					*namepnt != 0 && path == TRUE)
			{
				x = namepnt;
				i = 0;
				while (*x != 0)			// Advance to end of string
				{
					++x;
					++i;
				}
				while (i && *x != '\\' && *x != '/')
				{
					--i;
					--x;				// Find first slash
				}
				if (i && destsize >= i)
				{						// Copy pathname to dest
					++i;
					strncpy(dest, namepnt, i);
					*(dest + i) = 0; 	// Must manually terminate
					dest += i;
					destsize -= i;
					namepnt = x + 1;
				}
				else if (x != namepnt)
					return (EOF);		// Indicate string overflow
			}
			else if (path == FALSE && *namepnt != 0 && (inchr(namepnt, '/') ||
					inchr(namepnt, '\\')))
			{
				while (*namepnt++ != 0) // Otherwise ignore pathname
					;
				while (*namepnt != '\\' && *namepnt != '/')
					--namepnt;
				++namepnt;
			}
			if (name == TRUE && *namepnt != 0) // If filename body wanted
			{
				while ((c = *namepnt++) != '.' && c != 0 && destsize > 0)
				{
					*dest++ = c;
					--destsize;
				}
				--namepnt;				// Leave it at '.'
			}
			else if (name == FALSE && *namepnt != 0) // Ignore filename body
			{
				while ((c = *namepnt++) != '.' && c != 0)
					;
				--namepnt;
			}
			if (ext == TRUE && *namepnt != 0) // If filename ext wanted
			while (*namepnt != 0 && destsize > 0)
			{
				*dest++ = *namepnt++;
				--destsize;
			}
			if (destsize > 0 && destsize != dlx) // If not out of space and we
			{									 //   generated something
				*dest++ = sep;			// Store the separator char at the end
				--destsize;
			}
			namelist = namelist->next;	// Advance to next name
			looped   = 1;				// Indicate around loop
		}
		if (destsize != dlx)			// If we generated any filenames
		{
			--dest;						// Ignore last separator
			++destsize;
		}
    }
    *dest = 0;
    return (0);
}
