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

// This program generates an XOS Install Data (XID) file. The contents for the
//   is taken from the installed versions of XOS and applications on the system
//   that mkinstall is run on. This file is intended for use as input to the
//   Windows program mkxosbtdsk which creates a bootable USB disk from this
//   data.

// The XID file starts with an 8 byte header which has the following format:
//   Offset Size Description
//     0      4  File label = "XIDF"
//     4      1  File format version = 2
//     5      3  Reserved = 0
// This is immediately followed by a zlib compressed data stream which consists
//   of a sequence of directory/file items, tightly packed. Each item has the
//   following format:
//   Offset Size Description
//     0      2  Sync valie = 0xC55C
//     4      2  Name length - bit 15 set if data length value follows
//     6      4  Data length
//   6 or 10  n  File name (not 0 terminated)
//   This is immediately followed by the file data, if present. The first
//   character of the name field is not part of the name but contains bits
//   which specify item attributes:
//     Bit  7   - Set if directory/file should be copied to the boot disk
//     Bit  6   - Set if directory/file should be added to an install set
//     BIt  5   - Set if part of the OS
//     Bit  4   - Set if part of an application
//     BIts 3-0 - Item type:
//                  0 = Bootstrap file (must be first item)
//                  1 = File
//                  2 = Directory
//                  3 = Sub-directory
//   If bit 4 or 5 is not set, bit 6 is ignored.
//   If the type is 1 (directory) and bit 4 (application) is set, the name
//   must specify a single directory level which is the name of a "vendor"
//   directory which is created under the \app directory. This is followed
//   by a | character which is followed by the name of the application. This
//   name is used when adding the application to an install set to build the
//   name of the application. This is followed by another | character which
//   is followed by the version number as MJ.MN.EN.

//   If the type is 1 (directory) and bit 5 (OS) is set, the name must specify
//   a single directory level which is the name of a the base directory for
//   the OS. This must be followed by a | character which is followed by the
//   name of the main OS executable (without extension) which is in the
//   \base\sys\ directory. This is followed by a | character which is followed
//   by the version number as MJ.MN.EN.

//   All base directory specifications are assumed to start at the root (no
//   leading \ is needed or allowed). Sub-directory specifications start at
//   the base directory.

// This program is controlled by an input file which consists of a number of
//   lines that somewhat mirror the file structure specified above. Each line
//   begins with a two-letter prefix followed by a file name or directory
//   specification mostly as described above.

// The first letter specifies the item type as follows:
//   B - Bootstrap file - There must be exactly one B line which specifies
//       the bootstrap file using a full path specification. It does not have
//       to be first but it must be present.
//   O - Base directory for the OS
//   A - Base directory for an application
//   D - Base directory not part of the OS or a formal application
//   S - Sub-directory
//   F - File
//   X - Excluded file

// The second letter specifies how the item is handled when copied to a
//   boot disk as follows:
//   c - Copy item to the boot disk
//   i - Create an install set for the item or add the item to an install set
//   b - Do both of the above

// When an F line specifies a single * character as the file name, all files
//   in the directory not specified in an X line are copied as specified by
//   the second letter. In this case individual files may also be specified
//   the the second letter specifying only one destination. This will override
//   a b specification for the directory. In this case a second name may be
//   specified after a | character which will be used as the destination name.
//   This is intended to be used to allow different versions of a file for
//   the bootable image and for the install set. For example, this can be used
//   to put a version of ustartup.bat in the bootable image that will run
//   an install utility when the disk is booted but put the standard version
//   in the OS install set. (See the example below)
//   The second letter in an X line is ignored but must be present.
//   Blank lines and lines beginning with # are ignored.

// Example control file:
//
//   # This specifies the boot strap file.
//   Bbxos\sys\xosboot.run
//
//   # This installs the OS. Note that each sub-directory must be specified.
//   Obxos|sys\xos.run
//   Sbsys
//   Fb*
//   Sbcmd
//   Fb*
//   Sbcfg
//   Fb*
//   Fcustartup.bat
//   Fiustartupins.bat|ustartup.bat
//   Sbspl
//   Sbact
//   b*
//
//   # This installs the install application that consists of only the
//   #   executable.
//   Absystem|install
//
//   # This installs an application which includes some data files. Note
//   #   that the somevendor sub-directory is not specified since it is
//   #   created by the A line.
//   Absomevendor|someapp
//   Sbssomeapp\data
//   Fb*

// Command line format is:
//   mkinstall ctrlfile xidfile
// The default control file extention is .ins. The default XID file extension
//   is .xid.


char    *inpbufr;
char    *xidbufr;
z_stream stream;
long     amnt;
long     inhndl;
long     xidhndl;
long     chkhndl;
long     lastdot;
int      seqnum;
int      zflag = Z_NO_FLUSH;
FILE    *ctlfile;
char    *bootspec;
char     prgname[] = "MKINSTALL";

char    *ctlspec;
char    *xidspec;
char    *chkspec;

XIDHEAD xidhead = {"XIDF", 12, 1, 0, 0, 0, 0xFFFFFFFF};
DIRB   *dirhead;

IHEAD   ihead = {0xC183};


long getinput(char *bufr, long size, char *more);


static char *setupfile(char *arg, char *ext);


void main(
	int   argc,
	char *argv[])
{
	long  rtn;

	if (argc != 3 && argc != 4)
	{
		fputs("? MKINSTALL: Command error, usage is:\n"
				"               mkinstall ctrlfile outfile\n", stderr);
		exit(1);
	}

	// Allocate our buffers

	if ((inpbufr = malloc(INBUFRSZ)) == NULL ||
			(xidbufr = malloc(XIDBUFRSZ)) == NULL)
		fail(-errno, "Error allocating buffer");

	// Set up the file specs

	ctlspec = setupfile(argv[1], ".ins");
	xidspec = setupfile(argv[2], ".xid");
	if (argc == 4)
		chkspec = setupfile(argv[3], ".chk");

	// Open the INS file

	if ((ctlfile = fopen(ctlspec, "r")) == NULL)
		fail(-errno, "Error opening control file %s", ctlspec);

	// Read the INS file and set up our data base

	fputs("Setting up configuration", stdout);
	readins();
	if (bootspec == NULL)
	{
		fputs("? MKINSTALL: No bootstrap file specified\n", stderr);
		exit(1);
	}

	// Do any wild-card searches that are needed

	findfiles(dirhead, "");

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

	// Create the XID file

	fputs("\nStoring data items ", stdout);
	if ((xidhndl = svcIoOpen(XO_OUT|XO_TRUNCA|XO_CREATE, xidspec, NULL)) < 0)
		fail(xidhndl, "Error creating output file %s", xidspec);

	// Store the XID file header (this is not compressed). The length and
	//   checksum values will be filled in at the end.

	if ((rtn = svcIoOutBlock(xidhndl, (char *)&xidhead, sizeof(xidhead))) < 0)
		fail(rtn, "Error writing XID header");

	store1item(bootspec, "boot", IT_BOOT); // Store the bootstrap file
	storeitems(dirhead, "", IT_DIR);	// Store everything else

	// Store the final null file header to indicate EOF and finish up zlib
	//   output

	zflag = Z_FINISH;
	if ((!putinxid((char *)&ihead, 10)) < 0)
		fail(rtn, "Error storing EOF header in the XID file");
	if ((XIDBUFRSZ - stream.avail_out) > 0 &&
			(rtn = svcIoOutBlock(xidhndl, xidbufr, XIDBUFRSZ -
			stream.avail_out)) < 0)
		fail(rtn, "Error finishing output to the XID file");

	// Update the header with the uncompressed length and the CRC and close
	//   the file

	if ((rtn = svcIoSetPos(xidhndl, offsetof(XIDHEAD, totalsize), 0)) < 0 ||
			(rtn = svcIoOutBlock(xidhndl, (char *)&xidhead.totalsize, 12)) < 0)
		fail(rtn, "Error updating XID file header");
	if ((rtn = svcIoClose(xidhndl, 0)) < 0)
		fail(rtn, "Error closing the XID file");

	printf("\nComplete:\n        Total items: %,d\n       Total length: %,d\n"
			"  Compressed length: %,d\n             CRC-32: 0x%08X\n",
			xidhead.totalitems,  xidhead.totalsize, stream.total_out + 20,
			xidhead.crcvalue);
	exit(0);
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
	long  rtn;
	ulong crc;

	if (chkhndl > 0 && (rtn = svcIoOutBlock(chkhndl, bufr, size)) < 0)
		fail(rtn, "Error writing check file");

	stream.next_in = bufr;
	stream.avail_in = size;
	do
	{
		if ((xidhead.totalsize - lastdot) > 200000)
		{
			lastdot = xidhead.totalsize;
			fputs(".", stdout);
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

	xidhead.totalsize += size;

	crc = xidhead.crcvalue;
    while (--size >= 0)
        crc = crctable[(crc ^ *bufr++) & 0xFF] ^ (crc >> 8);
	xidhead.crcvalue = crc;
	return (0);
}


static char *setupfile(
	char *arg,
	char *ext)
{
	char *spec;
	char *pnt;
	int   len;
	char  chr;

	len = strlen(arg);
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


static char *tbl[] =
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
