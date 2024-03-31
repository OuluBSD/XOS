//--------------------------------------------------------------------------*
// Program: FONTUTIL.C
//
// Written by: John Goltz
//
//--------------------------------------------------------------------------*

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

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSTRM.H>
#include <XOSRTN.H>
#include <PROGARG.H>
#include <PROGHELP.H>
#include <XOSERMSG.H>
#include <FREETYPE.H>

#define VERSION 1
#define EDITNO  0

void (*cmd)(void);		// Pointer to command function
char  *specname;		// Pointer to specified name
char  *fontname;		// Pointer to actual font name
char  *fontcopyright;
char  *fonttrademark;
char  *fontversion;

long   filehndl;

TT_Engine   ttengine;
TT_Face     ttface;
TT_Error    tterror;
TT_Face_Properties
	    ttfaceprop;

ushort      ttplatformid;
ushort      ttencodingid;
ushort      ttlanguageid;
ushort      ttnameid;


char  copymsg[] = "";
char  prgname[] = "FONTUTIL";
char  envname[] = "^XOS^FONTUTIL";
Prog_Info pib;
char  example[] = "subcmd name {options}";
char  description[] = "THe FONTUTIL command installs or removes system fonts "
	"and lists the major parameters of an installed font or font file. "
	"The \"subcmd\" argument must be INSTALL, REMOVE, or LIST. If INSTALL "
	"is specified, the \"name\" argument specifes a file containing "
	"the font to install.  If REMOVE is specified, the \"name\" argument "
	"specifies the name of the installed font to remove. If LIST is "
	"specified, the \"name\" argument specifies the name of an installed "
	"font to list unless the /FILE option is present, in which case the "
	"\"name\" arguemnt specifies the name of a font file containing the "
	"font to list. If no name is specified, a list off all currently "
	"installed fonts is generated. Font names which contain embedded spaces must be "
	"enclosed in double quotes. A font file must contain either a "
	"TrueType font or an XOS bit-mapped font.";


void  cmdinstall(void);
void  cmdremove(void);
void  cmdlist(void);
char *getname(int num);
int   havearg(char *str);
void  listbitmap(void);
void  listtruetype(void);
int   openbitmap(void);
int   openfont(void);
int   opentruetype(void);
int   optfile(void);
int   optheight(arg_spec *arg);
int   optwidth(arg_spec *arg);
int   ttfail(char *msg);


arg_spec options[] =
{   {"FILE"  , 0, NULL, AF(optfile) , 0, "Indicates that a file specification "
	    "is given instead of a font name (only valid with the LIST "
	    "sub-command)"},
    {"HEIGHT", 0, NULL,    optheight, 0, "Specifies height of bit-mapped font "
	    "to remove"},
    {"WIDTH" , 0, NULL,    optwidth , 0, "Specifies width of bit-mapped font "
	    "to remove"},
    {"?"     , 0, NULL, AF(opthelp) , 0, "Displays this message"},
    {"H*ELP" , 0, NULL, AF(opthelp) , 0, "Displays this message"},
    { NULL   , 0, NULL, AF(NULL)    , 0, NULL}
};



void main(argc, argv)
int   argc;
char *argv[];
{
    reg_pib(&pib);

    // Set Program Information Block variables

    pib.opttbl = options; 		// Load the option table
    pib.kwdtbl = NULL;
    pib.build = __DATE__;
    pib.majedt = VERSION; 		// Major edit number
    pib.minedt = EDITNO; 		// Minor edit number
    pib.copymsg = copymsg;
    pib.prgname = prgname;
    pib.desc = description;
    pib.example = example;
    pib.errno = 0;
    getTrmParms();
    getHelpClr();

    if (argc < 2)
    {
	cmderror();
	exit(1);
    }
    ++argv;
    if (!progarg(argv, 0, options, NULL, havearg,
	    (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL))
	exit(1);
    (cmd)();
}

int optfile(void)

{

    return (TRUE);
}

int  optheight(
    arg_spec *arg)

{
    arg = arg;

    return (TRUE);
}

int  optwidth(
    arg_spec *arg)

{
    arg = arg;

    return (TRUE);
}

int havearg(
    char *str)

{
    if (cmd == NULL)			// First argument?
    {
	strupr(str);
	if (strcmp("INSTALL", str) == 0)
	   cmd = cmdinstall;
	else if (strcmp("REMOVE", str) == 0)
	   cmd = cmdremove;
	else if (strcmp("LIST", str) == 0)
	   cmd = cmdlist;
	else
	{
	    printf("? FONTUTIL: Invaid sub-command %s specified\n"
		    "            (FONTUTIL /H for help)\n", str);
	    exit(1);
	}
    }
    else if (specname == NULL)		// Second argument?
    {
	specname = getspace(strlen(str) + 1);
	strcpy(specname, str);
    }
    else
    {
	cmderror();
	return (FALSE);
    }
    return (TRUE);
}

void cmdinstall(void)

{

}

void cmdremove(void)

{

}

void cmdlist(void)

{
    switch (openfont())
    {
     case 0:
	exit(1);

     case 1:
	listbitmap();
	break;

     case 2:
	listtruetype();
	break;
    }
    exit (0);
}

//**********************************************************************
// Function: openfont - Open a font file and obtain some font parameters
// Returned: 0 if error, 1 if bit-mapped font, 2 if TrueType font
//**********************************************************************

int openfont(void)

{
    int   rtn;
    ulong header;

    if ((filehndl = svcIoOpen(O_IN, specname, NULL)) <= 0)
	femsg2(prgname, "Cannot open font file", filehndl, specname);
    if ((rtn = svcIoInBlock(filehndl, (char *)&header, 4)) < 0)
	femsg2(prgname, "Error reading font file header", rtn, specname);
    if (header == '*FMB')
	return (openbitmap());
    if (header == 0x100)
	return (opentruetype());
    fprintf(stderr, "? FONTUTIL: %s is not a valid font file\n", specname);
    return (0);
}


//***************************************************
// Function: openbitmap - Open a bit-mapped font file
// Returned: 1 if normal, 0 if error
//***************************************************

int openbitmap(void)

{

    return (0);
}

//***************************************************
// Function: opentruetype - Open a TrueType font file
// Returned: 2 if normal, 0 if error
//***************************************************

int opentruetype(void)

{
    int num;

    if ((tterror = TT_Init_FreeType(&ttengine)) != 0)
	return(ttfail("Error initializing TrueType functions"));

    if ((tterror = TT_Open_Face(ttengine, specname, &ttface)) != 0)
	return(ttfail("Error opening TrueType font"));

    if ((tterror = TT_Get_Face_Properties(ttface, &ttfaceprop)) != 0)
	return(ttfail("Error getting TrueType face properties"));

    num = 0;
    do
    {
	if ((tterror = TT_Get_Name_ID(ttface, num, &ttplatformid,
		&ttencodingid, &ttlanguageid, &ttnameid)) != 0)
	    return(ttfail("Error getting TrueType name string ID"));

	if (ttplatformid == 3 && (ttencodingid == 0 || ttencodingid == 1) &&
		ttlanguageid == 0x0409)
	{
	    if (ttnameid == 0)
		fontcopyright = getname(num);
	    else if (ttnameid == 4)
	    {
		if ((fontname = getname(num)) == NULL)
		    return (ttfail("Error getting TrueType font name"));
	    }
	    else if (ttnameid == 5)
		fontversion = getname(num);
	    else if (ttnameid == 7)
		fonttrademark = getname(num);
	}
    } while (++num < ttfaceprop.num_Names);
    if (fontname == NULL)
    {
	fprintf(stderr, "No valid name found in TrueType font file %s\n",
		specname);
	return (0);
    }
    return (2);
}


void listbitmap(void)

{

}


void listtruetype(void)

{
    printf("\nTrueType Font\n\n     Name: %s\n", fontname);
    if (fontcopyright != NULL)
	printf("Copyright: %s\n", fontcopyright);
    if (fontversion != NULL)
	printf("  Version: %s\n", fontversion);
    if (fonttrademark != NULL)
	printf("Trademark: %s\n", fonttrademark);

    if ((tterror = TT_Get_Face_Properties(ttface, &ttfaceprop)) != 0)
	ttfail("Error getting TT face properties");

    printf("\nBaseline at y=0: %s\n", (ttfaceprop.header->Flags & 0x0001) ?
	    "Yes" : "No");

    printf("Left side at x=0: %s\n", (ttfaceprop.header->Flags & 0x0002) ?
	    "Yes" : "No");

    printf("Instructions may depend on point size: %s\n",
	    (ttfaceprop.header->Flags & 0x0004) ? "Yes" : "No");

    printf("Force PPEM to integer values: %s\n",
	    (ttfaceprop.header->Flags & 0x0008) ? "Yes" : "No");

    printf("Instructions may alter advance width: %s\n",
	    (ttfaceprop.header->Flags & 0x0010) ? "Yes" : "No");

    printf("\n  Points/EM = %d\n", ttfaceprop.header->Units_Per_EM);
    printf("     Ascent = %d\n", ttfaceprop.os2->usWinAscent);
    printf("    Descent = %d\n", ttfaceprop.os2->usWinDescent);
    printf("Int leading = %d\n", ttfaceprop.os2->usWinAscent +
	    ttfaceprop.os2->usWinDescent - ttfaceprop.header->Units_Per_EM);
    printf("Ext leading = %d\n", ttfaceprop.horizontal->Line_Gap -
	    (ttfaceprop.os2->usWinAscent + ttfaceprop.os2->usWinDescent) +
	    (ttfaceprop.horizontal->Ascender -
	    ttfaceprop.horizontal->Descender));
}


//***********************************************
// Function: getname - Get a TrueType name string
// Returned: Pointer to name string
//***********************************************

char *getname(
    int   num)

{
    char  *pnt;
    char  *dst;
    char  *name;
    int    cnt;
    ushort len;

    if ((tterror = TT_Get_Name_String(ttface, num, &pnt, &len)) != 0)
	return(NULL);
    cnt = len / 2;
    dst = name = getspace(cnt + 1);
    pnt++;
    while (--cnt >= 0)
    {
	*dst++ = *pnt;
	pnt += 2;
    }
    *dst = 0;
    return (name);
}

//*****************************************
// Function: ttfail - Report TrueType error
// Returned: 0
//*****************************************

int ttfail(
    char *msg)

{
    fprintf(stderr, "? FONTUTIL: %s\n            %s\n", msg, 
	    TT_ErrorToString(tterror));
    return (0);
}

void cmderror(void)

{
    fputs("\n? FONTUTIL: Command error, type FONTUTIL/H for help\n", stderr);
}
