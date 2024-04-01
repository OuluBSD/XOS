#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"


int swcGetOpenFileSpec(
	char *title,
	char *inidir,
	char *filter,
	int   findex,
	char *defext,
	long  flags,
	char *buffer,
	int   size,
	long *foffset)

{
	OPENFILENAME ofn;
	int          rtn;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = findex;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = size;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = inidir;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = defext;
    ofn.lCustData = NULL;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;
	rtn = GetOpenFileName(&ofn);
	*foffset = ofn.nFileOffset;
	return (rtn);

}