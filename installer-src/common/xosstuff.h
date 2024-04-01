///#ifdef __cplusplus
///	extern "C"{
///#endif  

#if !defined XOSSTUFF_H
#define XOSSTUFF_H

typedef unsigned char    uchar;
typedef unsigned short   ushort;
typedef unsigned int     uint;
typedef unsigned long    ulong;
typedef unsigned __int64 ullong;
typedef          __int64 llong;

#define TRUE  1
#define FALSE 0

///typedef struct
///{	llong dt;
///} time_s;

///typedef struct
///{	llong dt;
///	short tzone;
///	short dlst;
///} time_sz;

#define XT_MILLISEC 50
#define XT_SECOND   49710
#define XT_MINUTE   2982616
#define XT_HOUR     178956971

#define SW(val) (((val & 0xFF00) >> 8) | ((val & 0xFF) << 8))
#define SL(val) (SW(val >> 16) | (SW(val) << 16))

#ifndef _MSC_VER

extern void int3();
#pragma aux int3 = \
"	int 3";

extern ushort sw(ulong value);
#pragma aux sw = \
"	xchg al, ah" \
	parm [EAX] value [AX];

extern ulong sl(ulong value);
#pragma aux sl = \
"	xchg al,ah" \
"	ror  eax,16" \
"	xchg al, ah" \
	parm [EAX] value [EAX];

#else
extern "C"{
extern ushort sw(ulong value);
extern ulong sl(ulong value);
extern ullong sll(ullong value);
}
#endif

///#pragma pack(1)

///#define offsetof(stype, field)
///    (size_t)( (char *) &(((stype *)0)->field) - (char *)0 )

void   longdiv(long *result, long num1, long num2, long dem);
void   xoserrmsg(long code, char *text);

///#ifdef __cplusplus
///	}
///#endif


#endif
