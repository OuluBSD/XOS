
#define _VER  4			// Version number
#define _EDIT 2			// Edit number

// v 2.8 - 26-May-90
//   Added handling for lines longer than 255 characters (lines are broken
//   at 250 characters)
// v 3.4 - 23-Oct-93
//   Fixed bug in inpfile in IO when expanding memory during file load.
// v 3.6 - 23-Oct-94
//   Added options to set screen colors; fixed problem with updating current
//   line after failed search.
// v 3.7 - 17-Mar-99
//   Fixed problem with finding end of name to add .BAK extension when have
//   periods in the path or multiple periods in the name.
// v 3.8 - 22-Mar-99
//   Fixed some problems with correctly reporting IO errors.
// v 3.9 - 25-Mar-99
//   Fixed to display name of new file; Added "new file created" message;
//   Added name of file being edited to process name.
// v 3.10 - 16-Sep-00
//   Added support for 4 space tabs, make this the default for .C, .CPP,
//   .H, and .HPP files - also added /TAB option to force tab size to 4
//   or 8 spaces.
// v 3.11 - 14-Nov-00
//   Fixed problem with extension checking which caused page fault for all
//   extensions longer than 1 character beginning with h or H; Fixed problem
//   with giving up memory when reducing size of file buffer.

#define FILEMAX 4		// Maximum number of open files
#define WINMAX  4		// Maximum number of screen windows

#define ASTSC	0xCD	// Character for active status line
#define ABRDC   0xB3	// Character for active border line

#define SCRNBUFR 0xF0000	// Offset to use for screen buffer
#define STGLEN   50			// Length of the string search and relpace
							//   strings

#define A_CASL 0x3E		// Color attribute byte for active status line
#define A_CISL 0x7E		// Color attribute byte for idle status
#define A_CCMD 0x03		// Color attribute byte for command line
#define A_CRSP 0x0C		// Color attribute byte for response line
#define A_CTXT 0x02		// Color attribute byte for text lines
#define A_CINV 0xA0		// Color attribute byte for inverse text
#define A_CCMC 0xA0		// Color attribute byte for text cursor indicator

#define A_MASL 0x70		// Monochrome attribute byte for active status line
#define A_MISL 0x70		// Monochrome attribute byte for idle status line
#define A_MCMD 0x07		// Monochrome attribute byte for command line
#define A_MRSP 0x07		// Monochrome attribute byte for response line
#define A_MTXT 0x07		// Monochrome attribute byte for text lines
#define A_MINV 0xF0		// Monochrome attribute byte for inverse text
#define A_MCMC 0xF0		// Monochrome attribute byte for text cursor indicator
#define A_MIWC 0xF0		// Monochrome attribute byte for idle window cursor

#define VS_NOP	0x80	// No operation
#define VS_CMD	0x81	// Go to command mode
#define VS_CUP	0x82	// Cursor up
#define VS_CDW	0x83	// Cursor down
#define VS_CLT	0x84	// Cursor left
#define VS_CRT	0x85	// Cursor right
#define VS_WLT	0x86	// Word left
#define VS_WRT	0x87	// Word right
#define VS_MLT	0x88	// Margin left
#define VS_MRT	0x89	// Margin right
#define VS_DLT	0x8A	// Delete character left
#define VS_DRT	0x8B	// Delete character right
#define VS_DEN	0x8C	// Delete to end of line
#define VS_DBG	0x8D	// Delete to begining of line
#define VS_DWD	0x8E	// Delete word
#define VS_ILN	0x8F	// Insert blank line
#define VS_DLN	0x90	// Delete line
#define VS_JOI	0x91	// Join the line
#define VS_RTN	0x92	// Carrage return
#define VS_ELN	0x93	// Erase text on current line
#define VS_CAS	0x94	// Toggle case of character
#define VS_RST	0x95	// Restore line from buffer
#define VS_LIT	0x96	// Literal character
#define VS_SUP	0x97	// Scroll up
#define VS_SDW	0x98	// Scroll down
#define VS_PUP	0x99	// Page up
#define VS_PDW	0x9A	// Page down
#define VS_TIM	0x9B	// Toggle insert mode
#define VS_TOP	0x9C	// Go to top of file
#define VS_BTM	0x9D	// Go to bottom of file
#define VS_FFN	0x9E	// Find forwards next
#define VS_LFN	0x9F	// Locate forwards next
#define VS_FRF	0xA0	// Find and replace forwards next
#define VS_LRF	0xA1	// Locate and replace forwards next
#define VS_CWN  0xA2	// Change window
#define VS_ESC	0xFF

typedef struct wdb_ WDB;
typedef struct fdb_ FDB;

struct wdb_
{	long  label;		// Label = 'WDB='
	FDB  *fdb;			// Address of file FDB
	WDB  *next;			// Address of next WDB for same FDB
	long  hpos;			// Horizontal position of window on screen
	long  vpos;			// Vertical position of window on screen
	long  width;		// Width of window
	long  height;		// Height of window
	long  window;		// Size of the text window
	long  scrollb;		// Lower scrolling limit
	long  scrollt;		// Upper scrolling limit
	long  left;			// Position of left edge of window
	long  toplin;		// Top of window line (number)
	long  curlin;		// Current line (number)
	long  toppnt;		// Top of window line (pointer)
	long  dsptop;		// Top of display
	long  lstlin;		// Last current line (pointer)
	long  lsteof;		// Length of the last line put in window
	long  lstbot;		// First clear line in the window
	char *curlinbuf;	// Offset of current line buffer count byte
	char *curpnt;		// Current line (pointer)
	long  usrcur;		// Offset in window for last user cursor
	char *scrnbufr;		// Address of start of window buffer
	char *stsline; 		// Address of window status line
	long  stsfilpos;	// Place to display file name
	long  stslinpos;	// Place to display current line number
	long  stsnlnpos;	// Place to display number of lines
	long  stschrpos;	// Place to display number of characters
	long  stshcppos;	// Place to display horizontal cursor position
	long  cursorinv;	// Current position of cursor in virtual line
	long  cursorbind;	// Bound position of cursor in virtual line
	long  cursorinb;	// Position of cursor in line buffer
	long  curlincnt;	// Line buffer count
	uchar haveline;		// Non-zero if have line in the line buffer
	uchar txtattrb;		// Text attributes
	uchar invattrb;
	uchar hlstate;
	long  hllowline;	// Highlight region lowest line number in file
	long  hllowchar;	// Highlight region lowest character in line
	long  hllowchrv;
	long  hlhighline;	// Highlight region highest line number in file
	long  hlhighchar;	// Highlight region highest character in line
	long  hlhighchrv;
	long  linelen[100];	// Line length table (number of characters displayed
						//   for line, from left edge of window to end of
						//   line or right edge of window - used when
						//   highlighting text)
};

// Define value for hlstate

#define HLS_NONE 0
#define HLS_UP   1
#define HLS_DOWN 2

struct fdb_
{	long   lable;		// Label = 'FDB='
	WDB   *fwdb;		// First associated WDB
	uchar *memtop;		// Highest available memory address
	long   numlin;		// Number of lines in file
	long   numchr;		// Number of characters in file
	uchar *endpnt;		// Pointer to byte beyond end of file in buffer
	long   filelen;		// Length of file
	uchar  modified;	// Non-zero if file modified
	uchar  xxx[3];
	char   filename[80];// File name buffer
	uchar  filbfr[1];	// Start of file data buffer
};

extern WDB  *cw;
extern FDB  *cf;
extern WDB   wdb0;
extern char  kbbufr[];
extern long  swidth;
extern uchar scrnbufr[];
extern uchar invattrb;
extern uchar txtattrb;
extern char  curlinbuf[];
extern int   tabmask;
extern long  holesize;

void dspnum(long value, int minlen, long offset);
void putlinef(void);
void putline(void);
void updwinfrc(void);
void dspnumlines(WDB *wdb);
void dspcurline(WDB *wdb);
void dspnumchars(WDB *wdb);
void getline(void);
void rmvhilite(void);
