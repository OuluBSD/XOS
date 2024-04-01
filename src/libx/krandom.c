#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <xos.h>
#include <xossvc.h>
#include <xosdisk.h>
#include <xostime.h>


static long seedx = 123456789;
static long seedy = 363436000;
static long seedz = 521288629;
static long seedc = 7654321;


ulong krandom(void);

//***********************************
// Function: krseed - Set seed values
// Returned: Nothing
//***********************************

void krseed(
	long sdx,
	long sdy,
	long sdz,
	long sdc)
{
	time_s curdt;

	if (sdx == 0)						// If sdx is 0, use date/time value
	{
		svcSysDateTime(1, &curdt);
		if ((seedx = (long)(curdt.dt >> 32)) < 100)
			seedx += 123456;
		if ((seedy = (long)curdt.dt) < 100)
			seedy += 54321;
		seedc = 7654321;
	}
	else
	{
		seedx = sdx;
		seedy = sdy;
		seedz = sdz;
		seedc = sdc;
	}
}


//******************************************************
// Function: krandom - Return 32-bit pseudo-random value
// Returned: Value
//******************************************************

ulong krandom(void)
{
	ullong t;
	ullong a = 698769069;

	seedx = 69069 * seedx + 12345;
	seedy ^= (seedy << 13);
	seedy ^= (seedy >> 17);
	seedy ^= (seedy << 5);
	t = a * seedz + seedc;
	seedc = (long)(t >> 32);
	seedz = (long)t;
	return (seedx + seedy + seedz);
}
