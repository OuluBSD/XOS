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

int     byte;
int     bit;
int     cnth;
int     cntw;
int     base;
int     width;
int     size;
int     cnt;
int     numchars;

char    prgname[] = "BMFCONV";


int main(
    int   argc,
    char *argv[])

{
    int rtn;

    fputs("BMFCONV - XOS bit-map font file converter, version 1.0\n\n", stdout);
    if (argc < 3 || argc > 4)
    {
	fputs("? BMFCONV: Command error, useage is:\n"
		"             BMFCONV infile outfile {fontname}\n", stderr);
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
    printf("  Version: 0x%04.4X\n", info.dfVersion);
    printf("     Size: %d\n", info.dfSize);
    printf("Copyright: %.60s\n", info.dfCopyright);
    printf("   Ascent: %d\n", info.dfAscent);
    printf("InLeading: %d\n", info.dfInternalLeading);
    printf("ExLeading: %d\n", info.dfExternalLeading);
    printf("   Italic: %d\n", info.dfItalic);
    printf("   Weight: %d\n", info.dfWeight);
    printf("  CharSet: %d\n", info.dfCharSet);
    printf("    Width: %d\n", info.dfPixWidth);
    printf("   Height: %d\n", info.dfPixHeight);
    printf(" AvgWidth: %d\n", info.dfAvgWidth);
    printf(" MaxWidth: %d\n", info.dfMaxWidth);
    printf("FirstChar: 0x%02.2X\n", info.dfFirstChar);
    printf(" LastChar: 0x%02.2X\n", info.dfLastChar);
    printf("DefltChar: 0x%02.2X\n", info.dfDefaultChar);

    strncpy(header.copyright, info.dfCopyright, sizeof(info.dfCopyright));
    header.ascent = info.dfAscent;
    header.inleading = info.dfInternalLeading;
    header.exleading = info.dfExternalLeading;
    header.height = info.dfPixHeight;
    header.descent = header.height - header.ascent;
    header.maxwidth = info.dfMaxWidth;
    header.avgwidth = info.dfAvgWidth;
    header.weight = info.dfWeight;
    header.underpos = 0;
    header.strikepos = 0;
    header.flags = ((info.dfItalic) ? 1 : 0) + ((info.dfPixWidth) ? 0 : 2);
    header.firstchar = info.dfFirstChar;
    header.dfltchar = info.dfDefaultChar;
    header.numchars = info.dfLastChar - header.firstchar + 1;

    numchars = (header.numchars == 0) ? 256 : header.numchars;

    printf("\nFont contains %d characters\n", numchars);

    cnt = numchars * sizeof(GLYPH2);
    oldgpnt = oldglyphtbl = getspace(cnt);
    newgpnt = newglyphtbl = getspace(numchars * 4);

    if ((rtn = svcIoInBlock(infile, (char *)oldglyphtbl, cnt)) != cnt)
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error reading glyph table", rtn, NULL);
    }

    header.length = numchars * 4 + sizeof(FONTHEADER);
    if ((rtn = svcIoSetPos(outfile, header.length, 0)) < 0)
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error setting output position", rtn, NULL);
    }
    cnt = (info.dfMaxWidth + 7)/8 * info.dfPixHeight;
    oldbitmap = getspace(cnt);
    newbitmap = getspace(cnt + 2);

    oldgpnt = oldglyphtbl;
    cnt = numchars;
    while (--cnt >= 0)
    {
	width = (oldgpnt->gWidth + 7)/8;
	size = width * info.dfPixHeight;
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
	do				//   to row major order
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
	*newgpnt++ = header.length;
	if ((rtn = svcIoOutBlock(outfile, newbitmap, cntw)) != cntw)
	{
	    if (rtn >= 0)
		rtn = ER_EOF;
	    femsg2(prgname, "Error writing bitmap data", rtn, NULL);
	}
	header.length += cntw;
	oldgpnt++;
    }

    if (argc == 4)
	strncpy(header.name, argv[3], sizeof(header.name));
    else
    {
	if ((rtn = svcIoSetPos(infile, info.dfFace + base, 0)) < 0 ||
		(rtn = svcIoInBlock(infile, header.name,
		sizeof(header.name))) < 0)
	    femsg2(prgname, "Error reading font name string", rtn, NULL);
	cnt = sizeof(header.name);	// Zero out anything following the
	lpnt = header.name;		//   name string
	while (cnt > 0 && *lpnt != 0)
	{
	    --cnt;
	    if (*lpnt == ' ')
		*lpnt = '_';
	    lpnt++;
	}
	while (--cnt >= 0)
	    *lpnt++ = 0;
    }
    printf("File size is %d bytes\n", header.length);
    if ((rtn = svcIoSetPos(outfile, 0, 0)) < 0 || (rtn = svcIoOutBlock(outfile,
	    (char *)&header, sizeof(FONTHEADER))) != sizeof(FONTHEADER))
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error writing font file header", rtn, NULL);
    }
    cnt = numchars * 4;
    if ((rtn = svcIoOutBlock(outfile, (char *)newglyphtbl, cnt)) != cnt)
    {
	if (rtn >= 0)
	    rtn = ER_EOF;
	femsg2(prgname, "Error writing glyph pointers", rtn, NULL);
    }
    return (0);
}
