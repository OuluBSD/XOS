/* PROGHELP.H - Compatibility header for XOS program help functions */

#ifndef _PROGHELP_H_
#define _PROGHELP_H_ 1

/* Include the base headers that contain all required types */
#include <XOS.H>
#include <XOSSVC.H>
#include <PROGARG.H>
#include <PROGASST.H>

/* 
 * Ensure all type definitions that source files expect are available
 * These are the expected type aliases in XOS
 */
#ifndef _TYPE_QAB_DEFINED
#define _TYPE_QAB_DEFINED
typedef QAB type_qab;
#endif

#ifndef _BYTE4_CHAR_DEFINED
#define _BYTE4_CHAR_DEFINED
typedef BYTE4CHAR byte4_char;
#endif

#ifndef _TEXT4_CHAR_DEFINED
#define _TEXT4_CHAR_DEFINED
typedef TEXT4CHAR text4_char;
#endif

/* Define any other required types */
#ifndef _BYTE4_PARM_DEFINED
#define _BYTE4_PARM_DEFINED
typedef BYTE4PARM byte4_parm;
#endif

#ifndef _TEXT4_PARM_DEFINED
#define _TEXT4_PARM_DEFINED
typedef TEXT4PARM text4_parm;
#endif

#endif /* _PROGHELP_H_ */