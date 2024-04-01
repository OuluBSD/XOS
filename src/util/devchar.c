//--------------------------------------------------------------------------*
// DEVCHAR.C
// Command to list or change device characteristics
//
// Written by: John R. Goltz
//
// Edit History:
// 08/20/92(brn) - Add comment header
// 05/12/94(brn) - Fix command abbreviations
// 03/08/95(sao) - Add progasst package
// 04/25/95(sao) - Changed 'brief' switch back to 'verbose'
// 05/13/95(sao) - Changed 'optional' id
// 05/16/95(sao) - Changed exit codes to reflect ALGRTN
// 05/17/95(sao) - Made 'brief' option do something (again).
// 18May95 (fpj) - Changed names from progasst.h to proghelp.h, and from
//                 optusage() to opthelp().
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

// Command format:
//   DEVCHAR {-Q{UIET}} devname {name1=parm1} {name2=parm2} ...

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xostime.h>
#include <xcmalloc.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrtn.h>
#include <xoserr.h>
#include <progarg.h>
#include <xoserrmsg.h>
#include <xosstr.h>

#define VERSION 4
#define EDITNO  0

#define MAXCHARS 20			// Maximum number of characteristics that can be
							//   specified individually

#define chrtbase ((CHRT *)0x100000)

typedef _Packed struct
{   uchar type;
    uchar length;
    char  name[8];
    union
    {   struct
        {   long num;
            long high;
        };
		llong  num64;
		ulong  num32;
	    ushort num16;
		char   num8;
        time_s dtv;
        char   ss[8];
		struct
		{	void *bufr;
		    short size;
		    short amount;
		};
    } val;
} CHRT;

typedef struct
{	char *bufr;
	long  size;
} INFOPNT;

INFOPNT *infolist;
INFOPNT *infopnt;

#if defined(syschar)
  char devname[] = "SYSTEM:";
#else
  char devname[10];			// Buffer for given device name
#endif
char   realname[16];		// Buffer for real device name

_Packed struct
{   BYTE4PARM  options;
    LNGSTRPARM name;
    char       end;
} opnparm =
{   {(PAR_SET|REP_HEXV), 4, IOPAR_FILEOPTN, XFO_PHYDEV|XFO_XOSDEV},
    {(PAR_GET|REP_STR ), 0, IOPAR_FILESPEC, realname, 16, 0}
};


QAB chrtqab =
{
#if defined(clschar) || defined(syschar)
    QFNC_WAIT|QFNC_CLASSFUNC,
#else
    QFNC_WAIT|QFNC_OPEN,
#endif
    0,						// status   - Returned status
    0,						// error    - Error code
    0,						// amount   - Amount
    0,						// handle   - Device handle
    0,						// vector   - Vector for interrupt
    {0},
    XO_PHYS|XO_NOMOUNT,		// option   - Options or command
    0,						// count    - Count
    devname,				// buffer1  - Pointer to file spec
    NULL,					// buffer2  - Unused
    NULL					// parm     - Pointer to parameter area
};

char    *buffer;
int      bufmax;
int      outsize;
int      charcnt;
int      chartotal;
int      labelsize;
CHRT    *chrtpnt = chrtbase;
char    *chrttop;

long     brief = FALSE;
long     quiet = FALSE;			// TRUE if no output wanted
long     mute = FALSE;			// TRUE if brief output wanted
char     errbfr[PROGASSTEBSZ];	// procarg error buffer


PROGINFO pib;
char  copymsg[] = "";

#if defined(clschar)
  char prgname[] = "CLSCHAR";
  char envname[] = "^XOS^CLSCHAR^OPT";
  char example[] = "{/option} class{:} char1=value1 ...";
  char description[] = "This command is used to modify and display " \
    "class characteristics for any device class in the system.  Refer " \
    "to chapter 5 of the XOS Command Reference Manual for a " \
    "description of classes and class characteristics.  The first " \
    "argument specifies the device class.  It may optionally be terminated " \
    "with a colon.  This argument is required and may not have a value.  " \
    "Each additional argument specifies a characteristic.  If no " \
    "characteristics are specified, the current values of all " \
    "characteristics for the device class are displayed.  If one or " \
    "more characteristics are specified with values, the characteristics " \
    "are set to the values specified.  The values of all characteristics " \
    "specified (with or without a new value being specified) are " \
    "displayed (unless /QUIET was specified).";
#elif defined(syschar)
  char prgname[] = "SYSCHAR";
  char envname[] = "^XOS^SYSCHAR^OPT";
  char example[] = "{/option} char1=value1 ...";
  char description[] = "This command is used to modify and display " \
    "class characteristics for any device class in the system.  Refer " \
    "to chapter 5 of the XOS Command Reference Manual for a " \
    "description of device classes and their characteristics.  Each " \
    "argument specifies a SYSTEM class characteristic.  If no " \
    "characteristics are specified, the current values of all " \
    "characteristics for the class are displayed.  If one or " \
    "more characteristics are specified with values, the characteristics " \
    "are set to the values specified.  The values of all characteristics " \
    "specified (with or without a new value being specified) are " \
    "displayed (unless /QUIET was specified).";
#else
  char prgname[] = "DEVCHAR";
  char envname[] = "^XOS^DEVCHAR^OPT";
  char example[] = "{/option} device{:} char1=value ...";
  char description[] = "This command is used to modify and display " \
    "device characteristics for any device unit in the system.  Refer " \
    "to chapter 6 of the XOS Command Reference Manual for a " \
    "description of devices and device characteristics.  The first " \
    "argument specifies the device.  It may optionally be terminated " \
    "with a colon.  This argument is required and may not have a value.  " \
    "Each additional argument specifies a characteristic.  If no " \
    "characteristics are specified, the current values of all " \
    "characteristics for the device unit are displayed.  If one or " \
    "more characteristics are specified with values, the characteristics " \
    "are set to the values specified.  The values of all characteristics " \
    "specified (with or without a new value being specified) are " \
    "displayed (unless /QUIET was specified).";
#endif


#if !defined(syschar)
  void firstarg(char *);
#endif
int   charhave(arg_data *);
void  chkbuffer(int type, int length);
void  dcharerr(long, CHRT *);
void  fatal(long);
CHRT *nextchrt(CHRT *chrt);
void  verifymem(int size);

// void helphave(void);

void illnumval(arg_data *arg);

// int  quiethave(arg_data *);

arg_spec keywords[] =
{   {"*" , ASF_LSVAL, NULL, charhave, 0},
    {NULL, 0        , NULL, NULL    , 0}
};

int havebrief( arg_data *arg);

arg_spec options[] =
{
    {"H*ELP"    , 0                 , NULL, AF(opthelp) , 0   , "This message."},
    {"?"        , 0                 , NULL, AF(opthelp) , 0   , "This message."},
    {"V*ERBOSE" , ASF_NEGATE        , NULL,    havebrief, TRUE, "Detailed output."},
    {"Q*UIET"   , ASF_BOOL|ASF_STORE, NULL, AF(&quiet)  , TRUE, "No output, except error messages."},
    {"M*UTE"    , ASF_BOOL|ASF_STORE, NULL, AF(&mute)   , TRUE, "No output, even error messages."},
    {NULL}
};

void main(
	int   argc,
	char *argv[])
{
	time_s timeval;
    char  *sep;
    char  *fmt;
    uchar *pnt;
	char  *ppnt;
	char  *lpnt;
	char  *linebufr;
    long  *vpnt;
    char  *foo[2];
    long   value;
    long   rtn;
	int    major;
	int    minor;
	int    edit;
    int    cnt;
	int    cnt2;
    int    type;
    int    rep;
    int    fsize;
    int    curpos;
    int    spaces;
    int    needed;
    char   label[100];
	uchar  mask;

    reg_pib(&pib);

    // Set defaults of control variables

    quiet=FALSE;
    mute=FALSE;

    // Set Program Information Block variables

    pib.opttbl=options; 				// Load the option table
    pib.kwdtbl=NULL;
    pib.build=__DATE__;
    pib.majedt = VERSION; 				// Major edit number
    pib.minedt = EDITNO; 				// Minor edit number
    pib.copymsg=copymsg;
    pib.prgname=prgname;
    pib.desc=description;
    pib.example=example;
    pib.errno=0;
    getTrmParms();
    getHelpClr();

    if (svcSysGetEnv(envname, (char const **)&foo[0]) > 0)
    {
		foo[1] = '\0';
		progarg(foo, 0, options, NULL, (int (*)(char *))NULL,
				(void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);
    }

    if (argc !=0)
        ++argv;
    progarg(argv, PAF_EATQUOTE, options, keywords, (int (*)(char *))NULL,
            (void (*)(char *, char *))NULL, (int (*)(void))NULL, NULL);

    if (devname[0] == '\0')				// Was a device specified?
    {									// No - fail
        if (!mute)
            fprintf(stderr, "? %s: No "
  #if defined(clschar) || defined (syschar)
                    "class"
  #else
                    "device"
  #endif
                    " specified\n", prgname);
        exit(EXIT_INVSWT);
    }

    // Here with all arguments processed - now do what we need to do

#if !(defined(clschar) || defined(syschar))
    if (!quiet && !mute)
        printf("Physical device name is _%s\n", realname);
#endif

    if (charcnt == 0)					// Any characteristics specified?
    {
		// Here if no characteristics specified - This means we want to
		//   display the values of all non-hidden characteristics. First
		//   see how much space we need

        chrtqab.option = CF_SIZE;
        if ((rtn = svcIoQueue(&chrtqab)) < 0 || (rtn = chrtqab.error) < 0)
        {
            if (rtn != ER_IFDEV)
                fatal(rtn);
            else
                chrtqab.amount = 0;
        }
        if (chrtqab.amount == 0)
        {
            fputs("No characteristics defined for "
  #if defined(clschar) || defined (syschar)
                    "class\n",
  #else
                    "device\n",
  #endif
                    stdout);
            exit(EXIT_NORM);
        }

		// Allocate the memory we need

		if ((rtn = svcMemChange((char *)chrtbase, PG_READ|PG_WRITE,
				(ushort)chrtqab.amount)) < 0)
			femsg2(prgname, "Error allocating memory", rtn, NULL);

		if (!brief)
			infolist = (INFOPNT *)getspace(((ushort *)&chrtqab.amount)[1]);

		// Get names and types for all characteristics

		chrtqab.option = CF_ALL + (PAR_GET << 8);
        chrtqab.count = chrtqab.amount;
        chrtqab.buffer2 = (char *)chrtbase;
		chrtqab.parm = (brief) ? NULL : (void *)infolist;
        if ((rtn = svcIoQueue(&chrtqab)) < 0 || (rtn = chrtqab.error) < 0)
            fatal(rtn);

	    //  When we get here, we have a list of characteristics starting at
		//    chrtbase and a list of info pointers starting at infolist. We
		//    first scan through these lists and allocate buffers as needed.

		infopnt = infolist;
		while ((type = chrtpnt->type) != 0)
		{
			type &= 0x0F;
			if (type == REP_STR || type == REP_DATAS) // String value?
			{								// Yes - allocate space for buffer
				needed = chrtpnt->val.size;
				chrtpnt->val.bufr = getspace(chrtpnt->val.size);
			}
			else
				needed = chrtpnt->length;
			if (!brief)
			{
				if (infopnt->size > 0)
				{
					infopnt->bufr = getspace(infopnt->size + 1);
					if ((infopnt->size + 3) > labelsize)
						labelsize = infopnt->size + 3;
				}
				infopnt++;
			}
			chrtpnt = nextchrt(chrtpnt);
		}
	}

    // Get or get and set the characteristics values (Note that even if quiet
	//   was specified without any values to set we still get the values.
	//   This might be useful to verify if a characteristic name is valid.

    chrtqab.option = CF_VALUES;
	chrtqab.count = ((((char *)infopnt) - ((char *)infolist)) << 16) +
			((char *)chrtpnt) - ((char *)chrtbase) + 1;
    chrtqab.buffer2 = (void *)chrtbase;
	chrtqab.parm = (brief) ? NULL : (void *)infolist;
    if ((rtn = svcIoQueue(&chrtqab)) < 0 || (rtn = chrtqab.error) < 0)
        dcharerr(rtn, chrtpnt);			// If error
    if (!quiet && !mute)				// Tell him about it if we should
    {
		// Here if should display the values we just obtained

		chrtpnt = chrtbase;
		infopnt = infolist;
		if (brief)
		{
			lpnt = linebufr = getspace(122);
			curpos = 0;
			spaces = 0;
		}
        buffer = getspace(labelsize + bufmax + 20);
        while ((type = chrtpnt->type) != 0)
        {
			ppnt = buffer;
            outsize = 0;
			label[0] = '\0';
            if (!brief)
			{
				if (infopnt->size > 0)
	                sprintf(label, " (%s)", infopnt->bufr);
				infopnt++;
			}
			ppnt += sprintf(ppnt, "%-8.8s%-*s = ", chrtpnt->name, labelsize - 1,
					label);
			type &= 0x0F;
			switch (type)
			{
			 case REP_STR:
				ppnt += sprintf(ppnt, "%.*s", chrtpnt->val.amount,
						chrtpnt->val.bufr);
				break;

			 case REP_DATAS:
                cnt = chrtpnt->val.amount;
                ppnt += sprintf(ppnt, "(size=%d)", cnt);
                pnt = (uchar *)chrtpnt->val.bufr;
                do
                {
					ppnt += sprintf(ppnt, " %02.2X", *pnt++);
                } while (--cnt > 0);
				break;

			 case REP_TEXT:
                ppnt += sprintf(ppnt, "%.*s", chrtpnt->length, chrtpnt->val.ss);
				break;

			 case REP_OCTV:
				fmt = "0%l";
				goto vcom;

			 case REP_HEXV:
				fmt = "0x%X";
				goto vcom;

			 case REP_DECV:
				fmt = "%d";
			 vcom:
                switch (chrtpnt->length)
                {
                 case 0:
                    value = 0;
                    goto longval;

                 case 1:
                    value = (uchar)(chrtpnt->val.num);
                    goto longval;

                 case 2:
                    value = (ushort)(chrtpnt->val.num);
                    goto longval;

                 case 3:
                    value = chrtpnt->val.num & 0xFFFFFFL;
                    goto longval;

                 case 4:
                    value = chrtpnt->val.num;
                 longval:
                    rep = (chrtpnt->type & 0x0F);
                    ppnt += sprintf(ppnt, (rep == REP_DECV) ? "%ld" :
                           (rep == REP_HEXV) ? "0x%lX" : "0%lo", value);
                    break;

                 case 5:
                    value = (uchar)(chrtpnt->val.high);
                    goto extval;

                 case 6:
                    value = (ushort)(chrtpnt->val.high);
                    goto extval;

                 case 7:
                    value = chrtpnt->val.high & 0xFFFFFFL;
                    goto extval;

                 case 8:
                    value = chrtpnt->val.high;
                 extval:
                    rep = (chrtpnt->type & 0x0F);
                    ppnt += sprintf(ppnt, fmt, value);
                    break;

                 default:
                    cnt = chrtpnt->length/4;
                    vpnt = (long *)&(chrtpnt->val);
                    while (TRUE)
                    {   ppnt += sprintf(ppnt, "%d", *vpnt++);
                        if (--cnt <= 0)
                            break;
                        *ppnt++ = ',';
                    }
                    break;
                }
				break;

			 case REP_BINV:
				cnt = chrtpnt->length;
				pnt = ((uchar *)&chrtpnt->val.num) + chrtpnt->length;
				ppnt = strmov(ppnt, "0b");
				while (--cnt >= 0)
				{
					cnt2 = 0;
					mask = 0x80;
					do
					{
						*ppnt++ = (*pnt & mask) ? '1' : '0';
						mask >>= 1;
					} while (--cnt2 > 0);
					pnt++;
				}
				*ppnt = 0;
				break;

			 case REP_VERN:
                if (chrtpnt->length <= 1)
                {
                    major = ((uchar *)(&chrtpnt->val))[0];
                    minor = edit = 0;
                }
                else if (chrtpnt->length == 2)
                {
                    major = ((uchar *)(&chrtpnt->val))[1];
                    minor = ((uchar *)(&chrtpnt->val))[0];
                    edit = 0;
                }
                else if (chrtpnt->length == 3)
                {
                    major = ((uchar *)(&chrtpnt->val))[2];
                    minor = ((uchar *)(&chrtpnt->val))[1];
                    edit = ((uchar *)(&chrtpnt->val))[0];
                }
                else
                {
                    major = ((uchar *)(&chrtpnt->val))[3];
                    minor = ((uchar *)(&chrtpnt->val))[2];
                    edit = ((ushort *)(&chrtpnt->val))[0];
                }
                ppnt += sprintf(ppnt, "%d.%d", major, minor);
                if (edit != 0)
                    ppnt += sprintf(ppnt, ".%d", edit);
				break;

			 case REP_TIME:
                timeval.dt = chrtpnt->val.num64;
                ppnt += sdt2str(ppnt, "%z%H:%m:%s", (time_sz *)&timeval);
				break;

			 case REP_DATE:
                timeval.dt = chrtpnt->val.num64;
                ppnt += sdt2str(ppnt, "%z%D-%3n-%y", (time_sz *)&timeval);
				break;

			 case REP_DT:
                ppnt += (((chrtpnt->val.num | chrtpnt->val.high) == 0) ?
                    	sprintf(ppnt, "None") :
						sdt2str(buffer, "%z%H:%m:%s %D-%3n-%y",
						(time_sz *)&(chrtpnt->val.dtv)));
				break;

			 case REP_DECB:
				sep = ".";
				fmt = "%d";
				goto bcom;

			 case REP_HEXB:
				sep = "-";
				fmt = "%02.2X";
				goto bcom;

			 case REP_OCTB:
				sep = "/";
				fmt = "%o";
			 bcom:
                cnt = chrtpnt->length;
                pnt = (uchar *)(&chrtpnt->val);
                while (--cnt >= 0)
                {
                    ppnt += sprintf(ppnt, fmt, *pnt++);
                    if (cnt > 0)
                        ppnt += sprintf(ppnt, sep);
                }
            }
            if (!brief)
                printf("  %s\n", buffer);
            else
            {
				outsize = ppnt - buffer;

///				fsize = (outsize <= 25) ? 26 : (outsize <= 51) ? 52 : 78;

				fsize = (outsize <= 19) ? 20 : (outsize <= 39) ? 40 :
						(outsize <= 59) ? 60 : 80;

                if ((curpos + fsize) > 80)
                {
					fputs(linebufr, stdout);
					lpnt = linebufr;
					lpnt += sprintf(linebufr, "\n%.120s", buffer);
					curpos = fsize;
                }
                else
                {
					lpnt += sprintf(lpnt, "%*s%.*s", spaces, "", 120 - curpos,
							buffer);
                    curpos += fsize;
                }
                spaces = fsize - outsize;
            }
            chrtpnt = nextchrt(chrtpnt);
        }
		if (brief)
		{
			*lpnt++ = '\n';
			*lpnt = 0;
			fputs(linebufr, stdout);
		}
    }
    exit(EXIT_NORM);
}

int havebrief(
    arg_data *arg)
{
    brief = (arg->flags & ASF_NEGATE) ? TRUE : FALSE;
    return (TRUE);
}

// Function to advance to next characteristic

CHRT *nextchrt(
    CHRT *chrt)
{
    return ((CHRT *)(((char *)chrt) + (((chrt->type & 0x0F) >= REP_DATAS) ?
			18 : (chrt->length + 10))));
}


//*************************************************
// Function: charhave - Process characteristic name
// Returned: TRUE if OK, false if error
//*************************************************

int charhave(
    arg_data *arg)
{
    char  *pnt;
    uchar *bpnt;
    long   rtn;
    ulong  value;
    ulong  bval;
    ulong  radix;
	int    csize;
    int    type;
    int    size;
    int    vsize;
    int    cnt;
    char   chr;
    char   list;

#if !defined(syschar)
    if (*devname == '\0')
    {
		firstarg(arg->name);			// Process device or class name
		return (TRUE);
    }
#endif

	if (charcnt++ == MAXCHARS)
	{
#if defined(clschar)
		fputs("% CLSCHAR: Too many characteristics, remainder ignored", stderr);

#elif defined(syschar)
		fputs("% SYSCHAR: Too many characteristics, remainder ignored", stderr);
#else
		fputs("% DEVCHAR: Too many characteristics, remainder ignored", stderr);
#endif
		return (TRUE);
	}
    strupr(arg->name);					// Convert name to upper case
    if (strlen(arg->name) > 8)
    {
        if (!mute)
            fprintf(stderr, "? %s: Characteristic name %s is too long\n",
					prgname, arg->name);
        exit(EXIT_INVSWT);
    }

	if (!brief && infolist == NULL)
		infolist = infopnt = (INFOPNT *)getspace(sizeof(INFOPNT) * MAXCHARS);

	// Get type and size for this characteristic

	verifymem(18);						// Make sure have enough memory for
    strmov(chrtpnt->name, arg->name);	//   the skelleton
    chrtqab.option = CF_ONE | (PAR_GET << 8);
	chrtqab.count = (4 << 16) + 18;
    chrtqab.buffer2 = (char *)chrtpnt;
	chrtqab.parm = (brief) ? NULL : (void *)infopnt;
    if ((rtn = svcIoQueue(&chrtqab)) < 0 || (rtn = chrtqab.error) < 0)
        dcharerr(rtn, chrtpnt);			// If error
    ++charcnt;
    type = chrtpnt->type & 0x0F;

	// Get the memory we need

    if (type == REP_STR || type == REP_DATAS)
    {
		chrtpnt->val.bufr = getspace(chrtpnt->val.size + 1);
		chkbuffer(type, chrtpnt->val.size + 1);
		csize = 18;
    }
    else
	{
		csize = chrtpnt->length + 10;
		chkbuffer(type, chrtpnt->length);
		verifymem(csize);
	}
    if (!brief)
	{
		if (infopnt->size > 0)
	    {
    	    infopnt->bufr = getspace(infopnt->size + 1);
        	if ((infopnt->size + 2) > labelsize)
            	labelsize = infopnt->size + 2;
		}
		infopnt++;
    }
    if (!(arg->flags & ADF_NONE))		// Have a value to set?
    {
		// Here if a value was specified for the parameter

        chrtpnt->type |= PAR_SET;
        if ((chrtpnt->type & 0xF) >= REP_TEXT) // Need string value?
        {
            if (arg->flags & ADF_LSVAL)
                vsize = arg->length;
            else
            {
                vsize = 0;
                arg->val.s = "";
            }
            if (type == REP_STR || type == REP_DATAS)
                size = chrtpnt->val.size - 1;
            else
                size = chrtpnt->length;
            if (vsize > size)			// Is our value too long?
            {
                if (!mute)
                    fprintf(stderr, "? %s: String value for characteristic "
                            "%s is too long\n", prgname, arg->name);
                exit(EXIT_INVSWT);
            }
            if (type == REP_STR || type == REP_DATAS)
            {
                strncpy(chrtpnt->val.bufr, arg->val.s, vsize);
                chrtpnt->val.amount = vsize;
            }
            else
                strncpy(chrtpnt->val.ss, arg->val.s, vsize);
        }
        else							// If need numeric value
        {
            value = 0;
            if ((vsize = arg->length) != 0)
            {
                list = FALSE;
                radix = 10;
                pnt = arg->val.s;
                while ((chr = *pnt++) != 0)
                {
                    if (chr == '.')
                    {
                        list = TRUE;
                        break;
                    }
                    if (chr == '-')
                    {
                        radix = 16;
                        list = TRUE;
                        break;
                    }
                }
                pnt = arg->val.s;

                if (!list)
                {
                    if (vsize > 0 && *pnt == '0')
                    {
                        pnt++;
                        vsize--;
                        radix = 8;
                        if (vsize > 0 && toupper(*pnt) == 'X')
                        {
                            pnt++;
                            vsize--;
                            radix = 16;
                        }
                    }
                    while ((chr = *pnt++) != 0)
                    {
                        if (chr == '.' || chr == '-')
                            break;
                        if (isxdigit(chr))
                        {
                            if (chr > 'A')
                                chr += 9;
                            chr &= 0x0F;
                            if (chr < radix)
                            {
                                value *= radix;
                                value += chr;
                                continue;
                            }
                        }
                    }
                }
                else
                {
                    bpnt = (uchar *)&value;
                    cnt = 4;
                    do
                    {
                        bval = 0;
                        while ((chr = *pnt++) != 0)
                        {
                            if (chr == '.' || chr == '-')
                                break;
                            if (isxdigit(chr))
                            {
                                if (chr > 'A')
                                    chr += 9;
                                chr &= 0x0F;
                                if (chr < radix)
                                {
                                    bval *= radix;
                                    bval += chr;
                                    continue;
                                }
                            }
                            illnumval(arg);
                        }
                        if (bval & 0xFFFFFF00)
                            illnumval(arg);
                        *bpnt++ = (uchar)bval;
                    } while (--cnt > 0 && chr != 0);
                }
                if (chr != 0)
                    illnumval(arg);
            }
            vsize = (value & 0xFFFF0000L)? 4:
                    (value & 0xFFFFFF00L)? 2:
                    (value != 0)? 1: 0;
            if (vsize > chrtpnt->length)
            {
                if (!mute)
                    fprintf(stderr, "? %s: Numeric value for characteristic "
                            "%s is out of range\n", prgname, arg->name);
                exit(EXIT_INVSWT);
            }
            chrtpnt->val.num = value;
            chrtpnt->val.high = 0;
        }
    }
	chrtpnt = (CHRT *)(((char *)chrtpnt) + csize);
    return (TRUE);
}


// Function to process first argument

#if !defined(syschar)

void firstarg(
    char *arg)

{
#if defined(devchar)
    long  rtn;
#endif
    char *pntn;
    char *device;
    int   cnt;
    char  chr;

    strupr(arg);
    device = arg;
    pntn = devname;
    cnt = 16;
    while ((chr = *arg++) != '\0' && chr != '=' && chr != ':')
    {
        if (--cnt < 0)
        {
            if (!mute)
                fprintf(stderr, "? %s: "
#if defined(clschar) || defined(syschar)
                        "Class"
#else
                        "Device"
#endif
                        " name %s is too long\n", prgname, devname);
            exit(EXIT_INVSWT);
        }
        *pntn++ = chr;
    }
    if (chr != '\0' && (chr != ':' || *arg != '\0'))
    {
        if (!mute)
            fprintf(stderr, "? %s: Illegal "
#if defined(clschar) || defined(syschar)
                    "class"
#else
                    "device"
#endif
                    " name %s specified\n", prgname, device);

        exit(EXIT_INVSWT);
    }
    *pntn = ':';

#if defined(devchar)
    chrtqab.parm = (uchar *)&opnparm;
    if ((rtn = svcIoQueue(&chrtqab)) < 0 || (rtn = chrtqab.error) < 0)
        fatal(rtn);
    chrtqab.func = QFNC_WAIT|QFNC_DEVCHAR;
    chrtqab.parm = NULL;
#endif
}

#endif

void illnumval(
    arg_data *arg)

{
    fprintf(stderr, "? %s: Illegal numeric value given for characteristic %s\n",
			prgname, arg->name);
    exit(EXIT_INVSWT);
}

// Function to determine size of buffer needed for value

void chkbuffer(
    int type,
    int length)

{
    int need;

    need = 0;
    switch (type)
    {
     case REP_DECV:
     case REP_HEXV:
     case REP_OCTV:
        need = length * 4;
        break;

     case REP_BINV:
     case REP_DECB:
     case REP_HEXB:
     case REP_OCTB:
        need = length * 5;
        break;

     case REP_VERN:
     case REP_TIME:
     case REP_DATE:
     case REP_DT:
        need = 20;
        break;

     case REP_DATAS:
        need = 20 + length * 3;
        break;

     case REP_TEXT:
     case REP_STR:
        need = length;
		break;
    }
    if (bufmax < need)
        bufmax = need;
}


//**************************************************************************
// Function: verifymem - Verify have enough memory to handle a charateristic
// Returned: Nothing (Does not return if error getting memory)
//**************************************************************************

void verifymem(
	int size)
{
	long rtn;

	if ((((char *)chrtpnt) + size) > chrttop)
	{
		if ((rtn = svcMemChange(chrtbase, PG_READ|PG_WRITE, 
				((char *)chrtpnt) - ((char *)chrtbase) + size)) < 0)
			femsg2(prgname, "Error allocating memory", rtn, NULL);
		chrttop = ((char *)chrtpnt) + rtn;
	}
}


// Function to report error getting or setting characteristic value

void dcharerr(
	long  code,
	CHRT *chrt)
{
    char errmsg[100];

    svcSysErrMsg(code, 3, errmsg); 		// Get error message string
    do									// Scan to find bad characteristic
    {
        if (chrt->type & PAR_ERROR)
        {
            if (!mute)
                fprintf(stderr, "? %s: Error %s characteristic %-0.8s\n"
						"           %s\n", prgname, (chrt->type & PAR_SET)?
						"setting": "getting", chrt->name, errmsg);
            exit(EXIT_INVSWT);
        }
    } while ((chrt = nextchrt(chrt))->type != 0);
    if (!mute)
        fprintf(stderr, "? %s: Error getting or setting characteristic\n"
                "           %s\n", prgname, errmsg);
    exit(EXIT_INVSWT);
}

void fatal(
    long rtn)

{
    char buffer[50];

    sprintf(buffer,
#if defined(clschar) || defined(syschar)
            "class"
#else
            "device"
#endif
            " %s", devname);
    femsg(prgname, rtn, buffer);
}
