#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "xosstuff.h"
#include "swcwindows.h"
#include "util.h"
#include "sizetbl.h"
#include "mms.h"
#include "msg.h"
#include "msg_trn.h"
#include "msg_inv.h"
#include "msg_tkt.h"


static char *getcdatom(char *pnt, char *atom, int len);


//**************************************************************
// Function: getsizecode - Converts size string to SMS size code
// Returned: SMS size code
//**************************************************************

int getsizecode(
	char *size)

{
	SIZETABLE *spnt;
	int        cnt;

	spnt = sizetbl;
	cnt = sizetblsz;
	do
	{
		if (stricmp(size, spnt->name) == 0)
			return (spnt->nnum);
		spnt++;
	} while (--cnt > 0);
	return (0);
}

char *getsizelbl(
	int size)

{
	SIZETABLE *spnt;
	int        cnt;

	static char text[16];

	spnt = sizetbl;
	cnt = sizetblsz;
	do
	{
		if (size == spnt->nnum)
			return (spnt->name);
		if (size < spnt->nnum)
			break;
		spnt++;
	} while (--cnt > 0);
	sprintf(text, "#%d", size);
	return (text);
}


char *gettrntypelbl(
	int type,
	int linetype)

{
	static char text[16];

	switch(type)
	{
	 case MSGTRN_VALUE_TYPE_SALE:
		return ("Sale");

	 case MSGTRN_VALUE_TYPE_STORE:
		return ("Store");

	 case MSGTRN_VALUE_TYPE_PINVADJ:
		return ("PInv");

	 case MSGTRN_VALUE_TYPE_MANADJ:
		return ("MInv");

	 case MSGTRN_VALUE_TYPE_SHPDDIST:
		return ("Shp-D");

	 case MSGTRN_VALUE_TYPE_SHPADJ:
		return ("ShpAdj");

	 case MSGTRN_VALUE_TYPE_RCVDDIST:
		return ("Rcv-D");

	 case MSGTRN_VALUE_TYPE_RCVDVEND:
		return ("Rcv-V");

	 case MSGTRN_VALUE_TYPE_RCVDDISTADJ:
		return ("RcvDistAdj");

	 case MSGTRN_VALUE_TYPE_RCVDVENDADJ:
		return ("RcvVendAdj");

	 case MSGTRN_VALUE_TYPE_XFRDOUT:
		return ("X-Out");

	 case MSGTRN_VALUE_TYPE_XFRDOUTADJ:
		return ("X_OutAdj");

	 case MSGTRN_VALUE_TYPE_XFRDIN:
		return ("X-In");

	 case MSGTRN_VALUE_TYPE_XFRDINADJ:
		return ("X_InAdj");

	 default:
		sprintf(text, "%d", type);
		return (text);
	}
}


void getstddate(
	char  *text,
	int    longyear,
	struct tm *tm)

{
	char daystr[8];
	char monstr[16];
	char yrstr[8];


	strftime(daystr, 8, "%d", tm);
	strftime(monstr, 16, "%B", tm);
	strftime(yrstr, 8, (longyear) ? "%Y" : "%y", tm);
	sprintf(text, "%s:%3s:%s", daystr, monstr, yrstr);
}

char *getaddtypelbl(
	int addtype)

{
	static char text[16];

	switch (addtype)
	{
	 case INVADD_PO:
		return ("PO");

	 case INVADD_RCV:
		return ("RCV");

	 case INVADD_XIN:
		return ("XIN");

	 case INVADD_XINR:
		return ("XINR");

	 case INVADD_XOUT:
		return ("XOUT");

	 case INVADD_SALE:
		return ("SALE");

	 case INVADD_RTN:
		return ("RTN");

	 case INVADD_SADJ:
		return ("SADJ");

	 case INVADD_PINV:
		return ("PINV");

	 case INVADD_MAN:
		return ("MAN");

	 default:
		printf(text, "%d", addtype);
		return (text);
	}
}

//***********************************************************
// Function: parsecreditcarddata - Parse the credit card data
// Returned: Nothing
//***********************************************************

// Format of the credit card data is:
//   number|name|address|zip|exp|auth

void parsecreditcarddata(
	char           *carddata,
	creditcarddata *ccdata)

{
	carddata = getcdatom(carddata, ccdata->number, sizeof(ccdata->number));
	carddata = getcdatom(carddata, ccdata->name, sizeof(ccdata->name));
	carddata = getcdatom(carddata, ccdata->address, sizeof(ccdata->address));
	carddata = getcdatom(carddata, ccdata->zip, sizeof(ccdata->zip));
	carddata = getcdatom(carddata, ccdata->exp, sizeof(ccdata->exp));
	if (strlen(ccdata->exp) == 4)
	{
		ccdata->exp[5] = 0;
		ccdata->exp[4] = ccdata->exp[3];
		ccdata->exp[3] = ccdata->exp[2];
		ccdata->exp[2] = '/';
	}
	carddata = getcdatom(carddata, ccdata->auth, sizeof(ccdata->auth));
}


static char *getcdatom(
	char *pnt,
	char *atom,
	int   len)

{
	char chr;

	while ((chr = *pnt) != 0)
	{
		pnt++;
		if (chr == '|')
			break;
		if (--len > 0)
			*atom++ = chr;
	}
	*atom = 0;
	return (pnt);
}

int getcreditcardtype(
	char *number)

{
	typedef struct
	{	int   nlen;
		int   plen;
		int   pmin;
		int   pmax;
		int   type;
	} CCTYPES;
	static CCTYPES cctypes[] =
	{	{	16, 2,   51,   55, TKT_TENDER_CCMC},
		{	13, 1,    4,    4, TKT_TENDER_CCVISA},
		{	16, 1,    4,    4, TKT_TENDER_CCVISA},
		{	15, 2,   34,   34, TKT_TENDER_CCAMEX},
		{	15, 2,   37,   37, TKT_TENDER_CCAMEX},
		{	14, 3,  300,  305, TKT_TENDER_CCDINER},
		{	14, 2,   36,   36, TKT_TENDER_CCDINER},
		{	14, 2,   38,   38, TKT_TENDER_CCDINER},
		{	16, 4, 6011, 6011, TKT_TENDER_CCDISC}
	};

	CCTYPES *ccpnt;
	char    *pnt;
	long     value;
	int      cclen;
	int      cnt1;
	int      cnt2;
	char     chr;

	pnt = number;
	while ((chr = *pnt) != 0 && isdigit(chr))
		pnt++;
	cclen = pnt - number;
	ccpnt = cctypes;
	cnt1 = sizeof(cctypes)/sizeof(CCTYPES);
	do
	{
		if (cclen == ccpnt->nlen)
		{
			cnt2 = ccpnt->plen;
			value = 0;
			pnt = number;
			do
			{
				value = value * 10 + (*pnt++ & 0x0F);
			} while (--cnt2 > 0);
			if (value >= ccpnt->pmin && value <= ccpnt->pmax)
				return (ccpnt->type);
		}
		ccpnt++;
	} while (--cnt1 > 0);
	return (TKT_TENDER_CCOTHER);
}
