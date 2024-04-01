//++++
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

#include "xosmkinst.h"

#include <dbt.h>

#include "crtdbg.h"

long         swc_XSize = 48000;
long         swc_YSize = 35000;
long         swc_XPos  = 0x80000000;
long         swc_YPos  = 0x80000000;

swcDisplay  *winhead;
swcDisplay  *winmain;
swcDisplay  *winfoot;
swcDisplay  *winprog;
swcButton   *btnstart;
swcList     *lstdev;
swcResponse *rspfile;

__int64 curadone;
__int64 adone;
__int64 amount;

uchar *dskbufr;

HANDLE scandisksthd;
uint   scandisksid;
HANDLE installthd;
uint   installid;

HEADBUFR itemhead;

XIDHEAD  xishead = {{'X', 'I', 'S', 'F'}, 20, 2};

HWND gofnhndl;

char maintext[] = "This program initializes a USB disk as a bootable XOS "
		"system/install disk.  The\ncontents of the disk are taken from a "
		"compressed XOS install data file (XID).";
char warntext[] = "WARNING: The entire current contents of the selected\n"
		"USB target disk will be erased and will probably not\n"
		"be recoverable!";

char        *direndpnt;

RECT    baserect;
RECT    apprect;
RECT    footrect;
RECT    progrect;
int     mainwid;

uint    devnum;
long    devbits;
__int64 devsize;

char     xisname[300];		// Name of install set being copied

int      drvltr;
char     filespec[512];
char     xidspec[512];

static char dirspec[650];

static UINT APIENTRY gfnhook(HWND hWnd, UINT uMsg, WPARAM wParam,LPARAM lParam);

OPENFILENAME opnfile =
{	sizeof(OPENFILENAME),		// DWORD         lStructSize;
	0,							// HWND          hwndOwner;
    0,							// HINSTANCE     hInstance;
	"*.XID (XOS Install Data)\0*.xid\0*.* (All files)\0*.*\0",
								// LPCTSTR       lpstrFilter;
    0,							// LPTSTR        lpstrCustomFilter;
	0,							// DWORD         nMaxCustFilter;
	0,							// DWORD         nFilterIndex;
	xidspec,					// LPTSTR        lpstrFile;
	sizeof(xidspec),			// DWORD         nMaxFile;
	NULL,						// LPTSTR        lpstrFileTitle;
	0,							// DWORD         nMaxFileTitle;
	dirspec,					// LPCTSTR       lpstrInitialDir;
	"Select input file",		// LPCTSTR       lpstrTitle; 
	OFN_ENABLESIZING|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOTESTFILECREATE|
			OFN_ENABLEHOOK,		// DWORD         Flags;
	0,							// WORD          nFileOffset; 
	0,							// WORD          nFileExtension;
	"XID",						// LPCTSTR       lpstrDefExt;
	0,							// DWORD         lCustData;
	gfnhook,					// LPOFNHOOKPROC lpfnHook;
	NULL						// LPCTSTR       lpTemplateName;
};


uchar        swc_WinType = SWC_WINTYPE_NORMAL;
uchar        terminate;

#define MAJORVER 1
#define MINORVER 2
#define EDITNUM  0

#define verstr1(maj,min,edit) verstr2(maj,min,edit)
#define verstr2(maj,min,edit) "XOS Boot/Install Disk Creator v" #maj "." #min \
		"." #edit
char  swc_AppMainBanner[] = verstr1(MAJORVER, MINORVER, EDITNUM);
char  swc_AppIniFileName[] = "makexosboot";

char   **disklist;

DEVITEM *devhead;

uchar   *bootimg;

int      stsnum;
int      targetnum;

static void devnotify(UINT message, UINT wparm, LONG lparm);

static uchar firsthook;

static void haveexitbutton(swcBase *win, long arg1, long arg2, void *arg3);
static void havestartbutton(swcBase *win, long arg1, long arg2, void *arg3);
static void havebrowsebutton(swcBase *win, long arg1, long arg2, void *arg3);

static void dodevice(uint dnum, long mask);
static int  digits(double value);
static int  baseclose();

static void status(MSG  *msg, void *pntr);
static void target(MSG  *msg, void *pntr);

static void progdraw(swcDisplayBase *win, HDC hdc);


HANDLE  phyhndl = (HANDLE)0xFFFFFFFF;		// Physical disk handle
HANDLE  hndltbl[25];						// Partition handle table
HANDLE  xishndl = (HANDLE) 0xFFFFFFFF;
HANDLE  xidhndl = (HANDLE) 0xFFFFFFFF;
HANDLE  hndl;
HANDLE  dhndl = (HANDLE)0xFFFFFFFF;
HANDLE  waithndl;

llong  tzone;

int xxxx;


void swcAppMain()
{
///	SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_ALWAYS_DF );
///	SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
///	afxMemDF |= checkAlwaysMemDF;

	GUID interfaceguid = {0x53F5630D, 0xB6BF, 0x11D0, 0x94, 0xF2, 0x00, 0xA0,
			0xC9, 0x1E, 0xFB, 0x8B};
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

	swcButton *btn;

	_crtDbgFlag |= 0x05;

	swc_BaseClose = baseclose;

	swc_ForceFocusEnable = true;
	swc_MouseFocus = true;

	swc_ForceFocusEnable = false;
	swc_MouseFocus = false;

	_tzset();
	tzone = _timezone * (llong)10000000;

	SendMessage(swc_BaseWin.hwin, WM_SETICON, ICON_SMALL, (LPARAM)

	LoadIcon(hinst, MAKEINTRESOURCE(111)));

    GetModuleFileName(hinst, dirspec, sizeof(dirspec) - 32);
    if ((direndpnt = strrchr(dirspec, '\\')) != NULL ||
			(direndpnt = strrchr(dirspec, ':')) != NULL)
		direndpnt++;
	else
		direndpnt = dirspec;
	strcpy(direndpnt, "default.bmp");
    swc_BaseWin.GetClientRect(&baserect);
    mainwid = baserect.right - baserect.left;
	swc_BaseWin.SetColors(0, 0xE0D0D0);

	winhead = new swcDisplay(&swc_BaseWin, 500, 500, mainwid - 1000, 4000,
			0xFFFFFF, 0xFF0000, SWC_BORDER_OUT_LG);
	winhead->PutText(0, 800, mainwid, 0, 0xFFFFFF, TXT_CENTER|FONT_HEADING|
			2400, "XOS Boot/Install Disk Creator");

	winmain = new swcDisplay(&swc_BaseWin, 500, 5000, mainwid - 1000,
			20500, 0xFFFFFF, 0xF0F0F0, SWC_BORDER_OUT_LG);
    winmain->GetClientRect(&apprect);

	winfoot = new swcDisplay(&swc_BaseWin, 500, 26000, mainwid - 1000,
			baserect.bottom - 26500, 0xFFFFFF, 0xF0F0F0, SWC_BORDER_OUT_LG);
    winfoot->GetClientRect(&footrect);

	winprog = new swcDisplay(winfoot, 1000, 4500,
			footrect.right - 2000, 1200, 0xFFFFFF, 0xC0C0C0, SWC_BORDER_IN_LG);
	GetClientRect(winprog->hwin, &progrect);
	winprog->PaintFunc = progdraw;
	curadone = 0;

	winmain->PutText(1000, 800, mainwid - 2000, 0xFFFFFF, 0x000000,
			TXT_LEFT|FONT_TEXT|1400, maintext);

	winmain->PutText(1000, 5500, mainwid - 2000, 0xFFFFFF, 0x000000,
			TXT_LEFT|FONT_TEXT|1400, "Select Target Disk");
	winmain->PutText(15000, 5500, mainwid - 2000, 0xFFFFFF, 0x000000,
			TXT_LEFT|FONT_TEXT|1400, "Select Input File");

	rspfile = new swcResponse(winmain, 15000, 7200,
			apprect.right - 15000 - 3000, 2200, SWC_SCROLL_HORIZ_A, "",
			FONT_TEXT|1400, 0x000000, 0xD0D0D0);
	btn = new swcButton(winmain, apprect.right - 3000, 7300, 2000, 1800,
			"...", FONT_TEXT|2300, 0xFFFFFF, 0x0000FF, true,
			::havebrowsebutton);

	btnstart = new swcButton(winmain, 20000, apprect.bottom - 9000, 6000, 1600,
			"Start", FONT_TEXT|1400, 0xFFFFFF, 0x0000FF, true,
			::havestartbutton);

	btn = new swcButton(winmain, 31000, apprect.bottom - 9000, 6000, 1600,
			"Exit", FONT_TEXT|1400, 0xFFFFFF, 0x0000FF, true,
			::haveexitbutton);

	winmain->PutText(15000, apprect.bottom - 5500, 200000, 0xFFFFFF, 0x0000FF,
			TXT_LEFT|FONT_TEXT|1400, warntext);

	stsnum = swcRespRegFunc(status, NULL);
	targetnum = swcRespRegFunc(target, NULL);

    if ((waithndl = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
    {
        seterrmsg("Error creating event");
        return;
    }
	if ((scandisksthd = (HANDLE)_beginthreadex(NULL, 0, scandisksthread, 0, 0,
			&scandisksid)) == NULL)
	{
		setmsg("? Thread create failed");
		return;
	}
	swc_DeviceNotify = devnotify;
    memset(&NotificationFilter, 0, sizeof(NotificationFilter));
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = interfaceguid;
    if (RegisterDeviceNotification(swc_BaseWin.hwin, &NotificationFilter,
			DEVICE_NOTIFY_WINDOW_HANDLE) == 0)
		setmsg("Could not register disk change notification\nDisk changes"
				"will not be shown");
}


//******************************************************************
// Function: baseclose - Called when the base window is being closed
// Returned: Always return true to continue closing
//******************************************************************

static int baseclose()
{
	terminate = true;
	if (gofnhndl != 0)
		EndDialog(gofnhndl, 0);
	return (true);
}


static void haveexitbutton(
	swcBase *win,
	long     arg1,
	long     arg2,
	void    *arg3)
{
	terminate = true;
	swcTerminate();
}


static void havestartbutton(
	swcBase *win,
	long     arg1,
	long     arg2,
	void    *arg3)
{
	uchar   *ipnt;
	DEVITEM *dpnt;
	int      item;

	if (xidspec[0] == 0)
		setmsg("? No source file specified");
	else
	{
		if ((item = lstdev->GetSelection()) < 0)
		{
			setmsg("? No target disk is selected");
			return;
		}
		if ((ipnt = lstdev->GetItem(item)) != NULL || ipnt[1] != '(')
		{
			devnum = atol((char *)ipnt + 1);
			if (devnum != 0)				// Safety check to make sure we
			{								//   don't zap the system disk!
				dpnt = devhead;
				while (dpnt != 0)
				{
					if (dpnt->dnum == devnum)
					{
						devbits = dpnt->mask;
						devsize = dpnt->length >> 9;
						install();
						return;
					}
					dpnt = dpnt->next;
				}
			}
		}
	}
}


static void havebrowsebutton(
	swcBase *win,
	long     arg1,
	long     arg2,
	void *arg3)
{
	*direndpnt = 0;
	firsthook = true;
	if (GetOpenFileName(&opnfile))
		rspfile->SetText(xidspec);
	gofnhndl = 0;
}

int eeee;

static void devnotify(
	UINT message,
	UINT wparm,
	LONG lparm)
{
	if (wparm == DBT_DEVICEARRIVAL || wparm == DBT_DEVICEREMOVECOMPLETE)
		SetEvent(waithndl);
}


#if 0
    switch (wparm)
    {
     case DBT_DEVICEARRIVAL:
		eeee = 1;
        break;

     case DBT_DEVICEREMOVECOMPLETE:
		eeee = 2;
        break;

     case DBT_DEVNODES_CHANGED:
		eeee = 3;
        break;

     default:
		eeee = 4;
        break;
    }
#endif


//*******************************************************************
// Function  gfnhook - Hook function for the GetOpenFileName function
// Returned: false
//*******************************************************************

UINT APIENTRY gfnhook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	HWND phndl;
	RECT crect;
	RECT prect;
	int  width;
	int  height;

	if (gofnhndl == 0)
		gofnhndl = GetParent(hwnd);
	if (firsthook && msg == WM_NOTIFY)
	{
		firsthook = false;
		phndl = GetParent(hwnd);
        GetWindowRect(phndl, &crect);
        width = crect.right - crect.left;
        height = crect.bottom - crect.top;
        GetWindowRect(swc_BaseWin.hwin, &prect);
		MoveWindow(phndl, prect.left + (prect.right - prect.left - width)/2,
				prect.top + (prect.bottom - prect.top - height)/2, width,
				height, true);
	}
    return (false);
}


#define MSG1X 1000
#define MSG1Y 600
#define MSG2X 1000
#define MSG2Y 2300

static void status(
	MSG  *msg,
	void *pntr)
{
	char *pnt1;
	char *pnt2;

	if (terminate)
		return;
	if (curadone != adone)
	{
		curadone = adone;
///		if (device == 0)
///			sprintf(text, "%d/%d", adone, amount);
///		else
///			sprintf(text, "%d/%d", adone/2, amount/2);
///		winprog->PutText(0, 400, mainwid - XPOS - 3000, 0, 0x000000,
///				TXT_CENTER|FONT_TEXT|1300, text);
		winprog->ForceRedraw();
	}
	pnt1 = (char *)msg->lParam;
	if ((pnt2 = strchr(pnt1, '\n')) != NULL)
	{
		*pnt2++ = 0;
		winfoot->PutText(MSG2X, MSG2Y, 200000, 10000, 0x000000, TXT_LEFT|
				FONT_TEXT|1400, pnt2);
	}
	winfoot->PutText(MSG1X, MSG1Y, 200000, 10000, (pnt1[0] == '?') ?
			0x0000FF : 0x000000, TXT_LEFT|FONT_TEXT|1400, pnt1);
	free(pnt1);
}




static void target(
	MSG  *msg,
	void *pntr)
{
	char    *tpnt;
	DEVITEM *dpnt;
	double   length;
	long     mask;
	long     bits;
	int      dltr;
	int      inx;
	char     buffer[256];

	if (terminate)
		return;
	if (lstdev != NULL)
	{
		delete lstdev;
		lstdev = NULL;
	}
	while (devhead != NULL)
	{
		dpnt = devhead->next;
		free(devhead);
		devhead = dpnt;
	}

	// Construct the strings for the disk select list box - 

	dpnt = devhead = (DEVITEM *)msg->lParam;
	inx = 0;
	while (dpnt != NULL)
	{
		if (dpnt->mask != 0)
		{
			tpnt = buffer + sprintf(buffer, "(%d)", dpnt->dnum);
			dltr = 'A';
			bits = dpnt->mask;
			mask = 0x01;
			do
			{
				if (bits & mask)
				{
					tpnt += sprintf(tpnt, " %c:", dltr);
					bits &= ~mask;
				}
				dltr++;
				mask += mask;
			} while (bits != 0);

			// Fixed by Seppo Pakonen 1.4.2024
			// This made the original xosmkinst to fail with over 9GB usb drives...
			// ...absolutely stupid and useless piece of code:
			//    if ((length = (double)(dpnt->length / 1000)) < 9000000)
			length = (double)(dpnt->length / 1000);
			{
				if (length > 1000000.0)
				{
					length /= 1000000.0;
					tpnt += sprintf(tpnt, " %.*fGB", digits(length), length);
				}
				else if (length > 1000.0)
				{
					length /= 1000.0;
					tpnt += sprintf(tpnt, " %.*fMB", digits(length), length);
				}
				else
					tpnt += sprintf(tpnt, " %.3fKB", digits(length), length);
				if (lstdev == NULL)
				{
					lstdev = new swcList(winmain, 1000, 7200, 12000,
							apprect.bottom - 8200, 1300, FONT_TEXT|1200, 0x000000,
							0xFFFFFF, NULL, 0, SWC_BORDER_IN_LG|SWC_SCROLL_VERT_C|
							SWC_STATE_ENABLE, NULL);
					btnstart->SetEnabled(true);
				}
				lstdev->PutItem(inx++, 0, buffer, tpnt - buffer, 0, 0);
			}
		}
		dpnt = dpnt->next;		
	}
	if (lstdev == NULL)
	{
		winmain->PutText(1000, 7200, 12000, 0xFFFFFF, 0x0000FF,
				TXT_LEFT|FONT_TEXT|1400, "There are no removable\nUSB disks "
				"available.");
		btnstart->SetEnabled(false);
	}

}


static int digits(
	double value)
{
	return ((value < 10) ? 3 : (value < 100) ? 2 : (value < 1000) ? 1 : 0);
}


//****************************************************************
// Function: progdraw - Paint function for the progress bar window
// Returned: Nothing
//****************************************************************

static void progdraw(
	swcDisplayBase *win,
	HDC             hdc)
{
	int len;

	DeleteObject(SelectObject(hdc, CreateSolidBrush(RGB(0, 255, 0))));
	DeleteObject(SelectObject(hdc, CreatePen(PS_NULL, 0, RGB(0, 255, 0))));
	len = (long)((amount > 0) ? ((progrect.right * adone) / amount) : 0);
	Rectangle(hdc, 0, 0, len, progrect.bottom);
	DeleteObject(SelectObject(hdc, CreateSolidBrush(RGB(0xC0, 0xc0, 0xC0))));
	DeleteObject(SelectObject(hdc, CreatePen(PS_NULL, 0,
			RGB(0xC0, 0xc0, 0xC0))));
	Rectangle(hdc, len, 0, progrect.right, progrect.bottom);
	DeleteObject(SelectObject(hdc, GetStockObject(WHITE_BRUSH))); // Clean up
	DeleteObject(SelectObject(hdc, GetStockObject(WHITE_PEN)));   //   the GUI
}
