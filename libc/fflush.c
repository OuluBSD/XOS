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


static int dofflush(FILE *file);


int fflush(
	FILE *file)
{
	int cnt;

	if (file != NULL)
		return (dofflush(file));
	file = __iob;
	cnt = 6;
	do
	{
		dofflush(file++);
	} while (--cnt > 0);

	file = __filehead;
	while (file != NULL)
	{
		if (dofflush(file) < 0)
			return (EOF);
		file = file->_next;
	}
	return (0);
}


static int dofflush(
	FILE *file)
{
	long rtn;

	file->_ungotten = 0;
	if (file->_handle > 0 && (file->_flag & _WRITE) &&
			(file->_flag & _IONBF) == 0)
	{
		if ((file->_flag & _DIRTY) && __writeblk(file) < 0)
			return (-1);
		if ((rtn = sysIoCommit(file->_handle)) < 0)
		{
			errno = -rtn;
			return (-1);
		}
	}
	return (0);
}
