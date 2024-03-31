//******************************
// Parameter defination for XLIB
//******************************
// Written by John Goltz
//******************************

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

#define offsetof(strt, label) (int)&(((strt *)0)->label)

#define FNAMESZ   256		// Maximum length for file specification
#define SYMMAXSZ  512		// Maximum length for symbol name
#define LISTPGSZ  50		// Page length (excluding header) for listing

// Define values for cmdindex

#define C_CREATE  1			// Create new library
#define C_INSERT  2			// Insert modules into existing library
#define C_UPDATE  3			// Update modules in existing library
#define C_EXTRACT 4			// Extract modules from existing library
#define C_LIST    5			// List only

// Define structure for the option table

struct opttable
{   char optstr[10];
    void (*optfnc)();
};

// Define structure of the name data block

typedef struct nameblk_ NAMEBLK;
struct nameblk_
{	NAMEBLK *nb_next;		// Pointer to next name block
	long     nb_offset;		// Offset to start of module
	long     nb_size;		// Size of module
	char     nb_file;		// TRUE if file specification
	char     nb_name[1];	// File specification
};

#define NB_fsp  13			// Offset for rb_fsp in nameblk

// Define structure for entry table entry

typedef struct et_ ET;
struct et_
{   ET      *et_next;		// Pointer to next entry
    NAMEBLK *et_name;		// Pointer to name block for file (NULL if
							//   module comes from input library)
    long     et_modsize;	// Size of this module
    int      et_namesize;	// Size of the module name
    int      et_listsize;	// Size of the entry list
    long     et_source;		// Offset to start of module in input
							//   library - 0 if not library
    long     et_offset;		// Offset to start of module in output
							//   library
    time_s   et_time;		// Time module created (seconds since
							//   1-Jan-70)
    char    *et_modname;	// Offset of module name
    char     et_entlist[1];	// Entry list
};

// Define structure for input OBJ file data

typedef struct
{   long     obj_offset;	// Offset in file of current entry
    long     obj_seccnt;	// Bytes left in current section

///	long     obj_block;		// Current block in OBJ file
///	long     obj_count;
///	char    *obj_pnt;
///	char    *obj_buf;
///	char    *obj_loc;

	long     obj_pos;		// Current position in file

    FILE    *obj_file;
    time_t   obj_time;
    NAMEBLK *obj_name;
	int      obj_libreclen;	// Length of MS library record
    char     obj_libflag;	// TRUE if library
    uchar    obj_language;	// Language byte from OBJ file
    char     obj_sectype;	// Current section type
    char     obj_modtype;	// Module type
} OBJ;

// Define values for obj_modtype

#define MT_XOS 1
#define MT_MS  2

// Define structure for file time data

/*
struct timedata
{   unsigned long  time;	// Time in seconds since 1-Jan-70
    unsigned short msec;	// Millisecond part of time value
};
*/

// Define OBJ file section types

#define SC_EOM    0			// End of module
#define SC_INDEX  1			// Library index section
#define SC_START  2			// Start of module section
#define SC_SEG    3			// Segment definition section
#define SC_MSECT  4			// Msect definition section
#define SC_PSECT  5			// Psect definition section
#define SC_DATA   6			// Data section
#define SC_INTERN 7			// Internal symbol definition section
#define SC_LOCAL  8			// Local symbol definition section
#define SC_EXPORT 9			// Exported symbol definition section
#define SC_STRADR 10		// Starting address section
#define SC_DEBUG  11		// Debugger address section
#define SC_STACK  12		// Initial stack address section
#define SC_FILREQ 13		// File request section
#define SC_EXTERN 14		// External symbol definition section
#define SC_SHARED 15		// Shared library definition section
#define SC_IMPORT 16		// Imported symbol definition section
#define SC_ENTRY  20		// Entry list section
#define SC_MAX    20

// Define values for MS format OBJ file sections

#define MSSC_MIN    0x80
#define MSSC_THEADR 0x80	// Module header
#define MSSC_LHEADR 0x82	// Library module header
#define MSSC_COMENT 0x88	// Comment
#define MSSC_MODEND 0x8A	// 16-bit module end
#define MSSC_386END 0x8B	// 32-bit module end
#define MSSC_EXTDEF 0x8C	// External definition
#define MSSC_TYPDEF 0x8E	// Type definition
#define MSSC_PUBDEF 0x90	// 16-bit public definition
#define MSSC_PUB386 0x91	// 32-bit public definition
#define MSSC_LOCSYM 0x92	// 16-bit local symbols
#define MSSC_LOC386 0x93	// 32-bit local symbols
#define MSSC_LINNUM 0x94	// 16-bit source line number
#define MSSC_LIN386 0x95	// 32-bit source line number
#define MSSC_LNAMES 0x96	// Name list
#define MSSC_SEGDEF 0x98	// 16-bit segment definition
#define MSSC_SEG386 0x99	// 32-bit segment definition
#define MSSC_GRPDEF 0x9A	// Group definition
#define MSSC_FIXUPP 0x9C	// Fix up previous 16-bit data image
#define MSSC_FIX386 0x9D	// Fix up previous 32-bit data image
#define MSSC_LEDATA 0xA0	// 16-bit logical data image
#define MSSC_LED386 0xA1	// 32-bit logical data image
#define MSSC_LIDATA 0xA2	// 16-bit logical repeated (iterated) data
							//   image
#define MSSC_LID386 0xA3	// 32-bit logical repeated (iterated) data
							//   image
#define MSSC_COMDEF 0xB0	// Communal names definition
#define MSSC_LOCEXD 0xB4	// External definition visible within module
							//   only
#define MSSC_FLCEXD 0xB5	// Func ext definition vis within module only
#define MSSC_LOCPUB 0xB6	// 16-bit public definition visible within
							//   module only
#define MSSC_LPB386 0xB7	// 32-bit public definition visible within
							//   module only
#define MSSC_LOCCOM 0xB8	// Communal name visible within module only
#define MSSC_MAX    0xB8

// Define bits for symbol flag byte

#define SYF_ABS    0x0080	// Absolute value
#define SYF_ENTRY  0x0040	// Entry symbol
#define SYF_ADDR   0x0020	// Address
#define SYF_SUP    0x0010	// Suppressed

extern char     prgname[];
extern int      cmdindex;
extern int      symsize;
extern int      outdd;
extern int      pageno;
extern int      headsz;
extern long     modoffset;
extern long     modsource;
extern long     firstmod;
extern char    *membtm;
extern OBJ      inplib;
extern OBJ      inpfil;
extern NAMEBLK *firstname;
extern NAMEBLK *thisname;
extern NAMEBLK *prevname;
extern NAMEBLK *libname;
extern ET      *prevet;
extern ET       ethead;
extern char    *outfsp;
extern char    *listfsp;
extern FILE    *listfp;
extern FILE    *outfp;
extern char     cmdline;
extern char     lowfirst;
extern char     havlname;
extern char     nocmd;
extern char     symbuf[];
extern char     tempname[];
extern char     notstart[];


void  badfmt(OBJ *obj);
void  badsec(OBJ *obj);
void  chkconflict(void);
void  chkhead(long amnt);
void  closeobj(OBJ *obj);
void  cpyfilent(OBJ *obj);
void  cpylibent(OBJ *obj);
void  endofsec(OBJ *obj);
char *entname(char *epnt);
void  fail(OBJ *obj, char  *msg);
void  getatom(void);
int   getbyte(OBJ *obj);
void *getelmem(int size);
char *getfsp(int offset, char *str, char *dftext);
int   getitem(OBJ *obj, uchar  size);
int   getlong(OBJ *obj);
int   getmsnum(OBJ *obj);
void  getmssym(OBJ *obj);
int   getnumber(OBJ *obj);
void  getsym(OBJ *obj);
int   getword(OBJ *obj);
void  list(void);
void  loose(void);
void  nooper(void);
void  openobj(OBJ *obj, NAMEBLK *namepnt);
void  passone(void);
void  passtwo(void);
void  putbyte(int value);
void  putlong(long value);
void  putword(int value);
void  setblk(OBJ *obj, long offset);
void  skpbyte(OBJ *obj, int amnt);
void  startmod(OBJ *obj);
int   startsec(OBJ *obj);
void (*srchatom(struct opttable *table, int size))(void);
void *xmem(int size);
