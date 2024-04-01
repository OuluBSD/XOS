#include <stdio.h>


///#pragma data_seg ("data", "data");
///#pragma code_seg ("code");

int zzz = 0x333;

char *ppp;

const int aaa = 0x555;


int segtest(
	int xxx,
	int yyy)
{
	ppp = "This is a test";

	return (*ppp + aaa + zzz + xxx + yyy + 1);
}
