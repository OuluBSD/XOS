/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


/*      The first parm to __math1err and __math2err is an unsigned int
        that indicates the math function, the type of error, and the
        expected result value.
*/
#ifndef _COMDEF_H_INCLUDED
 #include <_comdef.h>
#endif

_WMRTLINK extern double __math1err( unsigned int, double *arg1 );
_WMRTLINK extern double __math2err( unsigned int, double *arg1, double *arg2 );
_WMRTLINK extern double __log87_err( double, unsigned int );

#ifndef __math_func__
 #define __math_func__(code,name)  code
 enum func_codes {
#else
 #undef __math_func__
 #define __math_func__(code,name)  name
 static char *MathFuncNames[] = {
#endif

        __math_func__( FUNC_ACOS = 1,"acos" ),
        __math_func__( FUNC_ASIN,    "asin" ),
        __math_func__( FUNC_SQRT,    "sqrt" ),
        __math_func__( FUNC_EXP,     "exp"  ),
        __math_func__( FUNC_COSH,    "cosh" ),
        __math_func__( FUNC_SINH,    "sinh" ),
        __math_func__( FUNC_POW,     "pow"  ),
        __math_func__( FUNC_ACOSH,   "acosh"),
        __math_func__( FUNC_LOG2,    "log2" ),
        __math_func__( FUNC_LOG,     "log"  ),
        __math_func__( FUNC_LOG10,   "log10"),
        __math_func__( FUNC_ATANH,   "atanh"),
        __math_func__( FUNC_ATAN2,   "atan2"),
        __math_func__( FUNC_IPOW,    "ipow" ),
        __math_func__( FUNC_DPOWI,   "dpowi"),
        __math_func__( FUNC_COS,     "cos"  ),
        __math_func__( FUNC_SIN,     "sin"  ),
        __math_func__( FUNC_TAN,     "tan"  ),
        __math_func__( FUNC_Y0,      "y0"   ),
        __math_func__( FUNC_Y1,      "y1"   ),
        __math_func__( FUNC_YN,      "yn"   ),
        __math_func__( FUNC_MOD,     "mod"  ),
        __math_func__( FUNC_COTAN,   "cotan")
};
#define FUNC_MASK       0x001F

/*       bits for the type of math errors */

#define M_DOMAIN        0x0040
#define M_SING          0x0080
#define M_OVERFLOW      0x0100
#define M_PLOSS         0x0200
#define M_TLOSS         0x0400
#define M_UNDERFLOW     0x0800

/*       bits for the return value expected */

#define V_NEG_HUGEVAL   0x1000
#define V_ZERO          0x2000
#define V_ONE           0x4000
#define V_HUGEVAL       0x8000
