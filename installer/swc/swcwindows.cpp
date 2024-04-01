#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <dos.h>
#include <io.h>
#include <time.h>
#include "xoserr.h"
#include "xosstuff.h"
#include "xosqdes.h"
#include "xcstring.h"
#include "scancodes.h"
#include "swcwindows.h"

// Table which translates Windows virtual-key codes to XOS internal scan codes

uchar swcvirkey2xos[] =
{	0,				// 00
	0,				// 01 - Left mouse button
	0,				// 02 - Right mouse button
	0,				// 03 - Ctl-Break
	0,				// 04 - Middle mouse button
	0,				// 05
	0,				// 06
	0,				// 07
	ISC_BS,			// 08 - Backspace key
	ISC_HT,			// 09 - Tab key
	0,				// 0A
	0,				// 0B
	0,				// 0C - Clear key (keypad 5)
	ISC_ENT,		// 0D - Enter key
	0,				// 0E
	0,				// 0F
	ISC_LFTSHF+0x80,// 10 - Shift key
	ISC_LFTCTL+0x80,// 11 - Control key
	0,				// 12 - Menu key
	ISC_PAUSE,		// 13 - Pause key
	0,				// 14 - Caps lock key
	0,				// 15
	0,				// 16
	0,				// 17
	0,				// 18
	0,				// 19
	0,				// 1A
	ISC_ESC,		// 1B - Escape key
	0,				// 1C
	0,				// 1D
	0,				// 1E
	0,				// 1F
	ISC_SPACE,		// 20 - Space bar
	ISC_PGUP,		// 21 - Page up
	ISC_PGDN,		// 22 - Page down
	ISC_END,		// 23 - End
	ISC_HOME,		// 24 - Home
	ISC_LTARW,		// 25 - Left arrow
	ISC_UPARW,		// 26 - Up arrow
	ISC_RTARW,		// 27 - Right arrow
	ISC_DNARW,		// 28 - Down arrow
	0,				// 29 - Select key
	0,				// 2A - Print key
	0,				// 2B - Execute key
	ISC_PRTSCN,		// 2C - Print screen key
	ISC_INS,		// 2D - Insert key
	ISC_DELETE,		// 2E - Delete key
	0,				// 2F - Help key
	ISC_0,			// 30 - 0
	ISC_1,			// 31 - 1
	ISC_2,			// 32 - 2
	ISC_3,			// 33 - 3
	ISC_4,			// 34 - 4
	ISC_5,			// 35 - 5
	ISC_6,			// 36 - 6
	ISC_7,			// 37 - 7
	ISC_8,			// 38 - 8
	ISC_9,			// 39 - 9
	0,				// 3A
	0,				// 3B
	0,				// 3C
	0,				// 3D
	0,				// 3E
	0,				// 3F
	0,				// 40
	ISC_A,			// 41 - A
	ISC_B,			// 42 - B
	ISC_C,			// 43 - C
	ISC_D,			// 44 - D
	ISC_E,			// 45 - E
	ISC_F,			// 46 - F
	ISC_G,			// 47 - G
	ISC_H,			// 48 - H
	ISC_I,			// 49 - I
	ISC_J,			// 4A - J
	ISC_K,			// 4B - K
	ISC_L,			// 4C - L
	ISC_M,			// 4D - M
	ISC_N,			// 4E - N
	ISC_O,			// 4F - O
	ISC_P,			// 50 - P
	ISC_Q,			// 51 - Q
	ISC_R,			// 52 - R
	ISC_S,			// 53 - S
	ISC_T,			// 54 - T
	ISC_U,			// 56 - U
	ISC_V,			// 55 - V
	ISC_W,			// 57 - W
	ISC_X,			// 58 - X
	ISC_Y,			// 59 - Y
	ISC_Z,			// 5A - Z
	0,				// 5B
	0,				// 5C
	0,				// 5D
	0,				// 5E
	0,				// 5F
	ISC_KP0,		// 60 - Keypad 0
	ISC_KP1,		// 61 - Keypad 1
	ISC_KP2,		// 62 - Keypad 2
	ISC_KP3,		// 63 - Keypad 3
	ISC_KP4,		// 64 - Keypad 4
	ISC_KP5,		// 65 - Keypad 5
	ISC_KP6,		// 66 - Keypad 6
	ISC_KP7,		// 67 - Keypad 7
	ISC_KP8,		// 68 - Keypad 8
	ISC_KP9,		// 69 - Keypad 9
	ISC_KPSTAR,		// 6A - Keypad *
	ISC_KPPLUS,		// 6B - Keypad +
	0,				// 6C - Seperator key
	ISC_KPMINUS,	// 6D - Keypad -
	ISC_KPDELETE,	// 6E - Keypad .
	ISC_KPSLSH,		// 6F - Keypad /
	ISC_F1,			// 70 - F1 key
	ISC_F2,			// 71 - F2 key
	ISC_F3,			// 72 - F3 key
	ISC_F4,			// 73 - F4 key
	ISC_F5,			// 74 - F5 key
	ISC_F6,			// 75 - F6 key
	ISC_F7,			// 76 - F7 key
	ISC_F8,			// 77 - F8 key
	ISC_F9,			// 78 - F9 key
	ISC_F10,		// 79 - F10 key
	ISC_F11,		// 7A - F11 key
	ISC_F12,		// 7B - F12 key
	0,				// 7C - F13 kwy
	0,				// 7D - F14 kwy
	0,				// 7E - F15 kwy
	0,				// 7F - F16 kwy
	0,				// 80
	0,				// 81
	0,				// 82
	0,				// 83
	0,				// 84
	0,				// 85
	0,				// 86
	0,				// 87
	0,				// 88
	0,				// 89
	0,				// 8A
	0,				// 8B
	0,				// 8C
	0,				// 8D
	0,				// 8E
	0,				// 8F
	ISC_NUMLCK,		// 90 - NumLock key
	ISC_SCLLCK,		// 91 - ScrollLock key
	0,				// 92
	0,				// 93
	0,				// 94
	0,				// 95
	0,				// 96
	0,				// 97
	0,				// 98
	0,				// 99
	0,				// 9A
	0,				// 9B
	0,				// 9C
	0,				// 9D
	0,				// 9E
	0,				// 9F
	0,				// A0
	0,				// A1
	0,				// A2
	0,				// A3
	0,				// A4
	0,				// A5
	0,				// A6
	0,				// A7
	0,				// A8
	0,				// A9
	0,				// AA
	0,				// AB
	0,				// AC
	0,				// AD
	0,				// AE
	0,				// AF
	0,				// B0
	0,				// B1
	0,				// B2
	0,				// B3
	0,				// B4
	0,				// B5
	0,				// B6
	0,				// B7
	0,				// B8
	0,				// B9
	ISC_SEMI,		// BA - Semi-colon
	ISC_EQUAL,		// BB - =
	ISC_COMMA,		// BC - Comma
	ISC_MINUS,		// BD - -
	ISC_PER,		// BE - Period
	ISC_SLSH,		// BF - /
	ISC_GRAVE,		// C0 - Grave
	0,				// C1
	0,				// C2
	0,				// C3
	0,				// C4
	0,				// C5
	0,				// C6
	0,				// C7
	0,				// C8
	0,				// C9
	0,				// CA
	0,				// CB
	0,				// CC
	0,				// CD
	0,				// CE
	0,				// CF
	0,				// D0
	0,				// D1
	0,				// D2
	0,				// D3
	0,				// D4
	0,				// D5
	0,				// D6
	0,				// D7
	0,				// D8
	0,				// D9
	0,				// DA
	ISC_LBRKT,		// DB [
	ISC_BSLSH,		// DC Back-slash
	ISC_RBRKT,		// DD ]
	ISC_QUOTE,		// DE '
	0,				// DF
	0,				// E0
	0,				// E1
	0,				// E2 - Not-equal key
	0,				// E3
	0,				// E4
	0,				// E5
	0,				// E6
	0,				// E7
	0,				// E8
	0,				// E9
	0,				// EA
	0,				// EB
	0,				// EC
	0,				// ED
	0,				// EE
	0,				// EF
	0,				// F0
	0,				// F1
	0,				// F2
	0,				// F3
	0,				// F4
	0,				// F5
	0,				// F6
	0,				// F7
	0,				// F8
	0,				// F9
	0,				// FA
	0,				// FB
	0,				// FC
	0,				// FD
	0,				// FE
	0				// FF
};


scnfntdef swcscnfnttbl[FNTMAX+1] =
{	{14, 16, 0, 0, 0, NORMAL, "Times New Roman"},	// FONT_SMTEXT
	{16, 21, 0, 0, 0, NORMAL, "Times New Roman"},	// FONT_MSTEXT
	{20, 26, 0, 0, 0, NORMAL, "Times New Roman"},	// FONT_MDTEXT
	{30, 34, 0, 0, 0, NORMAL, "Times New Roman"},	// FONT_LMTEXT
	{40, 44, 0, 0, 0, NORMAL, "Times New Roman"},	// FONT_LGTEXT
	{13, 16, 0, 0, 0, NORMAL, "Arial"},				// FONT_SMSANS
	{17, 21, 0, 0, 0, NORMAL, "Arial"},				// FONT_MSSANS
	{22, 30, 0, 0, 0, NORMAL, "Arial"},				// FONT_MDSANS
	{30, 36, 0, 0, 0, NORMAL, "Arial"},				// FONT_LMSANS
	{38, 44, 0, 0, 0, NORMAL, "Arial"},				// FONT_LGSANS
	{80, 88, 0, 0, 0, NORMAL, "Arial"},				// FONT_VLSANS
	{14, 16, 0, 0, 0, BOLD  , "Arial"},				// FONT_SMHEADING
	{18, 21, 0, 0, 0, BOLD  , "Arial"},				// FONT_MSHEADING
	{22, 30, 0, 0, 0, BOLD  , "Arial"},				// FONT_MDHEADING
	{30, 36, 0, 0, 0, BOLD  , "Arial"},				// FONT_LMHEADING
	{38, 44, 0, 0, 0, BOLD  , "Arial"},				// FONT_LGHEADING
	{78, 88, 0, 0, 0, BOLD  , "Arial"},				// FONT_VLHEADING
///	{13, 14, 0, 0, 0, NORMAL, "Courier New"},		// FONT_SMFIXED
///	{19, 21, 0, 0, 0, NORMAL, "Courier New"},		// FONT_MSFIXED
///	{22, 24, 0, 0, 0, NORMAL, "Courier New"},		// FONT_MDFIXED
///	{32, 34, 0, 0, 0, NORMAL, "Courier New"},		// FONT_LMFIXED
///	{40, 44, 0, 0, 0, NORMAL, "Courier New"}			// FONT_LGFIXED

	{11, 14, 0, 0, 0, NORMAL, "Lucida Console"},		// FONT_SMFIXED
	{19, 21, 0, 0, 0, NORMAL, "Lucida Console"},		// FONT_MSFIXED
	{22, 24, 0, 0, 0, NORMAL, "Lucida Console"},		// FONT_MDFIXED
	{32, 34, 0, 0, 0, NORMAL, "Lucida Console"},		// FONT_LMFIXED
	{40, 44, 0, 0, 0, NORMAL, "Lucida Console"},		// FONT_LGFIXED
///	{12, 15, 0, 0, 0, NORMAL, "Lucida Sans Typewriter"},		// FONT_SMFIXED
///	{19, 21, 0, 0, 0, NORMAL, "Lucida Sans Typewriter"},		// FONT_MSFIXED
///	{22, 24, 0, 0, 0, NORMAL, "Lucida Sans Typewriter"},		// FONT_MDFIXED
///	{32, 34, 0, 0, 0, NORMAL, "Lucida Sans Typewriter"},		// FONT_LMFIXED
///	{40, 44, 0, 0, 0, NORMAL, "Lucida Sans Typewriter"},		// FONT_LGFIXED

	{13, 16, 0, 0, 0, NORMAL, "Arial Narrow"},		// FONT_SMSANSN
	{17, 21, 0, 0, 0, NORMAL, "Arial Narrow"},		// FONT_MSSANSN
	{22, 30, 0, 0, 0, NORMAL, "Arial Narrow"},		// FONT_MDSANSN
	{30, 36, 0, 0, 0, NORMAL, "Arial Narrow"},		// FONT_LMSANSN
	{38, 44, 0, 0, 0, NORMAL, "Arial Narrow"},		// FONT_LGSANSN
	{80, 88, 0, 0, 0, NORMAL, "Arial Narrow"}		// FONT_VLSANSN
};

fontblk    *fontfirst = NULL;

char *swcfontnametbl[] =
{	NULL,						//              = 0z00000000
	"Times New Roman",			// FONT_TEXT    = 0x10000000
	"Arial",					// FONT_HEADING = 0x20000000
	"Lucida Console",			// FONT_FIXED   = 0x30000000
	"Abri Barcode39N",			// FONT_BARCODE = 0x40000000
	"Control",					// FONT_CONTROL = 0x50000000
	"Arial Narrow",				// FONT_NARROW  = 0x60000000
	"Times New Roman",			//              = 0x70000000
	"Times New Roman",			//              = 0x80000000
	"Times New Roman",			//              = 0x90000000
	"Times New Roman",			//              = 0xA0000000
	"Times New Roman",			//              = 0xB0000000
	"Times New Roman",			//              = 0xC0000000
	"Times New Roman",			//              = 0xD0000000
	"Times New Roman",			//              = 0xE0000000
	"Times New Roman"			//              = 0xF0000000
};

int swcfontweighttbl[] =
{	FW_NORMAL,					// TXT_NORMAL = 0x00000000
	FW_THIN,					// TXT_THIN   = 0x01000000
	FW_EXTRALIGHT,				// TXT_XLIGHT = 0x02000000
	FW_LIGHT,					// TXT_LIGHT  = 0x03000000
	FW_MEDIUM,					// TXT_MEDIUM = 0x04000000
	FW_SEMIBOLD,				// TXT_DBOLD  = 0x05000000
	FW_BOLD,					// TXT_BOLD   = 0x06000000
	FW_EXTRABOLD,				// TXT_XBOLD  = 0x07000000
	FW_HEAVY,					// TXT_HEAVY  = 0x08000000
	FW_NORMAL,					//            = 0x09000000
	FW_NORMAL,					//            = 0x0A000000
	FW_NORMAL,					//            = 0x0B000000
	FW_NORMAL,					//            = 0x0C000000
	FW_NORMAL,					//            = 0x0D000000
	FW_NORMAL,					//            = 0x0E000000
	FW_NORMAL					//            = 0x0F000000
};

long        amountalloc;
long        numberalloc;
long        sfd = 40;
long        sfr = 20;
long        swcbasexoffset;
long        swcbaseyoffset;
void      (*swc_WindowGone)(swcBase *win) = NULL;
void      (*swc_ShiftStateHook)(int down) = NULL;
void      (*swc_ControlStateHook)(int down) = NULL;
int       (*swc_BaseClose)() = NULL;
void      (*swc_AllDone)() = NULL;
void      (*swc_DeviceNotify)(UINT msg, UINT wparm, LONG lparm);
swcTopLevel swc_BaseWin;
swcDisplay *winblkpnt;
RECT        swc_ScrnRect;

swcDisplayBase *swcwheelwin;

uint        swc_basetid;


HINSTANCE   hinst;
HCURSOR     harrow;
HCURSOR     hhand;
HCURSOR     htext;
HCURSOR     hwait;
HACCEL      hacccur = 0;
MSG         msg;

uchar       swc_NumLockDisable = false;
uchar       swc_NumLockInserted = false;
uchar       swc_ForceFocusEnable = false;
uchar       swc_MouseFocus = false;
uchar       swc_OSType = 0;

char        swc_rptfocus = true;
char        fatalfunc;
char        crashed = false;	// TRUE if have fatal error
char        inmenu = false;
char        menuhand = false;
char        mbdmenu = false;

char        online = false;
char        needdiscon = 0;		// 0 = Not connected, 1 = Connected,
								//   2 = Disconnecting
char        needexit = false;	// TRUE if need to exit
uchar       swc_Ending = false;	// TRUE if terminating


char        banner[120];

char        swc_ProgDir[256];
char       *swc_ProgName;
char       *swc_IniFileSpec;

static void dosetup(LPSTR cmdparam);
static int  CALLBACK findfontproc(CONST LOGFONT *, CONST TEXTMETRIC *, DWORD,
		LPARAM);

#define MSGTBLSZ 32


struct msgent
{	void (*func)(MSG *msg, void *pntr);
	void  *pntr;
};

msgent msgtbl[MSGTBLSZ];


long  memring[1024];
long *memrpnt = memring;


void *operator new(size_t size)

{
	long *mem;

	memrpnt[0] = 0;
	memrpnt[1] = size;
	memrpnt += 2;
	if (memrpnt >= (memring + 1024))
		memrpnt = memring;

	size = (size + 3) & 0xFFFFFFFC;
	amountalloc += size;
	numberalloc++;
	mem = (long *)malloc(size + 12);
	if (mem != NULL)
	{
		*mem++ = 0xF0F03CC3;
		*mem++ = size;
		mem[size/4] = 0xF0F02CC2;
	}
	return (mem);
}

long memerrorcnt;

void operator delete(void *p)

{
	long size;

	memrpnt[0] = 1;
	memrpnt[1] = ((long *)p)[-1];
	memrpnt += 2;
	if (memrpnt >= (memring + 1024))
		memrpnt = memring;
	if (((long *)p)[-2] != 0xF0F03CC3)
	{
///		int3();
		memerrorcnt++;
		return;
	}
	size = ((long *)p)[-1];
	if ((size & 0x03) != 0 || ((long *)p)[size/4] != 0xF0F02CC2)
	{
///		int3();
		memerrorcnt++;
		return;
	}
	amountalloc -= size;
	numberalloc--;
	memset(((long *)p) - 2, 0xFE, size + 12);
	free (((long *)p) - 2);
}

char *xxxx;

//******************************************
// Function: WinMain - Main windows function
// Returned: Return value
//******************************************

int APIENTRY WinMain(
    HINSTANCE hinstance,
    HINSTANCE hprevinstance,
    LPSTR     cmdparam,
    int       ncmdshow)

{
	ncmdshow = ncmdshow;

    hinst = hinstance;					// Make the instance handle globably
										//   available
	dosetup(cmdparam);




	swcAppMain();
    while (GetMessage(&msg, 0, 0, 0))
		swchandlemessage(&msg);
	if (swc_AllDone != NULL)
		swc_AllDone();
	while (fontfirst != NULL)
	{
		RemoveFontResource(fontfirst->file);
		fontfirst = fontfirst->next;
	}
    return (msg.wParam);
}



static void dosetup(
	LPSTR     cmdparam)
{
    RECT       rect;
    WNDCLASSEX wndclass;
    LOGFONT    logfnt;
	HFONT      font;
    HDC        hdc;
    TEXTMETRIC metrics;
	OSVERSIONINFO verinfo;
    scnfntdef *fpnt;
	char      *cpnt;
    int        cnt;
    int        nlen;
	uchar      needdebug = false;
	uchar      dualhead = false;
	char       text[32];

    if ((nlen = GetModuleFileName(hinst, swc_ProgDir,
            sizeof(swc_ProgDir) - 40)) > 0)
    {
	    if ((swc_ProgName = strrchr(swc_ProgDir, '\\')) != NULL)
		    swc_ProgName++;
	    else
        {
            if ((swc_ProgName = strrchr(swc_ProgDir, ':')) != NULL)
		        swc_ProgName++;
            else
                swc_ProgName = swc_ProgDir;
        }
    }
    else
    {
        strcpy(swc_ProgDir, "???");
        nlen = 3;
    }
    swc_IniFileSpec = new char[nlen + 5];
    if ((cpnt = strrchr(swc_ProgDir, '.')) != NULL)
        sprintf(swc_IniFileSpec, "%.*s.ini", cpnt - swc_ProgDir, swc_ProgDir);
    else
        strmov(strmov(swc_IniFileSpec, swc_ProgDir), ".ini");

    harrow = LoadCursor(0, IDC_ARROW);
    hhand = LoadCursor(hinst, "HAND");
    htext = LoadCursor(hinst, "TEXT");
    hwait = LoadCursor(0, IDC_WAIT);
	GetClientRect(GetDesktopWindow(), &rect);

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW|CS_VREDRAW|CS_CLASSDC;
	wndclass.lpfnWndProc = swcDisplayBase::displayproc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 4;
	wndclass.hInstance = hinst;
	wndclass.hIconSm = wndclass.hIcon = LoadIcon(hinst, "APPICON");
	wndclass.hCursor = harrow;
	wndclass.hbrBackground = 0;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "swcMAIN";
	RegisterClassEx(&wndclass);
	wndclass.style = CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc = swcDisplayBase::displayproc;
	wndclass.lpszClassName = "swcDISPLAY";
	RegisterClassEx(&wndclass);
	wndclass.style = CS_HREDRAW|CS_VREDRAW|CS_CLASSDC|CS_DBLCLKS;
	wndclass.lpszClassName = "swcDISPLAYDC";
	RegisterClassEx(&wndclass);

	verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&verinfo))
		swc_OSType = (uchar)((verinfo.dwMajorVersion > 4) ? SWC_OSTYPE_WINXP :
				SWC_OSTYPE_WIN98);

	// Process command line arguments. Command format is:
	//		{width} {D}
	//   "width" is width of the main window. 0 or not specified means to use
	//   the screen width. Supported widths are 800 and 1024. "D" if present,
	//   means to create a debug window.

	cnt = 0;
	needdebug = false;
	if ((cpnt = strtok(cmdparam, " ")) != NULL)
	{
		if (toupper(cpnt[0]) == 'D' && cpnt[1] == 0)
			needdebug = true;
		else if (toupper(cpnt[0]) == 'H' && cpnt[1] == 0)
		{
			dualhead = true;
			swc_WinType = SWC_WINTYPE_NORMAL;
		}
		else
		{
			cnt = atoi(cpnt);
			if ((cpnt = strtok(NULL, " ")) != NULL)
			{
				if (toupper(cpnt[0]) != 'D' || cpnt[1] != 0)
					needdebug = true;
			}
		}
	}
	if (cnt == 0)
	{
		swcGetIniString("DISPLAY", "HORIZ", "", text, sizeof(text)),
		cnt = atol(text);
	}
	if (cnt == 0)
	{
		cnt = rect.bottom;
		sfd = 57600 / cnt;
	}
	else
        sfd = 76800 / cnt;
	sfr = sfd / 2;

	swc_ScrnRect.left = rect.left * sfd;
	swc_ScrnRect.right = rect.right * sfd;
	swc_ScrnRect.top = rect.top * sfd;
	swc_ScrnRect.bottom = rect.bottom * sfd;
	if (swc_XPos == 0x80000000)
		swc_XPos = (swc_ScrnRect.right - swc_ScrnRect.left - swc_XSize)/2;
	if (swc_YPos == 0x80000000)
		swc_YPos = (swc_ScrnRect.bottom - swc_ScrnRect.top - swc_YSize)/2;

	swc_basetid = GetCurrentThreadId();

///	if (swc_WinType == SWC_WINTYPE_FULLSCREEN)
///	{
///		swcbasexoffset = (rect.right - sf * 200)/2;
///		swcbaseyoffset = (rect.bottom - sf * 150)/2;
///	}
///	else
		swcbasexoffset = swcbaseyoffset = 0;

	swc_BaseWin.hwin = CreateWindow("swcMAIN", swc_AppMainBanner,
			(swc_WinType == SWC_WINTYPE_FULLSCREEN) ? (WS_POPUP|WS_CLIPCHILDREN|
			WS_CLIPSIBLINGS) : (WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|
			WS_CLIPSIBLINGS), (swc_XPos + sfr) / sfd, (swc_YPos + sfr) / sfd,
			(swc_XSize + sfr) / sfd, (swc_YSize + sfr) / sfd, 0, 0, hinst, 0);
    SetWindowLong(swc_BaseWin.hwin, GWL_USERDATA, (LONG)&swc_BaseWin);
    GetClientRect(swc_BaseWin.hwin, &rect);
	swc_BaseWin.width = swc_XSize;
	swc_BaseWin.height = swc_YSize;
	swc_BaseWin.cwidth = rect.right * sfd;
	swc_BaseWin.cheight = rect.bottom * sfd;
///    swc_BaseWin.xpos = swc_BaseWin.ypos = 0;
///    swc_BaseWin.width = rect.right;
///    swc_BaseWin.height = rect.bottom;
    swc_BaseWin.txtcolor = 0x000000;
    swc_BaseWin.bgcolor = 0xD0D0D0;
	swc_BaseWin.parent = NULL;
	swc_BaseWin.sibling = NULL;
    UpdateWindow(swc_BaseWin.hwin);
    SetFocus(swc_BaseWin.hwin);
	if (swc_WinType == SWC_WINTYPE_FULLSCREEN)
		SetWindowOrgEx(GetDC(swc_BaseWin.hwin), -swcbasexoffset,
				-swcbaseyoffset, NULL);
    ShowWindow(swc_BaseWin.hwin, (swc_WinType == SWC_WINTYPE_FULLSCREEN) ?
			SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
    hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
    memset(&logfnt, 0, sizeof(logfnt));
    fpnt = swcscnfnttbl + FNTMAX;
    cnt = FNTMAX;
    do
    {
		logfnt.lfHeight = 20;	/////// (sf == 4) ? fpnt->size4 : fpnt->size5;
		logfnt.lfWeight = fpnt->weight;
		logfnt.lfItalic = fpnt->italic;
		strcpy(logfnt.lfFaceName, fpnt->name);
		font = CreateFontIndirect(&logfnt);
		SelectObject(hdc, font);
		GetTextMetrics(hdc, &metrics);
		fpnt->actual = (ushort)(metrics.tmHeight);
		SelectObject(hdc, GetStockFont(SYSTEM_FONT));
		DeleteObject(font);
		fpnt--;
    } while (--cnt >= 0);
    DeleteDC(hdc);
	if (needdebug)
	{
		wndclass.style = CS_HREDRAW|CS_VREDRAW;
		wndclass.hIcon = LoadIcon(hinst, MAKEINTRESOURCE(102));
		wndclass.lpfnWndProc = swcDisplayBase::displayproc;
		wndclass.lpszClassName = "swcDEBUG";
		RegisterClassEx(&wndclass);
		new swcDebug();
	}
}


int swcAddTrueTypeFont(
	char *name,
	char *file)

{
	HDC      hdc;
	int      rtn;
	int      havefont;
	fontblk *fb;

    hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	havefont = FALSE;
	EnumFontFamilies(hdc, name, &findfontproc, (LPARAM)&havefont);
    DeleteDC(hdc);
	if (havefont)
		return (-1);
	if ((rtn = AddFontResource(file)) <= 0)
		return (0);
	fb = new fontblk();
	strncpy(fb->file, file, 63);
	fb->file[63] = 0;
	fb->next = fontfirst;
	fontfirst = fb;
	return (rtn);
}


static int CALLBACK findfontproc(
	CONST LOGFONT    *elf,
	CONST TEXTMETRIC *ntm,
	DWORD             fonttype,
	LPARAM            lparam)

{
	NOTUSED(elf);
	NOTUSED(ntm);
	NOTUSED(fonttype);

	*(int *)lparam = true;
	return (true);
}


long thdmsgcnt;
long thderror;

void swchandlemessage(
	MSG *msg)

{
	swcBase        *pwin;
	swcDisplayBase *dwin;
	swcTopLevel    *tlwin;
	int             inx;

	if (msg->hwnd == NULL)
	{
		if (msg->message == (WM_USER + 50))
		{
			tlwin = (swcTopLevel *)(msg->lParam);
			if ((pwin = tlwin->ffwindow) != NULL)
			{
				if (pwin->wchild != GetFocus())
					SetFocus(pwin->hwin);
				if (pwin->type == SWC_WT_RESPONSE &&
						(((swcResponse *)pwin)->options & SWC_RESPONSE_SELALL))
					Edit_SetSel(pwin->hwin, 0, 0x7FFF);
				if (pwin->type == SWC_WT_DROPDOWN &&
						(((swcDropDown *)pwin)->bits & SWC_DROPDOWN_AUTO))
				{
					((swcDropDown *)pwin)->changed = false;
					ComboBox_ShowDropdown(pwin->hwin, true);
				}
				while ((dwin = pwin->parent) != NULL)
				{
					if (swc_rptfocus && dwin->havefocus(tlwin->ffwindow))
						break;
					pwin = dwin;
				}
			}
			if (swc_ShiftStateHook != NULL)
				swc_ShiftStateHook((GetKeyState(VK_SHIFT) & 0x80000000) != 0);
			if (swc_ControlStateHook != NULL)
				swc_ControlStateHook((GetKeyState(VK_CONTROL) &
						0x80000000) != 0);
		}
		else if (msg->message == (WM_USER + 51)) // Set selection in dropdown?
										// Yes - this is needed since the
										//   windows combobox control does not
										//   work right if the position is set
										//   in certain message processing
										//   routines.
			((swcDropDown *)(msg->lParam))->SetSelection(msg->wParam);
/*
		else if (msg->message == (WM_USER + 100))
			((winnetwork *)(msg->lParam))->networkmsg100(msg);
		else if (msg->message == (WM_USER + 101))
			((winnetwork *)(msg->lParam))->networkmsg101(msg);
*/
		else if (msg->message == (WM_APP + 111))
			((swcDisplayBase *)(msg->lParam))->msg111(msg);
		else if (msg->message == (WM_APP + 112))
			((swcDisplayBase *)(msg->lParam))->msg112(msg);

        else if (msg->message >= WM_APP + 149)
        {
			inx = msg->message - (WM_APP + 150);
		    if (inx >= 0 && inx < MSGTBLSZ &&
				    msgtbl[inx].func != NULL)
            {
                if (thdmsgcnt != 0)
                    thderror++;
                thdmsgcnt++;
			    (msgtbl[inx].func)(msg, msgtbl[inx].pntr);
                thdmsgcnt--;
            }
        }
	}
	if (!TranslateAccelerator(swc_BaseWin.hwin, hacccur, msg))
	{
		if (swc_NumLockDisable && msg->message == WM_KEYDOWN)
		{
			if (swc_NumLockInserted && ((uchar)(msg->lParam >> 16)) == 0x45)
			{
				swc_NumLockInserted = false;
				return;
			}
			if ((msg->lParam & 0x1000000) == 0)
			{
				INPUT  input[2];
				int    oemkey;
				static uchar keytbl[] = {0x67, 0x68, 0x69, 0x6D, 0x64, 0x65, 0x66,
						0x6B, 0x61, 0x62, 0x63, 0x60, 0x6E};

				oemkey = (uchar)(msg->lParam >> 16);
				if (oemkey >= 0x47 && oemkey <= 0x53)
				{
					if (msg->wParam != keytbl[oemkey - 0x47])
					{
						input[0].type = INPUT_KEYBOARD;
						input[0].ki.dwExtraInfo = 0;
						input[0].ki.dwFlags = 0;
						input[0].ki.time = 0;
						input[0].ki.wScan = 0x45;
						input[0].ki.wVk = 0x90;
						input[1].type = INPUT_KEYBOARD;
						input[1].ki.dwExtraInfo = 0;
						input[1].ki.dwFlags = KEYEVENTF_KEYUP;
						input[1].ki.time = 0;
						input[1].ki.wScan = 0x45;
						input[1].ki.wVk = 0x90;
						swc_NumLockInserted = true;
///						SendInput(2, input, sizeof(INPUT));
						msg->wParam = keytbl[oemkey - 0x47];
					}
				}
			}
		}
		if ((msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN ||
				msg->message == WM_KEYUP || msg->message == WM_SYSKEYUP) &&
				(swcvirkey2xos[msg->wParam] & 0x80))
		{
			if (swc_ShiftStateHook != NULL &&
					(swcvirkey2xos[msg->wParam] & 0x7F) == ISC_LFTSHF)
				swc_ShiftStateHook(msg->message == WM_KEYDOWN ||
						msg->message == WM_SYSKEYDOWN);
			if (swc_ControlStateHook != NULL &&
					(swcvirkey2xos[msg->wParam] & 0x7F) == ISC_LFTCTL)
				swc_ControlStateHook(msg->message == WM_KEYDOWN ||
						msg->message == WM_SYSKEYDOWN);
		}
		if (msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN)
		{
			if ((pwin = (swcBase *)GetWindowLong(msg->hwnd, GWL_USERDATA))
    				!= NULL && pwin->checkforkey(msg->wParam))
	    		return;
		}
		TranslateMessage(msg);
		DispatchMessage(msg);
	}
}


long swcconvertbits(
	long  bits,
	long *exbits)

{
	long value;
	long exvalue;

	value = 0;
	exvalue = 0;
	switch (bits & 0x0000F000)			// Title bar must only be specified
	{									//   for toplevel windows
	 case SWC_TTLBAR_CLS:
		value |= WS_DLGFRAME;
		break;

	 case SWC_TTLBAR_FULL:
		value |= WS_OVERLAPPEDWINDOW;
		break;
	}
	switch (bits & 0x00000FF0)
	{
	 case SWC_BORDER_NONE:
		break;

	 case SWC_BORDER_LINE_SM:
	 case SWC_BORDER_LINE_MD:
	 case SWC_BORDER_LINE_LG:
		value |= WS_BORDER;
		break;

	 case SWC_BORDER_IN_SM:				// This only works for child windows
	 case SWC_BORDER_IN_MD:
	 case SWC_BORDER_IN_LG:
		value |= WS_BORDER;
		exvalue |= WS_EX_CLIENTEDGE;
		break;

	 case SWC_BORDER_OUT_SM:
	 case SWC_BORDER_OUT_MD:
	 case SWC_BORDER_OUT_LG:
		value |= WS_DLGFRAME;
			break;

	 case SWC_BORDER_FRM_SM:			// This only works for overlapped
	 case SWC_BORDER_FRM_MD:			//   windows
	 case SWC_BORDER_FRM_LG:
		value |= WS_DLGFRAME;
		exvalue |= WS_EX_CLIENTEDGE;
		break;
	}
	switch (bits & 0x000F0000)
	{
	 case SWC_SCROLL_VERT_A:			// Vertical scroll bar (always
										//   displayed)
		value |= (WS_VSCROLL | LBS_DISABLENOSCROLL);
		break;

	 case SWC_SCROLL_VERT_C:			// Vertical scroll bar (conditionally
		value |= WS_VSCROLL;			//   displayed)
		break;

	 case SWC_SCROLL_HORIZ_A:			// Horizontal scroll bar (always
										//   displayed)
		value |= (WS_HSCROLL | LBS_DISABLENOSCROLL);
		break;

	 case SWC_SCROLL_HORIZ_C:			// Horizontal scroll bar (conditionally
		value |= WS_HSCROLL;			//   displayed)
		break;
	}
	if ((bits & SWC_STATE_ENABLE) == 0)
		value |= WS_DISABLED;
	if (bits & 0x40000000)
		value |= TCS_BUTTONS;
	*exbits = exvalue;
	return (value);
}



void swcGrabWheel(
	swcDisplayBase *win)
{
	swcwheelwin = win;
}


//********************************************************
// Function: swcRespRegFunc - Register a response function
// Returned: Message number (0 if no more available)
//********************************************************

int swcRespRegFunc(
	void (*func)(MSG *msg, void *pntr),
	void  *pntr)

{
	int num;

	// First see if the function is already in the table

	num = 0;
	do
	{
		if (msgtbl[num].func == func && msgtbl[num].pntr == pntr)
										// This one?
			return (num + 1);			// Yes - just return it
	} while (++num < MSGTBLSZ);

	// Here if did not find the function in the table - look for an empty slot
	//   to use

	num = 0;
	do
	{
		if (msgtbl[num].func == NULL)
		{
			msgtbl[num].func = func;
			msgtbl[num].pntr = pntr;
			return (num + 1);
		}
	} while (++num < MSGTBLSZ);
	return (0);							// If no empty slots, indicate failure
}


//****************************************
// Function: swcRespSend - Send a response
// Returned:
//****************************************

int swcRespSend(
	int  msgnum,
	long value1,
	long value2)

{
	return (PostThreadMessage(swc_basetid, WM_APP + 149 + msgnum, value1,
			value2));
}


long swcXlateCErr()

{
	static long errtbl[] =
	{	0,
		ER_ERROR,		// EPERM         1          Not owner
		ER_FILNF,		// ENOENT        2        *+No such file or directory
		ER_NSP,			// ESRCH         3          No such process
		ER_ERROR,		// EINTR         4          Interrupted system call
		ER_DEVER,		// EIO           5          I/O error
		ER_NSDEV,		// ENXIO         6          No such device or address
		ER_NTLNG,		// E2BIG         7        *+Argument list too long
		ER_IDFMT,		// ENOEXEC       8        *+Exec format error
		ER_BDDVH,		// EBADF         9        *+Bad file number
		ER_NSP,			// ECHILD       10         +No spawned processes
		ER_NEMA,		// EAGAIN       11         +No more processes; not enough memory;
						//                            maximum nesting level reached
		ER_NEMA,		// ENOMEM       12        *+Not enough memory
		ER_PRIV,		// EACCES       13        *+Permission denied
		ER_ADRER,		// EFAULT       14          Bad address
		ER_NTDSK,		// ENOTBLK      15          Block device required
		ER_BUSY,		// EBUSY        16          Mount device busy
		ER_FILEX,		// EEXIST       17        *+File exists
		ER_DFDEV,		// EXDEV        18        *+Cross-device link
		ER_NSDEV,		// ENODEV       19          No such device
		ER_NTDIR,		// ENOTDIR      20          Not a directory
		ER_ISDIR,		// EISDIR       21          Is a directory
		ER_VALUE,		// EINVAL       22        *+Invalid argument
		ER_TMDVP,		// ENFILE       23          File table overflow
		ER_TMDVP,		// EMFILE       24        *+Too many open files
		ER_NTTRM,		// ENOTTY       25          Not a teletype
		ER_BUSY,		// ETXTBSY      26          Text file busy
		ER_FIL2L,		// EFBIG        27          File too large
		ER_DEVFL,		// ENOSPC       28        *+No space left on device
		ER_SEKER,		// ESPIPE       29          Illegal seek
		ER_WPRER,		// EROFS        30          Read-only file system
		ER_ERROR,		// EMLINK       31          Too many links
		ER_BPIPE,		// EPIPE        32          Broken pipe
		ER_MATH,		// EDOM         33        *+Math argument
		ER_RANGE,		// ERANGE       34        *+Result too large
		ER_ERROR,		// EUCLEAN      35          File system needs cleaning
		ER_DLOCK,		// EDEADLK      36         +Resource deadlock would occur
	};

	long code;

	code = errno;
	return ((code > 36) ? ER_ERROR : errtbl[code]);
}


int swcGetIniString(
	char *section,
	char *keyname,
	char *dfltstr,
	char *buffer,
	int   length)

{
	return (GetPrivateProfileString(section, keyname, dfltstr, buffer, length,
			swc_IniFileSpec));
}

int swcPutIniString(
	char *section,
	char *keyname,
	char *buffer)

{
	return (WritePrivateProfileString(section, keyname, buffer, swc_IniFileSpec));
}


swcBase *swcGetFocus()

{
	HWND hwnd;

	if ((hwnd = ::GetFocus()) != 0)
		return ((swcBase *)GetWindowLong(hwnd, GWL_USERDATA));
	return (NULL);
}



void swcTerminate(void)

{
	swc_Ending = true;
	DestroyWindow(swc_BaseWin.hwin);
}

void swcBeep(
	int freq,
	int length)

{
	Beep(freq, length);
}


// The fontnum is encoded as follows
//  0xF0000000 - Font index
//  0x0F000000 - Font weight
//  0x00F00000 - Attributes
//  0x000F0000 - Reserved, must be 0
//  0x0000FFFF - Font height in logical units.
// If the font index is 0, an old-style font is specified with the font number
//   in the low 8 bits. Other bits are not used.

HFONT swcgetscnfont(
	int fontnum)

{
    LOGFONT logfnt;

    memset(&logfnt, 0, sizeof(logfnt));
	if (fontnum & 0xF0000000)
	{
		logfnt.lfHeight = ((fontnum & 0xFFFF) + sfr) / sfd;
		logfnt.lfWeight = swcfontweighttbl[(fontnum >> 24) & 0x0F];
		logfnt.lfItalic = ((fontnum & TXT_ITALIC) != 0);
		logfnt.lfUnderline = ((fontnum & TXT_UNDERLINE) != 0);
		strcpy(logfnt.lfFaceName, swcfontnametbl[fontnum >> 28]);
	}
	else
	{
		logfnt.lfHeight = swcgetfontact(fontnum);
		logfnt.lfWeight = swcscnfnttbl[fontnum].weight;
		logfnt.lfItalic = swcscnfnttbl[fontnum].italic;
		strcpy(logfnt.lfFaceName, swcscnfnttbl[fontnum].name);
	}
	return (CreateFontIndirect(&logfnt));
}


long swcgetfontact(
	int fontnum)

{
	if (fontnum & 0xF0000000)
		return (((fontnum & 0xFFFF) + sfr) / sfd);
	else
		return (((swcscnfnttbl[fontnum].size5 * 50) + sfr) / sfd);
}


swcRgn *swcCreateRectRgn(
	long hpos,
	long vpos,
	long width,
	long height)

{
	return (CreateRectRgn(hpos, vpos, hpos + width, vpos + height));
}

void swcDeleteRgn(
	swcRgn *rgn)

{
	DeleteRgn(rgn);
}


struct trmdef
{	void (*func)(long tmr, long arg1, void *arg2);
	long   arg1;
	void  *arg2;
	ulong  tmr;
	int    rpt;
} tmrtable[10];


void CALLBACK alarmfunc(
  HWND hwnd,
  UINT msg,
  UINT tmr,
  DWORD dwTime);


long swcAlarmSet(
	ulong  time,
	int    rpt,
	void (*func)(long tmr, long arg1, void *arg2),
	long   arg1,
	void  *arg2)

{
	int inx;

	inx = 0;
	do
	{
		if (tmrtable[inx].func == NULL)
		{
			tmrtable[inx].func = func;
			tmrtable[inx].arg1 = arg1;
			tmrtable[inx].arg2 = arg2;
			tmrtable[inx].rpt = rpt;
			tmrtable[inx].tmr = SetTimer(0, 0, time, alarmfunc);
			return (inx + 1);
		}
	} while (++inx <= 9);
	return (0);
}


void CALLBACK alarmfunc(
  HWND hwnd,
  UINT msg,
  UINT tmr,
  DWORD dwTime)

{
	void (*func)(long tmr, long arg1, void *arg2);
	int    inx;

	inx = 0;
	do
	{
		if (tmrtable[inx].tmr == tmr)
		{
			func = tmrtable[inx].func;
			if (!tmrtable[inx].rpt)
			{
				tmrtable[inx].func = NULL;
				KillTimer(0, tmr);
			}
			func(inx + 1, tmrtable[inx].arg1, tmrtable[inx].arg2);
			return;
		}
	} while (++inx <= 9);
	KillTimer(0, tmr);
}


void swcAlarmKill(
	long inx)

{
	if (tmrtable[--inx].func != NULL)
	{
		tmrtable[inx].func = NULL;
		KillTimer(0, tmrtable[inx].tmr);
	}
}


//********************************************
// Function: swcSetNumLock - Set NUMLOCK state
// Returned: Previous NUMLOCK state
//********************************************

// NOTE: This function does not work with NT (including XP). It does
//       not seem to do any harm however.

int swcSetNumLock(
	int state)

{
	int   value;
	uchar keystates[256];

	GetKeyboardState(keystates);
	value = keystates[VK_NUMLOCK];
	keystates[VK_NUMLOCK] = 0x01;
	SetKeyboardState(keystates);
	return (state);
}



void nullfunc(void)
{
}

int rtnzero(void)

{
    return (0);
}
