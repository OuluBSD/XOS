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
#include <STRING.H>
#include <XCSTRING.H>
#include <PROCARG.H>
#include <XOS.H>
#include <XOSSVC.H>
#include <XOSERR.H>
#include <XOSRTN.H>
#include <MKBOOT.H>

// Bootstrap versions before v8.0 store the saved boot block in the XOS.BSF
//   file in the root directory of the disk. Versions 8.0 and later store
//   the saved boot block in a reserved block following the boot block. This
//   version of mkboot will remove all versions of the bootstrap although
//   when removing versions before ???, the old boot block may not be restored
//   correctly.

static struct
{   TEXT8CHAR class;
    TEXT8CHAR unittype;
    TEXT8CHAR fstype;
    char      end;
} diskchar1 =
{   {(PAR_GET|REP_TEXT), 8, "CLASS"},
    {(PAR_GET|REP_TEXT), 8, "UNITTYPE"},
    {(PAR_GET|REP_TEXT), 8, "FSTYPE"},
};
static struct
{	BYTE1PARM attr;
	char      end;
} attrparm =
{	{PAR_SET|REP_HEXV, 1, IOPAR_FILATTR, XA_FILE}
};

static long memdev;				// Device handle for output (MEM) file
static long diskdev;			// Device handle for physical disk device
static char bootbfr[512];		// Buffer for disk boot block
static char memname[32];		// File specification for output file
static char bootstr[] ="? Boot error - press any key\x0D\x0A\0\x55\xAA";


//********************************************************
// Function: rmbootf - Remove the XOS bootstrap for a disk
// Returned: 0 if normal, 1 if error
//********************************************************

int rmbootf(
    char    diskname[],
    char   *disktype,
    mkbfail fail)

{
    long rtn;

    strmov(strmov(memname, diskname), "\\XOS.BSF");

    if ((diskdev = svcIoOpen(XO_IN|XO_OUT|XO_PHYS, diskname, NULL)) < 0)
    {
        fail("Error opening physical disk", diskdev, diskname);
        return (FALSE);
    }
    if ((rtn = svcIoDevChar(diskdev, &diskchar1)) < 0)
    {
        fail("Error getting disk characteristics", rtn, diskname);
        return (FALSE);
    }
    if (strcmp(diskchar1.class.value, "DISK") != 0)
    {
        char bufr[80];

        sprintf(bufr, "Device %s is not a disk\n", diskname);
        fail(bufr, 0, NULL);
        exit(FALSE);
    }
    *disktype = (stricmp(diskchar1.unittype.value, "HARD") == 0)? 0x80: 0;

    if ((rtn = svcIoSetPos(diskdev, 0, 0)) < 0)
    {									// Position to read boot block
        fail("Error setting position to read disk", rtn, diskname); // If error
        return (FALSE);
    }
    if ((rtn = svcIoInBlock(diskdev, bootbfr, 512)) < 0) // Read it
    {
        fail("Error reading disk",  rtn, diskname); // If error
        return (FALSE);
    }
    if (strncmp(bootstr, bootbfr + 0x1DF, 46) != 0) // Has the boot block
													//   been modified?
    {									// No - fail!
        fail("XOS Bootstrap not installed on disk - nothing to remove", 0,
                NULL);
        return (FALSE);
    }									// Yes - read the original boot block
										//   from where it was saved in
										//   the current XOS.BSF
    if ((memdev = svcIoOpen(XO_IN, memname, NULL)) < 0)
    {
        fail("Can not read XOS.BSF so can not restore original bootstrap",
                memdev, NULL);
        return (FALSE);
    }
    if ((rtn = svcIoSetPos(memdev, ((strcmp(diskchar1.fstype.value,
			"DOS32") == 0 && bootbfr[0x1DD] < 7) ||
			strcmp(diskchar1.fstype.value, "XFS") == 0) ? 512 : 1024, 0)) < 0)
    {									// Position to read saved boot block
        fail("Error setting position to read boot block", rtn, memname);
        return (FALSE);
    }
    if ((rtn = svcIoInBlock(memdev, bootbfr, 512)) < 0)
    {									// Read it
        fail("Error reading boot block", rtn, memname); // If error
        return (FALSE);
    }
    if (*(ushort *)(bootbfr+0x1FE) != 0xAA55)
    {
        fail("DOS bootstrap not saved in XOS.BSF so can not restore it",
                0, NULL);
        return (FALSE);
    }
    if ((rtn = svcIoSetPos(diskdev, 0, 0)) < 0)
    {					// Position to write boot block
        fail("Error setting position to write boot block", rtn, diskname);
        return (FALSE);
    }
    if ((rtn = svcIoOutBlock(diskdev, bootbfr, 512)) < 0)
    {					// Write the boot block
        fail("Error writing boot block", rtn, diskname); // If error
        return (FALSE);
    }
	if ((rtn = svcIoDevParm(0, memname, &attrparm)) < 0)
	{
		fail("Error clearing attributes for XOS.BSF", rtn, NULL);
		return (FALSE);
	}
    svcIoClose(memdev, 0);
	if ((rtn = svcIoDelete(0, memname, NULL)) < 0)
	{
		fail("Error deleting XOS.BSF", rtn, NULL);
		return (FALSE);
	}
    return (TRUE);
}
