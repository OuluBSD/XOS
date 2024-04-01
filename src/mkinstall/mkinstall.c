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
#include "progarg.h"

// This program generates an XOS Install Data (XID) file. The contents for the
//   is taken from the installed versions of XOS and applications on the system
//   that mkinstall is run on. This file is intended for use as input to the
//   Windows program mkxosbtdsk which creates a bootable USB disk from this
//   data.

// The XID file starts with an 8 byte header which has the following format:
//   Offset Size Description
//     0      4  File label = "XIDF"
//     4      1  File format version = 1
//     5      3  Reserved = 0
// This is immediately followed by a zlib compressed data stream which consists
//   of a sequence of directory/file items, tightly packed. Each item has the
//   following format:
//   Offset Size Description
//     0      2  Sync valie = 0xC55C
//     4      2  Name length - bit 15 set if data length value follows
//     6      4  Data length
//   6 or 10  n  Item name string (not 0 terminated)
//   This is immediately followed by the item data, if present. The first
//   character of the name field is not part of the name but contains bits
//   which specify item attributes:
//     Bit  7   - Set if item should be copied to the boot disk
//     Bit  6   - Set if item should be added to an install set
//     Bit  5   - Reserved, must be 0
//     Bits 4-0 - Item type
//                  0 = Bootstrap file (must be first item)
//                  1 = Start of OS install set
//                  2 = Start of additinal OS directory
//                  3 = Start of Application install set
//                  4 = Start of application data
//                  5 = Start of data install set
//                 16 = Sub-directory
//                 17 = File

// The format of the name string for each item is:

// Item 0 = Bootstrap file
//   The fully specified source file specification for the bootstrap file, |,
//   default program to load, |, timeout value (ASCII seconds - 0 means no
//   timeout, -1 means to load default immediately. Data part contains the
//   bootstrap code.

// Item 1 = Start of OS install set
//   The name of the source directory, |, the name of the target directory, |,
//   the name of the main OS executable without an extension. The target
//   directory should be a single level. Data part contains the code for the
//   main OS executable.

// Item 2 = Start of additional OS directory
//   The name of the source directory, |, the name of the target directory.
//   There is no data part.

// Item 3 = Start of Application install set
//   The name of the source directory, |, the name of the target directory, |,
//   the name of the main application executable without an extension. The
//   target directory should consist of two levels, the base application
//   directory and the vendor directory name. Data part contains the code
//   for the main application executable.

// Item 4 = Start of application data
//   The name of the source directory, |, the name of the target directory.
//   The target directory should consist of three levels, the base application
//   data directory, the vendor directory name, and the application name.
//   There is no data part.

// Item 5 = Start of data install set
//   The name of the source directory, |, the name of the target directory.
//   The target directory should generally be a single level. There is no
//   data part.

// Item 16 = Sub-directory
//   The name of the sub-directory. This must specify ALL levels under the
//   directory specified by item 1, 2, 3, 4, or 5. There is no data part.

// Item 17 = File
//   The name of a file to be included in the current sub-directory. Data
//   part contains the file data.

// The first item in the XID file must be a type 0 item which contains the
//   bootstrap file which is used to initialize the bootstrap on the install
//   disk. This must be the first item in the XID file and there can only be
//   one type 0 item.

// This program is controlled by an input file (INS file) which consists of a
//   number of lines that somewhat mirror the file structure specified above.
//   Each line begins with a two-letter prefix followed by a file name or
//   directory specification mostly as described above.

// NOTE: The format of the INS file is somewhat complex and there is not a
//       lot of error checking for things being out of order. Carefully read
//       and follow the comments below when creating an INS file. Its fairly
//       easy to create an invalid XID file.

// The first letter specifies the item type. The following items are top level
//   items which specify the start of an install set, an additional data
//   directory, or a bootstrap file.

//   B - Bootstrap file - This specifies a bootstrap file to be installed on
//       boot disk. It must be present exactly one time. It may be anywhere
//       in the INS file but must occur exactly once. (This is not really a
//       specification for an install set but it is a top level item so it is
//       included here.)
//         "B  filespec|default|timeout"

//   O - Specifies the start of the definition of an OS install set. This
//       consists of all files under the specified directory plus all files
//       specified under a following U item.
//         "Ox srcdir|trgdir|osrunfile" This will normally be
//         "Ox xossys|xossys|xos"
//       This may be followed by sub-directory and file items as needed.
//       The version number and date of an OS is taken form the OS's main
//       executable file. Multiple OS install sets can be present, although
//       only one OS can be specified to be installed on the boot disk.

//   A - Specifies the start of the definition of an application install set
//       and specifies the application executable directory and the name of
//       the application.
//         "Ex srcdir|trgdir|mainfile" This will normally be
//         "Ex xosapp\vendor|xosapp\vendor|appname"
//       The final target directory is used as the vendor's name and the name
//       of the main executable file is take as the application's name.
//       Additional executable files may be specified using F items but no S
//       items may be used. The version number and date of an application is
//       taken from the application's main executable file. Multiple versions
//       of the same application can be present but only one may be installed
//       on the boot disk. In these cases the source directories must be
//       different but the target directory can (should) be the same.

//   D - Specifies a data install set. This consists of only all the files
//       under the specified directory.
//         "Dx srcdir|trgdir|name" For example "Dx xossrc|xossrc|xossrc"
//       This may be followed by sub-directory and file items as needed.
//       The directory specified must contain a file "name.txt". The name of
//       this file is the name of the install set. It will usually be the same
//       as the target directory but does not have to be. The date of the
//       install set is the creation date of this file. The first line of the
//       file specifies the version of the install set as "major.minor.edit"
//       or "major.minor". For example "4.3.1" or "4.3". This must be the first
//       thing on the line and be followed either by the end of the line or by
//       whitespace.

//   R - Specifies the start of the definition of the an additional root level
//       directory associated with an install set. It must immediately follow
//       the complete install set specificaton. This may be followed by S and
//       F items as needed.
//         "Rx srcdir|trgdir"

// The following items are used to specify the sub-directories and files to
//   be included in an install set.

//   S - Sub-directory

//   F - Data file (wildcards are allowed) - Note that it is realitivly easy
//       to create overlapping specifications with wildcard names. This does
//       no harm other than introducing significant overhead, when creating
//       the XID file, especiall for large directories. It is valid to specify
//       "Fb *.*" and also specify "Fi name1|namex" and "Fc name2|namex". This
//       copy all files to both the install set and the boot disk except for
//       name1 and name2. name1 will be copied only to the install set and
//       name2 will be copied only to the boot disk. The fullly specified
//       names will always override any wildcard matches. If there are
//       overlapping wildcards with different destinations, the wildcard
//       specified first will override any matches from following wildcards.
//       Generally this kind of complexity should never be really useful and
//       should be avoided in any case.

//   X - Excluded file (simple wildcards are allowed) Any files specified in
//       X items are excluded if found by a wildcard search of if specifed
//       directly.

//   G - Globally excluded file (simple wildcards are allowed) same as an X
//       item except that it applies to all sub-directories in a directory.
//       It may occur anywhere in the directory specification and applies to
//       the entire directory specification.

// The second letter specifies how the item is handled when copied to a boot
//   disk as follows:
//   c - Copy item to the boot disk
//   i - Create an install set for the item or add the item to an install set
//   b - Do both of the above
//     - (blank) Does not apply to this line (D and B lines)

// All directory and sub-directory target names must be specified without a
//   device name and without a leading or a trailing backslash. All target
//   file names must be specified without a device or directory name.

// Blank lines and lines beginning with # are ignored. Lines beginning with
//   # are intended to be used as comments.

// Example INS file:
//
//   # This specifies the bootstrap file.
//
//   Bb xossys:xosboot.run|\xossys\xos|10
//
//   # This specifies the OS for the boot disk and creates an install set
//   # for it. Note that each sub-directory must be specified.
//
//   Ob XOSDEV:xossys|xossys|xos
//   Sb sys
//   Fb *
//   Sb cmd
//   Fb *
//   Sb cfg
//   Fb *
//   Fc ustartup.bat
//   Fi ustartupins.bat|ustartup.bat
//   Sb spl
//   Sb act
//   Fb *
//
//   # This adds the minimum user directory tree to the OS
//
//   Rb xosusr
//   Sb default
//   Fb *
//
//   # This installs the "install" application (vendor is "system") that
//   #   consists of only the executable and creates an install set for it.
//
//   Ab XOSDEV:xosapp\system|xosapp\system|install
//
//   # This creates an install set (only) for an application which includes a
//       second executable file and some data files.
//
//   Ai XOSDEV:xosapp\vendor|xosapp\vendor|name
//   Fi file.run
//
//   Ri XOSDEV:xosdata\vendor|xosdata\vendor
//   Fi *
//   Si data
//   Fi*

#define VERSION 1
#define EDITNO  5

// Command line format is:
//   mkinstall ctrlfile xidfile {chkfile}
// The default control file extention is .ins. The default XID file extension
//   is .xid. "chkfile" is the name for an optional uncompressed check file
//   that can be generated and used as input to chkinstall to fully verify
//   that the XID file was correctly generated.

char    *inpbufr;
char    *xidbufr;
z_stream stream;
long     amnt;
long     inhndl;
long     xidhndl;
long     chkhndl;
long     lastspin;
int      spininx;
int      seqnum;
int      zflag = Z_NO_FLUSH;
FILE    *ctlfile;
char    *osspec;

char     bootstrapsrc[256];
char     bootstrapdflt[256];
long     bootstraptime;
char     hviset;

PROGINFO pib;

char    *ctlspec;
char    *xidspec;
char    *chkspec;

char     zippath[512];	// Buffer to hold file name for "ZIP" file
char    *zipdir;		// Pointer to start of directory name in zippath
char    *zipsbdir;		// Pointer to start of subdirectory name in zippath
char    *zipfile;		// Pointer to start of file name in zippath

XIDHEAD  xidhead = {"XIDF", 20, 2, 0, 0, 0, 0, 0xFFFFFFFF};
DIRB    *dirhead;

ITEMHEAD itemhead = {0xC183, 0, 0, 0xFFFF};

long getinput(char *bufr, long size, char *more);

char *setupfile(char *arg, int len, char *ext);
char *fixdefault(char *file);

int  nonopt(char *arg);
int  optxid(ARGDATA *data);
int  optcheck(ARGDATA *data);
int  optdisk(ARGDATA *data);


#define AB XA_FILE|XA_RDONLY|XA_HIDDEN|XA_SYSTEM|XA_DIRECT

ARGSPEC options[] =
{   {"X*ID"  , ASF_LSVAL, NULL,    optxid  , 0, "Specify the XID output file, default is the name of the INS file"},
    {"C*HECK", ASF_LSVAL, NULL,    optcheck, 0, "Specify the check output file, default is the name of the INS file"},
    {"D*ISK" , ASF_LSVAL, NULL,    optdisk , 0, "Specify disk device where directories are copied (no default)"},
    {"H*ELP" , 0        , NULL, AF(opthelp), 0, "Display this screen" },
    {"?"     , 0        , NULL, AF(opthelp), 0, "Display this screen"},
    {NULL}
};

char    copymsg[] = "";
char    prgname[] = "MKINSTALL";	// Our programe name
char    envname[] = "^XOS^MKINSTALL^OPT"; // The environment option name
char    example[] = "{/options} insfile {/options}";
char    description[] = "This command creates an XID (XOS Install Data) file "
		"based on the information in the specified INS file. It can also "
		"produce a check file (which is an uncompressed version of the XID "
		"that can be used by chkinstall to verify that the XID file is "
		"correct). It can also optionally copy all of the directories used to "
		"specified device. This provides an easy way to generate ZIP files "
		"that have the same contents as the XID file.";



void main(
	int   argc,
	char *argv[])
{
	long  rtn;

	if (argc < 2)
	{
		fputs("? MKINSTALL: Command error, type /h for help\n", stderr);
		exit(1);
	}
	reg_pib(&pib);
    pib.opttbl=options; 				// Load the option table
    pib.kwdtbl=NULL;
    pib.build=__DATE__;
    pib.majedt = VERSION; 				// Major edit number
    pib.minedt = EDITNO; 				// Minor edit number
    pib.copymsg=copymsg;
    pib.prgname=prgname;
    pib.desc=description;
    pib.example=example;
    pib.errno=0;

///	getTrmParms();
///	getHelpClr();

	++argv;
	progarg(argv, 0, options, NULL, nonopt, (void (*)(char *, char *))NULL,
			(int (*)(void))NULL, NULL);

	if (ctlspec == NULL)
	{
		fputs("? MKINSTALL: No INS file was specified\n", stderr);
		exit(1);
	}
	if (ctlspec[0] == 0 || ctlspec[0] == '.')
	{
		fputs("? MKINSTALL: Invalid INS file name was specified\n", stderr);
		exit(1);
	}
	xidspec = fixdefault(xidspec);
	chkspec = fixdefault(chkspec);

	// Allocate our buffers

	if ((inpbufr = malloc(INBUFRSZ)) == NULL ||
			(xidbufr = malloc(XIDBUFRSZ)) == NULL)
		fail(-errno, "Error allocating buffer");

	// Open the INS file

	if ((ctlfile = fopen(ctlspec, "r")) == NULL)
		fail(-errno, "Error opening control file %s", ctlspec);

	// Read the INS file and set up our data base

	fputs("Reading the INS file\n", stdout);
	readins();

///	showall();

///	if (bootspec == NULL)
///	{
///		fputs("? MKINSTALL: No bootstrap file specified\n", stderr);
///		exit(1);
///	}

/*
	if (osspec == NULL)
	{
		fputs("? MKINSTALL: No operating system specified\n", stderr);
		exit(1);
	}
*/

	// Do any wild-card searches that are needed

	fputs("Finding files\n", stdout);
	findfiles();

///	showall();

	// Here with a complete list of the files we need to put in the XID
	//  file

	if (chkspec != NULL && (chkhndl = svcIoOpen(XO_OUT|XO_TRUNCA|XO_CREATE,
			chkspec, NULL)) < 0)
		fail(xidhndl, "Error creating check file %s", chkspec);

	// Initialize zlib

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
	if ((rtn = deflateInit(&stream, 6)) != Z_OK)
		fail((ushort)rtn | 0x80000000, "Error initializing zlib");
	stream.next_out = xidbufr;
	stream.avail_out = XIDBUFRSZ;

	fputs("Generating output", stdout);

	// Create the XID file

	svcSysDateTime(T_GTSYSDTTM, &xidhead.cdt);

	if ((xidhndl = svcIoOpen(XO_OUT|XO_TRUNCA|XO_CREATE, xidspec, NULL)) < 0)
		fail(xidhndl, "Error creating output file %s", xidspec);

	// Store the XID file header (this is not compressed). The length and
	//   checksum values will be filled in at the end.

	if ((rtn = svcIoOutBlock(xidhndl, (char *)&xidhead, sizeof(xidhead))) < 0)
		fail(rtn, "Error writing XID header");

	// Store the bootstrap file

	if (bootstrapsrc[0] != 0)
	{
		sprintf(inpbufr, "%s|%d", bootstrapdflt, bootstraptime);
		store1item(bootstrapsrc, inpbufr, IT_BSTRP, 0, NULL);
	}

	// Store everything else

	storeitems();

	// Store the final null item header to indicate EOF and finish up zlib
	//   output

	zflag = Z_FINISH;
	itemhead.seqnum = (char)seqnum;
	itemhead.flag = IT_END;
	itemhead.namelen = 0;
	itemhead.filelen = 0;
	if ((!putinxid((char *)&itemhead, sizeof(ITEMHEAD))) < 0)
		fail(rtn, "Error storing EOF header in the XID file");

	if ((XIDBUFRSZ - stream.avail_out) > 0 &&
			(rtn = svcIoOutBlock(xidhndl, xidbufr, XIDBUFRSZ -
			stream.avail_out)) < 0)
		fail(rtn, "Error finishing output to the XID file");
	lastspin = xidhead.totalsize;
	xidhead.totalitems++;				// Include the final item header in
										//   the count

	// Update the header with the uncompressed length and the CRC and close
	//   the file

	if ((rtn = svcIoSetPos(xidhndl, offsetof(XIDHEAD, totalsize), 0)) < 0 ||
			(rtn = svcIoOutBlock(xidhndl, (char *)&xidhead.totalsize, 12)) < 0)
		fail(rtn, "Error updating XID file header");
	if ((rtn = svcIoClose(xidhndl, 0)) < 0)
		fail(rtn, "Error closing the XID file");

	printf("\nComplete:\n        Total items: %,d\n       Total length: %,d\n"
			"  Compressed length: %,d\n             CRC-32: 0x%08X\n",
			xidhead.totalitems,  xidhead.totalsize, stream.total_out +
			sizeof(XIDHEAD), xidhead.crcvalue);
	exit(0);
}



//********************************************
// Function: nonopt - process non-option input
// Returned: Nothing
//********************************************

int nonopt(
    char *arg)
{
	ctlspec = setupfile(arg, strlen(arg), ".ins");
	return (TRUE);
}


int optxid(
	ARGDATA *data)
{
	if (data->val.s == NULL)
		data->val.s = "";
	xidspec = setupfile(data->val.s, data->length, ".xid");
	return (TRUE);
}


int optcheck(
	ARGDATA *data)
{

	if (data->val.s == NULL)
		data->val.s = "";
	chkspec = setupfile(data->val.s, data->length, ".chk");
	return (TRUE);
}


int optdisk(
	ARGDATA *data)
{
	if (data->length == 0)
	{
		fputs("? MKINSTALL: No disk name specified for /DISK\n", stderr);
		exit(1);
	}
	if (data->length > 16)
	{
		fputs("? MKINSTALL: Disk name is too long\n", stderr);
		exit(1);
	}
	memcpy(zippath, data->val.s, data->length);
	zipdir = zippath + data->length;
	if (zipdir[-1] != ':')
		*zipdir++ = ':';
	*zipdir++ = '\\';
	*zipdir = 0;

///	printf("### zipdev = |%s|\n", zippath);

	return (TRUE);
}



void *allocmem(
	int len)
{
	void *pnt;

	if ((pnt = malloc(len)) == NULL)
		fail(-errno, "Error allocating memory");
	return (pnt);
}



long putinxid(
	char *bufr,
	long  size)
{
	static char *spintbl[] = {"\b/", "\b-", "\b\\", "\b|"};

	long  rtn;
	ulong crc;

	if (chkhndl > 0 && (rtn = svcIoOutBlock(chkhndl, bufr, size)) < 0)
		fail(rtn, "Error writing check file");
	if (xidspec != NULL)
	{
		stream.next_in = bufr;
		stream.avail_in = size;
		do
		{
			if ((xidhead.totalsize - lastspin) > 200000)
			{
				lastspin = xidhead.totalsize;
				fputs(spintbl[spininx++], stdout);
				if (spininx > 3)
					spininx = 0;
			}
			if (stream.avail_out == 0)
			{
				if ((rtn = svcIoOutBlock(xidhndl, xidbufr, XIDBUFRSZ)) < 0)
					return (rtn);
				stream.next_out = xidbufr;
				stream.avail_out = XIDBUFRSZ;
			}
			if ((rtn = deflate(&stream, zflag)) < 0)
				return ((ushort)rtn | 0x80000000);
		} while (stream.avail_in != 0 || (zflag == Z_FINISH &&
				rtn != Z_STREAM_END));
	}
	xidhead.totalsize += size;
	crc = xidhead.crcvalue;
    while (--size >= 0)
        crc = crctable[(crc ^ *bufr++) & 0xFF] ^ (crc >> 8);
	xidhead.crcvalue = crc;
	return (0);
}


char *setupfile(
	char *arg,
	int   len,
	char *ext)
{
	char *spec;
	char *pnt;

	char  chr;

	spec = malloc(len + 5);
	strmov(spec, arg);
	pnt = arg + len;
	while ((chr = *--pnt) != '.')
	{
		if (pnt <= arg || (chr = *pnt) == '\\' | chr == ':')
		{
			strmov(spec + len, ext);
			break;
		}
	}
	return (spec);
}


char *fixdefault(
	char *file)
{
	char *newf;
	char *pnt;
	int   len;

	if (file == NULL || file[0] != '.')
		return (file);
	pnt = strrchr(ctlspec, '.');		// Find end of the INS name
	len = (pnt - ctlspec) + strlen(file) + 1;
	newf = malloc(len);
	strmov(strnmov(newf, ctlspec, pnt - ctlspec), file);
	free(file);
	return (newf);
}



char *tbl[] =
{	"Version error",			// -6
	"Buffer error",				// -5
	"Memory error",				// -4
	"Data error",				// -3
	"Stream error",				// -2
	NULL,						// -1
	"No error indicated",		//  0
	"Unexpected stream end",	//  1
	"Need Dictionary"			//  2
};


void fail(
	long code,
	char *fmt, ...)
{
	va_list pi;
	char    text[600];

   	va_start(pi, fmt);
   	vsprintf(text, fmt, pi);
	if (code == 0)						// Is there an error code?
	{
		fprintf(stderr, "? %s %s\n", prgname, text); // No - just display the
		exit(1);									 //   message and die
	}
	if ((code & 0xFFFF0000) != 0x80000000) // Is this a zlib error code?
		femsg2(prgname, text, code, NULL); // No
	code = (short)code;					// Yes
	if (code == -1)						// Really an errno value?
		femsg2(prgname, text, -errno, NULL); // Yes
	if (code < -6 || code > 2)			// No - it's really a zlib code
		fprintf(stderr, "? MKINSTALL: %s\n             Unknown zlib error "
				"code %d\n", text, code);
	else
		fprintf(stderr, "? MKINSTALL: %s\n             zlib error: %s\n", text,
			tbl[code + 6]);
	exit(1);
}
