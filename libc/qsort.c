// ++++
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


#define xchng(a,b,t) \
	t = *a; \
	*a = *b; \
	*b = t;

/// static void dump(int flag, long *base, int num, int size);

static void qsort4(long *base, size_t num,
		int (*cmp)(const void *, const void *));
static void qsortn(char *base, size_t num, size_t size,	
		int (*cmp)(const void *, const void *));


//******************************************************
// Function: qsort - General purpose quick-sort function
// Returned: Nothing
//******************************************************

void qsort(
	void  *base,
	size_t num,
	size_t size,
	int  (*cmp)(const void *, const void *))

{
	char *xp;

	if (num > 1)
	{
		if (size == 4)
			qsort4(base, num, cmp);
		else
			qsortn(base, num, size, cmp);

///		dump('E', base, num, size);

		xp = base;
		while (--num > 0)
		{
			if ((*cmp)(xp, xp + size) > 0)
			{
				printf("!!! qsort failed\n");
				break;
			}
			xp += size;
		}
	}

}


//************************************************************
// Function: qsort4 - Internal quick-sort function to sort
//				arrays with 4-byte elements (usually pointers)
// Returned: Nothing
//************************************************************

// We special-case this to be a little faster since its the
//   most common kind of sort

static void qsort4(
	long  *base,
	size_t num,
	int  (*cmp)(const void *, const void *))

{
	long *hp;
	long *lp;
	long *pp;

	long  temp;
	int   cnt;

///	dump('Q', base, num, 4);

	lp = base;
	hp = lp + num - 1;
	pp = lp;
	while (lp < hp)
	{
///		printf("DN: %d(%d) %d(%d) PV: %d\n", lp - base, *lp, hp - base,
///				*hp, *pp);

		while (lp < hp && (*cmp)(lp, pp) <= 0)
			lp++;

///		printf("UP: %d(%d) %d(%d) PV: %d\n", lp - base, *lp, hp - base,
///				*hp, *pp);

		while ((*cmp)(hp, pp) > 0)
			hp--;

///		printf("XX: %d(%d) %d(%d) PV: %d\n", lp - base, *lp, hp - base,
///				*hp, *pp);

		if (lp < hp)
		{
///			printf("** %d with %d\n", lp - base, hp - base);

			if (lp == base)
				pp = hp;
			xchng(hp, lp, temp);
		}

///		dump(' ', base, num, 4);

	}

///	printf("== %d with %d\n", lp - base, hp - base);

	xchng(base, hp, temp);

///	dump(' ', base, num, 4);

	if ((cnt = hp - base) > 1)
		qsort4(base, cnt, cmp);
	if ((cnt = num - (hp - base) - 1) > 1)
		qsort4(hp + 1, cnt, cmp);
}


//********************************************************
// Function: qsort4 - Internal quick-sort function to sort
//				arrays with any size elements
// Returned: Nothing
//********************************************************

static void qsortn(
	char  *base,
	size_t num,
	size_t size,	
	int  (*cmp)(const void *, const void *))

{
	char *hp;
	char *lp;
	char *pp;
	char *thp;
	char *tlp;
	int   cnt;
	char  temp;

	lp = base;
	hp = lp + size * (num - 1);
	pp = lp;
	while (lp < hp)
	{
		while (lp < hp && (*cmp)(lp, pp) <= 0)
			lp += size;;
		while ((*cmp)(hp, pp) > 0)
			hp -= size;;
		if (lp < hp)
		{
			if (lp == base)
				pp = hp;
			thp = hp;
			tlp = lp;
			cnt = size;
			do
			{
				xchng(thp, tlp, temp);
				thp++;
				tlp++;
			} while (--cnt > 0);
		}

		thp = hp;
		tlp = base;
		cnt = size;
		do
		{
			xchng(thp, tlp, temp);
			thp++;
			tlp++;
		} while (--cnt > 0);
    }
	if ((cnt = (hp - base)/size) > 1)
		qsortn(base, cnt, size, cmp);
	if ((cnt = num - (hp - base)/size - 1) > 1)
		qsortn(hp + size, cnt, size, cmp);
}


/*
static void dump(
	int   flag,
	long *base,
	int   num,
	int   size)

{
	long *lp;

	lp = (long *)base;
	printf("%c%2d/%d:", flag, num, size);
	while (--num >= 0)
	{
		printf(" %2d", *lp);
		lp++;
	}
	printf("\n");
}
*/
