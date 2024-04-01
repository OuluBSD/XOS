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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <xcstring.h>
#include <malloc.h>
#include <xcmalloc.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrun.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <xosrtn.h>
///#include <xosxxfs.h>
#include "fmtfuncs.h"

// An XFS disk in initialized as follows:
//	Block  0:   Boot block
//	Blocks 1-6: Reserved bootstrap blocks
//	Block  7:   Home block
//	Block  8:   First allocated cluster (cluster 1)
//	Cluster 1:  Root directory
//   The root directory will occupy one or two clusters since it will require
//   two blocks.  If the cluster size is 1, two clusters are needed, otherwise
//   only one is needed.
//   SAT clusters are allocated uniformally across the disk.  If only a single
//   SAT cluster is needed, it is placed at C/4, where C is the number of
//   clusters on the disk.  If N SAT clusters are required, each SAT cluster
//   is placed at (1+2n)(C/2N) where n is the number of the SAT cluster (between
//   0 and N-1).

// This program cannot produce a bootable disk directly.  A bootable disk can
//   be produced by running the MKBOOT utility after a disk is formatted.

extern char xfsbblk[];
extern char XFSBBLKSZ[];

static long     numsats;
static long     left;
///static long     begin;
static long     size;
///static long     homclus;
static long     clusnum;
static long     numsatblk;
static long     numsatclus;
static long     satsize;

static long     clusters;

static SATBLK **satbfrpnt;
static SATBLK **satbfrlist;

static ulong   *satcluspnt;
static ulong   *satcluslist;
static time_s   curdt;
static int      pntsize;

///static uchar   volcode[32];

static union
{   SATBLK  sat;
    HOMBLK  hom;
    FIBBLK  fib;
    DIRBLK  dir;
    BOOTBLK boot;
} buffer;

static struct
{   BYTE4PARM block;
    char      end;
} outparms =
{  {PAR_SET|REP_DECV, 4, IOPAR_ABSPOS, 0}
};



static void mark(long cluster);


int fmtxfs(
	char *diskname,
	int   fstypearg,
	int   clussize,
	int   reserved,
	int (*fmtready)(char *text))
{
    ulong  *pntpnt;
    SATBLK *satbfr;
    long    rtn;
    ulong   blk;
    int     cnt;
///	char    chr;

	reserved = reserved;
	fmtready = fmtready;

	fstype = fstypearg;

    if (clussize == 0)
        clussize = (blocks < 2000)? 1: (blocks < 20000)? 2:
                (blocks < 80000)? 4: 8;

    clusters = blocks/clussize;

    satsize = clussize * 4000;		// Number of bits in a SAT cluster

    numsatblk = (clusters + 3999)/4000;	// Number of SAT blocks
    numsatclus = (numsatblk + clussize - 1)/clussize; // Number of SAT clusters

    pntsize = (clusters < 0x10000) ? 2: (clusters < 0x1000000)? 3: 4;

/*	printf("%% FORMAT: Ready to %s disk %s (_%s)\n"
            "          Number of blocks     = %d\n"
            "          Cluster size         = %d\n"
            "          Number of clusters   = %d\n"
            "          Number of SAT blocks = %d\n"
            "          Pointer size         = %d\n"
            "          Volume name          = %.32s\n"
            "          Volume code          = %02.2X %02.2X %02.2X %02.2X"
            " %02.2X %02.2X %02.2X %02.2X\n"
            "                                 %02.2X %02.2X %02.2X %02.2X"
            " %02.2X %02.2X %02.2X %02.2X\n",
            (initonly)? "initialize": "format", diskname, physname, blocks,
            clussize, clusters, numsats, pntsize, vollabel, volcode[0],
            volcode[1], volcode[2], volcode[3], volcode[4], volcode[5],
            volcode[6], volcode[7], volcode[8], volcode[9], volcode[10],
            volcode[11], volcode[12], volcode[13], volcode[14], volcode[15]);
    if (confirm)
    {
        fputs("% FORMAT: Proceed? ", stdout);
        chr = getche();
        fputs("\n", stdout);
        if (toupper(chr) != 'Y')
        {
            fputs("% FORMAT: Terminated\n", stdout);
            exit(0);
        }
    }
*/
    //===================================
    // Construct the SAT blocks in memory
    //===================================

    satbfrpnt = satbfrlist = (SATBLK **)getspace(numsatclus * 4);
    satcluspnt = satcluslist = (ulong *)getspace(numsatblk * 4);
    left = clusters;
    clusnum = clusters/((numsatclus > 1) ? (2 * numsatclus) : 4);
    do
    {
        *satcluspnt++ =	clusnum;
		blk = clusnum * clussize + 8;
		cnt = clussize;
		do
		{
			size = (left > 4000)? 4000: left;
			left -= size;
			satbfr = (SATBLK *)getspace(512);
			memcpy(satbfr->sat_label, "SAT*", 4);
			satbfr->sat_selfl = blk;
			satbfr->sat_selfh = 0;
			memset(satbfr->sat_data, 0, 500);
			*satbfrpnt++ = satbfr;
		} while (left > 0 && --cnt > 0);
    } while (left > 0);

    //============================================
    // Mark all allocated blocks in the FAT blocks
    //============================================

    mark(1);							// Mark the first cluster for the root
    if (clussize == 1)					// Do we need a second cluster for the
										//   root?
		mark(2);						// Yes - mark it too
    cnt = numsatblk;					// Mark the SAT clusters
    satbfrpnt = satbfrlist;
    while (--cnt >= 0)
    {
		mark((*satbfrpnt)->sat_selfl/clussize);
		satbfrpnt++;
    }
    cnt = numsatblk * 4000 - clusters;	// Mark all clusters beyond the end of
    rtn = clusters;						//   the disk
    while (--cnt >= 0)
	mark(rtn++);

    //=========================
    // Write out the FAT blocks
    //=========================

    cnt = numsatblk;
    satbfrpnt = satbfrlist;
    while (--cnt >= 0)
    {
		satbfr = *satbfrpnt++;
		outparms.block.value = satbfr->sat_selfl * 512 + 4096;
		if ((rtn = svcIoOutBlockP(parthndl, (char *)satbfr, 512,
				(void *)&outparms)) < 0)
			femsg2(prgname, "Error writing SAT block", rtn, diskname);
    }

    //===================================================
    // Construct and write the FIB for the root directory    
    //===================================================

    memset(&buffer.fib, 0, 512);
    strmov(buffer.fib.fib_label, "FIB*");
    buffer.fib.fib_selfl = 8;
    buffer.fib.fib_selfh = 0;
    buffer.fib.fib_roof = 1;
    buffer.fib.fib_number = 1;
    buffer.fib.fib_base = 0;
    buffer.fib.fib_alloc = 1;
    buffer.fib.fib_attrib = XA_DIRECT;
    buffer.fib.fib_grpsize = groupsize;
    buffer.fib.fib_prot = 0;
    buffer.fib.fib_cdt = curdt;
    buffer.fib.fib_adt = curdt;
    buffer.fib.fib_mdt = curdt;
    strmov(buffer.fib.fib_owner, "ROOT");
    strmov(buffer.fib.fib_group, "SYSTEM");

    switch (pntsize)
    {
     case 2:
        buffer.fib.fib_f1pnt[1].p2.cnt = 1;
        buffer.fib.fib_f1pnt[1].p2.pnt = 1;
        break;
     case 3:
        buffer.fib.fib_f1pnt[1].p3.cnt = 1;
        buffer.fib.fib_f1pnt[1].p3.pntl = 1;
        buffer.fib.fib_f1pnt[1].p3.pnth = 0;
        break;
     case 4:
        buffer.fib.fib_f1pnt[1].p4.cnt = 1;
        buffer.fib.fib_f1pnt[1].p4.pnt = 0;
        break;
    }
    outparms.block.value = 8 * 512;
    if ((rtn = svcIoOutBlockP(parthndl, (char *)&buffer.fib, 512,
			(void *)&outparms)) < 0)
        femsg2(prgname, "Error writing root directory FIB", rtn, diskname);

    //============================================
    // Clear the data block for the root directory
    //============================================

    memset(&buffer.dir, 0, 512);
    outparms.block.value = 9 * 512;
    if ((rtn = svcIoOutBlockP(parthndl, (char *)&buffer.dir, 512,
            (void *)&outparms)) < 0)
	femsg2(prgname, "Error writing root directory block", rtn, diskname);

    //================================
    // Clear the boot extension blocks
    //================================

    cnt = 512;
    do
    {
	outparms.block.value = cnt;
	if ((rtn = svcIoOutBlockP(parthndl, (char *)&buffer.dir, 512,
			(void *)&outparms)) < 0)
	    femsg2(prgname, "Error writing root directory block", rtn,
		    diskname);
    } while ((cnt + 512) < 3584);

    //===================================
    // Construct and write the home block
    //===================================

    memset(&buffer.hom, 0, 512);
    strmov(buffer.hom.hom_label, "HOM*");
    buffer.hom.hom_selfl = 7;
    buffer.hom.hom_firstclus = 8;
    buffer.hom.hom_clussize = clussize;
    buffer.hom.hom_grpsize = groupsize;
    buffer.hom.hom_pntsize = pntsize;
    buffer.hom.hom_numsats = numsats;
    buffer.hom.hom_size = clusters;
    buffer.hom.hom_root = 8;
    pntpnt = buffer.hom.hom_sattbl;
    satcluspnt = satcluslist;
    cnt = numsats;
    do
    {
        *pntpnt++ = *satcluspnt++;
    } while (--cnt > 0);
    outparms.block.value = 7 * 512;
    if ((rtn = svcIoOutBlockP(parthndl, (char *)&buffer.hom, 512,
            (void *)&outparms)) < 0)
        femsg2(prgname, "Error writing home block", rtn, diskname);

    //===================================
    // Construct and write the boot block
    //===================================

    memcpy((char *)&buffer.boot, xfsbblk, (long)XFSBBLKSZ);
    memset(((char *)&buffer.boot) + (long)XFSBBLKSZ, 0, 510 - (long)XFSBBLKSZ);

///    buffer.boot.xb_heads = diskchars.numheads.value;
///    buffer.boot.xb_sects = diskchars.numsects.value;
///    buffer.boot.xb_clyns = diskchars.numcylns.value;

    buffer.boot.xb_btsig = 0xAA55;
    outparms.block.value = 0;
    if ((rtn = svcIoOutBlockP(parthndl, (char *)&buffer.boot, 512,
            (void *)&outparms)) < 0)
        femsg2(prgname, "Error writing boot block", rtn, diskname);
	return (0);
}

//****************************************
// Function: mark - Mark cluster as in-use
// Returned: Nothing
//****************************************

void mark(
    long clstr)

{
    int     bit;

    bit = clstr % 4000;
    (satbfrlist[clstr/4000])->sat_data[bit/8] |= (1 << (bit % 8));
}
