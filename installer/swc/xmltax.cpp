#include "common.h"
#include <atlbase.h>
#include "msg_cus.h"
#include "qbcom.h"



// Element table for the CustomerQueryRs element

xitem cusqrs[] =
{	{"ItemSalesTaxRet", NULL, dontcare, NULL, NULL, 0},
///	{"ItemServiceRet", NULL, dontcare, NULL, NULL, 0},
///	{"ItemIventoryRet", NULL, dontcare, NULL, NULL, 0},
///	{"ItemItemOtherChargeRet", NULL, dontcare, NULL, NULL, 0},
///	{"ItemDiscountRet", NULL, dontcare, NULL, NULL, 0},
	{ (char *)-1      , NULL  dontcare, NULL, NULL, 0};
	{ NULL}
};


int havecusrsbgn(
	int data)

{
	qbrsptype = QBRSPTYPE_CUS;
	return (0);
}


int havecusrsend(
	int   data,
	char *text)

{

	return (0);
}


static int havecusretbgn(
	int   data)

{
	if (cuscur != NULL)
		return (XMLERR_ELERR);
	cuscur = new cusdata();
	memset(cuscur, 0, sizeof(cusdata));
	return (0);
}


static int havecusretend(
	int   data,
	char *text)

{
	if (cuscur == NULL)
		return (XMLERR_ELERR);
	*cuslast = cuscur;
	cuslast = & cuscur->next;
	cuscur = NULL;
	return (0);
}


static int cusacct(
	int   data,
	char *text)

{
	if (strlen(text) > (sizeof(cuscur->account) - 1))
		return (XMLERR_ELERR);
