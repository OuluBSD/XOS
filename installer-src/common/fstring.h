class fstring
{public:
	void  init(char *bufr, int len);
	void  putsubstring(int horpos, char *str, int len = 0);
	void  putsubstring(int horpos, int width, char *str, int len = 0);
	void  setfont(int fontnum);
	void  settextcolor(long color);
	void  setlineheight(int height, int leading = 0);
	void  newline() {*pntr++ = '\n';};
	void  setbgcolor(int color);
	char *buffer() {return (bufr);};
	int   length() {return (len);};
 private:
	char *bufr;
	char *pntr;
	int   max;
	int   len;
};
