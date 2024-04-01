//**********************
// IO routines for XLIB
//**********************
// Written by John Goltz
//**********************

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
#include <errno.h>
#include <time.h>
#include <xos.h>
#include <xoserrmsg.h>
#include <xossvc.h>
#include "xlib.h"


static int readblk(OBJ *obj);


//*************************************************************
// Function: openobj - Open a OBJ file and read the first block
// Returned: Nothing
//*************************************************************

void openobj(
    OBJ     *obj,
    NAMEBLK *namepnt)
{
	char byte;

    if ((obj->obj_file = fopen(namepnt->nb_name, "r")) == NULL)
										// Open input OBJ file
		femsg(prgname, -errno, namepnt->nb_name); // If error

///	printf("### input file open: |%s|\n", namepnt->nb_name);

    obj->obj_name = namepnt;
	obj->obj_pos = 0;
    obj->obj_seccnt = 127;				// Make sure section count does not										//   go to 0 here
										//   run out here
	lowfirst = TRUE;
	if ((byte = getbyte(obj)) == 0x84)	// Is it the right format?
    {
        byte = getbyte(obj);			// Yes - get next byte
        if (byte > 1) 					// Legal value?
            badfmt(obj);				// No
        obj->obj_libflag = byte;		// Remember if library file
        obj->obj_modtype = MT_XOS;
    }
    else if (byte == 0x80)				// MS module start?
	{
		skpbyte(obj, -1);				// Yes - put back the first byte
        obj->obj_modtype = MT_MS;
	}
	else if (byte == 0xF0)				// MS library start?
	{
		obj->obj_libreclen = getword(obj) + 3; // Get record length

		if (obj->obj_libreclen > 512 || (512 % obj->obj_libreclen) != 0)
			femsg2(prgname, "Invalid record length in MS library", 0,
					namepnt->nb_name);
		skpbyte(obj, obj->obj_libreclen - 3); // Discard the rest of the record
		obj->obj_libflag = 1;
		obj->obj_modtype = MT_MS;

///		printf("### have MS library, record len = %d)\n", obj->obj_libreclen);
	}
    else
		badfmt(obj);					// No - loose!
}


//*************************************************************
// Function: startmod - Setup to read next module from OBJ file
// Returned: Nothing
//*************************************************************

void startmod(
    OBJ *obj)
{
    uchar byte;

	modsource = obj->obj_pos;

///	printf("### module at 0x%X\n", modoffset);

    if (obj->obj_modtype == MT_XOS)
    {
        obj->obj_seccnt = 127;
        byte = getbyte(obj);			// Get next byte
        if (byte == 0x80)				// High order byte first?
            lowfirst = FALSE;			// Yes
        else if (byte == 0x81)			// No - low order byte first?
            lowfirst = TRUE;			// Yes
        else
            badfmt(obj);				// No - error
        obj->obj_language = getbyte(obj); // Remember the language byte
    }
    else
    {
        lowfirst = TRUE;
        obj->obj_language = 0;
    }
    obj->obj_seccnt = 0;
}


//************************************
// Function: closeobj - Close OBJ file
// Returned: Nothing
//************************************

void closeobj(
    OBJ *obj)

{
	fclose(obj->obj_file);
    obj->obj_file = 0;					// Remember that it is closed
    obj->obj_name = NULL;
}


//********************************************************************
// Function: startsec - Advance to start of next section, read header,
//		and setup the byte count
// Returned: Section type
//********************************************************************

int startsec(
    OBJ *obj)
{
    int type;

    if (obj->obj_seccnt != 0)			// Anything to skip?
		skpbyte(obj, obj->obj_seccnt);	// Yes - skip it
    obj->obj_seccnt = 127;				// Ensure section count doesn't run
										//   out while reading the header
    if ((type = (int)getbyte(obj)) != 0) // Get section type
    {
///		printf("### sec type = %02X\n", type);

		if (type > SC_MAX && type < MSSC_MIN && type > MSSC_MAX)
										// Legal type?
			badsec(obj);				// No!
		obj->obj_seccnt = getword(obj);	// Yes - get section size
    }
    return (type);
}


//*******************************************
// Function: getbyte - Get byte from OBJ file
// Returned: Value of byte
//*******************************************

int getbyte(
    OBJ *obj)
{
	int val;

	if ((val = fgetc(obj->obj_file)) < 0)
		femsg(prgname, -errno, obj->obj_name->nb_name);
	obj->obj_pos++;
    if (--obj->obj_seccnt < 0)			// Reduce section count
		endofsec(obj);
    return (val);
}


//*******************************************
// Function: getword - Get word from OBJ file
// Returned: Value of word
//*******************************************

int getword(
    OBJ *obj)
{
    uint first;
    uint second;

    first = getbyte(obj);				// Get first byte
    second = getbyte(obj);				// Get second byte
    if (lowfirst)						// Low order byte first?
		return (first + (second << 8)); // Yes
    else
		return (second + (first << 8)); // No
}


//*******************************************
// Function: getlong - Get long from OBJ file
// Returned: Value of long
//*******************************************

int getlong(
    OBJ *obj)
{
    ulong first;
    ulong second;

    first = getword(obj);				// Get first word
    second = getword(obj);				// Get second word
    if (lowfirst)						// Low order byte first?
		return (first + (second << 16)); // Yes
    else
		return (second + (first << 16)); // No
}


//************************************************
// Function: getsym - Get symbol from XOS OBJ file
// Returned: Nothing
//************************************************

void getsym(
    OBJ *obj)
{
    int   cnt;
    char *pnt;

    pnt = symbuf;						// Point to symbol buffer
    cnt = SYMMAXSZ;
    while (!((*pnt=getbyte(obj)) & 0x80)) // Get symbol character
    {
	if (--cnt <= 0)						// Symbol too long?
	    fail(obj, "Symbol too long in file");
		++pnt;
    }
    *pnt++ &= 0x7F;						// Remove high bit
    *pnt = '\0';						// End it with a null
    symsize = (int)(pnt - symbuf);
}


//*************************************************
// Function: getmssym - Get symbol from MS OBJ file
// Returned: Nothing
//*************************************************

void getmssym(
    OBJ *obj)
{
    int   cnt;
    char *pnt;

    pnt = symbuf;						// Point to symbol buffer
    cnt = getbyte(obj);					// Get length of name
    if (cnt > SYMMAXSZ)
	fail(obj, "Symbol too long in file");
    symsize = cnt;
    while (--cnt >= 0)
        *pnt++ = getbyte(obj);
    *pnt = '\0';						// End it with a null
}


//*********************************************************
// Function: getitem - Get variable size item from OBJ file
// Returned: Value of item
//*********************************************************

int getitem(
    OBJ  *obj,
    uchar size)			// Size and extension flag:
						//   000 = 0 bytes, value = 0
						//   001 = 1 byte, 0 extension
						//   010 = 2 bytes, 0 extension
						//   011 = 4 bytes
						//   100 = 0 bytes, value = 0xFFFFFFFF
						//   101 = 1 byte, 1 extension
						//   110 = 2 bytes, 1 extension
						//   111 = 4 bytes
{
	switch (size & 0x03)
	{
	 case 0:
		if (size & 0x04)
			return (0xFFFFFFFFL);
		else
			return (0L);

	 case 1:
		if (size & 0x04)
			return (getbyte(obj) | 0xFFFFFF00L);
		else
			return (getbyte(obj));

	 case 2:
		if (size & 0x04)
			return (getword(obj) | 0xFFFF0000L);
		else
			return (getword(obj));

	 case 3:
		return (getlong(obj));
    }
    return (0);
}


//***********************************************************************
// Function: getnumber - Get variable length XOS number from the OBJ file
// Returned: Value of number
//***********************************************************************

int getnumber(
	OBJ *obj)
{
    int value;

    value = getbyte(obj);				// Get first byte
    if (!(value & 0x80))				// 8 bit value?
        return (value);					// Yes - return it
    value = ((value &0x7F) << 8) + getbyte(obj); // No - get next 8 bits
    if (!(value & 0x4000))				// 16 bit value?
        return (value);					// Yes - return it

    value = ((value & 0x3FFF) << 8) + getbyte(obj); // Get next 8 bits
    if (!(value & 0x200000))			// 24 bit value?
        return (value);					// Yes - return it
    return (((value & 0x1FFFFF) << 8) + getbyte(obj));
}


//*********************************************************************
// Function: getmsnum - Get variable length MS number from the OBJ file
// Returned: Value of number
//*********************************************************************

int getmsnum(
	OBJ *obj)
{
    int value;

    value = getbyte(obj);				// Get first byte
    if (!(value & 0x80L))				// 8 bit value?
        return (value);					// Yes - return it
    return (((value & 0x7F) << 8) + getbyte(obj)); // No - return 16 bits
}


//***********************************************
// Function: skpbyte - Skip bytes in the OBJ file
// Returned: Nothing
//***********************************************

void skpbyte(
	OBJ *obj,
    int  amnt)
{
	obj->obj_pos += amnt;

///	printf("### in skpbyte: %d (%d)\n", amnt, obj->obj_pos);

	if (fseek(obj->obj_file, obj->obj_pos, 0) < 0)
		femsg(prgname, -errno, obj->obj_name->nb_name);
}


//***********************************************************************
// Function: setblk - Setup to start reading OBJ file at offset specified
// Returned: Nothing
//***********************************************************************

void setblk(
	OBJ *obj,
	long offset)
{
///	printf("### in setblk: %d\n", offset);

	obj->obj_pos = offset;
	if (fseek(obj->obj_file, obj->obj_pos, 0) < 0)
		femsg(prgname, -errno, obj->obj_name->nb_name);
}


//************************************************************
// Function: endofsec - Report unexpected end of section error
// Returned: Nothing
//************************************************************

void endofsec(
	OBJ *obj)
{
    fail(obj, "Unexpected end of section");
}


//***********************************************
// Function: badsec - Report illegal section type
// Returend: Never returns
//***********************************************

void badsec(
	OBJ *obj)

{
    fail(obj, "Illegal section type");
}


//************************************************************
// Function: badfmt - Report illegal format in OBJ file errors
// Returned: Never returns
//************************************************************

void badfmt(
	OBJ *obj)

{
    fail(obj, "Illegal OBJ file format");
}


//*********************************************************
// Function: fail - Report fatal errors with simple message
// Returned: Never returns
//*********************************************************

void fail(
	OBJ  *obj,
    char *msg)

{
    fprintf(stderr, "? XLIB: %s, file %s\n", msg,obj->obj_name->nb_name);
    loose();
}


//*****************************************************************
// Function: loose - Clean up loose ends before exiting after error
// Returned: Never returns
//*****************************************************************

void loose(void)

{
    exit(1);
}
