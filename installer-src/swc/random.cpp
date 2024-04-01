#include <STDIO.H>
#include <STDDEF.H>
#include <STDLIB.H>
#include <CTYPE.H>
#include <STRING.H>
#include <TIME.H>
#include "XOSSTUFF.H"
///#include "SMS.H"
///#include "MSG.H"
#include "XOSQDES.H"
///#include "RANDOM.H"
///#include "MSG_TKT.H"
///#include "MSG_LOGIN.H"
///#include "INVITEM.H"
///#include "WINSTR.H"


void initrandom(void)

{

}


void getrandombytes(
	uchar *bufr,
	int cnt)

{
	memset(bufr, 0x37, cnt);
}
