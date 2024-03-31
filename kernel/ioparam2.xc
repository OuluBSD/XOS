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
#include <xoslib.h>
#include <xoserr.h>
#include <ctype.h>
#include <stddef.h>

// This file contains the new style routines to process device parameters.
//   The modivation in creating this is to provide a way to write device
//   routines which handle parameter in C. The old style scheme passed
//   arguments in registers, did not preserve registers and (worst) used
//   status bits located just before each function. (This CANNOT be done in
//   C!)

// These routines are exactly the same as the old ones from a user's viewpoint.
//   Any device routine currently can be written to use either the old or new
//   style routines. Once everything is converted, the old store routines will
//   go away.


//***********************************************************
// Function: sysIoProcDevParamNew - Process device parameters
// Returned: PDPF_ flag bits (positive) if normal or a
//		     negative XOS error code if error.
//***********************************************************

// This routine is called by each device driver call a device dependent
//   routine for each parameter in a parameter list.

// The address of the DCB is taken from knlTda.dcb. The address of the
//   parameter list is taken from knlTda.parm.

// Parameter tables are two level. The first level is a simple array of
//   pointers to second level tables. This array is index by the high order
//   byte of the parameter index. There are two first level tables for each
//   device. One handles parameters starting with 0x0000 (standard table) and
//   the other handles parameters starting with 0x8000 (device dependent
//   table). Each second level table is an array of PTBL structures.

// The first element in each table contains the length of the table. This is
//   followed immediately by an array as described above.

// There are two types of parameters: numeric and string. Numeric parameters
//   have an "immediate" value in the parameter list which may be between 1
//   and 32 bytes in length. String parameters use a seperate buffer with a
//   pointer and size value in the parameter list and may be up to 64KB in
//   length.

// Device routines for parameters are called as follows:

//	long XOSFNC parmfunc(
//		long    index,		// Parameter index + function - Bit 31 set to get
//							//   value, bit 30 set to set value.
//		char    value[],	// Value buffer
//		long    count,		// Maximum number of bytes to get or store
//		ushort *cpnt,		// Address of short value to receive size value
//		void   *data);		// Address of device driver's data area
//  Value returned must be the number of bytes stored (positive) if normal
//    or a negative XOS error code if error.

// The value address generally will point to user space and all accesses to
//   the value buffer MUST be protected against page faults.

// Note that the device routine does not know (or care) if the parameter is
//   numeric or string.

// The cpnt value is only used by the device if it will store a defered value
//   into a string parameter. In this case it will store the length of the
//   value stored into the buffer in the short pointed to by cpnt. This will
//   generaly be user space address.

// The valid functions and valid parameter kind (numeric or string) are
//   specified in the second level parameter table. Thus there is no need
//   for a device parameter function to check this. It will never be called
//   if an invalid function is specified or if the data buffer is too small.

// If a given parameter only supports one function (set or get) there is no
//   need to check the function bits. If both are supported, both bits must be
//   checked. A call with both set is valid. The set function is alwasy done
//   first, followed by the get function. Thus the get serves to verify the
//   value actually set by the set (which in some cases may not be exactly
//   what was specified). If the previous value is needed, a separate get
//   must be done before the set.

long XOSFNC sysIoProcDevParamNew(
	PARMTBL1 *spt,		// Address of standard parameter table
	PARMTBL1 *ddpt,		// Address of device dependent parameter table
	void     *data,		// Address of driver's data area
	long      flags)	// Initial flag bits:
						//   Bit 7: PDPF$CLASS - Have seen correct
						//                       IOPAR_CLASS value
						//   Bit 6: PDPF$IGNBP - Ignore illegal
						//                       parameter index values
{
	PARMTBL1 *table1;
	PARMTBL2 *table2;
	PARMITEM *pent;
	char     *param;			// Pointer to the parameter list
	char    *buffer;
	PINDEX   pindex;
	long     bfrlen;			// Length of parameter buffer
	long     strlen;			// Length of parameter string
	long     rtn;
	long     index;
	long     len;
	int      ptype;
	union
	{	long l;
		struct
		{	char   bits;
			char   len;
			ushort index;
		};
	} phead;

	if (knlTda.parm >= systembase)
		return (ER_ADRER);
	param = knlTda.parm;
	rtn = 0;
	while (TRUE)
	{
		if ((rtn = sysUSGetLong((long *)param, &phead.l)) < 0)
			return (rtn);
		if ((phead.bits & (PAR_GET|PAR_SET)) == 0)
			break;
		if ((rtn = sysUSAndByte(param, ~PAR_ERROR)) < 0)
			break;
		if (phead.index & 0x8000)			// Device dependent parameter
		{
			if ((phead.bits & PAR_SET) && phead.index == 0x8000)
										// "Setting" the device class parameter?
				flags |= PDPF_CLASS;	// Yes - remember this			
			else if ((flags & PDPF_CLASS) == 0)
			{
				rtn = ER_PARMI;
				break;
			}
			table1 = ddpt;
			phead.index &= 0x7FFF;
		}
		else
			table1 = spt;
		index = phead.index >> 8;
		if (index >= table1->size || table1->tbl[index] == NULL)
		{
			rtn = ER_PARMI;
			break;
		}
		table2 = table1->tbl[index];
		index = phead.index & 0xFF;

		if (index >= table2->size || (table2->tbl[index].setfunc == NULL &&
				table2->tbl[index].getfunc == NULL))
		{
			rtn = ER_PARMI;
			break;
		}
		pent = &table2->tbl[index];
		ptype = phead.bits & 0x0F;		// Get the parameter type
		if (((phead.bits & PAR_GET) && pent->getfunc == NULL) ||
				((phead.bits & PAR_SET) && pent->setfunc == NULL))
		{
			rtn = ER_PARMF;
			break;
		}
		*(long *)&pindex = (((long)phead.bits) << 24) + pent->length;
		if (ptype == REP_STR || ptype == REP_DATAS)
		{
			// Here for a string value

			if (pent->numeric != 0)
			{
				rtn = ER_PARMT;
				break;
			}
			if ((rtn = sysUSGetLong((long *)(param + 4),
					(long *)&buffer)) < 0 ||
					(rtn = sysUSGetLong((long *)(param + 8), &bfrlen)) < 0)
				break;
			if (buffer >= systembase)
			{
				rtn = ER_ADRER;
				break;
			}
			strlen = bfrlen >> 16;
			bfrlen &= 0xFFFF;
			if (pindex.set)
			{
				if (strlen == 0)
					strlen = bfrlen;
				if ((rtn = pent->setfunc(pindex, buffer, strlen, data)) < 0)
					break;
			}
			if (pindex.get)
			{
				if ((len = pent->getfunc(pindex, buffer, bfrlen,
						(ushort *)(param + 10), data)) < 0)
					break;
				if (len == 0 && (rtn = sysUSPutByte(buffer, 0)) < 0)
					break;
				if ((rtn = sysUSPutShort((short *)(param + 10), len)) < 0)
					break;
			}
			param += 12;
		}
		else
		{
			// Here for a numeric value

			if (pent->numeric == 0)
				return (ER_PARMT);
			if (pindex.set)
			{
				if ((rtn = pent->setfunc(pindex, param + 4, phead.len,
						data)) < 0)
					break;
			}
			if (pindex.get)
			{
				if ((rtn = pent->getfunc(pindex, param + 4, phead.len, NULL,
						data)) < 0)
					break;
			}
			param += (phead.len + 4);
		}
	}
	if (rtn < 0)
	{
		sysUSOrByte(param, PAR_ERROR);
		return (rtn);
	}
	return (flags);
}


//===============================
// Common device parameter tables
//===============================

// Common level 2 device parameter table used by most devices for the
//   general parameters

PARMTBL2 knlIoStdGenParams = {16,
{	PTITEM( ,  0, NULL             , NULL),				// 0x0000
	PTITEM(U,  4, sysIopGetFileOptn, sysIopSetFileOptn),// 0x0001
	PTITEM(L, 18, sysIopGetFileSpec, NULL),				// 0x0002
	PTITEM(U,  4, sysIopGetDevSts  , NULL),				// 0x0003
	PTITEM(U,  1, sysIopGetUnitNum , NULL),				// 0x0004
	PTITEM(U, 16, sysIopGetGlbId   , NULL),				// 0x0005
	PTITEM(U,  4, sysIopGetDelay   , sysIopSetDelay),	// 0x0006
	PTITEM(U,  4, sysIopGetTimeOut , sysIopSetTimeOut),	// 0x0007
	PTITEM(U,  1, sysIopGetInpSts  , NULL),				// 0x0008
	PTITEM(U,  1, sysIopGetOutSts  , NULL),				// 0x0009
	PTITEM(U,  1, sysIopGetInpQLmt , sysIopSetInpQLmt),	// 0x000A
	PTITEM(U,  1, sysIopGetOutQLmt , NULL),				// 0x000B
	PTITEM( ,  0, NULL             , NULL),				// 0x000C
	PTITEM( ,  0, NULL             , NULL),				// 0x000D
	PTITEM( ,  0, NULL             , NULL),				// 0x000E
	PTITEM(U,  4, sysIopGetNumOpen , NULL),				// 0x000F
	PTITEM(U,  1, sysIopGetBufrLmt , sysIopSetBufrLmt)	// 0x0010
}};

// Level 2 device parameter table for file IO parameters for non-file devices

PARMTBL2 knlIoDfltFilParams = {17,
{	PTITEM( ,  0, NULL         , NULL),				// 0x0100
	PTITEM( ,  0, NULL         , NULL),				// 0x0101
	PTITEM(U,  2, sysIopGetNull, sysIopSetNull),	// 0x0102 - SRCATTR
	PTITEM(U,  2, sysIopGetNull, sysIopSetNull),	// 0x0103 - FILEATTR
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0104 - DIROFS
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0105 - ABSPOS
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0106 - RELPOS
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0107 - EOFPOS
	PTITEM( ,  0, NULL         , NULL),				// 0x0108
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0109 - LENGTH
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x010A - REQALLOC
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x010B - RQRALLOC
	PTITEM(U,  1, sysIopGetNull, sysIopSetNull),	// 0x010C - GRPSIZE
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x010D - ADATE
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x010E - CDATE
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x010F - MDATE
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0110 - PROT
	PTITEM(L, 34, sysIopGetNull, sysIopSetNull),	// 0x0111 - OWNER
}};

// Level 2 device parameter table for terminal IO parameters for non-terminal
//   devices

PARMTBL2 knlIoDfltTrmParams = {23,
{	PTITEM( ,  0, NULL         , NULL),				// 0x0200
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0201 - TRMSINPMODE
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0202 - TRMCINPMODE
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0203 - TRMSOUTMODE
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0204 - TRMCOUTMODE
	PTITEM(U,  2, sysIopGetNull, sysIopSetNull),	// 0x0205 - TRMBFRLIMIT
	PTITEM(U,  1, NULL         , sysIopSetNull),	// 0x0206 - TRMCLRBUFR
	PTITEM(U,  1, sysIopGetNull, sysIopSetNull),	// 0x0207 - TRMCURTYPE
	PTITEM(U,  2, sysIopGetNull, sysIopSetNull),	// 0x0208 - TRMCURPOS
	PTITEM(U,  1, sysIopGetNull, sysIopSetNull),	// 0x0209 - TRMDISPAGE
	PTITEM(U,  4, sysIopGetNull, NULL),				// 0x020A - TRMSPSTATUS
	PTITEM(U,  1, NULL         , sysIopSetNull),	// 0x020B - TRMSPBREAK
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x020C - TRMSPMODEM
	PTITEM( ,  0, NULL         , NULL),				// 0x020D
	PTITEM( ,  0, NULL         , NULL),				// 0x020E
	PTITEM( ,  0, NULL         , NULL),				// 0x020F
	PTITEM( ,  0, NULL         , NULL),				// 0x0210
	PTITEM( ,  0, NULL         , NULL),				// 0x0211
	PTITEM( ,  0, NULL         , NULL),				// 0x0212
	PTITEM(U,  1, sysIopGetNull, sysIopSetNull),	// 0x0213 - TRMCPVECT
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull),	// 0x0214 - TRMCPDATA
	PTITEM(U,  1, sysIopGetNull, sysIopSetNull),	// 0x0215 - TRMMCVECT
	PTITEM(U,  4, sysIopGetNull, sysIopSetNull)		// 0x0216 - TRMMCDATA
}};

// Minimum Level 1 device parameter table

PARMTBL1 knlIoMinParams = {3,
{	&knlIoStdGenParams,
	&knlIoDfltFilParams,
	&knlIoDfltTrmParams
}};

// Minimum device dependent device parameter table - This table may be used by
//   device drivers which do not require any device dependent parameters

PARMTBL2 minddparams = {1,
{	PTITEM(U, 12, sysIopGetClass, sysIopChkClass)	// 0x8000 Device class
}};

PARMTBL1 knlIoMinDDParams = {1,
{	&minddparams
}};


//************************************************************
// Function: sysIoMinOpen - Minimum sd_open function
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

long XOSFNC sysIoMinOpen(
	char *spec)
{
	long rtn;

	rtn = 0;
	if (knlTda.parm != NULL)
		rtn = sysIoProcDevParamNew(&knlIoMinParams, &knlIoMinDDParams, NULL, 0);
	return (rtn);
}


//==================================
// Common device parameter functions
//==================================


//******************************************************
// Function: sysIopGetNull - Null get parameter function
// Returned: 0
//******************************************************

long XOSFNC sysIopGetNull(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (0);
}


//******************************************************
// Function: sysIopSetNull - Null set parameter function
// Returned: 0
//******************************************************

long XOSFNC sysIopSetNull(
	PINDEX index,
	char  *val,
	long   cnt,
	void  *data)
{
	return (0);
}


//**************************************************************
// Function: sysIopGetFileOptn - 0x001 - Get file options bits
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopGetFileOptn(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutLong((long *)val, knlTda.pvfileoptn));
}


//**************************************************************
// Function: sysIopSetFileOptn - 0x001 - Set file options bits
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopSetFileOptn(
	PINDEX  index,
	char   *val,
	long    cnt,
	void   *data)
{
	return (sysUSGetLong((long *)val, &knlTda.pvfileoptn));
}


//********************************************************************
// Function: sysIopGetFileSpec - 0x002 - Get file specification string
// Returned: Number of bytes stored or a negative XOS error code
//********************************************************************

// Storing of this string is defered if device is being opened. Otherwise
//   the name of the device followed by a colon is stored. The minimum size
//   is enough for the device name, a colon, and a 0 byte.

long XOSFNC sysIopGetFileSpec(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	if (knlTda.dcb != NULL)				// Do we have a device?
	{									// Yes
		if (val >= systembase || ((char *)cpnt) >= systembase)
										// Valid addresses?
			return (ER_ADRER);			// No - fail
		knlTda.ppfilespec = val;		// OK - store addresses and size
		knlTda.plfilespec = cpnt;
		knlTda.psfilespec = cnt;
		knlTda.pcfilespec = 0;
		if (knlTda.pvfileoptn & (XFO_XOSDEV|XFO_DOSDEV|XFO_VOLUME))
										  // Does he wnat the device name?
			return (sysIoStoreDevName()); // Yes - give it to him
	}
	return (0);
}


//**************************************************************
// Function: sysGetIopDevSts - 0x003 - Get device status
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopGetDevSts(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutLong((long *)val, knlTda.dcb->dsp));
}


//**************************************************************
// Function: sysIopGetUnitNum - 0x004 - Unit number
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopGetUnitNum(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutByte(val, knlTda.dcb->punit));
}


//**************************************************************
// Function: sysIopGetGblId - 0x005 - Global device ID
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

// The global device ID has the following format:
//   Size    Use
//     4   File ID (position of file on disk) (0 if not disk)
//     4   Device ID (dcb->id)
//     4   Node ID (network address) (0 for local device)
//     4   Network ID (0 for local device)

long XOSFNC sysIopGetGlbId(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	long rtn;

	if ((rtn = sysUSPutLong((long *)val, 0)) < 0 ||
			(rtn = sysUSPutLong((long *)(val + 4), knlTda.dcb->id)) < 0 ||
			(rtn = sysUSPutLong((long *)(val + 8), 0)) < 0 ||
			(rtn = sysUSPutLong((long *)(val + 12), 0)) < 0)
		return (rtn);
	return (16);
}


//**************************************************************
// Function: sysIopGetIopDelay - 0x006 - Get IO delay value
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopGetDelay(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutULLongV(val, knlTda.delay, cnt));
}


//**************************************************************
// Function: sysIopSetIopDelay - 0x006 - Set IO delay value
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopSetDelay(
	PINDEX  index,
	char   *val,
	long    cnt,
	void   *data)
{
	return (sysUSGetULLongV(val, &knlTda.delay, cnt));
}


//**************************************************************
// Function: sysIopGetTimeOut - 0x007 - Get IO time-out Value
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopGetTimeOut(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutULLongV(val, knlTda.timeout, cnt));
}


//**************************************************************
// Function: sysIopSetTimeOut - 0x007 - Get IO time-out Value
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopSetTimeOut(
	PINDEX  index,
	char   *val,
	long    cnt,
	void   *data)
{
	return (sysUSGetULLongV(val, &knlTda.timeout, cnt));
}


//******************************************************************
// Function: sysIopGetInpSts - 0x008 - Get device input ready status
// Returned: Number of bytes stored or a negative XOS error code
//******************************************************************

long XOSFNC sysIopGetInpSts(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutByte(val, 0));
}


//*******************************************************************
// Function: sysIopGetOutSts - 0x009 - Get device output ready status
// Returned: Number of bytes stored or a negative XOS error code
//*******************************************************************

long XOSFNC sysIopGetOutSts(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutByte(val, 0));
}


//**************************************************************
// Function: sysIopGetInpQLmt - 0x00A - Get input queue limit
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopGetInpQLmt(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutByte(val, knlTda.dcb->inpqlmt));
}


//**************************************************************
// Function: sysIopSetInpQLmt - 0x00A - Set input queue limit
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopSetInpQLmt(
	PINDEX  index,
	char   *val,
	long    cnt,
	void   *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetUByte(val, &value)) < 0)
		return (rtn);
	knlTda.dcb->inpqlmt = (char)value;
	return (0);
}


//***************************************************************
// Function: sysIopGetOutQLmt - 0x00B - Get output queue limit
// Returned: Number of bytes stored or a negative XOS error code
//***************************************************************

long XOSFNC sysIopGetOutQLmt(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutByte(val, knlTda.dcb->outqlmt));
}


//***************************************************************
// Function: sysIopSetOutQLmt - 0x00B - Set output queue limit
// Returned: Number of bytes stored or a negative XOS error code
//***************************************************************

long XOSFNC sysIopSetOutQLmt(
	PINDEX  index,
	char   *val,
	long    cnt,
	void   *data)
{
	long rtn;
	long value;

	if (index.set)
	{
		if ((rtn = sysUSGetUByte(val, &value)) < 0)
			return (rtn);
		knlTda.dcb->outqlmt = (char)value;
	}
	return (0);
}


//***************************************************************
// Function: sysIopGetNumOpen - 0x00F - Get number of time device
//			   is open
// Returned: Number of bytes stored or a negative XOS error code
//***************************************************************

long XOSFNC sysIopGetNumOpen(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutLong((long *)val, knlTda.dcb->opencnt));
}


//**************************************************************
// Function: sysIopGetBufrLmt - 0x010 - Get buffer limit
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopGetBufrLmt(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysUSPutByte(val, knlTda.dcb->bufrlmt));
}


//**************************************************************
// Function: sysIopSetBufrLmt - 0x010 - Set buffer limit
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopSetBufrLmt(
	PINDEX  index,
	char   *val,
	long    cnt,
	void   *data)
{
	long rtn;
	long value;

	if ((rtn = sysUSGetUByte(val, &value)) < 0)
		return (rtn);
	knlTda.dcb->bufrlmt = (char)value;
	return (0);
}


//**************************************************************
// Function: sysIopGetClass - 0x8000 - Get device class
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopGetClass(
	PINDEX  index,
	char   *val,
	long    cnt,
	ushort *cpnt,
	void   *data)
{
	return (sysLibStrNMovZ(val, knlTda.dcb->ccb->name, 8));
}


//**************************************************************
// Function: sysIopChkClass - 0x8000 - Check device class
// Returned: Number of bytes stored or a negative XOS error code
//**************************************************************

long XOSFNC sysIopChkClass(
	PINDEX  index,
	char   *val,
	long    cnt,
	void   *data)
{
	long rtn;

	if (index.set)
	{
		if ((rtn = sysLibStrNICmp(val, knlTda.dcb->ccb->name, 8)) < 0)
			return (rtn);
		if (rtn != 0)
			return (ER_PARMV);
	}
	return (0);
}
