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
#include <CTYPE.H>
#include <STRING.H>
#include <XCSTRING.H>
#include <MALLOC.H>
#include <XCMALLOC.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSRUN.H>
#include <XOSERR.H>
#include <XOSERMSG.H>
#include <XOSRTN.H>
#include <XOSXXFS.H>
#include "FORMAT.H"
#include "FOREXT.H"

long   numsats;
long   left;
long   begin;
long   size;
long   rootclus;
long   homclus;
long   clusnum;
ulong *satcluspnt;
ulong *satcluslist;
time_s curdt;
int    pntsize;

uchar  volcode[32];

union
{   sat_blk  sat;
    hom_blk  hom;
    fib_blk  fib;
    dir_blk  dir;
    boot_blk boot;
} buffer;

struct
{   byte4_parm block;
    char       end;
} outparms =
{  {PAR_SET|REP_DECV, 4, IOPAR_ABSPOS, 0}
};

void mark(long cluster);

void formatxos(void)

{
    int    cnt;
    long   rtn;
    ulong *pntpnt;
    char   chr;

    if (clussize == 0)
        clussize = (blocks < 2000)? 1: (blocks < 20000)? 2:
                (blocks < 80000)? 4: 8;

    clusters = (blocks + clussize - 1)/clussize;
    numsats = (clusters + 3999)/4000;
    rootclus = (clusters > 4000)? 2001: clusters/2 + 1;
    pntsize = (clusters < 0x10000)? 2: (clusters < 0x1000000)? 3: 4;
    satcluslist = (ulong *)getspace(numsats*4);

    printf("%% FORMAT: Ready to %s disk %s (_%s)\n"
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

    //=====================
    // Write the SAT blocks
    //=====================

    satcluspnt = satcluslist;
    left = clusters;
    begin = 0;
    do
    {
        size = (left > 4000)? 4000: left;
        clusnum = begin + size/2;
        if (homclus == 0)
            homclus = clusnum + 1;
        buffer.sat.sat_magic = DK_MAGIC;
        strmov(buffer.sat.sat_label, "SAT*");
        buffer.sat.sat_self = clusnum * clussize;
        memset(buffer.sat.sat_data, 0, 500);
        mark(0);
        mark(1);
        mark(rootclus);
        mark(clusnum);

        *satcluspnt++ = clusnum;
        outparms.block.value = clusnum * clussize * 512;
        if ((rtn = svcIoOutBlockP(hndl, (char far *)&buffer.sat, 512,
                (void far *)&outparms)) < 0)
            femsg2(prgname, "Error writing SAT block", rtn, diskname);

        if ((cnt = clussize - 1) > 0)
        {
            memset(&buffer.sat, 0, 512);
            do
            {
                outparms.block.value += 512;
                if ((rtn = svcIoOutBlockP(hndl, (char far *)&buffer.sat,
                        512, (void far *)&outparms)) < 0)
                    femsg2(prgname, "Error writing SAT block", rtn, diskname);
            } while (--cnt > 0);
        }
        begin += size;
        left -= size;
    } while (left > 0);

    //=====================================
    // Write the FIB for the root directory    
    //=====================================

    memset(&buffer.fib, 0, 512);
    buffer.fib.fib_magic = DK_MAGIC;
    strmov(buffer.fib.fib_label, "FIB*");
    buffer.fib.fib_self = rootclus * clussize;
    buffer.fib.fib_roof = clusters;
    buffer.fib.fib_number = 1;
    buffer.fib.fib_base = rootclus;
    buffer.fib.fib_alloc = 1;
    buffer.fib.fib_attrib = A_DIRECT;
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
        buffer.fib.fib_f1pnt[1].p2.pnt = rootclus;
        break;
     case 3:
        buffer.fib.fib_f1pnt[1].p3.cnt = 1;
        buffer.fib.fib_f1pnt[1].p3.pntl = (ushort)rootclus;
        buffer.fib.fib_f1pnt[1].p3.pnth = (uchar)(rootclus>>16);
        break;
     case 4:
        buffer.fib.fib_f1pnt[1].p4.cnt = 1;
        buffer.fib.fib_f1pnt[1].p4.pnt = rootclus;
        break;
    }
    outparms.block.value = buffer.fib.fib_self * 512;
    if ((rtn = svcIoOutBlockP(hndl, (char far *)&buffer.fib, 512,
            (void far *)&outparms)) < 0)
        femsg2(prgname, "Error writing root directory FIB", rtn, diskname);

    //============================================
    // Write the data block for the root directory
    //============================================

    memset(&buffer.dir, 0, 512);
    buffer.dir.dir_magic = DK_MAGIC;
    strmov(buffer.dir.dir_label, "DIR*");
    buffer.dir.dir_self = rootclus + 1;
    outparms.block.value = buffer.dir.dir_self * 512;
    if ((rtn = svcIoOutBlockP(hndl, (char far *)&buffer.dir, 512,
            (void far *)&outparms)) < 0)
        femsg2(prgname, "Error writing root directory block", rtn, diskname);

    //=====================
    // Write the home block
    //=====================

    memset(&buffer.hom, 0, 512);
    buffer.hom.hom_magic = DK_MAGIC;
    strmov(buffer.hom.hom_label, "HOM*");
    buffer.hom.hom_self = clussize;
    buffer.hom.hom_heads = diskchars.numheads.value;
    buffer.hom.hom_sects = diskchars.numsects.value;
    buffer.hom.hom_clyns = diskchars.numcylns.value;
    buffer.hom.hom_clussize = clussize;
    buffer.hom.hom_grpsize = groupsize;
    buffer.hom.hom_pntsize = pntsize;
    buffer.hom.hom_numsats = numsats;
    buffer.hom.hom_size = clusters;
    buffer.hom.hom_root = rootclus;
    pntpnt = buffer.hom.hom_sattbl;
    satcluspnt = satcluslist;
    cnt = numsats;
    do
    {
        *pntpnt++ = *satcluspnt++;
    } while (--cnt > 0);
    outparms.block.value = buffer.hom.hom_self * 512;
    if ((rtn = svcIoOutBlockP(hndl, (char far *)&buffer.hom, 512,
            (void far *)&outparms)) < 0)
        femsg2(prgname, "Error writing home block", rtn, diskname);

    //=====================
    // Write the boot block
    //=====================

    memset(&buffer.hom, 0, 512);

    *(long *)buffer.boot.boot_jump = 0x903CEB;
    strmov(buffer.boot.boot_xossig, "*XOS*FS*");
    buffer.boot.boot_homeblk = clussize;
    buffer.boot.boot_bootsig = 0xAA55;
    outparms.block.value = 0;
    if ((rtn = svcIoOutBlockP(hndl, (char far *)&buffer.boot, 512,
            (void far *)&outparms)) < 0)
        femsg2(prgname, "Error writing boot block", rtn, diskname);
}

//****************************************
// Function: mark - Mark cluster as in-use
// Returned: Nothing
//****************************************

void mark(
    long cluster)

{
    long byteos;
    long bitnum;

    if ((cluster >= begin) && (cluster < (begin+size)))
    {
        byteos = (cluster-begin)/8;
        bitnum = (cluster-begin)%8;
        buffer.sat.sat_data[byteos] |= (1<<bitnum);
    }    
}
