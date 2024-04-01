//=================================================
// FILE.C - XFP file server file managment routines
// Written by John Goltz
//=================================================

#include "xfpsrv.h"


static int dofile(CDB *cdb, uchar *msg, int len);


//****************************************************
// Function: funcopen - Process the open file function
// Returned: Nothing
//****************************************************

void funcopen(
	CDB   *cdb,
	uchar *msg,
	int    len)

{
///	printf("### Function: OPEN\n");

	cdb->diskqab.qab_func = QFNC_OPEN;
	if (dofile(cdb, msg, len))
	{
		cdb->hndltbl[cdb->channel - 1] = cdb->diskqab.qab_handle;
		cdb->total[cdb->channel - 1] = 0;
		cdb->ignore[cdb->channel - 1] = FALSE;
	}
}


void funcparm(
	CDB   *cdb,
	uchar *msg,
	int    len)

{
///	printf("### Function: PARM\n");

	cdb->diskqab.qab_func = QFNC_DEVPARM;
	dofile(cdb, msg, len);
}


void funcclose(
	CDB   *cdb,
	uchar *msg,
	int    len)

{
///	printf("### Function: CLOSE\n");

	cdb->diskqab.qab_func = QFNC_CLOSE;
	dofile(cdb, msg, len);
}


void funccommit(
	CDB   *cdb,
	uchar *msg,
	int    len)

{
	cdb = cdb;
	msg = msg;
	len = len;
}


void funcdelete(
	CDB   *cdb,
	uchar *msg,
	int    len)

{
///	printf("### Function: DELETE\n");

	cdb->diskqab.qab_func = QFNC_DELETE;
	dofile(cdb, msg, len);
}


void funcrename(
	CDB   *cdb,
	uchar *msg,
	int    len)

{
///	printf("### Function: RENAME\n");

	cdb->diskqab.qab_func = QFNC_RENAME;
	dofile(cdb, msg, len);
}


static int dofile(
	CDB   *cdb,
	uchar *msg,
	int    len)

{
	long rtn;
	long chnl;
	long amount;
	int  namelen;
	int  renlen;

	if ((chnl = cdb->buffer[3] & 0x0F) > NUMHNDL)
	{
		sendnormrsp(cdb, ER_NPERR, 0, NULL);
		return (FALSE);
	}
	rtn = 0;
	if (chnl != 0 && cdb->hndltbl[chnl - 1] != 0)
	{
		if (srvDebugStream != NULL)
			debugout(cdb, "cls", "chnl=%d, handle=%d", chnl,
					cdb->hndltbl[chnl - 1]);
		rtn = thdIoClose(cdb->hndltbl[chnl - 1], 0);
		cdb->hndltbl[chnl - 1] = 0;
	}
	cdb->channel = chnl;
	if (cdb->diskqab.qab_func == QFNC_OPEN && cdb->channel == 0)
	{
		sendnormrsp(cdb, ER_NPERR, 0, NULL);
		return (FALSE);
	}
	if (cdb->diskqab.qab_func == QFNC_CLOSE)
	{
		if (rtn >= 0)
		{
			sendnormrsp(cdb, 0, rtn, NULL);
			return (TRUE);
		}
		else
		{
			sendnormrsp(cdb, rtn, 1, NULL);
			return (FALSE);
		}
	}
	cdb->diskqab.qab_amount = 0;
	cdb->diskqab.qab_parm = NULL;
	if ((len -= 4) < 0)					// Must have at least 4 bytes
	{
		sendnormrsp(cdb, ER_NPERR, 0, NULL);
		return (FALSE);
	}
	cdb->diskqab.qab_option = sl(*(long *)msg); // Get the command bits
	msg += 4;

	if (!getvalue(&msg, &len, &namelen))
	{
		sendnormrsp(cdb, ER_NPERR, 0, NULL);
		return (FALSE);
	}
	cdb->diskqab.qab_buffer1 = msg;
	msg += namelen;
	len -= namelen;
	if (cdb->diskqab.qab_func == QFNC_RENAME)
	{
		if (!getvalue(&msg, &len, &renlen))

		{
			sendnormrsp(cdb, ER_NPERR, 0, NULL);
			return (FALSE);
		}
		cdb->diskqab.qab_buffer2 = msg;
		msg += renlen;
		len -= renlen;
	}

///	printf("### name is OK, len = %d\n", namelen);

	if (len > 0 && (rtn = setupparms(cdb, msg, len)) < 0)
	{
		sendnormrsp(cdb, rtn, 0, NULL);
		return (FALSE);
	}

///	printf("### parameters are OK\n");

	cdb->diskqab.qab_buffer1[namelen] = 0;
	if (cdb->diskqab.qab_func == QFNC_RENAME)
		cdb->diskqab.qab_buffer2[renlen] = 0;
	if (srvDebugStream != NULL)
	{
		if (cdb->diskqab.qab_func == QFNC_RENAME)
			debugout(cdb, "nam", "%08X |%s| <- |%s|", cdb->diskqab.qab_option,
					cdb->diskqab.qab_buffer1, cdb->diskqab.qab_buffer2);
		else
			debugout(cdb, "nam", "%08X |%s|", cdb->diskqab.qab_option,
					cdb->diskqab.qab_buffer1);
	}
	amount = 0;
	do
	{
		if (cdb->fsleft > 0 && cdb->diskqab.qab_parm == NULL)
		{
			((lngstr_parm *)(cdb->parm))->desp = PAR_GET + REP_STR;
			((lngstr_parm *)(cdb->parm))->size = 0;
			((lngstr_parm *)(cdb->parm))->index = IOPAR_FILSPEC;
			((lngstr_parm *)(cdb->parm))->buffer = &cdb->strings;
			((lngstr_parm *)(cdb->parm))->bfrlen = 
					((lngstr_parm *)(cdb->parm))->strlen = cdb->fscnt;
			cdb->parm[16] = 0;
		}
		if ((rtn = thdIoFunc(&cdb->diskqab)) < 0 ||
				(rtn = cdb->diskqab.qab_error) < 0)
		{
			amount += cdb->diskqab.qab_amount;
			sendnormrsp(cdb, rtn, cdb->diskqab.qab_amount,
					(uchar *)(cdb->diskqab.qab_parm));
			return (FALSE);
		}
		cdb->fsleft -= cdb->fscnt;
		if (srvDebugStream != NULL)
			debugout(cdb, "fil", "chnl=%d, code=%d, amount=%d, handle=%d",
					chnl, cdb->diskqab.qab_error, cdb->diskqab.qab_amount,
					cdb->diskqab.qab_handle);		
		if ((cdb->diskqab.qab_amount & 0x80000000) == 0)
			cdb->fsleft = 0;
		amount += (cdb->diskqab.qab_amount & 0x3FFFFFFF);
		if (cdb->fsleft <= 0)
			amount |= (cdb->diskqab.qab_amount &0x80000000);
		if ((cdb->fscnt = 1200) > cdb->fsleft)
			cdb->fscnt = cdb->fsleft;

///		printf("### fsleft = %d, fscnt = %d\n", cdb->fsleft, cdb->fscnt);

		sendnormrsp(cdb, 0, (cdb->fsleft > 0) ? -1 : amount,
				(uchar *)(cdb->diskqab.qab_parm));
		cdb->diskqab.qab_parm = NULL;
	} while (cdb->fsleft > 0);
	return (TRUE);
}


int getvalue(
	uchar **pmsg,
	int    *plen,
	int    *pval)

{
	int val;

	if (*plen <= 0)
		val = 0;
	else
	{
		(*plen)--;
		val = *(*pmsg)++;
		if (val & 0x80)
		{
			if (--(*plen) < 0)
				return (FALSE);
			val = ((val & 0x7F) << 8) + *(*pmsg)++;
			if (val & 0x4000)
			{
				if (--(*plen) < 0)
					return (FALSE);
				val = ((val & 0x3FFF) << 8) + *(*pmsg)++;
				if (val & 0x200000)
				{
					if (--(*plen) < 0)
						return (FALSE);
					val = ((val & 0x1FFFFF) << 8) + *(*pmsg)++;
				}
			}
		}
	}
	*pval = val;
	return (TRUE);
}


uchar *putvalue(
	uchar *pmsg,
	int    val)

{
	if ((val & 0xFFFFFF80) != 0)
	{
		if ((val & 0xFFFFC000) == 0)
			*pmsg++ = (uchar)((val >> 8) | 0x80);
		else if ((val & 0xFFE00000) == 0)
		{
			*pmsg++ = (uchar)((val >> 16) | 0xC0);
			*pmsg++ = (uchar)(val >> 8);
		}
		else
		{
			if ((val & 0xF0000000) == 0)
				*pmsg++ = (uchar)((val >> 24) | 0xE0);
			else
			{
				*pmsg++ = 0xF0;
				*pmsg++ = (uchar)(val >> 24);
			}
			*pmsg++ = (uchar)(val >> 16);
			*pmsg++ = (uchar)(val >> 8);
		}
	}
	*pmsg++ = (uchar)val;
	return (pmsg);
}


typedef struct
{	ushort rep;
	ushort par;
} XFP2XOS;



XFP2XOS xfp2xos[] =
{	{0       , 0},					//				   = 0
	{REP_HEXV, IOPAR_FILOPTN},		// XFPPAR_FILOPTN  = 1
	{REP_STR , IOPAR_FILSPEC},		// XFPPAR_FILSPEC  = 2
	{REP_HEXV, IOPAR_DEVSTS},		// XFPPAR_DEVSTS   = 3
	{REP_HEXV, IOPAR_UNITNUM},		// XFPPAR_UNITNUM  = 4
	{REP_HEXV, IOPAR_GLBID},		// XFPPAR_GLBID    = 5
	{0       , 0},					//				   = 6
	{REP_HEXV, IOPAR_TIMEOUT},		// XFPPAR_TIMEOUT  = 7
	{REP_HEXV, IOPAR_INPSTS},		// XFPPAR_INPSTS   = 8
	{REP_HEXV, IOPAR_OUTSTS},		// XFPPAR_OUTSTS   = 9
	{0       , 0},					//				   = 10
	{0       , 0},					//				   = 11
	{REP_HEXV, IOPAR_DIRHNDL},		// XFPPAR_DIRHNDL  = 12
	{0       , 0},					//				   = 13
	{REP_HEXV, IOPAR_SRCATTR},		// XFPPAR_SRCATTR  = 14
	{REP_HEXV, IOPAR_FILATTR},		// XFPPAR_FILATTR  = 15
	{REP_HEXV, IOPAR_DIROFS},		// XFPPAR_DIROFS   = 16
	{REP_HEXV, IOPAR_ABSPOS},		// XFPPAR_ABSPOS   = 17
	{REP_HEXV, IOPAR_RELPOS},		// XFPPAR_RELPOS   = 18
	{REP_HEXV, IOPAR_EOFPOS},		// XFPPAR_EOFPOS   = 19
	{0       , 0},					//				   = 20
	{REP_HEXV, IOPAR_LENGTH},		// XFPPAR_LENGTH   = 21
	{REP_HEXV, IOPAR_REQALLOC},		// XFPPAR_REQALLOC = 22
	{REP_HEXV, IOPAR_RQRALLOC},		// XFPPAR_RQRALLOC = 23
	{REP_HEXV, IOPAR_GRPSIZE},		// XFPPAR_GRPSIZE  = 24
	{REP_HEXV, IOPAR_ADATE},		// XFPPAR_ADATE    = 25
	{REP_HEXV, IOPAR_CDATE},		// XFPPAR_CDATE    = 26
	{REP_HEXV, IOPAR_MDATE},		// XFPPAR_MDATE    = 27
	{REP_HEXV, IOPAR_PROT},			// XFPPAR_PROT     = 28
	{REP_STR , IOPAR_OWNER},		// XFPPAR_OWNNAME  = 29
	{0       , 0},					//				   = 30
	{REP_HEXV, IOPAR_SETLOCK},		// XFPPAR_SETLOCK  = 31
	{REP_HEXV, IOPAR_CLRLOCK},		// XFPPAR_CLRLOCK  = 32
	{REP_HEXV, IOPAR_SHRPARMS},		// XFPPAR_SHRPARMS = 33
	{0       , 0},			 		//				   = 34
	{0       , 0},			 		//				   = 35
	{0       , 0},			 		//				   = 36
	{0       , 0},			 		//				   = 37
	{0       , 0},			 		//				   = 38
	{0       , 0},			 		//				   = 39
	{0       , 0},			 		//				   = 40
	{0       , 0},			 		//				   = 41
	{0       , 0},			 		//				   = 42
	{0       , 0},			 		//				   = 43
	{0       , 0},			 		//				   = 44
	{0       , 0},			 		//				   = 45
	{0       , 0},			 		//				   = 46
	{0       , 0},			 		//				   = 47
	{0       , 0},			 		//				   = 48
	{0       , 0},			 		//				   = 49
	{0       , 0},			 		//				   = 50
	{0       , 0},			 		//				   = 51
	{0       , 0},			 		//				   = 52
	{0       , 0},			 		//				   = 53
	{0       , 0},			 		//				   = 54
	{0       , 0},			 		//				   = 55
	{0       , 0},			 		//				   = 56
	{0       , 0},			 		//				   = 57
	{0       , 0},			 		//				   = 58
	{0       , 0},			 		//				   = 59
	{0       , 0},			 		//				   = 60
	{0       , 0},			 		//				   = 61
	{0       , 0},			 		//				   = 62
	{0       , 0},			 		//				   = 63
	{0       , 0},			 		//				   = 64
	{0       , 0},			 		//				   = 65
	{0       , 0},			 		//				   = 66
	{0       , 0},			 		//				   = 67
	{0       , 0},			 		//				   = 68
	{0       , 0},			 		//				   = 69
	{0       , 0},			 		//				   = 70
	{0       , 0},			 		//				   = 71
	{0       , 0},			 		//				   = 72
	{0       , 0},			 		//				   = 73
	{0       , 0},			 		//				   = 74
	{0       , 0},			 		//				   = 75
	{0       , 0},			 		//				   = 76
	{0       , 0},			 		//				   = 77
	{0       , 0},			 		//				   = 78
	{0       , 0},			 		//				   = 79
	{0       , 0},			 		//				   = 80
	{0       , 0},			 		//				   = 81
	{0       , 0},			 		//				   = 82
	{0       , 0},			 		//				   = 83
	{0       , 0},			 		//				   = 84
	{0       , 0},			 		//				   = 85
	{0       , 0},			 		//				   = 86
	{0       , 0},			 		//				   = 87
	{0       , 0},			 		//				   = 88
	{0       , 0},			 		//				   = 89
	{0       , 0},			 		//				   = 90
	{0       , 0},			 		//				   = 91
	{0       , 0},			 		//				   = 92
	{0       , 0},			 		//				   = 93
	{0       , 0},			 		//				   = 94
	{0       , 0},			 		//				   = 95
	{REP_HEXV, IOPAR_DSKFSTYPE},	// XFPPAR_FSTYPE   = 96
	{REP_HEXV, IOPAR_DSKSECTSIZE},	// XFPPAR_SECTSIZE = 97
	{REP_HEXV, IOPAR_DSKCLSSIZE},	// XFPPAR_CLSSIZE  = 98
	{REP_HEXV, IOPAR_DSKTTLSPACE},	// XFPPAR_TTLSPACE = 99
	{REP_HEXV, IOPAR_DSKAVLSPACE},	// XFPPAR_AVLSPACE = 100
	{REP_HEXV, IOPAR_DSKNUMHEAD}, 	// XFPPAR_NUMHEAD  = 101
	{REP_HEXV, IOPAR_DSKNUMSECT},	// XFPPAR_NUMSECT  = 102
	{REP_HEXV, IOPAR_DSKNUMCYLN} 	// XFPPAR_NUMCYLN  = 103
};


//**************************************************************
// Function: setupparms - Set up a parameter list for an IO call
//				given the received XFP parameter list.
// Returned: Length of parameter list (positive) if OK or
//				negative XOS error code if error
//**************************************************************

long setupparms(
	CDB   *cdb,
	uchar *xpnt,
	int    xcnt)

{
	union
	{	uchar    *c;
		ushort   *s;
		ulong    *l;
		time_sz  *dt;
		char far **a;
	}      ppnt;
	uchar *tpnt;
	uchar *spnt;
	int    tcnt;
	int    cnt;
	int    ph2;
	int    pval;
	int    prep;
	int    xospar;

	ppnt.c = cdb->parm;					// Point to place for XOS parameters
	cdb->diskqab.qab_parm = cdb->parm;
	cdb->fsleft = 0;
	cdb->fscnt = 0;
	spnt = cdb->strings;				// Get pointer for strings
	while ((xcnt -= 2) >= 0)
	{
		if (ppnt.c >= (cdb->parm + PARMSZ))
			return (ER_NPERR);
		pval = *xpnt++;
		ph2 = *xpnt++;					// Get the function bits and 5 bits of
										//   the count
///		printf("### par:%02X hd:%02X\n", pval, ph2);

		if ((ph2 & 0xC0) == 0)
			return (ER_NPERR);
		cnt = ph2 & 0x1F;
		if (ph2 & 0x20)					// Need more count bits?
		{
			if (--xcnt < 0)				// Yes
				return (ER_NPERR);
			cnt = ((cnt && 0x1F) << 8) + *xpnt++;
		}
		if ((ph2 & 0x80) && cnt > xcnt)
			return (ER_NPERR);
		if (pval == IOPAR_FILSPEC)		// 
		{
			cdb->fsleft = cnt;
			if (cnt > 1200)
				cnt = 1200;
			cdb->fscnt = cnt;
		}

///		printf("### cnt = %d\n", cnt);

		if (pval > PARMSZ || (xospar = xfp2xos[pval].par) == 0)
			return (ER_NPERR);

		prep = xfp2xos[pval].rep;
		*ppnt.c++ = (ph2 & 0xC0) + prep; // Store first parameter byte

		if (prep >= REP_DATAB && prep != REP_TEXT) // String value?
		{
///			printf("### string\n");

			if (cnt > 1023 || (spnt + cnt) >= (cdb->strings + STRSZ - 1))
				return (ER_NPERR);
			*ppnt.c++ = 0;
			*ppnt.s++ = xospar;
			*ppnt.a++ = spnt;
			*ppnt.s++ = 0;
			*ppnt.s++ = cnt;
			*ppnt.s++ = cnt;
			if (ph2 & PAR_SET)
			{
				memcpy(spnt, xpnt, cnt);
				spnt[cnt] = 0;
				xpnt += cnt;
				xcnt -= cnt;
			}
			spnt += (cnt + 1);
		}
		else
		{
			if (cnt > 32)
				return (ER_NPERR);
			*ppnt.c++ = cnt;
			*ppnt.s++ = xospar;
			if (prep == REP_TEXT || (prep >= REP_DECB && prep <= REP_OCTB))
			{
///				printf("### in-line bytes\n");

				memcpy(ppnt.c, xpnt, cnt);
				ppnt.c += cnt;
				if (ph2 & PAR_SET)
				{
					xpnt += cnt;
					xcnt -= cnt;
				}
			}
			else
			{
				if (pval == XFPPAR_DIRHNDL)
				{
///					printf("### dirhndl\n");

					if (prep > REP_BINV || (ph2 & PAR_SET) == 0 ||
							(ph2 & PAR_GET) != 0)
						return (ER_NPERR);
					ppnt.c[-3] = 2;
					*ppnt.s++ = (ushort)(cdb->hndltbl[0]);
					xpnt += cnt;
					xcnt -= cnt;
				}
				else if (prep == REP_DT && cnt >= 12)
				{
					// Here if we have a full system date/time value with
					//   time zone and dlst offset.

///					printf("### date/time\n");

					if (cnt > 12)
						return (ER_NPERR);
					if (ph2 & PAR_SET)
					{


//qqqqqqqqqqqqq

						xpnt += cnt;
						xcnt -= cnt;
					}
					ppnt.c += cnt;
				}
				else 
				{
					// Here if have a general numeric value - This is
					//   harder since we have to convert from network
					//   byte order to system byte order.

///					printf("### numeric\n");

					if (cnt > 16)
						return (ER_NPERR);
					if (ph2 & PAR_SET)
					{
						tpnt = ppnt.c + cnt - 1;
						tcnt = cnt;
						while (--tcnt >= 0)
							*tpnt-- = *xpnt++;
						xcnt -= cnt;
					}
					ppnt.c += cnt;
				}
			}
		}
	}

///	printf("### done\n");

	*ppnt.c = 0;
	cnt = ppnt.c - cdb->parm;
	if (srvDebugStream != NULL)
		debugdump(cdb, "prm", cdb->parm, cnt);
	return (cnt);						// Return the lenght of the parameter
}										//   list created




uchar xos2xfpgen[] =
{	0,					//                   = 0x000
	XFPPAR_FILOPTN,		// IOPAR_FILOPTN     = 0x001
	XFPPAR_FILSPEC,		// IOPAR_FILSPEC     = 0x002
	XFPPAR_DEVSTS,		// IOPAR_DEVSTS      = 0x003
	XFPPAR_UNITNUM,		// IOPAR_UNITNUM     = 0x004
	XFPPAR_GLBID,		// IOPAR_GLBID       = 0x005
	0,					// IOPAR_DELAY       = 0x006
	0,					// IOPAR_TIMEOUT     = 0x007
	0,					// IOPAR_INPSTS      = 0x008
	0,					// IOPAR_OUTSTS      = 0x009
	0,					// IOPAR_INPQLMT     = 0x00A
	0,					// IOPAR_OUTQLMT     = 0x00B
	0,					// IOPAR_SIGVECT1    = 0x00C
	0,					// IOPAR_SIGVECT2    = 0x00D
	0,					// IOPAR_SIGDATA     = 0x00E
	0,					// IOPAR_NUMOPEN     = 0x00F
	0					// IOPAR_BUFRLMT     = 0x010
};
#define XOSGENMAX sizeof(xos2xfpgen)

uchar xos2xfpfil[] =
{	0,					//                   = 0x100
	XFPPAR_DIRHNDL,		// IOPAR_DIRHNDL     = 0x101
	XFPPAR_SRCATTR,		// IOPAR_SRCATTR     = 0x102
	XFPPAR_FILATTR,		// IOPAR_FILATTR     = 0x103
	XFPPAR_DIROFS,		// IOPAR_DIROFS      = 0x104
	XFPPAR_ABSPOS,		// IOPAR_ABSPOS      = 0x105
	XFPPAR_RELPOS,		// IOPAR_RELPOS      = 0x106
	XFPPAR_EOFPOS,		// IOPAR_EOFPOS      = 0x107
	0,					//                   = 0x108
	XFPPAR_LENGTH,		// IOPAR_LENGTH      = 0x109
	XFPPAR_REQALLOC,	// IOPAR_REQALLOC    = 0x10A
	XFPPAR_RQRALLOC,	// IOPAR_RQRALLOC    = 0x10B
	XFPPAR_GRPSIZE,		// IOPAR_GRPSIZE     = 0x10C
	XFPPAR_ADATE,		// IOPAR_ADATE       = 0x10D
	XFPPAR_CDATE,		// IOPAR_CDATE       = 0x10E
	XFPPAR_MDATE,		// IOPAR_MDATE       = 0x10F
	XFPPAR_PROT,		// IOPAR_PROT        = 0x110
	XFPPAR_OWNER,		// IOPAR_OWNNAME     = 0x111
	0,					//                   = 0x112
	XFPPAR_SETLOCK,		// IOPAR_SETLOCK     = 0x113
	XFPPAR_CLRLOCK,		// IOPAR_CLRLOCK     = 0x114
	0,					// IOPAR_CLSTIME     = 0x115
	0,					// IOPAR_CLSNAME     = 0x116
	0,					// IOPAR_CLSMSG      = 0x117
	XFPPAR_SHRPARMS		// IOPAR_SHRPARMS    = 0x118
};
#define XOSFILMAX sizeof(xos2xfpfil)

uchar xos2xfpdsk[] =
{	0,					//                   = 0x300
	XFPPAR_FSTYPE,		// IOPAR_DSKFSTYPE   = 0x301
	XFPPAR_SECTSIZE,	// IOPAR_DSKSECTSIZE = 0x302
	XFPPAR_CLSSIZE,		// IOPAR_DSKCLSSIZE  = 0x303
	XFPPAR_TTLSPACE,	// IOPAR_DSKTTLSPACE = 0x304
	XFPPAR_AVLSPACE,	// IOPAR_DSKAVLSPACE = 0x305
	XFPPAR_NUMHEAD,		// IOPAR_DSKNUMHEAD  = 0x306
	XFPPAR_NUMSECT,		// IOPAR_DSKNUMSECT  = 0x307
	XFPPAR_NUMCYLN		// IOPAR_DSKNUMCYLN  = 0x308
};
#define XOSDSKMAX sizeof(xos2xfpdsk)


typedef struct
{	uchar *pnt;
	long  *max;
} XOS2XFP;

XOS2XFP xos2xfp[] =
{	{xos2xfpgen, XOSGENMAX}, // IOPAR_xxxxxx = 00xx - Generic IO parameters
	{xos2xfpfil, XOSFILMAX}, // IOPAR_FILxxx = 01xx - File system IO parameters
	{NULL      , 0},		 // IOPAR_TRMxxx = 02xx - Terminal IO parameters
	{xos2xfpdsk, XOSDSKMAX}	 // IOPAR_DSKxxx = 03xx - Disk IO parameters
};
#define XOSMAX (sizeof(xos2xfp)/8)


//*******************************************************************
// Function: storeparms - Store IO parameters into a response message
// Returned: Pointer to next byte in message or NULL if error
//*******************************************************************

uchar *storeparms(
	CDB   *cdb,
	uchar *xpnt,
	uchar *ppnt)

{
	uchar *vpnt;
	uchar *pagpnt;
	int    len;
	int    vlen;
	int    hd1;
	int    rep;
	int    xinx;
	int    pinx;
	int    ppag;

	cdb = cdb;

	if (ppnt == NULL)
		return (xpnt);
	while ((hd1 = ppnt[0]) != 0)
	{
		rep = hd1 & 0x0F;
		if ((ppag = ppnt[3]) >= XOSMAX ||
				(pagpnt = xos2xfp[ppag].pnt) == NULL ||
				(pinx = ppnt[2]) > xos2xfp[ppag].max ||
				(xinx = pagpnt[pinx]) == 0)
			return (NULL);
		len = ppnt[1];					// Get length of value
		if (hd1 & (PAR_GET | PAR_ERROR))
		{
			*xpnt++ = xinx;				// Store XFP parameter index
			if (hd1 & PAR_GET)
			{
				if (rep >= REP_DATAB && rep != REP_TEXT)
				{
					// Here if have a string value

					vlen = *((ushort *)(ppnt + 14)) & 0x1FFF;
					vpnt = *(uchar **)(ppnt + 4);
					if (vlen > 0x3F)
					{
						*xpnt++ = ((hd1 & PAR_ERROR) ? 0xC0 : 0x40) |
								((vlen >> 8) & 0x3F);
						*xpnt++ = (uchar)vlen;
					}
					else
						*xpnt++ = ((hd1 & PAR_ERROR) ? 0x80 : 0x00) | vlen;
					while (--vlen >= 0)
						*xpnt++ = *vpnt++;
					ppnt += 16;
				}
				else
				{
					// Here if have an in-line value

					vlen = ppnt[1];
					if (vlen > 0x1F)
					{
						*xpnt++ = ((hd1 & PAR_ERROR) ? 0xA0 : 0x20) |
								((vlen >> 5) & 0x1F);
						*xpnt++ = (uchar)(vlen & 0x1F);
					}
					else
						*xpnt++ = ((hd1 & PAR_ERROR) ? 0x80 : 0x00) | vlen;
					ppnt += 4;
					if (rep <= REP_BINV || rep == REP_DATE || rep == REP_TIME ||
							(rep == REP_DT && len != 12))
					{
						// Here if have a numeric value

						vpnt = ppnt + vlen - 1;
						while (--vlen >= 0)
							*xpnt++ = *vpnt--;
						ppnt += len;
					}
					else if (rep == REP_DT && len == 12)
					{
						// Here if have a date/time value

						((time_nz *)ppnt)->high = sl(((time_sz *)xpnt)->high);
						((time_nz *)ppnt)->low = sl(((time_sz *)xpnt)->low);
						((time_nz *)ppnt)->tzone = sw(((time_sz *)xpnt)->tzone);
						((time_nz *)ppnt)->dlst = sw(((time_sz *)xpnt)->dlst);
						xpnt += 12;
						ppnt += 12;
					}
					else
					{
						// Here if value is a sequence of bytes

						while (--vlen >= 0)
							*xpnt++ = *ppnt++;
					}

				}
			}
			else
			{
				// Here if not getting the value but have an error

				*xpnt++ = 0x80;
			}
		}
		else
			// If if not returning the parameter - just skip it

			ppnt += ((rep >= REP_DATAB && rep != REP_TEXT) ? 16 : (len + 4));
	}
	return (xpnt);
}
