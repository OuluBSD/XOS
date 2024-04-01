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
#include <xoserr.h>
#include <xosx.h>
#include <xosxchar.h>
#include <xosxparm.h>
#include <xosxlke.h>
#include <xoslib.h>
#include <xosnet.h>
#include <xosxnet.h>
#include <xosxip.h>
#include <xosxarp.h>

// This file contains routines with manage the IP routing cache

// Structure of the routing cache:

// The routing cache consists of a number of RTT (RouTe Table) items, each
//   stored in an XMB. There is a 256 entry hash table used to find an RTT
//   given its IP address. There is also a doubly linked list of all RTTs.
//   There are 3 basic types of RTTs: Local, foreign, and router. Each RTT
//   is also either pending or complete. A pending RTT does not have a
//   hardware address and probably has a list of held packets which are to
//   be sent when the hardware address is known. A complete RTT has a
//   hardware address and has no held packets. Local RTTs are stand-along,
//   they are not linked to any other RTTs. A router RTT has a doubly linked
//   list of foreign RTTs for IP addresses that are accessed though the
//   router. All foreign RTTs have the same hardware address as the associated
//   router RTT. Entries are only made for foreign addresses when a redirect
//   request is received from a router. If a foreign addresses is not found
//   in the cache the default router is used.

// We try to keep a balance between time spent searching the routing cache
//   and the number of times we need to use ARP to get an address. Generally,
//   we DO NOT search the cache and add entries when non-ARP messages are
//   received. The number of times this would add an entry is very small and
//   does not seem worth the time even though it would save an ARP request/
//   response when we respnd to a message from a new node. We do clear an
//   entry from the cache if a higher level protocol decides that a node is
//   not responding. We do add entries from all broadcast ARP messages but
//   we will not remove an entry to make room for one that is not targetted
//   to us.


static void XOSFNC givertt(IPNPDB *npdb, RTT *rtt);
static long XOSFNC makertt(IPNPDB *npdb, long inx, long ipaddr, RTT **prtt);
static long XOSFNC rtsearch(IPNPDB *npdb, long ipaddr, RTT **prtt,
		RTT ***ppprev);
static void XOSFNC routeremove(long code, IPNPDB *npdb, RTT *rtt);


static void XOSFNC unpendrtt(IPNPDB *npdb, RTT *rtt, llong hwaddr);


static long XOSFNC unhold(RTT *rtt, char hwaddr[6]);


//***************************************************************
// Function: xosipRtChkAddr - Check if IP address is in the cache
// Returned: 0 if normal (address was found) or a negative XOS
//           error code if error
//***************************************************************

long XOSFNC __export xosipRtChkAddr(
	NPDB *npdb,				// Address of IP NPDB
	long  addr)				// IP address
{


	return (ER_NIYT);
}


//***************************************************************
// Function: routegetaddr - Get hardware address for packet given
//                             IP address
// Returned: 0 (GHWA_NOSEND) if there is nothing to send, 1
//           (GHWA_SENDPKT) if should send packet on the network
//           2 (GHWA_SELF) if should send packet to self, 3
//           (GHWA_UNKNOWN) if address is not known (only when
//           the test argument is TRUE, or a negative XOS error
//           code if error.
//***************************************************************

// If the address is a "self" address, nothing is done here and GHWA_SELF
//   is returned. Otherwise the address cache is searched and if the address
//   is found case and is pending, the packet is added to the entry's hold
//   list and GHWA_NOSEND is returned. If the  entry is not pending, the
//   hardware address is filled in in the packet and GHWA_SENDPKT is returned.
//   If it is not found and the IP address is local, a pending cache entry
//   is created, the packet is put on the entry's hold list, an ARP request
//   is sent and GHWA_NOSEND is returned. If the IP address is not local and
//   a default router entry exists, the router's address is used and
//   GHWA_SENDPKT is returned. If a default router is specified but has no
//   entry, an entry for it is created much as described for a local address
//   above. If no default router is specified, an ER_NAUNR error is returned.

// If a value of 1 or an error code is returned, the packed has been consummed
//   except that the packet is never consummed if the test argument is TRUE.

long XOSFNC routegetaddr(
	IPNPDB *npdb,
	NPB    *npb,
	long    test)		// TRUE if only testing for known address
{
	RTT  *rtt;
	RTT  *rttp;
	long  rtraddr;
	RTT **prtrrtt;
	union
	{	char c[4];
		long l;
	}     ipaddr;
	long  rtn;
	long  inx;

	ipaddr.l = ((IPPKT *)(npb->data + npb->npofs))->dstaddr.l;
										// Get the destination IP address from
										//   the packet
	if ((ipaddr.l & npdb->ipsubnetmask) == 0 && npdb->ipsubnetmask != 0)
		return (ER_NILAD);				// Network part cannot be 0
	if (ipaddr.l == 0xFFFFFFFF)			// Limited broadcast address?
	{
		npdb->idb->ifdisp->setbcaddr(npdb->idb, npb); // Yes
		return (GHWA_SENDPKT);
	}
	if (ipaddr.c[0] == 127 || ipaddr.l == npdb->ipaddr)
	{									// Sending to self?
		npb->sts |= NPS_SELF;			// Yes - Indicate sending packet to self
		return (GHWA_SELF);
	}
	if ((inx = rtsearch(npdb, ipaddr.l, &rttp, NULL)) >= 0)
	{
		// Here if routing cache entry (RTT) found for the IP address

		rtt = rttp;
		if (rtt->type & RTT_PENDING)	// Is this a pending entry?
		{
			if (test)
				return (GHWA_UNKNOWN);			
		}
		else
		{
			rtt->used = knlSchCounts;	// Not pending - update the timestamp
			if (rtt->type == RTT_FOREIGN)
				rtt = rtt->router;
			npdb->idb->ifdisp->sethwaddr(npdb->idb, npb, rtt->hwaddr.c);
			return (GHWA_SENDPKT);
		}
	}
	else
	{
		// Here if no routing cache entry found for the IP address.

		if (test)						// Just testing?
			return (GHWA_UNKNOWN);		// Yes
		if ((ipaddr.l & npdb->ipsubnetmask) == npdb->ipnetaddr)
		{								// Local IP address?
			// Here with a local IP address. Create an RTT_LPENDING or
			//   RTT_RPENDING entry, put the packet on its hold list, send
			//   an ARP request for it and return NOSEND.

			if ((rtn = makertt(npdb, inx, ipaddr.l, &rttp)) < 0)
				return (rtn);
			rtt = rttp;
			rtt->hwaddr.ll = 0;
			rtt->type = (ipaddr.l == npdb->iprtraddr1 ||
					ipaddr.l == npdb->iprtraddr2) ? RTT_RPENDING : RTT_LPENDING;		
			if ((rtt->prev = npdb->iprttail) != NULL) // Put it on the local
				npdb->iprttail->next = rtt;			  //   list
			else
				npdb->iprthead = rtt;
			npdb->iprttail = rtt;
			rtt->retrycnt = 3;			// Do this 3 times
			rtt->retrytmr = 3;			// Every 3 seconds
			xosarpSendPkt(&npdb->iparpdb, 0x01, NULL, ipaddr.c);
		}								// Send an ARP request
		else
		{
			// Here with a foriegn IP address. If we have an entry for a
			//   default router, simply use the router's hardware address.
			//   We do not bother making an entry for the foriegn IP address
			//   (Foriegn entries are only created when a redirect request
			//   is received from a router.) If we have a default router
			//   specified but don't have an entry for it we create one much
			//   as for any other local IP address.


			if (npdb->iprtrrtt1 != NULL || npdb->iprtrrtt2 != NULL)
			{							// Do we have a known router yet?

				// Here with an entry for a default router. This is the path
				//   that will be taken almost all of the time for a foriegn
				//   IP address.

				rtt = (npdb->iprtrrtt1 != NULL) ? npdb->iprtrrtt1 :
						npdb->iprtrrtt2;
				if (rtt->type == RTT_ROUTER) // Is the router entry pending?
				{
					npdb->idb->ifdisp->sethwaddr(npdb->idb, npb, rtt->hwaddr.c);
					return (GHWA_SENDPKT); // No - use its address
				}
				if (rtt->type != RTT_RPENDING) // Yes - must wait for it
					CRASH('BRTT');
			}
			else
			{
				// Here if don't have a known router yet. First, make sure we
				//   have at least one router IP address specified. If not,
				//   there's not much we can do but fail. This is a fair
				//   amount of code that will most likely only be executed one
				//   at startup, but there's no way around that. It is possible
				//   for a router to change during operation of the system,
				//   but that is not very likely.

				if (npdb->iprtraddr1 == 0 && npdb->iprtraddr2 == 0)
					return (ER_NAUNR);

				// Make an RTT_RPENDING entry for the first router IP address
				//   we have. If this fails, we will try the other router
				//   address, if there is one.

				if (npdb->iprtraddr1 != 0)
				{
					rtraddr = npdb->iprtraddr1;
					prtrrtt = &npdb->iprtrrtt1;
				}
				else
				{
					rtraddr = npdb->iprtraddr2;
					prtrrtt = &npdb->iprtrrtt2;
				}
				if ((inx = rtsearch(npdb, rtraddr, &rttp, NULL)) >= 0)
					CRASH('RTEX');		// Make sure it's not there - This
										//   also calculates the hash index
										//   which makertt needs.

				if ((rtn = makertt(npdb, inx, rtraddr, &rttp)) < 0)
					return (rtn);
				*prtrrtt = rtt = rttp;
				rtt->type = RTT_RPENDING;
				if ((rtt->prev = npdb->iprttail) != NULL)
					npdb->iprttail->next = rtt; // Put the router entry on the
				else							//   entry list
					npdb->iprthead = rtt;
				npdb->iprttail = rtt;
				rtt->retrycnt = 3;		// Do this 3 times
				rtt->retrytmr = 3;		// Every 3 seconds
				xosarpSendPkt(&npdb->iparpdb, 0x01, NULL, (char *)&rtraddr);
										// Send an ARP request for the router
			}
		}
	}

	// If get here, we need to hold the packet

	npdb->idb->ifdisp->sethwaddr(npdb->idb, npb, rtt->hwaddr.c);
	npb->sendnext = NULL;				// Link this packet to the tail of the
	if (rtt->holdtail == NULL)			//   packet hold list for the RTT.
		rtt->holdhead = npb;
	else
		rtt->holdtail->sendnext = npb;
	rtt->holdtail = npb;
	return (GHWA_NOSEND);				// Nothing to send now
}


//*****************************************
// Function: Rrtredirect - Redirect a route
// ReturnedL:
//*****************************************

// This function converts an RTT_FOREIGN, RTT_FPENDING, or RTT_DPENDING entry
//   to an RTT_FOREIGN or RTT_FPENDING entry linked to a different RTT_LOCAL
//   or RTT_LPENDING entry.

long XOSFNC rtredirect(
	NPDB *npdb,				// Address of IP NPDB
	TPDB *tbpd,				// Address of ICMP TPDB
	long  fipaddr,			// Foreign IP address to redirect
	long  ripaddr)			// New router IP address (must be local)
{

#if 0

	ENTER	rtr_SIZE, 0
	MOVL	rtr_ipaddr[EBP], EAX	;Store IP address
	MOVL	rtr_rtipaddr[EBP], ECX	;Store new router address
	MOVL	EAX, ECX
	CLRL	EBX			;Indicate no packet
	CLRL	ECX
	CALL	geta2			;Get hardware address for the new router
	JC	16$.S			;If error
	TESTL	EBX, EBX		;Do we need to send a packet now?
	JE	2$.S			;No
	PUSHL	ECX			;Yes
	MOVL	npb_outdone[EBX], #ret002
	CALL	xossnpSendPktHw##	;Send the packet
	POPL	ECX
2$:	MOVB	AL, rt_type[ECX]
	CMPB	AL, #RTT_LPENDING 	;Is it a local address?
	JE	8$.S
	CMPB	AL, #RTT_ROUTER
	JE	8$.S
	CMPB	AL, #RTT_LOCAL
	JNE	16$.S			;No - forget it!
	MOVB	rt_type[ECX], #RTT_ROUTER ;OK
8$:	INCL	tpdb_icmpcntredirect[ESI] ;Count the redirected connection
	MOVL	rtr_local[EBP], ECX	;Save offset of local entry
	MOVL	EAX, rtr_ipaddr[EBP]	;Find entry for address to redirect
	CALL	rtsearch
	JE	10$.S			;If found it
	CLRL	EDX			;Note found - create one
	CLRL	ECX			;Don't worry about the hardware address
	MOVB	BL, #RTT_TEMP		;  since we don't really need it here!
	MOVB	BH, #1
	CALL	rtupdate
	JC	16$.S
10$:	CMPL	rt_router[ECX], #0	;Is it linked to a router now?
	JE	12$.S			;No
	CALL	rtunlkf			;Yes - unlink it
12$:	MOVL	EDX, rtr_local[EBP]	;Get offset of entry for router
	MOVB	AL, #RTT_FOREIGN	;Assume we have the router address now
	CMPB	rt_type[EDX], #RTT_ROUTER ;Right?
	JE	14$.S			;Yes
	MOVB	AL, #RTT_FPENDING	;No
14$:	MOVB	rt_type[ECX], AL
	MOVL	rt_router[ECX], EDX
	MOVL	EAX, rt_rtnhead[EDX]
	MOVL	rt_rtnnext[ECX], EAX
	MOVL	rt_rtnhead[EDX], ECX
	CLRL	EAX
	MOVL	rt_rtnprev[ECX], EAX
16$:	LEAVE				;Finished
	RET

#endif

	return (ER_NIYT);
}


//************************************************
// Function: routeupdate - Update IP routing table
// Returned: Nothing
//************************************************

// This function is called when and ARP request or response is received.
//   If a pending entry for the IP address is found it is changed to be
//   non-pending. This includes sending any held packets. If a pending
//   entry is not found, a new non-pending entry is created for the IP
//   address.

// The force argument is FALSE for broadcase ARP packets where we were not
//   the IP target. This prevents removing an old entry from the cache for
//   a new entry we probably do not care about.

void XOSFNC routeupdate(
	IPNPDB *npdb,
	long    ipaddr,
	llong   hwaddr,			// New hardware address
	long    force)			// TRUE to force update and update timestamp
{
	RTT  *rtt;
	RTT  *rttp;
	RTT  *lnked;
	RTT **pprev;
	int   inx;

	if ((inx = rtsearch(npdb, ipaddr, &rttp, &pprev)) < 0)
	{
		// Here if entry was not found - If this is for a foreign IP address
		//   we do nothing here. This can only happen on an incoming initial
		//   message from a remote IP address and should usually come from
		//   our main router. In this case, creating an RTT here gains nothing
		//   and would duplicates a significant amount of code from
		//   routegetaddr. Regardless of what router it came from, the normal
		//   address resolution mechanism should work when we try to send a
		//   response.  We will create an entry for it at that time linked to
		//   the default router which should redirect if necessary.

		if ((!force && npdb->iprtinuse >= npdb->iprtmax) ||
				(ipaddr & npdb->ipsubnetmask) != npdb->ipnetaddr)
			return;						// Forget it if not forcing and the
										//   routing table is full or if it's
										//   a foreign IP address
		if (makertt(npdb, inx, ipaddr, &rttp) < 0)
			return;
		rtt = rttp;
		if (npdb->iprtraddr1 == ipaddr) // See if this is from one of our
		{								//   known routers and remember the
			npdb->iprtrrtt1 = rtt;		//   address of its RTT if so
			rtt->type = RTT_ROUTER;
		}
		else if (npdb->iprtraddr2 == ipaddr)
		{
			npdb->iprtrrtt2 = rtt;
			rtt->type = RTT_ROUTER;
		}
		else
			rtt->type = RTT_LOCAL;
		rtt->hwaddr.ll = hwaddr;
		npdb->iprtinuse++;
		if ((rtt->prev = npdb->iprttail) != NULL) // Put it in the local list
			npdb->iprttail->next = rtt;
		else
			npdb->iprthead = rtt;
		npdb->iprttail = rtt;
	}
	else
	{
		// Here if entry was found - If it is a pending entry make it
		//   non-pending. In all cases store the "new" hardware address
		//   in the entry.

		rtt = rttp;
		if (rtt->type & RTT_PENDING)	// Is this a pending entry?
		{								// Yes - make it non-pending
			unpendrtt(npdb, rtt, hwaddr);
			if ((lnked = rtt->rtrhead) != NULL) // Have any linked entries?
			{
				if (rtt->type != RTT_ROUTER) // Yes - this must be a router!
					CRASH('RTE3');
				do
				{
					if (lnked->router != rtt) // Is it linked back to the
						CRASH('RTE4');		  //   router?
					unpendrtt(npdb, lnked, hwaddr); // Make the entry
					lnked = lnked->next;			//   non-pending
				} while (lnked != NULL);
			}
		}
		else
			rtt->hwaddr.ll = hwaddr;
		if (force)					// Update the timestamp if should
			rtt->used = knlSchCounts;
		rtt->t2lcnt = RTT2L_LOCAL;

	}
}


//************************************************************
// Function: makertt - Make a new routing table entry
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

// The caller must have determined (by calling rtsearch) that the entry
//   does not exist.

static long XOSFNC makertt(
	IPNPDB *npdb,
	long    inx,
	long    ipaddr,
	RTT   **prtt)
{
	RTT *rtt;
	RTT *frtt;
	RTT *rttp;
	RTT *oldrtt;
	long oldest;
	long used;
	long rtn;

	if (npdb->iprtinuse >= npdb->iprtmax) // Have room for another entry?
	{
		// Here if the routing table is full - Remove the least recently
		//   accessed entry.
		// NOTE: This is a circuliar compare that counts on the maximum
		//       time-to-live being signficantly less (no more that 90% or
		//       so) than a day. Normally, it will be well under an hour.

		rtt = npdb->iprthead;
		oldrtt = NULL;
		oldest = 0xFFFFFFFF;
		while (rtt != NULL)
		{
			if ((rtt->type & RTT_PENDING) == 0)
			{
				if ((used = rtt->used) > knlSchCounts) // Was it used yesterday?
					used -= SPPERDAY;	// Yes - fix up the value
				if (oldest > used)
				{
					oldest = used;
					oldrtt = rtt;
				}

				frtt = rtt->rtrhead;
				while (frtt != NULL)
				{
					if ((used = frtt->used) > knlSchCounts) // Used yesterday?
						used -= SPPERDAY; // Yes - fix up the value
					if (oldest > used)
					{
						oldest = used;
						oldrtt = frtt;
					}
					frtt = frtt->next;
				}
			}
			rtt = rtt->next;
		}
		if (oldrtt != NULL)				// Did we find one we can remove?
			routeremove(0, npdb, oldrtt); // Yes - remove it
	}

	// Create and insert the new routing entry

	if ((rtn = sysMemGetXmb(-XMBX_64, (char **)&rttp)) < 0)
		return (rtn);
	rtt = rttp;
	rtt->label = 'RTT*';
	rtt->ipaddr.l = ipaddr;
	rtt->t2lcnt = RTT2L_INITIAL;
	rtt->used = knlSchCounts;
	rtt->hashinx = inx;
	inx &= 0xFF;
	rtt->hashnext = npdb->iprthash[inx];
	npdb->iprthash[inx] = rtt;
	rtt->next = NULL;
	rtt->rtrhead = NULL;
	rtt->rtrtail = NULL;
	*prtt = rtt;
	return (0);
}


//**********************************************
// Function: unpendrtt - Make an RTT non-pending
// Returned: Nothing
//**********************************************

static void XOSFNC unpendrtt(
	IPNPDB *npdb,
	RTT    *rtt,
	llong   hwaddr)
{
	IDB *idb;
	NPB *npb;
	NPB *next;

	idb = npdb->idb;
	npb = rtt->holdhead;
	rtt->holdhead = rtt->holdtail = NULL;
	while (npb != NULL)
	{
		next = npb->sendnext;
		npb->sendnext = (NPB *)-1;
		idb->ifdisp->sethwaddr(idb, npb, (char *)&hwaddr);
		xosnetSendPkt(idb, npb);
		npb = next;
	}
	rtt->hwaddr.ll = hwaddr;
	rtt->type &= ~RTT_PENDING;
}


//*******************************************************
// Function: routermvip - Remove RTT given its IP address
// Returned:
//*******************************************************

long XOSFNC routermvip(
	IPNPDB *npdb,
	long    ipaddr)
{
#if 0

	REQUIREFORK
	TESTL	EAX, EAX		;Want to clear the routing table?
	JNE	xosipRtRmvOne	;No
	CMPL	sdb_label[ESI], #'SDB*'	;Yes - make sure have an SDB
	JNE	14$
	PUSHL	#0			;Initialize number to return

;;;;;	MOVL	EBX, pdb_ipndb[EDI]

	MOVL	ECX, pdb_iprthead[EDI]
	JREGZ	ECX, 8$
2$:	CMPB	rt_type[ECX], #RTT_PERM ;Should we remove this one?
	JB	4$			;Yes
	CMPB	rt_type[ECX], #RTT_PING ;Maybe
	JAE	12$			;No
4$:	MOVL	EAX, rt_ipaddr[ECX]	;Yes - Get IP address
	PUSHL	ECX
	CALL	xosipRtRmvOne		;Try to remove the entry
	JC	10$			;If error
	POPL	EDX			;OK - discard saved entry offset
	ADDL	[ESP], EAX		;Count it
6$:	TESTL	ECX, ECX		;More to do?
	JNE	2$			;Yes - continue
8$:	POPL	EAX			;No - get number removed
	RET				;Finished

;Here if have entry which cannot be removed here

10$:	POPL	ECX
12$:	MOVL	ECX, rt_next[ECX]	;Advance to next entry
	JMP	6$.S			;Continue

14$:	CRASH	NSDB			;[Not an SDB]

#endif

	return (0);
}


//***************************************************
// Function: routeremove - Remove routing table entry
// Returned: Nothing
//***************************************************

// This will also remove linked foreign entires when a router entry removed.
// Any held packets are discarded with the error code specified.

static void XOSFNC routeremove(
	long    code,
	IPNPDB *npdb,
	RTT    *rtt)
{
	NPB  *npb;
	NPB  *nextnpb;
	RTT  *rrtt;
	RTT  *nextrtt;
	RTT **hprev;

	REQUIREFORK;

	// Get rid of any held packets

	if ((rtt->type & RTT_PENDING) && (npb = rtt->holdhead) != NULL)
	{
		rtt->holdhead = rtt->holdtail = NULL;
		do
		{
			nextnpb = npb->sendnext;
			xosnetFinXmit(ER_NSNOD, npdb->idb, npb);
		} while ((npb = nextnpb) != NULL);
	}

	if ((rtt->type & 0x7F) == RTT_FOREIGN) // Foreign entry?
	{
		rrtt = rtt->router;				// Yes - remove it from the router's
		if ((rtt->prev) != NULL)		//   entry list
			rtt->prev->next = rtt->next;
		else
			rrtt->rtrhead = rtt->next;
		if ((rtt->next) != NULL)
			rtt->next->prev = rtt->prev;
		else
			rrtt->rtrtail = rtt->prev;
	}
	else								// Local entry
	{
		while (rtt->rtrhead != NULL)	// Get rid of any linked foreign entries
										//   (rtrhead is always NULL for non-
										//   router local entries)
			routeremove(code, npdb, rtt->rtrhead);
		if ((rtt->prev) != NULL)		 // Remove the entry from the local
			rtt->prev->next = rtt->next; //   entry list
		else
			npdb->iprthead = rtt->next;
		if ((rtt->next) != NULL)
			rtt->next->prev = rtt->prev;
		else
			npdb->iprttail = rtt->prev;
	}
	
	// Finally, remove the entry from its hash list and give it up

	hprev = &npdb->iprthash[(int)rtt->hashinx];
	while ((nextrtt = *hprev) != NULL)
	{
		if (nextrtt == rtt)
		{
			*hprev = rtt->hashnext;
			sysMemGiveXmb(rtt);
			return;
		}
		hprev = &nextrtt->hashnext;
	}
	CRASH('BRHL');						// [Bad Routing Hash List]
}


//*********************************************************
// Function: rtunlkf - Unlink a foreign routing table entry
// Returned:
//*********************************************************

long XOSFNC rtunlkf(
	RTT *rtt)
{

#if 0

	MOVL	EBX, rt_router[ECX]
	MOVL	EAX, rt_rtnprev[ECX]
	MOVL	EDX, rt_rtnnext[ECX]
	TESTL	EAX, EAX		;Have previous entry?
	JE	10$.S			;No
	MOVL	rt_rtnnext[EAX], EDX	;Yes
	JMP	12$.S

10$:	MOVL	rt_rtnhead[EBX], EDX
12$:	TESTL	EDX, EDX		;Have next entry?
	JE	14$.S
	MOVL	rt_rtnprev[EDX], EAX
	JMP	16$.S

14$:	MOVL	rt_rtntail[EBX], EAX
16$:	CLRL	EAX
	MOVL	rt_router[ECX], EAX
	RET

#endif

	return (ER_NIYT);
}


//****************************************************
// Function: rtsearch - Search for routing table entry
// Returned: Hash index (bit 31 set if not found)
//****************************************************

static long XOSFNC rtsearch(
	IPNPDB *npdb,
	long    ipaddr,
	RTT   **prtt,
	RTT  ***ppprev)
{
	RTT **pprev;
	RTT  *rtt;
	int   inx;

	inx = ((char *)&ipaddr)[0] ^ ((char *)&ipaddr)[1] ^ ((char *)&ipaddr)[2] ^
			((char *)&ipaddr)[3];
	pprev = &npdb->iprthash[inx];
	while ((rtt = *pprev) != NULL)
	{
		if (ipaddr == rtt->ipaddr.l)
		{
			*prtt = rtt;
			if (ppprev != NULL)
				*ppprev = pprev;
			return (inx);
		}
		pprev = &rtt->hashnext;
	}
	return (inx | 0x80000000);
}


//************************************************
// Function: rtunlink - Unlink routing table entry
// Returned:
//************************************************

// Does NOT give up the memory block

long XOSFNC rtunlink(
	NPDB *npdb,
	RTT  *rtt)
{

#if 0

	MOVL	EAX, rt_ipaddr[ECX]	;Calculate its hash index
	MOVZBL	EDX, AL
	XORB	DL, AH
	SHRL	EAX, #16t
	XORB	DL, AL
	XORB	DL, AH
	LEAL	EBX, pdb_iprthash[EDI+EDX*4]
4$:	CMPL	[EBX], ECX		;Find previous block
	JE	rtunlnk2.S		;OK - go unlink everything
	MOVL	EBX, [EBX]		;Advance to next
	TESTL	EBX, EBX
	JNE	4$
	CRASH	BRTH			;[Bad Routing Table Hash list]

	.SBTTL	rtunlnk2 - Subroutine to unlink block from routing table lists

;Subroutine to unlink block from the routing table lists
;	c{ECX} = Offset of block
;	c{EBX} = Offset of previous block in hash list (points to hash table
;		   entry if this is the first block in the hash list)
;	c{EDI} = Offset of PDB
;	CALL	rtunlnk2

rtunlnk2:
	MOVL	EDX, [ECX]		;Remove block from its hash list
	MOVL	[EBX], EDX
	MOVL	EBX, rt_next[ECX]	;Remove block from the routing table
	MOVL	EAX, rt_prev[ECX]	;  list
	TESTL	EAX, EAX
	JE	8$
	MOVL	rt_next[EAX], EBX
	JMP	10$

8$:	MOVL	pdb_iprthead[EDI], EBX
10$:	TESTL	EBX, EBX
	JE	12$
	MOVL	rt_prev[EBX], EAX
12$:	RET

#endif

	return (ER_NIYT);
}


//*********************************************************************
// Function: getrtdata - Get value for the ROUTES device characteristic
// Returned: Number of bytes stored (positive) if normal or a negative
//           XOS error code if error
//*********************************************************************

// This characteristic provides the contents of the routing table as
//   structured binary data.

// This table is make up of a link list of XMBs which may be modified at
//   device fork level. We don't really want to stay at fork level for this
//   entire routine but have to be protected against having the block we
//   are using pulled out from under us and corrupted before we use it,
//   especially before we use the next pointer! We handle this by going
//   to fork level when copying data, including the next pointer from a
//   block and going back to main program level to copy the data to the
//   user. Each time we go to fork level we check the routing cache sequence
//   number. This is incremented each time an items is deleted. We really
//   don't care about items being inserted since the fork level code makes
//   sure the forwark links are always valid. If an item has been deleted
//   (which isn't done very often) we start over. If this happens 3 times
//   we give up and return an ER_BUSY error.

long XOSFNC getrtdata(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	IPNPDB *npdb;
	RTT    *rtt;
	RTT    *rrtt;			// Saved router RRT when doing foreign RTTs
	char   *upnt;
	long    amnt;
	long    ucnt;
	long    rtn;
	long    seqnum;
	IPROUTE bufr;
	int     retry;

	*(long *)&bufr.type = 0;			// Clear the unused bytes
	retry = 3;
	npdb = (IPNPDB *)((IPDCB *)knlTda.dcb)->npdb;
	do
	{
		upnt = val;
		ucnt = cnt;
		amnt = 0;
		rrtt = NULL;
		seqnum = npdb->iprtseqnum;
		if ((rtt = npdb->iprthead) == NULL)
			return (0);
		while (TRUE)
		{
			if ((ucnt -= sizeof(IPROUTE)) < 0)
				return (amnt);
			TOFORK;
			if (seqnum != npdb->iprtseqnum)
			{
				FROMFORK;
				break;
			}
			bufr.type = rtt->type;
			bufr.ipaddr.l = rtt->ipaddr.l;
			bufr.t2l = rtt->t2lcnt;
			bufr.used = rtt->used;
			bufr.hwaddr.ll = (bufr.type == RTT_FOREIGN) ?
					rtt->router->ipaddr.l : rtt->hwaddr.ll;

			if (rrtt == NULL && rtt->rtrhead != NULL)
			{
				rrtt = rtt;
				rtt = rtt->rtrhead;
			}
			else
				rtt = rtt->next;
			FROMFORK;
			if ((rtn = sysLibMemCpy(upnt, (char *)&bufr, sizeof(IPROUTE))) < 0)
				return (rtn);
			amnt += sizeof(IPROUTE);

			if (rtt == NULL && rrtt != NULL)
			{
				rtt = rrtt->next;
				rrtt = NULL;
			}
			if (rtt == NULL)
				return (amnt);
			upnt += sizeof(IPROUTE);
		}

		// Here if the routing table was changed - we need to start over!

	} while (--retry > 0);
	return (ER_BUSY);					// Too may restarts - fail
}


//********************************************************
// Function: routeoas - Once-a-second function for routing
// Returned: Nothing
//********************************************************

void XOSFNC routeoas(void)
{
	IPNPDB *npdb;
	RTT    *rtt;
	RTT    *nextrtt;
	RTT    *frtt;
	RTT    *nextfrtt;

	npdb = (IPNPDB *)ipfirstnpdb;
	while (npdb != NULL)
	{
		rtt = npdb->iprthead;		// Get first RTT for the NPDB
		while (rtt != NULL)
		{
			nextrtt = rtt->next;
			if (rtt->type & RTT_PENDING) // Is this entry pending?
			{
				if (--(rtt->retrytmr) <= 0) // Yes - time to retry it?
				{
					if (--(rtt->retrycnt) <= 0) // Yes - done the enought?
						routeremove(ER_NSNOD, npdb, rtt); // Yes - get rid of it
					else
					{
						rtt->retrytmr = 3;
						xosarpSendPkt(&npdb->iparpdb, 0x01, NULL, // No - try
								rtt->ipaddr.c);					  //   again
					}
				}
			}
			else						// If not a pending entry
			{
				if (rtt->t2lcnt > 0 && --(rtt->t2lcnt) <= 0) // Time to get rid
															 //   of it?
					routeremove(0, npdb, rtt); // Yes
				else if ((frtt = rtt->rtrhead) != NULL) // No - have any linked
				{										//   foreign entries:
					do					// Yes - time them also (There are no
					{					//   ARP retries for foreign entries.)
						nextfrtt = frtt->next;
						if (frtt->t2lcnt > 0 && --(frtt->t2lcnt) <= 0)
							routeremove(0, npdb, frtt);
					} while ((frtt = nextfrtt) != NULL);
				}
			}
			rtt = nextrtt;
		}
		npdb = (IPNPDB *)npdb->nextp;
	}
}
