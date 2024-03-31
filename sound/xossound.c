#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <errmsg.h>
#include <xos.h>
#include <xossignal.h>
#include <xossvc.h>
#include <xosfibers.h>
#include <xossnd.h>
#include <xosswap.h>
#include <xosxws.h>
#include <xossound.h>

// These functions implement support for outputting sound using the standard
//   XOS sound device from an uncompressed WAV file.

// NOTE: Direct unbuffered IO is used to read the WAV file. While this results
//       in additional system calls when reading the initial header blocks it
//       avoids an extra copy of the large data blocks. When the amount read
//       is significantly larger than the size of the data buffer, buffered
//       IO is actually slower than unbuffered IO. The header blocks are read
//       before sound output is started so we are not as worried about speed
//       with them. The data blocks are read during sound output so any
//       reduction in overhead is significant.

#define DEBUG 0

volatile long sndActive;

static long   factor;
static long   factorx2;
static long   factorx4;

static char  *bufrmem;			// Address of sound output buffer
static long   bufrsize;			// Size of one buffer segment
static char  *inpmem;			// Address of IO input buffer
static long   inpsize;			// Size of the IO input buffer

static char  *filespec;
static long   volume;
static long   notcnt;
static long   freq;
static long   time;
static long   sndvect;

static volatile long   sndidle;
static volatile long   sndfunc;
static volatile long   fbrhndl;
static volatile long   fbrstop;

static volatile long   sndtid;	// TID of the sound thread
static volatile long   rtnderr;
static volatile char  *rtndmsg;

static char  *blkpnt;
static long   offset;
static long   avail;
static long   fileleft;
static long   datapos;
static long   sndhndl;
static long   filehndl;
static long   blkleft;			// Data bytes available in current block
static long   amnt;
static long   nottotal;
static long   notintrv;
static long   notnext;
static long   notdone;
static long   notstep;

static volatile long   toneval;
static volatile long   toneinc;
static volatile long   tonemax;
static volatile long   toneleft;

static volatile long   notify;	// Fiber handle for notification fiber

static volatile long   notready;

static void (*snderror)(long code, char *msg);

#if DEBUG
static int  volatile posreg;
static int  volatile sigcnt;
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
#endif


typedef struct
{	long reqrate;
	long avlrate;
	long factor;
} RATETBL;

static struct
{	char   label[4];
	long   length;
	char   format[4];
}    header;

static struct
{	ushort compress;
	ushort channels;
	long   rate;
	long   dmarate;
	ushort sampsize;
	ushort width;
}    fmtdata;

///static struct
///{	BYTE4PARM length;
///	uchar     end;
///} openparm =
///{	{PAR_GET|REP_DECV, 4, IOPAR_LENGTH}
///};


static struct
{	BYTE4PARM pos;
	uchar     end;
} posparm =
{	{PAR_SET|REP_DECV, 4, IOPAR_ABSPOS}
};

typedef struct
{	long  xxx[15];
	short datacnt;
	short signum;
	long  dataval;
	long  errcode;
	long  offset;
} SIGSTK;

static struct
{	BYTE4PARM vect;
	uchar     end;
} sigparms =
{	{PAR_SET | REP_HEXV, 4, IOPAR_SIGVECT1}
};

static struct
{	TEXT4PARM class;
	BYTE4PARM volume;
	BYTE4PARM rate;
	uchar     end;
} sndparms =
{	{PAR_SET|REP_TEXT        , 4, IOPAR_CLASS, "SND"},
	{PAR_SET|REP_HEXV        , 4, IOPAR_SND_VOLUME},
	{PAR_SET|PAR_GET|REP_HEXV, 4, IOPAR_SND_RATE}
};

static struct
{	TEXT4PARM class;
	uchar     end;
} spcparm =
{	{PAR_SET | REP_TEXT, 4, IOPAR_CLASS, "SND"},
};

static char chnls;
static char width;
static volatile char eof;
static volatile char stop;

static int  dosound(int func);
static int  fillbuffer(void);
static int  finalbuffer(void);
static int  stopsound(void);
static int  setuptone(void);
static int  setupwav(void);
static void sndsignal(SIGSTK stk);
///static void snddonethread(void);
static void sndfail(long code, int init, char *msg);
static void thrderror(long code, char *msg);
static long threadfunc(void);


//*********************************************************
// Function: sndInitalize - Initialize the sound routines
// Returned: TRUE if normal, FALSE if error (snderrorarg is
//				called before giving an error return.)
//*********************************************************

// This function must be called in a fibers environment. It creates a thread
//   which is used for actually doing sound IO. All of the initialization
//   is done in the calling fiber. It is assumed this function will be called
//   once during program start-up and that it is most important to ensure
//   that the initialization is complete before this function returns. All
//   of the buffers used must be allocated by the caller before calling this
//   function.

int sndInitalize(
	long  *stkmem,				// Address for the top of the thread's stack
	char  *bufrmemarg,			// Address of sound output buffer
	long   bufrsizearg,			// Size of one buffer segment
	char  *inpmemarg,			// Address of IO input buffer
	long   inpsizearg,			// Size of the IO input buffer
	long   notifyarg,			// Fiber to wake up for notifications
	long   sndvectarg,			// Vector for signal
	void (*snderrorarg)(long code, char *msg))
{
	long rtn;

	bufrmem = bufrmemarg;
	bufrsize = bufrsizearg;
	inpmem = inpmemarg;
	inpsize = inpsizearg;
	notify = notifyarg;
	sndvect = sndvectarg;
	snderror = snderrorarg;

	// Open the sound device

	if ((sndhndl = fbrIoOpen(0, "SND0:", NULL)) < 0)
	{
		snderror(sndhndl, "Cannot open sound device");
		return (FALSE);
	}

	// Tell the sound device about the signal vector

	sigparms.vect.value = sndvect;
	if ((rtn = fbrIoOutBlockP(sndhndl, NULL, 0, &sigparms)) < 0)
	{
		snderror(rtn, "Cannot specify signal vector");
		fbrIoClose(sndhndl, 0);
		sndhndl = 0;
		return (FALSE);
	}

	// Give the buffer address to the sound device

	if ((rtn = fbrIoSpecial(sndhndl, SDF_SND_SETBUFR, bufrmem, bufrsize,
			&spcparm)) < 0)
	{
		snderror(rtn, "Error setting buffer address");
		return (FALSE);
	}

	// Set up the output stream

	if ((rtn = fbrIoSpecial(sndhndl, SDF_SND_OUTSTRM, NULL, 0, &spcparm)) < 0)
	{
		snderror(rtn, "Error setting up output stream");
		return (FALSE);
	}

	// Create the sound thread

	fbrhndl = (long)fbrData;
	notready = TRUE;
	if ((sndtid = svcSchThread(0x01, stkmem, 0, threadfunc, NULL, 0)) < 0)
	{
		snderror(sndtid, "Error creating the sound thread");
		sndtid = 0;
		return (FALSE);
	}
	while (notready)					// Wait until the sound thread is ready
		fbrCtlSuspend((long *)&notready, -1);
#if DEBUG
	fputs("Thread startup complete\n", stdout);
#endif
	return (TRUE);
}


//****************************************************************
// Function: sndPlay - Output a .WAV file to the sound device
// Returned: TRUE if normal, FALSE if error (The snderror function
//				specified when soundinit was called is called
//				before giving an error return.)
//****************************************************************

// This function must be called in a fibers environment. It is a blocking
//   call that does not return until the .WAV file has been completely
//   output. It is compatable with an XWS graphics mode environment.

// All of the work for this function is done in the sound thread. This
//   function simply stores the parameters so they are available to that
//   thread, pokes it, and waits until it is finished. If the error function
//   is called it is called in the fiber which called this function.

int sndPlay(
	char *filespecarg,			// File specification
	long  volumearg,			// Volume
	long  notcntarg,			// Number of notifications desired
	long  notifyarg)			// Fiber to wake up for notifications
{
	fbrhndl = (long)fbrData;
	filespec = filespecarg;
	volume = volumearg;
	notcnt = notcntarg;
	notify = notifyarg;
	sndActive = TRUE;
	return(dosound(1));
}


//*************************************************************************
// Function: soundtone - Generate a tone for the specified length of time
// Returned: TRUE if normal, FALSE if error (The snderror function
//				specified when soundinit was called is called before
//				giving an error return.)
//*************************************************************************

// Tones are always generated using a sample rate of 48KHz. The waveform is
//   trianglar.

int sndTone(
	long freqarg,
	long timearg,
	long volumearg,
	long notcntarg,
	long notifyarg)
{
	fbrhndl = (long)fbrData;
	freq = freqarg;
	time = timearg;
	volume = volumearg;
	notcnt = notcntarg;
	notify = notifyarg;
	sndActive = TRUE;
	return(dosound(2));
}


//****************************************
// Function: soundstop - Stop sound output
// Returned: Nothing
//****************************************

// Must be called from a fiber.

int sndStop(void)
{
	fbrstop = (long)fbrData;
	if (sndActive)
		return (dosound(3));
	thrderror(ER_NACT, "Sound output is not active");
	return (FALSE);
}


//******************************************************************
// Function: dosound - Wake up sound thread, wait unit it's finished
//				and do final cleanup
// Returned: TRUE if normal, FALSE if error (The snderror function
//				specified when soundinit was called is called
//				before giving an error return.)
//******************************************************************

static int dosound(
	int func)
{
	sndfunc = func;
	sndidle = FALSE;
	svcSchWake(sndtid);
	while (sndActive)
		fbrCtlSuspend((long *)&sndActive, -1);

	if (rtndmsg != NULL)
	{
		snderror((long)rtnderr, (char *)rtndmsg);
		rtnderr = 0;
		rtndmsg = NULL;
		return (FALSE);
	}
	return (TRUE);
}	


//=====================================================
// Everything following is executed in the sound thread
//=====================================================

// NOTE: Since a part of the C run-time library is NOT thread-safe the code
//       running in the sound thread cannot use the library routines that are
//       not thread-safe, partictually malloc and the entire IO system! While
//       setvector is technically not thread-safe, we use it anyway since
//       it is only called once at start-up and there should not be a confict
//       with this. The str functions are thread-safe.


//*****************************************************************
// Function: threadfunc - Thread function for the sound thread
// Returned: Never returns (thread terminates by calling svcSchExit
//*****************************************************************

static long threadfunc(void)
{
	static int (*snddisp[])(void) =
	{	setupwav,
		setuptone,
		stopsound,
	};

	long rtn;

#if DEBUG
	printf("Sound thread started\n");
#endif

	sndidle = TRUE;

	// Set up the signal vector

	if (setvector(sndvect, 5, sndsignal) < 0)
		thrderror(-errno, "Cannot set signal vector for sound thread");

	notready = FALSE;
	fbrCtlWake(fbrhndl);

	svcSchSetLevel(0);

	while (TRUE)
	{
		while (sndidle)					// Wait until we have something to do
			svcSchSuspend((long *)&sndidle, -1);
		sndidle = TRUE;
#if DEBUG
		printf("have function %d\n", sndfunc);
#endif
		if (!snddisp[sndfunc - 1]())	// Do set up
			continue;

		// Set final parameters

		sndparms.volume.value = volume;
		sndparms.rate.value = fmtdata.rate;
#if DEBUG
		printf("requested rate is %d\n", sndparms.rate.value);
#endif
		if ((rtn = svcIoOutBlockP(sndhndl, NULL, 0, &sndparms)) < 0)
		{
			thrderror(rtn, "Error setting sound parameters");
			continue;
		}
#if DEBUG
		printf("returned rate is %d\n", sndparms.rate.value);
#endif

		offset = 0;						// Start at beginning of the buffer
		avail = 0;						// No data in file buffer now
		factor = 1;
		if (fmtdata.rate < 44100 && sndparms.rate.value >= 44100)
		{
			// Here if we need to upsample in software (Some sound devices
			//   can't handle anything slower than 44.1KB!)

			factor = sndparms.rate.value / fmtdata.rate;
		}

		// Store some values we will need during output

		factorx2 = factor * 2;
		factorx4 = factor * 4;
		notstep = bufrsize/(factorx2 * (width ^ 0x03) * (chnls ^ 0x03));

		// Fill the first two buffer segments

		offset = 0;						// Start at beginning of the buffer
		avail = 0;						// No data in file buffer now
		blkleft = 0;
		eof = FALSE;
		stop = FALSE;

#if DEBUG
		printf("rate factor is %d\n", factor);
		if ((rtn = svcIoOutBlockP(sndhndl, NULL, 0, &fifoparms)) < 0)
		{
			thrderror(rtn, "Error getting sound FIFO size");
			continue;
		}
		printf("FIFO size is %d\n", fifoparms.fifosz.value);
		regsparms.sndregs.value = 0x70000000;
		if ((rtn = svcIoOutBlockP(sndhndl, NULL, 0, &regsparms)) < 0)
		{
			thrderror(rtn, "Error mapping sound device registers");
			continue;
		}
		posreg = regsparms.sndregs.value;
		printf("Position register is at %X\n", posreg);
		fputs("First value is buffer offset from the signal\n", stdout);
		fputs("Second value is buffer offset at start of signal function\n",
				stdout);
		fputs("Third value is buffer offset at end of signal function\n",
				stdout);
		fputs("Fourth value is segment number (should alternate between 0 "
				"and 1)\n", stdout);
#endif

		// Fill the entire buffer

		if (!fillbuffer())					// Fill the first segment
			continue;
		if (!((eof) ? finalbuffer() : fillbuffer())) // Fill the second segment
			continue;
#if DEBUG
		printf("Initial buffer segments filled\n");
#endif

		// Start sound output

#if DEBUG
		printf("Starting sound output at %04X\n", *(ushort *)(0x70000000 +
				posreg));
		sigcnt = 0;
#endif

		if ((rtn = svcIoSpecial(sndhndl, SDF_SND_START, NULL, 0, &spcparm)) < 0)
		{
			thrderror(rtn, "Error starting sound output");
			continue;
		}
#if DEBUG
		printf("After starting offset is %04X\n", *(ushort *)(0x70000000 +
				posreg));
		sigcnt = 0;
#endif

///		while (sndplaying)
///			svcSchSuspend(&snddone, -1);
	}
}


//************************************************
// Function: setupwav - Set up to play a .WAV file
// Returned: Nothing
//************************************************

static int setupwav(void)
{
	long rtn;
	char text[128];

	// Open the file

	if ((filehndl = svcIoOpen(XO_IN, filespec, NULL)) < 0)
	{
		INT3;

		snprintf(text, sizeof(text) - 1, "Cannot open sound input WAV file %s",
				filespec);
		thrderror(filehndl, text);
		return (FALSE);
	}

	// Read and check the first header block

	if ((rtn = svcIoInBlock(filehndl, (char *)&header, sizeof(header))) !=
			sizeof(header))
	{
		if (rtn >= 0)
			rtn = ER_EOF;
		thrderror(rtn, "Error reading WAV file header");
		return (FALSE);
	}
	if (strncmp(header.label, "RIFF", 4) != 0 ||
			strncmp(header.format, "WAVE", 4) != 0)
	{
		thrderror(0, "Invalid WAV file header");
		return (FALSE);
	}

	// Header block is correct. Now find the "fmt" block and remember where
	//   the first "data" block is located.

	datapos = 0;
	memset(&fmtdata, 0, sizeof(fmtdata));
	posparm.pos.value = sizeof(header);
	blkleft = header.length;
	do
	{
		if ((rtn = svcIoInBlockP(filehndl, (char *)&header, 8, &posparm)) != 8)
		{
			if (rtn >= 0)
				rtn = ER_EOF;
			thrderror(rtn, "Error reading WAV file chunk header");
			return (FALSE);
		}
		if (strncmp(header.label, "fmt ", 4) == 0)
		{
			if ((amnt = header.length) > sizeof(fmtdata))
				amnt = sizeof(fmtdata);

			if ((rtn = svcIoInBlock(filehndl, (char *)&fmtdata,
					sizeof(fmtdata))) != sizeof(fmtdata))
			{
				if (rtn >= 0)
					rtn = ER_EOF;
				thrderror(rtn, "Error reading WAV file \"fmt\" data");
				return (FALSE);
			}
			if (fmtdata.channels != 1 && fmtdata.channels != 2)
			{
				sprintf(inpmem, "Unsupported number of channels (%d) in WAV "
						"file", fmtdata.channels);
				thrderror(0, inpmem);
				return (FALSE);
			}
			chnls = fmtdata.channels;

#if DEBUG
			printf("SampSize: %d\n   Width: %d\n", fmtdata.sampsize,
					fmtdata.width);
#endif
			if (fmtdata.width == 8)
				width = 1;
			else if (fmtdata.width == 16)
				width = 2;
			else
			{
				thrderror(0, "Unsupported sample format in WAV file");
				return (FALSE);
			}
			if (fmtdata.compress != 1)
			{
				thrderror(0, "Unsupported compression method in WAV file");
				return (FALSE);
			}
		}
		else if (strncmp(header.label, "data", 4) == 0)
		{
			datapos = posparm.pos.value;
			fileleft = blkleft - 4;
		}
		rtn = ((header.length + 9) & 0xFFFFFFFE);
		posparm.pos.value += rtn;
		blkleft -= rtn;
	} while (blkleft > 8 && (datapos == 0 || fmtdata.channels == 0));

	// Here with the FMT and first DATA blocks located - set up the stream's
	//   parameters

	if (fmtdata.channels == 0)
	{
		thrderror(0, "No \"fmt\" block found in WAV file");
		return (FALSE);
	}
	if (datapos == 0)
	{
		thrderror(0, "No \"data\" block found in WAV file");
		return (FALSE);
	}
#if DEBUG
	printf("    Rate: %d\n", fmtdata.rate);			
	printf("  Volume: %d/%d\n", ((ulong)volume} >> 16, (ushort)volume);
#endif

	tonemax = 0;
	posparm.pos.value = datapos;

	return (TRUE);
}


//********************************************
// Function: setuptone - Set up to play a tone
// Returned: Nothing
//********************************************

static int setuptone(void)
{
	long qspc;

	if (sndhndl <= 0)
		return (FALSE);
	qspc = (12000 / freq) & ~0x01;		// Determine the number of samples
										//   per quarter cycle and make sure
										//   it's even.
	toneinc = 0x4000 / qspc;			// Get amount to change each sample
	tonemax = toneinc * qspc;			// Get the exact maximum value
	toneleft = (4 * qspc * freq * time) / 1000; // Get total number of samples
	toneval = 0;
	nottotal = toneleft;
	notintrv = notnext = (notcnt > 0) ? ((nottotal + notcnt - 1)/notcnt) : 0;
	notdone = 0;
	notstep = bufrsize / 4;
	fmtdata.rate = 44100;
	return (TRUE);
}


//**********************************************************
// Function: thrderror - Report an error in the sound thread
// Returned: nothing
//**********************************************************

// This function is called on any error in the sound thread function or in
//   the sound signal function. 

static void thrderror(
	long  code,
	char *msg)
{
	if (filehndl > 0)
		svcIoClose(filehndl, 0);
	filehndl = 0;
	rtnderr = code;
	rtndmsg = msg;
	sndActive = FALSE;
	fbrCtlWake(fbrhndl);
}


//*******************************************************
// Function: sndsignal - Signal function for sound output
// Returned: Nothing
//*******************************************************

// Called in the sound thread

void sndsignal(
	SIGSTK stk)
{
	long rtn;
#if DEBUG
	long curoffset;
	long value;
#endif
	int  chr;

#if DEBUG
	value = *(ushort *)(0x70000000 + posreg);
#endif
	if (stk.offset == -1)
	{
#if DEBUG
		printf("END -%04X\nSound output is complete\n",
				*(ushort *)(0x70000000 + posreg));
#endif
		if (filehndl > 0)
			svcIoClose(filehndl, 0);
		filehndl = 0;
		sndActive = FALSE;
		fbrCtlWake(fbrhndl);
		return;
	}
	if (stk.errcode < 0)
	{
		thrderror(stk.errcode, "Error reported in sound signal");
		return;
	}
	if (stop)
	{
		if ((rtn = svcIoSpecial(sndhndl, SDF_SND_STOP, NULL, 0, &spcparm)) < 0)
		{
			thrderror(rtn, "Error stopping sound output");
			return;
		}
#if DEBUG
		printf("%04X-%04X-%04X-%d!%s", stk.offset, value,
				*(ushort *)(0x70000000 + posreg), offset >> 13,
				(++sigcnt & 0x03) ? " " : "\n");
#endif
		stop = FALSE;
	}
	else
	{
#if DEBUG
		curoffset = offset;
#endif
		if (eof)
		{
			rtn = finalbuffer();
			chr = '#';
		}
		else
		{
			rtn = fillbuffer();
			chr = ' ';
		}
#if DEBUG
		printf("%04X-%04X-%04X-%d%c%s", stk.offset,
				value, *(ushort *)(0x70000000 + posreg), curoffset >> 13, chr,
				(++sigcnt & 0x03) ? " " : "\n");
#endif
		if (rtn < 0)
		{
			thrderror(rtn, "Error reading WAV file");
			return;
		}
	}
}


//*******************************************************************
// Function: fillbuffer - Fill a sound buffer segment with sound data
// Returned: TRUE if normal, FALSE if error
//*******************************************************************

static int fillbuffer(void)
{
	union
	{	char   *c;
		ushort *s;
		long   *l;
	}    spnt;
	long cnt;
	long rtn;
	long segcnt;
	long data;
	union
	{	char   c;
		ushort s;
		long   l;
	}    value;

	spnt.c = bufrmem + offset;
	if (tonemax != 0)
	{
		segcnt = bufrsize / 4;
		if ((amnt = toneleft) > segcnt)
			amnt = segcnt;
		toneleft -= amnt;
		do
		{
			if (--amnt >= 0)
			{
				spnt.s[0] = toneval;
				spnt.s[1] = toneval;
				spnt.l++;
				toneval += toneinc;
				if (toneval >= tonemax || toneval <= -tonemax)
					toneinc = -toneinc;
			}
			else
				*spnt.l++ = 0;
		} while (--segcnt > 0);
		if (toneleft <= 0)
			eof = TRUE;
	}
	else
	{
		segcnt = 0;
		do
		{
			// avail = Number of bytes available from the input buffer (this
			//			 is independent of sample size.
			// amnt  = Number of bytes available to transfer (either the
			//			 maximum number that will fit or the number available,
			//			 whichever is less). This is relative to the buffer
			//           segment, that is, it is the size after the sample has
			//           been expanded to 16-bit stereo.
			// The samples stored in the buffer segment are always 4 bytes
			//   (stero, with 2 bytes per channel). The samples in the input
			//   buffer can be 1 byte (8-bit mono), 2 bytes (8-bit stereo or
			//   16-bit mono), or 4 bytes (16-bit stereo).

			if (avail > 0)
			{
				if (chnls == 2)
				{
					if (width == 2)
					{
						// Here if have 16-bit stereo data (4 bytes) - In this
						//   case we just copy samples without conversion.

						if ((amnt = (bufrsize - segcnt)) > (avail * factor))
							amnt = avail * factor;
						avail -= amnt/factor;
						segcnt += amnt;
						do
						{
							data = *(long *)blkpnt;
							blkpnt += 4;
							cnt = factor;
							do
							{
								*spnt.l++ = data;
							} while (--cnt > 0);
						} while ((amnt -= factorx4) > 0);
					}
					else
					{
						// Here if have 8-bit stereo data (2 bytes)

						if ((amnt = (bufrsize - segcnt)) > (avail * factorx2))
							amnt = avail * factorx2;
						avail -= (amnt/factorx2);
						segcnt += amnt;
						do
						{
							data = *(short *)blkpnt;
							blkpnt += 2;
							cnt = factor;
							do	
							{
								*spnt.l++ = ((data & 0xFF00) << 16) +
										((data & 0x00FF) << 8);
							} while (--cnt > 0);
						} while ((amnt -= factorx4) > 0);
					}
				}
				else
				{
					if (width == 2)
					{
						// Here if have 16-bit mono data (2 bytes per sample)

						if ((amnt = (bufrsize - segcnt)) > (avail * factorx2))
							amnt = avail * factorx2;
						avail -= (amnt/factorx2);
						segcnt += amnt;
						do
						{
							value.l = *(ushort *)blkpnt;
							blkpnt += 2;
							cnt = factor;
							do
							{
								*spnt.l++ = (value.l << 16) + value.l;
							} while (--cnt > 0);
						} while ((amnt -= factorx4) > 0);
					}
					else
					{
						// Here if have 8-bit mono data (1 byte per sample)

						if ((amnt = (bufrsize - segcnt)) > (avail * factorx4))
							amnt = avail * factorx4;
						avail -= (amnt/(factorx4));
						segcnt += amnt;
						do
						{
							value.s = (*blkpnt++ ^ 0x80) << 8;
							value.l = (value.s << 16) + value.s;
							cnt = factor;
							do
							{
								*spnt.l++ = value.l;
							} while (--cnt > 0);
						} while ((amnt -= factorx4) > 0);
					}
				}
			}
			else
			{
				// Get here if our input buffer is empty

				if (blkleft > 0)			// Anything left to read in the
				{							//   current block
					if ((amnt = blkleft) > inpsize) // Yes - read as much as
						amnt = inpsize;				//   we can
					if ((rtn = svcIoInBlock(filehndl, inpmem, amnt)) != amnt)
					{
						if (rtn >= 0)
							rtn = -ER_EOF;
						thrderror(rtn, "Error reading WAV file data block");
						return (FALSE);
					}
					blkleft -= rtn;
					avail = rtn;
					blkpnt = inpmem;
				}
				else
				{
					// Here if there is nothing more to read in the current
					//   data block - read the header for the next block

					if (fileleft < 8)
					{
						memset(spnt.c, 0, bufrsize - segcnt);
						eof = TRUE;
						break;
					}
#if DEBUG
					printf("\nReading block header at 0x%X, %d left\n", 
							posparm.pos.value, fileleft);
#endif
					if ((rtn = svcIoInBlockP(filehndl, (char *)&header, 8,
							&posparm)) != 8)
					{
						if (rtn >= 0)
							rtn = -ER_EOF;
						thrderror(rtn, "Error reading WAV file block header");
						return (FALSE);
					}
					rtn = ((header.length + 9) & 0xFFFFFFFE);
					posparm.pos.value += rtn;
					fileleft -= rtn;
#if DEBUG
					printf("Block: \"%.4s\": %d byte%s\n", header.label,
							header.length, (header.length == 1) ? "" : "s");
#endif
					if (strncmp(header.label, "data", 4) == 0)
						blkleft = header.length;
				}
			}

		} while (segcnt < bufrsize);
	}
	offset = (offset + bufrsize) & (bufrsize * 2 - 1);
	return (TRUE);
}


//***********************************************************
// Function: finalbuffer - Fill the final buffer with silence
// Returned: TRUE
//***********************************************************

static int finalbuffer(void)
{
	memset(bufrmem + offset, 0, bufrsize);
	stop = TRUE;
	return (TRUE);
}


//****************************************************
// Function: stopsound - Stop sound output immediately
// Returned: FALSE
//****************************************************


static int stopsound(void)
{
	long rtn;

	if ((rtn = svcIoSpecial(sndhndl, SDF_SND_STOP, NULL, 0, &spcparm)) < 0)
		thrderror(rtn, "Error stopping sound output");
#if DEBUG
	printf("STOP------%04X-%d!%s", *(ushort *)(0x70000000 + posreg),
			offset >> 13, (++sigcnt & 0x03) ? " " : "\n");
#endif
	sndActive = FALSE;
	fbrCtlWake(fbrstop);				// Wake up the fiber which is stopping
										//   sound (The fiber doing output
										//   will be woke up by the final
										//   signal.)
	return (FALSE);
}
