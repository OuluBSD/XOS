/* PROGHELP.H - Compatibility header for program argument help functions */

#ifndef _PROGHELP_H_
#define _PROGHELP_H_ 1

/* For compatibility with programs that expect PROGHELP.H */
/* This header provides type mappings that are expected by some source files */

/* 
 * Type mappings for compatibility
 * These types are defined elsewhere in XOS headers but used with different names in source
 */

/* 
 * These type aliases are expected by DOSCOM.C and other programs
 * but may not be properly exposed due to header organization issues
 */

/* Character structure aliases */
typedef struct { unsigned char desp; unsigned char size; char name[8]; long value; } byte4_char;
typedef struct { unsigned char desp; unsigned char size; char name[8]; char value[4]; } text4_char;

/* Parameter structure aliases */
typedef struct { unsigned char desp; unsigned char size; unsigned short index; long value; } byte4_parm;
typedef struct { unsigned char desp; unsigned char size; unsigned short index; char value[4]; } text4_parm;

/* QAB structure alias (forward declaration is insufficient, so we need to define it properly) */
/* We'll use a more generic approach since the structure is defined in xos.h */

/* Since there are header conflicts, we'll provide these as type aliases to the original types */
/* The actual definitions exist in XOS.H, but we need to make sure they are accessible here */

/* Include the base XOS header since the structures are defined there */
/* But we need to be careful about multiple inclusions */
/* This is a compatibility header only, so we should avoid redefining existing types */

/* For the QAB structure that's causing the error, we'll just ensure XOS.H is properly referenced */
/* The QAB type should be available through XOS.H which DOSCOM.C already includes */

/* Provide the needed function declaration */
/* opthelp is defined in PROGARG.H which is already included by DOSCOM.C */

/* The issue might just be that we need to include PROGARG.H but not duplicate types */
/* Let's do a minimal include that just provides the needed function */

#include <PROGARG.H>

#endif /* _PROGHELP_H_ */