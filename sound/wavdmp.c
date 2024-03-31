#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <errmsg.h>


FILE *file;

struct
{	char   flabel[4];
	long   length;
	uchar  ftype[4];
}    header;

struct
{	ushort compress;
	ushort channels;
	long   rate;
	long   dmarate;
	ushort sampsize;
	ushort width;
}    fmtdata;


typedef struct
{	int   value;
	char *name;
} COMP;

COMP comptbl[] =
{	{     0, "Unknown"},
	{     1, "Uncompressed"},
	{     2, "Microsoft ADPCM"},
	{     6, "ITU G.711 a-law"},
	{     7, "ITU G.711 u-law"},
	{    17, "IMA ADPCM"},
	{    20, "ITU G.723 ADPCM (Yamaha)"},
	{    49, "GSM 6.10"},
	{    64, "ITU G.721 ADPCM"},
	{    80, "MPEG"},
	{0xFFFF, "Experimental"},
	{    -1, "No description available"}
};

char prgname[] = "WAVDMP";


void main(
    int   argc,
    char *argv[])

{
	COMP *cpnt;
	long  rtn;
	long  fileleft;
	long  cleft;
	long  filepos;
	long  amnt;

	if (argc != 2)
	{
		fputs("? WAVDMP: Command error, usage is:\n"
				"            WAVDMP filespec\n\n", stderr);
		exit(1);
	}
	if ((file = fopen(argv[1], "r")) == NULL)
		femsg2(prgname, "Cannot open input file", -errno, NULL);

	if ((rtn = fread((char *)&header, 1, sizeof(header), file)) !=
			sizeof(header))
	{
		if (rtn >= 0)
			rtn = -ER_EOF;
		femsg2(prgname, "Error reading file header", -errno, NULL);
	}
	if (memcmp(header.flabel, "RIFF", 4) != 0)
		femsg2(prgname, "Header file label (RIFF) is incorrect", 0, NULL);
	if (memcmp(header.ftype, "WAVE", 4) != 0)
		femsg2(prgname, "Header file type (WAVE) is incorrect", 0, NULL);

	fileleft = header.length - 4;
	filepos = sizeof(header);

	while (fileleft > 8)
	{
		if (fseek(file, filepos, 0) < 0)
			femsg2(prgname, "Error setting input position", -errno, NULL);
		if ((rtn = fread((char *)&header, 1, 8, file)) != 8)
		{
			if (errno >= 0)
				errno = -ER_EOF;
			femsg2(prgname, "Error reading chunk header", -errno, NULL);
		}
		printf("Chunk: %.4s, %d byte%s\n", header.flabel, header.length,
				(header.length == 1) ? "" : "s");
		cleft = header.length;

		rtn = ((header.length + 9) & 0xFFFFFFFE);
		filepos += rtn;
		fileleft -= rtn;

		if (strncmp(header.flabel, "fmt ", 4) == 0)
		{
			memset(&fmtdata, 0, sizeof(fmtdata));
			if ((amnt = cleft) > sizeof(fmtdata))
				amnt = sizeof(fmtdata);

			if ((rtn = fread((char *)&fmtdata, 1, sizeof(fmtdata), file)) !=
					sizeof(fmtdata))
			{
				if (errno >= 0)
					errno = -ER_EOF;
				femsg2(prgname, "Error reading \"fmt\" data", -errno, NULL);
			}
			cpnt = comptbl;
			do
			{
				if (cpnt->value == fmtdata.compress)
					break;
				cpnt++;
			} while (cpnt->value != -1);
			printf("   Compression: %d (%s)\n", fmtdata.compress, cpnt->name);
			printf("      Channels: %d\n", fmtdata.channels);
			printf("    Frame rate: %d\n", fmtdata.rate);
			printf("     Byte rate: %d\n", fmtdata.dmarate);
			printf("   Bytes/frame: %d\n", fmtdata.sampsize);
			printf("   Bits/sample: %d\n", fmtdata.width);
		}
	}
	if (fileleft != 0)
		printf("%d excess byte%s at end of file\n", fileleft,
				(fileleft == 1) ? "" : "s");
}
