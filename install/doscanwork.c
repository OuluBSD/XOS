#include "install.h"
#include <xosrun.h>
#include <heapsort.h>

int   inscnt;

XWSWIN *btn1;
XWSWIN *btn2;

static char linei[] = "Select the program to install or update";
static char linen[] = "No installable programs were found";

///static char btext1[] = "F2-Backup & Install";
///static char btext2[] = "F3-Install Only";

static char btext1[] = "F2-Install";


static char diskcls[] = "DISK:";

char montbl[] = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec";

static QAB diqab =
{   QFNC_WAIT|QFNC_CLASSFUNC,	// func
    0,							// status
    0,							// error
    0,							// amount
    -1,							// handle
    0,							// vector
    {0},
    CF_PUNITS,					// option
    0,							// count
    &diskcls,					// buffer1
    NULL,						// buffer2
    NULL						// parm
};


long dirhndl;

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    ui8        end;
} dparms =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILEOPTN, XFO_DOSDEV},
    {PAR_GET|REP_STR , 0 , IOPAR_FILESPEC, NULL, 16, 16}
};

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE2PARM  srcattr;
    BYTE2PARM  filattr;
    BYTE4PARM  dirhndl;
    ui8        end;
} sparms =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILEOPTN, XFO_FILE},
    {PAR_GET|REP_STR , 0 , IOPAR_FILESPEC, NULL, 128, 128},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR, XA_FILE},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILEATTR, 0}
};


static void *allocmem(int size);
static void  scandone(int found);
static void getcurinfo(VBLK *vpnt, char *spec);
static char *getname(char *pnt, char *name);
static long  getval(char **pnt);
static char *getversion(char *pnt, long *num, int stopper);
static int   searchins(char *dev, char *srch);
static int   setupitem(char *dev, char *name);
static int   vdcmp(VBLK *vb1, VBLK *vb2);


       int   strxcmp(char *s1, char *s2, int len);


//***********************************************
// Function: doscanwork - Work thread function to
//				 find all installable programs
// Returned: Nothing
//***********************************************

// Each program found is added to the list in winlst. When finished, the
//   text in the main window is changed to reflect this.

void doscanwork(void)
{
	char *ptr;
	VBLK *vpnt;
	long  rtn;
	int   partnc;
	int   cnt;
	char  dosname[16];
	char  uname[256];
	char  dname[256];
	char  vtxt1[32];
	char  vtxt2[32];
	char  dtxt1[16];
	char  dtxt2[16];

	inscnt = 0;
	if ((rtn=svcIoQueue(&diqab)) < 0 || (rtn=diqab.error) < 0)
	{
		puterror(rtn, "Error obtaining list of disk names");
		diqab.amount = 0;
	}
	if ((numunits = diqab.count = diqab.amount) != 0)
	{
		cnt = (diqab.count + 1) * 8;
	    if ((diqab.buffer2 = unitnames = (char *)allocmem(cnt)) == NULL)
			return;
		memset(unitnames, 0, cnt);
    	if ((rtn = fbrIoFunc(&diqab)) < 0 || (rtn = diqab.error) < 0)
        	errormsg(rtn, "!Error getting disk unit names");

		// Sort the list of disk names

		partnc = FALSE;
		do
		{
			partnc = FALSE;
			cnt = numunits;
			ptr = unitnames;
			while (-- cnt > 0)			// Do this N - 1 times
			{
				if ((ptr[0] != ptr[8]) ? ((ptr[0] != 'F' && ptr[8] == 'F') ?
					TRUE : (ptr[0] != 'F' && ptr[0] > ptr[8])) :
					(strxcmp(ptr+1, ptr+9, 7) > 0))
				{
					*(llong *)uname = *(llong *)ptr;
					*(llong *)ptr = *(llong *)(ptr + 8);
					*(llong *)(ptr + 8) = *(llong *)uname;
					partnc = TRUE;
				}
				ptr += 8;
			}
		} while (partnc);

		// Here with the list of disk names sorted. Get the state of the disk
		//   This determines if the disk has a valid format and if XOS is
		//   installed on the disk.

		getstate();

		// Search the root directory of each disk for install files
		//   (xosinst-o-*.xis or xosinst-a-*.xis).

		dparms.filspec.buffer = dosname;
		sparms.filspec.buffer = dname;
		cnt = numunits;
		ptr = unitnames;
		do
		{
			rtn = sprintf(uname, "Scanning %.8s: ... Please wait", ptr);
			txtparms = xws_TxtParContainerDefault;
			txtparms.fnum = 1;
			txtparms.fheight = HEADSIZE;
			txtparms.lfmt = TLF_BCENTER;
			xwsWinTextNew(winw, 100, 2, TEXTYPOS, -2, 1, &txtparms,
					uname, rtn);
			sprintf(uname, "%.8s:\\", ptr);
			if ((dirhndl = fbrIoOpen(XO_ODFS, uname, &dparms)) < 0)
			{
				if (dirhndl != ER_NTRDY && dirhndl != ER_NTFIL &&
						dirhndl != ER_DIRNF && dirhndl != ER_FILNF)
					puterror(dirhndl, "Error scanning root of %.8s:", ptr);
			}
			else
			{
				if (dparms.filspec.strlen == 2)
				{
					ptr[0] = dosname[0];
					ptr[1] = 0;
				}
				searchins(ptr, "xosinst-*.xis");
				fbrIoClose(dirhndl, 0);
			}
			ptr += 8;
		} while (--cnt > 0);
	}
	if (vblkhead == NULL)
		scandone(FALSE);
	else
	{
		if (vblkhead->next != NULL)
			vblkhead = (VBLK *)heapsort(vblkhead,
					(int (*)(void *a, void *b, void *data))vdcmp, NULL);

		txtparms = xws_TxtParContainerDefault;
		txtparms.fheight = 5600;

		vpnt = vblkhead;
		do
		{
			if (!fmtdate(dtxt1, &vpnt->newdate))
				strmov(dtxt1, "???");
			if (!fmtver(vtxt1, &vpnt->newver))
				strmov(vtxt1, "???");
			if (!fmtdate(dtxt2, &vpnt->curdate))
			{
				strmov(dtxt2, "Not intalled");
				vtxt2[0] = 0;
			}
			else if (!fmtver(vtxt2, &vpnt->curver))
				vtxt2[0] = 0;
			vpnt->index = index;
			putline(COL_NORMAL, "%s%v%s%v%c%v%s%v%s%v%s%v%s%v%s", vpnt->vname,
					TF_HORPOS, POS_VENDOR, vpnt->pname,
					TF_HORPOS, POS_TYPE, toupper(vpnt->iset[8]),
					TF_HORPOS, POS_DEV, vpnt->dev, TF_HORPOS, POS_NDATE, dtxt1,
					TF_HORPOS, POS_NVER, vtxt1, TF_HORPOS, POS_CDATE, dtxt2,
					TF_HORPOS, POS_CVER, vtxt2);
			if (vpnt == vblkhead)
				xwsWinListSetSelItem(winlst, vpnt->index);
			vpnt = vpnt->next;
		} while (vpnt != NULL);
		xwsWinSetFocus(winlst, 0);
		scandone(TRUE);
	}
}


static void scandone(
	int found)
{
	winparms.butn = xws_WinParButtonDefault;
	txtparms = xws_TxtParButtonDefault;
	txtparms.fheight = 7000;

	// Always display the rescan and disk manager buttons

	disprescan();

	// If something installable was found, display the install buttons

	if (found)
	{
		winparms.butn.evarg1 = ISC_F2;
		xwsWinCreateButton(winw, BTNXPOS(1), mwsz.ysize - 20000, BTNWIDTH,
				12000, &winparms, &txtparms, buttonevent,
				XWS_EVMASK_WINDOW, &edb, &btn1, btext1, sizeof(btext1) - 1);

///		winparms.butn.evarg1 = ISC_F3;
///		xwsWinCreateButton(winw, BTNXPOS(2), mwsz.ysize - 20000, BTNWIDTH,
///				12000, &winparms, &txtparms, buttonevent,
///				XWS_EVMASK_WINDOW, &edb, &btn2, btext2, sizeof(btext2) - 1);
	}

	// Update the header line

	txtparms = xws_TxtParContainerDefault;
	txtparms.fnum = 1;
	txtparms.fheight = HEADSIZE;
	txtparms.lfmt = TLF_BCENTER;
	if (found)
		xwsWinTextNew(winw, 100, 2, TEXTYPOS, -2, 1, &txtparms, linei,
				sizeof(linei) - 1);
	else
		xwsWinTextNew(winw, 100, 2, TEXTYPOS, -2, 1, &txtparms, linen,
				sizeof(linen) - 1);
}



static int vdcmp(
	VBLK *vb1,
	VBLK *vb2)
{
	int  val;
	char type1;
	char type2;

	type1 = toupper(vb1->iset[8]);
	type2 = toupper(vb2->iset[8]);

	if (type1 != type2)
	{
		if (type1 == 'O')
			return (-11);
		if (type2 == 'O')
			return (1);
		return ((type2 == 'A') ? 1 : -1);
	}

	if ((val = stricmp(vb1->vname, vb2->vname)) != 0)
		return (val);
	return (stricmp(vb1->pname, vb2->pname));
}



static int searchins(
	char *dev,
	char *srch)
{
	char   *cpnt;
	VBLK   *vpnt;
	XIDHEAD header;
	VERNUM  newver;
	time_x  create;
	int     len;
	long    rtn;
	char    spec[FILENAME_MAX+20];
	char    vendor[NAMEMAX+4];
	char    program[NAMEMAX+4];
	char    dirname[160];
	char    crfile[300];

	while (TRUE)
	{
		if ((rtn = sysIoNextFile(dirhndl, srch, &sparms)) < 0)
		{
			if (rtn != ER_DIRNF && rtn != ER_FILNF)
			{
				puterror(rtn, "Error searching for install sets on %.8s:", dev);
				return (FALSE);
			}
			return (TRUE);
		}

		// Here with what might be an install set. Verify that the name is in
		//   the correct format and extract the vendor, name, and version
		//   number. The name has the following format:
		//     xosinst-x-ven-name-m-n-e.xis
		//     0123456789012345678901234567
		//   Where:
		//     x    - Install set type: "a" if application, "o" if operating
		//              system, or "d" if data
		//     ven  - Vendor name
		//     name - Install set name
		//     m    - Major version number
		//     n    - Minor version number
		//     e    - Edit number

///		if (setupitem(dev, sparms.filspec.buffer))
///			inscnt++;

		if (sparms.filspec.strlen < 22)
			continue;
		if ((cpnt = getname(sparms.filspec.buffer + 10, vendor)) == NULL)
			continue;					// Get the vendor name
		if ((cpnt = getname(cpnt, program)) == NULL) // Get the program
			continue;										//   name
		if ((cpnt = getversion(cpnt, &rtn, '-')) == NULL) // Get the major
			continue;									  //   version number
		newver.maj = (char)rtn;
		if ((cpnt = getversion(cpnt, &rtn, '-')) == NULL) // Get the minor
			continue;									  //   version number
		newver.min = (char)rtn;
		if ((cpnt = getversion(cpnt, &rtn, '.')) == NULL) // Get the edit
			continue;									  //   number
		newver.edit = (ushort)rtn;
		if (stricmp(cpnt, "xis") != 0)	// Make sure the end is right
			continue;

		// The XIS file name appears to be valid - Read and verify the header
		//   and get the date and time.

		sprintf(spec, "%.8s:\\%.*s", dev, FILENAME_MAX, sparms.filspec.buffer);
		if ((xishndl = fbrIoOpen(XO_IN, spec, NULL)) < 0)
		{
			puterror(xishndl, "Error opening %s", spec);
			continue;
		}
		rtn = fbrIoInBlock(xishndl, (char *)&header, sizeof(header));
		if (rtn != sizeof(header))
		{
			if (rtn >= 0)
				putline(COL_ERROR, "Illegal header format in %s", spec);
			else
				puterror(rtn, "Error reading %s", spec);
			continue;
		}
		if (*(long *)header.label != 'XISF' || header.fmtver != 2 ||
				header.hlen < 20)
		{
			putline(COL_ERROR, "Illegal header format in %s", spec);
			continue;
		}
		*(llong *)&create.sys = header.cdt;
		memset(&create.dos, 0, sizeof(create.dos));
		svcSysDateTime(10, &create);	// Get the broken down date

		if (!startinput())				// Set up to read the XIS file
			continue;

		if (!getitemhead())				// Read the header of the first item
			continue;

		if ((itemhead.flag & IF_TYPE) > IT_DTISET || itemhead.filelen != 0 ||
				itemhead.namelen > 126)
		{
			putline(COL_ERROR, "Illegal item format in %s", spec);
			continue;
		}
		strmov(dirname, outbufr);

		if (!getitemhead())				// Read the header of the next item
			continue;					//   which must be the primary file
										//   for the install set
		svcIoClose(xishndl, 0);
		if ((itemhead.flag & IF_TYPE) != IT_FILE || itemhead.namelen > 126)
		{
			putline(COL_ERROR, "Illegal item format in %s", spec);
			continue;
		}
		sprintf(crfile, "%s\\%s\\%s", tarprtn, dirname, outbufr);

		// Create the VBLK structure

		len = 0;		/// npnt - notes;
		if ((vpnt = (VBLK *)allocmem(sizeof(VBLK) + len)) == NULL)
			return (FALSE);

		strmov(vpnt->vname, vendor);
		strmov(vpnt->pname, program);
		vpnt->pname[NAMEMAX] = 0;

		strcpy(vpnt->iset, sparms.filspec.buffer);

		sprintf(vpnt->dev, "%.8s:", dev);
		vpnt->newver = newver;
		vpnt->newdate.year = create.dos.tmx_year;
		vpnt->newdate.month = create.dos.tmx_mon;
		vpnt->newdate.day = create.dos.tmx_mday;
		vpnt->needboot = TRUE;		// NEED TO FIX THIS!!

		// See if this install set is already installed and get the current
		//   date and version number. If the primary file is not present or
		//   if it contains invalid data we simply say the install set is
		//   not installed. This will set the version number in vpnt if a
		//   valid installed copy is found.

		getcurinfo(vpnt, crfile);

		// Link the VBLK structure to our list

		vpnt->next = vblkhead;
		vblkhead = vpnt;
	}
}



static void getcurinfo(
	VBLK *vpnt,
	char *spec)
{
	FILE    *file;
	char    *pnt;
	FILEINFO finfo;
	time_x   create;
	long     rtn;
	long     major;
	long     minor;
	long     editnum;
	int      len;
	char     bufr[sizeof(RUNHEAD) + 128];
	char     chr;

	vpnt->curdate.year = 0;
	vpnt->curver.maj = 0;
	if ((file = fopen(spec, "rb")) == NULL)
		return;
	if ((rtn = fgetfinfo(file, &finfo)) < 0)
	{
		fclose(file);
		return;
	}
	create.sys = finfo.modify;
 	memset(&create.dos, 0, sizeof(create.dos));
	svcSysDateTime(10, &create);	// Get the broken down date
	if ((pnt = strrchr(spec, '.')) == NULL)
	{
		fclose(file);
		return;
	}
	if (stricmp(pnt, ".run") == 0)
	{
		// Read the .run file header (We are probably reading more than we need
		//   but that is quicker than doing this in two steps to get the exact
		//   amount we need.)

		rtn = fread(bufr, 1, sizeof(RUNHEAD) + 64, file);
		fclose(file);
		if (rtn < 0)
			return;
		if ((rtn = ((RUNHEAD *)bufr)->versionpos) == 0)
			return;
		vpnt->curver.maj = ((VERDATA *)(bufr + rtn))->major;
		vpnt->curver.min = ((VERDATA *)(bufr + rtn))->minor;
		vpnt->curver.edit = ((VERDATA *)(bufr + rtn))->editnum;
	}
	else if (stricmp(pnt, ".txt") == 0)
	{
		// Read the first line of the text file

		pnt = fgets(bufr, sizeof(bufr), file);
		fclose(file);
		if (pnt == NULL)
			return;
		len = strlen(bufr);
		if (bufr[len - 1] == '\n')
			bufr[len - 1] = 0;

		if ((pnt = strchr(bufr, '|')) == NULL)
			return;
		pnt++;

		editnum = 0;
		major = strtol(pnt, &pnt, 10);
		if (*pnt++ != '-')
			return;
		minor = strtol(pnt, &pnt, 10);
		if ((chr = *pnt++) == '-')
		{
			editnum = strtol(pnt, &pnt, 10);
			chr = *pnt;
		}
		if (chr != 0 && !isspace(chr))
			return;
		vpnt->curver.maj = major;
		vpnt->curver.min = minor;
		vpnt->curver.edit = editnum;
	}
	else
		return;
	vpnt->curdate.day = create.dos.tmx_mday;
	vpnt->curdate.month = create.dos.tmx_mon;
	vpnt->curdate.year = create.dos.tmx_year;
}



static char *getname(
	char *pnt,
	char *name)
{
	int  cnt;
	char chr;

	cnt = NAMEMAX;
	while (--cnt > 0 && (chr = *pnt++) != 0 && chr != '-')
		*name++ = chr;
	*name = 0;
	if (chr != '-')
	{
		putline(COL_ERROR, "Illegal format name in %s",
				sparms.filspec.buffer);
		return (NULL);
	}
	return (pnt);
}


static char *getversion(
	char *pnt,
	long *pnum,
	int   stopper)
{
	long value;
	char chr;

	value = 0;
	while ((chr = *pnt++) != 0 && isdigit(chr))
		value = value * 10 + (chr & 0x0F);
	*pnum = value;
	if (chr != stopper)
	{
		putline(COL_ERROR, "Illegal version number in %s",
				sparms.filspec.buffer);
		return (NULL);
	}
	return (pnt);
}


//********************************************************
// Function: getrunver - Get version data from a .run file
// Returned: TRUE if OK, FALSE if error
//********************************************************

int getrunver(
	VERSN *versn,
	char  *spec,
	char  *vname,
	long   version,
	int    quiet)
{
	char    *vnamex;
	FILE    *file;
	FILEINFO finfo;
    VERDATA *vpnt;
	time_x   create;
	long     rtn;
	char     text[100];
	char     bufr[sizeof(RUNHEAD) + 64];

	if ((file = fopen(spec, "rb")) == NULL)
	{
		if (!quiet)
		{
			svcSysErrMsg(-errno, 0x02, text);
			puterror(-errno, "Error opening %s", spec);
		}
		return (FALSE);
	}

	// Get the .run file's creation date

	if ((rtn = fgetfinfo(file, &finfo)) < 0)
	{
		if (!quiet)
			putline(COL_ERROR, "Error getting date for %s", spec);
		fclose(file);
		return (FALSE);
	}
	create.sys = finfo.create;
	memset(&create.dos, 0, sizeof(create.dos));
	svcSysDateTime(10, &create);		// Get the broken down date

	// Read the .run file header (We are probably reading more than we need
	//   but that is quicker than doing this in two steps to get the exact
	//   amount we need.)

	rtn = fread(bufr, 1, sizeof(RUNHEAD) + 64, file);
	fclose(file);
	if (rtn < 0)
	{
		if (!quiet)
			putline(COL_ERROR, "Error reading file header for %s", spec);
		return (FALSE);
	}
	if ((rtn = ((RUNHEAD *)bufr)->versionpos) == 0)
	{
		if (!quiet)
			putline(COL_ERROR, "No version information available for %s", spec);
		return (FALSE);
	}
	vpnt = (VERDATA *)(bufr + rtn);
	if ((rtn = vpnt->vendorlen) > 63)
		rtn = 63;
	if (rtn > 0)
		memcpy(versn->vendor, vpnt->vendor, rtn);
	versn->vendor[rtn] = 0;
	versn->date.year = create.dos.tmx_year;
	versn->date.month = create.dos.tmx_mon;
	versn->date.day = create.dos.tmx_mday;
	versn->version.maj = vpnt->major;
	versn->version.min = vpnt->minor;
	versn->version.edit = vpnt->editnum;

	// Here with the version data from the .run file. See if it was complete
	//   and consistant.

	vnamex = (vname[0] != 0) ? vname : "system";

	if (stricmp(vnamex, versn->vendor) != 0)
	{
		if (!quiet)
			putline(COL_ERROR, "Inconsistant vendor names in %s\n", spec,
					vnamex, versn->vendor, spec);
		return (FALSE);
	}
	if (version != 0xFFFFFFFF && version != versn->version.l)
	{
		if (!quiet)
			putline(COL_ERROR, "Inconsistant version numbers for %s:\n"
					"      %08X / %08X", spec, version, versn->version.l);
		return (FALSE);
	}
	return (TRUE);
}


static void *allocmem(
	int size)
{
	void *pnt;

	if ((pnt = xwsMalloc(size)) == NULL)
	{
		putline(COL_FAIL, "Cannot allocate memory - CANNOT CONTINUE");
		return (NULL);
	}
	return (pnt);
}


//************************************************************
// Function: strxcmp - Compare two disk names so that variable
//				length numeric parts compare as expected
// Return: 1 if s1 > s2, 0 if s1 == s2, -1 if s1 < s2
//************************************************************

int strxcmp(
	char *s1,
	char *s2,
	int   len)
{
	int  d1;
	int  d2;
	char c1;
	char c2;

	while (--len >= 0)
	{
		c1 = *s1++;
		c2 = *s2++;
		if (c1 == c2)
		{
			if (c1 == 0)
				return (0);
		}
		else
		{
			if (c1 == 0)
				return ((c2 == 0) ? 0 : -1);
			if (c2 == 0)
				return (1);
			d1 = isdigit(c1);
			d2 = isdigit(c2);
			if (d1 != d2)
				return ((d1) ? 1 : -1);
			else
			{
				if (d1)
				{
					s1--;
					s2--;
					return ((getval(&s1) > getval(&s2)) ? 1 : -1);
				}
				else
					return ((c1 < c2)? 1 : -1);
			}
		}
	}
	return (0);
}


//***************************************************
// Function: getval - Get decimal value from a string
//				and advance pointer past the value
// Returned: Value
//***************************************************

static long getval(
	char **pnt)
{
	long value;
	char chr;

	value = 0;
	while ((chr = **pnt) != 0 && isdigit(chr))
	{
		(*pnt)++;
		value = value * 10 + (chr & 0x0F);
	}
	return (value);
}


int fmtdate(
	char *text,
	DATE *date)
{
	if (date->year == 0)
		return (FALSE);
	sprintf(text, "%d-%s-%d", date->day, montbl + (date->month - 1) * 4,
			date->year);
	return (TRUE);
}


int fmtver(
	char   *text,
	VERNUM *version)
{
	if (version->maj == 0 && version->min == 0)
		return (FALSE);
	sprintf(text, (version->edit == 0xFFFF) ? "%d.%d" : "%d.%d.%d",
			version->maj, version->min, version->edit);
	return (TRUE);
}
