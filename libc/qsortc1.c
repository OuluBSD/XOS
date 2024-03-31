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


void qsort(
	void  *base,
	size_t num,
	size_t size,
	int  (*cmp)(const void *, const void *))

{
	char *up;
	char *dn;
	char *pp;
	char *tup;
	char *tdn;
	long  pv;
	union
	{	long l;
		char c;
	}     temp;
	int   cnt;

	printf("Q%2d:", num);
	cnt = num;
	pp = base;
	while (--cnt >= 0)
	{
		printf(" %2d", *(long *)pp);
		pp += size;
	}
	printf("\n");

	if (num > 1)
	{
		dn = base;
		up = dn + size * (num - 1);
		if (size == 4)
		{
			pv = *(long *)base;
			while (dn < up)
			{
				printf("DN: %d(%d) %d(%d) PV: %d\n", (dn - base)/size, *dn,
						(up - base)/size, *up, pv);

				while (dn < up && (*cmp)(dn, &pv) <= 0)
					dn += size;

				printf("UP: %d(%d) %d(%d) PV: %d\n", (dn - base)/size, *dn,
						(up - base)/size, *up, pv);

				while ((*cmp)(up, &pv) > 0)
					up -= size;

				printf("XX: %d(%d) %d(%d) PV: %d\n", (dn - base)/size, *dn,
						(up - base)/size, *up, pv);

				if (dn < up)
				{
					printf("** %d with %d\n", (dn - base)/size,
							(up - base)/size);

					temp.l = *(long *)up;
					*(long *)up = *(long *)dn;
					*(long *)dn = temp.l;
				}

				printf("%3d:", num);
				cnt = num;
				pp = base;
				while (--cnt >= 0)
				{
					printf(" %2d", *pp);
					pp += size;
				}
				printf("\n");

			}

			printf("** %d with %d\n", (dn - base)/size,
					(up - base)/size);

			*(long *)base = *(long *)up;
			*(long *)up = pv;

			printf("%3d:", num);
			cnt = num;
			pp = base;
			while (--cnt >= 0)
			{
				printf(" %2d", *(long *)pp);
				pp += size;
			}
			printf("\n");

    	}
		else
		{
			pp = base;
			while (dn < up)
			{
				while (dn < up && (*cmp)(dn, pp) <= 0)
					dn += size;

				while (dn < up && (*cmp)(pp, up) <= 0)
					up -= size;

				if (dn < up)
				{
					tup = up;
					tdn = dn;
					cnt = size;
					do
					{
						temp.c = *tup;
						*tup = *tdn;
						*tdn = temp.c;
						tup++;
						tdn++;
					} while (--cnt > 0);
				}
			}
		}
		qsort(base, (up - base)/size, size, cmp);
		qsort(up + size, num - (up - base)/size - 1, size, cmp);
	}

	if (num > 1)
	{
		printf("E%2d:", num);
		cnt = num;
		pp = base;
		while (--cnt >= 0)
		{
			printf(" %2d", *(long *)pp);
			pp += size;
		}
		printf("\n");

		pp = base;
		while (--num > 0)
		{
			if ((*cmp)(pp, pp + size) > 0)
			{
				printf("!!! qsort failed\n");
				break;
			}
			pp += size;
		}
	}
}
