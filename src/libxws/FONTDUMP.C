#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSERMSG.H>
#include <XOSSTR.H>


// Format of the an XOS font file:
//   The header (the first 100 bytes of the file) is specified by the
//   fontheader structure.
//   The header is followed immediatly (starting in by 100 of the file) with
//     the glyph pointer table.  Each entry is a long which contains the offset
//     from the start of the glyph table to the bit-map data for the character.
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


typedef struct fontheader
{   char   magic1[4];		// "BMF*"
    long   magic2;		// 0xFFFF0000
    ushort version;		// High byte major version, low byte minor
				//   version (currently 1.0)
    ushort glyphtbl;		// Offset in file of start of the glyph
				//   pointer table (currently always 100)
    char   copyright[60];	// Copyright notice
    ushort ascent;		// Ascent (pixels)
    ushort descent;		// Descent (pixels)
    ushort inleading;		// Internal leading (pixels)
    ushort exleading;		// External leading (pixels)
    ushort height;		// Height (pixels) (must be ascent + descent
				//   + inleading)
    ushort maxwidth;		// Maximum width
    ushort avgwidth;		// Average width
    ushort weight;		// Weight
    ushort underpos;		// Underscore position
    ushort strikepos;		// Strike-out position
    long   flags;		// Flag bits:
				//   Bit 0: 1 if italic font
				//   Bit 1: 1 if monospaced font
				//   Other bits are reserved and must be 0
    uchar  firstchar;		// First character
    uchar  numchars;		// Number of characters
    uchar  defaultchar;		// Default character
    uchar  reserved1;		// Reserved, must be 0
} FONTHEADER;

typedef struct fontinfo
{   ushort dfVersion;
    long   dfSize;
    char   dfCopyright[60];
    ushort dfType;
    ushort dfPoints;
    ushort dfVertRes;
    ushort dfHorizRes;
    ushort dfAscent;
    ushort dfInternalLeading;
    ushort dfExternalLeading;
    uchar  dfItalic;
    uchar  dfUnderline;
    uchar  dfStrikeOut;
    ushort dfWeight;
    uchar  dfCharSet;
    ushort dfPixWidth;
    ushort dfPixHeight;
    uchar  dfPitchAndFamily;
    ushort dfAvgWidth;
    ushort dfMaxWidth;
    uchar  dfFirstChar;
    uchar  dfLastChar; 
    uchar  dfDefaultChar;
    uchar  dfBreakChar;
    ushort dfWidthBytes;
    long   dfDevice;
    long   dfFace;
    long   dfBitsPointer;
    long   dfBitsOffset;
    uchar  dfReserved;
} FONTINFO;

typedef struct glyph2
{   ushort gWidth;
    ushort gOffset;
} GLYPH2;

typedef struct glyph3
{   ushort gWidth;
    long   gOffset;
} GLYPH3;

FONTINFO   info;
FONTHEADER header = {"BMF*", 0xFFFF, 0, 0x0100, 100};


extern ulong swapword (ulong value);
#pragma aux swapword =	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];

extern ulong swaplong (ulong value);
#pragma aux swaplong =	\
   "xchg al, ah"	\
   "ror  eax, 16"	\
   "xchg al, ah"	\
    parm [EAX] value [EAX];


extern int errno;


int     infile;
int     outfile;
GLYPH2 *oldglyphtbl;
GLYPH2 *oldgpnt;
long   *newglyphtbl;
long   *newgpnt;

uchar  *oldbitmap;
uchar  *newbitmap;
uchar  *bpnt;
uchar  *spnt;
uchar  *dpnt;
uchar  *hold;
char   *lpnt;

int     filepos;
int     byte;
int     bit;
int     cnth;
int     cntw;
int     base;
int     total;
int     width;
int     size;
int     num;
int     cnt;

char    line[256];
char    prgname[] = "FONTDUMP";


int main(
    int   argc,
    char *argv[])

{
    int rtn;

    if (argc < 3)
    {
	fputs("? FONTDUMP: Command error, useage is:\n"
		"              FONTDUMP infile outfile\n", stderr);
	exit (1);
    }

    base = 12;

    if ((infile = svcIoOpen(O_IN, argv[1], NULL)) < 0)
	femsg2(prgname, "Cannot open input font file", infile, argv[1]);

    if ((outfile = svcIoOpen(O_IN|O_OUT|O_CREATE|O_TRUNCW, argv[2], NULL)) < 0)
	femsg2(prgname, "Cannot open output font file", infile, argv[2]);

    svcIoSetPos(infile, base, 0);


    if ((rtn = svcIoInBlock(infile, (char *)&info, sizeof(info))) !=
	    sizeof(info))
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error reading fontinfo data", rtn, NULL);
    }

    printf("    Version: 0x%04.4X\n", info.dfVersion);
    printf("       Size: %d\n", info.dfSize);
    printf("  Copyright: %.60s\n", info.dfCopyright);
    printf("       Type: 0x%04.4X\n", info.dfType);
    printf("     Points: %d\n", info.dfPoints);
    printf("    VertRes: %d\n", info.dfVertRes);
    printf("   HorizRes: %d\n", info.dfHorizRes);
    printf("     Ascent: %d\n", info.dfAscent);
    printf("  InLeading: %d\n", info.dfInternalLeading);
    printf("  ExLeading: %d\n", info.dfExternalLeading);
    printf("     Italic: %d\n", info.dfItalic);
    printf("  Underline: %d\n", info.dfUnderline);
    printf("  Strikeout: %d\n", info.dfStrikeOut);
    printf("     Weight: %d\n", info.dfWeight);
    printf("    CharSet: %d\n", info.dfCharSet);
    printf("   PixWidth: %d\n", info.dfPixWidth);
    printf("  PixHeight: %d\n", info.dfPixHeight);
    printf("     Family: 0x%02.2X\n", info.dfPitchAndFamily);
    printf("   AvgWidth: %d\n", info.dfAvgWidth);
    printf("   MaxWidth: %d\n", info.dfMaxWidth);
    printf("  FirstChar: 0x%02.2X\n", info.dfFirstChar);
    printf("   LastChar: 0x%02.2X\n", info.dfLastChar);
    printf("  DefltChar: 0x%02.2X\n", info.dfDefaultChar);
    printf("    BrkChar: 0x%02.2X\n", info.dfBreakChar);
    printf(" WidthBytes: %d\n", info.dfWidthBytes);
    printf("     Device: 0x%08.8X\n", info.dfDevice);
    printf("       Face: 0x%08.8X\n", info.dfFace);
    printf("BitsPointer: %d\n", info.dfBitsPointer);
    printf(" BitsOffset: 0x%08.8X\n", info.dfBitsOffset);

    strncpy(header.copyright, info.dfCopyright, 60);
    header.ascent = info.dfAscent;
    header.inleading = info.dfInternalLeading;
    header.exleading = info.dfExternalLeading;
    header.height = info.dfPixHeight;
    header.descent = header.height - (header.ascent + header.inleading);
    header.maxwidth = info.dfMaxWidth;
    header.avgwidth = info.dfAvgWidth;
    header.weight = info.dfWeight;
    header.underpos = 0;
    header.strikepos = 0;
    header.flags = ((info.dfItalic) ? 1 : 0) + ((info.dfPixWidth) ? 0 : 2);
    header.firstchar = info.dfFirstChar;
    header.defaultchar = info.dfDefaultChar;
    header.numchars = info.dfLastChar - header.firstchar + 1;

    printf("\nFont contains %d characters\n", header.numchars);

    cnt = header.numchars * sizeof(GLYPH2);
    oldgpnt = oldglyphtbl = getspace(cnt);
    newgpnt = newglyphtbl = getspace(header.numchars * 4);

    if ((rtn = svcIoInBlock(infile, (char *)oldglyphtbl, cnt)) != cnt)
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error reading glyph table", rtn, NULL);
    }

    filepos = header.numchars * 4 + sizeof(FONTHEADER);
    if ((rtn = svcIoOutBlock(outfile, (char *)&header, sizeof(FONTHEADER))) !=
	    sizeof(FONTHEADER) || (rtn = svcIoSetPos(outfile, filepos, 0)) < 0)
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error writing font file header", rtn, NULL);
    }

    num = info.dfFirstChar;
    cnt = header.numchars;
    total = 0;
    while (--cnt >= 0)
    {
	printf("Char 0x%02.2X: w=%-2d bm=0x%04.4X\n", num++, oldgpnt->gWidth,
		oldgpnt->gOffset);
	total += (oldgpnt->gWidth + 7)/8;
	oldgpnt++;
    }

    cnt = (info.dfMaxWidth + 7)/8 * info.dfPixHeight;
    oldbitmap = getspace(cnt);
    newbitmap = getspace(cnt + 2);

    oldgpnt = oldglyphtbl;
    cnt = header.numchars;
    num = info.dfFirstChar;
    while (--cnt >= 0)
    {
	width = (oldgpnt->gWidth + 7)/8;
	size = width * info.dfPixHeight;

	printf("### width = %d, size = %d\n", width, size);

	if ((rtn = svcIoSetPos(infile, oldgpnt->gOffset + base, 0)) < 0 ||
		(rtn = svcIoInBlock(infile, oldbitmap, size)) != size)
	{
	    if (rtn >= 0)
		rtn = ER_EOF;
	    femsg2(prgname, "Error reading bitmap data", rtn, NULL);
	}
	spnt = oldbitmap;
	dpnt = newbitmap + 2;
	*(ushort *)newbitmap = oldgpnt->gWidth;

	cnth = info.dfPixHeight;	// Convert bit-map from column major
	*dpnt++ = oldgpnt->gWidth;	//   to row major
	do
	{
	    cntw = width;
	    hold = spnt;
	    do
	    {
		*dpnt++ = *spnt;
		spnt += info.dfPixHeight;
	    } while (--cntw > 0);
	    spnt = hold + 1;

	} while (--cnth > 0);

	while ((((long)dpnt) & 0x03) != 0) // Fill out the bit-map to a long
	    *dpnt++ = 0;		   //   boundry
	cntw = dpnt - newbitmap;
	*newgpnt++ = filepos;
	if ((rtn = svcIoOutBlock(outfile, oldbitmap, cntw)) != cntw)
	{
	    if (rtn >= 0)
		rtn = ER_EOF;
	    femsg2(prgname, "Error writing bitmap data", rtn, NULL);
	}
	filepos += cnt;

	printf("\nChar 0x%02.2X: ", num++);
	cnth = info.dfPixHeight;

	do
	{
	    lpnt = line;
	    if (cnth != info.dfPixHeight)
		lpnt = strmov(lpnt, "           ");
	    cntw = oldgpnt->gWidth;
	    bit = 0;
	    do
	    {
		if ((bit++ & 0x07) == 0)
		    byte = *bpnt++;
		*lpnt++ = (byte & 0x80) ? '@' : 0xF9;
		byte <<= 1;
	    } while (--cntw > 0);
	    *lpnt = 0;
	    printf("%s\n", line);
	} while (--cnth > 0);

	oldgpnt++;
    }
    return (0);
}
