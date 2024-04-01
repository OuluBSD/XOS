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

// This file contains new style common characeristics functions and some
//   commonly used info strings

// Define some commonly used device characteristic information strings

INFO(knlChrMsgOpenCnt , "Number of times device opened");
INFO(knlChrMsgAccess  , "System access class");
INFO(knlChrMsgManufctr, "Manufacturer");
INFO(knlChrMsgProduct , "Product");
INFO(knlChrMsgModel   , "Device model");
INFO(knlChrMsgSerialNm, "Serial number or other data");
INFO(knlChrMsgRevision, "Revision");
INFO(knlChrMsgUnit    , "Unit number");
INFO(knlChrMsgUnitType, "Unit type");
INFO(knlChrMsgUnitDesp, "Unit description");
INFO(knlChrMsgClass   , "Device class");
INFO(knlChrMsgType    , "Device type");
INFO(knlChrMsgConDesp , "Controller description");
INFO(knlChrMsgIoReg   , "Base IO register number");
INFO(knlChrMsgMemReg  , "Base memory register address");
INFO(knlChrMsgInt     , "Interrupt request number");
INFO(knlChrMsgDma     , "DMA channel number");
INFO(knlChrMsgIndex   , "Index or unit number on controller");
INFO(knlChrMsgTimeOut , "Time-out period (in seconds)");
INFO(knlChrMsgByteOut , "Number of bytes output");
INFO(knlChrMsgByteIn  , "Number of bytes input");
INFO(knlChrMsgWTMax   , "Write transfer limit");
INFO(knlChrMsgRAMax   , "Maximum read-ahead blocks");
INFO(knlChrMsgDTHLimit, "Data transfer hardware limit");
INFO(knlChrMsgTDataErr, "Total number of data errors");
INFO(knlChrMsgHDataErr, "Number of hard data errors");
INFO(knlChrMsgHungErr , "Number of device time-outs");
INFO(knlChrMsgTOvrnErr, "Total number of DMA overrun errors");
INFO(knlChrMsgHOvrnErr, "Number of hard DMA overrun errors");
INFO(knlChrMsgTDevErr , "Total number of device errors");
INFO(knlChrMsgHDevErr , "Number of hard device errors");
INFO(knlChrMsgPciSlot , "PCI slot number");
INFO(knlChrMsgPciBus  , "PCI bus number");


//********************************************************
// Function: sysChrIgnore - Ignore characteristic
// Returned: 0
//********************************************************

long XOSFNC sysChrIgnore(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (0);
}


//********************************************************
// Function: sysChrDevGetClass - Get class name for device
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//********************************************************

long XOSFNC sysChrDevGetClass(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *(llong *)knlTda.dcb->ccb->name, cnt));
}


//**********************************************************
// Function: sysChrDevChkClass - Check class name for device
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//**********************************************************

long XOSFNC sysChrDevChkClass(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	llong name;
	long  rtn;

	if ((rtn = sysUSGetULLongV(val, &name, cnt)) < 0)
		return (rtn);
	return ((name != *(llong *)knlTda.dcb->ccb->name) ? ER_IDEVC : 0);
}


//*******************************************************************
// Function: sysChrDevGetSChar - Get 1-byte signed value from the DCB
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//*******************************************************************

long XOSFNC sysChrDevGetSChar(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutSLLongV(val, *((schar *)knlTda.dcb) + item->data, cnt));
}


//*********************************************************************
// Function: sysChrDevGetUChar - Get 1-byte unsigned value from the DCB
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//*********************************************************************

long XOSFNC sysChrDevGetUChar(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *((uchar *)knlTda.dcb) + item->data, cnt));
}


//********************************************************************
// Function: sysChrDevGetSShort - Get 2-byte signed value from the DCB
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//********************************************************************

long XOSFNC sysChrDevGetSShort(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutSLLongV(val, *(short *)(((char *)knlTda.dcb) +
			item->data), cnt));
}


//**********************************************************************
// Function: sysChrDevGetUShort - Get 2-byte unsigned value from the DCB
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//**********************************************************************

long XOSFNC sysChrDevGetUShort(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *(ushort *)(((char *)knlTda.dcb) +
			item->data), cnt));
}


//*******************************************************************
// Function: sysChrDevGetSLong - Get 4-byte signed value from the DCB
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//*******************************************************************

long XOSFNC sysChrDevGetSLong(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutSLLongV(val, *(long *)(((char *)knlTda.dcb) + item->data),
			cnt));
}


//*********************************************************************
// Function: sysChrDevGetULong - Get 4-byte unsigned value from the DCB
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//*********************************************************************

long XOSFNC sysChrDevGetULong(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *(ulong *)(((char *)knlTda.dcb) + item->data),
			cnt));
}


//**********************************************************
// Function: sysChrDevSetULong - Set 4-byte value in the DCB
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//**********************************************************

// This function is generally used to reset counter values where we really
//   don't care what the new value is!

long XOSFNC sysChrDevSetULong(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSGetULongV(val, (long *)(((char *)knlTda.dcb) + item->data),
			cnt));
}


//********************************************************************
// Function: sysChrDevGetSLLong - Get 8-byte signed value from the DCB
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//********************************************************************

long XOSFNC sysChrDevGetSLLong(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutSLLongV(val, *(llong *)(((char *)knlTda.dcb) + item->data),
			cnt));
}


//**********************************************************************
// Function: sysChrDevGetULLong - Get 8-byte unsigned value from the DCB
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//**********************************************************************

long XOSFNC sysChrDevGetULLong(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *(llong *)(((char *)knlTda.dcb) + item->data),
			cnt));
}


//**********************************************************************
// Function: sysChrDevGetStr - Get a null terminated string from the DCB
// Returned: Number of bytes returned if normal or a negative XOS
//			 error code if error
//**********************************************************************

long XOSFNC sysChrDevGetStr(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysLibStrNMovZ(val, ((char *)knlTda.dcb) + item->data, cnt - 1));
}


//********************************************************************
// Function: sysChrDevSetStr - Set a null terminated string in the DCB
// Returned: Number of bytes returned if normal or a negative XOS
//			 error code if error
//********************************************************************

long XOSFNC sysChrDevSetStr(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysLibStrNMovX(((char *)knlTda.dcb) + item->data, val, cnt - 1));
}


//***************************************************
// Function: sysChrGetSChar - Get 1-byte signed value
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//***************************************************

long XOSFNC sysChrGetSChar(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutSLLongV(val, *(schar *)item->data, cnt));
}


//*****************************************************
// Function: sysChrGetUChar - Get 1-byte unsigned value
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//*****************************************************

long XOSFNC sysChrGetUChar(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *(uchar *)item->data, cnt));
}


//****************************************************
// Function: sysChrGetSShort - Get 2-byte signed value
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//****************************************************

long XOSFNC sysChrGetSShort(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *(short *)item->data, cnt));
}


//******************************************************
// Function: sysChrGetUShort - Get 2-byte unsigned value
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//******************************************************

long XOSFNC sysChrGetUShort(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *(ushort *)item->data, cnt));
}


//***************************************************
// Function: sysChrGetSLong - Get 4-byte signed value
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//***************************************************

long XOSFNC sysChrGetSLong(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutSLLongV(val, *(long *)item->data, cnt));
}


//*****************************************************
// Function: sysChrGetULong - Get 4-byte unsigned value
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//*****************************************************

long XOSFNC sysChrGetULong(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *(ulong *)item->data, cnt));
}


//************************************************
// Function: sysChrSetULong - Set 4-byte value
// Returned: Number of bytes returned if normal or
//			 a negative XOS error code if error
//************************************************

// This function is generally used to reset counter values where we really
//   don't care what the new value is!

long XOSFNC sysChrSetULong(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSGetULongV(val, (long *)item->data, cnt));
}


//******************************************************
// Function: sysChrGetULLong - Get 8-byte unsigned value
// Returned: Number of bytes returned if normal or a
//			 a negative XOS error code if error
//******************************************************

long XOSFNC sysChrGetULLong(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysUSPutULLongV(val, *(llong *)item->data, cnt));
}


//*********************************************************
// Function: sysChrGetSysStr - Get a null terminated string
// Returned: Number of bytes returned if normal or a
//			 negative XOS error code if error
//*********************************************************

long XOSFNC sysChrGetStr(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysLibStrNMovZ(val, (char *)item->data, cnt - 1));
}



long XOSFNC sysChrSetStr(
	CHARITEM const *item,
	char           *val,
	long            cnt,
	void           *data)
{
	return (sysLibStrNMovX((char *)item->data, val, cnt - 1));
}


//**************************************************************
// Function: sysChrGetYesNo - Get a value of YES (Y) or NO (N)
// Returned: 0 if NO, 1 if YES, or a negative XOS error if error
//**************************************************************

long XOSFNC sysChrGetYesNo(
	char *val,
	long  cnt)
{
	long rtn;
	union
	{ char c[4];
	  long v;
	}    text;

	if (cnt > 4)
		cnt = 4;
	text.v = 0;
	if ((rtn = sysLibStrNMov(text.c, val, cnt)) < 0)
		return (rtn);
	if (sysLibStrICmp("YES", text.c) == 0 || sysLibStrICmp("Y", text.c) == 0)
		return (1);
	if (sysLibStrICmp("NO", text.c) == 0 || sysLibStrICmp("N", text.c) == 0)
		return (0);
	return (ER_CHARV);
}


//*******************************************************************
// Function: sysChrGetValTbl - Search table for characteristics value
// Returned: Value from table (positive) or ER_CHARV if not found or
//           other negative XOS error code if error
//*******************************************************************

// Value is assumed to be a string of up to 7 characters. The table must be
//   an array of CHRSTRVAL structures. The match is for the entire string.
//   It is not case sensitive.

long XOSFNC __export sysChrGetValTbl(
	char      *str,			// String value
	long       strln,
	CHRSTRVAL *tbl,			// The table
	long       tblln)		// Number of items in the table
{
	long rtn;

	strln = sysLibStrNLen(str, strln);
	if (strln > 7)
		return (ER_CHARV);

	while (--tblln >= 0)
	{
		if ((rtn = sysLibStrNICmp(str, tbl->name, strln)) <= 0)
		{
			if (rtn < 0)
				return (rtn);
			if (tbl->name[strln] == 0)
				break;
		}
		tbl++;
	}
	if (tblln < 0)
		return (ER_CHARV);
	return (tbl->value);
}
