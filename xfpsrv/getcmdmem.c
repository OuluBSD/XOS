//======================================
// GETCMDMEM.C
// Written by John Goltz
//======================================

#include <STDIO.H>
#include <STDDEF.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include <XOSSTR.H>
#include <XOSTIME.H>
#include <ERRMSG.H>
#include <XOSACCT.H>
#include <XOSERR.H>
#include <XOSSVC.H>
#include <PROCARG.H>
#include <ERRNO.H>
#include <XOSTHREADS.H>
#include <XOSSERVERT.H>
#include <MMS.H>
#include <MSG.H>

//****************************************************************
// Function: getcmdmem - Allocate permanent memory for ADD command
// Returned: Address of memory allocated or NULL if error
//****************************************************************

void *getcmdmem(
    int size)

{
    void *pnt;

    if ((pnt = sbrk(size)) == NULL)
    {
		srvCmdErrorResp(0, "Cannot allocate memory", NULL, srvMsgDst);
		return (NULL);
    }
    return (pnt);
}
