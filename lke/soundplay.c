
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <xoserrmsg.h>
#include <xos.h>
#include <xossignal.h>
#include <xossvc.h>
#include <xossnd.h>
#include <xosswap.h>



#define BUFRSZ 0x1000
#define BLKBSZ 0x2000

extern long busy;
extern char buffer[];
extern char thread[];
extern char sndregs[];
extern char prgname[];

uchar *blkpnt;
FILE  *file;
long   bufrsize;
long   fragsize;
long   offset;
long   avail;
long   filepos;
long   fileleft;
long   datapos;
long   hndl;
long   blkleft;				// Data bytes available in current block
long   amnt;
long   posreg;
long   sigcnt;

uchar  blkbufr[BLKBSZ];


struct
{	char   label[4];
	long   length;
	char   format[4];
}    header;

struct
{	ushort compress;
	ushort channels;
	long   rate;
	long   dmarate;
	ushort sampsize;
	ushort width;
}    fmtdata;

struct
{	TEXT8PARM class;
	char      end;
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
	char      end;
} sigparms =
{	{PAR_SET | REP_HEXV, 4, IOPAR_SIGVECT1, 60}
};

struct
{	TEXT4PARM class;
	BYTE4PARM volume;
	BYTE4PARM rate;
	char      end;
} sndparms =
{	{PAR_SET | REP_TEXT, 4, IOPAR_CLASS, "SND"},
	{PAR_SET | REP_HEXV, 4, IOPAR_SND_VOLUME},
	{PAR_SET | REP_HEXV, 4, IOPAR_SND_RATE}
};

struct
{	TEXT4PARM class;
	BYTE4PARM fifosz;
	char      end;
} fifoparms =
{	{PAR_SET | REP_TEXT, 4, IOPAR_CLASS, "SND"},
	{PAR_GET | REP_DECV, 4, IOPAR_SND_FIFOSZ}
};

struct
{	TEXT4PARM class;
	BYTE4PARM sndregs;
	char      end;
} regsparms =
{	{PAR_SET | REP_TEXT, 4, IOPAR_CLASS, "SND"},
	{PAR_GET | PAR_SET | REP_DECV, 4, IOPAR_SND_MAPREG}
};

struct
{	TEXT4PARM class;
	char      end;
} spcparm =
{	{PAR_SET | REP_TEXT, 4, IOPAR_CLASS, "SND"},
};

uchar chnls;
uchar width;
uchar eof;
uchar stop;

long  args[2];


void fillbuffer(void);
void finalbuffer(void);
void endmsg(long bpos);

void finish(long code, char *msg);

void sndsignal(SIGSTK stk);

long threadfunc(long  volume, char *filespec);



int soundplay(
	char *filespec,
	long  volume)
{
	long rtn;

	if ((rtn = svcMemChange(thread, 2, 0x1000)) < 0)
		finish(rtn, "Error allocating memory for the sound thread stack");

	args[0] = volume;
	args[1] = (long)filespec;

	if ((rtn = svcSchThread(0x01, (long *)(thread + 0x1000), 0, threadfunc,
			args, 2)) < 0)
		finish(rtn, "Error creating the sound thread");
	return (rtn);
}


long threadfunc(
	long  volume,
	char *filespec)
{
	long rtn;

	printf("Sound thread started\n");

	if ((file = fopen(filespec, "rb")) == NULL)
		finish(-errno, "Cannot open input file");

	if ((rtn = fread((char *)&header, 1, sizeof(header), file)) !=
			sizeof(header))
	{
		if (rtn >= 0)
			rtn = -ER_EOF;
		finish(-errno, "Error reading file header");
	}
	if (strncmp(header.label, "RIFF", 4) != 0 ||
			strncmp(header.format, "WAVE", 4) != 0)
		finish(0, "Invalid WAV file header");

	// Header block is correct. Now find the "fmt" block and remember where
	//   the first "data" block is located.

	datapos = 0;
	memset(&fmtdata, 0, sizeof(fmtdata));
	filepos = sizeof(header);
	blkleft = header.length;
	do
	{
		if (fseek(file, filepos, 0) < 0)
			finish(-errno, "Error setting input position");
		if ((rtn = fread((char *)&header, 1, 8, file)) != 8)
		{
			if (rtn >= 0)
				rtn = -ER_EOF;
			finish(-errno, "Error reading chunk header");
		}
		if (strncmp(header.label, "fmt ", 4) == 0)
		{
			if ((amnt = header.length) > sizeof(fmtdata))
				amnt = sizeof(fmtdata);

			if ((rtn = fread((char *)&fmtdata, 1, sizeof(fmtdata), file)) !=
					sizeof(fmtdata))
			{
				if (errno >= 0)
					errno = -ER_EOF;
				finish(-errno, "Error reading \"fmt\" data");
			}
			if (fmtdata.channels != 1 && fmtdata.channels != 2)
			{
				sprintf(blkbufr, "Unsupported number of channels (%d)",
						fmtdata.channels);
				finish(0, blkbufr);
			}
			chnls = fmtdata.channels;

			printf("SampSize: %d\n   Width: %d\n", fmtdata.sampsize,
					fmtdata.width);

			if (fmtdata.width == 8)
				width = 1;
			else if (fmtdata.width == 16)
				width = 2;
			else
			{
				fputs("> WPLAY: Unsupported sample format", stdout);
				exit (1);
			}
			if (fmtdata.compress != 1)
			{
				printf("? WPLAY: Unsupported compression method (%d)\n",
						fmtdata.compress);
				exit (1);
			}
		}
		else if (strncmp(header.label, "data", 4) == 0)
		{
			datapos = filepos;
			fileleft = blkleft - 4;
		}
		rtn = ((header.length + 9) & 0xFFFFFFFE);
		filepos += rtn;
		blkleft -= rtn;
	} while (blkleft > 8 && (datapos == 0 || fmtdata.channels == 0));

	if (fmtdata.channels == 0)
		finish(0, "No \"fmt\" block found in file");
	if (datapos == 0)
		finish(0, "No \"data\" block found in file");
	printf("    Rate: %d\n", fmtdata.rate);			

	sndparms.rate.value = 44100;

	if ((rtn = svcMemChange(buffer, 2, BUFRSZ * 2)) < 0)
		finish(rtn, "Cannot allocate buffer");

	if ((hndl = svcIoOpen(0, "SND0:", NULL)) < 0)
		finish(hndl, "Cannot open sound device");

	if (setvector(60, 3, sndsignal) < 0)
		finish(-errno, "Cannot set signal vector");

	if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &sigparms)) < 0)
		finish(rtn, "Cannot specify signal vector");

	sndparms.volume.value = volume;
	printf("  Volume: %d\n", sndparms.volume.value);
	sndparms.volume.value |= (sndparms.volume.value << 8);

	filepos = datapos;

	offset = 0;							// Start at beginning of the buffer
	avail = 0;							// No data in file buffer now

	sndparms.rate.value = fmtdata.rate;

///	sndparms.rate.value = 44100;

	// Set final parameters

	if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &sndparms)) < 0)
		finish(rtn, "Error setting sound parameters");

	// Set up the buffer

	if ((rtn = svcIoSpecial(hndl, SDF_SND_SETBUFR, buffer, BUFRSZ,
			&spcparm)) < 0)
		finish(rtn, "Error setting buffer address");

	printf("Buffer set up\n");

	svcSchSetLevel(0);

	// Set up a stream

	if ((rtn = svcIoSpecial(hndl, SDF_SND_OUTSTRM, NULL, 0, &spcparm)) < 0)
		finish(rtn, "Error setting up sound stream");

	printf("Sound stream set up\n");

	if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &fifoparms)) < 0)
		finish(rtn, "Error getting FIFO size");
	printf("FIFO size is %d\n", fifoparms.fifosz.value);

	regsparms.sndregs.value = (long)sndregs;
	if ((rtn = svcIoOutBlockP(hndl, NULL, 0, &regsparms)) < 0)
		finish(rtn, "Error mapping sound device registers");
	posreg = regsparms.sndregs.value;
	printf("Position register is at %X\n", posreg);

	fillbuffer();						// Fill the entire buffer
	if (eof)							// Less than one segment?
		finalbuffer();
	else
		fillbuffer();

	printf("Initial buffer segments filled\n");

	// Start sound output

	printf("Starting sound output\n");

	sigcnt = 0;
	if ((rtn = svcIoSpecial(hndl, SDF_SND_START, NULL, 0, &spcparm)) < 0)
		finish(rtn, "Error starting sound output");

	while (TRUE)
		svcSchSuspend(NULL, -1);
}



void sndsignal(
	SIGSTK stk)
{
	long rtn;
	long value;
	int  chr;

	if (stk.offset == -1)
		endmsg(*(ushort *)(sndregs + posreg));
	if (stk.errcode < 0)
		finish(stk.errcode, "Error reported in signal");
	value = *(ushort *)(sndregs + posreg);
	if (stop)
	{
		if ((rtn = svcIoSpecial(hndl, SDF_SND_STOP, NULL, 0, &spcparm)) < 0)
			finish(rtn, "Error stopping sound output");
		printf("%04X-%04X-%04X!%s", stk.offset, value, *(ushort *)(sndregs +
			posreg), (++sigcnt & 0x03) ? " " : "\n");
		stop = FALSE;
	}
	else
	{
		if (eof)
		{
			finalbuffer();
			chr = '#';
		}
		else
		{
			fillbuffer();
			chr = ' ';
		}
		printf("%04X-%04X-%04X%c%s", stk.offset, value, *(ushort *)(sndregs +
				posreg), chr, (++sigcnt & 0x03) ? " " : "\n");
	}
}


void endmsg(
	long bpos)
{
	char bufr[32];

	sprintf(bufr, "END -%04X\nSound output is complete", bpos);
	finish(1, bufr);
}




void fillbuffer(void)
{
	union
	{	uchar  *c;
		ushort *s;
		long   *l;
	}    spnt;
	long rtn;
	long segcnt;
	long data;
	union
	{	uchar  c;
		ushort s;
		long   l;
	}    value;


	segcnt = 0;
	spnt.c = buffer + offset;
	do
	{
		// avail = Number of bytes available from the input buffer (this
		//			 is independent of sample size.
		// amnt  = Number of bytes available to transfer (either the
		//			 maximum number that will fit or the number available,
		//			 whichever is less). This is relative to the buffer
		//           segment, that is, it is the size after the sample has
		//           been expanded to 16-bit stereo.
		// The samples stored in the buffer segment are always 4 bytes (stereo,
		//   with 2 bytes per channel). The samples in the input buffer can
		//   be 1 byte (8-bit mono), 2 bytes (8-bit stereo or 16-bit mono), or
		//   4 bytes (16-bit stereo).

		if (avail > 0)
		{
			if (chnls == 2)
			{
				if (width == 2)
				{
					// Here if have 16-bit stereo data (4 bytes) - In this
					//   case we just copy samples without conversion.

					if ((amnt = (BUFRSZ - segcnt)) > avail)
						amnt = avail;
					avail -= amnt;
					segcnt += amnt;
					do
					{
						data = *(long *)blkpnt;
						blkpnt += 4;
						*spnt.l++ = data;
					} while ((amnt -= 4) > 0);
				}
				else
				{
					// Here if have 8-bit stereo data (2 bytes)

					if ((amnt = (BUFRSZ - segcnt)) > (avail * 2))
						amnt = avail * 2;
					avail -= (amnt/2);
					segcnt += amnt;
					do
					{
						data = *(short *)blkpnt;
						blkpnt += 2;
						*spnt.l++ = ((data & 0xFF00) << 16) +
								((data & 0x00FF) << 8);
					} while ((amnt -= 4) > 0);
				}
			}
			else
			{
				if (width == 2)
				{
					// Here if have 16-bit mono data (2 bytes per sample)

					if ((amnt = (BUFRSZ - segcnt)) > (avail * 2))
						amnt = avail * 2;
					avail -= (amnt/2);
					segcnt += amnt;
					do
					{
						value.s = *(short *)blkpnt;
						blkpnt += 2;
						*spnt.l++ = (value.s << 16) + value.s;
					} while ((amnt -= 4) > 0);
				}
				else
				{
					// Here if have 8-bit mono data (1 byte per sample)

					if ((amnt = (BUFRSZ - segcnt)) > (avail * 4 * 4))
						amnt = avail * 4 * 4;
					avail -= (amnt/(4 * 4));
					segcnt += amnt;
					do
					{
						value.s = (*blkpnt++ ^ 0x80) << 8;
						value.l = (value.s << 16) + value.s;
						*spnt.l++ = value.l;
						*spnt.l++ = value.l;
						*spnt.l++ = value.l;
						*spnt.l++ = value.l;
					} while ((amnt -= 16) > 0);
				}
			}
		}
		else
		{
			// Get here if our input buffer is empty

			if (blkleft > 0)			// Anything left to read in current
			{							//   block?
				if ((amnt = blkleft) > BLKBSZ)
					amnt = BLKBSZ;
				if ((rtn = fread(blkbufr, 1, amnt, file)) != amnt)
				{
					if (rtn >= 0)
						rtn = -ER_EOF;
					finish(-errno, "Error reading sound data block");
				}
				blkleft -= rtn;
				avail = rtn;
				blkpnt = blkbufr;
			}
			else
			{
				// Here if there is nothing more to read in the current block -
				//   Read the header for the next block

				if (fileleft < 8)
				{
					if (fileleft != 0)
						printf("Have %d excess bytes at end of file\n",
								fileleft, (fileleft == 1) ? "" : "s");
					memset(spnt.c, 0, BUFRSZ - segcnt);
					eof = TRUE;
					break;
				}
				printf("\nReading block header at 0x%X, %d left\n", filepos,
						fileleft);
				if (fseek(file, filepos, 0) < 0)
					finish(-errno, "Error setting input position");
				if ((rtn = fread((char *)&header, 1, 8, file)) != 8)
				{
					if (rtn >= 0)
						rtn = -ER_EOF;
					finish(-errno, "Error reading block header");
				}
				rtn = ((header.length + 9) & 0xFFFFFFFE);
				filepos += rtn;
				fileleft -= rtn;

				printf("Block: \"%.4s\": %d byte%s\n", header.label,
						header.length, (header.length == 1) ? "" : "s");

				if (strncmp(header.label, "data", 4) == 0)
					blkleft = header.length;
			}
		}

	} while (segcnt < (BUFRSZ));
	offset = (offset + BUFRSZ) & ((BUFRSZ * 2) - 1);
}


void finalbuffer(void)

{
	memset(buffer + offset, 0, BUFRSZ);
	stop = TRUE;
}


void finish(
	long  code,
	char *msg)
{
	if (code <= 0)
		errormsg(code, msg);
	else
		printf("%s\n", msg);
	fputs("Sound thread is terminating\n", stdout);
	busy = 0;
	svcSchWake(0);					// Wake up the base thread
	svcSchExit(0);
}

