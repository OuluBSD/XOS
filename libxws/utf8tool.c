#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


void main(
    int   argc,
    char *argv[])

{
	char *endp;
	long  code;

	if (argc != 2)
	{
		fputs("? Command error, usage is:\n    UTF8TOOL charcode\n", stderr);
		exit(1);
	}
	code = strtol(argv[1], &endp, 16);

	printf("code = 0x%X\n", code);

	if (*endp != 0 || code > 0x0010FFFF)
	{
		fputs("? UTFTOOL: Invalid value\n", stderr);
		exit(1);
	}
	if (code <= 0x7F)
		printf("UTF8 value: %02X\n", code);
	else if (code <= 0x1FFF)
		printf("UTF8 value: %02X %02X\n", 0xC0 | (code >> 6),
				0x80 | (code & 0x3F));
	else if (code <= 0xFFFF)
		printf("UTF8 value: %02X %02X %02X\n", 0xE0 | (code >> 12),
				0x80 | ((code >> 6) & 0x3F), 0x80 | (code & 0x3F));
	else
		printf("UTF8 value: %02X %02X %02X %02X\n", 0xF0 | (code >> 18),
				0x80 | ((code >> 12) & 0x3F), 0x80 | ((code >> 6) & 0x3F),
				0x80 | (code & 0x3F));
	exit(0);
}
