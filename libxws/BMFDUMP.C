#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERMSG.H>
#include <XOSSTR.H>
#include "BMF.H"


// Format of the an XOS font file:
//   The header (the first 100 bytes of the file) is specified by the
//   fontheader structure.
//   The header is followed immediatly (starting in by 100 of the file) with
//     the glyph pointer table.  Each entry is a long which contains the offset
//     from the start of the file to the bit-map data for the character.
//     There are "numchars" pointers with the first pointer corresponding to
//     "firstchar".
//   The remainder of the file contains the bit-map data structures for each
//     character.  This data is normally stored starting immediately after the
//     glyph pointer table in character code order, but this is not required.
//     Each structure must begin on a long boundry.  Each structure has the
//     following format:
//	ushort width;		// Width of the character (pixels)
//	uchar  data[n];		// Bit-map data
//   The bit-map data is a monochrome bit-map for the character cell. It is
//     stored by rows starting with the top row.  Each row is stored from left
//     to right with the high order bit of the first byte representing the
//     left-most pixel on the row.  Each row must be byte aligned.  There must
//     be "height" rows for each character.


extern int errno;

int     file;

long   *glyphtbl;
long   *gpnt;
uchar  *bitmap;
uchar  *bpnt;
char   *lpnt;

int     byte;
int     bit;
int     cnth;
int     cntw;
int     width;
int     size;
int     cnt;
int     num;
int     numchars;

FONTHEADER header;

char    line[256];
char    prgname[] = "BMFDUMP";


int main(
    int   argc,
    char *argv[])

{
    int rtn;

    if (argc < 2)
    {
	fputs("? BMFDUMP: Command error, useage is:\n"
		"             FONTDUMP file\n", stderr);
	exit (1);
    }

    if ((file = svcIoOpen(O_IN, argv[1], NULL)) < 0)
	femsg2(prgname, "Cannot open input font file", file, argv[1]);

    if ((rtn = svcIoInBlock(file, (char *)&header, sizeof(header))) !=
	    sizeof(header))
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error reading font file header data", rtn, NULL);
    }

    numchars = (header.numchars == 0) ? 256 : header.numchars;

    printf("    Version: 0x%04.4X\n", header.version);
    printf("       Size: %d\n", header.length);
    printf("  Copyright: %.60s\n", header.copyright);
    printf("       Name: %.32s\n", header.name);
    printf("     Ascent: %d\n", header.ascent);
    printf("    Descent: %d\n", header.descent);
    printf("  InLeading: %d\n", header.inleading);
    printf("  ExLeading: %d\n", header.exleading);
    printf("     Height: %d\n", header.height);
    printf("  Underline: %d\n", header.underpos);
    printf("  Strikeout: %d\n", header.strikepos);
    printf("     Weight: %d\n", header.weight);
    printf("   AvgWidth: %d\n", header.avgwidth);
    printf("   MaxWidth: %d\n", header.maxwidth);
    printf("  FirstChar: 0x%02.2X\n", header.firstchar);
    printf("   NumChars: %d\n", numchars);
    printf("  DefltChar: 0x%02.2X\n", header.dfltchar);

    // Allocate space and read the entire glyph pointer table

    cnt = numchars * 4;
    gpnt = glyphtbl = getspace(cnt);
    if ((rtn = svcIoInBlock(file, (char *)glyphtbl, cnt)) != cnt)
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error reading glyph pointer table", rtn, NULL);
    }
    bitmap = getspace((header.maxwidth + 7)/8 * header.height + 2);

    cnt = numchars;
    num = header.firstchar;
    while (--cnt >= 0)
    {
	if ((rtn = svcIoSetPos(file, *gpnt++, 0)) < 0 ||
		(rtn = svcIoInBlock(file, (char *)&width, 2)) != 2)
	{
	    if (rtn >= 0)
		rtn = ER_EOF;
	    femsg2(prgname, "Error reading bit-map data", rtn, NULL);
	}
	size = ((width + 7)/8) * header.height;
	if ((rtn = svcIoInBlock(file, bitmap, size)) != size)
	{
	    if (rtn >= 0)
		rtn = ER_EOF;
	    femsg2(prgname, "Error reading bit-map data", rtn, NULL);
	}
	printf("\nChar 0x%02.2X: width = %d\n", num++, width);
	cnth = header.height;
	bpnt = bitmap;
	do
	{
	    lpnt = line;
	    cntw = width;
	    bit = 0;
	    do
	    {
		if ((bit++ & 0x07) == 0)
		    byte = *bpnt++;
		*lpnt++ = (byte & 0x80) ? 0xDB : 0xF9;
		byte <<= 1;
	    } while (--cntw > 0);
	    *lpnt = 0;
	    printf("    %s\n", line);
	} while (--cnth > 0);
    }
    svcIoClose(file, 0);
    return (0);
}
