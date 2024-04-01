/****************************************/
/* once.c - Main program                */
/* Once-only user mode startup routines */
/****************************************/
/* Written by John Goltz                */
/****************************************/

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

// This program is run by INIT when the system starts up. It is run with
//   XOSSYS: still defined as the once-only ram-disk. The boot disk has not
//   been set up yet. Initially it reads BOOT.CFG from the ram-disk and
//   runs the programs indicated there. This will include what is necessary
//   to set up the boot disk. It then reassigns XOSSYS: to point to the
//   actual boot device and directory (as determined by comparing disk ID
//   values - see below) and gives up the ram-disk.

// BOOT.CFG is a simple text file. It is used by the real mode once-only code
//   and by this program. The real mode code uses it to determine which files
//   to copy to the RAM disk. This program uses it to determine what to do with
//   these files. Each line begins with a flag character which indicates how to
//   process the line as follows:
//   - Indicates the name (with extension) of a program or LKE to copy to
//     the RAM disk from XOSSYS:. This line is ignored here.
//   : Indicates the name (without extension) of an LKE to load.
//   $ Indicates the name (with extension) of a program to run. Command line
//     arguments may follow the name.
//   = Indicates the name (with extension) of a program to copy to
//     the RAM disk from XOSCMD:. This line is ignored here.
//   @ Indicates the name (without extension) of a program to run as a
//     symbiont. Command line arguments may follow the name.
//   ! Indicates a delay time in milliseconds
//   Any line beginning with a different character is quietly ignored. It is
//     suggested that lines intended as comments should begin with #.

// NOTE: In the following discussion CD is used to refer to any removable
//       optical disk.

// We cannot directly determine the actual boot device. The problem is that the
//   initial real mode code uses the BIOS to access the boot device. Various
//   BIOSs map the actual disk controllers and units to the BIOS disk unit
//   numbers differently. In general, there is no defined way to determine
//   what this mapping is. The following situations exist and are supported:
//   1) System with only an IDE controller booting from an IDE disk. This
// 	    is usually the easiest. The disk mapping is usually what is expected
//      (0x80->D0:, 0x81->D1:, 0x82->D2:, and 0x83->D3:).
// 	 2) System with legacy floppy controller booting from floppy. This is
// 	    also generally easy, (0x00->F0: and 0x01->F1:) although a few systems
//      provide an option for swapping drives A and B and do not provide any
//      indication that this has been done.
//   3) Booting from a SATA disk. The situation has been observed on a
//      motherboard that has one IDE port and multiple SATA ports that A
//      SATA disk will be mapped as drive 0x80 by the BIOS if no IDE drive is
//      connected but is accessed by XOS as drive D2:.
//   4) Booting from a CD/DVD drive. Similiar to the above situalion except
//      there are more ways to get confused. There is supposed to be a way
//      to determine the mapping but it appears to seldom work.
//   5) Booting from an external USB disk of some kind. Same as the situation
//      with CDs except there is no doceumented way at all to determine which
//      is the boot disk.
//   We get around this by assuming that each disk has a unique 4-byte
//   "disk ID" stored in its MBR at offset 0x1B8. The real mode code reads
//   this value from the boot device it is accessing and stores it. We compare
//   this value to the disk ID values for each disk we can access. We use the
//   first one that matches (which should be the only one that matches). The
//   only time this is likely to fail is when booting from an old removable
//   disk of some kind that does not have a disk ID stored. In this case we
//   try to use the device the BIOS determined. If that does not work we can
//   not boot from that disk! Note that we do know which partition on the disk
//   we are booting from. For non-partitioned disks (floppys and some flash
//   disks) we attempt to obtain a disk ID value from the boot block.//   

// The definition of XOSDIR: has already been set up based on the BIOS's idea
//   of what disk and directory we were loaded from. We use the directory
//   name from this with the device name determined from the disk ID.

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

///#include "init.h"

extern int errno;

extern uchar scnpage[];


#define TRMCLASS 0x4D5254L

// Main program

long oncepid;

long conhndl;
long rdhndl;

int  rdleft;

TRMMODES data = {0, 0, 0, 0, 80, 60};

char *rdbufr;

char line[100];
char msgbfr[100];
char sysname[34];
char welcome[50];

struct dichar
{	BYTE4CHAR diskid;
	char      end;
} dichar =
{	{PAR_GET|REP_HEXV,  4, "DISKID"}
};

QAB diqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// func
    0,							// status
    0,							// error
    0,							// amount
    -1,							// handle
    0,							// vector
    0, 0, 0,
    CF_PUNITS,					// option
    0,							// count
    "DISK:",					// buffer1
    NULL,						// buffer2
    NULL						// parm
};

typedef struct
{	char *pathend;
	char  dev[32];
	char  path[256];
} BOOT;

typedef struct
{   ushort src;
    ushort dst;
    long   bits;
} DEVITEM;

typedef struct
{	char   name[11];
	uchar  remove;
	ushort unitnum;
	ushort partnum;
	long   unittype;
} UNITINFO;

typedef struct
{	long name;
	long flag;
} UNITTBL;

#define SRCH_FIX 0x01
#define SRCH_CD  0x02
#define SRCH_USB 0x04
#define SRCH_RMV 0x08
#define SRCH_FLP 0x10
#define SRCH_ALL 0x1F


typedef struct
{   DEVITEM dlstdin;
    DEVITEM dlstdout;
    DEVITEM dlstderr;
    DEVITEM dlstdtrm;
} DEVLIST;

DEVLIST devlist =				// Device list for runprgm
{  {0, STDIN , XO_IN      },	//   STDIN = 1
   {0, STDOUT, XO_IN|XO_OUT},	//   STDOUT = 2
   {0, STDERR, XO_IN|XO_OUT},	//   STDERR = 3
   {0, STDTRM, XO_IN|XO_OUT},	//   STDTRM = 5
};

struct
{   LNGSTRPARM arglist;
    LNGSTRPARM devlist;
    char       end;
} runparm =
{  {PAR_SET|REP_STR, 0, IOPAR_RUNCMDTAIL},
   {PAR_SET|REP_STR, 0, IOPAR_RUNDEVLIST, (char *)&devlist, sizeof(devlist), 0}
};

QAB runqab =
{   RFNC_WAIT|RFNC_RUN,		// func    - Function
    0,						// status  - Returned status
    0,						// error   - Error code
    0,						// amount  - Process ID
    0,						// handle  - Device handle
    0,						// vector  - Vector for interrupt
    {0},					//         - Reserved
    XR_CHILDTERM,			// option  - Options or command
    0,						// count   - Count
    NULL,					// buffer1 - Pointer to file spec
    NULL,					// buffer2 - Unused
    (char *)&runparm		// parm    - Pointer to parameter area
};

struct
{	BYTE4PARM curpos;
	uchar     end;
} curparm =
{	{PAR_SET|REP_HEXV, 2, IOPAR_TRMCURPOS}
};


struct
{   LNGSTRCHAR sysname;
    BYTE4CHAR  memtotal;
	BYTE4CHAR  bootid;
    char       end;
} cfgchar =
{   {(PAR_GET|REP_STR ), 0, "SYSNAME" , sysname, 34, 0},
    {(PAR_GET|REP_DECV), 4, "TOTALMEM", 0},
    {(PAR_GET|REP_HEXV), 4, "BOOTID", 0}
};

struct
{	BYTE4CHAR sysstate;
    char      end;
} statechar =
{   {(PAR_SET|REP_HEXV), 4, "STATE", 1}
};

char systemspec[] = "SYSTEM:";

QAB cfgqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// func    - Function
    0,							// status  - Returned status
    0,							// error   - Error code
    0,							// amount  - Process ID
    0,							// handle  - Device handle
    0,							// vector  - Vector for interrupt
    0, 0, 0,					//         - Reserved
    CF_VALUES,					// option  - Options or command
    0,							// count   - Count
    systemspec, 				// buffer1 - Pointer to file spec
    (char *)&cfgchar,			// buffer2 - Unused
    NULL						// parm    - Pointer to parameter area
};


char strfin[] = "----ONCE    ******** System initialization complete";

#define VERSION 1
#define EDITNO  0

int  getline(void);
int  getpartnum(char *dname);
void defineln(BOOT *boot, long type, char *name, char *dir);
void fail(long code, char *fmt, ...);
long dorun(char *prg, char *tail);

void fatal(long code, char *fmt, ...);
void warn(long code, char *fmt, ...);


void mainmin(
    img_data *args)
{
	time_sz dt;
	BOOT    boot;
	uchar  *scnbufr;
	char   *spnt;
	char   *dpnt;
	char   *unitnames;
	char   *npnt;
	long    rtn;
	long    newhndl;
	long    dskhndl;
    int     numunits;
	int     cnt;
	int     len;
	int     partnum;
    char    buffer[160];
	char    dttmbfr[32];
	char    chr;
	char    dskname[20];

	svcSysSetPName("Once");
    if ((conhndl = svcIoOpen(XO_IN|XO_OUT, "OOCD0:", NULL)) < 0)
		exit(1);
	if ((rtn = dorun("OORDSK:symbiont.run", "SYMBIONT LOGGER")) < 0)
		warn(rtn, "Error loading symbiont LOGGER - No logging will be done");
	if ((rtn = dorun("OORDSK:server.run", "server logger add ins=1 "
			"name=XOSACT:xos")) < 0)
		warn(rtn, "Error starting logging - No logging will be done");

    oncepid = svcSysGetPid();			// Get the ONCE process ID
    svcIoQueue(&cfgqab);
    svcSysDateTime(T_GTSYSDTTM, &dt);	// Get current date and time
	rtn = sprintf(buffer, "----ONCE    ******** %s (ONCE v %d.%d) System "
			"initialization started", sysname, VERSION, EDITNO);
	svcSysLog(buffer, rtn);
    sprintf(welcome, "\n%s\n", sysname);
    sdt2str(dttmbfr, "%z%T:%m%a on %w, %D-%3n-%l", &dt);
    sprintf(buffer, "%s/ONCE - version %d.%d\nSystem initialization started "
			"at %s\nTotal memory is %dKB\n", sysname, VERSION, EDITNO,
			dttmbfr, cfgchar.memtotal.value);
    svcIoOutString(conhndl, buffer, 0);

    svcSchSetLevel(0);					// Enable signals

    if ((rdhndl = svcIoOpen(XO_IN, "OORDSK:BOOT.CFG", NULL)) < 0)
		fail(rdhndl, "Error opening BOOT.CFG");
	if ((rdbufr = sbrk(0x1000)) == (char *)-1)
		fail(ER_NEMA, "Can not allocate buffer for BOOT.CFG");
	if ((rdleft = svcIoInBlock(rdhndl, rdbufr, 0x1000)) < 0)
		fail(rdleft, "Error reading BOOT.CFG\n");
	svcIoClose(rdhndl, 0);

	if (rdleft >= 0x1000)
		fail(0, "BOOT.CFG is too long\n");

	while (getline())
	{
///		sprintf(buffer, "### |%s|\n", line);
///		svcIoOutString(conhndl, buffer, 0);

		switch (line[0])
		{
		 case ':':						// LKE to load
			strmov(strmov(buffer, "LKELOAD "), line + 1);
			if ((rtn = dorun("OORDSK:lkeload.run", buffer)) < 0)
				warn(rtn, "Error loading LKE %s", line + 1);
			break;

		 case '$':						// Program to run
			dpnt = strmov(buffer, "OORDSK:");
			spnt = line + 1;
			while ((chr = *spnt++) != 0 && !isspace(chr))
				*dpnt++ = chr;
			*dpnt = 0;
			if ((rtn = dorun(buffer, line + 1)) < 0)
				warn(rtn, "Error runing program %s", line + 1);
			break;

		 case '@':						// Symbiont to load
			strmov(strmov(buffer, "SYMBIONT "), line + 1);
			if ((rtn = dorun("OORDSK:symbiont.run", buffer)) < 0)
				warn(rtn, "Error loading symbiont %s", line + 1);
			break;

		 case '!':						// Delay time
			if ((rtn = atol(line + 1)) > 0)
				svcSchSuspend(NULL, ST_MILLISEC * rtn);

///			svcIoOutString(conhndl, "### Delay complete\n", 0);

			break;
		}
	}

///	svcIoOutString(conhndl, "### Complete\n", 0);

	// Get the BIOS's idea of the boot device and the boot directory name
	//   from the definition of XOSDIR: (which was set up by the kernel
	//   once-only code)

	if ((len = svcLogGet(0xFF, "XOSDIR:", buffer, sizeof(buffer))) < 0)
		fail(len, "Error getting definition for XOSDIR:");
	len &= 0x0FFFFFFF;
	if (len > (sizeof(boot.path) - 2))
		fail(0, "Boot directory name is too long");
	spnt = buffer;
	dpnt = boot.dev;
	do
	{
		chr = *spnt++;
		*dpnt++ = chr;
	} while (chr != 0 && chr != ':');
	if (chr == 0)
		fail(0, "Invalid device name obtained from BIOS");
	*dpnt = 0;
	strmov(boot.path, spnt);

	// Get the partition number from the disk name. This name should be of the
	//   form Du: ot Fu:if disk is not partitioned (floppy) or DuPp: if disk
	//   is partitioned.

	if ((partnum = getpartnum(boot.dev)) < 0)
		fail(0, "Invalid boot device name %s", boot.dev);

	sprintf(buffer, "BIOS Dev:%s BIOS Path:%s Partition:%d Boot disk ID:"
			"%08X\n", boot.dev, boot.path, partnum, cfgchar.bootid.value);
	svcIoOutString(conhndl, buffer, 0);

	// Find the boot disk. If there was no ID on the boot disk simply use
	//   what the BIOS said. Otherwise we require a match on the disk ID.

	if (cfgchar.bootid.value != 0)		// Do we have a disk ID for the boot
	{									//   disk?
		// Here if have a disk ID for the boot disk. Get a list of all disk
		//   units known to the system and see which one matchs the boot disk.
		//   Delay for 4 seconds first to allow time for USB disks to be set
		//   up.

///		svcSchSuspend(NULL, 4 * ST_SECOND);

	    if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.error) < 0)
			fail(rtn, "Error getting size of list of disk units");
    	numunits = diqab.count = diqab.amount;
		if ((unitnames = sbrk(diqab.count * 8)) == (char *)-1)
			fail(ER_NEMA, "Can not allocate memory for disk information");
		diqab.buffer2 = unitnames;
		if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.error) < 0)
			fail(rtn, "Error getting list of disk units");

		// Here with a list of the names of all available disk units. Scan
		//   through the list and find the one that matchs our boot disk ID

		sprintf(buffer, "Found %d disk unit%s\n", numunits,
				(numunits == 1) ? "" : "s");
		svcIoOutString(conhndl, buffer, 0);

		svcIoOutString(conhndl, "Checking:", 0);
		npnt = unitnames;
		cnt = numunits;
		while (--cnt >= 0)
		{
			while (TRUE)
			{
				sprintf(dskname, " %.8s:", npnt);
				if (partnum != getpartnum(dskname + 1))
					break;
				svcIoOutString(conhndl, dskname, 0);
				dskname[0] = '_';
	        	if ((dskhndl = svcIoOpen(XO_PHYS|XO_NOMOUNT, dskname, NULL)) < 0)
				{
					warn(rtn, "Error getting disk ID for %s", dskname);
					break;
				}
				rtn = svcIoDevChar(dskhndl, (char *)&dichar);
				svcIoClose(dskhndl, 0);
				if (rtn < 0)
				{
					warn(rtn, "Error getting disk ID for %s", dskname);
					break;
				}

				sprintf(buffer, "%08X", dichar.diskid.value);
				svcIoOutString(conhndl, buffer, 0);

				if (dichar.diskid.value == cfgchar.bootid.value)
					cnt = -1;
				break;
			}
			npnt += 8;
		}
		svcIoOutString(conhndl, "\n", 0);

		if (cnt >= -1)					// Did we find a match on the disk ID?
			fail(0, "Could not find the boot disk"); // No
		strcpy(boot.dev, dskname);		// Yes
	}
	sprintf(buffer, "Using %s as the boot device\n", boot.dev + 1);
	svcIoOutString(conhndl, buffer, 0);

	// Define the final logical names.

	boot.pathend = boot.dev + strlen(boot.dev);
	defineln(&boot, 0, "Z:", "");
	defineln(&boot, 0, "HOME:", "");
	defineln(&boot, 0, "XOSDEV:", "");
	boot.pathend = strmov(boot.pathend, boot.path);
	defineln(&boot, 0x40000000, "XOSDIR:", "");
	defineln(&boot, 0x40000000, "XOSSYS:", "sys\\");
	defineln(&boot, 0x40000000, "XOSCFG:", "cfg\\");
	defineln(&boot, 0x40000000, "XOSCMD:", "cmd\\");
	defineln(&boot, 0, "CMD:", "cmd\\");
	defineln(&boot, 0x40000000, "XOSUSB:", "usb\\");
	defineln(&boot, 0x40000000, "XOSACT:", "act\\");
	svcIoOutString(conhndl, "Logical names defined\n", 0);

	// Make sure we can access the boot directory

	svcIoOutString(conhndl, "Mounting the boot disk ...", 0);
	if ((rtn = svcIoDevParm(XO_ODFS, "XOSDIR:", NULL)) < 0)
		fail(rtn, "Error accessing the boot directory");

	// When get here the boot device should be fully set up and is being used.
	//   The full console driver should have been loaded. New we switch to
	//   the the full console driver and we completely remove the once-only
	//   code from the system.

	svcIoOutString(conhndl, "\nBoot directory accessed\n", 0);
	curparm.curpos.value = svcIoOutBlock(conhndl, NULL, -1);
										// Get the current cursor position from
										//   the once-only display driver (this
										//   is a kludge unique to OOCD0:!)

	// Save the current display contents

	if ((rtn = svcMemMapPhys(scnpage, 0xA0000, 0x03, 0x3000)) < 0)
		fail(rtn, "Error mapping console screen buffer");
	if ((scnbufr = sbrk(0x2580)) == (char *)-1)
		fail(ER_NEMA, "Error allocating screen buffer");
	memcpy(scnbufr, scnpage, 0x2580);	// Save the current screen contents

	// Open the real terminal device

	if ((newhndl = svcIoOpen(XO_IN|XO_OUT, "TRM0S1:", NULL)) < 0)
		fail(newhndl, "Error opening console device");
    if ((rtn = svcTrmDspMode(newhndl, DM_TEXT|DM_USEDATA, &data)) < 0)
        warn(rtn, "Error setting display mode");
	memcpy(scnpage, scnbufr, 0x2580);	// Resetore the screen contents
	svcIoOutBlockP(newhndl, NULL, 0, (char *)&curparm);
										// Restore the cursor position
	svcIoClose(conhndl, XC_ONCEONLY);	// Close the once-only console device
										//   - Since this is the last close this
										//   will cause the driver to remove
										//   itself from the system.
	conhndl = newhndl;					// From now on, use the real terminal
										//   device

	// Next we give up all of the once-only exec code and data.  We do this by
	//   opening the OORDSK: device and closing it with C$ONCEONLY set.

	svcIoClose(svcIoOpen(0, "OORDSK:BOOT.CFG", NULL), XC_ONCEONLY);
	svcIoOutString(conhndl, "Once-only code and data discarded\n", 0);

	// At this point all of the once-only/startup stuff should be gone. There
	//   is still quite a bit of set up to do, but it all uses normal system
	//   functions that can be done at any time. This is all driven by the
	//   XOSCFG:startup.bat file.

	// Execute the startup batch file(s)

	if ((rtn = dorun("XOSCMD:xshell.run", "xshell !XOSCFG:startup.bat")) < 0)
		warn(rtn, "Error runing startup.bat");

	// Make a final log entry

	svcSysLog(strfin, sizeof(strfin) - 1);
    svcSysDateTime(T_GTSYSDTTM, &dt);	// Get current date and time
    sdt2str(buffer, "ONCE: XOS System initialization complete at "
			"%z%T:%m%a on %w, %D-%3n-%l\n", &dt);
    svcIoOutString(conhndl, buffer, 0);
    cfgqab.buffer2 = (char *)&statechar; // Set system state to normal
    svcIoQueue(&cfgqab);
	exit(0);
}   


int getpartnum(
	char *dname)
{
	int  val;
	char chr;

	if (*dname == '_')
		dname++;
	if (!isalpha(*dname++))
		return (-1);
	val = 0;
	while ((chr = *dname) != 0 && chr != ':')
	{
		dname++;
		if (!isdigit(chr))
		{
			if (chr != 'P')
				return (-1);
			break;
		}
	}
	while ((chr = *dname++) != 0 && chr != ':')
	{
		if (!isdigit(chr))
			return (-1);
		val = val * 10 + (chr & 0x0F);
	}
	return (val);
}


void defineln(
	BOOT *boot,
	long  type,
	char *name,
	char *dir)
{
	long rtn;
///	char debug[128];

	strcpy(boot->pathend, dir);

///	sprintf(debug, "### defining %s as %s\n", name, boot->dev);
///	svcIoOutString(conhndl, debug, 0);

	if ((rtn = svcLogDefine(type | 0xFF, name, boot->dev)) < 0)
		fail(rtn, "Error defining logical name %s", name);
}


int getline(void)
{
	char *pnt;
	char  chr;

	if (rdleft < 2)
		return (FALSE);
	pnt = line;
	while (--rdleft >= 0 && (chr = *rdbufr++) != 0 && chr != '\r' &&
			chr != '\n')
		*pnt++ = chr;
	*pnt = 0;
	while (rdleft >= 0 && chr != '\n')
	{
		--rdleft;
		chr = *rdbufr++;
	}
	return (TRUE);
}



long dorun(
	char *prg,
	char *tail)
{
	long rtn;
///	char debug[128];

	runqab.buffer1 = prg;
	runqab.option = XR_CHILDTERM;
	runparm.devlist.desp = PAR_SET|REP_STR;
	devlist.dlstdin.src = conhndl;	// Set up the device list
	devlist.dlstdout.src = conhndl;
	devlist.dlstderr.src = conhndl;
	devlist.dlstdtrm.src = conhndl;
	runparm.devlist.bfrlen = runparm.devlist.strlen = sizeof(devlist);
	runparm.arglist.buffer = tail;
	runparm.arglist.bfrlen = runparm.arglist.strlen = strlen(tail);

///	sprintf(debug, "### In dorun, about to call svcIoRun for |%s|\n", prg);
///	svcIoOutString(conhndl, debug, 0);
///	sprintf(debug, "   tail: |%s|\n", tail);
///	svcIoOutString(conhndl, debug, 0);

	if ((rtn = svcIoRun(&runqab)) < 0 ||
			(rtn = runqab.error) < 0)
		return (rtn);
	return (0);
}


void fail(
	long  code,
	char *fmt, ...)
{
	va_list pi;
	char    text[200];

    va_start(pi, fmt);
    vsprintf(strmov(text, "\n? ONCE: "), fmt, pi);
	svcIoOutString(conhndl, text, 0);
	if (code < 0)
	{
		svcSysErrMsg(code, 0x03, strmov(text, "\n        "));
		strcat(text, "\n");
		svcIoOutString(conhndl, text, 0);
	}
	svcIoOutString(conhndl, "\n"
			"  *************************************\n"
			"  *  CAN NOT COMPLETE SYSTEM SET UP!  *\n"
			"  *************************************", 0);
	dead: goto dead;
}


void warn(
	long  code,
	char *fmt, ...)
{
	va_list pi;
	char    text[200];

    va_start(pi, fmt);
    vsprintf(strmov(text, "\n% ONCE: "), fmt, pi);
	svcIoOutString(conhndl, text, 0);
	if (code < 0)
	{
		svcSysErrMsg(code, 0x03, strmov(text, "\n        "));
		strcat(text, "\n");
		svcIoOutString(conhndl, text, 0);
	}
	svcIoOutString(conhndl, "\nPRESS <ENTER> TO CONTINUE", 0);
	svcIoInBlock(conhndl, text, 1);
}
