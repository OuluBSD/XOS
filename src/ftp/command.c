//--------------------------------------------------------------------------*
// COMMAND.C - XOS FTP server FTP command routines
//
// Written by: John Goltz
//--------------------------------------------------------------------------*

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

#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <xosnet.h>
#include <procarg.h>
#include <xosthreads.h>
#include <xosservert.h>
#include <xosudf.h>
#include <xosvfyuser.h>
#include <xosgetusersig.h>
#include <xosstr.h>
#include "ftpsrv.h"


static void dofilestore(CDB *cdb, char *pnt, long bits);
static void dodirectory(CDB *cdb, char *pnt, int func);


void docommand(
	CDB  *cdb,
	char *pnt)

{
	typedef struct
	{	char   name[6];
		uchar  needlogin;
		uchar  clsdata;
		void (*func)(CDB *cdb, char *pnt);
		
	} CMDTBL;

	static CMDTBL cmdtbl[] =
	{	{"USER", FALSE, TRUE , ftpcmduser},
		{"PASS", FALSE, TRUE , ftpcmdpass},
		{"ACCT", FALSE, TRUE , ftpcmdacct},
		{"QUIT", FALSE, TRUE , ftpcmdquit},
		{"HELP", FALSE, TRUE , ftpcmdhelp},
		{"NOOP", FALSE, TRUE , ftpcmdnoop},
		{"CWD" , TRUE , TRUE , ftpcmdcwd },
		{"CDUP", TRUE , TRUE , ftpcmdcdup},
		{"SMNT", TRUE , TRUE , ftpcmdsmnt},
		{"REIN", TRUE , TRUE , ftpcmdrein},
		{"PORT", TRUE , FALSE, ftpcmdport},
		{"PASV", TRUE , FALSE, ftpcmdpasv},
		{"TYPE", TRUE , TRUE , ftpcmdtype},
		{"STRU", TRUE , TRUE , ftpcmdstru},
		{"MODE", TRUE , TRUE , ftpcmdmode},
		{"RETR", TRUE , TRUE , ftpcmdretr},
		{"STOR", TRUE , TRUE , ftpcmdstor},
		{"STOU", TRUE , TRUE , ftpcmdstou},
		{"APPE", TRUE , TRUE , ftpcmdappe},
		{"ALLO", TRUE , TRUE , ftpcmdallo},
		{"REST", TRUE , TRUE , ftpcmdrest},
		{"RNFR", TRUE , TRUE , ftpcmdrnfr},
		{"RNTO", TRUE , TRUE , ftpcmdrnto},
		{"ABOR", TRUE , TRUE , ftpcmdabor},
		{"DELE", TRUE , TRUE , ftpcmddele},
		{"RMD" , TRUE , TRUE , ftpcmdrmd },
		{"MKD" , TRUE , TRUE , ftpcmdmkd },
		{"PWD" , TRUE , TRUE , ftpcmdpwd },
		{"LIST", TRUE , TRUE , ftpcmdlist},
		{"NLST", TRUE , TRUE , ftpcmdnlst},
		{"SITE", TRUE , TRUE , ftpcmdsite},
		{"SYST", TRUE , TRUE , ftpcmdsyst},
		{"STAT", TRUE , TRUE , ftpcmdstat},
		{"XAFL", TRUE , TRUE , ftpcmdxafl}
	};

	CMDTBL *tpnt;
	int     cnt;
	char    atom[16];

	if (srvDebugLevel >= 2)
		debug(cdb->slot, "CMD", 0, pnt);
	getatom(&pnt, atom, 16);
	strupr(atom);
	tpnt = cmdtbl;
	cnt = sizeof(cmdtbl)/sizeof(CMDTBL);
	do
	{
		if (strcmp(tpnt->name, atom) == 0)
		{
			if (tpnt->needlogin && !cdb->loggedin)
				cmdresponse(cdb, msg530);
			else
			{
				(tpnt->func)(cdb, pnt);
				if (tpnt->clsdata && cdb->datahndl > 0)
					datafunc(cdb, DATAFUNC_CLOSE);
			}
			return;
		}
		tpnt++;
	} while (--cnt > 0);
	cmdresponse(cdb, msg500a);
}


//****************************************************
// Function: ftpcmduser - Process the USER FTP command
// Returned: Nothing
//****************************************************

void ftpcmduser(
	CDB  *cdb,
	char *pnt)

{
	getatom(&pnt, cdb->username, sizeof(cdb->username));
	if (!requireend(cdb, pnt))
		cdb->username[0] = 0;
	else
		cmdresponse(cdb, msg331);
}


//****************************************************
// Function: ftpcmdpass - Process the PASS FTP command
// Returned: Nothing
//****************************************************

void ftpcmdpass(
	CDB  *cdb,
	char *pnt)

{
	char  *spnt;
	char  *dpnt;
	VUDATA vd;
	char   pswd[36];
	uchar  vubufr[1000];
	char   chr;

	if (cdb->idb->nologin)
	{
		cmdresponse(cdb, msg230c);
		return;
	}
	if (cdb->username[0] == 0)
	{
		cmdresponse(cdb, msg503a);
		return;
	}
	getatom(&pnt, pswd, sizeof(pswd));
	if (!requireend(cdb, pnt))
		return;
	if (!verifyuser(cdb->username, NULL, URQ1_PASSWORD|URQ1_HOMEDIR|
			URQ1_AVLPRIV|URQ1_INLPRIV, 0, FALSE, &vd, vubufr, 1000))
	{
		cmdresponse(cdb, msg503b);
		return;
	}
	if (vd.error1 != 0 || !verifypassword(pswd, strlen(pswd), NULL, NULL, &vd))
	{
		memset(pswd, 0, sizeof(pswd));
		cmdresponse(cdb, msg503b);
		return;
	}
	memset(pswd, 0, sizeof(pswd));
	cdb->loggedin = TRUE;

	if (checkpriv(vd.apvpnt, "FTPAL\xCC"))
		cdb->allfiles = (checkpriv(vd.ipvpnt, "FTPAL\xCC")) ? 2 : 1;

	if (vd.dirlen == 0 && cdb->allfiles == 0)
	{
		cmdresponse(cdb, msg503b);
		return;
	}
	if (vd.dirlen == 0)
	{
		cdb->homdirstr = xosrootstr;
		cdb->homdirlen = xosrootlen;
	}
	else
	{
		if ((cdb->homdirstr = malloc(vd.dirlen + 1)) == NULL)
		{
			cmdresponse(cdb, msg452);
			return;
		}
		dpnt = cdb->homdirstr;
		spnt = vd.dirpnt;
		do
		{
			if ((chr = *spnt++) == '\\')
				chr = '/';
			*dpnt++ = chr;
		} while (chr != 0);
		cdb->homdirlen = vd.dirlen;
	}
	switch (cdb->allfiles)
	{
	 default:
		cmdresponse(cdb, msg230a);
		return;

	 case 1:
		cmdresponse(cdb, msg230b);
		return;

	 case 2:
		cmdresponse(cdb, msg230c);
		return;
	}
}


//****************************************************
// Function: ftpcmdacct - Process the ACCT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdacct(
	CDB  *cdb,
	char *pnt)

{
	pnt = pnt;

	cmdresponse(cdb, msg202a);
}


//****************************************************
// Function: ftpcmdquit - Process the QUIT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdquit(
	CDB  *cdb,
	char *pnt)

{
	if (requireend(cdb, pnt))
		terminate(cdb);
}


//****************************************************
// Function: ftpcmdhelp - Process the HELP FTP command
// Returned: Nothing
//****************************************************

void ftpcmdhelp(
	CDB  *cdb,
	char *pnt)

{
	static DEFCSTR (msghelp  ,("214-Valid commands are: (* means valid without "\
			"login)\r\n" \
			"    USER* PASS* ACCT* QUIT* HELP* NOOP* CWD   CDUP  SMNT  REIN\r\n"\
			"    PORT  PASV  TYPE  STRU  MODE  RETR  STOR  STOU  APPE  ALLO\r\n"\
			"    REST  RNFR  RNTO  ABOR  DELE  RMD   MKD   PWD   LIST  NLST\r\n"\
			"    SITE  SYST  STAT  XAFL\r\n"\
			"214 End of help message\r\n"));

	pnt = pnt;

	cmdresponse(cdb, msghelp);
}


//****************************************************
// Function: ftpcmdnoop - Process the NOOP FTP command
// Returned: Nothing
//****************************************************

void ftpcmdnoop(
	CDB  *cdb,
	char *pnt)

{
	pnt = pnt;

	cmdresponse(cdb, msg200);
}


//**************************************************
// Function: ftpcmdcwd - Process the CWD FTP command
// Returned: Nothing
//**************************************************

void ftpcmdcwd(
	CDB  *cdb,
	char *pnt)

{
	char *path;
	int   len;

	if (*pnt != 0)
	{
		if ((len = getdirspec(cdb, pnt, cdb->specbufr, sizeof(cdb->specbufr)))
				< 0)
			return;
		if (len > 0)
		{
			if ((path = malloc(len + 1)) == NULL)
			{
				cmdresponse(cdb, msg452);
				return;
			}
			memcpy(path, cdb->specbufr, len);
			path[len] = 0;
		}
		else
			path = NULL;
	}
	else
	{
		len = 0;
		path = NULL;
	}
	if (cdb->curdirstr != NULL)
		free(cdb->curdirstr);
	cdb->curdirstr = path;
	cdb->curdirlen = len;
	tellworkingdir(cdb);
}


//****************************************************
// Function: ftpcmdcdup - Process the CDUP FTP command
// Returned: Nothing
//****************************************************

void ftpcmdcdup(
	CDB  *cdb,
	char *pnt)

{
	if (!requireend(cdb, pnt))
		return;
	ftpcmdcwd(cdb, "..");
}


//****************************************************
// Function: ftpcmdsmnt - Process the SMNT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdsmnt(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdresponse(cdb, msg202b);
}


//****************************************************
// Function: ftpcmdrein - Process the REIN FTP command
// Returned: Nothing
//****************************************************

void ftpcmdrein(
	CDB  *cdb,
	char *pnt)

{
	if (!requireend(cdb, pnt))
		return;
	if (cdb->homdirstr != NULL)
	{
		free(cdb->homdirstr);
		cdb->homdirstr = NULL;
		cdb->homdirlen = 0;
	}
	if (cdb->curdirstr != NULL)
	{
		free(cdb->curdirstr);
		cdb->curdirstr = NULL;
		cdb->curdirlen = 0;
	}
	if (cdb->rnfrstr != NULL)
	{
		free(cdb->rnfrstr);
		cdb->rnfrstr = NULL;
	}
	if (cdb->idb->nologin)
		cmdresponse(cdb, msg220d);
	else
	{
		cdb->loggedin = FALSE;
		cdb->allfiles = 0;
		cmdresponse(cdb, msg220c);
	}
}


//****************************************************
// Function: ftpcmdport - Process the PORT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdport(
	CDB  *cdb,
	char *pnt)

{
	union
	{	long  v;
		uchar c[4];
	} addr;
	union
	{   ushort v;
		uchar  c[2];
	} port;

	if (!getbyteval(&pnt, &addr.c[0], ',') ||
			!getbyteval(&pnt, &addr.c[1], ',') ||
			!getbyteval(&pnt, &addr.c[2], ',') ||
			!getbyteval(&pnt, &addr.c[3], ',') ||
			!getbyteval(&pnt, &port.c[1], ',') ||
			!getbyteval(&pnt, &port.c[0], 0))
	{
		cmdresponse(cdb, msg500b);
		return;
	}
	cdb->rmtdataport.v = port.v;
	cmdresponse(cdb, msg200);
}


//****************************************************
// Function: ftpcmdpasv - Process the PASV FTP command
// Returned: Nothing
//****************************************************

void ftpcmdpasv(
	CDB  *cdb,
	char *pnt)

{
	cstr msg;
	IDB *idb;
	char bufr[100];

	if (!requireend(cdb, pnt))
		return;
	datafunc(cdb, DATAFUNC_PASV);
	datawait(cdb);
	if (cdb->datadone < 0)
	{
		xoserror(cdb, cdb->datadone, "425 Error opening passive data "
				"connection");
		return;
	}
	idb = cdb->idb;
	msg.len = sprintf(bufr, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)."
			"\r\n", idb->ipaddr.c[0], idb->ipaddr.c[1], idb->ipaddr.c[2],
			idb->ipaddr.c[3], cdb->lcldataport.c[1], cdb->lcldataport.c[0]);
	msg.str = bufr;
	cmdresponse(cdb, msg);
}


//****************************************************
// Function: ftpcmdtype - Process the TYPE FTP command
// Returned: Nothing
//****************************************************

// Note: The type command is implemented, but it does nothing!  Since
//       our internal representation for files is the same as the NVT
//       format, no conversion is needed for text files.  We do not
//       support EBCDIC.  Also, since XOS does not internally support
//       carriage control, this information is irrelavent.  We do
//       check for valid arguments as follows:
//			Type:	A, I, and L are valid, E is not.  L requires a byte
//					size of 8.
//			Format:	N, T, and C are valid.

void ftpcmdtype(
	CDB  *cdb,
	char *pnt)

{
	cstr msg;
	char atom1[4];
	char atom2[4];
	char bufr[100];

	atom2[1] = 0;
	getatom(&pnt, atom1, 4);
	getatom(&pnt, atom2, 4);
	atom1[0] = toupper(atom1[0]);
	atom2[0] = toupper(atom2[0]);
	if (!requireend(cdb, pnt))
		return;
	if (atom1[1] == 0 && atom2[1] == 0 &&
			((atom1[0] == 'I' && atom2[0] == 0) ||
				(atom1[0] == 'A' && (atom2[0] == 0 || atom2[0] == 'N' ||
					atom2[0] == 'T' || atom2[0] == 'C')) ||
				(atom1[0] == 'L' && atom2[0] == '8')))
	{
		msg.str = bufr;
		msg.len = sprintf(bufr, "200 Representation set to %s\r\n",
				(atom1[0] == 'A') ? "ASCII" : (atom1[0] == 'I') ?
				"Image" : "Local 8");
		cmdresponse(cdb, msg);
	}
	else
		cmdresponse(cdb, msg500b);
}


//****************************************************
// Function: ftpcmdstru - Process the STRU FTP command
// Returned: Nothing
//****************************************************

void ftpcmdstru(
	CDB  *cdb,
	char *pnt)

{
	char atom[4];

	getatom(&pnt, atom, 4);
	atom[0] = toupper(atom[0]);
	cmdresponse(cdb, (atom[1] != 0 || (atom[0] != 'F' && atom[0] != 'R' &&
			atom[0] != 'P')) ? msg501b : (atom[0] != 'F') ? msg504a : msg200);
}


//****************************************************
// Function: ftpcmdmode - Process the MODE FTP command
// Returned: Nothing
//****************************************************

void ftpcmdmode(
	CDB  *cdb,
	char *pnt)

{
	char atom[4];

	getatom(&pnt, atom, 4);
	atom[0] = toupper(atom[0]);
	cmdresponse(cdb, (atom[1] != 0 || (atom[0] != 'S' && atom[0] != 'B' &&
			atom[0] != 'C')) ? msg501b : (atom[0] != 'S') ? msg504a : msg200);
}


//****************************************************
// Function: ftpcmdretr - Process the RETR FTP command
// Returned: Nothing
//****************************************************

void ftpcmdretr(
	CDB  *cdb,
	char *pnt)

{
	char spec[300];
	struct
	{	byte4_parm  optn;
		lngstr_parm spec;
		time8_parm  mdate;
		uchar       end;
	}    foparms =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_NOPREFIX|XFO_FILENAME},
		{PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, 0, sizeof(spec),
				sizeof(spec)},
		{PAR_GET|REP_DT  , 8, IOPAR_MDATE},
		0
	};
	cstr msg;
	int  status;
	long rtn;
	char bufr[400];	
	char mdate[64];

	if ((rtn = getfilespec(cdb, &pnt, cdb->specbufr, sizeof(cdb->specbufr) - 4,
			&status)) < 0 || !requireend(cdb, pnt))
		return;
	if (rtn == 0 || cdb->specbufr[rtn - 1] == ':' ||
			cdb->specbufr[rtn - 1] == '/')
	{
		cmdresponse(cdb, msg550b);
		return;
	}
	if (srvDebugLevel > 0)
		debug(cdb->slot, "DTA", 0, "Opening file %s for RETR", cdb->specbufr);
	foparms.spec.buffer = spec;
	if ((rtn = thdIoOpen(XO_IN, cdb->specbufr, &foparms)) < 0)
	{
		xoserror(cdb, rtn, "550 Error opening file");
		return;
	}
	cdb->diskhndl = rtn;
	sdt2str(mdate, "%l%r%d%h%m%s", (time_sz *)&(foparms.mdate.value));
	msg.str = bufr;
	msg.len = sprintf(bufr, "150 Opening data connection for file %s (%s)\r\n",
			spec, mdate);
	cmdresponse(cdb, msg);
	datafunc(cdb, DATAFUNC_RETR);
	datawait(cdb);
	if (cdb->datadone < 0)
	{
		xoserror(cdb, cdb->datadone, (cdb->dataerror != NULL) ?
				cdb->dataerror : "550 Error retrieving file");
		return;
	}
	cmdresponse(cdb, msg250a);
}


//****************************************************
// Function: ftpcmdstor - Process the STOR FTP command
// Returned: Nothing
//****************************************************

void ftpcmdstor(
	CDB  *cdb,
	char *pnt)

{
	dofilestore(cdb, pnt, XO_OUT|XO_CREATE|XO_TRUNCA);
}


//****************************************************
// Function: ftpcmdappe - Process the APPE FTP command
// Returned: Nothing
//****************************************************

void ftpcmdappe(
	CDB  *cdb,
	char *pnt)

{
	dofilestore(cdb, pnt, XO_OUT|XO_CREATE|XO_APPEND);
}


//****************************************************
// Function: ftpcmdstou - Process the STOU FTP command
// Returned: Nothing
//****************************************************

void ftpcmdstou(
	CDB  *cdb,
	char *pnt)

{
	dofilestore(cdb, pnt, XO_OUT|XO_CREATE|XO_TRUNCA|XO_UNQNAME);
}


static void dofilestore(
	CDB  *cdb,
	char *pnt,
	long  bits)

{
	char spec[300];
	struct
	{	byte4_parm  optn;
		lngstr_parm spec;
		time8_parm  cdate;
		time8_parm  mdate;
		uchar       end;
	}    foparms =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_FILOPTN, XFO_NOPREFIX|XFO_FILENAME},
		{PAR_GET|REP_STR , 0, IOPAR_FILSPEC, NULL, 0, sizeof(spec),
				sizeof(spec)},
		{PAR_SET|REP_DT  , 8, IOPAR_CDATE},
		{PAR_SET|REP_DT  , 8, IOPAR_MDATE},
		0
	};
	cstr msg;
	int  status;
	long rtn;
	char bufr[400];	

	if ((rtn = getfilespec(cdb, &pnt, cdb->specbufr, sizeof(cdb->specbufr) - 4,
			&status)) < 0 || !getfiledatetime(cdb, &pnt) ||
			!requireend(cdb, pnt))
		return;
	if (rtn == 0 || cdb->specbufr[rtn - 1] == ':' ||
			cdb->specbufr[rtn - 1] == '/')
	{
		cmdresponse(cdb, msg550b);
		return;
	}
	if (cdb->filedt.high != 0)
	{
		foparms.cdate.desp = PAR_SET|REP_DT;
		foparms.cdate.value = *(time_s *)&(cdb->filedt);
		foparms.mdate.value = *(time_s *)&(cdb->filedt);
	}
	else
		foparms.cdate.desp = 0;
	if (srvDebugLevel > 0)
		debug(cdb->slot, "DTA", 0, "Opening file %s for STOR", cdb->specbufr);
	foparms.spec.buffer = spec;
	if ((rtn = thdIoOpen(bits, cdb->specbufr, &foparms)) < 0)
	{
		xoserror(cdb, rtn, "550 Error opening file");
		return;
	}
	cdb->diskhndl = rtn;
	msg.str = bufr;
	msg.len = sprintf(bufr, "150 Opening data connection for file %s\r\n",
			spec);
	cmdresponse(cdb, msg);
	datafunc(cdb, DATAFUNC_STOR);
	datawait(cdb);
	if (cdb->datadone < 0)
	{
		xoserror(cdb, cdb->datadone, (cdb->dataerror != NULL) ?
				cdb->dataerror : "550 Error retrieving file");
		return;
	}
	cmdresponse(cdb, msg250a);
}


//****************************************************
// Function: ftpcmdallo - Process the ALLO FTP command
// Returned: Nothing
//****************************************************

void ftpcmdallo(
	CDB  *cdb,
	char *pnt)

{
	pnt = pnt;

	cmdresponse(cdb, msg202b);
}


//****************************************************
// Function: ftpcmdrest - Process the REST FTP command
// Returned: Nothing
//****************************************************

void ftpcmdrest(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdresponse(cdb, msg502);
}


//****************************************************
// Function: ftpcmdrnfr - Process the RNFR FTP command
// Returned: Nothing
//****************************************************

void ftpcmdrnfr(
	CDB  *cdb,
	char *pnt)

{
	long rtn;
	int  status;

	if (cdb->rnfrstr != NULL)
	{
		free(cdb->rnfrstr);
		cdb->rnfrstr = NULL;
	}
	if ((rtn = getfilespec(cdb, &pnt, cdb->specbufr, sizeof(cdb->specbufr) - 4,
			&status)) < 0 || !requireend(cdb, pnt))
		return;
	if (status & 0x02)
	{
		cmdresponse(cdb, msg550c);
		return;
	}
	if (rtn == 0 || cdb->specbufr[rtn - 1] == ':' ||
			cdb->specbufr[rtn - 1] == '/')
	{
		cmdresponse(cdb, msg550b);
		return;
	}
	if ((cdb->rnfrstr = malloc(rtn + 1)) == NULL)
	{
		cmdresponse(cdb, msg452);
		return;
	}
	memcpy(cdb->rnfrstr, cdb->specbufr, rtn);
	cdb->rnfrstr[rtn] = 0;
	cmdresponse(cdb, msg350);
}


//****************************************************
// Function: ftpcmdrnto - Process the RNTO FTP command
// Returned: Nothing
//****************************************************

void ftpcmdrnto(
	CDB  *cdb,
	char *pnt)

{
	long rtn;
	int  status;

	if (cdb->rnfrstr == NULL)
	{
		cmdresponse(cdb, msg503c);
		return;
	}
	if ((rtn = getfilespec(cdb, &pnt, cdb->specbufr, sizeof(cdb->specbufr) - 4,
			&status)) < 0 || !requireend(cdb, pnt))
		return;
	if (status & 0x02)
	{
		cmdresponse(cdb, msg550c);
		return;
	}
	if (rtn == 0 || cdb->specbufr[rtn - 1] == ':' ||
			cdb->specbufr[rtn - 1] == '/')
	{
		cmdresponse(cdb, msg550b);
		return;
	}
	rtn = thdIoRename(0, cdb->specbufr, cdb->rnfrstr, NULL);
	free(cdb->rnfrstr);
	cdb->rnfrstr = NULL;
	if (rtn < 0)
	{
		xoserror(cdb, rtn, "550 Error renaming file");
		return;
	}
	cmdresponse(cdb, msg250d);
}


//****************************************************
// Function: ftpcmdabor - Process the ABOR FTP command
// Returned: Nothing
//****************************************************

void ftpcmdabor(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdresponse(cdb, msg502);
}


//****************************************************
// Function: ftpcmddele - Process the DELE FTP command
// Returned: Nothing
//****************************************************

void ftpcmddele(
	CDB  *cdb,
	char *pnt)

{
	long rtn;
	int  status;

	if ((rtn = getfilespec(cdb, &pnt, cdb->specbufr, sizeof(cdb->specbufr) - 4,
			&status)) < 0 || !requireend(cdb, pnt))
		return;
	if (status & 0x02)
	{
		cmdresponse(cdb, msg550c);
		return;
	}
	if (rtn == 0 || cdb->specbufr[rtn - 1] == ':' ||
			cdb->specbufr[rtn - 1] == '/')
	{
		cmdresponse(cdb, msg550b);
		return;
	}

	if ((rtn = thdIoDelete(0, cdb->specbufr, NULL)) < 0)
	{
		xoserror(cdb, rtn, "550 Error deleting file");
		return;
	}
	cmdresponse(cdb, msg250c);
}


//**************************************************
// Function: ftpcmdrmd - Process the RMD FTP command
// Returned: Nothing
//**************************************************

void ftpcmdrmd(
	CDB  *cdb,
	char *pnt)

{
	cstr msg;
	long rtn;
	char bufr[650];
	struct
	{	byte1_parm attr;
		uchar      end;
	} delparms =
	{	{PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR, XA_DIRECT}
	};

	if ((rtn = getdirspec(cdb, pnt, cdb->specbufr, sizeof(cdb->specbufr))) < 0)
		return;
	if (rtn == 0)
	{
		cmdresponse(cdb, msg500c);
		return;
	}
	cdb->specbufr[rtn - 1] = 0;			// Remove final "/"
	if ((rtn = thdIoDelete(0, cdb->specbufr, &delparms)) < 0)
		xoserror(cdb, rtn, "550 Directory not removed");
	else
	{
		msg.str = bufr;
		msg.len = sprintf(bufr, "200 \"%s\" directory removed\r\n",
				cdb->specbufr);
		cmdresponse(cdb, msg);
	}
}


//**************************************************
// Function: ftpcmdmkd - Process the MKD FTP command
// Returned: Nothing
//**************************************************

void ftpcmdmkd(
	CDB  *cdb,
	char *pnt)

{
	cstr msg;
	long rtn;
	char bufr[650];

	if ((rtn = getdirspec(cdb, pnt, cdb->specbufr, sizeof(cdb->specbufr))) < 0)
		return;
	if (rtn == 0)
	{
		cmdresponse(cdb, msg500c);
		return;
	}
	if ((rtn = thdIoDevParm(XO_ODF|XO_CREATE|XO_FAILEX, cdb->specbufr,
			NULL)) < 0)
		xoserror(cdb, rtn, "550 Directory not created");
	else
	{
		msg.str = bufr;
		msg.len = sprintf(bufr, "257 \"%.600s\" directory created\r\n",
				cdb->specbufr);
		cmdresponse(cdb, msg);
	}
}


//****************************************************
// Function: ftpcmdpwd - Process the PWD FTP command
// Returned: Nothing
//****************************************************

void ftpcmdpwd(
	CDB  *cdb,
	char *pnt)

{
	if (!requireend(cdb, pnt))
		return;
	tellworkingdir(cdb);
}


//****************************************************
// Function: ftpcmdlist - Process the LIST FTP command
// Returned: Nothing
//****************************************************

void ftpcmdlist(
	CDB  *cdb,
	char *pnt)

{
	dodirectory(cdb, pnt, DATAFUNC_LIST);
}


//****************************************************
// Function: ftpcmdnlst - Process the NLST FTP command
// Returned: Nothing
//****************************************************

void ftpcmdnlst(
	CDB  *cdb,
	char *pnt)

{
	dodirectory(cdb, pnt, DATAFUNC_NLST);
}


static void dodirectory(
	CDB  *cdb,
	char *pnt,
	int   func)

{
	int  status;
	long rtn;

	if ((rtn = getfilespec(cdb, &pnt, cdb->specbufr, sizeof(cdb->specbufr) - 4,
			&status)) < 0 || !requireend(cdb, pnt))
		return;
	if (rtn == 0 || cdb->specbufr[rtn - 1] == ':' ||
			cdb->specbufr[rtn - 1] == '/')
		strcpy(cdb->specbufr + rtn, "*.*");		

	if (srvDebugLevel > 0)
		debug(cdb->slot, "DTA", 0, "Opening directory %s for search",
				cdb->specbufr);

	if ((rtn = thdIoOpen(XO_ODF, cdb->specbufr, NULL)) < 0)
	{
		xoserror(cdb, rtn, "550 Error opening directory for search");
		return;
	}
	cdb->diskhndl = rtn;
	cmdresponse(cdb, msg150a);
	datafunc(cdb, func);
	datawait(cdb);
	if (cdb->datadone < 0)
	{
		xoserror(cdb, cdb->datadone, (cdb->dataerror != NULL) ?
				cdb->dataerror : "550 Error listing direcory");
		return;
	}
	cmdresponse(cdb, msg250b);
}


//****************************************************
// Function: ftpcmdsite - Process the SITE FTP command
// Returned: Nothing
//****************************************************

void ftpcmdsite(
	CDB  *cdb,
	char *pnt)

{
	pnt = pnt;

	cmdresponse(cdb, msg202c);
}


//****************************************************
// Function: ftpcmdsyst - Process the SYST FTP command
// Returned: Nothing
//****************************************************

void ftpcmdsyst(
	CDB  *cdb,
	char *pnt)

{
	cstr msg;
	char bufr[64];

	if (!requireend(cdb, pnt))
		return;
	msg.str = bufr;
	msg.len = sprintf(bufr, "215 XOS ver %u.%u.%u\r\n", xosver.major,
			xosver.minor, xosver.editnum);
	cmdresponse(cdb, msg);
}


//****************************************************
// Function: ftpcmdstat - Process the STAT FTP command
// Returned: Nothing
//****************************************************

void ftpcmdstat(
	CDB  *cdb,
	char *pnt)

{
	cdb = cdb; pnt = pnt;

	cmdresponse(cdb, msg502);
}


//****************************************************
// Function: ftpcmdxafl - Process the XAFL FTP command
// Returned: Nothing
//****************************************************

void ftpcmdxafl(
	CDB  *cdb,
	char *pnt)

{
	cstr msg;
	char bufr[128];

	if (cdb->allfiles == 0)
	{
		cmdresponse(cdb, msg533);
		return;
	}
	if (!requireend(cdb, pnt))
		return;
	cdb->allfiles ^= 0x03;

	if (cdb->curdirstr != NULL)
	{
		free(cdb->curdirstr);
		cdb->curdirstr = NULL;
		cdb->curdirlen = 0;
	}
	msg.str = bufr;
	msg.len = sprintf(bufr, "200 All-files mode %s, working directory "
			"reset\r\n", (cdb->allfiles == 1) ? "disabled" : "enabled");
	cmdresponse(cdb, msg);
}
