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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <xos.h>
#include <xoserrmsg.h>
#include <xoserr.h>
#include <xcstring.h>
#include <xostime.h>
#include <xossvc.h>
#include <xosxws.h>

#define VERSNO 3
#define EDITNO 0

#if 0
#define MT_XOS 1
#define MT_MS  2

long  seccnt;
ulong offset;
long  symnum;
long  namenum;
long  segnum;
char *tname[] =
{   "not specified",
    "Data",
    "Code",
    "Stack",
    "Combined"
};
int   byte1;
int   byte2;
int   filetype;
int   modtype;
char  filespec[100];
char  name[258];
char  lowfirst;
char  eofok;
#endif

FILE *fp;
char  prgname[] = "PTRDMP";

struct
{	long prefix;
	long type;
	long length;
}     header;


int  getbyte(void);
void dump(char *label, uchar *pnt, int cnt);


//**********************************
// Function: main - Main program
// Returned: 0 if normal, 1 if error
//**********************************

main(
    int   argc,
    char *argv[])

{
	char *pnt;
	char *cpypnt;
	char *label;
	long  rtn;
///	long  cnt;
	long  cpycnt;
	long  value;
	long  linenum;
///	long  amnt;
	int   leadin;
	int   prefix;
	char  hdrrec[512];
	char  buffer[20];
	char  chr;

    printf("PTRDMP - version %d.%d\n", VERSNO, EDITNO);
    if (argc != 2)
    {
        fputs("? PTRDMP: Command error, correct usage is:\n\
              RELDMP filename\n", stderr);
        exit(1);
    }
	if ((fp = fopen(argv[1], "rb")) == NULL)
        femsg2(prgname, "Error opening input file", -errno, NULL);

	// Read the file prefix and the header for the first record (which should
	//   be a header record)

	if ((fread((char *)&header, 1, sizeof(header), fp))< 0)
		femsg2(prgname, "Error reading file header", -errno, NULL);

	if (header.prefix != '@RTP')
	{
		fputs("? File prefix is incorrect\n", stdout);
		exit(1);
	}
	if (header.type != PTRRECTYPE_HEADER)
	{
		fputs("? First record is not a HEADER record\n", stdout);
		exit(1);
	}
	if (header.length > sizeof(hdrrec))
	{
		fputs("? Header record is too long\n", stdout);
		exit(1);
	}

	// Read the header record

	if ((rtn = fread(hdrrec, 1, header.length, fp)) < 0)
		femsg2(prgname, "Error reading header record", -errno, NULL);

	// The first line in the header record is the job name

	pnt = hdrrec;
	while (--header.length >= 0 && (chr = *pnt) != 0 && chr != '\n')
	{
		if (chr == '\r')
			*pnt = 0;
		pnt++;
	}
	*pnt++ = 0;

	// The second line contains the copy count

	cpypnt = pnt;
	while (--header.length >= 0 && (chr = *pnt) != 0 && chr != '\n')
	{
		if (chr == '\r')
			*pnt = 0;
		pnt++;
	}
	*pnt++ = 0;
	if ((cpycnt = atoi(cpypnt)) < 1)
		cpycnt = 1;

	printf("Print job: %s, copies: %d\n", hdrrec, cpycnt);

	// Read and display the data records

	while (errno = 0, (rtn = fread((char *)&header.type, 1, 8, fp)) == 8)
	{
		printf("Data record: %d\n", header.length);
		linenum = 0;
		while (header.length > 0)
		{
			pnt = buffer;
			label = "Text";
			while ((header.length > 0 && (chr = getbyte()) != 0x1B))
			{
				*pnt++ = chr;
				if (pnt == (buffer + 16))
				{
					dump(label, buffer, 16);
					label = NULL;
					pnt = buffer;
				}
			}
			if (pnt != buffer)
				dump(label, buffer, pnt - buffer);
			if (header.length <= 0)
				break;

			// Here with ESC

			if ((leadin = getbyte()) >= '0') // Single character sequence?
			{
				printf("Single: %c\n", leadin);
				continue;
			}

			// If have a multi-character sequence

			prefix = getbyte();

			while (TRUE)
			{
				value = 0;
				while (isdigit((chr = getbyte())))
					value = value * 10 + (chr & 0x0F);

				switch ((((leadin << 8) + prefix) << 8) + toupper(chr))
				{
				 case '*bM':
					printf("Set compression: *b%dM\n", value);
					break;

				 case '*rA':
					printf("Start raster: *r%dA\n", value);
					break;

				 case '*rB':
					printf("End raster: *r%dB\n", value);
					break;

				 case '*rC':
					printf("End raster: *r%dC\n", value);
					break;

				 case '*bY':
					printf("Skip: *b%dY\n", value);
					linenum += value;
					break;

				 case '*bW':
					sprintf(buffer, "Line %d (*b%dW)", linenum++, value);
					dump(buffer, NULL, value);
					break;

				 default:
					printf("??? %c %c %d %c\n", leadin, prefix, value, chr);
					break;
				}

				if (isupper(chr))
					break;
			}

		}
	}
	if (errno != 0 || rtn > 0)
	{
		if (errno == 0)
			errno = -ER_EOF;
		femsg2(prgname, "Error reading record header", -errno, NULL);
	}
	printf("Finished\n");
}


int getbyte(void)
{
	int chr;

	if (--(header.length) < 0)
		femsg2(prgname, "Record overrun", 0, NULL);
	errno = 0;
	if ((chr = fgetc(fp)) < 0)
	{
		if (errno == 0)
			errno = -ER_EOF;
		femsg2(prgname, "Error reading data", -errno, NULL);
	}
	return (chr);
}


void dump(
	char  *label,
	uchar *pnt,
	int    cnt)
{
	char *dpnt;
	char *tpnt;
	int   cnt2;
	int   chr;
	char  buffer[64];
	char  text[20];

	if (label != NULL)
		printf("%s:\n", label);
	while (cnt > 0)
	{
		dpnt = buffer;
		tpnt = text;
		cnt2 = 16;
		do
		{
			chr = (pnt != NULL) ? *pnt++ : getbyte();
			dpnt += sprintf(dpnt, " %02X", chr);
			*tpnt++ = (chr > ' ' && chr < 0x7F) ? chr : '.';
		} while (--cnt > 0 && --cnt2 > 0);
		*tpnt = 0;
		printf("  %-50s|%s|\n", buffer, text);
	}
}
