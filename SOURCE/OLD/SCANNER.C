/*
   Scanner.c - lexical analyser.

   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, Imre Leber.

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

   module:

*/

#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>
#include <string.h>
#include <dir.h>
#include <stdio.h>

#include "parser.h"
#include "scanner.h"
#include "..\misc\bool.h"                       /* FALSE and TRUE */
#include "exepath.h"

static TOKEN ScanHeader(void);
static int ScanKeyWord(char* keyword);
static int Identify(char* karakters);
static void UpperCaseScannerData(void);

static char* ScannerData = NULL, *ScannerPosition, *ScannerEnd;
static char* ScannerPrevious;

static int   ScannerLine = 1;

#define AMOFKEYWORDS 23

static char* KeyWords[] = {"REBOOT",          
			   "AUDIBLE",         
			   "AUTOEXIT",         
			   "OUTPUT",            
			   "METHOD",             
			   "CRITERIUM",           
			   "ORDER",                
			   "YES",                   
			   "NORMAL",                 
			   "FULL",        
			   "NO",           
			   "UNFRAGMENT",    
			   "SELECTIVE",      
			   "ASCENDING",       
			   "DESENDING",        
			   "NAME",              
			   "EXTENSION",          
			   "SIZE",                
			   "DATEANDTIME",          
			   "DATE&TIME",    
			   "DATE",         
			   "TIME",           
			   "MOMENT"};         

static int KeyTokens[] = {TknREBOOT,
			  TknAUDIBLE,
			  TknAUTOEXIT,
			  TknOUTPUT,
			  TknMETHOD,
			  TknCRITERIUM,
			  TknSORTORDER,
			  TknYES,
			  TknNORMAL,
			  TknFULL,
			  TknNO,
			  TknUNFRAGMENT,
			  TknSELECTIVE,
			  TknASCENDING,
			  TknDESCENDING,
			  TknNAME,
			  TknEXTENSION,
			  TknSIZE,
			  TknDATEANDTIME,
			  TknDATEANDTIME,
			  TknDATEANDTIME,
			  TknDATEANDTIME,
			  TknDATEANDTIME};

#define AMOFHEADERS 5

static char* HeaderWords[] = {"OPTIONS",
			      "OPTION",
			      "DEFRAGMENTATION",
			      "DEFRAG",
			      "SORT"};


static int HeaderTokens[] = {TknOPTIONHEADER,
			     TknOPTIONHEADER,
			     TknDEFRAGHEADER,
			     TknDEFRAGHEADER,
			     TknSORTHEADER};

static char INIPathName[128];
static int  Initialized;

/*
** Get the directory where to search for the ini file.
**
** Input: filename: file name to append to the directory.
**
** Output: buffer : full path of .ini file.
*/
char* GetIniDir(char* filename, char* buffer)
{
   char* pathname;

   if (!Initialized)
   {
      Initialized = TRUE;

      /* a) is there an environment variable? */
      pathname = getenv(INIVARIABLEHI);
      if (!pathname) pathname = getenv(INIVARIABLELOW);

      if (pathname)
      {
	 strcpy(INIPathName, pathname);

	 if (INIPathName[strlen(INIPathName)] != '\\')
	 {
	    INIPathName[strlen(INIPathName)+1] = '\0';
	    INIPathName[strlen(INIPathName)] = '\\';
	 }

	 strcpy(buffer, INIPathName);
	 strcat(buffer, INIFILENAME);
	 if (access(buffer, 0) != 0) pathname = NULL;
      }

      /* b) is there an .ini file in the executable's directory? */
      if (!pathname)
      {
	 GetExePath(buffer, 128);
	 strcpy(INIPathName, buffer);
	 if (buffer[0] == 0) return SCANFILENOTFOUND; /* buffer to small!? */
	 strncat(buffer, INIFILENAME, 128);
	 if (access(buffer, 0) != 0)
	 {
	    /* c) is there an .ini file in the current directory?     */
	    INIPathName[0] = 0;
	 }
      }
   }

   strcpy(buffer, INIPathName);
   strcat(buffer, filename);

   return buffer;
}

/*
** Reads the raw file from the file into the buffer.
**
** Returns:  0  if file not found.
**          -1  if error when reading file.
**           1  if success.
*/

int ReadScannerFile(char* filename)
{
   FILE* fptr;
   char  buffer[128];
   long  filesize;
   int   c;

   /* First look where we can find the ini file. */
   if (!filename) filename = GetIniDir(INIFILENAME, buffer);
   if (access(filename, 0) != 0) return SCANFILENOTFOUND;

   /* Now that we have found the file open it. */
   if ((fptr = fopen(filename, "rt")) != NULL)
   {
      /* Now we have opened the file, allocate memory for it's contents. */
      fseek(fptr, 0, SEEK_END);
      filesize = ftell(fptr);
      fseek(fptr, 0, SEEK_SET);

      if (filesize > 32767)
      {
	 fclose(fptr);
	 return SCANFILEERROR;
      }

      if ((ScannerData = malloc((size_t) filesize)) == NULL)
      {
	 fclose(fptr);
	 return SCANFILEERROR;
      }
      ScannerPosition = ScannerEnd = ScannerData;

      /* Read the file. */
      while ((c = fgetc(fptr)) != EOF) *ScannerEnd++ = (char) c;
      
      /* Close the file. */
      fclose(fptr);
   }
   else
      return SCANFILEERROR;

   /* Set everything in the buffer to upper case. */
   UpperCaseScannerData();

   return SCANSUCCESS;
}

/*
** Closes the scanner and releases all allocated memory.
*/
void CloseScanner()
{
     if (ScannerData) free(ScannerData);
}

/*
** Restarts the scanning process.
*/
void RestartScanning()
{
     ScannerPosition = ScannerData;
}

/*
** Returns next token in the buffer, or TknDONE if end of buffer reached.
**
** Note: scanning is done in place, so normal string routines cannot be used.
*/
TOKEN GetNextToken()
{
     int i;
     ScannerPrevious = ScannerPosition;

     if (ScannerPosition == ScannerEnd) return TknDONE;

     /* See if we have a comment. */
     if (Identify("#"))
     {
	while (!Identify("\n\r")) ScannerPosition++;
     }

     /* See if we have a simple white space. */
     if (Identify(" \t\x1A"))
     {
	ScannerPosition++;
	return TknSPACE;
     }

     /* See if we have a line feed. */
     if (Identify("\n"))
     {
	ScannerPosition++;
	ScannerLine++;
	return TknRETURN;
     }

     /* See if we have cariage return. */
     if (Identify("\r"))
     {
	ScannerPosition++;
	if (Identify("\n")) ScannerPosition++;     /* account for CR/LF. */
	ScannerLine++;
	return TknRETURN;
     }

     /* See if we have a = token. */
     if (Identify("="))
     {
	ScannerPosition++;
	return TknASSIGN;
     }

     /* See if we have a header token. */
     if (*ScannerPosition == '[') return ScanHeader();

     /* Scan normal keywords. */
     for (i = 0; i < AMOFKEYWORDS; i++)
	 if (ScanKeyWord(KeyWords[i])) return KeyTokens[i];

     return TknNONE;
}

/*
** Returns the last scannend unit.
**
** Note: it is the users responsibility to free the returned pointer.
*/
char* ScannerString()
{
     char* result, *iterator1, *iterator2;
     int len = ScannerPosition - ScannerPrevious + 1;


     if ((result = malloc(len)) != NULL)
     {
	iterator2 = result;
	for (iterator1 = ScannerPrevious; iterator1 != ScannerPosition;
							     iterator1++)
	    *iterator2++ = *iterator1;
	*iterator2 = '\0';
     }

     return result;
}

/*
** Returns the next token, without advancing the input.
*/
TOKEN PeekToken()
{
     char* savedpos  = ScannerPosition;
     char* savedprev = ScannerPrevious;
     int   savedline = ScannerLine;

     TOKEN result = GetNextToken();

     ScannerPosition = savedpos;
     ScannerPrevious = savedprev;
     ScannerLine     = savedline;

     return result;
}

int GetScannerLine()
{
    return ScannerLine;
}

static TOKEN ScanHeader()
{
     int seen, i;
     ScannerPosition++; /* [ already seen. */

     if (Identify("-")) ScannerPosition++;

     while (Identify(" \t")) ScannerPosition++;

     seen = TknNONE;
     for (i = 0; i < AMOFHEADERS; i++)
	 if (ScanKeyWord(HeaderWords[i]))
	 {
	    seen = HeaderTokens[i];
	 }
     
     if (seen == TknNONE) return TknNONE;

     while (Identify(" \t")) ScannerPosition++;

     if (Identify("-")) ScannerPosition++;
     if (Identify("]")) {ScannerPosition++; return seen;}

     return TknNONE;
}

static int ScanKeyWord(char* keyword)
{
     char* original = ScannerPosition;
     char  buf[2]; buf[1] = '\0';

     while (*keyword)
     {
	  *buf = *keyword;
	  if (!Identify(buf))
	  {
	     ScannerPosition = original;
	     return FALSE;
	  }

	  ScannerPosition++;
	  keyword++;
     }
     return TRUE;
}

static int Identify(char* characters)
{
     if (ScannerPosition >= ScannerEnd) return FALSE;

     while (*characters)
	   if (*ScannerPosition == *characters)
	      return TRUE;
	   else
	      characters++;

     return FALSE;
}

static void UpperCaseScannerData()
{
   char* pointer = ScannerData;

   while (pointer != ScannerEnd)
   {
	 *pointer = toupper(*pointer);
	 pointer++;
   }
}
