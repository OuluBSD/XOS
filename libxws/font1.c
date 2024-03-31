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

#include <xosxxws.h>

// Following is a summary of now the FreeType engine is used here.  During
//   startup (xwsftfinit), the XOSFNT: directory is scanned and all .FTF
//   files found are opened by the engine.  The font names and charmap
//   numbers are recorded in FNB blocks, which constitute the list off all
//   available fonts.  No faces are left open at this time.  No FSBs or FNTs
//   are created at this time.  When a font is loaded (xwsFontLoad), the face
//   is opened if it is not already open, the charmap is set up for quick
//   access, and various parameters are stored in the FSB and FNT which are
//   created at this time.  No glyphs are mapped at this time but a 256 entry
//   glyph pointer table is allocated as part of the FNT.  When xwsFontGetGlyph
//   is called to obtain a glyph bitmap, the bitmap is generated and a pointer
//   to it is stored in the glyph pointer table, which is then used to access
//   that glyph.

// THIS VERSION DOES NOT DO ANY CACHE MANAGEMENT --- ALL GLYPHS ARE RETAINED
//   IN MEMORY UNTIL THE ENTIRE FONT IS UNLOADED!

extern char prgname[];

XWSFNB *xwsfirstfnb;

static FT_Library  ftlibrary;
static FT_Error    fterror;

FT_Size_Metrics *xxxsize;

long xxxascent;
long xxxdescent;
long xxxheightc;
long xxxheightg;

static char *copyname(char *dst, char *src, long *cnt);
static long convtopx(long emsize, long height, long value);
static void makefsb(XWSFNB *fnb, XWSFSB **prevfsb, long width, long height,
		long szinx, long faceinx, char *filename);

//*********************************************************************//
// Function: xwsfontinit - Initialize fonts - This must be called once //
//		upon startup.  It scans the XOSFNT: directory and constructs   //
//		FNBs for all valid font files it finds there.                  //
// Returned: 0 if normal, negative XOS error code if error             //
//*********************************************************************//

long XOSFNC xwsfontinit(void)
{
	XWSFNB **prevfnb;
	XWSFSB **prevfsb;
    XWSFNB  *fnb;
	XWSFSB  *fsb;
	FT_Bitmap_Size
            *szpnt;
	char    *pnt;
    FT_Face  ftface;
    long     rtn;
	long     dirhndl;
	int      szcnt;
    int      filelen;
	int      fontinx;
	int      numindex;
    char     fontname[128];
    char     filename[256];
    char     namebufr[128];

	static struct
	{	BYTE4PARM  optn;
		LNGSTRPARM spec;
		char       end;
	} srchparms =
	{	{PAR_SET|REP_HEXV, 4, IOPAR_FILEOPTN, XFO_FILE},
		{PAR_GET|REP_STR , 0, IOPAR_FILESPEC, NULL, sizeof(namebufr),
				sizeof(namebufr)}
	};

	if ((fterror = FT_Init_FreeType(&ftlibrary)) != 0)
	{
		printf("? %s: FreeType initialization (FT_Init_FreeType) failed: %s",
				FT_ErrMsg(fterror));
		exit (1);
	}
	srchparms.spec.buffer = namebufr;
	if ((dirhndl = svcIoOpen(XO_ODFS, "XOSFNT:", NULL)) < 0)
		femsg(prgname, dirhndl, "Error opening XOS font directory");
	while (TRUE)
	{
		if ((rtn = sysIoNextFile(dirhndl, "*.*", (char *)&srchparms)) < 0)
		{
			if (rtn == ER_FILNF)
				break;
			femsg(prgname, rtn, "Error scanning XOS font directory");
		}
		filelen = sprintf(filename, "XOSFNT:\\%s", namebufr) - 6;
		fontinx = 0;
		do
		{
			if ((fterror = FT_New_Face(ftlibrary, filename, fontinx,
					&ftface)) != 0)
			{
				printf("Error opening TT font file %s(%d): %s\n", filename,
						fontinx, FT_ErrMsg(fterror));
				break;
			}
			if (fontinx == 0)
				numindex = ftface->num_faces;
			rtn = 127;
			pnt = copyname(fontname, ftface->family_name, &rtn);
			*pnt++ = '|';
			pnt = copyname(pnt, ftface->style_name, &rtn);

///			printf("Name: %s (%s)\n", fontname, namebufr);

			// Here with a valid face - find the place for it in the FNB
			//   list (this also checks for duplicates)

			prevfnb = &xwsfirstfnb;
			while ((fnb = *prevfnb) != NULL &&
					(rtn = stricmp(fontname, fnb->fontname)) < 0)
				prevfnb = &(fnb->next);

			if (fnb != NULL && rtn == 0) //Duplicate name?
			{
				// Here with a duplicate name - This is only valid for a
				//   bitmapped font.

				if ((ftface->face_flags & FT_FACE_FLAG_SCALABLE) ||
						fnb->ffsb == NULL)
				{
					printf("Duplicate font name %s in files %s and %s\n",
							fontname, namebufr, fnb->ffsb->filename);
					continue;
				}
			}
			else
			{
				// Not a duplicate name - If a bitmapped font make sure at
				//   least one size is specified.

				if ((ftface->face_flags & FT_FACE_FLAG_SCALABLE) == 0 &&
						ftface->num_fixed_sizes <= 0)
				{
					printf("Non-scalable font %s does not specify any sizes\n",
							fontname);
					continue;
				}

				// OK - create an FNB

				rtn = strlen(fontname) + 1;
				if ((fnb = (XWSFNB *)xwsMalloc(offsetof(XWSFNB, fontname) +
						rtn)) == NULL)
					return (0);
				memset(fnb, 0, offsetof(XWSFNB, fontname));
				fnb->label = 'FNB#';
				fnb->ffsb = NULL;
				memcpy(fnb->fontname, fontname, rtn);
				fnb->next = *prevfnb;
				*prevfnb = fnb;
			}
			if ((ftface->face_flags & FT_FACE_FLAG_SCALABLE) == 0)
			{
				// Here if have a bitmapped font. For each size in the font
				//   find where the FSB goes and check for duplicat sizes.
				//   (FSBs are ordered first by increasing height then by
				//   increasing width.)

				if ((szcnt = ftface->num_fixed_sizes) > 0 &&
						(szpnt = ftface->available_sizes) != NULL)
				{
					do
					{
						prevfsb = &fnb->ffsb;
						while ((fsb = *prevfsb) != NULL &&
								(fsb->height < szpnt->height ||
								(fsb->height == szpnt->height &&
								fsb->width < szpnt->width)))
							prevfsb = &fsb->next;

						if (fsb != NULL && fsb->height == szpnt->height &&
								fsb->width == szpnt->width) // Duplicate size?
						{
							printf("Duplicate sizes (%d, %d) found in font %s "
									"(%s) and (%s)\n", fsb->width, fsb->height,
									fontname, namebufr, fsb->fnb->fontname,
									fsb->filename);
							continue;
						}
						makefsb(fnb, prevfsb, szpnt->width, szpnt->height,
							szpnt - ftface->available_sizes, ftface->face_index,
							namebufr);
					} while (++szpnt, --szcnt > 0);
				}
			}
			else
			{
				// Here if have a scalable font. We create a single dummy
				//   FSB.

				makefsb(fnb, &fnb->ffsb, 0, 0, 0, ftface->face_index, namebufr);
			}

			if (ftface->charmap->encoding != FT_ENCODING_UNICODE)
			{
				printf("Warning: Charmap for TT font %s (%d) in file %s is "
						"not Unicode\n", fontname, fontinx, namebufr);
				continue;
			}

			if ((fterror = FT_Set_Pixel_Sizes(ftface, 256, 256)) != 0)
			{
				printf("Warning: Could not set up face for size check, 1.00 "
						"assumed");
				continue;
			}

			xxxascent = ftface->size->metrics.ascender;
			xxxdescent = -ftface->size->metrics.descender;
			xxxheightc = ftface->size->metrics.height;

			fnb->scale = xxxheightc >> 6;

///			printf("%4X %4X %4X (%4X) %X %s\n", (xxxascent+0x3F) >> 6,
///					(xxxdescent+0x3F) >> 6, (xxxheightc+0x3F) >> 6,
///					(xxxascent + xxxdescent + 0x3F) >> 6, fnb->scale, fontname);

			if ((fterror = FT_Done_Face(ftface)) != 0)
				printf("Error closing font face %s (%d) in file %s: %s\n",
						fontname, fontinx,  namebufr, fterror);
		} while (++fontinx < numindex);
	}
	svcIoClose(dirhndl, 0);
	return (0);
}	


static char *copyname(
	char *dst,
	char *src,
	long *cnt)

{
	char chr;

	while (--(*cnt) > 0 && (chr = *src++) != 0)
	{
		if (chr == ' ' || chr == '|')
			chr = '_';
		*dst++ = chr;
	}
	*dst = 0;
	return (dst);
}


static void makefsb(
	XWSFNB  *fnb,
	XWSFSB **prevfsb,
	long     width,
	long     height,
	long     szinx,
	long     faceinx,
	char    *filename)

{
	XWSFSB *fsb;
	int     len;

	len = strlen(filename) + 1;
	if ((fsb = (XWSFSB *)xwsMalloc(sizeof(XWSFSB) + len)) == NULL)
	{
		printf("Error allocating an FSB for font %s (%d) in file %s\n",
				fnb->fontname, faceinx, filename);
		return;
	}
	memset(fsb, 0, sizeof(XWSFSB));
	fsb->label = 'FSB#';
	fsb->fnb = fnb;
	fsb->width = width;
	fsb->height = height;
	fsb->szinx = szinx;
	fsb->faceinx = faceinx;
	memcpy(fsb->filename, filename, len);
	fsb->next = *prevfsb;
	*prevfsb = fsb;
}


//**********************************************************************//
// Function: xwsFontLoad - Load a font                                  //
// Returned: Pointer to the FNT for the font if normal or NULL if error //
//**********************************************************************//


long xxxascendfu;
long xxxdescendfu;
long xxxEMunits;
long xxxxscale;
long xxxyscale;
long xxxascendpx;
long xxxdescendpx;
long xxxheight;


XWSFNT* XOSFNC xwsFontLoad(
    char   *name,		// Name of the font
	XWSWIN *win,
    long    height,		// Font height, in pixels
    long    width,		// Font width, in pixels (-1 means don't care)
    long    attr)		// Font attributes (bit 31 set if height and width
						//   values are unscaled)
{
	XWSFNB  *fnb;
    XWSFSB  *fsb;
    XWSFNT  *fnt;
    XWSFNT **prevfnt;
	FT_Face  ftface;
	long     emsize;
    char     filename[256];

	if (attr >= 0)
	{
		height = SCALE(win->xsfr, win->xsfd, height);
		if (width > 0)
			width = SCALE(win->ysfr, win->ysfd, width);
	}
	fnb = xwsfirstfnb;
	while (fnb != NULL && stricmp(name, fnb->fontname) != 0)
		fnb = fnb->next;
	if (fnb == NULL)
		return (NULL);

	// Found the font - If it is a bitmapped font see if we have the size
	//   he wants

	fsb = fnb->ffsb;
	if (fsb->height != 0)
	{
		// Here if this is a bitmapped font - Find the correct FSB for the
		//   size he wants. Note that the match must be exact execpt that
		//   the width can be unspecified.

		while (fsb != NULL && (height > fsb->height || (width != 0 &&
				height == fsb->height && width > fsb->width)))
			fsb = fsb->next;
		if (fsb == NULL || height != fsb->height ||
					(width != 0 && width != fsb->width))
			return (NULL);
		prevfnt = &fsb->ffnt;			// Get the FNT for the font if its
		fnt = *prevfnt;					//   already loaded (There is only
										//   one FNT for each bitmapped font
	}									//   size.)
	else
	{
		// Here if this is a scalled font - See if we already have an FNT
		//   for this exact size. If so use it, if not create a new FNT for
		//   the size we want.

		// THIS VERSION CREATES A DIFFERENT FACE FOR EACH SIZE. THIS NEEDS TO
		//   BE CHANGED!!!!

		prevfnt = &(fsb->ffnt);
		if (width == 0)
			width = height;
		while ((fnt = *prevfnt) != NULL && (height > fnt->height ||
				height == fnt->height && width > fnt->width))
			prevfnt = &fnt->next;
	}

	if (fnt == NULL || height != fnt->height || width != fnt->width)
	{
		// Here if an existing FNT was not found - Create a new one.

		sprintf(filename, "XOSFNT:\\%s", fsb->filename);
		if ((fterror = FT_New_Face(ftlibrary, filename, fsb->faceinx,
				&ftface)) != 0)
			xwsFail(0, "Error opening font face %s (%d): %s", fsb->filename,
					fsb->faceinx, FT_ErrMsg(fterror));
		xxxsize = &ftface->size->metrics;

		if ((fterror = FT_Set_Pixel_Sizes(ftface,
				((width << 8) + (fnb->scale > 1)) / fnb->scale,
				((height << 8) + (fnb->scale > 1)) / fnb->scale)) != 0)
			xwsFail(0, "Error setting size for font face %s (%d): %s",
					fsb->filename, fsb->faceinx, FT_ErrMsg(fterror));

		xxxascendfu = ftface->ascender;
		xxxdescendfu = ftface->descender;
		xxxEMunits = ftface->units_per_EM;
		xxxxscale = ftface->size->metrics.x_scale;
		xxxyscale = ftface->size->metrics.y_scale;
		xxxascendpx = ftface->size->metrics.ascender;
		xxxdescendpx = ftface->size->metrics.descender;
		xxxheight = ftface->height;

		xxxascent = ftface->size->metrics.ascender;
		xxxdescent = ftface->size->metrics.descender;
		xxxheight = ftface->size->metrics.height;

		// Face is now open - allocate an FNT

		if ((fnt = (XWSFNT *)xwsMalloc(sizeof(XWSFNT))) == NULL)
			xwsFail(0, "Error allocating memory for font face %s (%d): %s",
					fsb->filename, fsb->faceinx, strerror(errno));
		memset(fnt, 0, sizeof(XWSFNT));
		fnt->label = 'FNT#';
		fnt->fnb = fnb;
		fnt->fsb = fsb;
		fnt->ftface = ftface;
		fnt->width = width;
		fnt->height = height;
		emsize = fnt->ftface->units_per_EM;
		xxxascent = fnt->ascent = (ftface->size->metrics.ascender + 0x3F) >> 6;

///convtopx(emsize, ftface->ascender, height);

		xxxdescent = fnt->descent = (-ftface->size->metrics.descender + 0x3F) >> 6;

///fnt->descent = convtopx(emsize, abs(ftface->descender), height);
		xxxheightc = (ftface->size->metrics.height + 0x3F) >> 6;

///convtopx(emsize, abs(ftface->height), height);
		xxxheightg = height;

		fnt->leading = 0;		// ????

		fnt->lnspace = 0;

///		fnt->lnspace = convtopx(emsize, ftface->height, height);


		fnt->maxwidth = 0;

///		fnt->maxwidth = convtopx(emsize, ftface->max_advance_width, height);
		fnt->attr = 0;

		fnt->weight = (ftface->style_flags & FT_STYLE_FLAG_BOLD) ? 120 : 100;
		fnt->underpos = convtopx(emsize, abs(ftface->underline_position),
				height);
		fnt->undersize = convtopx(emsize, ftface->underline_thickness, height);
		fnt->strikepos = fnt->ascent/3; // ????
		fnt->strikesize = fnt->undersize; // ????

		fnt->attr &= ~(FNTA_MONO|FNTA_ITALIC);
		if (ftface->face_flags & FT_FACE_FLAG_FIXED_WIDTH)
			fnt->attr |= FNTA_MONO;
		if (ftface->style_flags & FT_STYLE_FLAG_ITALIC)
			fnt->attr |= FNTA_ITALIC;

///		BREAK();

		// Finally, link the FNT into our list of FNTs for this FSB

		fnt->next = *prevfnt;
		*prevfnt = fnt;
		fnt->loadcnt = 0;
	}
	fnt->loadcnt++;
	return (fnt);
}


static long convtopx(
	long emsize,
	long height,
	long value)

{
	long result[2];

	slongdiv(result, value << 8, height, emsize);
	return ((result[0] + 0x40) >> 8);
}


long ffff;

//**************************************************************//
// Function: xwsFontUnload - Unload a font - The load count for //
//		the font is decremented.  If it goes to 0, the font     //
//		glyph pointer table and the glyph bit-maps are removed  //
//		from memory.                                            //
// Returned: Nothing                                            //
//**************************************************************//

void XOSFNC xwsFontUnload(
    XWSFNT *fnt)		// Pointer to FNT for the font
{
	XWSGCB **htpnt;
	XWSGCB  *gpnt;
	XWSGCB  *gnxt;
	XWSFSB  *fsb;
	XWSFNT **prevp;
	XWSFNT  *fpnt;
	long     rtn;
	int      cnt;

	ffff = fnt->loadcnt;
	if (--(fnt->loadcnt) <= 0)
	{
		if (fnt->loadcnt < 0)
			xwsFail(0, "Load count (loadcnt) for font is negative");
		fsb = fnt->fsb;
		prevp = &fsb->ffnt;
		while ((fpnt = *prevp) != NULL && fpnt->next == fnt)
			prevp = &fpnt->next;
		*prevp = fnt->next;					// Unlink this FNT
		htpnt = fnt->glyphtbl;
		cnt = 128;
		do
		{
			if ((gpnt = *htpnt++) != NULL)
			{
				do
				{
					FT_Done_Glyph(gpnt->ftglyph);
					gnxt = gpnt->next;
					xwsFree(gpnt);
					gpnt = gnxt;
				} while (gpnt != NULL);
			}
		} while (--cnt > 0);
		if ((rtn = FT_Done_Face(fnt->ftface)) != 0)
			xwsFail(0, "Error %d returned by FT_Done_Face", rtn);
		xwsFree(fnt);
	}
}


//*********************************************************************//
// Function: xwsFontGetLength - Get length of a text string for a font //
// Returned: Length of the rendered string in pixels                   //
//*********************************************************************//

/*
long XOSFNC xwsFontGetLength(
	XWSFNT *fnt,
	XWSWIN *win,
	char   *text,
	long    cnt)

{
	return (xwsfontgetlengthns(fnt, text, cnt) * win->xsfd);
}
*/


//**********************************************************************//
// Function: xwsfontgetchrwidthns - Get width of a character for a font //
// Returned: Number of bytes consumed                                   //
//**********************************************************************//

// The character's width is stored in the long pointed to by pwidth. This
//   function handles UTF-8 characters.

long XOSFNC xwsfontgetchrwidthns(
	XWSFNT *fnt,			// XWSFNT for the font
	char   *pchr,			// Address of byte byte for character
	long    prev,			// Index for previous character (for kerning)
	long    cnt,			// Number of bytes available
	long   *pwidth)			// Address of long to receive the width
{
	XWSGCB *gcb;
	int     len;
	long    chr;
	long    chr2;
	long    chr3;
	long    chr4;

	len = 1;							// Assume a one byte character
	chr = pchr[0];						// Get the first byte

	if (chr & 0x80)						// Unicode multibyte character?
	{
		if ((chr & 0xE0) == 0xC0)		// 2-byte character?
		{				
			if (cnt < 2 || ((chr2 = pchr[1]) & 0xC0) != 0x80)
				chr = '?';
			else
			{
				len++;
				chr = (chr2 & 0x3F) | ((chr & 0x1F) << 6);
			}
		}
		else if ((chr & 0xF0) == 0xE0)	// 3-byte character?
		{
			if ((cnt < 3) < 0 || ((chr2 = pchr[1]) & 0xC0) != 0x80 ||
					((chr3 = pchr[2]) & 0xC0) != 0x80)
				chr = '?';
			else
			{
				len = 3;
				chr = (chr3 & 0x3F) | ((chr2 & 0x3F) << 6) |
						((chr & 0x0F) << 12);
			}
		}
		else if ((chr & 0xF8) == 0xF0)	// 4-byte character?
		{
			if ((cnt < 4) < 0 || ((chr2 = pchr[1]) & 0xC0) != 0x80 ||
					((chr3 = pchr[2]) & 0xC0) != 0x80 ||
					((chr4 = pchr[3]) & 0xC0) != 0x80)
				chr = '?';
			else
			{
				len = 4;
				chr = (chr4 & 0x3F) | ((chr3 & 0x3F) << 6) |
						((chr2 & 0x3F) << 6) | ((chr & 0x07) << 18);
			}
		}
		else
			chr = '?';
	}
	*pwidth = ((gcb = xwsfontgetglyph(fnt, chr)) == NULL) ? 0 :
			((gcb->ftglyph->advance.x + 0xFFFF) >> 16);
	return (len);
}


//**************************************************************************//
// Function: xwsfontgetstrlengthns - Get length of a text string for a font //
// Returned: Length of the rendered string in pixels                        //
//**************************************************************************//

long XOSFNC xwsfontgetstrlengthns(
	XWSFNT *fnt,
	char   *text,
	long    cnt)
{
	XWSGCB *gcb;
	int     length;
	long    chr;
	long    chr2;
	long    chr3;
	long    chr4;

	length = 0;
	while (--cnt >= 0 && (chr = *text++) != 0)
	{
		if (chr & 0x80)					// Unicode multibyte character?
		{
			if ((chr & 0xE0) == 0xC0)	// 2-byte character?
			{
				if (--cnt < 0)
					break;

				if (((chr2 = *text++) & 0xC0) != 0x80)
					continue;
				chr = (chr2 & 0x3F) | ((chr & 0x1F) << 6);
			}
			else if ((chr & 0xF0) == 0xE0) // 3-byte character?
			{
				if ((cnt -= 2) < 0)
					break;
				if (((chr2 = *text++) & 0xC0) != 0x80)
					continue;
				if (((chr3 = *text++) & 0xC0) != 0x80)
					continue;
				chr = (chr3 & 0x3F) | ((chr2 & 0x3F) << 6) |
						((chr & 0x0F) << 12);
			}
			else if ((chr & 0xF8) == 0xF0) // 4-byte character?
			{
				if ((cnt -= 3) < 0)
					break;
				if (((chr2 = *text++) & 0xC0) != 0x80)
					continue;
				if (((chr3 = *text++) & 0xC0) != 0x80)
					continue;
				if (((chr4 = *text++) & 0xC0) != 0x80)
					continue;
				chr = (chr4 & 0x3F) | ((chr3 & 0x3F) << 6) |
						((chr2 & 0x3F) << 6) | ((chr & 0x07) << 18);
			}
			else						// If invalid sequence - ignore it!
				continue;
		}
		if ((gcb = xwsfontgetglyph(fnt, chr)) == NULL)
			return (-1);
		length += ((gcb->ftglyph->advance.x + 0xFFFF) >> 16);
	}
	return (length);
}


//***************************************************//
// Function: xwsfontgetglyph - Get a character glyph //
// Returned: Pointer to the glyph bit-map structure  //
//***************************************************//

XWSGCB* XOSFNC xwsfontgetglyph(
    XWSFNT *fnt,
    int     chr)
{
	XWSGCB **prevgcb;
    XWSGCB  *gcb;
	XWSGCB  *ngcb;
	long     inx;

	inx = FT_Get_Char_Index(fnt->ftface, chr);
	prevgcb = &fnt->glyphtbl[inx & 0x7F];
	while ((gcb = *prevgcb) != NULL && (gcb->glyphinx < inx ||
			(gcb->glyphinx == inx && gcb->fnt < fnt)))
		prevgcb = &gcb->hash;
	if (gcb != NULL && gcb->glyphinx == inx)
		return (gcb);

	// Here if the character was not found in the cache

	if ((fterror = FT_Load_Glyph(fnt->ftface, inx,
			FT_LOAD_FORCE_AUTOHINT)) != 0)
	{
		return (NULL);
	}
	if ((ngcb = (XWSGCB *)xwsMalloc(sizeof(XWSGCB))) == NULL)
		xwsFail(-errno, "Cannot allocate meory for glyph");
	if ((fterror = FT_Get_Glyph(fnt->ftface->glyph, &ngcb->ftglyph)) != 0)
	{

		xwsFree(ngcb);
		return (NULL);
	}
	if ((fterror = FT_Glyph_To_Bitmap(&ngcb->ftglyph, FT_RENDER_MODE_MONO,
			NULL, TRUE)) != 0)
	{

		xwsFree(ngcb);
		FT_Done_Glyph(ngcb->ftglyph);
		return (NULL);
	}
	ngcb->label = 'GCB#';
	ngcb->hash = gcb;
	*prevgcb = ngcb;
	ngcb->next = NULL;
	ngcb->prev = NULL;
	ngcb->glyphinx = inx;
	ngcb->fnt = fnt;
	return (ngcb);
}
