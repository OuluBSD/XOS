/* PROGHELP.H - Compatibility header for program argument help functions */

#ifndef _PROGHELP_H_
#define _PROGHELP_H_ 1

/* Include the actual headers that contain these functions and types */
#include <PROGARG.H>
#include <XOS.H>

/* 
 * Type mappings for compatibility
 * XOS defines these types with uppercase names in xos.h
 * but source code often uses lowercase names with underscores
 */ 
typedef BYTE4CHAR  byte4_char;
typedef TEXT4CHAR  text4_char;
typedef BYTE4PARM  byte4_parm;
typedef TEXT4PARM  text4_parm;

/* 
 * Additional mappings that might be needed
 */
typedef BYTE2CHAR  byte2_char;
typedef TEXT2CHAR  text2_char;
typedef BYTE8CHAR  byte8_char;
typedef TEXT8CHAR  text8_char;
typedef LNGSTRCHAR lngstr_char;

typedef BYTE2PARM  byte2_parm;
typedef TEXT2PARM  text2_parm;
typedef BYTE8PARM  byte8_parm;
typedef TEXT8PARM  text8_parm;
typedef BYTE16PARM byte16_parm;
typedef LNGSTRPARM lngstr_parm;

#endif /* _PROGHELP_H_ */