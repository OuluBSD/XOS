#include <stdio.h>
#include <search.h>


int cmp(void *a, void *b);


int main(void)

{
	long test[16] = {25, 57, 48, 37, 12, 92, 86, 33};

	qsort(test, 8, 4, cmp);
	return (0);
}


int cmp(
	void *a,
	void *b)

{
	if (*(long *)a == *(long *)b)
		return (0);
	return ((*(long *)a > *(long *)b) ? 1 : -1);
}
