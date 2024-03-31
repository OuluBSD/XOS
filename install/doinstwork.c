#include "install.h"
#include <dirscan.h>
#include <zlib.h>

static char brkline[] = "=================================================="
		"=========================\n";

extern ulong crctable[256];

// dirscan data

z_stream stream;

char dsbufr[512];

struct
{   BYTE4PARM  filoptn;
    LNGSTRPARM filspec;
    BYTE2PARM  srcattr;
    BYTE2PARM  filattr;
    char       end;
} fileparm =
{   {PAR_SET|REP_HEXV, 4 , IOPAR_FILEOPTN, XFO_DOSDEV|XFO_NODENUM|XFO_NODENAME|
            XFO_RDOSDEV|XFO_PATH|XFO_FILE},
    {PAR_GET|REP_STR , 0 , IOPAR_FILESPEC, dsbufr, 512, 512},
    {PAR_SET|REP_HEXV, 2 , IOPAR_SRCATTR, XA_FILE},
    {PAR_GET|REP_HEXV, 2 , IOPAR_FILEATTR, 0}
};

struct
{	BYTE8PARM cdate;
	BYTE8PARM mdate;
	char      end;
} dtparms =
{	{PAR_SET|REP_HEXV, 8, IOPAR_CDATE},
	{PAR_SET|REP_HEXV, 8, IOPAR_MDATE}
};

static void dserrmsg(const char *arg, long  code);
static int  dsdofile(void);

DIRSCANDATA dsd =
{	(DIRSCANPL *)&fileparm,	// parmlist - Address of parameter list
	(PROCFILE *)dsdofile,
							// func     - Function called for each file matched
	dserrmsg,				// error    - Function called on error
	DSSORT_ASCEN			// sort     - Directory sort order
};

int   filecnt;
int   speclen;

char *dstspec;
char *srcspec;
char *srcpath;
char *pname;

char  newinst;
char  appinst;
char  needboot;

char  ispec[200];		// File spec for install source directory
///char  dspec[200];		// File spec for install destination directory
char  bspec[200];		// FIle spec for backup destination directory
char  dirspec[200];
char  xisspec[200];

#if __DISKMAN
static char initmsg[] = "The system disk does not appear to be correctly "
		"initialized for use with XOS. If you believe this is not correct "
		"you should cancel this operation and call customer support. If you "
		"proceed all data now on the system disk \x11\x08\x02\x11\x1A\xFF"
		"\x01\x01WILL BE ERASED\x11\x08\x01\x11\x1A\x01\x01\x01! To proceed "
		"and initialize the system disk enter \"ERASE\" (all upper case "
		"without the quotes) in the field below and press <ENTER>. To cancel "
		"and not change the contents of the system disk, press <ESC>.";
#endif

static void dirdone(void);

static int  makedir(char *spec, int ask);
static void mkbtfail(char *str1, long code, char *str2);
static void mkbtnotice(char *str);


void doinstwork(void)
{
	time_s curdt;
	char  *devend;
	char  *baseend;
	char  *dirend;
	long   rtn;
	long   amnt;
	long   filehndl;
	int    itype;
	char   vtxt[16];
	char   ttxt[64];
	char   text[200];
	char   title[128];
	char   base[650];

	// Note that the dir item in VBLK is not really necessary since it could
	//   be derived from the name items. We store it anyway since this keeps
	//   the code that parses the name in one place (scan.c) and we don't
	//   really care about the extra memory needed.

	if (logfile == NULL &&
			(logfile = fopen("XOSACT:install.log", "a+")) == NULL)
		puterror(-errno, "Error opening XOSACT:install.log, this sequence "
				"will not be logged");
	newinst = (vsel->curver.maj == 0);
	if (!fmtver(vtxt, &vsel->newver))
		strmov(vtxt, "???");
	sprintf(title, "%s %s_%s %s", (newinst) ? "Installing" : "Updating",
			vsel->vname, vsel->pname, vtxt);
	if (toupper(vsel->iset[8]) != 'O')	// Installing OS?
	{									// No - installing an application
		appinst = TRUE;					// Yes
		if (!havexos)					// Is XOS installed?
		{
			continueinst = FALSE;		// No - can't procceed with this
			putline(COL_FAIL, "The operating system is not installed. It "
					"must be installed before installing applications");
		}
		else
		{
			// This code makes the following asumptions about the structure of
			//   an application:
			//   1) All application executables are stored in the directory
			//      XOSDEV:\XOSAPP\ven\ where "ven" is the vendor name.
			//   2) The main executable for the application is name.run ("name"
			//      is the name of the program).
			//   3) All common data files associated with the application are
			//      stored in the directory XOSDEV:\xosdata\ven\name, where
			//      "ven" is the vendor name and "name" is the application
			//      name. This directory may contain the data files or may
			//      contain subdirectories, etc.

			// When a backup of an application is created, a directory named
			//   D0P1:\BCK\ven\name_yyyy_mm_dd is created and all files and
			//      directories matching D0P1:\APP\ven\name*.* are copied to it.
			// Note that there is no restriction (within reason) on the length
			//   of the "ven" and "name" strings. The "ven" and "name" strings
			//	 must NOT contain the "_" character.
			// This convention provides a minimum number of directores that
			//   must be searched to load an application but still keeps the
			//   applcations distinct.

		}
	}
	else								// If installing the OS
	{
		if (tarprtn[0] == 0)			// Do we have a partition available?
		{								// No
#if __DISKMAN
			doinit(initmsg, sizeof(initmsg) - 1, vsel); // No
										// The initialize routine will restart
										//   this proceedure when it is
#else									//   complete.
			continueinst = FALSE;		// No - can't procceed with this
			putline(COL_FAIL, "There is not disk available to install to");
#endif
			return;
		}
		appinst = FALSE;
		needboot = vsel->needboot;		// Remember if need to install bootstrap
		if (didinit)
		{
			txtparms = xws_TxtParContainerDefault;
			txtparms.fnum = 1;
			txtparms.fheight = HEADSIZE;
			txtparms.lfmt = TLF_BCENTER;
			xwsWinTextNew(winw, 100, 2, TEXTYPOS, -2, 1, &txtparms,
					insttitle, insttitlelen);
		}
	}
///	sprintf(ispec, "%s\\xosinstall\\%.176s\\", vsel->dev, vsel->dir);
	if (dobackup)
	{
		svcSysDateTime(1, (time_sz *)&curdt);
		sdt2str(text,"%l-%r-%d", (time_sz *)&curdt);
		sprintf(bspec, "D0P1:\\bck\\%s\\%s_%s\\", (appinst) ? vsel->vname :
				"os", vsel->pname, text);
	}
	else
		bspec[0] = 0;
///	xwsFree(vsel);						// We are finished with the VBLK now

	// Back up the current version if that was requested

#if 0
	if (continueinst && bspec[0] != 0)
	{
		if (logfile != NULL)
			fputs(brkline, logfile);

		if (newinst)
			putline(COL_WARN, "There is nothing to back up");
		else
		{
			svcSysDateTime(1, (time_sz *)&curdt);
			sdt2str(ttxt,"%z%H:%m:%s on %D-%3n-%l", (time_sz *)&curdt);
			putline(COL_NOSEL, "Backing up current version of %s at %s",
					pname, ttxt);
			if (logfile != NULL)
				fputs(brkline, logfile);

			pnt = strchr(strchr(strchr(bspec, '\\') + 1, '\\') + 1, '\\');
			len = pnt - bspec + 1;		// Find 2nd backslash
			memcpy(text, bspec, len);	// Get spec for the first BCK directory
			text[len] = 0;
			if (!makedir(text, FALSE))	// Create the directory if it's not
				continueinst = FALSE;	//   there
			else
			{
				if (!makedir(bspec, TRUE)) // Create the next directory if it's
					continueinst = FALSE;  //   there
				else
				{
					continueinst = (continueinst) ? copyfiles(bspec, dspec,
							TRUE) : FALSE;
					svcSysDateTime(1, (time_sz *)&curdt);
					sdt2str(ttxt, "%z%H:%m:%s on %D-%3n-%l", (time_sz *)&curdt);
					putline((continueinst) ? COL_GOOD : COL_FAIL, "Backup %s "
							"complete at %s", (continueinst) ? "is" :
							"did not", ttxt);
				}
			}
		}
	}
#endif

	// Install the new version

	while (continueinst)				// Not really a loop - Uses "break"
	{									//   as a way to branch to the end!
		if (logfile != NULL)
			fputs(brkline, logfile);
		svcSysDateTime(1, (time_sz *)&curdt);
		sdt2str(ttxt, "%z%H:%m:%s on %D-%3n-%l", (time_sz *)&curdt);
		putline(COL_NOSEL, "%s at %s", insttitle, ttxt);
		if (logfile != NULL)
			fputs(brkline, logfile);

		// Open the XIS file

		sprintf(xisspec, "%s\\%s", vsel->dev, vsel->iset);
		if ((xishndl = fbrIoOpen(XO_IN, xisspec, NULL)) < 0)
		{
			puterror(xishndl, "Error opening XIS file %s", xisspec);
			continueinst = FALSE;
			break;
		}

		// Read the XIS header

		if ((rtn = fbrIoInBlock(xishndl, (char *)&xishead,
				sizeof(XIDHEAD))) < 0)
		{
			puterror(rtn, "Error reading XIS file header for %s", xisspec);
			continueinst = FALSE;
			break;
		}
		if (*(long *)xishead.label != 'XISF')
		{
			putline(COL_ERROR, "Incorrect label value in XIS file header "
					"for %s", xisspec);
			continueinst = FALSE;
			break;
		}
		if (xishead.hlen < 12)
		{
			putline(COL_ERROR, "XIS file header is too short in %s", xisspec);
			continueinst = FALSE;
			break;
		}
		if (xishead.fmtver != 2)
		{
			putline(COL_ERROR, "XIS format version is not 2 in %s", xisspec);
			continueinst = FALSE;
			break;
		}
		if (xishead.hlen > 12 && (rtn = svcIoSetPos(xishndl, xishead.hlen + 8,
				0)) < 0)
		{
			puterror(rtn, "Error setting position in XIS file %s, xispec");
			continueinst = FALSE;
			break;
		}

		if (!startinput())
		{
			continueinst = FALSE;
			break;
		}
		devend = strmov(base, tarprtn);

///		putline(COL_DEBUG, "Tarprtn: %s", tarprtn);

		// When get here we are ready to start reading the compressed items

		while (continueinst)
		{
			itemnum++;

///			offset = totalsize;

			if (!getitemhead())
			{
				continueinst = FALSE;
				break;
			}

			// Here with the complete header and name

			if ((itype = itemhead.flag & IF_TYPE) == IT_END) // Finished?
				break;
			dtparms.cdate.value = itemhead.cdt;
			dtparms.mdate.value = itemhead.cdt;
			if (itype < IT_SBDIR && itype != IT_BSTRP) // Is this a root level
			{										   //   directory?
///				putline(COL_DEBUG, "RTDIR: %s", outbufr);

				baseend = strmov(devend, outbufr);
				*baseend++ = '\\';
				*baseend = 0;
				dirend = baseend;
				continueinst = makedir(base, FALSE);
			}
			else if (itype == IT_SBDIR)	// Is this a sub-directory?
			{
///				putline(COL_DEBUG, "SBDIR: %s", outbufr);

				dirend = strmov(baseend, outbufr);
				*dirend++ = '\\';
				*dirend = 0;
				continueinst = makedir(base, FALSE);
			}
			else if (itype == IT_FILE)	// Is this a file?
			{
				// Here with a file to copy

///				putline(COL_DEBUG, "FILE: %s", outbufr);

				strmov(dirend, outbufr);
				if ((filehndl = fbrIoOpen(XO_CREATE|XO_TRUNCA|XO_IN|XO_OUT,
						base, &dtparms)) < 0)
				{
					puterror(filehndl, "Error creating file %s", base);
					continueinst = FALSE;
					break;
				}
				sdt2str(ttxt, "%Z%h:%m:%s %D-%3n-%l", (time_sz *)&itemhead.cdt);
				putline(COL_NORMAL | TEMP, "Copying file %s", base);
				do
				{
					if ((amnt = itemhead.filelen) > OUTBUFRSZ)
						amnt = OUTBUFRSZ;
					if ((rtn = getinput(outbufr, amnt)) < 0)
					{
						puterror(filehndl, "Error expanding data for file %s",
								base);
						fbrIoClose(filehndl, 0);
						continueinst = FALSE;
						break;
					}
					if ((rtn = fbrIoOutBlock(filehndl, outbufr, amnt)) < 0)
					{
						puterror(filehndl, "Error writing data to file %s",
								base);
						fbrIoClose(filehndl, 0);
						continueinst = FALSE;
						break;
					}
					itemhead.filelen -= amnt;
				} while (itemhead.filelen > 0);
				fbrIoClose(filehndl, 0);
			}
			else
			{
				putline(COL_ERROR, "Invalid item type in XIS file %s", xisspec);
				continueinst = FALSE;
				break;
			}
		}
		if (continueinst)
		{
			if (crcvalue != xishead.crcvalue)
			{
				putline(COL_ERROR, "Incorrect CRC value for XIS file %s",
						xisspec);
				continueinst = FALSE;
			}
			else if (totalsize != xishead.totalsize)
			{
				putline(COL_ERROR, "Incorrect length (%,d/%,d) for XIS "
						"file %s", totalsize, xishead.totalsize, xisspec);
				continueinst = FALSE;
			}
		}
		if (continueinst && !appinst && needboot)
		{
			long maxrsvd;
			char fstype[16];

			sprintf(base, "%s\\%s\\sys\\boot.run", tarprtn, vsel->pname);
			sprintf(outbufr, "\\%s\\%s", vsel->pname, vsel->vname);
			if (mkbootf(tarprtn, "\x80", fstype, &maxrsvd, base, NULL,
					outbufr, 0, 10, 0, mkbtfail, mkbtnotice))
				putline(COL_NOSEL, "XOS bootstrap installed");
			else
				continueinst = FALSE;
		}
		svcSysDateTime(1, (time_sz *)&curdt);
		sdt2str(ttxt, "%z%H:%m:%s on %D-%3n-%l", (time_sz *)&curdt);
		putline((continueinst) ? COL_GOOD : COL_FAIL, "%s %s complete at %s",
				(newinst) ? "Installation" : "Update", (continueinst) ?
				"is" : "did not", ttxt);
		break;
	}
	xwsWinDestroy(btncancel);
	btncancel = NULL;
	if (logfile != NULL)
	{
		fclose(logfile);
		logfile = NULL;
	}
	state = STATE_DONE;
	disprescan();
}


//**********************************************************************
// Function: startinput - Set up to start reading the compresed XIS file
// Returned: TRUE if normal or FALSE if error
//**********************************************************************

int startinput(void)
{
	long rtn;

	// Fill the XIS buffer

	if ((rtn = fbrIoInBlock(xishndl, xisbufr, XISBUFRSZ)) <= 0)
	{
		if (rtn == 0)
			rtn = ER_EOF;
		puterror(rtn, "Error reading XIS file %s", xisspec);
		return (FALSE);
	}
	stream.avail_in = rtn;
	stream.next_in = xisbufr;

	// Initialize zlib

   	stream.zalloc = (alloc_func)0;
   	stream.zfree = (free_func)0;
   	stream.opaque = (voidpf)0;
	if ((rtn = inflateInit(&stream)) != Z_OK)
	{
		puterror((ushort)rtn | 0x80000000, "Error initializing zlib");
		continueinst = FALSE;
		return (FALSE);
	}
	itemnum = 0;
	seqnum = 0;
	totalsize = 0;
	crcvalue = 0xFFFFFFFF;
	return (TRUE);
}


//****************************************************************
// Function: getitemhead = Read an item header, including the name
// Returned: TRUE if normal or FALSE if error
//****************************************************************

int getitemhead()
{
	long rtn;

	if ((rtn = getinput((char *)&itemhead, sizeof(itemhead))) < 0)
	{
		puterror(rtn, "Error expanding file header");
		return (FALSE);
	}
	if (itemhead.sync != 0xC183)
	{
		putline(COL_ERROR, "Incorrect sync value for item %d in %s", itemnum,
				xisspec);
		return (FALSE);
	}
	if (itemhead.seqnum != (char)seqnum)
	{
		putline(COL_ERROR, "Incorrect sequence number %02X, should be %02X "
				"in %s", (char)itemhead.seqnum, (char)seqnum, xisspec);
		return (FALSE);
	}
	seqnum++;
	if (itemhead.namelen != 0 &&
			(rtn = getinput(outbufr, itemhead.namelen)) < 0)
	{									// Get the name string
		puterror(rtn, "Error expanding file header");
		return (FALSE);
	}
	outbufr[itemhead.namelen] = 0;
	return (TRUE);
}


//************************************************************
// Function: getinput - Read data from the compressed XIS file
// Returned: Positive amount read if error or a negative XOS
//				or ZLIB error code if error
//************************************************************

long getinput(
	char *bufr,
	long  size)
{
	long  amnt;
	long  rtn;
	ulong crc;

	stream.next_out = bufr;
	stream.avail_out = size;

	amnt = 0;
	while (size > 0)
	{
///		if (*more)
		{
			if (stream.avail_in == 0)
			{
				if ((rtn = fbrIoInBlock(xishndl, xisbufr, XISBUFRSZ)) < 0)
				{
					if (rtn != ER_EOF)
						puterror(rtn, "Error reading XIS file, %s", xisspec);
///					*more = FALSE;
					rtn = 0;
				}
		    	stream.next_in = xisbufr;
		   		stream.avail_in = rtn;
			}
		}
		if ((rtn = inflate(&stream, Z_NO_FLUSH)) < 0)
			puterror(((ushort)rtn) | 0x80000000, "Error expanding XIS data "
					"in %s, xisspec");
		amnt += (size - stream.avail_out);
		size -= (size - stream.avail_out);
		if (rtn == Z_STREAM_END)
			break;
	}
	totalsize += amnt;

	rtn = amnt;
	crc = crcvalue;
    while (--rtn >= 0)
        crc = crctable[(crc ^ *bufr++) & 0xFF] ^ (crc >> 8);
	crcvalue = crc;
	return (amnt);
}


#if 0
static int copyfiles(
	char *dstarg,
	char *srcarg,
	int   complain)
{
	char sspec[300];

	complain = complain;
	if (!continueinst)
		return (FALSE);
	dstspec = dstarg;
	srcspec = srcarg;
	if (!makedir(dstarg, FALSE))
		return (FALSE);
	sprintf(sspec, "%s...\\*.*", srcarg);
	srcpath = strchr(srcspec, '\\');
	speclen = strlen(srcpath);
	if (!dirscan(sspec, &dsd, TRUE))	// Scan the directory
		return (FALSE);
	dirdone();
	return (TRUE);
}
#endif

//**************************************************************
// Function: dsdofile - Function called by dirscan for each file
// Returned: TRUE if should continue, FALSE if should terminate
//**************************************************************

static int dsdofile(void)
{
	char *pnt;
	long  dhndl;
	long  shndl;
	long  len;
	char  dst[300];
	char  src[300];
	char  buffer[32*1024];

	pnt = buffer;
	len = 8;
	do
	{
		*pnt = 0;
		pnt += 0x1000;
	} while (--len > 0);
	pnt[-1] = 0;

///	putline(COL_DEBUG, "dsdofile: %s | %s | %s", dsd.devname,  dsd.pathname,
///			dsd.filename);

	if (dsd.filenamelen == 0)
		return (TRUE);

	if (!continueinst)
		return (FALSE);
	if (strnicmp(dsd.pathname, srcpath, speclen) != 0)
	{
		putline(COL_ERROR, "Returned path did not match specified path:\n"
				"\x11\x05\x1F\x40Spec: %s\n\x11\x05\x1F\x40Rtnd: %s",
				srcpath, dsd.pathname);
		return (FALSE);
	}
///	if (dsd.changed & DSCHNG_PATHNAME)
	{
		dirdone();
		strmov(strmov(dirspec, dstspec), dsd.pathname + speclen);
///		putline(COL_NOSEL, "Copying to directory %s", dirspec);
	}
	sprintf(dst, "%s%s%s", dstspec, dsd.pathname + speclen, dsd.filename);
	sprintf(src, "%s%s", dsd.pathname, dsd.filename);

	putline(COL_NOSEL | TEMP, "Copying to %s", dst);
///	putline(COL_NOSEL, "Copying to %s", dst);

///	putline(COL_DEBUG, "Copying from %s", src);

///	fbrCtlSuspend(NULL, 0, ST_SECOND/10);

	// Open the source file

	if ((shndl = fbrIoOpen(XO_IN, src, NULL)) < 0)
	{
		puterror(shndl, "Error opening file %s", dst);
		continueinst = FALSE;
		return (FALSE);
	}

	// Create the destination file

	if ((dhndl = fbrIoOpen(XO_OUT|XO_TRUNCA|XO_CREATE, dst, NULL)) < 0)
	{
		puterror(dhndl, "Error creating file %s", dst);
		fbrIoClose(shndl, 0);
		continueinst = FALSE;
		return (FALSE);
	}

	// Copy the file

	while (TRUE)
	{
		if ((len = fbrIoInBlock(shndl, (char *)buffer, sizeof(buffer))) < 0 &&
				len != ER_EOF)
		{
			puterror(len, "Error reading file %s", src);
			fbrIoClose(dhndl, 0);
			fbrIoClose(shndl, 0);
			continueinst = FALSE;
			return (FALSE);
		}
		if (len <= 0)
			break;
		if ((len = fbrIoOutBlock(dhndl, (char *)buffer, len)) < 0)
		{
			puterror(len, "Error writing file %s", dst);
			fbrIoClose(dhndl, 0);
			fbrIoClose(shndl, 0);
			continueinst = FALSE;
			return (FALSE);
		}
	}

	// Close the files

	fbrIoClose(shndl, 0);
	if ((len = fbrIoClose(dhndl, 0)) < 0)
	{
		puterror(len, "Error closing destination file %s", dst);
		continueinst = FALSE;
		return (FALSE);
	}
	filecnt++;
	return (TRUE);
}


static int makedir(
	char *spec,
	int   ask)
{
	char *spnt;
	char *dpnt;
	char *epnt;
	long  rtn;
	char  name[600];
	char  chr;

///	putline(COL_DEBUG, "in makedir: %s", spec);

	while (TRUE)
	{
		if ((rtn = fbrIoDevParm(XO_CREATE|XO_ODFS, spec, &dtparms)) < 0)
		{
///			puterror(rtn, "Error creating directory %s", spec);

			if (rtn == ER_DIRNF || rtn == ER_FILNF)
			{
				spnt = spec;
				dpnt = name;
				epnt = NULL;
				while ((chr = *spnt++) != 0)
				{
					*dpnt++ = chr;
					if (chr == '\\' && *spnt != 0)
						epnt = dpnt;
				}
				if (epnt != NULL)
				{
					*epnt = 0;
					if (!makedir(name, ask))
						return (FALSE);
					continue;
				}
			}
			if (rtn != ER_FILEX && rtn != ER_DIREX)
			{
				puterror (rtn, "Error creating directory %s", spec);
				return (FALSE);
			}
			if (ask && question(0xC0C0FF, 0x8080FF, ESC << 8, "ESC - Cancel",
					'\r' << 8, "<ENTER> - Continue", "This backup set already "
					"exists. If you continue it will be overwritten.") != 2)
			{
				putline(COL_FAIL, "--- CANCELED ---");
				return (FALSE);
			}
			putline(COL_NOSEL, "Directory %s exists", spec);
		}
		else
			putline(COL_NOSEL, "Directory %s created", spec);
		return (TRUE);
	}
}


static void dirdone(void)
{
	if (continueinst && filecnt > 0)
	{
		putline(COL_NOSEL, "%d file%s copied to directory %s", filecnt,
				(filecnt == 1) ? "" : "s", dirspec);
		filecnt = 0;
	}
}


static void mkbtfail(
	char *str1,
	long  code,
	char *str2)
{
	if (str2 != NULL)
		puterror(code, "%s: %s", str1, str2);
	else
		puterror(code, str1);
}


static void mkbtnotice(
	char *str)
{
	putline(COL_NOSEL, str);
}


//*******************************************************
// Function: dserrmsg - Display error message for dirscan
// Returned: Never returns
//*******************************************************

static void dserrmsg(
    const char *arg,
    long  code)
{
	puterror(code, (char *)arg);
}
