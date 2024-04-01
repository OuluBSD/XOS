//--------------------------------------------------------------------------*
// hndltest.c
// Program to test lots of open devices
//
// Written by: John R. Goltz
//
//-------------------------------------------------------------------------*/

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
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.H>
#include <xosstr.h>
#include <global.h>
#include <progarg.h>
#include <proghelp.h>

char    prgname[] = "ENVTEST";	// Our programe name


void listenv();


int main(
	int   argc,
	char *argv[])
{   
	sysEnvDefine(0, "abc=This is the first string");
	listenv();

	sysEnvDefine(0, "xyz=This is the second string");
	listenv();

	sysEnvDefine(0, "opq=This is the third string");
	listenv();

	sysEnvDefine(0, "xyz=This one has been made longer 123456");
	listenv();

	sysEnvDefine(0, "xyz=Now it's shorter");
	listenv();

	INT3;

	sysEnvDefine(0, "xyz=");
	listenv();

	INT3;

	return (0);
}



void listenv()
{
	char *pnt;
	long  rtn;
	char  buffer[2000];

	printf("Current environment:\n");
	if ((rtn = sysEnvGetAll(buffer, 2000)) < 0)
	{
		printf("? Error %d\n", rtn);
		return;
	}
	pnt = buffer;
	while (*pnt != 0)
		pnt += (printf("  %s\n", pnt) - 3 + 1);
}
