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

#include "mkinstall.h"

static int linenum;

char *item1;
char *item2;
char *item3;
int   len1;
int   len2;
int   len3;

static char *nexttok(char *pnt, char **item, int *len);


DIRB **pnt2dir;
SUBB **pnt2sub;
FILB **pnt2file;
FILB **pnt2xcld;
FILB **pnt2gblx;
DIRB  *dir;
SUBB  *sub;
FILB  *file;


//******************************************************************
// Function: readins - Read the INS file and construct our data base
// Returned: Nothing (does not return if error)
//******************************************************************

void readins(void)
{
	char *pnt;
	char *epnt;
	int   flags;
	int   len;
	char  chr;

	pnt2dir = &dirhead;
	dir = NULL;
	sub = NULL;
	file = NULL;
	linenum = 0;
	while (fgets(inpbufr, 300, ctlfile) != NULL)
	{
		linenum++;
		len = strlen(inpbufr);
		pnt = inpbufr + len - 1;		// Remove line terminators
		while (pnt >= inpbufr && ((chr = *pnt) == '\r' || chr == '\n'))
			*pnt-- = 0;
		pnt = inpbufr;
		while ((chr = *pnt) != 0 && isspace(chr)) // Skip leading whitespace
			pnt++;

		if (pnt[0] == 0 || pnt[0] == '#')
			continue;

		switch(pnt[1])
		{
		 case 'c':						// Copy to boot disk
			flags = IF_ACOPY|IF_COPY;
			break;

		 case 'i':						// Add to install set
			flags = IF_AISET|IF_ISET;
			break;

		 case 'b':						// Do both
			flags = IF_ACOPY|IF_AISET|IF_COPY|IF_ISET;
			break;

		 case ' ':
			if (pnt[0] == 'X' || pnt[0] == 'G' || pnt[0] == 'B')
				break;
		 default:
			sprintf(inpbufr, "Second letter in line (%c) is invalid", pnt[1]);
			inserror(inpbufr, linenum);
			break;
		}
		len = pnt[0];
		pnt += 2;
		while ((chr = *pnt) != 0 && isspace(chr)) // Skip whitespace
			pnt++;

		pnt = nexttok(pnt, &item1, &len1);
		pnt = nexttok(pnt, &item2, &len2);
		nexttok(pnt, &item3, &len3);

		switch(len)
		{
		 case 'B':						// Bootstrap file
			if (bootstrapsrc[0] != 0)
				inserror("More than one bootstrap file specification", linenum);
			strmov(bootstrapsrc, item1);
			strmov(bootstrapdflt, item2);
			bootstraptime = strtol(item3, &epnt, 0);
			if (*epnt != 0)
				inserror("Invalid bootstrap time-out value", linenum);
			break;

		 case 'O':						// Start of OS install set
			flags |= IT_OSISET;
			goto isetcom;

		 case 'A':						// Start of application install set
			flags |= IT_APISET;
			goto isetcom;

		 case 'D':						// Start of data install set
			flags |= IT_DTISET;
		 isetcom:
			if (item3 == NULL || item3[0] == 0)
				inserror("No primary install set file specified", linenum);
			if (item1 == NULL || item1[0] == 0)
				inserror("No source directory specified", linenum);
			if (item2 == NULL || item2[0] == 0)
				inserror("No target directory specified", linenum);
			hviset = TRUE;
			goto dircom;

		 case 'R':						// Root directory
			if (!hviset)
				inserror("Root directory is not part of an install set",
						linenum);
			if (flags & (IF_COPY|IF_ISET) & ~(dir->flag))
				inserror("Root directory cannot override install set "
						"destination", linenum);
			flags |= IT_RTDIR;
		 dircom:
			dir = allocmem(offsetof(DIRB, srcname) + len1 + len2 + 2);
			dir->flag = flags;
			dir->linenum = linenum;
			dir->subhead = NULL;
			dir->filehead = NULL;
			dir->xcldhead = NULL;
			pnt2sub = &dir->subhead;
			pnt2file = &dir->filehead;
			pnt2xcld = &dir->xcldhead;
			pnt2gblx = &dir->gblxhead;
			sub = NULL;
			file = NULL;
			memcpy(dir->srcname, item1, len1 + 1);
			if (item2[0] == 0)
				dir->tgtname = dir->srcname;
			else
			{
				dir->tgtname = dir->srcname + len1 + 1;
				memcpy(dir->tgtname, item2, len2 + 1);
			}
			dir->next = NULL;
			*pnt2dir = dir;
			pnt2dir = &dir->next;
			if ((flags & IF_TYPE) == IT_RTDIR)
				break;
			item1 = item2 = item3;
			len1 = len2 = len3;
			flags = (flags & ~IF_TYPE) | IT_FILE;
			goto dofile;

		 case 'S':						// Sub-directory specification
			if (!hviset)
				inserror("Sub-directory is not part of an install set",
						linenum);
			if (flags & (IF_COPY|IF_ISET) & ~(dir->flag))
				inserror("Sub-directory cannot override install set "
						"destination", linenum);
			flags |= IT_SBDIR;
			sub = allocmem(offsetof(SUBB, srcname) + len1 + len2 + 2);
			sub->flag = flags;
			sub->linenum = linenum;
			sub->filehead = NULL;
			sub->xcldhead = NULL;
			pnt2file = &sub->filehead;
			pnt2xcld = &sub->xcldhead;
			memcpy(sub->srcname, item1, len1 + 1);
			if (item2[0] == 0)
				sub->tgtname = sub->srcname;
			else
			{
				sub->tgtname = sub->srcname + len1 + 1;
				memcpy(sub->tgtname, item2, len2 + 1);
			}
			sub->next = NULL;
			*pnt2sub = sub;
			pnt2sub = &sub->next;
			break;

		 case 'F':						// File name
			if (!hviset)
				inserror("File is not part of an install set", linenum);
			flags |= IT_FILE;
			len = (sub != NULL) ? sub->flag : dir->flag;
			if (flags & (IF_COPY|IF_ISET) & ~len)
				inserror("File cannot override install set destination",
						linenum);
		 dofile:
			len = (sub != NULL) ? sub->flag : dir->flag;
			if (flags & (IF_COPY|IF_ISET) & ~len)
				inserror("File cannot override directory installation",
						linenum);
			file = allocmem(offsetof(DIRB, srcname) + len1 + len2 + 2);
			file->flag = flags;
			file->linenum = linenum;
			memcpy(file->srcname, item1, len1 + 1);
			if (item2[0] == 0)
				file->tgtname = file->srcname;
			else
			{
				file->tgtname = file->srcname + len1 + 1;
				memcpy(file->tgtname, item2, len2 + 1);
			}
			file->next = NULL;
			*pnt2file = file;
			pnt2file = &file->next;
			break;

		 case 'X':						// Excluded file
			if (!hviset)
				inserror("Excluded file is not part of an install set",
						linenum);
			flags |= IT_FILE;
			file = allocmem(offsetof(FILB, srcname) + len1 + 1);
			file->flag = flags;
			file->linenum = linenum;
			memcpy(file->srcname, item1, len1 + 1);
			file->next = NULL;
			*pnt2xcld = file;
			pnt2xcld = &file->next;
			break;

		 case 'G':						// Globally excluded file
			if (!hviset)
				inserror("Excluded file is not part of an install set",
						linenum);
			flags |= IT_FILE;
			file = allocmem(offsetof(FILB, srcname) + len1 + 1);
			file->flag = flags;
			file->linenum = linenum;
			memcpy(file->srcname, item1, len1 + 1);
			file->next = NULL;
			*pnt2gblx = file;
			pnt2gblx = &file->next;
			break;

		 default:
			sprintf(inpbufr, "First letter in line (%c) is invalid", pnt[1]);
			inserror(inpbufr, linenum);
		}
	}
}


static char *nexttok(
	char  *pnt,
	char **ptok,
	int   *plen)
{
	char  chr;

	while ((chr = *pnt) != 0 && isspace(chr)) // Skip leading whitespace
		pnt++;
	*ptok = pnt;
	while ((chr = *pnt) != 0 && !isspace(chr) && chr != '|') // Skip to end of
		pnt++;												 //   the token
	*pnt = 0;							// Terminate the token
	*plen = pnt - *ptok;
	if (isspace(chr))
	{
		while ((chr = *pnt) != 0 && isspace(chr)) // Skip trailing whitespace
			pnt++;
		if (chr == 0 | chr == '|')		// Valid terminator?
			inserror("Syntax error", linenum);
	}
	if (chr != 0)
		pnt++;
	return (pnt);
}


static char *getletters(
	int flags);

void showall(void)
{
	DIRB *dir;
	SUBB *sub;

	if (bootstrapsrc[0] != 0)
		printf("BOOT: %s | %s | %d\n", bootstrapsrc, bootstrapdflt,
				bootstraptime);
	dir = dirhead;
	while (dir != NULL)
	{		
		printf("DIR: %s %s | %s\n", getletters(dir->flag), dir->srcname,
				dir->tgtname);
		showfiles(dir->gblxhead, dir->filehead, dir->xcldhead, "");

		sub = dir->subhead;
		while (sub != NULL)
		{
			if (sub->srcname == sub->tgtname)
				printf("  SUB: %s %s\n", getletters(sub->flag),
						sub->srcname);
			else
				printf("  SUB: %s %s | %s\n", getletters(sub->flag),
						sub->srcname, sub->tgtname);
			showfiles(NULL, sub->filehead, sub->xcldhead, "  ");
			sub = sub->next;
		}
		dir = dir->next;
	}
}


void showfiles(
	FILB *gblx,
	FILB *file,
	FILB *xcld,
	char *indent)
{
	while (gblx != NULL)
	{
		printf("  %sGBLX: %s %s\n", indent, getletters(gblx->flag),
				gblx->srcname);
		gblx = gblx->next;
	}
	while (file != NULL)
	{
		if (file->srcname == file->tgtname)
			printf("  %sFILE: %s %s\n", indent, getletters(file->flag),
					file->srcname);
		else
			printf("  %sFILE: %s %s | %s\n", indent, getletters(file->flag),
					file->srcname, file->tgtname);
		file = file->next;
	}
	while (xcld != NULL)
	{
		printf("  %sXCLD: %s %s\n", indent, getletters(xcld->flag),
				xcld->srcname);
		xcld = xcld->next;
	}
}


static char *getletters(
	int flags)
{
	static char text[4];
	static char typetbl[16] =
	{	'B', 'O', 'A', 'D', '?', '?', '?', '?',
		'R', 'S', 'F', 'X', '?', '?', '?', 'E'
	};

	text[0] = typetbl[flags & IF_TYPE];
	text[1] = ((flags & (IF_COPY|IF_ISET)) == (IF_COPY|IF_ISET)) ? 'b' :
			(flags & IF_COPY) ? 'c' : (flags & IF_ISET) ? 'i' : ' ';
	return (text);
}


//*****************************************************************
// Function: inserror - Complain about INS file error and terminate
// Returned: Does not return
//*****************************************************************

void inserror(
	char *msg,
	int   linenum)
{
	fprintf(stderr, "? MKINSTALL: Error in INS file at line %d\n"
			"             %s\n", linenum, msg);
	exit(1);

}
