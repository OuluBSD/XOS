typedef struct
{	uchar boot;
	uchar bgnhead;
	uchar bgnsect;
	uchar bgncyln;
	uchar type;
	uchar endhead;
	uchar endsect;
	uchar endcyln;
	ulong begin;
	ulong length;
} PTENT;

typedef struct
{	uchar  boot;
	uchar  type;
	uchar  bgnhead;
	uchar  bgnsect;
	int    bgncyln;
	uchar  numbr;
	uchar  xxx;
	uchar  endhead;
	uchar  endsect;
	int    endcyln;
	ulong  begin;
	ulong  length;
} PDATA;

typedef struct
{
	char   btcd[440]
	long   diskid;
	char   fill[2];
	PDATA  part[4];
	ushort label;
} MBR;

extern uchar diskinitlbamode;

int  diskinitclearmbr(void);
int  diskinitclose(void);
void diskiniterror(long  code, char *msg);
int  diskinitfinish(void);
int  diskinitinitmbr(void);
int  diskinitmakepart(int type, int active, long begin, long length,
		long heads, long sects);
int  diskinitopen(char *diskname);
int  diskinitreadmbr(MBR *mbr);
