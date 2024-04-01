///#include <stdio.h>
///#include <stdlib.h>
///#include <ctype.h>

///#include "dw.h"

typedef enum {
    DW_MODEL_NONE    = 0,
    DW_MODEL_FLAT    = 1,
    DW_MODEL_SMALL   = 2,
    DW_MODEL_MEDIUM  = 3,
    DW_MODEL_COMPACT = 4,
    DW_MODEL_LARGE   = 5,
    DW_MODEL_HUGE    = 6,
}xx_model;

struct qqqq
{	xx_model xxx;
} qqqq;

#define strize(x) strize2(x)
#define strize2(x) x


#if defined(__XOS__)
  #pragma message ("__XOS__ is defined");
#endif
#if defined(__XOS_386__)
  #pragma message ("__XOS_386__ is defined");
#endif
#if defined(__FLAT__)
  #pragma message ("__FLAT__ is defined");
#endif

#pragma message (strize(__DATE__))

long version = __WATCOMC__;


xx_model zzzz[] = {DW_MODEL_NONE, DW_MODEL_FLAT, DW_MODEL_SMALL,
		DW_MODEL_MEDIUM};

int main(
	int aaa,
	int bbb)
{
	return (aaa + bbb);
}
