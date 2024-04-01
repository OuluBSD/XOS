#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"

static int haveprefix(swcBase *win, ulong code, long argi, void *argp);
static int haveprefix2(ulong code, void *argp);

swcKBWedge::swcKBWedge(
	swcBase *winarg,
	int      maxcharsarg,
	int      prefixtimearg,
	int      totaltimearg,
	int    (*funcarg)(swcBase *winarg, long argn, void *argp, char *bufr,
			int len),
	void    *argparg)

{
	seqlistpnt = NULL;
	kbwwin = winarg;
	prefixtime = prefixtimearg;
	totaltime = totaltimearg;
	func = funcarg;
	argp = argparg;
	maxchars = maxcharsarg ;
	bufr = new char[maxchars + 2];
	bufrpnt = NULL;
}

swcKBWedge::~swcKBWedge()

{
	swcseqlist *tmp;

	while (seqlistpnt != NULL)
	{
		tmp = seqlistpnt->next;
		delete seqlistpnt;
		seqlistpnt = tmp;
	}
}



void swcKBWedge::DefineSeq(
	ushort *prefix,
	int     suffixarg,
	long    argnarg)

{
	swcseqlist *slp;
	ushort     *pfp;
	int         cnt;

	slp = new swcseqlist();
	cnt = 4;
	pfp = slp->prefix;
	while (--cnt >= 0 && *prefix != 0)
		*pfp++ = *prefix++;
	*pfp = 0;
	slp->suffix = (ushort)suffixarg;
	slp->argn = argnarg;
	slp->next = seqlistpnt;
	seqlistpnt = slp;
	kbwwin->SetGblKeyHook(slp->prefix[0], ::haveprefix, 0, this);
}

static int haveprefix(
	swcBase *win,
	ulong    code,
	long     argi,
	void    *argp)

{
	NOTUSED(win);
	NOTUSED(argi);

	return (((swcKBWedge *)argp)->haveprefix((int) code));
}

int swcKBWedge::haveprefix(
	int code)

{
	NOTUSED(code);

	if (seqlistpnt->prefix[1] == 0)		// Have single prefix character?
	{
		dataslp = seqlistpnt;			// Yes - start collecting data
		bufrpnt = bufr;
	}
	else
		prefixindex = 1;
	kbwwin->SetGblKeyFunc(::haveprefix2, this);
	return (TRUE);
}

static int haveprefix2(
	ulong code,
	void *argp)

{
	return (((swcKBWedge *)argp)->haveprefix2((int) code));
}


int swcKBWedge::haveprefix2(
	int code)

{
	swcseqlist *slp;
	int         len;

	if (bufrpnt == NULL)
	{
		slp = seqlistpnt;
		while (slp != NULL && slp->prefix[prefixindex] != code)
			slp = slp->next;
		if (slp == NULL)				// Any match?
		{
			kbwwin->SetGblKeyFunc(NULL); // No - give up
			return (FALSE);
		}
		if (slp->prefix[prefixindex + 1] != 0) // Complete match?
		{
			prefixindex++;				// No - continue
			return (TRUE);
		}
		dataslp = slp;					// Yes - start collecting data
		bufrpnt = bufr;
		return (TRUE);
	}
	if (code == dataslp->suffix)		// Have suffix?
	{
		kbwwin->SetGblKeyFunc(NULL);	// Yes - finished
		len = bufrpnt - bufr;
		*bufrpnt = 0;
		bufrpnt = NULL;
		func(kbwwin, dataslp->argn, argp, bufr, len);
	}
	else if ((bufrpnt - bufr) <= maxchars)
		*bufrpnt++ = (swckeytbl[(code >> 8) & 0x03])[code & 0x7F];
	return (TRUE);
}
