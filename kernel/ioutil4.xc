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


#include <xos.h>
#include <xosx.h>
#include <xosxdisk.h>
#include <xoslib.h>
#include <xoserr.h>
#include <ctype.h>
#include <stddef.h>

//***************************************************************************
// Function: sysIoFindDevByName - Find device DCB given device name string
// Returned: 0 if found a match or a negative XOS error code if error. if 0
//			 is returned, the address of the dcb is stored in knlTda.Dcb.
//***************************************************************************

// If the rtndcb argument is FALSE only the existence of the device is checked.
//   0 is returned if the device exists. If the rtndcb argument is TRUE, the
//   check is for the availability of the device. If the device is available
//   and it has a static DCB that DCB is returned. If it is available and does
//   not have a static DCB, a dynamic DCB is created (by the device check
//   function for the device) and returned. In either case, it must eventually
//   be given up by calling the sd_cdcb device dependent function.

// If the rtndcb argument is TRUE, the address of the DCB found or created is
//   stored in knlTda.dcb. If it is FALSE, knlTda.dcb is not changed.

// The device check function for each device class is called as follows:
//	long devchkfunc(
//		char *name,			// Address of device name buffer
//		char *path,			// Address of patch specification buffer
//		long  rtndcb);		// TRUE if should return a DCB

// The value returned is 0 if no match, 1 if have a match, 2 if should restart
//   the search, or a negative error code if error. An error must only be
//   returned if it has been determined that there is a match on the name
//   but that the device can not be used for some reason. This function may
//   modify the device name and/or the path specification. When this is done,
//   the value returned should be 2. Currently this is only done by the SPLCLS
//   class driver. The number of calls to device check functions is limited
//   to 4 times the number of CCBs in the system  to prevent infinite loops
//   caused by restarts. Normally only 1 restart should ever be requested for
//   any device search.
// If 1 is returned, this function must store the address of the DCB in
//   tdaDcb. Otherwise, it must return 0 in tdaDcb. It may use tdaDcb but
//   must set it to 0 before turning other than 1. It is does not use tdaDcb
//   it does not need to 0 it.
// The error return must only be given if the name is known to match the device
//   and a DCB cannot be returned for some reason.

long XOSFNC sysIoFindDevByName(
	char  name[16],			// Device name (16 bytes, 0 filled)
	char *path,				// Path specification (buffer must be at
							//   least FILESPCSIZE + 1 bytes long)
	long  rtndcb)			// TRUE if should create DCB
{
	CCB *ccb;
	long count;
	long rtn;

	name[0] &= 0x7F;					// Make sure physical only bit is not
										//   set
	if (name[0] == 0)					// Device name cannot be null
		return (ER_NSDEV);
	count = 3;							// Maximum number of times to loop
										//   (We need to have a limit since some
										//   device check routines change the
										//   device name and restart the search
										//   - this could cause an infinite
										//   loop if done wrong!)
	while (--count >= 0)
	{
		ccb = knlFirstCcb;				// Point to first CCB
		while (ccb != NULL)
		{
			if ((*(long *)name & ccb->npfxmsk) == ccb->npfxval)
			{							// Can this one match?
				if ((rtn = ccb->devchk(name, path, rtndcb)) != 0)
				{						// Yes - does it match?
					if (rtn == 1)		// Do we have a match?
					{					// Yes - can we have it?
						if (rtndcb && knlTda.dcb->opencnt != 0 &&
								(knlTda.dcb->dsp & DS_MLTUSER) == 0 &&
								(knlPda.actpriv & PP_SHAREDEV) == 0 &&
								knlTda.dcb->sespda != knlPda.sespda)
							return (ER_DEVIU);
						if (knlTda.dcb->dsp & DS_SPOOL)	// ???????
							knlTda.dcb->sunit = knlTda.spoolunit;
						return (0);		// Give good return
					}
					if (rtn < 0)		// Error indicated?
						return (rtn);	// Yes - return it
					break;				// Must be rescan request so start over
				}
			}
			ccb = ccb->next;
		}
	}
	return (ER_NSDEV);
}


//************************************************************
// Function: sysIoStoreDevName - Give the user the device name
// Returned: Number of characters stored if normal or a
//           negative XOS error code if error
//************************************************************

// Global variables used: (all are updated here)
//	 char *ppfilspc - Buffer pointer
//   long  pcfilspc - Total number of bytes seen (may be larger than the
//                      number of bytes stored)
//   long  psfilspc - Number of bytes available in the buffer

// Even though this is a resident function, it knows enough about disk
//   devices to return the correct name.

long XOSFNC sysIoStoreDevName(void)
{
	char *name;
	long  rtn;
	int   len;
	int   amnt;

	if (knlTda.psfilespec < 1)
		return (ER_BFRTS);
	if ((knlTda.pvfileoptn & XFO_XOSDEV) == 0 && // Does he not want the XOS
			(knlTda.dcb->dsp & DS_LCLDISK) &&	 //   device name and is this a
			knlTda.adb != NULL)					 //   local disk?
	{
		if ((knlTda.pvfileoptn & XFO_DOSDEV) && // Does he want the DOS name and
				dskucb->dosname[0] != 0)		//   do we have one?
			name = dskucb->volname;				// Yes
		else if ((knlTda.pvfileoptn & XFO_VOLUME) && // Does he want the volume
				dskucb->volname[0] != 0)			 //   name & do we have one?
			name = dskucb->volname;					 // Yes
		else
			name = knlTda.dcb->name;
	}
	else
		name = knlTda.dcb->name;
	len = amnt = sysLibStrNLen(name, 16);
	if (amnt > (knlTda.psfilespec - 1))
		amnt = knlTda.psfilespec - 1;
	if ((rtn = sysLibMemCpy(knlTda.ppfilespec, name, amnt)) < 0)
		return (rtn);
	knlTda.ppfilespec += rtn;
	knlTda.psfilespec -= rtn;
	knlTda.pcfilespec += len;
	if (knlTda.psfilespec < 2)
	{
		if ((rtn = sysUSPutByte(knlTda.ppfilespec, 0)) < 0)
			return (rtn);
	}
	else
	{
		if ((rtn = sysUSPutShort((short *)knlTda.ppfilespec, ':')) < 0)
			return (rtn);
		knlTda.ppfilespec++;
		knlTda.psfilespec--;
		knlTda.pcfilespec++;
	}
	if ((rtn = sysUSAddShort((short *)knlTda.plfilespec,
			(short)knlTda.pcfilespec)) < 0)
		return (rtn);
	return (amnt + 1);
}
