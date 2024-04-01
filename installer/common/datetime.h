#define STDDATE_FULLYEAR 0x04
#define STDDATE_SUPPRESS 0x02

int   displaydate(char *str, swcDateTime *date, int format, int options);
int   getdate(struct tm *dt, char *str, int yropt, int offset);
long  getdatetime(struct tm *dt, char *str, int offset = 49);
long  parsedate(char *str, int yropt = false, int offset = 49);
long  parsedatetime(swcDateTime *dt, char *str, int offset = 49);
int   standarddate(char *stddate, swcDateTime *date, int fullyear);
