#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"


swcRawPrint::swcRawPrint(
	char *prtname,
	char *docname)

{
	DOC_INFO_1 docinfo;
	DWORD      job;

	if (!OpenPrinter(prtname, &hp, NULL))
	{
		hp = 0;
		return;
	}
	docinfo.pDocName = docname;
	docinfo.pOutputFile = NULL;
	docinfo.pDatatype = "RAW";
	if ((job = StartDocPrinter(hp, 1, (uchar *)&docinfo )) == 0)
	{
		ClosePrinter(hp);
		hp = 0;
		return;
	}
	pageact = false;
	docact = false;
}


swcRawPrint::~swcRawPrint()

{
	if (hp != 0)
	{
		if (pageact)
			EndPagePrinter(hp);
		if (docact)
			EndDocPrinter(hp);
		ClosePrinter(hp);
		hp = 0;
	}
}


int swcRawPrint::BeginPage()

{
	if (hp == 0)
		return (false);
	if (!StartPagePrinter(hp))
	{
		EndDocPrinter(hp);
		ClosePrinter(hp);
		hp = 0;
		pageact = docact = false;
		return (false);
	}
	pageact = true;
	return (true);
}


int swcRawPrint::EndPage()

{
	if (hp == 0)
		return (false);
	pageact = false;
	if (!EndPagePrinter(hp))
	{
		EndDocPrinter(hp);
		ClosePrinter(hp);
		docact = false;
		hp = 0;
		return (false);
	}
	return (true);
}


int swcRawPrint::PutData(
	char *data,
	int   len)

{
	DWORD rtnval;

	if (hp == 0)
		return (false);
	if (!WritePrinter(hp, data, len, &rtnval) || ((uint)len) != rtnval)
	{
		EndPagePrinter(hp);
		EndDocPrinter(hp);
		ClosePrinter(hp);
		hp = 0;
		return (false);
	}
	return (true);
}


int swcRawPrint::CancelPrint()

{
	return (false);
}
