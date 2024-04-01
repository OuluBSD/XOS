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
#include <xosx.h>
#include <xossvc.h>
#include <xosxdisk.h>
#include <xosxffs.h>
#include <xoserr.h>
#include <xoserrmsg.H>
#include <xosstr.h>
#include <global.h>
#include <progarg.h>


char    prgname[] = "BTBLKDMP";		// Our programe name


long  hndl;
union
{	BTBLK16 bb16;
	BTBLK32 bb32;
} btblk;

long sectors;
long clusters;
long secpfat;
long pntspfat;
int  fattype;


char *cleantext(char *pnt, int len);


int main(
	int   argc,
	char *argv[])
{   
	long rtn;

	// Open the parition

	if ((hndl = svcIoOpen(XO_IN|XO_PHYS, argv[1], NULL)) < 0)
		femsg2(prgname, "Error opening partition", hndl, NULL);

	if ((rtn = svcIoInBlock(hndl, (char *)&btblk, 512)) < 0)
		femsg2(prgname, "Error reading boot block", rtn, NULL);


	sectors = (btblk.bb16.sectors16 != 0) ? btblk.bb16.sectors16 :
				btblk.bb16.sectors32;
	secpfat = (btblk.bb16.secpfat16 != 0) ? btblk.bb16.secpfat16 :
			btblk.bb32.secpfat32;

	if (btblk.bb16.secpcls < 1 || btblk.bb16.secpcls > 128 ||
			(btblk.bb16.secpcls & (btblk.bb16.secpcls - 1)) != 0)
		printf("Cluster size value (%d) is invalid - cannot determine FAT "
				"type!\n", btblk.bb16.secpcls);
	else
	{
		clusters = (sectors - (secpfat * btblk.bb16.numfats) - 
				((btblk.bb16.rootsize + 15) / 32)) / btblk.bb16.secpcls;

		if (secpfat != 0 && btblk.bb16.secpcls != 0)
		{
			pntspfat = clusters  / secpfat;
			fattype = (pntspfat <= 128) ? 32 : (pntspfat <= 256) ? 16 :
					(pntspfat <= 341) ? 12 : 0;

///			printf("### pntspfat = %d fattype = %d\n", pntspfat, fattype);
		}
		else
			fputs("Warning: Sectors per FAT value is 0!\n", stdout);

		if (fattype == 0)
			fputs("\nSector count values appear to be invalid, cannot "
					"determine FAT type!\n", stdout);
		else
			printf("\nBased on the sector count values, this appears to be a "
					"%d-bit FAT boot block\n", fattype);
	}
	if (pntspfat != 0)
		printf("    Calculated pointers per FAT block = %d\n", pntspfat);

	// Items which are common to all FAT boot blocks

	fputs("\nItems which are common to all FAT boot blocks:\n", stdout);
	printf("  Jump instruction: 0x%02X 0x%02X 0x%02X\n", btblk.bb16.jump[0],
			btblk.bb16.jump[1], btblk.bb16.jump[0]);
	printf("          OEM name: \"%.8s\"\n", cleantext(btblk.bb16.oemname, 8));
	printf("       Sector size: %,d bytes\n", btblk.bb16.secsize);
	printf("      Cluster size: %,d sector%s\n", btblk.bb16.secpcls,
			(btblk.bb16.secpcls == 1) ? "" : "s");
	printf("  Reserved sectors: %,d sector%s\n", btblk.bb16.reserved,
			(btblk.bb16.reserved == 1) ? "" : "s");
	printf("    Number of FATs: %,d\n", btblk.bb16.numfats);
	printf("      Root entires: %,d\n", btblk.bb16.rootsize);
	printf("           Sectors: %,d (16-bit value)\n", btblk.bb16.sectors16);
	printf("  Media descriptor: 0x%02X\n", btblk.bb16.media);
	printf("   Sectors per FAT: %,d (16-bit value)\n", btblk.bb16.secpfat16);
	printf(" Sectors per track: %,d\n", btblk.bb16.secptrk);
	printf("   Number of heads: %,d\n", btblk.bb16.heads);
	printf("           Sectors: %,d (32-bit value)\n", btblk.bb16.sectors32);

	if (fattype != 32)
	{
		fputs("\nItems for 12 and 16-bit FAT boot blocks\n", stdout);
		printf("      Drive number: 0x%02X\n", btblk.bb16.drive);
		printf("     Reserved byte: 0x%02X\n", btblk.bb16.xxx);
		printf("    EBPB signature: 0x%02X (%s)\n", btblk.bb16.extsig,
				(btblk.bb16.extsig == 0x29) ? "correct" : "should be 0x29");
		printf("           Disk ID: 0x%08X\n", btblk.bb16.sernum);
		printf("      Volume label: \"%.11s\"\n", btblk.bb16.vollbl);
		printf("    File system ID: \"%.8s\"\n", cleantext(btblk.bb16.sysid,
				8));
	}
	if (fattype != 12 && fattype != 16)
	{
		fputs("\n Items for 32-bit FAT boot blocks\n", stdout);

		printf("   Sectors per FAT: %,d (32-bit value)\n",
				btblk.bb32.secpfat32);
		printf("       Extra flags: 0x%04X\n", btblk.bb32.extflags);
		printf("        FS version: 0x%04X\n", btblk.bb32.version);
		printf("      Root cluster: %,d\n", btblk.bb32.rootcls);
		printf("    FS info sector: %,d\n", btblk.bb32.fsinfo);
		printf("Backup boot sector: %,d\n", btblk.bb32.backboot);
		printf("    Reserved bytes: 0x%02X 0x%02X 0x%02X, 0x%02X 0x%02X "
			   "0x%02X\n                    0x%02X 0x%02X 0x%02X, 0x%02X "
				"0x%02X 0x%02X\n", btblk.bb32.rsrvd[0], btblk.bb32.rsrvd[1],
				btblk.bb32.rsrvd[2], btblk.bb32.rsrvd[3], btblk.bb32.rsrvd[4],
				btblk.bb32.rsrvd[5], btblk.bb32.rsrvd[6], btblk.bb32.rsrvd[7],
				btblk.bb32.rsrvd[8], btblk.bb32.rsrvd[9], btblk.bb32.rsrvd[10],
				btblk.bb32.rsrvd[22]);
		printf("      Drive number: 0x%02X\n", btblk.bb32.drive);
		printf("     Reserved byte: 0x%02X\n", btblk.bb32.xxx2);
		printf("    EBPB signature: 0x%02X (%s)\n", btblk.bb32.extsig,
				(btblk.bb32.extsig == 0x29) ? "correct" : "should be 0x29");
		printf("           Disk ID: 0x%08X\n", btblk.bb32.sernum);
		printf("      Volume label: \"%.11s\"\n", cleantext(btblk.bb32.vollbl,
				11));
		printf("    File system ID: \"%.8s\"\n", cleantext(btblk.bb32.sysid,
				8));
		printf("    XOS drive info: 0x%02X\n", btblk.bb32.xosdrive);
		printf("   First BS sector: %,d\n", btblk.bb32.lvl2blk);
		printf("           BS size: %,d\n", btblk.bb32.lvl2num);
	}
	fputs("\nFinal items used in all boot blocks:\n", stdout);
	printf("    XOS BS version: %d\n", btblk.bb32.bsversion);
	printf("XOS BS edit number: %d\n", btblk.bb32.bseditnum);
	printf("XOS error msg text: \"%.31s\"\n", cleantext(btblk.bb32.errmsg, 31));
	printf("   Block end label: 0x%04X (%s)\n", btblk.bb32.label,
			(btblk.bb32.label == 0xAA55) ? "correct" : "should be 0xAA55");
	return (0);
}


char *cleantext(
	char *txt,
	int   len)
{
	char *pnt;
	char  chr;

	static char text[64];

	pnt = text;
	while (--len >= 0)
		*pnt++ = ((chr = *txt++) != 0 && chr < 0x20 || chr > 0x7F) ? '.' : chr;
	*pnt = 0;
	return (text);
}
