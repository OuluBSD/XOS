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


#include "usbctl.h"


long memtotal;
long memblks;


uchar *usbmalloc(
	int size)

{
	uchar *pnt;

	size = (size + 3) & 0xFFFFFFFC;
	if ((pnt = malloc(size + 12)) == NULL)
		return (NULL);
	memtotal += size;
	memblks++;
	*(long *)(pnt) = 0x5AC369E7;
	*(long *)(pnt + 4) = size;
	*(long *)(pnt + 8 + size) = 0x9E7335AC;
	return (pnt + 8);
}

void usbfree(
	void *pnt)

{
	long size;

	if (*(long *)(((uchar *)pnt) - 8) != 0x5AC369E7)
		BREAK();
	size = *(long *)(((uchar *)pnt) - 4);
	if (*(long *)(((uchar *)pnt) + size) != 0x9E7335AC)
		BREAK();
	memtotal -= size;
	memblks--;
	free(((uchar *)pnt) - 8);
}


void debugmem(void)

{
///	debugprint("MEMORY: %7d%6d", memtotal, memblks);
}
