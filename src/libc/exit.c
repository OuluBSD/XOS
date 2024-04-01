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
#include <io.h>
#include <xos.h>
#include <errno.h>
#include <string.h>
#include <xossvc.h>


__EXITLIST *__exitlist;


//***********************************
// Function: exit - Terminate process
// Returned: Does not return
//***********************************

void exit(
	int status)
{
	FILE *pnt;
	int   cnt;

	while (__exitlist != NULL)
	{
		(__exitlist->func)();
		__exitlist = __exitlist->next;
	}
	cnt = 6;							// Close the static streams
	pnt = __iob;
	do
	{
		if (pnt->_handle > 0)
			fclose(pnt);
		pnt++;
	} while (--cnt > 0);
	while (__filehead != NULL)			// Close the allocated streams
	{
		if (__filehead->_handle > 0)
			fclose(__filehead);
		else
			__filehead = __filehead->_next;
	}
	svcSchExit(status);					// Terminate the process
}
