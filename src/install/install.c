

// An install set consists of a directory tree under the INSTALL directory in
//   the root of a disk partition. The name of this directory has one of the
//   following formats:
//		1) OS_m_n_e
//		2) APP_vendor_program_m_n_e
//   The first format is used for new versions of the OS. The second format
//   is used for new versions of applications. "m" is the major version number
//   of the release. "n" is the minor version number and "e" is the edit
//   number. Some applications will not have an edit number. "vendor" is the
//   short form of the vendor name. For XOS utilities which are installed as
//   separate applications it is "SYSTEM". "program" is the name of the
//   application.
//   For the OS this directory generally contains one item:
//		1) The XOS directory. This directory contains all of the OS files
// 			 and the OS.VER file.
//   When installing the OS the XOS directory tree from the INSTALL directory
//   is copied to the root of the target partition.
//   For applications, this directory generally contains two items:
//		1) The program.run file. (The main executable for the application)
//		2) A "program" directory which contains all other associated files
//			 and the program.ver file, including the program.VER file. This
//			 directory tree may contain empty subdirectories which will be
//			 created when the application is installed.
//   When installing the applications, the run file is copied to the APP
//   directory and the "program" directory tree is copied to the APP\DATA
//   directory.
//   The OS.VER or program.VER file is a text file in which each line begins
//   with a keyword followed by "=". All of the following keywords must be
//   included exept as indicated. They can be in any order.
//		VEN=  Specifies the short from of the vendor name. For applications,
//				this must be the same as appeared in the directory name. This
//				is not used for OS install sets.
//      PROG= Specifies the program name. For OS install sets it is "OS".
//				This must match the program name embedded in the directory
//				name.
//		VER=  Specifies the version number as m.n.e or m.n. This must match
//				the version number embedded in the directory name.
//		DATE= Specifies the date of the release as dd-mmm-yyyy (For example,
//				12-Nov-2009).
//   All blank lines are ignored. All whitespace is ignored and any text after
//   a semicolon is a comment and is ignored.

// NOTE: THIS VERSION DOES NOT IMPLIMENT BACKUP!

// All backup copies are made to directories in the BCK directory in the root
//   of the boot device. Directly under BCK are directories named with the
//   vender name or "OS" for operating system backups. Under each of these
//   directories is a directory with a name of the from program_yyyy-mm-dd
//   where "program" is the application name, "yyyy" is the 4-digit year, "mm"
//   is the 2-digit month number (1 to 12), and "dd" is the 2-digit day in
//   the month (1-31). All numeric values include leading 0s if necessary.
//   For operating system backups, the program name is "OS".

#include "install.h"

#define BLUE  0x00FF
#define LBLUE 0x4080FF

#define FIBR_BASE  0x40000000 // Base thread
#define FIBR_WORK  0x40020000 // Work thread
#define FIBR_XWS   0x40040000 // First thread used by XWS

// Define vectors

#define VECT_STATUS 50
#define VECT_XWS    51
#define VECT_FIBR   60		// First vector used by the fiber routines

#define MAXFIBRS 30			// Maximum number of fibers

#define HEIGHT 270000


VBLK   *vblkhead;
VBLK   *vsel;

///extern long xwscursorx;
///extern long xwscursory;

FILE *logfile;
long errcode;

int  index;

///FNT *font;
XWSFNT *stsfont;
XWSFNT *rptfont;

XWSWIN  *wina;
XWSWIN  *winh;
XWSWIN  *winw;
XWSWIN  *winlst;
XWSWIN  *wininit;

XWSWIN  *btnexit;
XWSWIN  *btnrescan;
XWSWIN  *btninitdisk;
XWSWIN  *btncancel;

///XWSWIN *winb;
///XWSWIN *winv;
///XWSWIN *winstatus;

XWSBITMAP *bm1;

XWSBITMAP *bmbg;

int  state;

long totalmemdsp; 
long totalblkdsp;
long alloccntdsp;
long freecntdsp;

long cursorxdsp;
long cursorydsp;

long wrkfbr;

long xpos;
long ypos = 20;

long curappver;

long dskhndl;
FILE *verfile;

long  xishndl;
long  totalsize;
long  crcvalue;
int   itemnum;
int   seqnum;

XIDHEAD  xishead;
ITEMHEAD itemhead;

#define OUTBUFRSZ 0x5000
#define XISBUFRSZ 0x4000

char *xisbufr;
char *outbufr;

char dispname[260];
char insttitle[300];
int  insttitlelen;

char *unitnames;
long  numunits;

char  tarprtn[20];
char  tarbase[20];
char  havexos;

long prevline;

char *fontnames[] =
{	"DejaVu_Serif|Book",
	"DejaVu_Serif|Bold",
	"DejaVu_Sans|Book",
	"DejaVu_Sans|Bold",
	"DejaVu_Sans_Mono|Book",
	"DejaVu_Sans_Mono|Bold"
};

long colors[] =
{   -2,					// 0
    -1,					// 1
    0x000000,			// 2 - Black
    0xFFFFFF,			// 3 - White
    0xC00000,			// 4 - Dark red
    0x008000,			// 5 - Dark green
    0x000080,			// 6 - Dark blue
    0xC0C0C0,			// 7 - Gray
    0xC0F0C0			// 8 - Light green
};

XWSEDB edb = { 8, fontnames, 9, colors};

XWSRECT  mwsz;
XWSRECT  scnsz;

XWSDRAW  drawbase;
XWSDRAW  drawstatus;
XWSDRAW  drawmouse;
XWSEVENT eventbase;

int      horiz;
int      vert;

XWSWINPAR   winparms;
XWSTXTPAR   txtparms;

///XWSTXTPAR   txtwinb;

uchar       dobackup;
uchar       continueinst;
uchar       showdierror;
uchar       didinit;

char prgname[] = "INSTALL";


///uchar  ready = FALSE;


static char line1[] = "XOS Installation/Update Utility";

static char btextrescan[] = "F4-Rescan Disks";
#if __DISKMAN
  static char btextinitdisk[] = "F6-Initialize Disk";
  static char initmsg[] = "You have asked to initialize disk %s (the system's "
		"primary disk) Doing so will \x11\x08\x02\x11\x1A\xFF\x01\x01""ERASE "
		"ALL\x11\x08\x01\x11\x1A\x01\x01\x01 data now on the disk! If this is "
		"what you want to do, enter \"ERASE\" (all upper case without the "
		"quotes) in the field below and press <ENTER>. To cancel and not "
		"change the contents of the disk, press <ESC>.";
#endif

void main(
    int   argc,
    char *argv[])

{
	long rtn;
	char text[64];

    // Do initial set up

	if (argc <= 1)
		horiz = 1280;
	else
	{
		horiz = atoi(argv[1]);
		if (argc >= 3)
			vert = atoi(argv[2]);
	}

	if ((xisbufr = malloc(XISBUFRSZ)) == NULL ||
			(outbufr = malloc(OUTBUFRSZ)) == NULL)
	{
		errormsg(-errno, "!? Error allocating memory for data buffers");
		exit(1);
	}

	if ((rtn = fbrCtlInitialize(MAXFIBRS, VECT_FIBR, FIBR_BASE + 0x20000 -
			sizeof(FBRDATA), 0x1F000, 0)) < 0)
		femsg2(prgname, "Error initializing threads", rtn, NULL);
	xwsInitBegin("TRM:", 6, (uchar *)FIBR_XWS, VECT_XWS, horiz, vert);
	xwsInitScreen();
	xwsInitCursor();
	xwsInitMouse();

	wrkfbr = FIBR_WORK +0x20000 - sizeof(FBRDATA);
	if ((rtn = fbrCtlCreate(wrkfbr, 0x1E000 - sizeof(FBRDATA), 0, workfiber,
			NULL, 0)) < 0)
		errormsg(rtn, "!Error creating work thread");

    if (xwsInitVesaA() != 0)
		errormsg(0, "!Could not initialize display (Is the graphics "
				"driver loaded?)");
    winparms.cntr = xws_WinParContainerPlain;
    winparms.bgdcolor = 0x8080FF;
    winparms.cursor = xws_CurPoint;
    winparms.bits2 |= WB2_NOFOCUS;
	winparms.draw = drawbase;
    xwsInitStart(&winparms, eventbase, XWS_EVMASK_KEYMAKE, &edb);

    // Set up the screen

    xwsWinGetBndBox(xws_ScreenWIN, &scnsz);
    xwsCurSetPos(0, 0);

///	if (xwsBitmapLoad((horiz <= 1024) ? "XOSIMG:BGB1024.JPG" :
///			"XOSIMG:BGB1280.JPG", NULL, 1, &bm1) >= 0)
///		xwsWinInvalidateRgn(xws_ScreenWIN, NULL);

	stsfont = xwsFontLoad("DejaVu_Serif|Book", xws_ScreenWIN, 16, 0, 0);

	// Create the main window

	winparms.cntr = xws_WinParContainerDefault;
	winparms.bgdcolor = 0xD0D0D0; /// 0xF5DDB7;	/// 0x60C0C0;
	winparms.bdrwidtho = 2000;
	winparms.bdrwidthc = 2000;
	winparms.bdrwidthi = 2000;
	winparms.zorder = 140;
    winparms.bits2 = WB2_NOFOCUS;
	xwsWinCreateContainer(xws_ScreenWIN, (scnsz.xsize - 370000)/2, 10000,
			370000, -10000, &winparms, NULL, 0, &edb, &wina);

	// Create the title window

	winparms.cntr = xws_WinParContainerDefault;
	winparms.bgdcolor = 0x0000FF;
	winparms.bdrwidtho = 700;
	winparms.bdrwidthc = 0;
	winparms.bdrwidthi = 0;
	winparms.zorder = 140;
    winparms.bits2 = WB2_NOFOCUS;
	xwsWinCreateContainer(wina, 0, 0, 0, 22000, &winparms, NULL,
			0, &edb, &winh);

	// Display the title

	txtparms = xws_TxtParContainerDefault;
	txtparms.fnum = 3;
	txtparms.fheight = 12000;
	txtparms.lfmt = TLF_BCENTER;
	txtparms.txtcolor = 0xFFFFFF;
	xwsWinTextNew(winh, 100, 2, 4000, -2, 40000, &txtparms, line1,
			sizeof(line1) - 1);
	rtn = sprintf(text, "V %d.%d.%d", MV, NV, EN);
	txtparms.fheight = 7000;
	txtparms.lfmt = TLF_RIGHT;
	xwsWinTextNew(winh, 101, 0, 8000, -6000, 1, &txtparms, text, rtn);

	// Create the work window

	winparms.cntr = xws_WinParContainerPlain;
	winparms.bgdcolor = 0xD0D0D0; /// 0xF5DDB7;	/// 0x60C0C0;
	winparms.zorder = 140;
    winparms.bits2 = WB2_NOFOCUS;
	xwsWinCreateContainer(wina, 0, 22000, 0, 0, &winparms, NULL, 0, &edb,
			&winw);

	xwsWinGetClient(winw, &mwsz);

	doscan();

///	xwsWinSetFocus(wina, 0);

    // Now start up the xws main loop

    xwsCtlMain();
}


//*********************************************************
// Function: clearmain - Clear (initialize) the main window
// Returned: Nothing
//*********************************************************

void clearmain(
	char *msg)
{
	VBLK *vpnt;

	xwsWinClear(winw);					// Clear the window
	while (vblkhead != NULL)			// Give up the program list
	{
		vpnt = vblkhead->next;
		xwsFree(vblkhead);
		vblkhead = vpnt;
	}

	// Display the initial header text to tell him to wait while we
	//   scan the disks

	txtparms = xws_TxtParContainerDefault;
	txtparms.fnum = 1;
	txtparms.fheight = HEADSIZE;
	txtparms.lfmt = TLF_BCENTER;
	xwsWinTextNew(winw, 100, 2, TEXTYPOS, -2, 1, &txtparms, msg, strlen(msg));

	// Create the exit button

	winparms.butn = xws_WinParButtonDefault;
	txtparms = xws_TxtParButtonDefault;
	txtparms.fheight = 7600;
	winparms.evarg1 = ISC_F12;
	xwsWinCreateButton(winw, BTNXPOS(4), mwsz.ysize - 20000, BTNWIDTH,
			12000, &winparms, &txtparms, buttonevent, XWS_EVMASK_WINDOW,
			&edb, &btnexit, "F12-Exit", 8);
}


void disprescan(void)
{
	winparms.butn = xws_WinParButtonDefault;
	txtparms = xws_TxtParButtonDefault;
	txtparms.fheight = 7600;
	winparms.evarg1 = ISC_F4;
	xwsWinCreateButton(winw, BTNXPOS(2), mwsz.ysize - 20000, BTNWIDTH,
			12000, &winparms, &txtparms, buttonevent, XWS_EVMASK_WINDOW,
			&edb, &btnrescan, btextrescan, sizeof(btextrescan) - 1);
	winparms.butn.evarg1 = ISC_F6;

#if __DISKMAN
	xwsWinCreateButton(winw, BTNXPOS(3), mwsz.ysize - 20000, BTNWIDTH,
			12000, &winparms, &txtparms, buttonevent, XWS_EVMASK_WINDOW,
			&edb, &btninitdisk, btextinitdisk, sizeof(btextinitdisk) - 1);
#endif
}



//*************************************************************
// Function: puterror - Put an error line in the selection list
// Returned: Nothing
//*************************************************************

void puterror(
	long  code,
	char *fmt, ...)
{
	va_list pi;
	int     len;
	char    text[300];

	va_start(pi, fmt);
	len = vsprintf(text, fmt, pi);
	strcpy(text + len, "\n    ");
	len += svcSysErrMsg(code, 0x03, text + len + 5);
	txtparms = xws_TxtParContainerDefault;
	txtparms.bgdcolor = COL_ERROR;
	txtparms.fnum = 1;
	txtparms.fheight = 6000;

///	txtparms.attr = (TA_VERCEN | TA_NOSELECT);
	txtparms.tbattr = TBA_NOSELECT;

	txtparms.leftmargin = 4000;
	txtparms.topmargin = 1000;
	txtparms.btmmargin = 1000;
	xwsWinTextNew(winlst, index++, 0, 1600, 0, 1, &txtparms, text, len + 5);
	if (logfile != NULL)
		fprintf(logfile, "%s\n", text);
}


//*****************************************************
// Function: putline - Put a line in the selection list
// Returned: Nothing
//*****************************************************

void putline(
	long  color,
	char *fmt, ...)
{
	va_list pi;
	int     len;
	int     incx;
	char    text[200];

	va_start(pi, fmt);
	incx = ((color & TEMP) == 0);
	len = vsprintf(text, fmt, pi);
	txtparms = xws_TxtParContainerDefault;
	txtparms.bgdcolor = color;
	txtparms.fnum = 1;
	txtparms.fheight = 6600;
	txtparms.tbattr = (color == COL_NORMAL) ? TBA_VERCEN :
			(TBA_VERCEN | TBA_NOSELECT);
	txtparms.leftmargin = 4000;
	txtparms.topmargin = 1000;
	txtparms.btmmargin = 1000;
	xwsWinTextNew(winlst, index, 0, 1600, 0, 1, &txtparms, text, len);
	xwsWinListPutItemInView(winlst, index);
	index += incx;
	if (logfile != NULL)
		fprintf(logfile, "%s\n", text);
}



void drawbase(
	XWSWIN *win)

{
	xwsWinDrawBegin(win);
///	xwsDrawBitmap(win, bm1, 0, 0, 0, 0, 1, 1, 0);
	xwsWinDrawEnd(win);
}


//*********************************************************
// Function: eventbase - Event function for the base window
// Returned: Nothing
//*********************************************************

// This handles all key presses except those in edit windows

long eventbase(
	XWSEDBLK *edblk)

{
	if (edblk->type == XWS_EVENT_KEYMAKE)
	{
		if (((XWSEDBLK_KEYBRD *)edblk)->scancd == ISC_F12 &&
				(((XWSEDBLK_KEYBRD *)edblk)->kbsts &
				(XWS_EVENT_KBS_LFTCTL|XWS_EVENT_KBS_RHTCTL)))
		{
			debugtoggle();
			return (XWS_EVRTN_DONE);
		}
		return (keymakeevent(((XWSEDBLK_KEYBRD *)edblk)->charcd,
				((XWSEDBLK_KEYBRD *)edblk)->scancd));
	}
    return (XWS_EVRTN_SYS);
}



long buttonevent(
	XWSEDBLK *edblk)

{
	return (keymakeevent(edblk->arg1 >> 8, (uchar)(edblk->arg1)));
};


//******************************************************
// Function: keymakeevent - Handle key make events
// Returned: Event return value
//******************************************************

// This is also called by the cmd button event function. Each button is defined
//   with an event argument which is equal to the corresponding function key
//   scan code (which is just the function key number). Thus it is easy to
//   make a button press look like a function key press.

int keymakeevent(
	long charcd,
	long scancd)

{
#if __DISKMAN
	char text[512];
#endif

	switch(state)
	{
	 case STATE_ERROR:
		if (charcd == '\r')
		{
			errorclose();
			return (XWS_EVRTN_DONE);
		}
		break;

	 case STATE_QUESTION:
		return (questionevent(scancd + (charcd << 8)));

	 case STATE_SELECT:
		switch(scancd)
		{
///		 case ISC_F2:
///			dobackup = TRUE;
///			goto inst;

		 case ISC_F2:
			dobackup = FALSE;
///		 inst:
			doinst();
			return (XWS_EVRTN_DONE);

		 case ISC_F4:
			doscan();
			return (XWS_EVRTN_DONE);

#if __DISKMAN
		 case ISC_F6:
			continueinst = FALSE;
			doinit(text, sprintf(text, initmsg, tarbase), NULL);
			return (XWS_EVRTN_DONE);
#endif

		 case ISC_F12:

			xwsWinClear(winw);			// Clear the window (DEBUG!)

			exit(0);
		}
		break;

	 case STATE_INST:
		switch(scancd)
		{
		 case ISC_F11:
			continueinst = FALSE;
			putline(COL_FAIL, "--- INTERRUPTED ---");
			return (XWS_EVRTN_DONE);

		 case ISC_F12:
			exit(0);
		}
		break;

	 case STATE_DONE:
		switch(scancd)
		{
		 case ISC_F4:
			doscan();
			return (XWS_EVRTN_DONE);

		 case ISC_F12:
			exit(0);
		}
		break;

	 case STATE_INITIALIZE:
		switch(scancd)
		{
		 case ISC_ESC:
			xwsWinDestroy(wininit);
			state = STATE_SELECT;
			return (XWS_EVRTN_DONE);
		}
		break;
	}
	return (XWS_EVRTN_SYS);
}


void eventfunc(
	int charcd,
	int scancd)
{
	charcd = charcd;
	scancd = scancd;


}
