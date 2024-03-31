#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H


FT_Library library;
FT_Face face;

int error;


void main ()

{
	error = FT_Init_FreeType(&library);
	if (error)
	{
		printf("? FT_InitFreeType failed, %s\n", FT_ErrMsg(error));
		exit (1);
	}
	error = FT_New_Face(library, "sans.ttf", 0, &face);
	{
		printf("? FT_New_Face failed, %s\n", FT_ErrMsg(error));
		exit (1);
	}




	printf("DONE\n");
	exit (0);
}
