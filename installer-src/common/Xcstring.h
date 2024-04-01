///char     *strnchr(char *str, char chr, int len);
char     *strnmov(char *dest, const char *src, int maxlen);
char     *strmov(char *dest, const char *src);
int       strnlen(const char *s1, int len);
char     *strncpyn(char *dst, const char *src, int maxlen);
///char     *strupper(char *s);
///char     *strlower(char *s);
///int       stricmp(const char *s1, const char *s2);
///int       strnicmp(const char *s1, const char *s2, int maxlen);
void	  movedata(unsigned srcseg, unsigned srcoff, unsigned dstseg,
		  unsigned dstoff, unsigned n);
///char     *strlwr(char *s);
///char     *strupr(char *s);
///char     *strrev(char *s);
///char     *stristr(char *s1, char *s2);
