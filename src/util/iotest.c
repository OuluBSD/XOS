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
#include <ctype.h>
#include <errno.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.H>
#include <xosstr.h>
#include <global.h>

struct
{   BYTE1PARM  fileatr;
    char       end;
} openparms =
{   {PAR_SET|REP_HEXV, 1, IOPAR_SRCATTR, XA_NORMAL|XA_DIRECT},
	0
};

FILE *file;
uchar data[4096];
char  prgname[] = "IOTEST";

int main(
	int   argc,
	char *argv[])

{   
	int num;

	if (argc != 2)
	{
		fputs("IOTEST: Invalid arguments, usage is:\n          iotest num\n",
				stderr);
		return (1);
	}
	num = atol(argv[1]);
	switch (num)
	{
	 case 1:
		printf("Test 1: Random update before end, < 1 block\n");
		if ((file = fopen("test1.dat", "w+")) == NULL)
			femsg2(prgname, "Error opening test1.dat for update", -errno,
					NULL);
		{
			setdata(0, 64);
			if (fwrite(data, 1, 128, file) < 0)
				femsg2(prgname, "Error writing data\n", -errno, NULL);
			printf("FIle written\n");
			if (fseek(file, 16, 0))
				femsg2(prgname, "Error setting position\n", -errno, NULL);
			printf("Position set\n");
			data[0] = 0xFF;
			if (fwrite(data, 1, 1, file) < 0)
				femsg2(prgname, "Error updating data\n", -errno, NULL);
			printf("File updated\n");
			if (fclose(file) < 0)
				femsg2(prgname, "Error closing file\n", -errno, NULL);
			printf("File closed\n");
		}
		break;

	 case 2:
		printf("Test 2: Random update and extend, < 1 block\n");
		if ((file = fopen("test2.dat", "w+")) == NULL)
			femsg2(prgname, "Error opening test2.dat for update", -errno,
					NULL);
		{
			setdata(0, 64);
			if (fwrite(data, 1, 128, file) < 0)
				femsg2(prgname, "Error writing data\n", -errno, NULL);
			printf("File written\n");

			if (fseek(file, 511, 0))
				femsg2(prgname, "Error setting position\n", -errno, NULL);
			printf("Position set\n");
			data[0] = 0x33;
			data[1] = 0x44;
			if (fwrite(data, 1, 2, file) < 0)
				femsg2(prgname, "Error updating data\n", -errno, NULL);
			printf("File updated\n");

			if (fseek(file, 1024 + 16, 0))
				femsg2(prgname, "Error setting position\n", -errno, NULL);
			printf("Position set\n");
			data[0] = 0xFF;
			if (fwrite(data, 1, 1, file) < 0)
				femsg2(prgname, "Error updating data\n", -errno, NULL);
			printf("File updated\n");

			if (fclose(file) < 0)
				femsg2(prgname, "Error closing file\n", -errno, NULL);
			printf("File closed\n");
		}
		break;

	 case 3:
		printf("Test 3: Random update and extend, > 1 block\n");
		if ((file = fopen("test3.dat", "w+")) == NULL)
			femsg2(prgname, "Error opening test3.dat for update", -errno,
					NULL);
		{
			setdata(0, 64);
			if (fwrite(data, 1, 128, file) < 0)
				femsg2(prgname, "Error writing data\n", -errno, NULL);
			printf("FIle written\n");
			if (fseek(file, 8192 + 16, 0))
				femsg2(prgname, "Error setting position\n", -errno, NULL);
			printf("Position set\n");
			data[0] = 0xFF;
			if (fwrite(data, 1, 1, file) < 0)
				femsg2(prgname, "Error updating data\n", -errno, NULL);
			printf("File updated\n");
			if (fclose(file) < 0)
				femsg2(prgname, "Error closing file\n", -errno, NULL);
			printf("File closed\n");
		}
		break;

	 case 4:
		printf("Test 4: Random update and extend to block end\n");
		if ((file = fopen("test4.dat", "w+")) == NULL)
			femsg2(prgname, "Error opening test4.dat for update", -errno,
					NULL);
		{
			setdata(0, 64);
			if (fwrite(data, 1, 128, file) < 0)
				femsg2(prgname, "Error writing data\n", -errno, NULL);
			printf("FIle written\n");
			if (fseek(file, 8192, 0))
				femsg2(prgname, "Error setting position\n", -errno, NULL);
			printf("Position set\n");
			data[0] = 0xFF;
			if (fwrite(data, 1, 1, file) < 0)
				femsg2(prgname, "Error updating data\n", -errno, NULL);
			printf("File updated\n");
			if (fclose(file) < 0)
				femsg2(prgname, "Error closing file\n", -errno, NULL);
			printf("File closed\n");
		}
		break;

	 case 5:
		printf("Test 5: Random update at block end\n");
		if ((file = fopen("test5.dat", "w+")) == NULL)
			femsg2(prgname, "Error opening test4.dat for update", -errno,
					NULL);
		{
			setdata(0, 64);
			if (fwrite(data, 1, 128, file) < 0)
				femsg2(prgname, "Error writing data\n", -errno, NULL);
			printf("FIle written\n");
			if (fseek(file, 4095, 0))
				femsg2(prgname, "Error setting position\n", -errno, NULL);
			printf("Position set\n");
			data[0] = 0x33;
			data[1] = 0x44;
			if (fwrite(data, 1, 2, file) < 0)
				femsg2(prgname, "Error updating data\n", -errno, NULL);
			printf("File updated\n");
			if (fclose(file) < 0)
				femsg2(prgname, "Error closing file\n", -errno, NULL);
			printf("File closed\n");
		}
		break;

	 default:
		printf("Invalid test number %d\n", num);
		return (1);
	}
	return (0);
}



void setdata(
	int begin,
	int cnt)
{
	ushort *pnt;

	pnt = (ushort *)data;
	do
	{
		*pnt++ = begin++;
	} while (--cnt > 0);
}
