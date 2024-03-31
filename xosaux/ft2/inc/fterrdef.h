/***************************************************************************/
/*                                                                         */
/*  fterrdef.h                                                             */
/*                                                                         */
/*    FreeType error codes (specification).                                */
/*                                                                         */
/*  Copyright 2002, 2004, 2006 by                                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                LIST OF ERROR CODES/MESSAGES             *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/


  /* You need to define both FT_ERRORDEF_ and FT_NOERRORDEF_ before */
  /* including this file.                                           */


  /* generic errors */

  FT_NOERRORDEF_( Ok,                                        0x00, \
                  "No error indicated" )

  FT_ERRORDEF_( Cannot_Open_Resource,                        0x01, \
                "Cannot open resource" )
  FT_ERRORDEF_( Unknown_File_Format,                         0x02, \
                "Unknown file format" )
  FT_ERRORDEF_( Invalid_File_Format,                         0x03, \
                "Broken file" )
  FT_ERRORDEF_( Invalid_Version,                             0x04, \
                "Invalid FreeType version" )
  FT_ERRORDEF_( Lower_Module_Version,                        0x05, \
                "Module version is too low" )
  FT_ERRORDEF_( Invalid_Argument,                            0x06, \
                "Invalid argument" )
  FT_ERRORDEF_( Unimplemented_Feature,                       0x07, \
                "Unimplemented feature" )
  FT_ERRORDEF_( Invalid_Table,                               0x08, \
                "Broken table" )
  FT_ERRORDEF_( Invalid_Offset,                              0x09, \
                "Broken offset within table" )
  FT_ERRORDEF_( Array_Too_Large,                             0x0A, \
                "Array allocation size too large" )

  /* glyph/character errors */

  FT_ERRORDEF_( Invalid_Glyph_Index,                         0x10, \
                "Invalid glyph index" )
  FT_ERRORDEF_( Invalid_Character_Code,                      0x11, \
                "Invalid character code" )
  FT_ERRORDEF_( Invalid_Glyph_Format,                        0x12, \
                "Unsupported glyph image format" )
  FT_ERRORDEF_( Cannot_Render_Glyph,                         0x13, \
                "Cannot render this glyph format" )
  FT_ERRORDEF_( Invalid_Outline,                             0x14, \
                "Invalid outline" )
  FT_ERRORDEF_( Invalid_Composite,                           0x15, \
                "Invalid composite glyph" )
  FT_ERRORDEF_( Too_Many_Hints,                              0x16, \
                "Too many hints" )
  FT_ERRORDEF_( Invalid_Pixel_Size,                          0x17, \
                "Invalid pixel size" )

  /* handle errors */

  FT_ERRORDEF_( Invalid_Handle,                              0x20, \
                "Invalid object handle" )
  FT_ERRORDEF_( Invalid_Library_Handle,                      0x21, \
                "Invalid library handle" )
  FT_ERRORDEF_( Invalid_Driver_Handle,                       0x22, \
                "Invalid module handle" )
  FT_ERRORDEF_( Invalid_Face_Handle,                         0x23, \
                "Invalid face handle" )
  FT_ERRORDEF_( Invalid_Size_Handle,                         0x24, \
                "Invalid size handle" )
  FT_ERRORDEF_( Invalid_Slot_Handle,                         0x25, \
                "Invalid glyph slot handle" )
  FT_ERRORDEF_( Invalid_CharMap_Handle,                      0x26, \
                "Invalid charmap handle" )
  FT_ERRORDEF_( Invalid_Cache_Handle,                        0x27, \
                "Invalid cache manager handle" )
  FT_ERRORDEF_( Invalid_Stream_Handle,                       0x28, \
                "Invalid stream handle" )

  /* driver errors */

  FT_ERRORDEF_( Too_Many_Drivers,                            0x30, \
                "Too many modules" )
  FT_ERRORDEF_( Too_Many_Extensions,                         0x31, \
                "Too many extensions" )

  /* memory errors */

  FT_ERRORDEF_( Out_Of_Memory,                               0x40, \
                "Out of memory" )
  FT_ERRORDEF_( Unlisted_Object,                             0x41, \
                "Unlisted object" )

  /* stream errors */

  FT_ERRORDEF_( Cannot_Open_Stream,                          0x51, \
                "Cannot open stream" )
  FT_ERRORDEF_( Invalid_Stream_Seek,                         0x52, \
                "Invalid stream seek" )
  FT_ERRORDEF_( Invalid_Stream_Skip,                         0x53, \
                "Invalid stream skip" )
  FT_ERRORDEF_( Invalid_Stream_Read,                         0x54, \
                "Invalid stream read" )
  FT_ERRORDEF_( Invalid_Stream_Operation,                    0x55, \
                "Invalid stream operation" )
  FT_ERRORDEF_( Invalid_Frame_Operation,                     0x56, \
                "Invalid frame operation" )
  FT_ERRORDEF_( Nested_Frame_Access,                         0x57, \
                "Nested frame access" )
  FT_ERRORDEF_( Invalid_Frame_Read,                          0x58, \
                "Invalid frame read" )

  /* raster errors */

  FT_ERRORDEF_( Raster_Uninitialized,                        0x60, \
                "Raster uninitialized" )
  FT_ERRORDEF_( Raster_Corrupted,                            0x61, \
                "Raster corrupted" )
  FT_ERRORDEF_( Raster_Overflow,                             0x62, \
                "Raster overflow" )
  FT_ERRORDEF_( Raster_Negative_Height,                      0x63, \
                "Negative height while rastering" )

  /* cache errors */

  FT_ERRORDEF_( Too_Many_Caches,                             0x70, \
                "Too many registered caches" )

  /* TrueType and SFNT errors */

  FT_ERRORDEF_( Invalid_Opcode,                              0x80, \
                "Invalid opcode" )
  FT_ERRORDEF_( Too_Few_Arguments,                           0x81, \
                "Too few arguments" )
  FT_ERRORDEF_( Stack_Overflow,                              0x82, \
                "Stack overflow" )
  FT_ERRORDEF_( Code_Overflow,                               0x83, \
                "Code overflow" )
  FT_ERRORDEF_( Bad_Argument,                                0x84, \
                "Bad argument" )
  FT_ERRORDEF_( Divide_By_Zero,                              0x85, \
                "Division by zero" )
  FT_ERRORDEF_( Invalid_Reference,                           0x86, \
                "Invalid reference" )
  FT_ERRORDEF_( Debug_OpCode,                                0x87, \
                "Found debug opcode" )
  FT_ERRORDEF_( ENDF_In_Exec_Stream,                         0x88, \
                "Found ENDF opcode in execution stream" )
  FT_ERRORDEF_( Nested_DEFS,                                 0x89, \
                "Nested DEFS" )
  FT_ERRORDEF_( Invalid_CodeRange,                           0x8A, \
                "Invalid code range" )
  FT_ERRORDEF_( Execution_Too_Long,                          0x8B, \
                "Execution context too long" )
  FT_ERRORDEF_( Too_Many_Function_Defs,                      0x8C, \
                "Too many function definitions" )
  FT_ERRORDEF_( Too_Many_Instruction_Defs,                   0x8D, \
                "Too many instruction definitions" )
  FT_ERRORDEF_( Table_Missing,                               0x8E, \
                "SFNT font table missing" )
  FT_ERRORDEF_( Horiz_Header_Missing,                        0x8F, \
                "Horizontal header (hhea) table missing" )
  FT_ERRORDEF_( Locations_Missing,                           0x90, \
                "Locations (loca) table missing" )
  FT_ERRORDEF_( Name_Table_Missing,                          0x91, \
                "Name table missing" )
  FT_ERRORDEF_( CMap_Table_Missing,                          0x92, \
                "Character map (cmap) table missing" )
  FT_ERRORDEF_( Hmtx_Table_Missing,                          0x93, \
                "Horizontal metrics (hmtx) table missing" )
  FT_ERRORDEF_( Post_Table_Missing,                          0x94, \
                "PostScript (post) table missing" )
  FT_ERRORDEF_( Invalid_Horiz_Metrics,                       0x95, \
                "Invalid horizontal metrics" )
  FT_ERRORDEF_( Invalid_CharMap_Format,                      0x96, \
                "Invalid character map (cmap) format" )
  FT_ERRORDEF_( Invalid_PPem,                                0x97, \
                "Invalid ppem value" )
  FT_ERRORDEF_( Invalid_Vert_Metrics,                        0x98, \
                "Invalid vertical metrics" )
  FT_ERRORDEF_( Could_Not_Find_Context,                      0x99, \
                "Could not find context" )
  FT_ERRORDEF_( Invalid_Post_Table_Format,                   0x9A, \
                "Invalid PostScript (post) table format" )
  FT_ERRORDEF_( Invalid_Post_Table,                          0x9B, \
                "Invalid PostScript (post) table" )

  /* CFF, CID, and Type 1 errors */

  FT_ERRORDEF_( Syntax_Error,                                0xA0, \
                "Opcode syntax error" )
  FT_ERRORDEF_( Stack_Underflow,                             0xA1, \
                "Argument stack underflow" )
  FT_ERRORDEF_( Ignore,                                      0xA2, \
                "ignore" )

  /* BDF errors */

  FT_ERRORDEF_( Missing_Startfont_Field,                     0xB0, \
                "`STARTFONT' field missing" )
  FT_ERRORDEF_( Missing_Font_Field,                          0xB1, \
                "`FONT' field missing" )
  FT_ERRORDEF_( Missing_Size_Field,                          0xB2, \
                "`SIZE' field missing" )
  FT_ERRORDEF_( Missing_Chars_Field,                         0xB3, \
                "`CHARS' field missing" )
  FT_ERRORDEF_( Missing_Startchar_Field,                     0xB4, \
                "`STARTCHAR' field missing" )
  FT_ERRORDEF_( Missing_Encoding_Field,                      0xB5, \
                "`ENCODING' field missing" )
  FT_ERRORDEF_( Missing_Bbx_Field,                           0xB6, \
                "`BBX' field missing" )
  FT_ERRORDEF_( Bbx_Too_Big,                                 0xB7, \
                "`BBX' too big" )


/* END */
