/************************************************************/
/* gphintel - Main program                                   */
/* Graphic mode support symbiont for Intel displays for XOS */
/***********************************************************/
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

// Notes on programming the Intel graphics controlers:

// 1) These are very complex devices with many order dependencies.
// 2) There are a large number of versions of these devices and even more names
//    used to refer to these versions. The naming is extremely confusing and
//    is not very well documented.
// 3) The different versions are "largely" compatable, but there are significant
//    differences.
// 4) Enabling and disabling a video stream is fairly complex and MUST follow
//    the order used here. Many stream parameters cannot be changed unless the
//    stream is disabled. (A stream is a combination of one or more display
//    surfaces, a display pipe, and one or more ports.)
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
#include "gphvesa.h"

extern int errno;

extern uchar vesacode[];
extern uchar vesacodesz[];
extern uchar knlPageZero[];
extern uchar knlV86CodeBase[];

#define TRMCLASS 0x4D5254L

// Main program

uchar startdone = FALSE;

#define VERSION 1
#define EDITNO  6

REGS regs;

int flag = 1;

char msgbfr[32];
char srcbfr[128];
char reqbfr[64];

struct
{   LNGSTRPARM srcname;
    char        end;
} msginpparm =
{   {PAR_GET|REP_STR, 0, IOPAR_MSGRMTADDRR, srcbfr, 128, 0}
};

struct
{   LNGSTRPARM srcname;
    char       end;
} msgoutparm =
{   {PAR_SET|REP_STR, 0, IOPAR_MSGRMTADDRS, reqbfr, 64, 0}
};

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
    (char *)&msginpparm		// parm    - Pointer to parameter area
};

struct
{	BYTE4CHAR portlist;
	uchar     more[64];
} iopchars =
{	{PAR_GET|REP_HEXV, 64, "IOPORTS"}
};


struct
{	LNGSTRCHAR disptype;
	LNGSTRCHAR dispdesc;
	LNGSTRCHAR dispver;
	LNGSTRCHAR dispven;
	LNGSTRCHAR disprev;
	uchar      end;
} dispchars =
{	{PAR_SET|REP_STR, 0, "DISPTYPE"},
	{PAR_SET|REP_STR, 0, "DISPDESC"},
	{PAR_SET|REP_STR, 0, "DISPVER"},
	{PAR_SET|REP_STR, 0, "DISPVEN"},
	{PAR_SET|REP_STR, 0, "DISPREV"}
};

int size1 = sizeof(VBEINFO);
int size2 = sizeof(VBEMODE);

extern VBEINFO vbeinfo;
extern VBEMODE vbemode;

long *xpnt;
int   cnt;

MDATA  *modelist[4];

char *getreqname(img_data *args, char *reqname);

void  dosetup(img_data *args);

void  setaddr(LNGSTRCHAR *lsc, RPNTR *addr);

void  response(char *msg, int len);

void  setupfail(long code, char *fmt, ...);

void  setmode(long pid, int mode, int xres, int yres);
void  setstart(long pid, int xpos, int ypos);

MDATA *findmode(int mode, int xres, int yres);

void  reportmode(MDATA *mpnt, char *label);

long  v86call(REGS *regs, long stk, long start);

void  msgsignal(void);


void mainmin(
    img_data *args)

{
	dosetup(args);
	while (TRUE)
		svcSchSuspend(NULL, -1);		// Wait for something to happen
}


//*************************************************************
// Function: dosetup - Do once-only set up - This is a separate
//				function so we can reclaim the stack space.
// Returned: Nothing
//*************************************************************

void dosetup(
	img_data *args)
{
	ushort *modepnt;
	MDATA **listprev;
	MDATA  *listpnt;
	MDATA  *datapnt;

///	ushort * dpnt;

///	MDATA *mpnt;

	long    rtn;
	long    conhndl;
	long    bufrsz;
	int     vbemodenum;
	int     xosmode;
	int     redsize;
	int     redpos;
	int     grnsize;
	int     grnpos;
	int     blusize;
	int     blupos;
	int     modecnt;
	uchar   version3;
	char    bufr1[128];
	char    bufr2[128];

	if (getreqname(args, reqbfr) == 0) // Get name of our requestor
		exit(1);					   // Can't do anything without the name!
    setvector(VECT_MESSAGE, 0x04, msgsignal); // Set message received vector
    if ((msginpqab.handle = svcIoOpen(XO_IN|XO_OUT, "IPM:SYS^GRAPH",
			 NULL)) < 0)				// Also can't do much without a
		exit(1);						//   message device!
	svcSysSetPName("GphVESA");

	// Open the console terminal and get the list of IO ports it may need
	//   to access and enable access to them.

	if ((conhndl = svcIoOpen(XO_IN, "TRM0S1:", NULL)) < 0)
		setupfail(conhndl, "Error opening console terminal");
	rtn = svcIoDevChar(conhndl, (char *)&iopchars);
	if (rtn < 0)
	{
		svcIoClose(conhndl, 0);
		setupfail(rtn, "Error getting IO port list");
	}
	if ((rtn = svcIoPorts(1, (ushort *)&iopchars.portlist.value)) < 0)
	{
		svcIoClose(conhndl, 0);
		setupfail(rtn, "Error enabling IO port access");
	}

	// Create the V86 environment needed to do the VBE BIOS calls

	if ((rtn = svcMemChange((uchar *)0, PG_WRITE|PG_READ, 0x1000)) < 0)
	{
		svcIoClose(conhndl, 0);
		setupfail(rtn, "Error allocating V86 mode memory");
	}

	// Set up the DOS/BIOS vector table that existed when the system was
	//   loaded.

	memcpy((uchar *)0, (uchar *)&knlPageZero, 0x1000);

	// Provide access to the console display buffer and the BIOS ROMs.

	if ((rtn = svcMemMapPhys((uchar *)0xA0000, 0xA0000, PG_WRITE|PG_READ,
			0x20000)) < 0)
	{
		svcIoClose(conhndl, 0);
		setupfail(rtn, "Error mapping V86 mode base memory");
	}
	if ((rtn = svcMemMapPhys((uchar *)0xC0000, 0xC0000, PG_WRITE|PG_READ,
			0x20000)) < 0)
	{
		svcIoClose(conhndl, 0);
		setupfail(rtn, "Error mapping V86 mode display BIOS");
	}
	if ((rtn = svcMemMapSys((uchar *)0xED000, knlV86CodeBase, PG_WRITE|PG_READ,
			0x3000)) < 0)
	{
		svcIoClose(conhndl, 0);
		setupfail(rtn, "Error mapping V86 mode XOS memory");
	}
	if ((rtn = svcMemMapPhys((uchar *)0xF0000, 0xF0000, PG_WRITE|PG_READ,
			0x10000)) < 0)
	{
		svcIoClose(conhndl, 0);
		setupfail(rtn, "Error mapping V86 mode basic BIOS");
	}

	// Copy the necessary V86 mode code to page 0. This code executes an
	//   INT 0x10 followed by an INT knl_DISMISS.

	memcpy((uchar *)0x0C00, vesacode, (long)vesacodesz);
	vbeinfo.label = '2EBV';
	regs.rgEAX = 0x4F00;				// Set up for VBE function 0 (return
	regs.rgEDI = (long)&vbeinfo;		//   controller information)
	regs.rgES = 0;
	if ((rtn = v86call(&regs, 0x00000B00, 0x00C00000)) < 0)
	{
		svcIoClose(conhndl, 0);
		setupfail(rtn, "Error calling VBE function 0");
	}

	// Here with the VBE controller information

	if (vbeinfo.version < 0x0200)
	{
		svcIoClose(conhndl, 0);
		setupfail(0, "VBE version 1.x is not supported");
	}
	dispchars.disptype.strlen = sprintf(bufr1, "VESA (%X.%X) display controller",
			vbeinfo.version >> 8, (uchar)vbeinfo.version);
	dispchars.disptype.buffer = bufr1;
	setaddr(&dispchars.dispdesc, &vbeinfo.oemstr);
	dispchars.dispver.strlen = sprintf(bufr2, "%X.%X", vbeinfo.oemrev >> 8,
			(uchar)vbeinfo.oemrev);
	dispchars.dispver.buffer = bufr2;
	setaddr(&dispchars.dispven, &vbeinfo.venstr);
	setaddr(&dispchars.disprev, &vbeinfo.revstr);
	svcIoDevChar(conhndl, (char *)&dispchars);
	svcIoClose(conhndl, 0);
	version3 = (vbeinfo.version >= 0x0300);
	rtn = sprintf(bufr1, "\x01VBE version is %X", vbeinfo.version);
	response(bufr1, rtn);
	modepnt = (ushort *)(vbeinfo.modelist.offset +
			(vbeinfo.modelist.segment << 4));
	modecnt = 0;
	while ((vbemodenum = *modepnt++) != 0xFFFF)
	{
		regs.rgEAX = 0x4F01;			// Set up for VBE function 1 (get mode
		regs.rgEDI = (long)&vbemode;	//   information)
		regs.rgES = 0;
		regs.rgECX = vbemodenum;
		if ((rtn = v86call(&regs, 0x00000B00, 0x00C00000)) < 0)
			setupfail(rtn, "Error calling VBE function 1");
		if (vbemode.attr.modeok && vbemode.attr.graphic &&
				vbemode.attr.linear && vbemode.memmdl == 6)
		{
			if (version3)
			{
				redsize = vbemode.linredsize;
				redpos = vbemode.linredpos;
				grnsize = vbemode.lingrnsize;
				grnpos = vbemode.lingrnpos;
				blusize = vbemode.linblusize;
				blupos = vbemode.linblupos;
			}
			else
			{
				redsize = vbemode.redsize;
				redpos = vbemode.redpos;
				grnsize = vbemode.grnsize;
				grnpos = vbemode.grnpos;
				blusize = vbemode.blusize;
				blupos = vbemode.blupos;
			}
			bufrsz = ((((vbemode.xres * vbemode.yres * (vbemode.pixsz / 8)) +
					0xFFFF)) & 0xFFFF0000) * (vbemode.linimgnm + 1);

			if (vbemode.pixsz == 16 && redsize == 5 && blusize == 5 &&
					grnpos == 5 && blupos == 0)
			{
				if (grnsize == 6 && redpos == 11)
					xosmode = DM_HCG16;
				else if (grnsize == 5 && redpos == 10)
					xosmode = DM_HCGx15;
				else
					continue;
			}
			else if (vbemode.pixsz == 32 && redsize == 8 && grnsize == 8 &&
					blusize == 8 && grnpos == 8)
			{
				if (redpos == 16 && grnpos == 8 && blupos == 0)
					xosmode = DM_TCGx24;
				else if (redpos == 24 && grnpos == 16 && blupos == 8)
					xosmode = DM_TCG24x;
				else
					continue;
			}
			else
				continue;

			// Here with a usable mode. Allocate and set up the mode block.

			if ((datapnt = (MDATA *)sbrk(sizeof(MDATA))) == (MDATA *)-1)
				setupfail(-errno, "Error allocating memory for mode data");
			datapnt->vesamode = vbemodenum;
			datapnt->bufrpa = vbemode.linaddr;
			datapnt->bufrsz = bufrsz;
			datapnt->maxpixclk = vbemode.maxpixclk;
			datapnt->xres = vbemode.xres;
			datapnt->yres = vbemode.yres;
			modecnt++;

			// Insert the mode block into the list. There is a seperate list
			//   for each XOS mode. Each list is ordered first by increasing
			//   horizontal resolution and then by increasing vertical
			//   resolution.

			listprev = &modelist[xosmode - 2];
			while ((listpnt = *listprev) != NULL)
			{
				if (vbemode.xres < listpnt->xres ||
						(vbemode.xres == listpnt->xres &&
						vbemode.yres < listpnt->yres))
					break;
				listprev = &listpnt->next;
			}
			datapnt->next = listpnt;
			*listprev = datapnt;
		}
	}
	if (modecnt == 0)
		setupfail(0, "No usable display modes were found");

	reportmode(modelist[0], "HCGx15");
	reportmode(modelist[1], "HCG16");
	reportmode(modelist[2], "TCGx24");
	reportmode(modelist[3], "TCG24x");
	msginpparm.srcname.bfrlen = 64;
	if ((rtn = svcIoQueue(&msginpqab)) < 0 || (rtn = msginpqab.error) < 0)
		setupfail(rtn, "Cannot start message input on IPM:SYS^GRAPH");
	rtn = sprintf(bufr1, STR_MT_FINALMSG"GphVESA: Startup is complete, %d "
			"mode%s were found", modecnt, (modecnt != 1) ? "s" : "");
	response(bufr1, rtn);
}


void reportmode(
	MDATA *mpnt,
	char  *label)
{
	int  rtn;
	char bufr[128];

	if (mpnt == NULL)
	{
		rtn = sprintf(bufr, STR_MT_INTRMDMSG"%8s: None found", label);
		response(bufr, rtn);
	}
	else
	{
		rtn = sprintf(bufr, STR_MT_INTRMDMSG"%8s:", label);
		response(bufr, rtn);
		do
		{
			rtn = sprintf(bufr, STR_MT_INTRMDMSG"    M:%03X X:%-4d Y:%-4d "
					"BS:%07X BA:%08X", mpnt->vesamode, mpnt->xres, mpnt->yres,
					mpnt->bufrsz, mpnt->bufrpa);
			response(bufr, rtn);
		} while ((mpnt = mpnt->next) != NULL);
	}
}


//********************************************************
// Function: response - Generate start-up response message
// Returned: Nothing
//********************************************************

void response(
	char *msg,
	int   len)
{
	if (reqbfr[0] == 0)					// Running from a terminal?
	{									// Yes - output message to the terminal
		svcIoOutBlock(DH_STDERR, msg + 1, len - 1);
		svcIoOutBlock(DH_STDERR, "\r\n", 2);
	}
	else
		svcIoOutBlockP(msginpqab.handle, msg, len, (char *)&msgoutparm);
}


//********************************************************
// Function: msgsignal - Message available signal function
//********************************************************

// Format of the request message:
//  Offset Size
//     0     1  Message ID = 0x3C
//     1     1  Function: 1 = Set mode, 2 = Get values, 3 = Set display start
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
	MDATA *mpnt;
	long   pid;

    for (;;)
    {
		if (msginpqab.amount >= 12 && msgbfr[0] == MT_GSREQUEST)
		{
			pid = *(long *)(msgbfr + 4);
			switch (msgbfr[1])
			{
			 case 1:
				setmode(pid, msgbfr[2], *(ushort *)(msgbfr + 8),
						*(ushort *)(msgbfr + 10));
				break;

			 case 2:
				if ((mpnt = findmode(msgbfr[2], *(ushort *)(msgbfr + 8),
						*(ushort *)(msgbfr + 10))) == NULL)
					svcGphDone(pid, ER_DMDNA, 0, 0, 0, 0);
				else
					svcGphDone(pid, mpnt->xres + (mpnt->yres << 16),
							mpnt->bufrpa, mpnt->bufrsz, 0, 0);
				break;

			 case 3:
				setstart(pid, *(ushort *)(msgbfr + 8),
						*(ushort *)(msgbfr + 10));
				break;

			 default:
				svcGphDone(pid, ER_FUNC, 0, 0, 0, 0);
			}
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
	MDATA *mpnt;
	long   rtn;
	long   size;
	long   addr;

	pid = pid;

	size = 0;
	addr = 0;
	regs.rgEAX = 0x4F02;				// Set up for VBE function 2
	regs.rgEDI = 0;
	regs.rgES = 0;
	if (mode == DM_TEXT)
	{
		regs.rgEBX = 0x03;
		rtn = v86call(&regs, 0x00000B00, 0x00C00000);
	}
	else
	{
		if ((mpnt = findmode(mode, xres, yres)) == NULL)
			rtn = ER_DMDNA;
		else
		{
			regs.rgEBX = 0x4000 | mpnt->vesamode;
			if ((rtn = v86call(&regs, 0x00000B00, 0x00C00000)) >= 0)
			{
				rtn = mpnt->xres + (mpnt->yres << 16);
				size = mpnt->bufrsz;
				addr = mpnt->bufrpa;
			}
		}
	}
	svcGphDone(pid, rtn, addr, size, 0, 0);
}


//******************************************************************
// Function: findmode - Find best mode for the resolutions specified
// Returned: Address of the MDATA if a mode was found or NULL if
//				nothing usable was found
//******************************************************************

MDATA *findmode(
	int mode,
	int xres,
	int yres)
{
	MDATA *mpnt;
	MDATA *ppnt;

	// Find an X resolution value that will work

	mpnt = modelist[mode - 2];
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
	} while ((mpnt = mpnt->next) != NULL);
	if (mpnt == NULL)					// Did we find a usable mode?
	{
		if (ppnt != NULL)
			mpnt = ppnt;
		else
			return (NULL);				// No - return failure
	}

	// Here with a mode that has an X resolution that is as large as possible
	//   without exceeding the specified value. If the specified value is
	//   smaller than our smallest X resolution, that mode is used. Now find
	//   the best Y resolution.

	ppnt = mpnt;
	do
	{
		if (mpnt->xres != xres || mpnt->yres > yres)
										// Past what we might be able to use?
			return (ppnt);				// Yes - return the best we found
		if (mpnt->yres == yres)			// Exact match?
			return (mpnt);				// Yes - return it
		ppnt = mpnt;					// No - continue
	} while ((mpnt = mpnt->next) != NULL);
	return (NULL);
}


void setstart(
	long pid,
	int  xpos,
	int  ypos)
{
	long rtn;

	regs.rgEAX = 0x4F07;				// Set up for VBE function 7
	regs.rgEBX = 0x0000;
	regs.rgECX = xpos;
	regs.rgEDX = ypos;
	rtn = v86call(&regs, 0x00000B00, 0x00C00000);
	svcGphDone(pid, rtn, -1, 0, 0, 0);
}


void setaddr(
	LNGSTRCHAR *lsc,
	RPNTR      *addr)
{
	lsc->buffer = (char *)((addr->segment << 4) + addr->offset);
	lsc->strlen = strnlen(lsc->buffer, 63);
}


void setupfail(
	long  code,
	char *fmt,
	...)
{
	va_list pi;
	char   *pnt;
	char    text[200];

    va_start(pi, fmt);
    pnt = text + vsprintf(strmov(text, STR_MT_FINALERR"? GRAPHIC: "), fmt, pi);
	if (code < 0)
	{
		if ((code & 0x00FFFF00) == 0)
			pnt += sprintf(pnt, "\r\n        VESA error #%d", code & 0xFF);
		else
			pnt += (10 + svcSysErrMsg(code, 0x03, strmov(pnt, "\r\n        ")));
		pnt = strmov(pnt, "\r\n");
	}
	response(text, pnt - text);
	exit(1);
}
