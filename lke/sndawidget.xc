//--------------------------------------------------------------------------
// sndawidget - Widget set up routines for sndacls
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
#include "sndacls.h"

typedef struct
{	WDG  *wdg;
	long  inx;
} PATH;


#pragma code_seg ("x_ONCE");

static void XOSFNC cleanup(WDG **wdgtbl);
static long XOSFNC findpath(SCB *scb, WDG *srcpnt, WDG *tarpnt, WDG **wtbladj,
		long level, PATH *path);
static void XOSFNC addvolctl(SCB *scb, VC *volctl, WDG *wdgpth, long inx,
		long inpvol);


// NOTE: The HDA spec use of the terms "input amplifier" and "output amplifier"
//       is somewhat confusing, especially in regard to PIN nodes. Mixer and
//       selector nodes are pretty clear. A mixer or selector node may have
//       an "output amplifier at its output and a separate "input amplifier"
//       on each input. (The spec is ambiguous about "input amplifiers" on
//       each selector input, but it does appear to be possible.) PIN nodes
//       use the terms somewhat differently. A PIN node is really two separate
//       nodes, one for output and one for input. When dealing with a PIN node
//       the terms "input" and "output" are relative to the entire codec, not
//       to the PIN node! Thus the "input amplifier" has its input connected
//       to the external connection and delivers its output to whatever other
//       nodes it is connected to. This amplifier is normally used to provide
//       a gain boost for low output microphones. The "output amplifier"
//       drives the external connection and takes its input from a selector
//       that is part of the PIN node. It appears that this selector does NOT
//       allow any amplifiers on its inputs from other nodes. This "output
//       amplifier" is often unity gain (it's there only to provide a mute
//       capability) but can be used to impliment a high power amplifier to
//       directly drive a speaker system.


//******************************************************************
// Function: setupwidgets - Set up the widgets for an HDA controller
// Returned: 0 if normal or a negative XOS error code if error
//******************************************************************

// This version uses the first audio function group that it finds. If this
//   function group does not meed our minimum requirements, we cannot use
//   the HDAC. So far we have not found ANY system where this causes a
//   problem!

typedef union
{ long l;
  struct
  {	ulong agno: 7;
	ulong     : 1;
	ulong agnn: 7;
	ulong     : 1;
	ulong agns: 7;
	ulong     : 8;
	ulong mute: 1;
  };
} AMPCAP;

long XOSFNC setupwidgets(
	SCB *scb)
{
	WDG   **wdgtbl;
	PATH   *ppnt;
	PATH   *vpnt;
	WDG    *wdgpnt;
	WDG    *dacwdg;
	WDG    *adcwdg;
	WDG    *bestwdg;
	WDG    *sink;
	WDG    *source;
	WDG    *wdgpth;
	PATH   path[16];
	WDB    *wdbpnt;
	ushort *ipnt;
	union
	{ long   *l;
	  ushort *s;
	  char   *c;
	}       cpnt;
	AMPCAP  ampcap;
	long    type;
	long    bits;
	long    rtn;
	long    cnt;
	long    ncnt;
	long    icnt;
	long    inx;
	long    prev;
	long    item;
	long    next;
	long    amnt;
	long    grpnode;
	long    node;
	long    nodeaddr;
	long    firstnode;
	long    numnodes;
	long    codecbits;
	long    codecmask;
	long    codecnum;
	long    codecaddr;
	WCAP    wcap;
	long    cllen;
	long    clisz;
	long    clinc;
	long    dfltspcm;
	long    dfltssf;
	long    dfltiampc;
	long    dfltoampc;
	long    inpvolinx;
	long    outvolinx;
	long    boostinx;

///	if (offsetof(SCB, strmtbl) != (long)scb_strmtbl)
///		CRASH('SCBE');

	// Find the first codec that contains an audio function group. We only
	//   support one audio function group and simply use the first one we
	//   find. Most current systems only have one audio function group.

	codecbits = scb->codecmsk;
	codecmask = 0x0001;
	codecnum = 0;
	while (codecbits != 0)
	{
		if (codecbits & codecmask)
		{
			// Here with a codec - First read its root node to see how many
			//   function groups it contains.

			codecaddr = codecnum << 8;	// Get number of function group nodes
			if ((rtn = codeccmd(scb, codecaddr, CV_GETPARAM|CP_SNCNT)) < 0)
				return (rtn);
			grpnode = (scb->rirbrsp >> 16) & 0xFF; // Get starting node
			cnt = scb->rirbrsp & 0xFF;			// Get number of nodes
			do
			{
				// Here with a function group - see if is an audio group

				if ((rtn = codeccmd(scb, codecaddr | grpnode, CV_GETPARAM|
						CP_FGT)) < 0)
					return (rtn);
				if (((char)scb->rirbrsp) == 1) // Audio function?
					break;					   // Yes
				grpnode++;
			} while (--cnt > 0);
			if (cnt > 0)				// Did we find an audio group?
				break;					// Yes
		}
		codecbits &= ~codecmask;
		codecmask <<= 1;				// Advance to next codec
		codecnum++;
	}
	if (codecbits == 0)					// Did we find an audio function group?
		return (ER_RFGNA);				// No - fail

	// Here with an audio function group to use.
	//   c{codecnum}  = Codec number
	//   c{codecaddr} = Codec number positioned for the argument to codeccmd
	//   c{grpnode}   = Node number for the function group node

	if ((rtn = codeccmd(scb, codecaddr, CV_GETPARAM|CP_VENID)) < 0)
		return (rtn);					// Get the vendor and device IDs
	scb->codecven = scb->rirbrsp;

	if ((rtn = codeccmd(scb, codecaddr, CV_GETPARAM|CP_REVID)) < 0)
		return (rtn);					// Get the version information
	scb->codecver = scb->rirbrsp;

	if ((rtn = codeccmd(scb, codecaddr | grpnode, CV_GETPARAM|CP_SNCNT)) < 0)
		return (rtn);					// Get the starting node and node count
	firstnode = (scb->rirbrsp >> 16) & 0xFF; // Starting node
	numnodes = scb->rirbrsp & 0xFF;			 // Number of nodes

	// Here when we are ready to analyize the audio function group. We allocate
	//   space for a widget table which contains pointers to WDG structures
	//   (each in its own XMB).

	if ((rtn = sysMemGetXmb((numnodes + 1) * sizeof(WDG *),
			(char **)&wdgtbl)) < 0)
		return (rtn);
	sysLibMemSetLong((long *)wdgtbl, 0, numnodes * sizeof(WDG *) / 4);

	// Get the default parameter values for the group

	dfltspcm = (codeccmd(scb, codecaddr | grpnode,
			CV_GETPARAM|CP_SPCM) < 0) ? 0 : scb->rirbrsp;
	dfltssf = (codeccmd(scb, codecaddr | grpnode,
			CV_GETPARAM|CP_SSF) < 0) ? 0 : scb->rirbrsp;
	dfltiampc = (codeccmd(scb, codecaddr | grpnode,
			CV_GETPARAM|CP_IAMPC) < 0) ? 0 : scb->rirbrsp;
	dfltoampc = (codeccmd(scb, codecaddr | grpnode,
			CV_GETPARAM|CP_OAMPC) < 0) ? 0 : scb->rirbrsp;

	// Set up a WDG structure for each node in the group. This also sets each
	//   node to an initial state with all amplifiers set to unity gain and
	//   muted. All inputs and outputs (PIN nodes) are disabled. All converters
	//   are set to stream 0.

	node = firstnode;
	ncnt = numnodes;
	while (--ncnt >= 0)
	{
		nodeaddr = codecaddr | node;
		if ((rtn = codeccmd(scb, nodeaddr, CV_GETPARAM|CP_CLL)) < 0)
		{
			cleanup(wdgtbl);
			return (rtn);				// Get the connect list length
		}
		cllen = scb->rirbrsp & 0x7F;
		if (scb->rirbrsp & 0x80)
		{
			clisz = 2;
			clinc = 2;
		}
		else
		{
			clisz = 1;
			clinc = 4;
		}
		if ((rtn = sysMemGetXmb(sizeof(WDG) + cllen * clisz,
				(char **)&wdgpnt)) < 0)
		{
			cleanup(wdgtbl);
			return (rtn);
		}
		wdgtbl[node - firstnode] = wdgpnt;
		wdgpnt->label = 'WIDG';
		wdgpnt->node = node;
		wdgpnt->cllen = cllen;
		wdgpnt->path = 0xFFFF;
		wdgpnt->volinp = wdgpnt->volout = 0;
		if ((rtn = codeccmd(scb, nodeaddr, CV_GETPARAM|CP_AWCAP)) < 0)
		{
			cleanup(wdgtbl);
			return (rtn);				// Get the widget capabilities
		}
		wdgpnt->wcap.l = wcap.l = scb->rirbrsp;

		// Get the connect list. This is overly complex because entires
		//   can be either 8 or 16 bits (even though there is no way to use
		//   a 16-bit node number!) and the list can contain individual
		//   entries or a range of entries. (We have never seen a codec that
		//   uses the range form even when it could!) If we find any illegal
		//   items in the list we simply ignore the node. To make it easier
		//   to use the connect we store, we always store it as 16-bit
		//   individual items.

		// NOTE: We may be incrementing inx wrong when have a range of
		//       values! The spec does not address this!!

		ipnt = &wdgpnt->clist;
		cnt = cllen;
		icnt = 0;
		inx = 0;
		prev = -1;
		do
		{
			if (--icnt <= 0)			// Need more data?
			{							// Yes - get 2 or 4 items
				if ((rtn = codeccmd(scb, nodeaddr, CV_GETCLE | inx)) < 0)
				{
					cleanup(wdgtbl);
					return (rtn);
				}
				inx += clinc;
				icnt = clinc;
				cpnt.l = &scb->rirbrsp;
			}
			if (clisz == 1)
			{
				item = *cpnt.c++;
				if (item & 0x80)
					item ^= 0x8080;
			}
			else
				item = *cpnt.s++;

			if ((item & 0x8000) == 0)	// Is this a range item?
			{
				*ipnt++ = item;			// No - just store it
				prev = item;			// Remember in case its the start of
			}							//   a range
			else
			{
				item &= 0x7FFF;
				if (prev == -1 || (amnt = item - prev) < 0 || amnt > cnt)
				{						// End of a range - is it valid?
					wcap.l = 0;			// No - bad list - make this node
					break;				//   invalid
				}
				cnt -= amnt;
				do
				{	
					*ipnt++ = ++prev;
				} while (--amnt > 0);
			}
		} while (--cnt > 0);

///		if (node == 0x10)
///			INT3;

		wdgpnt->iagns = wdgpnt->oagns = 0; // Assume no gain control
		if (wcap.inamp)					// Get input amplifier parameters if
		{								//   the node has an input amplifier
			if (wcap.ampor)
			{
				if ((rtn = codeccmd(scb, nodeaddr, CV_GETPARAM|CP_IAMPC)) < 0)
				{
					cleanup(wdgtbl);
					return (rtn);
				}
				ampcap.l = scb->rirbrsp;
			}
			else
				ampcap.l = dfltiampc;
			wdgpnt->iagno = ampcap.agno;
			wdgpnt->iagnn = ampcap.agnn;
			wdgpnt->iagns = ampcap.agns;

			// Set all input amplifiers to muted with unity gain

			cnt = cllen;
			inx = 0x7080 + ampcap.agno;
			do
			{
				if ((rtn = codeccmd(scb, nodeaddr, CV_SETAGM | inx)) < 0)
				{
					cleanup(wdgtbl);
					return (rtn);
				}
				inx += 0x0100;
			} while (--cnt > 0);
		}
		if (wcap.outamp)				// Get output amplifier parameters if
		{								//   the node  has an output amplifier
			if (wcap.ampor)
			{
				if ((rtn = codeccmd(scb, nodeaddr, CV_GETPARAM|CP_OAMPC)) < 0)
				{
					cleanup(wdgtbl);
					return (rtn);
				}
				ampcap.l = scb->rirbrsp;
			}
			else
				ampcap.l = dfltoampc;
			wdgpnt->oagno = ampcap.agno;
			wdgpnt->oagnn = ampcap.agnn;
			wdgpnt->oagns = ampcap.agns;

			if ((rtn = codeccmd(scb, nodeaddr, CV_SETAGM | 0xB080 +
					ampcap.agno)) < 0)
			{
				cleanup(wdgtbl);
				return (rtn);
			}
		}
		type = wcap.type;				// Get the node type
		wdgpnt->source = FALSE;
		switch (type)
		{
		 case CWT_DAC:					// Audio output (DAC)
			wdgpnt->source = TRUE;
		 case CWT_ADC:					// Audio input (ADC)
			if (wcap.fmtor)
			{
				if ((rtn = codeccmd(scb, nodeaddr, CV_GETPARAM|CP_SPCM)) < 0)
				{
					cleanup(wdgtbl);
					return (rtn);
				}
				wdgpnt->supported = scb->rirbrsp;
				if ((rtn = codeccmd(scb, nodeaddr, CV_GETPARAM|CP_SSF)) < 0)
				{
					cleanup(wdgtbl);
					return (rtn);
				}
				if ((scb->rirbrsp & 0x01) == 0)
					wdgpnt->supported = 0;
			}
			else
			{
				wdgpnt->supported = dfltspcm;
				if ((dfltssf & 0x01) == 0)
					wdgpnt->supported = 0;
			}

			// Set stream to 0 to effectively disable the converter

			if ((rtn = codeccmd(scb, nodeaddr, CV_SETCSC | 0)) < 0)
			{
				cleanup(wdgtbl);
				return (rtn);
			}
			break;

		 case CWT_MIXER:				// Audio mixer

			break;

		 case CWT_SEL:					// Audio selector

			break;

		 case CWT_PIN:					// Pin complex (jack)
			if ((rtn = codeccmd(scb, nodeaddr, CV_GETPIND)) < 0)
			{
				cleanup(wdgtbl);
				return (rtn);
			}
			wdgpnt->pcd.l = scb->rirbrsp;
			if ((rtn = codeccmd(scb, nodeaddr, CV_GETPARAM|CP_PINC)) < 0)
			{
				cleanup(wdgtbl);
				return (rtn);
			}
			wdgpnt->pincap = scb->rirbrsp;
			if (wdgpnt->pcd.dev == PDEV_LI || wdgpnt->pcd.dev == PDEV_MIC)
				wdgpnt->source = TRUE;
			break;

		 case CWT_POWER:				// Power control

			break;

		 case CWT_VOL:					// Volume knob

			break;
		}
		wdgpnt->type = type;
		node++;
	}
	wdgtbl[node - firstnode] = NULL;	// Mark the end with a NULL

	// Here with parameters for all widgets in our table. We must determine
	//   how to connect the widgets. There are some significant differences
	//   between different HDA codecs in the interconnections that are
	//   available. Generally, the intent appears to be to provide a single
	//   audio stream consisting of 2 to 8 channels (depending on the codec)
	//   to a collection of output jacks. Currently we only support a single
	//   stereo stream. Thus we attempt to connect one stereo DAC to the all
	//   of the headphone and line-out jacks and we attempt to connect all
	//   microphone input jacks to a stereo ADC. We also identifiy a volume
	//   volume control widget to use for output and one to use for input.
	//   When complete, we have the necessary widgets configured and we have
	//   stored the codec number and the relavant widget node numbers for use
	//   during normal operation.

	dacwdg = NULL;
	adcwdg = NULL;

	// First find the DAC to use. The DAC MUST support PCM and SHOULD support
	//   the 44.1KB rate. We do not bother checking for rates below 44.1KB
	//   since we have not found ANY codecs that support these rates. We use
	//   the first DAC (based on node number) that supports both 44.1KB and
	//   48KB or, if there are none that support both, the first that supports
	//   44.1KB. (According to the spec, ALL codecs must support 44.1KB.)

	inpvolinx = outvolinx = boostinx = 0;
	node = firstnode;
	cnt = numnodes;
	bestwdg = NULL;
	while (--cnt >= 0)
	{
		wdgpnt = wdgtbl[node - firstnode];
		if (wdgpnt->type == CWT_DAC && (wdgpnt->supported & SPCM_16BIT) &&
				wdgpnt->supported & SPCM_48)
		{
			if (wdgpnt->supported & SPCM_44)
			{
				dacwdg = wdgpnt;
				break;
			}
			bestwdg = wdgpnt;
		}
		node++;
	}
	if (dacwdg == NULL)
	{
		if (bestwdg == NULL)
		{
			cleanup(wdgtbl);
			return (ER_CFGER);
		}
		dacwdg = bestwdg;
	}

	// Here with a DAC to use - Enable it and finish setting up the output SDB.


	if ((rtn = codeccmd(scb, node | codecaddr, CV_SETCSC | 0x10)) < 0)
	{
		cleanup(wdgtbl);
		return (rtn);
	}
	scb->outsdb.supported = dacwdg->supported;
	scb->outsdb.cnode = dacwdg->node | codecaddr;
	scb->outsdb.maxrate = getmaxrate(wdgpnt->supported);

	// Find the ADC to use. We will continue to load even if we don't find
	//   one.

	node = firstnode;
	cnt = numnodes;
	bestwdg = NULL;
	while (--cnt >= 0)
	{
		wdgpnt = wdgtbl[node - firstnode];
		if (wdgpnt->type == CWT_ADC && (wdgpnt->supported & SPCM_16BIT) &&
				wdgpnt->supported & SPCM_48)
		{
			if (wdgpnt->supported & SPCM_44)
			{
				adcwdg = wdgpnt;
				break;
			}
			bestwdg = wdgpnt;
		}
		node++;
	}
	if (adcwdg == NULL)
		adcwdg = bestwdg;

	if (adcwdg != NULL)
	{
		// Here with an ADC to use - Enable it and finish setting up the
		//   input SDB

		if ((rtn = codeccmd(scb, node | codecaddr, CV_SETCSC | 0x20)) < 0)
		{
			cleanup(wdgtbl);
			return (rtn);
		}
		scb->inpsdb.supported = dacwdg->supported;
		scb->inpsdb.cnode = dacwdg->node | codecaddr;
		scb->inpsdb.maxrate = getmaxrate(wdgpnt->supported);
	}

	// Find the output and input jacks we want to use and connect each one to
	//   the DAC or ADC we have selected. There will usually be one or more
	//   intermediate nodes that must be set up. We also remember any gain
	//   setable amplifiers we encounter.

	// Generally, the structure of an audio codec consists of a number of
	//   trees with and data sink (output PIN or ADC) at the base and a
	//   number of data sources (input PIN or DAC) at end of each branch.
	//   Usually, the output trees are built using selectors with no mixing
	//   possible between sources (DACs). Usually, the input trees are built
	//   using mixers such that all pins are mixed into 1 or 2 ADCs. There
	//   may be internections between the input and output trees. Most of
	//   the RealTec (everything except for 2xx series) have an input tree
	//   which feeds inputs from input PINs into the output trees. This is
	//   apparently intended be used to directly monitor inputs using one or
	//   more outputs. The RealTec 2xx series lacks this montoring capability.
	//   The Via VT1708B has a much less complex structure but does provide a
	//   monitoring capability. We do not use the monitoring capability in
	//   this version.

	// We find paths through the trees in the codec by starting at a data
	//   sink and trying all input paths recursively until we reach the
	//   desired data source.

	// Junctions in the tree are either selectors or mixers.

	sysLibMemSetLong((long *)path, 0, 15);
	path[15].wdg = (WDG *)0xFFFFFFFF;
	node = firstnode - 1;
	cnt = numnodes;
	scb->outsdb.volume[7].anode.val = 0xFFFF;
	scb->inpsdb.volume[7].anode.val = 0xFFFF;
	scb->boost[7].anode.val = 0xFFFF;
	while (--cnt >= 0)
	{
		node++;
		wdgpnt = wdgtbl[node - firstnode];
		if (wdgpnt->type == CWT_PIN) /// && wdgpnt->pcd.epc == 0)
		{
			if (wdgpnt->pcd.dev == PDEV_SPKR || wdgpnt->pcd.dev == PDEV_LO ||
					wdgpnt->pcd.dev == PDEV_HPO)
			{
				sink = wdgpnt;
				source = dacwdg;
				bits = 0xC0;
			}
			else if (adcwdg != NULL && (wdgpnt->pcd.dev == PDEV_MIC ||
					wdgpnt->pcd.dev == PDEV_LI))
			{
				sink = adcwdg;
				source = wdgpnt;
				bits = 0x20;
			}
			else
				continue;


			// Enable the PIN widget

			if ((rtn = codeccmd(scb, node | codecaddr, CV_SETPW | bits)) < 0)
			{
				cleanup(wdgtbl);
				return (rtn);
			}

			// Find the connection path to/from the PIN widget

			if ((rtn = findpath(scb, sink, source,  wdgtbl - firstnode, 0,
					path)) < 0)
			{
				cleanup(wdgtbl);
				return (rtn);
			}
			if (rtn != 1)
				continue;

			// Here with the desired connection path - Now set up the
			//   connection

			ppnt = path;
			while ((wdgpth = ppnt->wdg) != NULL)
			{
				// If this is a selector, mixer, or input PIN, get the index
				//   into the connect list to use

				nodeaddr = wdgpth->node | codecaddr;
				inx = 0;
				if (wdgpth->type == CWT_SEL || wdgpth->type == CWT_MIXER ||
						(wdgpth->type == CWT_PIN && wdgpth == sink))
				{
					icnt = wdgpth->cllen;
					next = ppnt[1].wdg->node;
					while (--icnt >= 0 && wdgpth->clist[inx] != next)
						inx++;
					if (icnt < 0)		// It must be there?
						CRASH('HDAE');

					if (wdgpth->path != 0xFFFF && // Can we set this path?
							wdgpth->path != wdgpth->node)
						CRASH('HDAE'); // No - internal error!

					// If this is a selector or input PIN, select the input

					if (wdgpth->type != CWT_MIXER)
					{
						if ((rtn = codeccmd(scb, nodeaddr, CV_SETCS | inx)) < 0)
						{
							cleanup(wdgtbl);
							return (rtn);
						}
						wdgpth->path = next;
					}
				}
				ppnt->inx = inx;

				// Unmute any amplifiers in the path - This is done separately
				//   for PIN widgets since they have their amplifiers set up
				//   differently from all other widgets. It would probably be
				//   posible to combine this into one set of tests but this
				//   seems to be a lot easier to follow.

///				if (wdgpth->node == 0x14)
///					INT3;

				if (wdgpth->type == CWT_PIN)
				{
					// Here if this is a PIN widget. In this case the "output"
					//   amplifier is in the output path, not at the output of
					//   the widget. The "input" amplifier is in the input
					//   path (This is the only bi-directional widget.)

					// PIN widgets are always sources or sinks. Although PIN
					//   widgets can be bi-directional, we only use them in
					//   one direction. Thus we only unmute one amplifier,
					//   depending on if it is a source or sink.

					if (wdgpth == path->wdg) // Is this a sink?
					{
						// Here if this PIN is a sink (output) widget. Unmute
						//   its output amplifer. (We don't bother to check
						//   to see if there is an output amplifier. Unmuting
						//   a non-existant amplifier does not harm and there
						//   will usually be one.

						if ((rtn = codeccmd(scb, nodeaddr, CV_SETAGM | 0xB000 |
								wdgpth->oagno)) < 0)
						{
							cleanup(wdgtbl);
							return (rtn);
						}
					}
					else
					{
						// Here if this PIN is a source (input) widget. Unmute
						//   its input amplifier.

						if ((rtn = codeccmd(scb, nodeaddr, CV_SETAGM | 0x7000 |
								wdgpth->oagno)) < 0)
						{
							cleanup(wdgtbl);
							return (rtn);
						}
					}
				}
				else
				{
					// Here if this is not a PIN widget. In this case the
					//   "input" amplifier is at the input side of the widget
					//   and the "output" amplifier is at the output side.
					//   Both are used. If this is a selector or mixer there
					//   may be more than one input amplifier. We only unmute
					//   the one in the path we are setting up here.

					// Unmute input amplifier if there is one

					if (wdgpth->wcap.inamp && (rtn = codeccmd(scb, nodeaddr,
							CV_SETAGM | 0x7000 | (inx << 8) |
							wdgpth->iagno)) < 0)
					{
						cleanup(wdgtbl);
						return (rtn);
					}

					// Unmute output amplifier if there is one

					if (wdgpth->wcap.outamp && (rtn = codeccmd(scb, nodeaddr,
							 CV_SETAGM | 0xB000 | wdgpth->oagno)) < 0)
					{
						cleanup(wdgtbl);
						return (rtn);
					}
				}
				ppnt++;
			}

			// Find a volume control to use for this path. If the source is a
			//   PIN we find the volume control that is closest to the sink
			//   (the ADC). If the source is a DAC, we find the volume closest
			//   to the DAC. The goal here is to find a single volume control
			//   for all outputs and one to use for all inputs. We do support
			//   multiple volume controls (up to 8) if necessary. For each
			//   microphone input (up to 8) we also record the node for its
			//   boost amplifier if there is one.

			if (sink->type == CWT_PIN)
			{
				// Here if this is an output path - Start looking for a volume
				//   control at the DAC (the source).

				vpnt = ppnt;
				while (vpnt > path)
				{
					--vpnt;
					wdgpth = vpnt->wdg;
					if (wdgpth->type != CWT_PIN && wdgpth->wcap.inamp &&
							wdgpth->iagnn > 1)
					{
						addvolctl(scb, &scb->outsdb.volume, wdgpth, vpnt->inx,
								TRUE);
						break;
					}
					if (wdgpth->wcap.outamp && wdgpth->oagnn > 1)
					{
						addvolctl(scb, &scb->outsdb.volume, wdgpth, 0, FALSE);
						break;
					}
				}
			}
			else
			{
				// Here if this is an input path - Start looking for a volume
				//   control at the ADC (the sink).

				vpnt = path;
				while ((wdgpth = vpnt->wdg)->type != CWT_PIN)
				{
					if (wdgpth->wcap.inamp && wdgpth->iagnn > 1)
					{
						addvolctl(scb, &scb->inpsdb.volume, wdgpth, vpnt->inx,
								TRUE);
						break;
					}
					if (wdgpth->wcap.outamp && wdgpth->oagnn > 1)
					{
						addvolctl(scb, &scb->inpsdb.volume, wdgpth, 0, FALSE);
						break;
					}
					vpnt++;
				}

				// Check the PIN widget (source) to see if it has a boost
				//   amplifier. This is normally used to boost low level
				//   microphone output and is not suitable for use as a
				//   volume control.

				if (source->wcap.inamp && source->iagnn > 1)
					addvolctl(scb, &scb->boost, source, 0, TRUE);
			}

			// Store the path in the widget data area. This information is
			//   not needed for the operation of the HDA but can be accessed
			//   with a device characteristic to verify that the HDA has been
			//   configured reasonably.

			// NOTE: We count on the fact that this is all done with the exec
			//       memory resource so that no one else can allocate exec
			//       static memory and that nothing else here allocates
			//       static exec memory. Thus all of the blocks allocated
			//       here will be contigous in memory.

			icnt = ppnt - path;
			if ((rtn = sysMemGetXMem(icnt * sizeof(WDB),
					(char **)&wdbpnt)) < 0)
			{
				cleanup(wdgtbl);
				return (rtn);
			}
			if (scb->wdgdata == NULL)
				scb->wdgdata = wdbpnt;
			do
			{
				--ppnt;
				wdgpth = ppnt->wdg;
				wdbpnt->node = wdgpth->node;
				wdbpnt->inx = ppnt->inx;
				wdbpnt->type = wdgpth->type;
				wdbpnt->voli = wdgpth->volinp;
				wdbpnt->volo = wdgpth->volout;
				wdbpnt->end = 0;
				wdbpnt->eapd = ((wdgpth->pincap & 0x10000) != 0);
				wdbpnt->dev = wdgpth->pcd.dev;
				wdbpnt->colr = wdgpth->pcd.color;
				wdbpnt->pos = wdgpth->pcd.loc;
				wdbpnt++;
			} while (--icnt > 0);
			wdbpnt[-1].end = TRUE;
		}
	}
	scb->outsdb.volume[7].anode.val = 0;
	scb->inpsdb.volume[7].anode.val = 0;
	scb->boost[7].anode.val = 0;
	if (scb->wdgdata != NULL)
		scb->wdgdsz = ((char *)wdbpnt) - ((char *)scb->wdgdata);
	cleanup(wdgtbl);
	return (0);
}


//****************************************************************
// Function: findpath - Function to connect a data sink (OPIN or
//                      DAC) to a data source (IPIN or ADC)
// Returned: 0 if successful or a negative XOS error code if error
//****************************************************************

// This function works by recurively trying all possible paths to the sink
//   node until one is found that is from the source.

static long XOSFNC findpath(
	SCB  *scb,
	WDG  *sink,				// Sink widget
	WDG  *source,			// Target source widget
	WDG **wtbladj,			// Address of wdgtbl adjusted by firstnode
	long  level,
	PATH *path)
{
	ushort *lpnt;
	long    lcnt;
	long    rtn;
	long    node;

	if (level >= 14)					// Too deep?
		return (0);						// Yes - say no path
	path->wdg = sink;					// Put this node in the path
	path++;
	if (sink == source)					// Is this the target source?
	{
		path->wdg = NULL;				// Yes - indicate end of path
		return (1);						// Return success
	}
	if (level > 0 && (sink->type <= CWT_ADC || sink->type == CWT_PIN))
		return (0);						// No path is at some other source
										//   or sink

	// If this node is a selector which is being used, only try the input that
	//   is already being used.

	if (sink->type == CWT_SEL && sink->path != 0xFFFF)
		return (findpath(scb, wtbladj[sink->path], source, wtbladj, level + 1,
				path));

	// If this node is not being used or is a mixer, try all inputs

	lpnt = sink->clist;
	lcnt = sink->cllen;
	do
	{
		if ((node = *lpnt++) > 1 && (rtn = findpath(scb, wtbladj[node], source,
				wtbladj, level + 1, path)) != 0) // Try this path if it exists
			return (rtn);				// We found it!
	} while (--lcnt > 0);
	return (0);							// Couldn't find a path through this
}										//   node



static void XOSFNC addvolctl(
	SCB *scb, 
	VC  *volctl,
	WDG *wdgpth,
	long inx,
	long inpvol)
{
	// First, see if we are already using this node for volume control

	while (volctl->anode.val != 0 && volctl->anode.val != 0xFFFF &&
			volctl->anode.val != (wdgpth->node | (inx << 12)))
		volctl++;

	if (volctl->anode.val == 0xFFFF)	// Have too many volume controls?
		return;							// Yes - forget it!
	if (volctl->anode.val != 0)			// Did we find it?
		return;							// Yes - nothing needed here
	volctl->anode.num = wdgpth->node;	// No - add it to the list
	volctl->anode.inx = inx;
	if (inpvol)							// Using an input amplifier?
	{
		wdgpth->volinp |= TRUE;
		volctl->agno = wdgpth->iagno | 0x80;
		volctl->agns = wdgpth->iagns;
		volctl->agnn = wdgpth->iagnn;
	}
	else
	{
		wdgpth->volout |= TRUE;
		volctl->agno = wdgpth->oagno;
		volctl->agns = wdgpth->oagns;
		volctl->agnn = wdgpth->oagnn;
	}
}


long XOSFNC getmaxrate(
	long supported)
{
	RTVAL *rtpnt;
	long   bit;

	// Find the bit for the highest rate

	bit = SPCM_192;
	do
	{
		if (supported & bit)
		{
			rtpnt = ratetbl;
			while (TRUE)
			{
				if (rtpnt->rbit & bit)
					return (rtpnt->rate);
				rtpnt++;
			}
		}
	} while ((bit >>= 1) != 0);
	return (0);
}


//**************************************************
// Function: cleanup - Give up XMBs before returning
// Returned: Nothing
//**************************************************

static void XOSFNC cleanup(
	WDG **wdgtbl)
{
	WDG **wtpnt;
	WDG  *wpnt;

	wtpnt = wdgtbl;
	while ((wpnt = *wtpnt++) != NULL)
		sysMemGiveXmb(wpnt);
	sysMemGiveXmb(wdgtbl);
}
