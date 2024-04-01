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
#include <xossvc.h>
#include <xoslib.h>
#include <xoserr.h>
#include <ctype.h>

// For now, modify this to save all registers across the functions defined
//   here until everything is recompiled .

#pragma aux __syscall "*" parm reverse routine [] value struct float \
		struct caller [] modify [EAX]


//*****************************************************************
// Function: sysIoDevParm - Get or set device parameters
// Returned: 0x7FFF if normal or a negative XOS error code if error
//*****************************************************************

XOSCALL sysIoDevParm(
	long  cmd,				// Command bits
	char *name,				// Address of file specification string
	void *parm)				// Address of parameter list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_DEVPARM;
	qab.option = cmd;
	qab.buffer1 = name;
	qab.count = 0;
	qab.buffer2 = 0;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.handle);
}


//************************************************************
// Function: sysIoNextFile - Get next file in open directory
// Returned: 1 if normal or a negative XOS error code if error
//************************************************************

XOSCALL sysIoNextFile(
	long  handle,			// Device handle
	char *name,				// Name to find
	void *parm)				// Address of parameter list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_NEXTFILE;
	qab.option = 0;
	qab.handle = handle;
	qab.buffer1 = name;
	qab.count = 0;
	qab.buffer2 = 0;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.handle);
}


//************************************************
// Function: sysIoOpen - Open device/file
// Returned: Device handle if normal or a negative
//				XOS error code if error
//************************************************

XOSCALL sysIoOpen(
	long  cmd,				// Command bits
	char *name,				// Address of file specification string
	void *parm)				// Address of parameter list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_OPEN;
	qab.option = cmd;
	qab.buffer1 = name;
	qab.count = 0;
	qab.buffer2 = 0;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.handle);
}


//***************************************************
// Function: sysIoClose - Close device
// Returned: 0 if normal or a negative XOS error code
//***************************************************

XOSCALL sysIoClose(
	long handle,			// Device handle
	long cmd)				// Command bits
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_CLOSE;
	qab.option = cmd;
	qab.handle = handle;
	qab.parm = NULL;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			qab.amount);
}


//***************************************************
// Function: sysIoSetPos - Set position for IO
// Returned: Position in file (positive) if normal or
//				a negative XOS error code if error
//***************************************************

XOSCALL sysIoSetPos(
	long handle,			// Device handle
	long position,			// Position in file
	long mode)				// Mode: 0 = Absolute position
							//		 1 = Relative to current position
							//		 2 = Relative to end of file
							//		 3 = Return position only
{
	QAB  qab;
	long rtn;
	struct
	{	BYTE4PARM pos;
		char      end;
	}    parms =
	{	{PAR_GET|PAR_SET|REP_DECV, 4, IOPAR_ABSPOS}
	};

	switch (mode)
	{
	 case 0:
		break;

	 case 1:
		parms.pos.index = IOPAR_RELPOS;
		break;

	 case 2:
		parms.pos.index = IOPAR_EOFPOS;
		break;

	 default:
		parms.pos.desp = PAR_GET|REP_DECV;
		break;
	}
	parms.pos.value = position;
	sysLibMemSetLong((long *)&qab, 0, sizeof(QAB) / 4);
	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_INBLOCK;
	qab.handle = handle;
	qab.parm = &parms;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 parms.pos.value);
}


//**********************************************************
// Function: sysIoInSingleP - Input one byte with parameters
// Returned: Input byte (8 bits, 0 filled to 32 bits)
//				or a negative XOS error code
//**********************************************************

XOSCALL sysIoInSingleP(
	long  handle,		// Device handle
	void *parm)			// Address of parameter list
{
	QAB  qab;
	long rtn;
	char buffer[4];

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_INBLOCK;
	qab.handle = handle;
	qab.buffer1 = buffer;
	qab.count = 1;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			buffer[0]);
}


//***************************************************
// Function: sysIoInSingle - Input one byte
// Returned: Input byte (8 bits, 0 filled to 32 bits)
//				or a negative XOS error code
//***************************************************

XOSCALL sysIoInSingle(
	long handle)			// Device handle
{
	QAB  qab;
	long rtn;
	char buffer[4];

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_INBLOCK;
	qab.handle = handle;
	qab.buffer1 = buffer;
	qab.count = 1;
	qab.parm = NULL;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			buffer[0]);
}


//************************************************************
// Function: sysIoOutSingleP - Output one byte with parameters
// Returned: Number of bytes output (1) or a negative XOS
//				error code
//************************************************************

XOSCALL sysIoOutSingleP(
	long  handle,			// Device handle
	long  value,			// Value to output
	void *parm)				// Address of parameter list
{
	QAB  qab;
	long rtn;

	qab.handle = handle;
	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_OUTBLOCK;
	qab.buffer1 = (char *)&value;
	qab.count = 1;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//*******************************************
// Function: sysIoOutSingle - Output one byte
// Returned: Number of bytes output (1) or a
//				negative XOS error code
//*******************************************

XOSCALL sysIoOutSingle(
	long handle,			// Device handle
	long value)				// Value to output
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_OUTBLOCK;
	qab.handle = handle;
	qab.buffer1 = (char *)&value;
	qab.count = 1;
	qab.parm = NULL;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//**************************************************************
// Function: sysIoOutStringP - Output string with parameter list
// Returned: The number of bytes actually output (positive) or
//				a negative XOS error code if error
//**************************************************************

XOSCALL sysIoOutStringP(
	long  handle,		// Device handle
	char *buffer,		// Address of buffer
	long  count,		// Maximum number of bytes to output
	void *parm)			// Address of parameter list
{
	long rtn;

	return (((rtn = sysLibStrNLen(buffer, count)) < 0) ? rtn :
			sysIoOutBlockP(handle, buffer, rtn, parm));
}


//**************************************************************
// Function: sysIoOutString - Output string
// Returned: The number of bytes actually output (positive) or
//				a negative XOS error code if error
//**************************************************************

XOSCALL sysIoOutString(
	long  handle,			// Device handle
	char *buffer,			// Address of buffer
	long  count)			// Maximum number of bytes to output
{
	long rtn;

	return (((rtn = sysLibStrNLen(buffer, count)) < 0) ? rtn :
			sysIoOutBlock(handle, buffer, rtn));
}


//***********************************************************
// Function: sysIoInBlockP- Input block with parameter list
// Returned: The number of bytes actually input (positive) or
//				a negative XOS error code if error
//***********************************************************

XOSCALL sysIoInBlockP(
	long  handle,			// Device handle
	char *buffer,			// Address of buffer
	long  count,			// Length of buffer
	void *parm)				// Address of parameter list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_INBLOCK;
	qab.handle = handle;
	qab.buffer1 = buffer;
	qab.count = count;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//********************************************************
// Function: sysIoInBlock - Input block
// Returned: The number of bytes actually input (positive)
//				or a negative XOS error code if error
//********************************************************

XOSCALL sysIoInBlock(
	long   handle,			// Device handle
	char  *buffer,			// Address of buffer
	long   count)			// Length of buffer
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_INBLOCK;
	qab.handle = handle;
	qab.buffer1 = buffer;
	qab.count = count;
	qab.parm = 0;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//***********************************************************
// Function: sysIoOutBlockP- Output block with parameter list
// Returned: The number of bytes actually input (positive) or
//				a negative XOS error code if error
//***********************************************************

XOSCALL sysIoOutBlockP(
	long  handle,			// Device handle
	char *buffer,			// Address of buffer
	long  count,			// Number of bytes to output
	void *parm)				// Address of parameter list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_OUTBLOCK;
	qab.handle = handle;
	qab.buffer1 = buffer;
	qab.count = count;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//********************************************************
// Function: sysIoOutBlock - Output block
// Returned: The number of bytes actually input (positive)
//				or a negative XOS error code if error
//********************************************************

XOSCALL sysIoOutBlock(
	long   handle,			// Device handle
	char  *buffer,			// Address of buffer
	long   count)			// Number of bytes to output
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_OUTBLOCK;
	qab.handle = handle;
	qab.buffer1 = buffer;
	qab.count = count;
	qab.parm = 0;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//*************************************************************
// Function: sysIoPath - Path function
// Returned: Length of the string returned (positive) if normal
//				or a negative XOS error code if error
//*************************************************************

XOSCALL sysIoPath(
	long  options,			// Option bits
	char *name,				// Name
	char *buffer,			// Buffer
	long  length)			// Length of the buffer
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_PATH; // Store function
	qab.option = options;
	qab.buffer1 = name;					// Address of device name
	qab.count = length;					// Length of the buffer
	qab.buffer2 = buffer;				// Address of the buffrer
	qab.handle = 0;
	qab.parm = 0;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//*************************************************************
// Function: sysIoSpecial - Special device function
// Returned: 0  if normal or a negative XOS error code if error
//*************************************************************

XOSCALL sysIoSpecial(
	long  handle,			// Device handle
	long  function,			// Function value
	char *buffer,			// Address value
	long  count,			// Count value
	void *parm)				// Address of parameter list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_SPECIAL;
	qab.handle = handle;
	qab.option = function;
	qab.buffer1 = buffer;
	qab.buffer2 = 0;
	qab.count = count;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//**************************************************************
// Function: sysIoDelete - Delete file
// Returned: Depends on function and device (positive) if normal
//				or a negative XOS error code if error
//**************************************************************

long sysIoDelete(
	long  cmdbits,			// Open command bits
	char *name,				// Address of file specification string
	void *parm)				// Address of parameter list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_DELETE;
	qab.buffer1 = name;
	qab.count = 0;
	qab.option = cmdbits;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//************************************************************
// Function: sysIoRename - Rename file
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

XOSCALL sysIoRename(
	long  cmd,				// Command bits
	char *oldname,			// Old name
	char *newname,			// New name
	void *parm)				// Address of parameter list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_RENAME;
	qab.option = cmd;
	qab.buffer1 = oldname;
	qab.count = 0;
	qab.buffer2 = newname;
	qab.parm = parm;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//*****************************************************************
// Function: sysIoClsChar - Class characteristics values function
// Returned: 0x7FFF if normal or a negative XOS error code if error
//*****************************************************************

XOSCALL sysIoClsChar(
	char *name,				// Address of class name string
	char *chrlst)			// Address of characteristics list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_CLASSFUNC;
	qab.option = CF_VALUES; 			// Store class function
	qab.buffer1 = name;					// Class name
	qab.count = 0;
	qab.buffer2 = chrlst;				// Characteristics list
	qab.parm = 0;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//*****************************************************************
// Function: sysIoDevChar - Device characteristics values function
// Returned: 0x7FFF if normal or a negative XOS error code if error
//*****************************************************************

XOSCALL sysIoDevChar(
	long  handle,			// Device handle
	void *chrlst)			// Address of characteristics list
{
	QAB  qab;
	long rtn;

	qab.func = QFNC_WAIT|QFNC_DEVCHAR;
	qab.option = CF_VALUES; 			// Store class function
	qab.handle = handle;
	qab.count = 0;
	qab.buffer2 = chrlst;				// Characteristics list
	qab.parm = 0;
	qab.vector = 0;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//************************************************************
// Function: sysIoCommit
// Returned: 0 if normal or a negative XOS error code if error
//************************************************************

XOSCALL sysIoCommit(
	long handle)			// Device handle
{
	QAB  qab;
	long rtn;

	sysLibMemSetLong((long *)&qab, 0, sizeof(QAB)/4);
	qab.func = QFNC_WAIT|QFNC_DIO|QFNC_COMMIT;
	qab.handle = handle;
	return (((rtn = svcIoQueue(&qab)) < 0 || (rtn = qab.error) < 0) ? rtn :
			 qab.amount);
}


//*********************************************************************
// Function: sysIoGetDosDevice - Get corresponding DOS single letter
//		 		device if one is available given a file specification.
//				The caller should have already verified that the
//				device name is not a single character.
// Returned:  The length of the modified file specification if a single
//				character name was available,  0 if no single character
//				name was available (the file specfication is unchanged)
//				or a negative XOS error code if error.
//*********************************************************************

XOSCALL sysIoGetDosDevice(
	char *spec,				// Address of full file specification
	char *buffer,			// Buffer to receive file specification (should
							//   be at least the same length as the file
							//   specification) - May be the same as the file
							//   specification. In this case it is not modified
							//   if no single character device name was found.
	long  length)
{
	long  pos;
	long  rtn;
	char  bufr[32];
	struct
	{	BYTE4PARM  optn;
		LNGSTRPARM spec;
		uchar      end;
	}     parms =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_DOSDEV},
		{PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, 31, 31}
	};
	if ((pos = sysLibStrChr(spec, ':')) < 0)
	{									// Caller must include a device name!
		if (pos == ER_NTFND)
			pos = ER_VALUE;
		return (pos);
	}
	parms.spec.buffer = bufr; 
	if ((rtn = svcIoDevParm(XO_RAW, spec, (uchar *)&parms)) < 0)
		return (rtn);

	if (bufr[1] != ':')					// Did we get a single character name?
		return (0);						// No - return 0
										// Yes - set up the string and return
										//   the length
	buffer[0] = bufr[0];
	buffer[1] = ':';
	if ((rtn = sysLibStrMov(buffer + 2, buffer + pos)) < 0)
		return (0);
	return (rtn + 2);
}
