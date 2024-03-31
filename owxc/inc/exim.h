
typedef struct exim__ EXIM;
struct exim__
{	EXIM *next;
	char  ctype;
	char  cclass;
	char  stype;
	char  flag;
	char  nlen;
	char  name[];
};

extern EXIM * eximhead;
extern EXIM **eximprev;


