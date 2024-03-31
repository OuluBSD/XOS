#define VECT_MESSAGE 50
#define VECT_SECOND  51

#define ulong  unsigned long
#define ushort unsigned short
#define uchar  unsigned char

struct cfgchar
{   LNGSTRCHAR sysname;
    BYTE4CHAR  memtotal;
    char       end;
};

struct statechar
{	BYTE4CHAR sysstate;
    BYTE4CHAR initial;
    char      end;
};

typedef struct sesnreq_ SESNREQ;
struct sesnreq_
{	SESNREQ *next;
	char     grp[18];
	char     usr[18];
	char     trm[18];
	char     cmd[1];
};

extern SESNREQ  *sesnhead;
extern SESNREQ **sesnppnt;

extern uchar startdone;

struct intdata
{   long  svdEDI;
    long  svdESI;
    long  svdEBP;
    long  svdESP;
    long  svdEBX;
    long  svdEDX;
    long  svdECX;
    long  svdEAX;
    long  intGS;
    long  intFS;
    long  intES;
    long  intDS;
    long  intEIP;
    long  intCS;
    long  intEFR;
    short intcnt;
    short intnum;
    long  pid;
    long  term;
    long  pEIP;
    long  pCS;
    long  pEFR;
    long  pos;
    long  pseg;
    long  data;
};

struct vectdata
{   short size;
    char  level;
    char  vector;
};

typedef struct
{   char  mask;
    char  match;
    long  rate;
    short delay;
} RATEDATA;

extern int      mincnt;
extern ulong    initpid;
extern long     conhndl;
extern long     logdev;
extern long     logdate;
extern uchar    logcommit;
extern time_sz  bgndt;
extern long     rtnval;
extern char     dttmbfr[];
extern char     envlist[];
extern char     welcome[];
extern char     childtext[];
extern RATEDATA ratedata1[];
extern int      numrates1;
extern RATEDATA ratedata2[];
extern int      numrates2;
extern char     prgname[];
extern char     imsg0[];
extern char     sysname[];
extern char     startupdone;
extern char     ipmerrmsg[];
extern struct   cfgchar   cfgchar;
extern struct   statechar statechar;
extern QAB      cfgqab;
extern char     pwbufr[];
extern char     prgmbufr[];

extern char     msgbfr[2000];
extern char     srcbfr[64];

void   childgone(struct intdata);
long   makesession(char *trmname, char *cmdline, char *group, char *user);
void   message(void);
void   onceasecond(void);
uchar *passwd_garble(unsigned char *);
void   putinlog(ulong pid, char *label, char *text);
void   response(char *header, char *msg);
void   rolllog(long date, char *text);
void   sessionreq(uchar *msg, int len);
void   symbiontreq(uchar *msg, int len);
void   syslogdata(void);
void   terminaldata(uchar *msg, int len);
long   timechg(time_s *, time_s *);
