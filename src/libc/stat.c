#include <stdio.h>
#include <xos.h>
#include <xossvc.h>
#include <errno.h>
#include <stat.h>
#include <uniutil.h>

static struct
{	TIME8PARM atime;
	TIME8PARM ctime;
	TIME8PARM mtime;
	BYTE4PARM length;
	BYTE4PARM alloc;
	BYTE4PARM sectsize;
	BYTE1PARM filattr;
	BYTE1PARM srcattr;
	uchar     end;
} ioparms =
{	{PAR_GET|REP_DT  , 8, IOPAR_ADATE},
	{PAR_GET|REP_DT  , 8, IOPAR_CDATE},
	{PAR_GET|REP_DT  , 8, IOPAR_MDATE},
	{PAR_GET|REP_DECV, 4, IOPAR_LENGTH},
	{PAR_GET|REP_DECV, 4, IOPAR_REQALLOC},
	{PAR_GET|REP_DECV, 4, IOPAR_DSKSECTSIZE},
	{PAR_GET|REP_HEXV, 1, IOPAR_FILEATTR},
	{PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR, XA_FILE|XA_DIRECT}
};

static void storevalues(STAT *stp);


//*******************************************************
// Functin: stat - Return file information give file name
// Retuned: 0 if OK, -1 if error and errno is set
//*******************************************************

int stat(
	const char *name,
	STAT *stp)

{
	long rtn;

	ioparms.srcattr.desp = PAR_SET|REP_HEXV;
	if ((rtn = svcIoDevParm(0, (char *)name, (void *)&ioparms)) < 0)
	{
		errno = - rtn;
		return (-1);
	}
	storevalues(stp);
	return (0);
}


//*************************************************************
// Function: fstat - Return file information give device handle
// Returned: 0 if OK, -1 if error and errno is set
//*************************************************************

int fstat(
	long hndl,
	STAT *stp)

{
	long rtn;

	ioparms.srcattr.desp = 0;
	if ((rtn = svcIoInBlockP(hndl, NULL, 0, (char *)&ioparms)) < 0)
	{
		errno = - rtn;
		return (-1);
	}
	storevalues(stp);
	return (0);
}


//***************************************************
// Function: storevalues - Copy values from parameter
//				list to STAT structure
// Returned: Nothing
//***************************************************

static void storevalues(
	STAT *stp)

{
	stp->st_atime = __xosdt2unixdt(&ioparms.atime.value);
	stp->st_ctime = __xosdt2unixdt(&ioparms.ctime.value);
	stp->st_mtime = __xosdt2unixdt(&ioparms.mtime.value);
	stp->st_dev = 0;
	stp->st_uid = 0;					// We don't support Unix IDs!
	stp->st_gid = 0;
	stp->st_ino = 0;					// We don't use inodes
	stp->st_mode = (ioparms.filattr.value & XA_DIRECT) ? S_IFDIR : S_IFREG;
	stp->st_nlink = 1;					// We don't support links
	stp->st_size = ioparms.length.value;
	stp->st_blksize = ioparms.sectsize.value;
	stp->st_blocks = ioparms.alloc.value;
	stp->st_rdev = 0;					// We don't support Unix devic type val
}
