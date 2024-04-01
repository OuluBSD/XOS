//--------------------------------------------------------------------------*
// DELAY.C
// Command to delay for N seconds
//
// Written by: John R. Goltz
//
// Edit History:
//-------------------------------------------------------------------------*

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

// Command format:
//      DELAY seconds

#include <stdio.h>
#include <stdlib.h>
#include <xos.h>
#include <xostime.h>
#include <xossvc.h>


#define VERSION 1
#define EDITNO  0

///char prgname[] = "DELAY";

int main(
	int   argc, 
	char *argv[])

{
	char *end;
	long  time;

	if (argc != 2)
	{
		fputs("? DELAY: Command error, usage is:\n           DELAY "
				"milliseconds\n", stderr);
		exit(1);
	}
	time = strtol(argv[1], &end, 0);
	if (*end != 0 && *end != '\n')
	{
		fputs("? DELAY: Invalid numeric value\n", stderr);
		exit(1);
	}
	if (time < 1 || time > 1000000)
	{
		fputs("? DELAY: Amount to delay must be between 1 and 1000000 "
				"milliseconds\n", stderr);
		exit(1);
	}
	svcSchSuspend(NULL, time * ST_MILLISEC);
	return (0);
}
