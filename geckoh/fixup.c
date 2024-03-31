#include "stdio.h"
#include "errno.h"
#include "ctype.h"
#include "string.h"

FILE *ifile;
FILE *ofile;

char  ibuffer[100];
char  obuffer[100];
char  names[32][400];

char *ipnt;
char *opnt;
char *npnt;
char *nnxt = names[0];

int   nnum = 0;


int main(void)
{
	int  cnt;
	char chr;

	if ((ifile = fopen("otable.c", "r")) == NULL)
	{
		fputs("Cannot open input file\n", stderr);
		return (1);
	}

	if ((ofile = fopen("ntable.c", "w")) == NULL)
	{
		fputs("Cannot open output file\n", stderr);
		return (1);
	}

	while (fgets(ibuffer, sizeof(ibuffer), ifile) != NULL)
	{
		ipnt = ibuffer;
		opnt = obuffer;

		while ((chr = *ipnt) != 0 && isspace(chr))
		{
			*opnt++ = chr;
			ipnt++;
		}

		while ((chr = *ipnt) != 0 && !isspace(chr) && chr != ',')
		{
			*opnt++ = chr;
			ipnt++;
		}

		*opnt++ = '"';

		while ((chr = *ipnt) != 0 && chr != '"')
		{
			*opnt++ = chr;
			ipnt++;
		}

		while ((chr = *ipnt) != 0 && !isspace(chr) && chr != ',')
		{
			*opnt++ = chr;
			ipnt++;
		}

		*opnt++ = '"';

		cnt = 6;
		do
		{
			while ((chr = *ipnt) != 0 && chr != ',')
			{
				*opnt++ = chr;
				ipnt++;
			}
			*opnt++ = *ipnt++;
			*opnt++ = *ipnt++;
			*opnt++ = '0';
			*opnt++ = 'x';
		} while (--cnt > 0);


		while ((chr = *ipnt++) != 0 && chr != ',')
			*opnt++ = chr;
		*opnt++ = chr;
		*opnt++ = *ipnt++;	

		npnt = nnxt;
		while ((chr = *ipnt) != 0 && chr != '\n')
		{
			*opnt++ = chr;
			ipnt++;
			*npnt++ = chr;
		}
		*npnt = 0;

		cnt = nnum;

		npnt = names[0];

		while (--cnt >= 0)
		{
			if (strcmp(npnt, nnxt) == 0)
				break;
			npnt += 32;
		}
		if (cnt < 0)
		{
			printf("Have new name: |%s|\n", nnxt);
			nnxt += 32;
			nnum++;
		}
		else
			printf("Have dup name: |%s|\n", nnxt);

///		fgets(ibuffer, 10, stdin);

		*opnt++ = '}';
		*opnt++ = ',';
		*opnt++ = '\n';
		*opnt++ = 0;
		fputs(obuffer, ofile);
	}

	printf("Found %d names", nnum);

	cnt = 0;
	npnt = names[0];
	while (--nnum >= 0)
	{
		if (--cnt < 0)
		{
			cnt = 5;
			fputs(";\nextern OFUNC ", ofile);
		}
		else
			fputs(", ", ofile);
		fputs(npnt, ofile);
		npnt += 32;
	}

	fclose(ifile);
	fclose(ofile);
	return (0);
}
