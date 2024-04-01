#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "xosstuff.h"
#include "swcwindows.h"

static swcDebug *dbwin = NULL;
static int       txtpos = 1;

swcDebug::swcDebug() :
	swcTopLevel(NULL, "Debug text", "swcDEBUG", 0, 0, 121, 125, 0, 0x000000,
			SWC_TTLBAR_FULL | SWC_BORDER_FRM_SM)

{
	dbwin = this;
}

swcDebug::~swcDebug()

{
	dbwin = NULL;
}


void swcDebug::Text(
	char *msg, ...)

{
	va_list     pi;
	struct tm   tm;
    swcDateTime dt;
	char        text[300];

	if (dbwin != NULL)
	{
	    va_start(pi, msg);
		dt.GetLocal();
		dt.ConvToDos(&tm);
		strftime(text, sizeof(text), "%H:%M:%S ", &tm);
    	vsprintf(text + 9, msg, pi);
		dbwin->PutText(1, txtpos, 100, 0, 0x00FF00, FONT_SMFIXED, TXT_LEFT,
				text);
		txtpos += 3;
		dbwin->VertScrollRange(txtpos + 1, 3);
		dbwin->VertScrollPosition(txtpos + 1);
	}
}

