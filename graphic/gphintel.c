/************************************************************/
/* gphintel - Mainprogram                                   */
/* Graphic mode support symbiont for Intel displays for XOS */
/************************************************************/
/* Written by John Goltz                                    */
/************************************************************/

// ++++
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

// This is a user mode symbiont that is used to switch the console display
//   into and out of graphic modes. This version only supports "high color"
//   mode (16 bits (RGB 5-6-5) per pixed). It only supports graphic mode
//   set up. There is no support here for any kind of video acceration.

// Notes on programming the Intel graphics controllers:

// 1) These are very complex devices with many order dependencies.
// 2) There are a large number of versions of these devices and even more names
//    used to refer to these versions. The naming is extremely confusing and
//    is not very well documented.
// 3) The different versions are "largely" compatable, but there are significant
//    differences.
// 4) Enabling and disabling a video stream is fairly complex and MUST follow
//    the order used here. Many stream parameters cannot be changed unless the
//    stream is disabled with the associated clock (DPLL) running . (A stream
//    is a combination of one or more display surfaces, a display pipe, and
//    one or more ports.)
// 5) The (auguably) least well documented order dependency is the proceedure
//    for updating the stride value. The value written to the STRIDE register
//    is not transfered to the active register until the OFFSET register is
//    also written. This appears to work only when the stream is fully active
//    (except for actual video output). Most other timing parameters are
//    transfered when the BASE register is written or at the next vblank event.
// 6) The proceedure for switching into and out of VGA mode is fairly well
//    documented but the description is scattered throught the documents.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <xcstring.h>
#include <utility.h>
#include <xos.h>
#include <xossvc.h>
#include <xostrm.h>
#include <xossignal.h>
#include <xosmsg.h>
#include <xoserr.h>
#include <xostime.h>
#include <xossysp.h>
#include <xosusr.h>
#include "gph.h"
#include "gphintel.h"

extern int errno;

extern volatile long   mmr[];
extern volatile uchar  mmb[];
extern volatile ushort dsp[];
extern volatile long   pgtbl[];

#pragma pack (4);

long mmrphy;
long dspphy;
long dspsize;

long vgapalette[64];			// VGA palette values

#define VERSION 2
#define EDITNO  0

typedef struct
{	long   xres;
	long   yres;
	long   dpllctrl;
	long   dplldiv;
	long   htotal;
	long   hblank;
	long   hsync;
	long   vtotal;
	long   vblank;
	long   vsync;
	long   srcsize;
	ushort syncpol;
	ushort stride;
} MTBL;


//               dpplctrl    dppldiv     htotal      hblank      hsync
//          vtotal      vblank      vsync       srcsize  syncpol stride
MTBL mhcg16[] =
{	{ 640,  480, 0x108B0000, 0x00021207, 0x031F027F, 0x03170287, 0x02EF028F,
			0x020C01DF, 0x020401E7, 0x01EB01E9, 0x027F01DF, 0x00, 1280},
	{ 800,  600, 0x10850000, 0x00041808, 0x041F031F, 0x041F031F, 0x03C70347,
			0x02730257, 0x02730257, 0x025C0258, 0x031F0257, 0x18, 1600},
	{1024,  768, 0x10820000, 0x00041608, 0x053F03FF, 0x053F03FF, 0x049F0417,
			0x032502FF, 0x030702FF, 0x03080302, 0x03FF02FF, 0x18, 2048},
	{1280, 1024, 0x10810000, 0x00021206, 0x069704FF, 0x069704FF, 0x059F052F,
			0x042903FF, 0x042903FF, 0x04030400, 0x04FF03FF, 0x18, 2560},
	{1600, 1200, 0x10A00000, 0x00021206, 0x096F063F, 0x086F063F, 0x073F067F,
			0x04E104AF, 0x04E104AF, 0x04B304B0, 0x063F04AF, 0x18, 3200},
	{1920, 1440, 0x10200000, 0x00041405, 0x0A27077F, 0x0A27077F, 0x08CF07FF,
			0x05DB059F, 0x05DB059F, 0x05A305A0, 0x077F059F, 0x18, 3840},
	{0}
};

///long pgtblsz[] = {512*1024, 256*1024, 128*1024, 1024*1024, 2048*1024,
///		1536*1024, 512*1024, 512*1024};

REGS regs;

int flag = 1;

char msgbfr[32];
char srcbfr[128];

#pragma pack (1)

struct
{   LNGSTRPARM srcname;
    char       end;
} msginpparm =
{   {PAR_GET|REP_STR, 0, IOPAR_MSGRMTADDRR, srcbfr, 64, 0}
};

struct
{   LNGSTRPARM srcname;
    char       end;
} msgoutparm =
{   {PAR_SET|REP_STR, 0, IOPAR_MSGRMTADDRS, srcbfr, 64, 0}
};

#pragma pack (4)

QAB msginpqab =
{   QFNC_INBLOCK,			// func    - Function
    0,						// status  - Returned status
    0,						// error   - Error code
    0,						// amount  - Amount transfered
    0,						// handle  - Device handle
    VECT_MESSAGE,			// vector  - Vector for interrupt
    {0},					//         - Reserved
    0,						// option  - Options or command
    32,						// count   - Amount to transfer
    msgbfr,					// buffer1 - Pointer to data buffer
    NULL,					// buffer2 - Pointer to source string
    &msginpparm				// parm    - Pointer to parameter area
};

long *xpnt;
int   cnt;

#pragma pack (1)

struct
{	BYTE4CHAR portlist;
	uchar     more[64];
} iopchars =
{	{PAR_GET|REP_HEXV, 64, "IOPORTS"}
};


typedef struct
{	uchar type;
	uchar length;
	char  name[8];
	union
	{	struct
		{	long lownum;
			long hinum;
		};
		struct
		{	char *pnt;
			short size;
			short amount;
		};
        time_s dtv;
        char   ss[8];
    };
} DCHAR;

#pragma pack (4)

typedef struct
{	ulong  addr;
	ushort venid;
	ushort devid;
	ulong  class;
	ulong  regC;
	uchar  irq;
	uchar  intline;
	ushort xxx;
	ulong  addr0;
	ulong  size0;
	ulong  addr1;
	ulong  size1;
	ulong  addr2;
	ulong  size2;
	ulong  addr3;
	ulong  size3;
	ulong  addr4;
	ulong  size4;
	ulong  addr5;
	ulong  size5;
} PCIVAL;

typedef union
{	char  *c;
	DCHAR *d;
} DPNT;

typedef struct
{	long addr;
	long size;
} BAR;

QAB charqab =
{	QFNC_WAIT|QFNC_DEVCHAR,
	0,							// status  - Returned status
	0,							// error   - Error code
	0,							// amount  - Amount
	0,							// handle  - Device handle
	0,							// vector  - Vector for interrupt
	{0},
	CF_SIZE,					// option  - Options or command
	0,							// count   - Count
	NULL,						// buffer1 - Pointer to file spec
	NULL,						// buffer2 - Unused
	NULL						// parm    - Pointer to parameter area
};


#define DEV_830M   0x3577
#define DEV_845G   0x2562
#define DEV_855GM  0x3582
#define DEV_865G   0x2572
#define DEV_915G   0x2582	// 0x2782???
#define DEV_915GM  0x2592
#define DEV_945G   0x2772
#define DEV_945GM  0x27A2
#define DEV_945GME 0x27AE
#define DEV_946GZ  0x2972
#define DEV_965G   0x29A2
#define DEV_965GZ  0x2922
#define DEV_965GM  0x2A02
#define DEV_965GME 0x2A12
#define DEV_965Q   0x2992
#define DEV_G33    0x29C2
#define DEV_G35    0x2982
#define DEV_Q35    0x29B2
#define DEV_Q33    0x29D2
#define DEV_810    0x7121
#define DEV_810_DC 0x7122
#define DEV_815    0x1132

#pragma pack (2)

typedef struct
{	ushort devid;
	char  *desc;
} DEVTBL;

// Note: add " Graphics Controller" after each string

DEVTBL devtbl[] =
{	{DEV_G35   , "G35 Express"},
	{DEV_Q35   , "Q35 Express"},
	{DEV_Q33   , "Q33 Express"},
	{DEV_G33   , "G33/G31 Express"},
	{DEV_965GM , "Mobile GM965 Express"},
	{DEV_965GME, "Mobile GME965 Express"},
	{DEV_965G  , "G965"},
	{DEV_946GZ , "946GZ/GL"},
	{DEV_945G  , "945G"},
	{DEV_945GM , "Mobile 945GM/GMS/940GML Express"},
	{DEV_945GME, "Mobile 945GME Express"},
	{DEV_915G  , "82915G/GV/910GL Express"},
	{DEV_915GM , "Mobile 915GM/GMS/910GML Express"},
	{DEV_865G  , "82865G"},
	{DEV_855GM , "82852/855GM/855GME"},
	{DEV_845G  , "82845G/GL/GE"},
	{DEV_830M  , "830M"},
	{DEV_815   , "82815"},
	{DEV_810   , "82810"},
	{DEV_810_DC, "82810-M DC-100"}
};

#pragma pack (4)

char *getreqname(img_data *args, char *reqname);

void  dosetup(img_data *args);
void  setupfail(long code, char *fmt, ...);
void  setmode(long pid, int mode, int xres, int yres);
void  setstart(long pid, int xpos, int ypos);
long  setdispmode(MTBL *tbl, int vga);
MTBL *findmode(int mode, int xres, int yres);
int   gethexval(char *str, int len);


//*************
// Main program
//*************

void mainalt(
    img_data *args)

{
	dosetup(args);
	while (TRUE)
		svcSchSuspend(NULL, -1, -1);	// Wait for something to happen
}


//********************************************************
// Function: msgsignal - Message available signal function
//********************************************************

// Format of the request message:
//  Offset Size
//     0     1  Message ID = 0x3C
//     1     1  Function: 1 = Set mode, 2 = Set display start
//     2     1  XOS mode
//     3     1  Reserved, must be 0
//     4     4  TID of thread to wake up when finished
//     8     2  X resolution (If setting to graphics mode) or x position as
//                pixels (if setting position)
//    10     2  Y resolution (if setting to graphics mode) or y position as
//                pixels (if setting position)
// All messages must be at least 12 bytes long.

void msgsignal(void)
{
	for (;;)
	{
		if (msginpqab.amount >= 12 && msgbfr[0] == MT_GSREQUEST &&
				msgbfr[1] >= 1 && msgbfr[1] <= 2)
		{
			if (msgbfr[1] == 1)
				setmode(*(long *)(msgbfr + 4), msgbfr[2],
						*(ushort *)(msgbfr + 8), *(ushort *)(msgbfr + 10));
			else
				setstart(*(long *)(msgbfr + 4), *(ushort *)(msgbfr + 8),
						*(ushort *)(msgbfr + 10));
		}
		msginpqab.vector = 0;
		msginpparm.srcname.bfrlen = 64;
		if (svcIoQueue(&msginpqab) >= 0)
		{
			msginpqab.vector = VECT_MESSAGE;
			if (msginpqab.status & QSTS_DONE)
				continue;
		}
		break;
	}
}


//*****************************************
// Function: setmode - Set the display mode
// Returned: Nothing
//*****************************************

void setmode(
	long pid,
	int  mode,
	int  xres,
	int  yres)
{
	MTBL *mpnt;
	long  rtn;
	long  size;
	long  addr;

	size = 0;
	addr = 0;
	if (mode == DM_TEXT)
	{
		setdispmode(mhcg16 + 0, TRUE);
		rtn = 0;
	}
	else
	{
		if ((mpnt = findmode(mode, xres, yres)) == NULL)
			rtn = ER_DMDNA;
		else
		{
			setdispmode(mpnt, FALSE);
			rtn = mpnt->xres + (mpnt->yres << 16);
			size = dspsize;
			addr = dspphy;
		}
	}
	svcGphDone(pid, rtn, addr, size);
}


//*******************************************************************
// Function: findmode - Find best mode for the resolutions specified
// Returned: Address of the modetbl entry if a mode was found or NULL
//				if nothing usable was found
//*******************************************************************

MTBL *findmode(
	int mode,
	int xres,
	int yres)
{
	MTBL *mpnt;
	MTBL *ppnt;

	if (mode != DM_HCG16)				// We only support this mode!
		return (NULL);

	// Find an X resolution value that will work

	mpnt = mhcg16;
	ppnt = NULL;
	do
	{
		if (mpnt->xres == xres)
			break;
		if (mpnt->xres > xres)
		{
			if (ppnt != NULL)
				mpnt = ppnt;
			break;
		}
		ppnt = mpnt;
		mpnt++;
	} while (mpnt->xres != 0);
	if (mpnt == NULL)					// Did we find a usable mode?
	{
		if (ppnt != NULL)
			mpnt = ppnt;
		else
			return (NULL);				// No - return failure
	}

	// Here with a mode that has an X resolution that is as large as
	//   possible without exceeding the specified value. If the specified
	//   value is smaller than our smallest X resolution, that mode is
	//   used. Now find the best Y resolution.

	ppnt = mpnt;
	do
	{
		if (mpnt->xres != xres || mpnt->yres > yres)
										// Past what we might be able to use?
			return (ppnt);				// Yes - return the best we found
		if (mpnt->yres == yres)			// Exact match?
			return (mpnt);				// Yes - return it
		ppnt = mpnt;					// No - continue
		mpnt++;
	} while (mpnt->xres != 0);
	return (NULL);
}


void setstart(
	long pid,
	int  xpos,
	int  ypos)
{
	mmr[MM_ADSPLINOFF] = ypos * mmr[MM_ADSPSTRIDE] + xpos * 2;
	svcGphDone(pid, 0, -1, 0);
}

// DEBUG ONLY

static setdispmodex(
	MTBL *tbl)
{
	long *dpnt;
	int   cnt;

	setdispmode(tbl, FALSE);

	dpnt = (long *)dsp;
	cnt = tbl->xres * tbl->yres / 2;
	do
	{
		*dpnt++ = 0;
	} while (--cnt > 0);

	dpnt = (long *)dsp;
	cnt = tbl->yres;
	do
	{
		*dpnt++ = 0xF800F800;
		*dpnt++ = 0xF800F800;
		*dpnt++ = 0xF800F800;
		*dpnt++ = 0xF800F800;
		dpnt += (tbl->xres / 2 - 3);
	} while (--cnt > 0);

	return (0);
}

// END OF DEBUG ROUTINE


//*********************************************
// Function: setdispmode - Set the display mode
// Returned:
//*********************************************

// This is a fairly involved proceedure, but each step is short!

long setdispmode(
	MTBL *tbl,
	int      vga)
{
	long *rpnt;
	long *src;
	long  rtn;
	int   cnt;

	// 1) Disable sDVO ports' stall input (not needed now)

	// 2) Program sDVO ADD device ???? (not needed now)

	// 3) Disable ports (for now we only use port A)

	mmr[MM_APORTCTRL] |= 0x0C00;		// Turn off sync to the monitor
	svcSchSuspend(NULL, 0xFFFFFFFF, 4);	// Waiting may make the switch a bit
										//   cleaner.
	mmr[MM_APORTCTRL] &= 0x7FFFFFFF;	// Disable video

	// 4) Disable all planes  (for now we only use plane A

	mmr[MM_ADSPCTRL] &= 0x7FFFFFFF;		// Disable plane A
	mmr[MM_ACURCTRL] = 0;				// Disable cursor A
	mmr[MM_APIPECFG] |= 0xC0000;		// Disable all connected planes
	svcSchSuspend(NULL, 0xFFFFFFFF, 3);	// Wait for as short a time as we can

	// 5) Disable the pipe

	mmr[MM_APIPECFG] &= 0x7FFFFFFF;
	svcSchSuspend(NULL, 0xFFFFFFFF, 3);	// Wait again

	// 6) Disable VGA mode (Must be done after pipe is disabled since disabling
	//	  it first will stop vertical syncs if no planes are enabled (the usual
	//	  case when VGA mode is set) and the pipe will never completely stop!)

	if ((mmr[MM_VGACTRL] & 0x80000000) == 0) // In VGA mode now?
	{									// Yes
		mmb[VGA_SEQX] = 1;				// Turn off the VGA screen (must do
		mmb[VGA_SEQD] = 0x20;			//   this before disabling VGA mode)
		mmr[MM_VGACTRL] |= 0x80000000;	// Disable VGA mode (Must do this AFTER
										//   disabling the pipe. This bit is
										//   reversed!)
		svcSchSuspend(NULL, 0xFFFFFFFF, 3); // Wait once more
	}

	// 7) Disable panelfitter (not needed now)

	// 8) Disable the DPLL

	mmr[MM_ADPLLCTRL] &= 0x7FFFFFFF;	// Disable DPLL A (the only one we use)
	svcSchSuspend(NULL, 0xFFFFFFFF, 3);	// Wait yet again (Might not be needed
										//   but should not hurt!)

	// At this point everything should be idle. We can now set the DPLL
	//   parameters and start it. The DPLL must be running before we set
	//   and other parameters or enable anything.

	// 9) Do some magic fiddling with pipes and planes - This seems unnecessary
	//    but the PRM says to do it so we do it!

  	mmr[MM_APIPECFG] &= ~0xC0000;		// Clear bits 19:18
	mmr[MM_ADSPCTRL] |= 0x80000000;
	mmr[MM_ADSPBASE] = 0;
	mmr[MM_ADSPCTRL] &= 0x7FFFFFFF;
	mmr[MM_ADSPBASE] = 0;

	// 10) Program and enable the DPLL

	mmr[MM_A0DPLLDIV] = tbl->dplldiv;	// Set the divisors
	mmr[MM_ADPLLCTRL] = tbl->dpllctrl;	// Set the other parameters
	mmr[MM_ADPLLCTRL] |= 0x80000000;	// Start the DPLL
	svcSchSuspend(NULL, 0xFFFFFFFF, 3);	// Wait until it is stable

	// 11) Set display and pipe parameters

	mmr[MM_ADSPCTRL] = (mmr[MM_ADSPCTRL] & 0xB0000) |  0x14004000;
										// Set display for 16-bit (5-6-5),
										//   pixels, gamma correction enabled,
										//   no multiply, use linear memory
	mmr[MM_ADSPSTRIDE] = tbl->stride;
	mmr[MM_AHTOTAL] = tbl->htotal;		// Set the display timing parameters
	mmr[MM_AHBLANK] = tbl->hblank;
	mmr[MM_AHSYNC] = tbl->hsync;
	mmr[MM_AVTOTAL] = tbl->vtotal;
	mmr[MM_AVBLANK] = tbl->vblank;
	mmr[MM_AVSYNC] = tbl->vsync;

	mmr[MM_ASRCSIZE] = tbl->srcsize;
	mmr[MM_ADSPBASE] = 0;				// This must be last to trigger update
	mmr[MM_APIPECFG] = 0;				// Set pipe for non-interlace, 8-bit
										//   gamma lookup, 8-bit color, normal
										//   operation

	// 12) Enable panelfitter if needed (not needed now)

	// 13) Enable the pipe

	mmr[MM_APIPECFG] |= 0x80000000;
	svcSchSuspend(NULL, 0xFFFFFFFF, 3);	// Give it time to start

	// 14) Enable planes (VGA or hires) - Also initializes the palette (which
	//     we use for gamma correction).

	rpnt = (long *)(mmr + MM_APALETTE);
	if (vga)
	{
		mmr[MM_VGACTRL] &= 0x7FFFFFFF;	// (This bit is reversed!)
		cnt = 256;						// Restore the VGA palette
		src = vgapalette;
		do
		{
			*rpnt++ = *src++;
		} while (--cnt > 0);		
	}
	else
	{
		mmr[MM_ADSPCTRL] |= 0x80000000;	// Enable display plane A
		rtn = 0;						// Set up a linear palette
		cnt = 1024;
		do
		{
			*rpnt++ = rtn;
			rtn += 0x00010101;
		} while (--cnt > 0);
	}

	// 15) Enable ports (for now, this is just port A (analog output)

	mmr[MM_APORTCTRL] = (mmr[MM_APORTCTRL] & 0xFFFFFFE7) | tbl->syncpol;
										// Set bits 4:3 (sync polarity) for mode
	mmr[MM_APORTCTRL] &= 0xFFFFF3FF;	// Clear bits 11:10 to enable sync

	mmr[MM_ADSPLINOFF] = 0;				// This MUST be set here once everthing
										//   is ready to cause the STRIDE value
										//   to be used. Otherwise the STRIDE
										//   value set above is ignored!
	// Delay here???

	mmr[MM_APORTCTRL] |= 0x80000000;	// Set bit 31 to enable video

	return (0);
}


//*************************************************************
// Function: dosetup - Do once-only set up - This is a separate
//				function so we can reclaim the stack space.
// Returned: Nothing
//*************************************************************

// We must locate the display addresses, map the registers and display buffer,
//   and save the current VGA palette.

static void dosetup(
	img_data *args)
{
	DEVTBL *tpnt;

	PCIVAL *vpnt;
	DPNT    cpnts;
	DPNT    cpntd;
	BAR    *bpnt;

	long    rtn;

	int     length;
	int     type;

	if (getreqname(args, srcbfr) == 0) // Get name of our requestor
		exit(1);					   // Can't do anything without the name!

    setvector(VECT_MESSAGE, 0x04, msgsignal); // Set message received vector
    if ((msginpqab.handle = svcIoOpen(XO_IN|XO_OUT, "IPM:SYS^GRAPH",
			 NULL)) < 0)				// Also can't do much without a
		exit(1);						//   message device!
	svcSysSetPName("GphIntel");
    svcMemChange(args, 0, 0);			// Give up the argument msect

	// Find the display device by scanning the PCI devices to find one that
	//   we support

	if ((charqab.handle = svcIoOpen(0, "_PCI0:", NULL)) < 0)
		setupfail(charqab.handle, "Error opening PCI0: device");

	if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.error) < 0)
		setupfail(rtn, "Error getting PCI0: device characteristics");
	charqab.count = (ushort)charqab.amount;
	if ((charqab.buffer2 = (char *)malloc(charqab.count + 16)) == NULL)
		setupfail(-errno, "Cannot allocate memory for device characteristics");
	charqab.option = CF_ALL;
	if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.error) < 0)
		setupfail(rtn, "Error getting PCI0: device characteristics");

	// Now we have a complete list of the device characteristics for the
	//   PCI0 device.  We now remove everything except for the DEVbbddn
	//   characteristics

	cpnts.c = cpntd.c = (char near *)charqab.buffer2;
	while (--charqab.amount >= 0)
	{
		type = cpnts.d->type & 0x0F;
		length = (type >= 14) ? sizeof(DCHAR) : (cpnts.d->length + 10);
		if (strncmp(cpnts.d->name, "DEV", 3) == 0)
		{
			memcpy(cpntd.c, cpnts.c, length);
			cpntd.d->type |= PAR_GET;
			cpntd.d->pnt = (char *)malloc(cpntd.d->size);
			cpntd.c += length;
		}
		cpnts.c += length;
	}
	*cpntd.c = 0;
	charqab.option = CF_VALUES;
	if ((rtn = svcIoQueue(&charqab)) < 0 || (rtn = charqab.error) < 0)
		setupfail(rtn, "Error getting PCI0: device characteristics");
	cpnts.c = (char near *)charqab.buffer2;
	while (cpnts.d->type != 0)
	{
		vpnt = (PCIVAL *)(cpnts.d->pnt);
		if ((vpnt->class >> 24) == 0x03 && 
				gethexval(cpnts.d->name + 7, 1) == 0 &&
				vpnt->venid == 0x8086 && vpnt->size0 >= 0x80000)
		{								// Is this an Intel display controller?
			tpnt = devtbl;
			cnt =  sizeof(devtbl)/sizeof(DEVTBL);
			do
			{
				if (vpnt->devid == tpnt->devid)
					break;
				tpnt++;
			} while (--cnt > 0);
			if (cnt > 0)
			{
				// Here with what looks like an Intel display controller.
				//   It should use 3 address ranges but different versions
				//   may specify them in different BARs. The register area
				//   is memory mapped and is 512KB in length. The display
				//   buffer is also memory mapped and is prefetchable (bit
				//   3 is set in the BAR). Its length may be between about
				//   8MB and 256MB. The IO area (which we do not use) is
				//   8 bytes in length.

				dspphy = 0;
				mmrphy = 0;
				bpnt = (BAR *)&vpnt->addr0;
				cnt = 6;
				do
				{
					if ((bpnt->addr & 0x01) == 0)
					{
						if ((bpnt->addr & 0x08) && bpnt->size >= 0x800000 &&
								dspphy == 0)
						{
							dspphy = bpnt->addr & 0xFFFFFFF0;
						    dspsize = bpnt->size;
						}
						else if (bpnt->size == 0x80000 && mmrphy == 0)
							mmrphy = bpnt->addr & 0xFFFFFFF0;
					}
					bpnt++;
				} while (--cnt > 0);
				if (dspphy != 0 && mmrphy != 0)
					break;
			}
		}
		cpnts.d++;
	}
	if (cpnts.d->type == 0)
		setupfail(0, "No supported display controller found");

	// Here with a supported display contro;ler found

	if ((rtn = svcMemMapPhys((char *)mmr, mmrphy, PG_WRITE|PG_READ,
				0x80000)) < 0)
		setupfail(rtn, "Error mapping registers");

	if ((rtn = svcMemMapPhys((char *)dsp, dspphy, PG_WRITE|PG_READ,
				0x800000)) < 0)
		setupfail(rtn, "Error mapping display buffer");

	// Here with controller memory mapped - Save the VGA pallete. We assume
	//   that the display is in VGA mode when we get here so the VGA palette
	//   should be loaded.

	memcpy(vgapalette, mmr + MM_APALETTE, 256);

// START OF DEBUG CODE

	setdispmodex(mhcg16 + 0);			// 640 x 480

	INT3;

	setdispmodex(mhcg16 + 1);			// 800 x 600

	INT3;

	setdispmodex(mhcg16 + 2);			// 1024 x 768

	INT3;

	setdispmodex(mhcg16 + 3);			// 1280 x 1024

	INT3;

	setdispmodex(mhcg16 + 4);			// 1600 x 1200

	INT3;

	setdispmodex(mhcg16 + 5);			// 1929 x 1440

	INT3;

	setdispmode(mhcg16 + 0, TRUE);		// VGA text

	INT3;

// END OF DEBUG CODE

	msginpparm.srcname.bfrlen = 64;
	if ((rtn = svcIoQueue(&msginpqab)) < 0 || (rtn = msginpqab.error) < 0)
		setupfail(rtn, "Cannot start message input on IPM:SYS^GRAPH");
	response(STR_MT_FINALMSG"GphIntel: Startup is complete", 30);
}


//**********************************************************
// Function: setupfail - Called during set up on fatal error
// Returned: Nothing
//**********************************************************

static void setupfail(
	long  code,
	char *fmt,
	...)
{
	va_list pi;
	char   *pnt;
	char    text[200];

    va_start(pi, fmt);
    pnt = text + vsprintf(strmov(text, STR_MT_FINALERR"? GphIntel: "), fmt,
			&va_arg(pi, long)) + 13;
	if (code < 0)
	{
		if ((code & 0x00FFFF00) == 0)
			pnt += sprintf(pnt, "\r\n            VESA error #%d", code & 0xFF);
		else
			pnt += (10 + svcSysErrMsg(code, 0x03, strmov(pnt, "\r\n        ")));
		pnt = strmov(pnt, "\r\n");
	}
	response(text, pnt - text);
	exit(1);
}


//******************************************************
// Function: response - Generate set up response message
// Returned: Nothing
//******************************************************

void response(
	char *msg,
	int   len)
{
	if (srcbfr[0] == 0)					// Running from a terminal?
	{									// Yes - output message to the terminal
		svcIoOutBlock(DH_STDERR, msg + 1, len - 1);
		svcIoOutBlock(DH_STDERR, "\r\n", 2);
	}
	else
		svcIoOutBlockP(msginpqab.handle, msg, len, &msgoutparm);
}


//**********************************************************
// Function: gethexval - Extract hex value for a text string
// Returned: The value
//**********************************************************

static int gethexval(
	char *str,
	int   len)

{
	int  value;
	char chr;

	value = 0;
	while (--len >= 0)
	{
		if ((chr = *str++) >= 'A')
			chr += 9;
		value = value * 16 + (chr & 0x0F);
	}
	return (value);

}
