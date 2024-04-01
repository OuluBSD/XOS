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
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xosrtn.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <errno.h>
#include <zlib.h>


#define INBUFRSZ  0x4000
#define OUTBUFRSZ 0x5000

char    *inbufr;
char    *outbufr;
z_stream stream;
long     amnt;
long     ihndl;
long     ohndl;
char     prgname[] = "TEST1";
char     more;


long getinput(char *bufr, long size, char *more);


void main(
	int   argc,
	char *argv[])
{
	long rtn;

	if ((ihndl = svcIoOpen(XO_IN, argv[1], NULL)) < 0)
		femsg2(prgname, "Error opening input file", ihndl, NULL);

	if ((ohndl = svcIoOpen(XO_OUT|XO_TRUNCA|XO_CREATE, "testa", NULL)) < 0)
		femsg2(prgname, "Error opening output file (testa)", ohndl, NULL);

	if ((inbufr = malloc(INBUFRSZ)) == NULL ||
			(outbufr = malloc(OUTBUFRSZ)) == NULL)
		femsg2(prgname, "Error allocating buffer", -errno, NULL);

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    stream.next_in = inbufr;
    stream.avail_in = INBUFRSZ;

    stream.next_out = outbufr;
    stream.avail_out = OUTBUFRSZ;

	if ((rtn = deflateInit(&stream, 9)) != Z_OK)
	{
		fprintf(stderr, "deflateInit returned %d\n", rtn);
		exit(1);
	}
	more = TRUE;
	do
	{
		if ((rtn = svcIoInBlock(ihndl, inbufr, INBUFRSZ)) != INBUFRSZ)
		{
			if (rtn < 0 && rtn != ER_EOF)
				femsg2(prgname, "Error reading input file", rtn, NULL);
			if (rtn == 0)
				break;
			more = FALSE;
		}

		printf("### read %d\n", rtn);

	    stream.next_in = inbufr;
    	stream.avail_in = rtn;
		do
		{
			printf("### call deflate with %d, more=%d\n", stream.avail_in,
					more);

		    stream.next_out = outbufr;
    		stream.avail_out = OUTBUFRSZ;
			if ((rtn = deflate(&stream, (more) ? Z_NO_FLUSH : Z_FINISH)) < 0)
			{
				fprintf(stderr, "deflate returned %d\n", rtn);
				exit(1);
			}

			fprintf(stderr, "deflate returned %d\n", rtn);

			amnt = OUTBUFRSZ - stream.avail_out;			

			printf("### deflate generated %d\n", amnt);

			if ((amnt = svcIoOutBlock(ohndl, outbufr, amnt)) < 0)
				femsg2(prgname, "Error writing output file", amnt, NULL);
		} while (stream.avail_in != 0 || (!more && rtn != Z_STREAM_END));
	} while (more);

/*
    stream.next_in = inbufr;
	stream.avail_in = 0;
	if ((rtn = deflate(&stream, Z_FINISH)) != Z_STREAM_END)
	{
		fprintf(stderr, "deflate (FINISH) returned %d\n", rtn);
		exit(1);
	}
	amnt = OUTBUFRSZ - stream.avail_out;			

	printf("### final deflate generated %d\n", amnt);
*/

	if ((rtn = svcIoOutBlock(ohndl, outbufr, amnt)) < 0)
		femsg2(prgname, "Error writing output file (testa)", rtn, NULL);
	if ((rtn = svcIoClose(ohndl, 0)) < 0)
		femsg2(prgname, "Error closing output file (testa)", rtn, NULL);
	if ((rtn = deflateEnd(&stream)) != Z_OK)
	{
		fprintf(stderr, "deflateEnd returned %d\n", rtn);
		exit(1);
	}

	// Deflate complete - now inflate

	if ((ihndl = svcIoOpen(XO_IN, "testa", NULL)) < 0)
		femsg2(prgname, "Error opening input file (testa)", ihndl, NULL);

	if ((ohndl = svcIoOpen(XO_OUT|XO_TRUNCA|XO_CREATE, "testb", NULL)) < 0)
		femsg2(prgname, "Error opening output file (testb)", ohndl, NULL);

	more = TRUE;
	if ((rtn = svcIoInBlock(ihndl, inbufr, INBUFRSZ)) != INBUFRSZ)
	{
		if (rtn < 0 && rtn != ER_EOF)
			femsg2(prgname, "Error reading input file (testa)", rtn, NULL);
		if (rtn == 0)
		{
			fputs("Input file is 0 length\n", stderr);
			exit(1);
		}
		more = FALSE;
	}
    stream.next_in = inbufr;
   	stream.avail_in = rtn;

	if ((rtn = inflateInit(&stream)) != Z_OK)
	{
		fprintf(stderr, "inflateInit returned %d\n", rtn);
		exit(1);
	}
	do
	{
		amnt = getinput(outbufr, OUTBUFRSZ, &more);

		printf("### getinput returned %d\n", amnt);

		if ((rtn = svcIoOutBlock(ohndl, outbufr, amnt)) < 0)
			femsg2(prgname, "Error writing output file (testb)", rtn, NULL);
	} while (amnt == OUTBUFRSZ);
	if ((ohndl = svcIoClose(ohndl, 0)) < 0)
		femsg2(prgname, "Error closing output file (testb)", ohndl, NULL);

	printf("Finished\n");
	exit(0);
}


long getinput(
	char *bufr,
	long  size,
	char *more)
{
	long amnt;
	long rtn;

	stream.next_out = bufr;
	stream.avail_out = size;

	amnt = 0;
	while (size > 0)
	{
		if (*more)
		{
			if (stream.avail_in == 0)
			{
				if ((rtn = svcIoInBlock(ihndl, inbufr, INBUFRSZ)) < 0)
				{
					if (rtn != ER_EOF)
						femsg2(prgname, "Error reading input file (testa)", rtn,
								NULL);
					*more = FALSE;
					rtn = 0;
				}

				printf("### reading input, have %d\n", rtn);

		    	stream.next_in = inbufr;
		   		stream.avail_in = rtn;
			}
		}

		printf("### calling inflate, %d available\n", stream.avail_in);

		if ((rtn = inflate(&stream, Z_NO_FLUSH)) < 0)
		{
			fprintf(stderr, "inflate returned %d\n", rtn);
			exit(1);
		}

		printf("### inflate generated %d\n", size - stream.avail_out);

		amnt += (size - stream.avail_out);
		size -= (size - stream.avail_out);
		if (rtn == Z_STREAM_END)
			break;
	}
	return (amnt);
}
