#ifndef _SWCWINDOWS_H
#define _SWCWINDOWS_H

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef unsigned char    uchar;
typedef unsigned short   ushort;
typedef unsigned int     uint;
typedef unsigned long    ulong;
typedef unsigned __int64 ullong;
typedef          __int64 llong;

#if defined _MSC_VER
  #define NOTUSED(x)
  #pragma warning(disable : 4100)
  #pragma warning(disable : 4127)
  #pragma warning(disable : 4310)
  #pragma warning(disable : 4355)
#elif defined __WATCOMC__ 
  #define NOTUSED(x) x=x
#endif

/// long FAR PASCAL GdiSeeGdiDo(ushort, ushort, ushort, ushort);
/// long gdisel;

/// int dumpgdiobj(HGDIOBJ hndl, int show);

#define DATETIME_MAX  0x2BCA2875F4373FFF
#define TICKPERDAY    0xC92A69C000
#define TICKPERDAY_LO 0x2A69C000
#define TICKPERDAY_HI 0xC9
#define TICKPERSEC    10000000

#define SWC_OSTYPE_WIN98 1
#define SWC_OSTYPE_WINXP 2
#define SWC_OSTYPE_XOS   3

#define SWC_WT_DISPLAY   1
#define SWC_WT_BUTTON    2
#define SWC_WT_RESPONSE  3
#define SWC_WT_EDIT      4
#define SWC_WT_DROPDOWN  5
#define SWC_WT_LIST      6
#define SWC_WT_CHECKBOX  7
#define SWC_WT_DROPDOWN2 8
#define SWC_WT_TABDISP   9

#define SWC_STATE_ENABLE  0x01
#define SWC_STATE_DISABLE 0x00
#define SWC_STATE_SET     0x02
#define SWC_STATE_CLEAR   0x00

#define SWC_TEXT_LEFT     0x40

#define SWC_BORDER_NONE    0x00000000	// No border
#define SWC_BORDER_LINE_SM 0x00000110	// Small line border
#define SWC_BORDER_LINE_MD 0x00000120	// Medium line border
#define SWC_BORDER_LINE_LG 0x00000130	// Large line border
#define SWC_BORDER_IN_SM   0x00000210	// Small inset border
#define SWC_BORDER_IN_MD   0x00000220	// Medium inset border
#define SWC_BORDER_IN_LG   0x00000230	// Large inset border
#define SWC_BORDER_OUT_SM  0x00000310	// Small outset border
#define SWC_BORDER_OUT_MD  0x00000320	// Medium outset border
#define SWC_BORDER_OUT_LG  0x00000330	// Large outset border
#define SWC_BORDER_FRM_SM  0x00000410	// Small frame border
#define SWC_BORDER_FRM_MD  0x00000420	// Medium frame border
#define SWC_BORDER_FRM_LG  0x00000430	// Large frame border

#define SWC_TTLBAR_NONE    0x00000000	// No title bar
#define SWC_TTLBAR_CLS     0x00001000	// Title bar with close button only
#define SWC_TTLBAR_FULL    0x00002000	// Title bar with all buttons

#define SWC_SCROLL_NONE    0x00000000	// No scroll bars
#define SWC_SCROLL_VERT_A  0x00010000	// Vertical scroll bar (always
										//   displayed)
#define SWC_SCROLL_VERT_C  0x00020000	// Vertical scroll bar (conditionally
										//   displayed)
#define SWC_SCROLL_HORIZ_A 0x00030000	// Horizontal scroll bar (always
										//   displayed)
#define SWC_SCROLL_HORIZ_C 0x00040000	// Horizontal scroll bar (conditionally
										//   displayed)

#define SWC_ANCHOR_LEFT    0x00100000	// Window is anchored at left edge
#define SWC_ANCHOR_RIGHT   0x00200000	// Window is anchored at right edge
#define SWC_ANCHOR_TOP     0x00400000	// Window is anchored at top edge
#define SWC_ANCHOR_BOTTOM  0x00800000	// Window is anchored at bottom edge

#define SWC_MMOVE_ACTION   0x01000000	// Call action function on mouse
										//   movement
#define SWC_WIN_USEDBLCLK  0x02000000	// Detect double click in window

#define SWC_IMAGE_NONE 0
#define SWC_IMAGE_BMP  1
#define SWC_IMAGE_GIF  2
#define SWC_IMAGE_JPEG 3
#define SWC_IMAGE_PNG  4


struct swcMouseMove
{	long xpos;
	long ypos;
	long zdelta;
	long time;
	long arg;
};


struct txtblk
{	txtblk *next;
	long    left;
	long    right;
	long    top;
	long    bottom;
	long    txtcolor;
	int     length;
	int     mode;
	int     fontnum;
	char    text[1];
};

class swcImage
{public:
	long    type;
	BITMAPINFOHEADER
	        bmih;
	uchar  *data;
	int  usecnt;

	swcImage();
	swcImage(long width, long height, int allocdata = true);
	swcImage(char *fspec);
	swcImage(uchar *data, int size);
	~swcImage();
	void Use();
	void Release();
	long GetRawHeight() {return (bmih.biHeight);};
	long GetRawWidth() {return (bmih.biWidth);};
	swcImage *Copy();
	swcImage *CopyScaled(long width, long height);
	swcImage *scalecolorli(long width, long height);
	swcImage *scalesmooth(long width, long height);
///	swcImage *scalecolor2xli(tr);
	int       PutJPEG(char *filespec, int quality);
	uchar    *PutJPEG(int quality, long *sizepntr);
	long      PutJPEG(int quality, uchar *bufr);
	void      dojpeg(struct jpeg_compress_struct *cinfo, int quality);
	static void scalecolorlilow(ulong *datad, long wd, long hd,
		ulong *datas, long ws, long hs);
///	static void scalecolor2xlilow (ulong *datad, long wd,
///		ulong *datas, long ws, long hs);

 private:
	void getbmpimage(uchar *bufr, int size, FILE *file);
	void getjpegimage(uchar *bufr, int size, FILE *file);
	void getpngimage(uchar *bufr, int size, FILE  *file);
};


class imgblk
{public:
	imgblk   *next;
	long      xpos;
	long      ypos;
	long      width;
	long      height;
	swcImage *img;

///	imgblk();
///	~imgblk();
};


typedef struct HRGN__ swcRgn;

extern long amountalloc;
extern long numberalloc;
void *operator new(size_t size);
void  operator delete(void *p);

class swcBase;
class swcDisplay;
class swcTopLevel;
class swcDisplayBase;

typedef void SWCACTION(swcBase *win, long arg);

extern HINSTANCE hinst;

// Define the structure which contains value to specify a base window

extern swcBase *travlist[5];
extern long sfd;
extern long sfr;

// Items which must be defined in the client program

extern long   swc_XSize;
extern long   swc_YSize;
extern long   swc_XPos;
extern long   swc_YPos;
extern uchar  swc_WinType;
extern uchar  swc_Ending;
extern char   swc_AppMainBanner[];
extern char  *swc_IniFileSpec;
extern char   swc_ProgDir[];
extern char  *swc_ProgName;
extern void (*swc_WindowGone)(swcBase *win);
extern void (*swc_ShiftStateHook)(int down);
extern void (*swc_ControlStateHook)(int down);
extern void (*swc_AllDone)();
extern int  (*swc_BaseClose)();
extern void (*swc_DeviceNotify)(UINT msg, UINT wparm, LONG lparm);

extern uint   swc_basetid;

extern swcDisplayBase *swcwheelwin;


// Values for wcs_WinType

#define SWC_WINTYPE_NORMAL     1
#define SWC_WINTYPE_FULLSCREEN 2


class swcBase
{public:
    HWND        hwin;				// Window handle
    uchar       type;
	long        xpos;
	long        ypos;
	long        height;
	long        width;
	long        xanchor;
	long        yanchor;
    long        txtcolor;
    long        bgcolor;
	long        userdata;
	void       *userpntr;

	swcDisplayBase *parent;
	swcTopLevel    *toplvl;
	swcBase        *sibling;
	int             arg2data;
	void           *arg3data;
    void          (*action)(swcBase *win, long arg1, long arg2, void *arg3);
	WNDPROC         orgproc;
	HWND            wchild;
	uchar           anchor;

	void         ForceRedraw();
	int          GetClientRect(RECT *rect);
	int          GetWindowRect(RECT *rect);
	int          GetUnscaledClientRect(RECT *rect);
	int          SetEnabled(int enable);
	int          Visible(int visible);
	void         SetFocus();
	void         ForceFocus();
	void         SetFont(int index);
	int			 GetText(char *bufr, int size);
	int			 GetTextLength();
	int          SetText(char *bufr);
	void         SetMouseFocus(int value);
	int          MoveResize(long xpos, long ypos, long width, long height,
			int  repaint);
	int          Move(long xpos, long ypos, int repaint);
	void         SetColors(long txtcolor, long bkgcolor);
	void         BringToTop();
	int          SetTraverse(uint group, uint number, int travrtn = FALSE);
	int          SetTraverse(swcDisplayBase *win, uint group, uint number,
			int  travrtn = FALSE);
	virtual void drawitem(DRAWITEMSTRUCT *dis) {dis = dis;};
	virtual void measureitem(MEASUREITEMSTRUCT *mis) {mis = mis;};
	virtual HBRUSH ctlbkgcolor(HDC hdc) {hdc = hdc; return (NULL);};
	virtual void xxxdeleteitem(char *data) {data = data;};
	virtual long ctlcolor(HDC hdc) {hdc = hdc; return (0);};
	virtual void createwindow(char *winclass, char *wintext, long ctlid,
			long bits, long exbits);
	virtual void sizefixup() {};
	bool         checkfocus(HWND fwin);
	virtual     ~swcBase();
	void         SetGblKeyHook(int code, int (*func)(swcBase *win, ulong data,
			long argi, void *argp), long argi, void *argp = NULL);
	int          RemoveGblKeyHook(int code);
	void         SetGblKeyFunc(int (*func)(ulong data, void *argp),
			void *argp = NULL)
			{gblfunc = func; gblargp = argp;};
	int          GetTextHeight(int fontnum, char *text, int len, int width);
	int          checkforkey(long wval);

 protected:
	struct keyhook
	{	keyhook *next;
		long     code;
		int    (*func)(swcBase *win, ulong data, long argi, void *argp);
		long     argi;
		void    *argp;
	};

	swcBase *travlist[5];		// Head pointers for this window's lists
	swcBase *travwin;			// Window whose list we are in
	swcBase *travnext;			// Next window in list
	swcBase *travprev;			// Previous window in list
	keyhook *gkhook;
	int    (*gblfunc)(ulong code, void *argp);
	void    *gblargp;
	int      prefixindex;
	uchar    travgroup;
	uchar    travnumber;
	uchar    travrtn;			// Use CR as traverse character if TRUE;
	uchar    disabled;			// Window is disabled
	uchar    dieing;			// true if object is being terminated
	uchar    mousefocus;

	void     travremove();
	int      travcheck(int chr);
	         swcBase();
	         swcBase(long xpos, long ypos, long width, long height,
			swcDisplayBase *parent);
	void     setfocustoprev(void);
	void     setfocustonext(void);
};

class swcDisplayBase : public swcBase
{public:
	HENHMETAFILE hmf;
	long     cwidth;
	long     cheight;
	swcBase *fchild;
    txtblk  *txthead;
    txtblk  *txttail;
    imgblk  *imghead;
    imgblk  *imgtail;
	long     xoffset;
	long     xrange;
	long     xincrement;
	long     yoffset;
	long     yrange;
	long     yincrement;
	char     mmoveaction;
	     swcDisplayBase();
	     swcDisplayBase(char *name, char *label, swcDisplayBase *parent,
				long xpos, long ypos, long width, long height, long txtcolor,
				long bgcolor, long bits, long wbits,
				void (*action)(swcBase *win, long arg1, long arg2,
				void *arg3) = NULL, void *arg3 = NULL, int arg2 = 0);
	     ~swcDisplayBase();
	void Clear();
	void BringToTop();
	void PutImage(long xpos, long ypos, swcImage *img);
	void PutImage(long xpos, long ypos, long width, long height, swcImage *img);
///	imgblk *putbmpimage(long size, uchar *data);
///	imgblk *putjpegimage(long size, uchar *data);

	void PutText(long xpos, long ypos, long width = 0, long height = 0,
		    long txtcolor = 0, int font = 0, char *text = NULL);
	void PutText(long xpos, long ypos, long width, long height,
		    long txtcolor, int font, int horiz, char *text = NULL);
	int  GetTextExtent(int fontnum, char *text, int len, long *size);
	void HorizScrollRange(long range, long increment);
	void HorizScrollPosition(long pos);
	void VertScrollRange(long range, long increment);
	void VertScrollPosition(long pos);
	void (*PaintFunc)(swcDisplayBase *win, HDC hdc);
	void DrawLine(long type, long xpos, long ypos, long length, long width);

	virtual int  haveclose();
	virtual int  havefocus(swcBase *win);
	virtual void sizefixup();
	virtual void msg111(MSG *msg);
	virtual void msg112(MSG *msg);

	static long APIENTRY displayproc(HWND hwnd, UINT msg, UINT wparam,
			LONG lparam);
	void adjustchilds();
};

// Define the swcDisplay class - This is the general container-type class
//   which can contain text, images, and child windows.

class swcDisplay : public swcDisplayBase
{public:
	swcDisplay();
	swcDisplay(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, long txtcolor, long bgcolor, long bits,
			void (*action)(swcBase *win, long arg1, long arg2,
			void *arg3) = NULL, void *arg3 = NULL, int arg2 = 0);
};


class swcTabDisp : public swcDisplayBase
{public:
	swcTabDisp();
	swcTabDisp(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, long xpad, long ypad, long   fontnum, long txtcolor,
			long bgcolor, long bits, void (*action)(swcBase *win, long arg1,
			long arg2, void *arg3) = NULL, void *arg3 = NULL, int arg2 = 0);
	int  AddTab(int index, char *text, int image);
	int  GetClientRect(RECT *rect);
	int  SetSelection(int sel);
	void selchanged(int index);

 protected:
	static long APIENTRY tabproc(HWND hwnd, UINT msg, UINT wparam,
		    LONG lparam);

	uchar havetab;
};



// Define the swcDisplayLinked class - This is a decendent of the swcDisplay
//   class which is linked to an swcList object for horizontal scrolling.

class swcDisplayLinked : public swcDisplay
{public:
	swcDisplayLinked(swcDisplayBase *parent, class swcList *link, long xpos,
			long ypos, long width, long height, long txtcolor, long bgcolor,
			long bits);
	~swcDisplayLinked();
	void unlink(swcList *listarg);

protected:
	swcList *listwin;
};

// Define the swcTopLevel class - This is a general container-type class
//   which creates a top-level independent window.

class swcTopLevel : public swcDisplayBase
{public:
	swcTopLevel();
    swcTopLevel(swcDisplayBase *parent, char *caption, long xpos, long ypos,
			long width, long height, long txtcolor, long bgcolor,
			long bits);
    swcTopLevel(swcDisplayBase *parent, char *caption, char *classname,
			long xpos, long ypos, long width, long height, long txtcolor,
			long bgcolor, long bits);
	void SetTitle(char *text);

	swcBase *ffwindow;
};

class swcButton : public swcBase
{public:
///	int xxxx;
	swcButton(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, char *text, long font, long txtcolor, long bgcolor,
			int state, void (*action)(swcBase *win, long arg1, long arg2,
			void *arg3) = NULL, void *arg3 = NULL, long arg2 = 0,
			void (*scfunc)(int func, long arg2, void *obj) = NULL,
			void *scobj = NULL);
	~swcButton();
	void SetText(char *text);
	int  GetState() {return (pressed);};
	void SetState(char *text, int state);
	void SetShortCut(char *text);
	void gettext(char *bufr, char *text);
	void havekey2();
	void setshortcut(long arg2);
	void clearshortcut();

	static int havekey(swcBase *win, ulong code, long argi, void *argp);

	long   letter;

 protected:
	static long APIENTRY buttonproc(HWND hwnd, UINT msg, UINT wparam,
		    LONG lparam);

	void (*scfunc)(int func, long arg2, void *obj);
	void  *scobj;
	uchar  scactive;
 public:
	uchar  ignore;
	uchar  pressed;
};

class swcCheckBox : public swcBase
{public:
	int xxxx;
	swcCheckBox(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, char *text, long font, long txtcolor, long bgcolor,
			int state, void (*action)(swcBase *win, long arg1, long arg2,
			void *arg3) = NULL, void *arg3 = NULL, long arg2 = 0);
	int  GetState();
	void SetState(int state);
	HBRUSH ctlbkgcolor(HDC hdc);
 protected:
	static long APIENTRY checkboxproc(HWND hwnd, UINT msg, UINT wparam,
		    LONG lparam);
};

class swcResponse : public swcBase
{public:
	int xxxx;
	swcResponse(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, int options, char *text, long font, long txtcolor,
			long bgcolor, void (*action)(swcBase *win, long arg1, long arg2,
			void *arg3) = NULL, void *arg3 = NULL, long arg2 = 0);
	~swcResponse();
	void SetText(char *text);
	void SelectAll();

	int (*charcheck)(swcResponse *win, int chr);
	int   options;

	static long APIENTRY responseproc(HWND hwnd, UINT msg, UINT wparam,
			LONG lparam);
	HWND    frame;
};

// Define bits used in the options argument above

#define SWC_RESPONSE_HIDETEXT 0x0001	// Hide text
#define SWC_RESPONSE_SELALL   0x0002	// Select all text when getting focus
#define SWC_RESPONSE_READONLY 0x0004	// Read only


class swcResponseNum : public swcResponse
{public:
	swcResponseNum(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, int hidetext, char *text, long font, long txtcolor,
			long bgcolor, void (*action)(swcBase *win, long arg1, long arg2,
			void *arg3) = NULL, void *arg3 = NULL, long arg2 = 0);
	int GetValue(long *value);
 private:
	static int numcheck(swcResponse *win, int chr);
};

class swcResponseChar : public swcResponse
{public:
	swcResponseChar(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, char *text, long font, long txtcolor, long bgcolor,
			int (*action)(swcResponseChar *win, long arg1, void *arg2),
			void *arg2);
 private:
	static int charcheckfunc(swcResponseChar *win, int chr);
	int  (*charaction)(swcResponseChar *win, long arg1, void *arg2);
	void  *charactarg;
};


class swcEdit : public swcBase
{public:
	int xxxx;
	swcEdit(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, int options, long fontnum, long txtcolor,
			long bgcolor, char *text, long bits,
			void (*action)(swcBase *win, long arg1, long arg2,
			void *arg3) = NULL, void *arg3 = NULL, long arg2 = 0);
	void SetText(char *text);

 protected:
	static long APIENTRY editproc(HWND hwnd, UINT msg, UINT wparam,
			LONG lparam);
};


#define SWC_EDIT_HIDETEXT 0x0001		// Hide text
#define SWC_EDIT_SELALL   0x0002		// Select all text when getting focus
#define SWC_EDIT_READONLY 0x0004		// Read only

class swcDropDown : public swcBase
{public:
	swcDropDown(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, long ddheight, long font, long txtcolor, long bgcolor,
			char **contents, int select, long bits, void (*action)(swcBase *win,
			long arg1, long arg2, void *arg3) = NULL, void *arg3 = NULL,
			long arg2 = 0);
	~swcDropDown();
	int     GetSelection();
	int     SetSelection(int sel);
	int     GetItem(int inx, char *str, int len);
	int     PutItem(char *str);
	int     InsertItem(int inx, char *str);
	int     RemoveItem(int index);
	int     FindString(int start, char *str);
	int     FindStringExact(int start, char *str);
	int     ShowDropDown(int state);
	int     ddcheckforkey(int code);
	int     addtolist();
	int     processcmdmsg(UINT wparam);
	WNDPROC editproc;

	swcDropDown *ddnext;
	long         bits;
	int          cursel;
	int          numitems;
	uchar        changed;
	uchar        removed;

	static swcDropDown *ddfirst;

 protected:
	static long APIENTRY dropdownproc(HWND hwnd, UINT msg, UINT wparam,
		    LONG lparam);
	static long APIENTRY ddeditproc(HWND hwnd, UINT msg, UINT wparam,
		    LONG lparam);
};

#define SWC_DROPDOWN_EDIT  0x0001		// Items can be edited
#define SWC_DROPDOWN_AUTO  0x0002		// Show list when get focus
#define SWC_DROPDOWN_FRCUC 0x0004

class swcDropDown2 : public swcBase
{public:
	int xxxx;
	swcDropDown2(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, long font, long txtcolor, long bgcolor,
			char **contents, int select, void (*action)(swcBase *win,
			long arg1, long arg2, void *arg3) = NULL, void *arg3 = NULL,
			long arg2 = 0);
	int  GetSelection();
	int  SetSelection(int sel);
	int  PutItem(char *str);
 protected:
	static long APIENTRY dropdown2proc(HWND hwnd, UINT msg, UINT wparam,
		    LONG lparam);
	HWND    frame;
	HWND    list;
};

class swcList : public swcBase
{public:
	struct listitem
	{	ushort  extra;
		ushort  height;
		long    bgcolor;
		txtblk *txthead;
		txtblk *txttail;
		ushort  topmargin;
		uchar   data[1];
	};

	swcList(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, long font, long txtcolor, long bgcolor,
			char **contents, int select, long bits, void (*action)(swcBase *win,
			long arg1, long arg2, void *arg3) = NULL, void *arg3 = NULL,
			long arg2 = 0);
	swcList(swcDisplayBase *parent, long xpos, long ypos, long width,
			long height, long iheight, long font, long txtcolor, long bgcolor,
			char **contents, int select, long bits, void (*action)(swcBase *win,
			long arg1, long arg2, void *arg3) = NULL, void *arg3 = NULL,
			long arg2 = 0);
	~swcList();
	void listconstruct(long iheight, long font, long txtcolor, long bgcolor,
			char **contents, int select, long bits, void (*action)(swcBase *win,
			long arg1, long arg2, void *arg3) = NULL, void *arg3 = NULL,
			long arg2 = 0);
	uchar *GetItem(int index);
	int    GetSelection();
	uchar *PutItem(int index, int extra, char *text, int len = 0,
			int height = 7, int topmargin = 0);
	uchar *PutItem(int index, int extra, long color, char *text, int len = 0,
			int height = 7, int topmargin = 0);
	uchar *UpdateItem(int index, int extra, char *text, int len = 0,
			int height = 7, int topmargin = 0);
	uchar *UpdateItem(int index, int extra, long color, char *text, int len = 0,
			int height = 7, int topmargin = 0);
	int    RemoveItem(int index);
	void   PutText(int index, long xp, long yp, long width = 0, long height = 0,
			long txtcolor = 0, int font = 0, int mode = 0, char *text= NULL);
	void   SetSelection(int sel);
	int    SetTopItem(int sel);
	void   SetSelBgColor(long color) {bgcolorsel1 = color;bgcolorsel2 = color;};
	void   SetSelBgColor(long color1, long color2) {bgcolorsel1 =
			color1;bgcolorsel2 = color2;};
	int    ClearContent();
	void   HorizScrollRange(int range);
	void   setxlink(void (*func)(void *arg, int xpos), swcDisplayLinked *arg);
	void   clrxlink(swcDisplayLinked *arg);
	static long APIENTRY listproc(HWND hwnd, UINT msg, UINT wparam,
		    LONG lparam);
	void   yyydeleteitem(listitem *data);

 private:
	virtual void   measureitem(MEASUREITEMSTRUCT *mis);
	virtual void   drawitem(DRAWITEMSTRUCT *dis);
	int    xpos;
	long   iheight;
	long   ihbits;
	long   bgcolorsel1;
	long   bgcolorsel2;
	int    fontnum;
	void (*xlinkfunc)(void *arg, int xpos);
	void  *xlinkarg;
};

// Define bits used in the iheight argument above

#define SWC_LIST_VARIABLE   0x40000000
#define SWC_LIST_LINE       0x20000000
#define SWC_LIST_SHOWSELECT 0x10000000

/*
class swcNotify : public swcTopLevel
{public:
	swcNotify(char *label, int flag, char *text, int width, long color,
			int autormv = TRUE, int (*donefunc)(int code, void *arg) = NULL,
			void *arg = NULL, char *but1text = "Continue",
			char *but2text = NULL, char *but3text = NULL);
	~swcNotify();
	void ReplaceText(char *text);

	static void Remove();

 protected:
///	static void haveclose(swcTopLevel *win, void *arg);
	int    getheight(int flag, char *text, int width);
	static void havebutton(swcBase *win, long arg1, long arg2, void *arg3);
	int    autormv;
	int  (*donefunc)(int code, void *arg);
	void  *arg;
};
*/


class swcDebug : public swcTopLevel
{public:
	swcDebug();
	~swcDebug();
	static void Text(char *msg, ...);
};


class swcPrint
{public:
	enum  nfunc {begin = 1, status, finish, error};

	swcPrint(swcDisplayBase *owner);
	~swcPrint();
	int     BeginPrint(int landscape, char *name, int xres, int yres,
			void (*notify)(nfunc func, char *text, void *arg), void *arg);
	void    PutText(long xpos, long ypos, long width, long height,
			long txtcolor, ulong font, int mode, char *text);
	void    PutImage(long xpos, long ypos, long width, long height,
			swcImage *img);
	void    DrawRect(long xpos, long ypos, long width, long height, long color);
	void    DrawRect(long xpos, long ypos, long width, long height, long fcolor,
			long brdrwid, long bcolor);
	void    DrawLine(long xbgn, long ybgn, long xend, long yend, long width,
			long color);
	swcRgn *CreateRectRgn(long hpos, long vpos, long width,
			long height);
	swcRgn *GetClipRgn();
	int     SetClipRgn(swcRgn *rgn);
	int     BeginPage();
	int     EndPage();
	int     EndPrint();
	void    CancelPrint();
	HFONT   getprtfont(ulong fontnum, int weight, int angle, int italic);
	void    errornotify(char *text);

///	swcNotify *pnotify;
	swcRgn    *clprgn;
	ulong      fontnum;
	int        weight;
	int        xres;
	int        yres;
	int        pagenum;
///	uchar      abortflg;

protected:
	void  (*notify)(nfunc func, char *text, void *arg);
	void   *notarg;
	HDC     hdc;
	DOCINFO docinfo;
	swcDisplayBase
	       *owner;
	swcRgn *basergn;
	uchar   notifydone;
};

/*
class swcPrintNotify : public swcTopLevel
{public:
	swcPrintNotify(uchar *abort);
	~swcPrintNotify();
	void ShowPage(int page);

	uchar *abortpnt;
	uchar  abortflg;
};
*/

class swcRawPrint
{public:
	swcRawPrint(char *prtname, char *docname);
	~swcRawPrint();
	
	int PutData(char *data, int len);
	int BeginPage();
	int EndPage();
	int CancelPrint();

	HANDLE hp;
	uchar pageact;
	uchar docact;
};



struct swcseqlist
{	swcseqlist *next;
	ushort      prefix[5];
	ushort      suffix;
	long        argn;
};

class swcKBWedge
{public:
	swcKBWedge(swcBase *win, int maxchars, int prefixtime, int totaltime,
			int (*func)(swcBase *winarg, long argn, void *argp, char *bufr,
			int len), void *argp = NULL);
	~swcKBWedge();
	void  DefineSeq(ushort *prefix, int suffix, long argn);
	int   haveprefix(int code);
	int   haveprefix2(int code);
	int (*func)(swcBase *winarg, long argn, void *argp, char *bufr, int len);
	swcBase    *kbwwin;
	swcseqlist *seqlistpnt;
	swcseqlist *dataslp;


	void       *argp;
	char       *bufr;
	char       *bufrpnt;
	int         prefixindex;
	int         prefixtime;
	int         totaltime;
	int         maxchars;
};

// Values for horiz in TXTBLK

#define LIGHT  200
#define NORMAL 400
#define BOLD   700

struct scnfntdef
{//	HFONT  font;
    uchar  size4;
	uchar  size5;
	uchar  xxx;
    uchar  italic;
    ushort actual;
    ushort weight;
    char   name[32];
};

#define FNTMAX 32

// Define screen font names

#define FONT_SMTEXT    0
#define FONT_MSTEXT    1
#define FONT_MDTEXT    2
#define FONT_LMTEXT    3
#define FONT_LGTEXT    4
#define FONT_SMSANS    5
#define FONT_MSSANS    6
#define FONT_MDSANS    7
#define FONT_LMSANS    8
#define FONT_LGSANS    9
#define FONT_VLSANS    10
#define FONT_SMHEADING 11
#define FONT_MSHEADING 12
#define FONT_MDHEADING 13
#define FONT_LMHEADING 14
#define FONT_LGHEADING 15
#define FONT_VLHEADING 16
#define FONT_SMFIXED   17
#define FONT_MSFIXED   18
#define FONT_MDFIXED   19
#define FONT_LMFIXED   20
#define FONT_LGFIXED   21
#define FONT_SMSANSN   22
#define FONT_MSSANSN   23
#define FONT_MDSANSN   24
#define FONT_LMSANSN   25
#define FONT_LGSANSN   26
#define FONT_VLSANSN   27

// Define printer font names

#define FONT_TEXT     0x10000000
#define FONT_HEADING  0x20000000
#define FONT_FIXED    0x30000000
#define FONT_BARCODE  0x40000000
#define FONT_CONTROL  0x50000000
#define FONT_NARROW   0x60000000

#define TXT_NORMAL    0x00000000
#define TXT_THIN      0x01000000
#define TXT_XLIGHT    0x02000000
#define TXT_LIGHT     0x03000000
#define TXT_MEDIUM    0x04000000
#define TXT_DBOLD     0x05000000
#define TXT_BOLD      0x06000000
#define TXT_XBOLD     0x07000000
#define TXT_HEAVY     0x08000000

// Define text type values

#define TXT_LEFT      0x00010000
#define TXT_CENTER    0x00020000
#define TXT_RIGHT     0x00030000
#define TXT_DECIMAL   0x00040000

#define TXT_UNDERLINE 0x00400000
#define TXT_ITALIC    0x00800000

#define IMG_BMP 1

struct fontblk
{	fontblk *next;
	char     file[64];
};

extern swcTopLevel swc_BaseWin;
///extern swcDisplay swc_BaseWin;
extern scnfntdef  swcscnfnttbl[FNTMAX+1];
extern uchar      swcnotdone;
///extern swcNotify *swc_NotifyWin;

extern uchar     *swckeytbl[];
extern uchar      swcvirkey2xos[];
extern long       swcbasexoffset;
extern long       swcbaseyoffset;

extern uchar      swc_OSType;
extern uchar      swc_NumLockDisable;
extern uchar      swc_NumLockInserted;
extern uchar      swc_ForceFocusEnable;
extern uchar      swc_MouseFocus;


class swcThread
{public:
	swcThread(unsigned (__stdcall *start_address)(void *), void *args,
			long stksize);

	uint  thrdid;
	ulong thrdhndl;
};


typedef struct
{	long  date;
    ulong time;
} time_n;

typedef struct
{	long  high;
	long  low;
    short tzone;
    short dlst;
} time_nz;

class swcDateTime
{public:
	llong dt;
	short tzone;
	short dlst;

	swcDateTime(){};
	swcDateTime(llong dtval, int tzoneval, int dlstval)
	{	dt = dtval;
		tzone = (ushort)tzoneval;
		dlst = (ushort)dlstval;
	};
	void ConvToDos(struct tm *tm);
	void ConvTimeToDos(struct tm *tm);
	void ConvDateToDos(struct tm *tm);
	void ConvFromDos(struct tm *tm);
	void ConvTimeFromDos(struct tm *tm);
	void ConvDateFromDos(struct tm *tm);
	long ConvToDays();
	void ConvFromDays(long days);
	void TimeOnly(swcDateTime *time);
	void TimeOnly(){ dt %= TICKPERDAY;};
	void ConvToExternal(time_nz *nz);
	void ConvFromExternal(time_nz *nz);
	void GetLocal();
	void GetLocalTime();
	int  SetCurrent();
	void ConvFromOldFmt(long lowval, long highval, int tzoneval, int dlstval);
	void Set(llong dtval, int tzoneval, int dlstval)
	{	dt = dtval;
		tzone = (short)tzoneval;
		dlst = (short)dlstval;
	};

	int operator > (llong val)
	{	return (dt > val);
	};
	int operator >= (llong val)
	{	return (dt >= val);
	};
	int operator < (llong val)
	{	return (dt < val);
	};
	int operator <= (llong val)
	{	return (dt <= val);
	};
	int operator == (llong val)
	{	return (dt == val);
	};
	int operator != (llong val)
	{	return (dt != val);
	};
	swcDateTime operator += (const llong val)
	{	dt += val;
		return (*this);
	};
	swcDateTime operator += (const swcDateTime &op2)
	{	dt += op2.dt;
		return (*this);
	};

	int operator == (swcDateTime &val)
	{	return (dt == val.dt);
	};
	int operator != (swcDateTime &val)
	{	return (dt != val.dt);
	};
	int operator < (swcDateTime &val)
	{	return (dt < val.dt);
	};
	int operator <= (swcDateTime &val)
	{	return (dt <= val.dt);
	};
	int operator > (swcDateTime &val)
	{	return (dt > val.dt);
	};
	int operator >= (swcDateTime &val)
	{	return (dt >= val.dt);
	};

	swcDateTime operator + (const swcDateTime &op2)
	{	swcDateTime temp;
		temp.dt = dt + op2.dt;
		temp.tzone = tzone;
		temp.dlst = dlst;
		return (temp);
	};
	swcDateTime operator - (const swcDateTime &op2)
	{	swcDateTime temp;
		temp.dt = dt - op2.dt;
		temp.tzone = tzone;
		temp.dlst = dlst;
		return (temp);
	};


	static long  GetLocalDate();
	static llong Diff(swcDateTime *a, swcDateTime *b);
};

class swcDTSpan
{
	llong dt;

 public:
	void Diff(swcDateTime *a, swcDateTime *b);
	void FromDays(long days);
	long ToDays(int rounding);
};

void    *getmemory(int size);
void     givememory(void *mem);
int      swcAddTrueTypeFont(char *name, char *file);
long     swcAlarmSet(ulong time, int rpt, void (*func)(long tmr, long arg1,
		void *arg2), long arg1, void *arg2);
void     swcAlarmKill(long inx);
void     swcAppMain();
void     swcBeep(int freq, int length);
swcRgn  *swcCreateRectRgn(long hpos, long vpos, long width, long height);
///void     swcConvDateDos2Xos(time_s *date, struct tm *tm);
///void     swcConvDateXos2Dos(struct tm *tm, time_s *dt);
///long     swcConvDateXos2Days(time_s *dt);
///void     swcConvDateDays2Xos(time_s *dt, long days);
long     swcconvertbits(long bits, long *exbits);
///void     swcConvTimeDos2Xos(time_s *time, struct tm *tm);
///void     swcConvTimeXos2Dos(struct tm *tm, time_s *time);
void     swcDeleteRgn(swcRgn *rgn);
///void     swcGetCurrentDateTime(time_sz *dt);
///void     swcGetCurrentDate(time_s *dt);
///void     swcGetCurrentTime(time_s *dt);
swcBase *swcGetFocus();
long     swcgetfontact(int fontnum);
int      swcGetIniString(char *section, char *keyname, char *dfltstr,
		char *buffer, int length);
int      swcGetOpenFileSpec(char *title, char *inidir, char *filter,
		int findex, char *defext, long flags, char *buffer, int size,
		long *foffset);
void     swchandlemessage(MSG *msg);
HFONT    swcgetscnfont(int fontnum);
int      swcPutIniString(char *section, char *keyname, char *buffer);
///void     swcSetCurrentDateTime(time_sz *dt);
int      swcSetNumLock(int state);
void     swcTerminate();
///void     swcTimeOnly(time_s *time, time_s *dt);

int      swcRespRegFunc(void (*func)(MSG *msg, void *pntr), void *pntr);
int      swcRespSend(int msgnum, long value1, long value2);
void     swcGrabWheel(swcDisplayBase *win);
long     swcXlateCErr();


long     xlatewinsocerror(int wscode);


#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
