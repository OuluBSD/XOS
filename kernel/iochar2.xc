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

#pragma disable_message (120);


// This file contains the new style routines to process characteristics.
//   The modivation in creating this is to provide a way to write
//   characteristics routines in C. The old style scheme passed
//   arguments in registers, did not preserve registers.

// From a user's viewpoint, these routines are the same as the old ones. Any
//   device routine currently can be written to use either the old or new
//   style routines. Once everything is converted, the old style routines will
//   go away.

// The first two functions implement the QFNC_CLASSFUNC and QFNC_DEVCHAR
//   queued IO function. Arguments in the QAB are used as follows:
//		qab_option  - Bits 7-0: Function:
// 						CF_SIZES   = 1  - Get sizes needed for complete list.
// 						CF_ALL     = 3  - Get names, types, and sizes of all
// 											items. If qab_parm is not NULL the
//					 						the info buffer is filled in with
//											the size of each info string. Value
//											returned is the number of items
//											filled in.
//						CF_ONE     = 5  - Get type and size of single
//											characteristic. Also returns length
//											of the info string if qab_parm is
//											not NULL.
//						CF_VALUES  = 7  - Get or set characteristics values. If
//											getting a value and qab_parm is not
//											NULL the info string also returned.
//						CF_ADDUNIT = 8  - Add unit function.
//						CF_PUNITS  = 9  - Get information about physical units.
//						CF_AUNITS  = 10 - Get information about active units.
//   					For QFNC_CLASSFUNC, each class may also define
//						additional class dependent functions start with a
//						value of 11. Also, some classes may not implement one
//						or more of the functions 8 through 10.
//		qab_count   - Size of available characteristics buffer plus 0x10000
//						times size of available info list buffer (CF_ALL only).
//		qab_buffer1 - Address of the class name string.
//		qab_buffer2 - Address of device characteristics list.
//		qab_parm    - Address of info pointer list.
//	Value returned in qab_amount is:
//		CF_SIZE   - Minimum length for the characteristics list plus 0x10000
//					  times the minimum length for the info pointer list.
//		CF_ALL    - The number of items in the list
//		CF_ONE    - 0
//		CF_VALUES - The number of items processed

// The info buffer contains an address/length pair (both longs) for each
//   characteristic in the same order as the entires in the caller's
//   characteristics list.

// Obtaining the full characteristics list is a 5-step process:
//   1) Obtain the size buffer(s) needed for a complete characteristics with
//        CF_SIZE.
//   2) Allocate the buffer(s).
//   3) Fill in the characteristics list with CF_ALL.
//   4) Allocate buffers for all string values and for all info strings.
//   5) Obtain the values (and info strings) using CF_VALUES.

// Obtaining the value of a single characteristic is simplier:
//   1) Use CF_ONE to obtain the type and size of the function. The buffer
//      specified must be at least 16 bytes, which is the maximum length that
//      will be written for single characteristic. Note that a characteristic
//      may be longer than this. No terminating 0 is stored. If an info list
//      is specified, it must be at least 8 bytes.
//   2) Allocate buffer(s) if necessary.
//   3) Use CF_VALUES to obtain the value.

// Each device or class specifies a characteristics function which is called
//   for a CF_VALUES function. The address of this function is stored in the
//   "func" item in the characteristics table header (CHARHEAD). Normally
//   this item contains the address of sysIoCharValues{New} which processes
//   each characteristic in the table. If necessary a device or class can
//   specify it's own routine which does specific setup for the device or
//   class and then calls sysIoCharVaues{New}. This function is called
//   as a svcIoQueue function and may invoke an IO thread by calling
//   sysIoThreadQueue. It may return requesting a direct IO re-do.
//   Characteristic routines already executed will not be executed on a re-do
//   (assuming that sysCharVaues{New} is used to execute the routines. The
//   other CF_* functions are handled completely in this module using only the
//   information in the characteristics table. They never start an IO thread
//   or request a direct re-do. Any additional function specified by a class
//   are also handled as svcIoQueue functions.

// The CF_VALUES routine is called as a svcIoQueue function with knlTda.dcb
//   containing the address of the DCB. The address of the caller's
//   characteristics list is in knlTda.buffer2 and the address of his info list
//   buffer is in knlTda.parm.

// This module pretty much replaces the old style characteristics module. The
//   format of the characteristics tables has not changed but the calling
//   sequences for the characteristics get and set function is significantly
//   different. This module replaces the old module except for the CF_VALUES
//   function. When an old style characteristics table is encountered (as
//   identified by its header label value) the old CF_VALUES function is
//   called.

// All new format characteristics get and set functions have the same calling
//   sequence:
//		long XOSFNC charfunc(CHARITEM *item, char *data, long size);
//   Where:
//     item = Address of the CHARITEM structure for the characteristic
//     data = Address of the user's data buffer
//     size = Length of the user's data buffer


static long XOSFNC charbad(const CHARTBL *tbl);
static long XOSFNC charsize(const CHARTBL *tbl);
static long XOSFNC charall(const CHARTBL *tbl);
static long XOSFNC charone(const CHARTBL *tbl);
static long XOSFNC charvalue(const CHARTBL *tbl);
static long XOSFNC getcharsize(const CHARITEM *item);
static long XOSFNC getcharall(const CHARITEM *item);
static long XOSFNC getcharone(const CHARITEM *item);


static long XOSFNC findchar(CHARITEM const *list, CHARUSER *user,
		CHARITEM const **pitem);
static CHARITEM const XOSFNC *findchar2(CHARITEM const *item, llong name);


// Dispath table for characteristics functions - Used for both device and
//   class characteristics.

static long XOSFNC (*chardisp[])(CHARTBL const *) =
{	charbad,
	charsize,			// CF_SIZE   = 1 - Get size needed for everything
	charbad,			//           = 2
	charall,			// CF_ALL    = 3 - Get names, types, and sizes of all
						//                 characteristics
	charbad,			//           = 4
	charone,			// CF_ONE    = 5 - Get type and sizes for single
						//                 characteristic
	charbad,			//           = 6
	charvalue			// CF_VALUES = 7 - Get or set characteristic
};						//                 values


//************************************************************
// Function: classfunc - Queued IO function for QFNC_CLASSFUNC
// Returned: Queued IO return value
//************************************************************

long XOSFNC classfunc(void)
{
	char *junk;
	CCB  *ccb;
	union
	{	long   *l;
		const CFDISP *t;
	}     cfdisp;
	long  rtn;

	if (knlTda.buffer1 >= systembase && knlTda.buffer1 < usercodebase &&
			knlTda.buffer1 > usercodetop)
		return (ER_ADRER);
	if ((rtn = setdevname(knlTda.buffer1, 0, &junk)) < 0)
		return (rtn);
	if (knlTda.devname[8] != 0)
		return (ER_NSCLS);
	ccb = knlFirstCcb;
	while (ccb != NULL)
	{
		if (*(long *)(knlTda.devname + 0) == *(long *)(ccb->name + 0) &&
				*(long *)(knlTda.devname + 4) == *(long *)(ccb->name + 4))
			break;
		ccb = ccb->next;
	}
	if (ccb == NULL)
		return (ER_NSCLS);
	knlTda.ccb = ccb;
	knlTda.chardata = 0;				// Clear our data item

	if (((char)knlTda.option) <= 7)		// Characteristics function?
	{
		if (ccb->clschar == NULL)
			return (ER_IFDEV);
		if ((rtn = chardisp[(int)((char)knlTda.option)](ccb->clschar)) < 0)
			return (rtn);
		knlTda.status |= QSTS_DONE;
		return (0);
	}
	else
	{
		// This supports both old and new style function dispatch tables. The
		//   old style table has the number of entries stored in the long
		//   immediately before the table while the new style has the number
		//   of entires in the first long of the table. In both cases this
		//   long is immediately followed by an array of function addresses
		//   indexed by the class function number minus 8. We determine which
		//   format we have by checking bit 31 of the long pointed to by
		//   ccb.cfdisp. If it is set it is assumed to be a function address
		//   and we have an old style table. If it is clear, it is assumed to
		//   be the entry count and we have a new style table.

		knlTda.option -= 8;
		cfdisp.t = ccb->cfdisp;
		if (*cfdisp.l < 0)				// Old style table?
			cfdisp.l--;					// Yes - fix up the address to make it
										//   look like a new style table!

		if (((char)knlTda.option) >= cfdisp.t->size ||
				cfdisp.t->func[(int)(char)knlTda.option] == NULL)
			return (ER_IFDEV);
		return (cfdisp.t->func[(int)(char)knlTda.option]());
	}
}


//************************************************************
// Function: devcharfunc - Queued IO function for QFNC_DEVCHAR
// Returned: Queued IO return value
//************************************************************

long XOSFNC devcharfunc(void)
{
	long rtn;

	if ((rtn = sysIoGetDcb()) < 0)		// Get our DCB
		return (rtn);

///	TESTB	dcb_sts1[EDI], #D1$REMOVED
///	JNE	devrmvd#

	if (((char)knlTda.option) > 7 || knlTda.dcb->devchar == NULL)
		return (ER_IFDEV);
	if ((rtn = chardisp[(int)(char)knlTda.option](knlTda.dcb->devchar)) < 0)
		return (rtn);
	knlTda.dcb = NULL;
	knlTda.status |= QSTS_DONE;
	return (0);
}


//***********************************************************
// Function: charsize - Implements the CF_SIZE characteristic
//                        function - get sizes needed for all
//                        characteristics.
// Returned:
//***********************************************************

// When complete the following global values have been set:
//   c[TDA.amount+0] = Length needed for the characteristic list buffer
//   c[TDA.amount+2] = Length needed for the info pointer table buffer

static long XOSFNC charsize(
	const CHARTBL *tbl)
{
	long rtn;

	if ((rtn = getcharsize(tbl->head.first)) < 0)
		return (rtn);
	(*((ushort *)&knlTda.amount))++;
	knlTda.status |= QSTS_DONE;
	return (0);
}


//**********************************************************
// Function: charall - Implements the DCF_ALL characteristic
//					   function - Get name and type for all
//					   characteristics
// Returned:
//**********************************************************

// The following global values are used as parameters:
//   c{TDA.buffer2} = Address of buffer for the characteristic list
//   c{TDA.parm}    = Address of buffer for the info pointer table
//   c[TDA.count+0] = Length of the characteristic list buffer
//   c[TDA.count+2] = Length of the info pointer table buffer
// When complete the following global values have been set:
//   c[TDA.amount+0] = Actual length of the characteristic list stored
//   c[TDA.amount+2] = Actual length of the info pointer table stored

static long XOSFNC charall(
	const CHARTBL *tbl)
{
	long rtn;

	if ((rtn = sysLibMemSet(knlTda.buffer2, 0, (ushort)knlTda.count)) < 0)
		return (rtn);					// Clear the buffer
	(*((ushort *)&knlTda.amount))++;	// Allow for a final 0
	(*((ushort *)&knlTda.count))--;
	if ((rtn = getcharall(tbl->head.first)) || // Do the work
			(rtn = sysUSPutByte(knlTda.buffer2, 0)) < 0) // Put a 0 at the end
		return (rtn);
	knlTda.status |= QSTS_DONE;
	return (0);
}


//***************************************************************
// Function: charone - Implements the DCF_ONE function - Get type
//					   and sizes for a single characteristic
// Returned:
//***************************************************************

static long XOSFNC charone(
	const CHARTBL *tbl)
{
	CHARITEM const *item;
	long            rtn;

	if ((rtn = findchar(tbl->head.first, knlTda.buffer2, &item)) < 0 ||
										// Find the item
			(rtn = sysLibMemSet(knlTda.buffer2, 0, 10)) < 0)
		return (rtn);					// Clear the minimum buffer we need
	knlTda.count = 0x7FFF7FFF;
	if ((rtn = getcharone(item)) < 0)	// Give him the values
		return (rtn);
	knlTda.status |= QSTS_DONE;
	return (0);
}


//***************************************************************
// Function: charvalue - Implements the DCF_VALUES function - Get
//						 or set characteristic values
// Returned:
//***************************************************************

static long XOSFNC charvalue(
	const CHARTBL *tbl)
{
	long rtn;


	if (tbl->head.label == 'CHRH')
		rtn = tbl->head.values(0, tbl->head.first, NULL);
	else
		rtn = ((long XOSFNC (*)(long, const CHARTBL *))
				tbl->head.values)(0, tbl);
	if (rtn < 0)
		return (rtn);

///	if ((rtn = tbl->head.values(0, (tbl->head.label == 'CHRH') ?
///			tbl->head.first : (CHARITEM *)tbl)) < 0)
///		return (rtn);

	knlTda.status |= QSTS_DONE;
	return (0);
}


//***********************************************************
// Function: charbad - Illegal characteristic function
// Returned: ER_IFDEV
//***********************************************************

static long XOSFNC charbad(
	const CHARTBL *tbl)
{
	return (ER_IFDEV);
}


static long XOSFNC getcharsize(
	const CHARITEM *item)
{
	long len;
	long rtn;

	while (item != NULL)
	{
		if ((item->flags & CIF_HIDDEN) == 0)
		{
			if (item->flags & CIF_INDIR)
			{
				if ((rtn = getcharsize(item->link)) < 0)
					return (rtn);
			}
			else
			{
				len = (item->rep == REP_STR || item->rep == REP_DATAS) ? 18 :
						(item->clength == 0xFFFF) ? item->getlen() :
						(len = item->clength + 10);
				*(ushort *)&knlTda.amount += (ushort)len;
				if (item->info != NULL)
					*(ushort *)(((char *)&knlTda.amount) + 2) += 
							(item->info->size + 1);
			}
		}
		item = item->next;
	}
	return (0);
}


//**************************************************************
// Function: getcharall - Returns a skelleton for all non-hidden
//                        charactertistics
// Returned:
//**************************************************************

// This is called recursively for indirect tables

static long XOSFNC getcharall(
	const CHARITEM *item)
{
	long rtn;

	while (item != NULL)
	{
		if ((item->flags & CIF_HIDDEN) == 0)
		{
			if (item->flags & CIF_INDIR)
			{
				if ((rtn = getcharall(item->link)) < 0)
					return (rtn);
			}
			else
			{
				if ((rtn = getcharone(item)) < 0)
					return (rtn);
			}
		}
		item = item->next;
	}
	return (0);
}


static long XOSFNC getcharone(
	const CHARITEM *item)
{
	long rtn;
	long need;
	long len;

	if (item->rep != REP_STR && item->rep != REP_DATAS)
	{
		len = (item->clength == 0xFFFF) ? item->getlen() : item->clength;
		need = len + 10;
		if ((*((short *)&knlTda.count) -= need) < 0)
			return (ER_BFRTS);
	}
	else
	{
		len = 0;
		need = 18;
		if ((*((short *)&knlTda.count) -= 18) < 0)
			return (ER_BFRTS);
		if ((rtn = sysUSPutShort((short *)(((char *)knlTda.buffer2) + 14),
				(item->clength == 0xFFFF) ? item->getlen() :
				item->clength)) < 0)
			return (rtn);
	}
	if ((rtn = sysUSPutShort((short *)knlTda.buffer2, (item->rep |
				(len << 8)) | ((char *)&knlTda.option)[1])) < 0 ||
			(rtn = sysUSPutLLong((llong *)(((char *)knlTda.buffer2) + 2),
				*(llong *)item->name)) < 0)
		return (rtn);
	knlTda.buffer2c += need;
	knlTda.amount++;
	if (knlTda.parm != NULL)			// Does he want the info size?
	{
		if ((*((short *)((char *)&knlTda.count) + 2) -= 8) < 0)
			return (ER_BFRTS);

		if ((rtn = sysUSPutLong(&((INFOPNT *)knlTda.parm)->length,
				(item->info == NULL) ? 0 : (item->info->size + 1))) < 0)
			return (rtn);
		knlTda.parmc += 8;
	}
	return (0);
}


//****************************************************
// Function: sysChrValuesNew - Process characteristics
// Returned:
//****************************************************

// This function is either called directly from the "values" dispatch in the
//   Characteristics table header or is called at the end of a device/class
//   dependent "values" function. It is also called directly to process
//   characteristics for addunit routines.

// The following static arguments are used:
//   c{tda.buffer2} = Address of the characteristics list
//   c{tda.parm}    = Address of the info pointer buffer
//   c{tda.Dcb}     = Address of DCB (device characteristtics only)

long XOSFNC sysChrValuesNew(
	long      badnm,			// Non-0 if bad names are OK
	const CHARITEM *first,		// Address of the first characteristics item
	void     *data)
{
	const CHARITEM *citem;
	ushort   *pcnt;
	INFOPNT  *infopnt;
	union
	{	CHARUSER *i;
		char     *c;
		ushort   *s;
	}         ctpnt;
	char     *buffer;
	long      amount;
	union
	{ long    v;			// (Must be long so we can use it with
	  struct				//    sysUSGetUShort)
	  { union
		{ char bits;
		  struct
		  {	char rep: 4;
		    char    : 2;
			char get: 1;
			char set: 1;
		  };
		};
		char  len;
	  };
	}         chead;
	int       ctype;
	long      len;
	long      length;
	long      rtn;

	if (knlTda.buffer2c >= systembase || knlTda.parmc >= systembase)
		return (ER_ADRER);
	ctpnt.i = knlTda.buffer2;
	infopnt = knlTda.parm;
	amount = 0;
	while (TRUE)
	{
		if ((rtn = sysUSGetUShort(ctpnt.s, &chead.v)) < 0)
			return (rtn);
		if ((chead.bits & (PAR_GET|PAR_SET)) == 0)
			break;
		if ((rtn = sysUSAndByte(ctpnt.c, ~PAR_ERROR)) < 0)
			break;
		ctype = chead.rep;
		if ((rtn = findchar(first, ctpnt.i, &citem)) < 0)
		{
			if (rtn != ER_CHARN || badnm == 0)
				break;
			ctpnt.c += ((ctype == REP_STR || ctype == REP_DATAS) ? 18 :
					(10 + chead.len));
		}
		else
		{
			if (ctype == REP_STR || ctype == REP_DATAS)
			{
				// Here for a string value

				if (citem->rep != REP_STR && citem->rep != REP_DATAS)
				{
					rtn = ER_CHART;
					break;
				}
				pcnt = (ushort *)(ctpnt.c + 10);
				if ((rtn = sysUSGetLong((long *)(ctpnt.c + 10),
						(long *)&buffer)) < 0 ||
						(rtn = sysUSGetUShort((ushort *)pcnt, &length)) < 0)
					break;
				if (buffer >= systembase)
				{
					rtn = ER_ADRER;
					break;
				}
///				if (length < citem->clength)
///				{
///					rtn = ER_CHARS;
///					break;
///				}
				if (chead.set)
				{
					if ((rtn = citem->setfunc(citem, buffer, length, data)) < 0)
						break;
				}
				if (chead.get)
				{
					if ((len = citem->getfunc(citem, buffer, length, data)) < 0)
					{
						rtn = len;
						break;
					}
					if (len == 0 && (rtn = sysUSPutByte(buffer, 0)) < 0)
						break;
					if ((rtn = sysUSPutShort((short *)(ctpnt.c + 16), len)) < 0)
						break;
				}
				ctpnt.c += 18;
			}
			else
			{
				// Here for a numeric value

				if (citem->rep == REP_STR || citem->rep == REP_DATAS)
				{
					rtn = ER_PARMT;
					break;
				}
				if (chead.set && (rtn = citem->setfunc(citem, ctpnt.c + 10,
						chead.len, data)) < 0)
					break;
				if (chead.get && (rtn = citem->getfunc(citem, ctpnt.c + 10,
						chead.len, data)) < 0)
					break;
				ctpnt.c += (chead.len + 10);
			}
			if (infopnt != NULL)
			{
				if ((rtn = sysUSGetUShort((ushort *)&infopnt->length,
							&length)) < 0 ||
						(rtn = sysUSGetLong((long *)&infopnt->buffer,
							(long *)&buffer)) < 0)
					break;
				if (length > 0 && buffer != NULL)
				{
					if (length > citem->info->size)
						length = citem->info->size;
					if ((rtn = sysLibStrNMovZ(buffer, citem->info->text,
							length)) < 0)
						break;
					if ((rtn = sysUSPutShort((short *)
							(((char *)&infopnt->length) + 2),
							citem->info->size)) < 0)
						break;
				}
				infopnt++;
			}
		}
	}
	if (rtn < 0)
	{
		sysUSOrByte(ctpnt.c, PAR_ERROR);
		return (rtn);
	}
	return (0);
}


//*********************************************************
// Function: findchar - Find device characteristic in table
// Returned:
//*********************************************************

static long XOSFNC findchar(
	CHARITEM const  *list,		// Address of the characteristics table
    CHARUSER        *user,		// Address of user's characteristic item
	CHARITEM const **pitem)		// Address of pointer to receive address of
								//   characteristics item found
{
	CHARITEM const *fnd;
	long            rtn;
	union
	{	char  c[12];
		llong ll;
	}               name;

	if ((rtn = sysUSAndByte((char *)user, ~PAR_ERROR)) < 0)
		return (rtn);					// Clear user's error field
	name.ll = 0;						// Clear the name buffer

	if ((rtn = sysLibStrNMovZ(name.c, user->name, 8)) < 0)
		return (rtn);					// Copy up to 8 name characters
	name.ll = sysLibStr8ToUpper(name.ll);

	if ((fnd = findchar2(list, name.ll)) == NULL)
		return (ER_CHARN);
	*pitem = fnd;
	return (0);
}


//**********************************************************
// Function: findchar2 - Find name in a characteritics table
// REturned: The address of the characteristics entry found
//		     or NULL if not found
//**********************************************************

// This is called recursively for indirect tables

static const CHARITEM XOSFNC *findchar2(
	CHARITEM const *item,		// Address of first characteristics entry
	llong           name)		// Address of name to search for (zero filled
								//   to 8 bytes)
{
	CHARITEM const *fnd;

	while (item != NULL)
	{
		if (item->flags & CIF_INDIR)
		{
			if ((fnd = findchar2(item->link, name)) != NULL)
				return (fnd);
		}
		else
		{
			if (name  == *(llong *)item->name)
				return (item);
		}
		item = item->next;
	}
	return (NULL);
}


//**************************************************************
// Function: sysChrFixUp - Fix up the characteristics table when
//					       device is initialized
// Returned: Nothing (Crashes system if error)
//**************************************************************

// This function is needed because there is no good way to create a forward
//   linked list in C. It is called recursively for linked tables.

void XOSFNC sysChrFixUp(
	CHARTBL *tbl)
{
	CHARITEM *item;

	if (tbl->head.label == 'CHRH')		// Do we have a pointer to a header?
	{
		item = (CHARITEM *)(((char *)tbl) + sizeof(CHARHEAD));
		tbl->head.first = item;			// Yes - store address of first item
	}
	else if (tbl->head.label == 'CHRI' || tbl->head.label == 'CHRL')
										// Do we have a pointer to a table?
		item = (CHARITEM *)tbl;			// Yes
	else
		CRASH('NCHR');

	// Store the forward pointer in each item except the last

	while (TRUE)
	{
		if (item->label == 'CHRL')
			sysChrFixUp((CHARTBL *)item->link);
		else if (item->label != 'CHRI')
			CRASH('NCHR');
		if (item->next != (CHARITEM *)0xFFFFFFFF)
			break;
		item->next = item + 1;
		item++;
	}
}
