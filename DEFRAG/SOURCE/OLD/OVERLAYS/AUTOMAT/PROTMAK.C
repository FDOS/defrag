/*
   Protmak.C - protocol files creation utility.

   Copyright (C) 2000, Imre Leber.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have recieved a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   If you have any questions, comments, suggestions, or fixes please
   email me at:  imre.leber@worldonline.be

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION " á 0.1"

void ShowHelp(void);
void GenerateInclFile(FILE* ifptr, FILE* ofptr, 
		      char* precode, char* prefix, char* postcode);
void CreateFileName(char* src, char* dst, char* extension);

int main (int argc, char** argv)
{
    FILE  *ifptr, *ofptr;

    char dest[128];

    if (argc != 2)
    {
       ShowHelp();
       return 1;
    }

    if ((ifptr = fopen(argv[1], "rt")) == NULL)
    {
       puts("Cannot open source file.");
       return 1;
    }

    CreateFileName(argv[1], dest, ".h");

    if ((ofptr = fopen(dest, "wt")) == NULL)
    {
       puts("Cannot open destination file.");
       fclose(ifptr);
       return 1;
    }

    GenerateInclFile(ifptr, ofptr,
		     "#ifndef GENET_PROTMAK_H_\n"
		     "#define GENET_PROTMAK_H_\n\n",
		     "#define ",
		     "\n#endif");

    fclose(ifptr);
    fclose(ofptr);

    if ((ifptr = fopen(argv[1], "rt")) == NULL)
    {
       puts("Cannot open source file.");
       return 1;
    }

    CreateFileName(argv[1], dest, ".inc");

    if ((ofptr = fopen(dest, "wt")) == NULL)
    {
       puts("Cannot open destination file.");
       fclose(ifptr);
       return 1;
    }

    GenerateInclFile(ifptr, ofptr, "", "%assign ", "");

    fclose(ifptr);
    fclose(ofptr);

    return 0;
}

void CreateFileName(char* src, char* dst, char* extension)
{
     char *pos, *pos1;

     strcpy(dst, src);

     if ((pos = strrchr(dst, '.')) != NULL)
     {
	if (((pos1 = strrchr(dst, '\\')) == NULL) ||
	    (pos1 < pos))
	   *pos = '\0';
     }

     strcat(dst, extension);
}


void GenerateInclFile(FILE* ifptr, FILE* ofptr,
		      char* precode, char* prefix, char* postcode)
{
     int counter = 0, c, leave = 0;

     fprintf(ofptr, precode);

     while (!leave)
     {
	 switch (c = fgetc(ifptr))
	 {
	    case EOF:
		 leave = 1;
		 break;

	    case ' ':
	    case '\n':
	    case '\t':
		 break;

	    case '#':
		 while (((c = fgetc(ifptr)) != EOF) && (c != '\n'));
		 if (c == EOF) leave = 1;
		 break;

	    default:
		 fprintf(ofptr, prefix);
		 fputc(c, ofptr);
		 while (((c = fgetc(ifptr)) != EOF) && (c != '\n'))
		       fputc(c, ofptr);
		 fprintf(ofptr, " %d\n", counter++);
		 if (c == EOF) leave = 1;
		 break;
	 }
     }

     fprintf(ofptr, postcode);
}


void ShowHelp()
{
     printf("protmak" VERSION "\n"
	    "Make protocol include files.\n"
	    "\n"
	    "(C) 2000, by Imre Leber.\n"
	    "\n"
	    "Protmak <file>\n"
	    "\n"
	    "file: file to use as source for protocol include files.\n");
}
