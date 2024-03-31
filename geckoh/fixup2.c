#include "stdio.h"
#include "errno.h"
#include "ctype.h"
#include "string.h"

FILE *ifile;
FILE *ofile;

char  ibuffer[100];
char  name[32];

char *ipnt;
char *npnt;

int main(void)
{
	int cnt;
	uchar chr;

	if ((ifile = fopen("xxx.c", "r")) == NULL)
	{
		fputs("Cannot open input file\n", stderr);
		return (1);
	}

	if ((ofile = fopen("head.c", "w")) == NULL)
	{
		fputs("Cannot open output file\n", stderr);
		return (1);
	}

	cnt = 0;

	while (fgets(ibuffer, sizeof(ibuffer), ifile) != NULL)
	{
		ipnt = ibuffer;
		npnt = name;
		while ((chr = *ipnt++) != 0 && chr != '\n' && chr != ';')
			*npnt++ = chr;
		*npnt = 0;

		if (--cnt <= 0)
		{
			cnt = 5;
			fputs("\nRFUNC", ofile);
		}
		fprintf(ofile, "%s %s", (cnt == 5) ? "" : ",", name);
	}

	if ((ifile = fopen("xxx.c", "r")) == NULL)
	{
		fputs("Cannot open input file\n", stderr);
		return (1);
	}

	if ((ofile = fopen("body.c", "w")) == NULL)
	{
		fputs("Cannot open output file\n", stderr);
		return (1);
	}

	while (fgets(ibuffer, sizeof(ibuffer), ifile) != NULL)
	{
		ipnt = ibuffer;
		npnt = name;
		while ((chr = *ipnt++) != 0 && chr != '\n' && chr != ';')
			*npnt++ = chr;
		*npnt = 0;
		fputs("\n//***************************************\n", ofile);
		fprintf(ofile, "// Function: %s -\n", name);
		fputs("// Returned: Return code\n", ofile);
		fputs("//***************************************\n\n", ofile);
		fprintf(ofile, "int %s(void)\n{\n\n\treturn (RTN_DONE);\n}\n\n", name);

	}



	return (0);
}
