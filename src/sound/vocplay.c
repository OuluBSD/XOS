#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <errmsg.h>
#include <xos.h>
#include <xossvc.h>
#include <xossnd.h>

#define sndbufr ((uchar *)0x00400000)

FILE *file;
long  sb;

long  bufrsize;
long  fragsize;
long  offset;
long  avail;

struct
{	char   idstr[20];
	ushort datablk;
	uchar  vernum[2];
	ushort verchk;
}    header;

struct
{	text8_parm class;
	byte4_parm state;
	byte4_parm rate;
	byte4_parm volume;
	uchar      end;
} sbparms =
{	{PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "SND"},
	{PAR_SET|REP_DECV, 4, IOPAR_SND_CMD, SND_CMD_START},
	{PAR_SET|REP_DECV, 4, IOPAR_SND_RATE},
	{PAR_SET|REP_DECV, 4, IOPAR_SND_VOLUME}
};

struct
{	text8_parm class;
	uchar      end;
} spdfparms =
{	{PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "SND"}
};

char  prgname[] = "VOCPLAY";

void soundout(int cnt);

void main(
    int   argc,
    char *argv[])

{
	long  rtn;
	long  pos;
	long  blklen;
	uchar bdata[4];
	uchar blktype;

	if (argc != 3)
	{
		fputs("? VOCPLAY: Command error, usage is:\n"
				"             VOCPLAY filespec volume\n\n", stderr);
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

	if ((sb = svcIoOpen(XO_OUT, "SND:", NULL)) < 0)
		femsg2(prgname, "Error opening sound device", sb, NULL);



	if ((bufrsize = svcIoSpecial(sb, SPDF_SNDA_MAPOUTBFR, sndbufr, 0,
			&spdfparms)) < 0)


		femsg2(prgname, "Error mapping sound buffer", bufrsize, NULL);
	fragsize = bufrsize/2;
	offset = 0;
	printf("DMA size = %d\n", fragsize);

	sbparms.volume.value = atoi(argv[2]);
	printf("Volume = %d\n", sbparms.volume.value);
	sbparms.volume.value |= (sbparms.volume.value << 8);

	avail = fragsize;
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

			printf("%d", fragsize - avail);

			sbparms.state.value = SNDA_STATE_FINAL;
			if ((rtn = svcIoOutBlockP(sb, NULL, fragsize - avail,
					&sbparms)) < 0)
				femsg2(prgname, "Error writing final block to sound device",
						rtn, NULL);

			fputs("!\n", stdout);

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
			sbparms.rate.value = 1000000/(256 - bdata[0]);
			printf("  Sound data: Rate=%d (%d), Compression=%d\n", bdata[0],
					sbparms.rate.value, bdata[1]);
			soundout(blklen - 2);
			break;

		 case 2:						// Continued sound data
			fputs("  Continued sound data\n", stdout);
			soundout(blklen);
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



void soundout(
	int len)

{
	long amount;
	long rtn;

	while (len > 0)
	{
		if (avail <= 0)
		{
///			printf("%d", fragsize);

			if ((rtn = svcIoOutBlockP(sb, NULL, fragsize, &sbparms)) < 0)
				femsg2(prgname, "Error writing to sound device", rtn, NULL);
			sbparms.state.value = SNDA_STATE_CONT;
			fputs("-", stdout);
			avail = fragsize;
			if ((offset += fragsize) >= bufrsize)
				offset = 0;
		}
		if ((amount = len) > avail)
			amount = avail;
		if ((rtn = fread(sndbufr + offset + fragsize - avail, 1, amount,
				file)) != amount)
		{
			if (rtn >= 0)
				rtn = -ER_EOF;
			femsg2(prgname, "Error reading sound data block", -errno, NULL);
		}
		avail -= amount;
		len -= amount;
	}
}
