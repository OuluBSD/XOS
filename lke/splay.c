#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <errmsg.h>
#include <xos.h>
#include <xossignal.h>
#include <xossvc.h>
#include <xossnd.h>


#define BUFRSZ 0x400
#define BLKBSZ 0x20000

extern char buffer[];

uchar *blkpnt;
FILE  *file;
long   bufrsize;
long   fragsize;
long   offset;
long   avail;
long   filepos;
long   hndl;
long   blkcnt;				// Data bytes available in current block

uchar  blkbufr[BLKBSZ];


struct
{	char   idstr[20];
	ushort datablk;
	uchar  vernum[2];
	ushort verchk;
}    header;

struct
{	TEXT8PARM class;
	uchar     end;
} spdfparms =
{	{PAR_SET|REP_TEXT, 8, IOPAR_CLASS, "SND"}
};

typedef struct
{	long  xxx[15];
	short datacnt;
	short signum;
	long  dataval;
	long  errcode;
	long  offset;
} SIGSTK;

struct
{	BYTE4PARM vect;
	uchar     end;
} sigparms =
{	{PAR_SET | REP_HEXV, 4, IOPAR_SIGVECT1, 60}
};

struct
{	TEXT4PARM class;
	BYTE4PARM volume;
	BYTE4PARM rate;
	uchar     end;
} sndparms =
{	{PAR_SET | REP_TEXT, 4, IOPAR_CLASS, "SND"},
	{PAR_SET | REP_HEXV, 4, IOPAR_SND_VOLUME},
	{PAR_SET | REP_HEXV, 4, IOPAR_SND_RATE}
};

struct
{	TEXT4PARM class;
	BYTE4PARM start;
	uchar     end;
} strtparms =
{	{PAR_SET | REP_TEXT, 4, IOPAR_CLASS, "SND"},
	{PAR_SET | REP_HEXV, 4, IOPAR_SND_CMD, SCMD_START}
};

struct
{	TEXT4PARM class;
	BYTE4PARM stop;
	uchar     end;
} stopparms =
{	{PAR_SET | REP_TEXT, 4, IOPAR_CLASS, "SND"},
	{PAR_SET | REP_HEXV, 4, IOPAR_SND_CMD, SCMD_STOP}
};




uchar eof;
uchar stop;


char  prgname[] = "SPLAY";


void fillbuffer(void);
void finalbuffer(void);

void sndsignal(SIGSTK stk);


void main(
    int   argc,
    char *argv[])

{
	long  rtn;

	if (argc != 3)
	{
		fputs("? SPLAY: Command error, usage is:\n"
				"             SPLAY filespec volume\n\n", stderr);
		exit(1);
	}
	if ((file = fopen(argv[1], "rb")) == NULL)
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

	if ((rtn = svcMemChange(buffer, 2, BUFRSZ)) < 0)
		femsg2(prgname, "Cannot allocate buffer", rtn, NULL);

	if ((hndl = svcIoOpen(0, "SND:", NULL)) < 0)
		femsg2(prgname, "Cannot open sound device", hndl, NULL);

	if (setvector(60, 3, sndsignal) < 0)
		femsg2(prgname, "Cannot set signal vector", -errno, NULL);

	if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &sigparms)) < 0)
		femsg2(prgname, "Cannot specify signal vector", rtn, NULL);

	if ((rtn = svcIoOutBlock(hndl, buffer, BUFRSZ)) < 0)
		femsg2(prgname, "Error setting buffer address", rtn, NULL);

	sndparms.volume.value = atoi(argv[2]);
	printf("Volume = %d\n", sndparms.volume.value);
	sndparms.volume.value |= (sndparms.volume.value << 8);


	filepos = header.datablk;


	offset = 0;							// Start at beginning of the buffer
	avail = 0;							// No data in file buffer now
	fillbuffer();						// Fill the entire buffer
	if (eof)							// Less than one segment?
		finalbuffer();
	else
		fillbuffer();


///	sndparms.rate.value = 41000;

	if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &sndparms)) < 0)
		femsg2(prgname, "Error setting sound parameters", rtn, NULL);

	svcSchSetLevel(0);

	if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &strtparms)) < 0)
		femsg2(prgname, "Error starting sound output", rtn, NULL);


	while (TRUE)
		svcSchSuspendT(NULL, -1, -1);

}



void sndsignal(
	SIGSTK stk)

{
	long rtn;

	printf("%06X  ", stk.offset);

	if (stop)
	{
		if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &stopparms)) < 0)
			femsg2(prgname, "Error stopping sound output", rtn, NULL);
		printf("Finished\n");
		exit(0);
	}
	else
	{
		if (eof)
			finalbuffer();
		else
			fillbuffer();
	}
}



void fillbuffer(void)

{
	long *spnt;
	long  rtn;
	long  segcnt;
	long  blklen;
	long  amnt;
	long  data;
	uchar blkhead[4];
	uchar snddata[4];

	segcnt = 0;
	spnt = (long *)(buffer + offset);
	do
	{
		if (avail > 0)
		{
			if ((amnt = (BUFRSZ/2 - segcnt)) > (avail * 4))
				amnt = avail * 4;
			avail -= (amnt/4);
			segcnt += amnt;
			do
			{
				data = ((int)(*blkpnt++)) - 0x80;
				data = ((data << 24) & 0xFFFF0000) + ((data << 8) & 0x0000FFFF);
				*spnt++ = data;
			} while ((amnt -= 4) > 0);
		}
		else
		{
			// Get here if our input buffer is empty

			if (blkcnt > 0)				// Anything left to read in current
			{							//   block?
				if ((amnt = blkcnt) > BLKBSZ)
					amnt = BLKBSZ;
				if ((rtn = fread(blkbufr, 1, amnt, file)) != amnt)
				{
					if (rtn >= 0)
						rtn = -ER_EOF;
					femsg2(prgname, "Error reading sound data block", -errno,
							NULL);
				}
				blkcnt -= rtn;
				avail = rtn;
				blkpnt = blkbufr;
			}
			else
			{
				// Here if there is nothing more to read in the current block -
				//   Read the header for the next block

				printf("### reading block header at 0x%X\n", filepos);

				if (fseek(file, filepos, 0) < 0)
					femsg2(prgname, "Error setting input position", -errno,
							NULL);
				if ((rtn = fread(blkhead, 1, 4, file)) != 4)
				{
					if (rtn < 1 || blkhead[0] != 0) // Normal EOF?
					{
						if (rtn >= 0)
							rtn = -ER_EOF;
						femsg2(prgname, "Error reading block header", -errno,
								NULL);
					}
				}
				blklen = (blkhead[0] == 0) ? 0 :
						*(long *)(blkhead + 1) & 0xFFFFFF;

				filepos += (blklen + 4);

				printf("Block %d: %d byte%s\n", blkhead[0], blklen,
						(blklen == 1) ? "" : "s");
				switch (blkhead[0])
				{
				 case 0:					// End of file
					fputs("  End of file\n", stdout);
					memset(spnt, 0, BUFRSZ - segcnt * 16);
					eof = TRUE;
					return;

				 case 1:					// Sound data
					if ((rtn = fread(snddata, 1, 2, file)) != 2)
					{
						if (rtn >= 0)
							rtn = -ER_EOF;
						femsg2(prgname, "Error reading sound data block",
								-errno, NULL);
					}
					sndparms.rate.value = 1000000/(256 - snddata[0]);
					printf("  Sound data: Rate=%d (%d), Compression=%d\n",
						snddata[0], sndparms.rate.value, snddata[1]);
				 case 2:					// Continued sound data
					blkcnt = blklen - 2;
					break;

				 case 3:					// Silence
					printf("  Silence: Length=%d, Rate=%d\n", 0, 0);
					break;

				 case 4:					// Marker
					printf("  Marker: Number=%d\n", 0);
					break;

				 case 5:					// ASCII
					printf("  ASCII: Text=%.*s\n", 0, "");
					break;

				 case 6:					// Repeat
					printf("  Start repeat: Count=%d\n", 0);
					break;

				 case 7:					// End repeat
					fputs("  End repeat\n", stdout);
					break;
		
				 case 8:					// Extended
					printf("  Extended info:\n");
					break;

				 default:
					fputs("  Unknown block type, block skipped", stdout);
					break;
				}
			}
		}

	} while (segcnt < (BUFRSZ/2));
	offset = (offset + BUFRSZ/2) & (BUFRSZ - 1);
}


void finalbuffer(void)

{
	memset(buffer + offset, 0, BUFRSZ/2);
	stop = TRUE;
}
