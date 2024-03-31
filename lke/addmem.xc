//--------------------------------------------------------------------------
// addmem.xc
//
// Written by: John Goltz
//
// Edit History:
//
//--------------------------------------------------------------------------

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
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES//  LOSS
//   OF USE, DATA, OR PROFITS//  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <stddef.h>
#include <ctype.h>
#include <xos.h>
#include <xoslib.h>
#include <xoserr.h>
#include <xosx.h>
#include <xosxchar.h>
#include <xosxparm.h>
#include <xosxlke.h>

#define MAJVER  4
#define MINVER  0
#define EDITNUM 0

static INITLKE initaddmem;

#pragma data_seg(_HEAD);

	LKEHEAD(ADDMEM, initaddmem, MAJVER, MINVER, EDITNUM, CLASS); 

#pragma data_seg();

// This LKE is used to add additional memory to the system. At start up only
//   that extended memory reported by the BIOS (limited to 0xFFFF KB) is set
//   up for use. We need to scan to determine how much memory is really there
//   and do not want to do that as part of the standard start up code since it
//   has been observed to cause serious problems with some systems. This LKE
//   is intended to be run from startup.bat. It will scan to find the top of
//   memory starting with the amount specified by the BIOS. Normally does
//   nothing if the size specified by the BIOS is less that 0xFFFF KB. A
//   can be forced if desired. An upper limit for the scan can also be scan
//   specified. The default is to scan the full 4GB address space.


static CHARFNC valend;
static CHARFNC valnoscan;
static CHARFNC valscan;
static CHARFNC resstr;


static CHARITEM initblk[] =
{ CTITEM(END   , DECV, u, 4, 0, NULL  , valend   , 0),
  CTITEM(NOSCAN, DECV, u, 4, 0, NULL  , valnoscan, 0),
  CTITEM(SCAN  , DECV, u, 4, 0, NULL  , valscan  , 0),
  CTLAST(RESULT, STR ,  , 0, 0, resstr, NULL     , 0)
};

// Characteristics are used as follows:
//   END   =num Specifies the maximum memory address for a scan. The scan is
//              only done if the BIOS specified size is the maximum. Default
//              is 4GB.
//   NOSCAN=num Specifies that no scan is to be done. The top of memory is
//              set to num without any varification. There is no default, a
//              non-zero value must be specified. USE THIS WITH CARE!
//   SCAN  =num Specifies the maximum memory address for a scan. This scan is
//              always done. Default is 4GB
//   If none of these is specified, a conditional scan up to 4GB is done.
//   RESULT=str Specifies a string to receive an informational message
//              indicating the result.

// If the specified top of memory is less than the current top of memory or if
//   no additional memory is found, no action is taken.

// Since this version of XOS does not support fragmented physical memory (other
//   than the normal hole between 0xA000 and 0x100000) memory is always added
//   starting with the current top of memory.

static char nomsg[] = "ADDMEM: No memory was added";


//************************************************************
// Function: initaddmem - Once-only initialization routine
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// Since this is an init-only LKE, this initialization function contains all
//   the executable code in the LKE)

#pragma code_seg ("x_ONCE");

_Packed struct
{	char pfix[8];
	char txt[64];
}    buffer = {"ADDMEM: "};
long buflen = 0;;

typedef struct
{	ulong memend;
	char  force;
	char  noscan;
	char  fill[2];
} DATA;


static long XOSFNC initaddmem(
	char **pctop,
	char **pdtop)
{
	DATA  data;
	char *pnt;
	long *pent;
	long  rtn;
	long  addr;
	ulong textval;
	ulong save;
	long  memhigh;
	ulong addrend;
	long  need;
	long  page;
	long  freelist;
	long  oldtop;

	sysLibMemSetLong((long *)&data, 0, sizeof(data)/4);	
	sysChrFixUp((CHARTBL *)&initblk);
	if (knlTda.buffer2 != NULL && (rtn = sysChrValuesNew(0, initblk,
			&data)) < 0)
		return (rtn);
	if (data.noscan && data.memend == 0)
		return (ER_VALUE);

	// Here if can scan memory

	if (data.memend == 0)
		data.memend = 0xFFFFF;
	if (!data.noscan && (data.force || data.memend > knlMemTop))
	{									// Should we scan?
		// Here to scan memory. Given the memory configurations common in
		//   modern systems, we only check for memory at 1MiB intervals.
		//   This speeds up the scan quite a bit.

		addr = (knlMemTop << 12) | 0x7B; // Address plus page bits (PWT is set)
		textval = *(long *)textdispbase;
		addrend = data.memend << 12;
		do
		{
			knlRamAccessPTE = addr;		// Map the page
			__asm("INVLPG knlRamAccess");
			save = *(ulong *)knlRamAccess;
			*(ulong *)knlRamAccess = 0x55773322; // Write a pattern
			sysSchSDelay(1);
			if (*(ulong *)knlRamAccess != 0x55773322) // Check it
				break;
			if (textval != *(ulong *)textdispbase)
			{
				*(ulong *)textdispbase = textval;
				break;
			}
			*(ulong *)knlRamAccess = 0;	// Write 0s
			sysSchSDelay(1);
			if (textval != *(ulong *)textdispbase)
			{
				*(ulong *)textdispbase = textval;
				break;
			}
			if (*(ulong *)knlRamAccess != 0) // Check it
				break;
			*(ulong *)knlRamAccess = 0xFFFFFFFF; // Write 1s
			sysSchSDelay(1);
			if (textval != *(ulong *)textdispbase)
				break;
			sysSchSDelay(1);
			if (*(ulong *)knlRamAccess != 0xFFFFFFFF) // Check it
				break;
			*(ulong *)knlRamAccess = save;
		} while ((addr += 0x100000) < addrend);
		data.memend = addr >> 12;		// Get new top page
	}

	// When get here data.memend contains the new top memory page, either
	//   because it was specified or because we scanned for it.

	if (data.memend <= knlMemTop)		// Do we have anything to add?
		buflen = sysLibStrMov(buffer.txt, nomsg);
	else
	{
		// Here if have some memory to add - Get more knlMemTable pages if
		//   we need them

		memhigh = (knlMemTop + 0x01FF) & 0xFFFFFE00;
		if ((need = ((data.memend - memhigh) + 0x01FF) & 0xFFFFFE00) > 0 &&
				(rtn = sysMemGetPages(knlMemTable + memhigh, need >> 9,
				MT_SYS, 0x23, 0)) < 0)
			return (rtn);
		sysLibMemSetLong((long *)(knlMemTable + memhigh), 0, need << 1);

		// Construct a list from the new memory pages we found

		page = knlMemTop;
		pent = &freelist;
		do
		{
			*pent = page;
			pent = &knlMemTable[page].nextfree;
			page++;
		} while (page < data.memend);

		// Find the end of the current memory free list and add our new
		//   list to the end - Must do this at fork level.

		TOFORK;
		pent = &knlMemFree;
		while ((page = *pent) != 0)
			pent = &knlMemTable[page].nextfree;
		*pent = freelist;
		oldtop = knlMemTop;
		need = data.memend - knlMemTop;
		knlMemTotal += need;
		knlMemAvail += need;
		knlMemUser += need;
		knlMemTop = data.memend;

		// Following is for debugging only

		pent = &knlMemFree;
		rtn = 0;
		while ((page = *pent) != 0)
		{
			pent = &knlMemTable[page].nextfree;
			rtn++;
		}
		if (rtn != knlMemAvail)
			CRASH('BMFL');				// [Bad Memory Free List]

		// End of debug code

		FROMFORK;

		// Construct the result string

		pnt = buffer.txt + sysLibDec2Str(buffer.txt,
				(data.memend - oldtop) * 4);
		pnt += sysLibStrMov(pnt, "KiB added at 0x");
		pnt += sysLibHex2Str(pnt, oldtop << 12, 8);
		pnt += sysLibStrMov(pnt, " to 0x");
		pnt += sysLibHex2Str(pnt, knlMemTop << 12, 8);
		buflen = pnt - ((char *)&buffer);
	}
	if (knlTda.buffer2 != NULL && (rtn = sysChrValuesNew(0, initblk,
			&data)) < 0)
		return (rtn);
	*pctop = (char *)-1;
	*pdtop = (char *)-1;
	return (0);
}


//**************************************************************
// Funcion: valend - Set the value of the END LKE characteristic
// Returned: 0 if normal or a negative XOS error code if error
//**************************************************************

static long XOSFNC valend(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	long value;
	long rtn;

	if ((rtn = sysUSGetULongV(val, &value, cnt)) < 0)
		return (rtn);
	((DATA *)data)->memend = (value >> 12) & 0xFFFFFFC00; // Round down to
	return (0);											  //   whole MiB
}


//********************************************************************
// Funcion: valnoscan - Set the value of the NOSCAN LKE characteristic
// Returned: 0 if normal or a negative XOS error code if error
//********************************************************************

static long XOSFNC valnoscan(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	((DATA *)data)->noscan = TRUE;
	return (valend(item, val, cnt, data));
}	


//****************************************************************
// Funcion: valscan - Set the value of the SCAN LKE characteristic
// Returned: 0 if normal or a negative XOS error code if error
//****************************************************************

static long XOSFNC valscan(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	((DATA *)data)->force = TRUE;
	return (valend(item, val, cnt, data));
}	


//*****************************************************************
// Funcion: resstr - Set the value of the RESULT LKE characteristic
// Returned: 0 if normal or a negative XOS error code if error
//*****************************************************************

static long XOSFNC resstr(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysLibStrNMov(val, (char *)&buffer, cnt));
}
