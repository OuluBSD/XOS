#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <errmsg.h>


FILE *file;

	struct
	{	char   idstr[20];
		ushort datablk;
		uchar  vernum[2];
		ushort verchk;
	}    header;

char  prgname[] = "VOCDMP";


void main(
    int   argc,
    char *argv[])

{
	long  rtn;
	long  pos;
	long  blklen;
	uchar bdata[4];
	uchar blktype;
///	char  text[64];

	if (argc != 2)
	{
		fputs("? VOCDMP: Command error, usage is:\n"
				"            VOCDMP filespec\n\n", stderr);
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
	if (memcmp(header.idstr, "Creative Voice File\x1A", 14) != 0)
		femsg2(prgname, "Header ID string is incorrect", 0, NULL);

	printf("Version: %d.%d\n", header.vernum[1], header.vernum[0]);


	pos = header.datablk;

	while (TRUE)
	{
		if (fseek(file, pos, 0) < 0)
			femsg2(prgname, "Error setting input position", -errno, NULL);
		if ((rtn = fread(&blktype, 1, 1, file)) != 1)
		{
			if (rtn >= 0)
				rtn = -ER_EOF;
			femsg2(prgname, "Error reading block type", -errno, NULL);
		}
		if (blktype == 0)
		{
			fputs("Block 0: End of file\n", stdout);
			exit(0);
		}
		blklen = 0;
		if ((rtn = fread(&blklen, 1, 3, file)) != 3)
		{
			if (rtn >= 0)
				rtn = -ER_EOF;
			femsg2(prgname, "Error reading block length", -errno, NULL);
		}
		printf("Block %d: %d byte%s\n", blktype, blklen, (blklen == 1) ?
				"" : "s");
		switch (blktype)
		{
		 case 1:						// Sound data

			if ((rtn = fread(&bdata, 1, 2, file)) != 2)
			{
				if (rtn >= 0)
					rtn = -ER_EOF;
				femsg2(prgname, "Error reading sound data block", -errno, NULL);
			}
			printf("  Sound data: Rate=%d (%d), Compression=%d\n", bdata[0],
					1000000/(256 - bdata[0]), bdata[1]);


			break;

		 case 2:						// Continued sound data
			fputs("  Continued sound data\n", stdout);

			break;

		 case 3:						// Silence
			printf("  Silence: Length=%d, Rate=%d\n", 0, 0);

			break;

		 case 4:						// Marker
			printf("  Marker: Number=%d\n", 0);
			break;

		 case 5:						// ASCII
			printf("  ASCII: Text=%.*s\n", 0, "");
			break;

		 case 6:						// Repeat
			printf("  Start repeat: Count=%d\n", 0);
			break;

		 case 7:						// End repeat
			fputs("  End repeat\n", stdout);
			break;

		 case 8:						// Extended
			printf("  Extended info:\n");
			break;

		 default:
			fputs("  Unknown block type, block skipped", stdout);
			break;
		}
		pos += (blklen + 4);

	}
}
