//--------------------------------------------------------------------------*
// btblkdmp.c
// Program to boot block
//
// Written by: John R. Goltz
//
//-------------------------------------------------------------------------*/

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
#include <string.h>
#include <ctype.h>
#include <xcmalloc.h>
#include <xos.h>
#include <xossvc.h>
#include <xosxffs.h>
#include <xoserr.h>
#include <xoserrmsg.H>
#include <xosstr.h>
#include <diskutilfuncs.h>


char prgname[] = "MBRDMP";		// Our programe name


int main(
	int   argc,
	char *argv[])
{   
	PTBL *pnt;
	int   num;
	long  hndl;
	long  rtn;
	MBR   mbr;

	// Open the parition

	if ((hndl = svcIoOpen(XO_IN|XO_PHYS, argv[1], NULL)) < 0)
		femsg2(prgname, "Error opening partition", hndl, NULL);

	if ((rtn = svcIoInBlock(hndl, (char *)&mbr, 512)) < 0)
		femsg2(prgname, "Error reading MBR block", rtn, NULL);

	printf("\nDisk ID: 0x%08X\n", mbr.diskid);
	fputs("\nSlot Boot ----First----- Prtn -----Last-----\n", stdout);
	fputs("num  flag Head Sect Cyln type Head Sect Cyln    First block "
			"Nmbr of blocks\n", stdout);
	pnt = mbr.ptbl;						// Point to first slot
	num = 1;
	do
	{
		if (*(llong *)pnt == 0 && *(llong *)&pnt->begin == 0)
			printf(" %2d    Empty\n", num);
		else
			printf(" %2d  0x%02X %4d %4d %4d 0x%02X %4d %4d %4d %,14d %,14d\n",
				num, pnt->boot, pnt->bgnhead, pnt->bgnsect & 0x3F,
				pnt->bgncyln + ((pnt->bgnsect & 0x0C0) << 2), pnt->type,
				pnt->endhead, pnt->endsect & 0x3F, pnt->endcyln +
				((pnt->endsect & 0xC0) << 2), pnt->begin, pnt->length);
		pnt++;
	} while (++num <= 4);

	printf("\nBlock end label: 0x%04X (%s)\n", mbr.endlbl,

			(mbr.endlbl == 0xAA55) ? "correct" : "should be 0xAA55");

	return (0);
}
