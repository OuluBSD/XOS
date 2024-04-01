/*
 * Name:
 *	XMAKE.h
 */

#define VERSION 4
#define EDITNO  0

#define PREREQ_MAGIC 123
#define FILE_MAGIC   543
#define SHELL_MAGIC  678
#define TARG_MAGIC   987
#define SYMBOL_MAGIC 653

#define MAXLIN     512
#define SYMLEN     64
#define MAXTARGETS 1200
#define CMDSIZE    16000		// Max length of command line generated
#define OPTSIZE    4000			// Max length of command options
#define LINESIZE   12000		// Max length of input line

/*
 * Data block definitions for dependency tree, rule list, and file list
 */

typedef struct filenode_  FILENODE;
typedef struct treenode_  TREENODE;
typedef struct rulenode_  RULENODE;
typedef struct macronode_ MACRONODE;

struct filenode_
{	char     *fname;
    ulong     fdatetime;
    FILENODE *chain;
};

struct treenode_
{	TREENODE *next;			// Ptr to next dependency in list
    FILENODE *file;			// Ptr to filenode for target
    TREENODE *tree_list;	// Ptr to list of dependents
    TREENODE *tree_add;		// Ptr for adding dependents
    TREENODE *side;			// Ptr to 2nd double-colon dependency
    TREENODE *side_add;		// Ptr for adding 2nd :: dependency
    char     *shell_list;	// Ptr to cmds for this dependency
    char      type;			// Type of dependency
    int       level;		// Tree level where this node was used
};


struct rulenode_
{
    RULENODE *next;
    char     *srctype;
    char     *desttype;
    char     *command;
};

struct macronode_
{	char      *token;		// Name of macro
    char      *value;		// Body of macro
    int        lock;		// Lock flag to prevent re-definition
    int        vsize;		// Size of body of macro
    MACRONODE *nested;		// Pointer for macro invokation list
    char      *position;	// Pointer into this macro when invoked
    MACRONODE *next;		// Pointer to next macro in macro list
};

struct opttbl				// Structure for options table
{   char op_name[11];		// Name of option
    void (*op_rout)(void);	// Pointer to function for option
};

// AMAKE global variables

extern char *linebuffer;	/* Buffer for each line read from Makefile  */
extern char *linepntr;		/* Pointer into Makefile buffer             */

extern FILE       *makefile;	/* FILE ptr for Makefile                    */
extern char       *xmakex;		/* New name for the makefile                */
extern TREENODE   *treepnt;	/* Pointer to dependency tree               */
extern TREENODE   *treeadd;	/* Tail pointer to dependency tree          */
extern FILENODE   *filepnt;	/* Pointer to list of file nodes            */
extern RULENODE   *rulepnt;	/* Pointer to default rule list             */
extern RULENODE   *ruleadd;	/* Tail pointer to default rule list        */
extern MACRONODE  *macpnt;	/* Pointer to macro list                    */
extern MACRONODE  *invoke;	/* List of currently invoked macros         */
extern int         mline;	/* Current input line number in Makefile    */

extern int OPT_print;		/* Print out macro and target defs         */
extern int OPT_noexec;		/* Print out commands, but don't exec      */
extern int OPT_precious;	/* Targets are not to be deleted           */
extern int OPT_envlock;		/* Don't allow re-def of enviroment macros */
extern int OPT_debug;		/* debug mode - Display all actions        */
extern int OPT_silent;		/* Silent mode - don't print out commands  */
extern int OPT_ignore;		/* Ignore error codes returned by commands */
extern int OPT_question;	/* Question mode.  Do nothing, set status  */
extern int OPT_cont;		/* Continue if possible.                   */
extern char *default_cmd;	/* Pointer to default command string       */

extern int CMD_silent;		/* Silent mode for this command            */
extern int CMD_ignore;		/* Ignore errors returned for this command */

// file.c

char *getatom(void);		// Function to read an atom from MAKEFILE
void  ungetatom(char *);
char *getcmd(void);			// Function to read a command from MAKEFILE
char *getmacro(void);		// Function reads a macro body frm MAKEFILE
int   readchr(void);		// Function to read a char from MAKEFILE
void  getinfo(FILENODE *);	// Return creation date/time for file

// tree.c

void buildtree(void);
void depend(char *, char);
void macrodef(char *);
void defltrule(char *);
void dotcmd(char *);
void dot_default(void);
FILENODE *fnodemake(char *);
FILENODE *filesrch(char *);

/*
 * Switch.c
 */

char *switchparse(int, char **);
void chkoptn(void);
struct opttbl *srchtbl(char *, struct opttbl *, int);
char *optfile(char *);
char *getfsp(char *);
int   peekchar(void);
void  getcx(char *);
void  skipsp(void);
void  badcmd(void);

/*
 * Utility.c
 */

void prtprint(unsigned char);
void cmdprint(unsigned char *);
void showerror(void);
void syntax(void);
char *getmemory(unsigned long);
char *getmore(unsigned long, char *);
int  inchr(char *, char);
int  instr(char *, char *);
void fatal(char *, char *, int);
TREENODE *tsearch(char *, TREENODE *);
int  fcompare(char *, char *);
TREENODE *firstsrch(char *, TREENODE *);
int  inslash(char *);
void research(TREENODE *, TREENODE *, int);
int  treewalk(TREENODE *, int);
void makethis(TREENODE *);
void spaces(FILE *, int);
void showtime(long);

// xosrun.c

void cntlcsrv(void);
void runprog(char *, char *, TREENODE *);
void parsecmd(char *, char *, char *, TREENODE *);
int  cmdcopy(char *, unsigned char *, TREENODE *, int, int);

/*
 * ASM functions
 */

unsigned long fctime(char *);	/* Get file creation time	*/
unsigned long cdatetime(void);	/* Get current time		*/
