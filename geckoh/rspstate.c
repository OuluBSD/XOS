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

#include "geckoh.h"

int lkelistcnt = 0;

// Three values are used to manage LKE symbol tables. Whenever we get a state
//   chaged response it includes a lkeinmem value and an lkeloaded value.
//		lkeinmem   = Number of most recently loaded LKE that is still in memory.
//		lkeloaded  = Number of most recently loaded LKE.
//		lastlkenum = Number of highest numbered LKE for which we have tried to
//                   load symbols.
// The lke number is assigned by the kernel. It starts at 1 for the first LKE
//   loaded and is incremented by 1 for each LKE loaded.
// It is possible that one or more transisent LKEs were loaded and discarded
//   since we last had a state response, thus the lkeinmem and lkeloaded
//   value may not be the same. lkeinmem will always be less than or equal
//   to lkeloaded.
// If our lke number (lastlkenum) is less that lkeloaded we need to send a
//   request for a list of currently loaded LKEs. If lastlkenum is less than
//   lkeinmem, there are some LKE loaded for which we need symbols. If our
//   value is greater than lkeinmem we must have loaded symbols for one or
//   more transisent LKEs and we should discard these symbols. We can
//   determine both of these situations for the LKE list we obtain. We only
//   ask for a list starting with lastlkenum since an LKE cannot be discarded
//   once is has been completely loaded.


static void dispefr(void);


void rspstate(void)
{
	RINFO *rpnt;
	int    num;
	int    cnt;
	char   text[400];

	if (rsplen < sizeof(STATERP))
	{
		putstr("\1? STATE response is too short\2\n");
		return;
	}
	target.maj = rspbufr.state.version.maj;
	target.min = rspbufr.state.version.min;
	target.edit = rspbufr.state.version.edit;
	topline();
	entrytype = rspbufr.state.enttype;
	regtbl.eip.value = rspbufr.state.eip;
	regtbl.cs.value = rspbufr.state.cs;
	regtbl.efr.value = rspbufr.state.efr;
	regtbl.eax.value = rspbufr.state.eax;
	regtbl.ebx.value = rspbufr.state.ebx;
	regtbl.ecx.value = rspbufr.state.ecx;
	regtbl.edx.value = rspbufr.state.edx;
	regtbl.esp.value = rspbufr.state.esp;
	regtbl.ebp.value = rspbufr.state.ebp;
	regtbl.edi.value = rspbufr.state.edi;
	regtbl.esi.value = rspbufr.state.esi;
	regtbl.ss.value = rspbufr.state.ss;
	regtbl.ds.value = rspbufr.state.ds;
	regtbl.es.value = rspbufr.state.es;
	regtbl.fs.value = rspbufr.state.fs;
	regtbl.gs.value = rspbufr.state.gs;

	// Update the displayed register values

	cnt = 15;
	rpnt = &regtbl.eax;
	do
	{
		if (rpnt->name[2])
			sprintf(text, "%08X", rpnt->value);
		else
			sprintf(text, "%04X", (ushort)(rpnt->value));
		putstrns(rpnt->spos, text);
		rpnt++;
	} while (--cnt > 0);
	dispefr();

	if (lastlkenum < rspbufr.state.lkeinmem)
	{
		lkelistcnt++;
		reqbufr.lkelist.number = (ushort)(lastlkenum + 1);
		if (sendrequest(DBREQ_LKELIST, sizeof(LKELISTRQ)) < 0)
		{
			putstr("\1? Error sending request for LKE list\2\n");
			if (showentry())
				begincmd();
		}
	}
	else
	{
		if (rspbufr.state.lkeinmem < lastlkenum)
		{
			// Here if we have symbols loaded for one or more LKEs that have
			//   been recently discarded. We discard the symbol tables for
			//   these LKEs.

			num = rspbufr.state.lkeinmem + 1;
			do
			{
				removesymtbl(num, TRUE);
			} while (++num <= lastlkenum);
			lastlkenum = rspbufr.state.lkeinmem;
		}
		if (showentry())
			begincmd();
	}
}


//*********************************************
// Function: showentry - Display the entry line
// Returned: Nothing
//*********************************************

int showentry(void)
{
	char *ppnt;
	char  text[8];
	char  addr[200];

	switch (entrytype)
	{
	 case ENTRY_CALLED:
		ppnt = "EN";
		break;

	 case ENTRY_BKTPNT:
		ppnt = "BP";
		break;

	 case ENTRY_WCHPNT:
		ppnt = "WP";
		break;

	 case ENTRY_SINGLE:
		ppnt = "SS";
		break;

	 case ENTRY_UNXPBP:
		ppnt = "UB";
		break;

	 case ENTRY_UNXWP:
		ppnt = "UW";
		break;

	 case ENTRY_UNXTP:
		ppnt = "UT";
		break;

	 default:
		ppnt = text;
		sprintf(text, "%02X", entrytype);
	}
	fmtnumeric(addr, regtbl.eip.value);
	putstr("\4%s: %s! ", ppnt, addr);
	suspaddr = TRUE;
	reqbufr.rdmem.count = 0;
	reqbufr.rdmem.size = DMEM_DISPINST | DMEM_INST32;
	reqbufr.rdmem.addr = regtbl.eip.value;
	if (sendrequest(DBREQ_RDMEM, sizeof(RDMEMRQ)) < 0)
		return (TRUE);
	return (FALSE);
}



static void dispefr(void)
{
	FLGS *fpnt;
	char *tpnt;
	int   cnt;
	int   value;
	char  text[100];

	tpnt = text;
	fpnt = flgtbl;
	cnt = flgtblsz;
	do
	{
		value = (regtbl.efr.value >> fpnt->bit) & fpnt->mask;
		*tpnt++ = ' ';
		*tpnt++ = fpnt->name[value][0];
		*tpnt++ = fpnt->name[value][1];
		fpnt++;
	} while (--cnt > 0);
	*tpnt = 0;
	putstrns((ushort *)(scrnbufr + 696), text);
}
