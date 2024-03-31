#include <stdio.h>
///#include <malloc.h>

extern void *mallocxxx(size_t *size);

extern void freexxx(void *pnt);


void *malloc(
	size_t *size)

{
	void *rtn;


	rtn = mallocxxx(size);

///	printf("+++ malloc: %08X\n", rtn);

	return (rtn);
}


void free(
	void *pnt)

{
///	printf("+++ free: %08X\n", pnt);

	freexxx(pnt);
}
