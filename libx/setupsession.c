#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xossetupsession.h>


typedef struct chrs
{	LNGSTRCHAR item;
	uchar      end;
} CHRS;


static long setchrs(CHRS *chrs, uchar *val);


long setupsession(
	char *username, 
	uchar *apvpnt,
	uchar *ipvpnt,
	uchar *ascpnt,
	uchar *iscpnt)

{
	static CHRS apvchrs = {PAR_SET|REP_STR, 0, "PRIVAVL"};
	static CHRS ipvchrs = {PAR_SET|REP_STR, 0, "PRIVCUR"};
	static CHRS ascchrs = {PAR_SET|REP_STR, 0, "SECTAVL"};
	static CHRS iscchrs = {PAR_SET|REP_STR, 0, "SECTCUR"};
	static CHRS usrchrs = {PAR_SET|REP_STR, 0, "USER"};

	long rtn;

	if ((rtn = setchrs(&apvchrs, apvpnt)) < 0)
		return (rtn);
	if ((rtn = setchrs(&ipvchrs, ipvpnt)) < 0)
		return (rtn);
	if ((rtn = setchrs(&ascchrs, ascpnt)) < 0)
		return (rtn);
	if ((rtn = setchrs(&iscchrs, iscpnt)) < 0)
		return (rtn);
	usrchrs.item.buffer = username;
	return (svcIoClsChar("SESSION:", (char *)&usrchrs));
}


static long setchrs(
	CHRS  *chrs,
	uchar *val)

{
	uchar *pnt;
	uchar  bufr[4000];
	uchar  chr;

	if (val == NULL || val[0] == 0)
		return (0);
	pnt = bufr;
	while (pnt < (bufr + 3998))
	{
		if ((chr = *val++) == 0)
			break;
		if ((chr & 0x80) != 0)
		{
			*pnt++ = chr & 0x7F;
			*pnt++ = '+';
		}
		else
			*pnt++ = chr;
	}
	if (pnt >= (bufr + 3998))
		return (ER_DATTR);
	pnt[-1] = 0;
	chrs->item.buffer = bufr;
	return (svcIoClsChar("SESSION:", (char *)chrs));
}
