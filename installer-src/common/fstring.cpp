#include <stdio.h>
#include <stddef.h>
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <time.h>
#include "xcstring.h"
#include "xosstuff.h"
#include "swcwindows.h"
#include "fstring.h"

void fstring::init(
    char *bufrarg,
    int   lenarg)

{
    pntr = bufr = bufrarg;
    max = lenarg;
    len = 0;
}


int aaaaa = 0;

void fstring::putsubstring(
    int   horpos,
    char *str,
    int   strln)

{
    *pntr++ = 0x11;
    if (horpos <= 255)
    {
        *pntr++ = 0x04;
        *pntr++ = (uchar)horpos;
    }
    else
    {
        *pntr++ = 0x05;
        *pntr++ = (uchar)horpos;
        *pntr++ = (uchar)(horpos >> 8);
    }
    if (strln == 0)
        strln = strlen(str);
    len = pntr - bufr;
    if (strln > (max - len - 1))
        strln = max - len - 1;
    if (strln > 0)
    {
        memcpy(pntr, str, strln);
        pntr += strln;
        len += strln;
    }
    *pntr = 0;
}


void fstring::putsubstring(
    int   horpos,
    int   width,
    char *str,
    int   strln)

{
    *pntr++ = 0x11;
/// horpos *= sfd;
    if (horpos <= 255)
    {
        *pntr++ = 0x04;
        *pntr++ = (uchar)horpos;
    }
    else
    {
        *pntr++ = 0x05;
        *pntr++ = (uchar)horpos;
        *pntr++ = (uchar)(horpos >> 8);
    }
    *pntr++ = 0x11;
/// width *= sfd;
    if (width >= 0 && width <= 255)
    {
        *pntr++ = 0x06;
        *pntr++ = (uchar)width;
    }
    else
    {
        *pntr++ = 0x07;
        *pntr++ = (uchar)width;
        *pntr++ = (uchar)(width >> 8);
    }
    if (strln == 0)
        strln = strlen(str);
    len = pntr - bufr;
    if (strln > (max - len - 1))
        strln = max - len - 1;
    if (strln > 0)
    {
        memcpy(pntr, str, strln);
        pntr += strln;
        len += strln;
    }
    *pntr = 0;
}


void fstring::setlineheight(
    int height,
    int leading)

{
    *pntr++ = 0x11;
    *pntr++ = 0x0B;
    *pntr++ = (uchar)leading;
    *pntr++ = (uchar)height;
}


void fstring::setfont(
    int fontnum)

{
    *pntr++ = 0x11;
    *pntr++ = 0x08;
    *pntr++ = (uchar)fontnum;
}


void fstring::settextcolor(
    long color)

{
    *pntr++ = 0x11;
    if (color == 0x80000000)
        *pntr++ = 0x0C;
    else
    {
        *pntr++ = 0x0A;
        *pntr++ = (uchar)(color >> 16);
        *pntr++ = (uchar)(color >> 8);
        *pntr++ = (uchar)color;
    }
}


void fstring::setbgcolor(
    int color)

{
    *pntr++ = 0x11;
    *pntr++ = 0x22;
    *pntr++ = (uchar)(color >> 16);
    *pntr++ = (uchar)(color >> 8);
    *pntr++ = (uchar)color;
}
