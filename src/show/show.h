extern long infocnt;
extern int  validcount;
extern char prgname[];

int   crshinfo(void);
int   dcsinfo(void);
int   devinfo(void);
int   diskinfo(void);
void *getinfo(int func);
int   intinfo(void);
int   ioinfo(void);
int   lasinfo(void);
int   lkeinfo(void);
int   pciinfo(void);
int   terminfo(void);
