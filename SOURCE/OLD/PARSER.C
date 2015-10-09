/*
   Parser.c - syntactic analyser.

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

*/

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "scanner.h"
#include "..\misc\bool.h"

#define INITERROR -1

static void SkipWhiteSpace(int losely);
static void ParseINIFILE(void);
static void ShowParserError(void);
static void ParseBlock(struct LValueArray* lvalues, int amount);

static void ActOnREBOOT(TOKEN rvalue);
static void ActOnAUDIBLE(TOKEN rvalue);
static void ActOnAUTOEXIT(TOKEN rvalue);
static void ActOnOUTPUT(TOKEN rvalue);
static void ActOnMETHOD(TOKEN rvalue);
static void ActOnCRITERIUM(TOKEN rvalue);
static void ActOnORDER(TOKEN rvalue);

struct LValueArray OptionLvals[] = {{TknREBOOT,   ActOnREBOOT},
				    {TknAUDIBLE,  ActOnAUDIBLE},
				    {TknAUTOEXIT, ActOnAUTOEXIT},
				    {TknOUTPUT,   ActOnOUTPUT}};

struct LValueArray DefragLvals[] = {{TknMETHOD, ActOnMETHOD}};

struct LValueArray SortLVals[] = {{TknCRITERIUM, ActOnCRITERIUM},
				  {TknSORTORDER, ActOnORDER}};

#define AMOFHEADERS 3
struct HeaderArray Headers[] = {{4, TknOPTIONHEADER, OptionLvals},
				{1, TknDEFRAGHEADER, DefragLvals},
				{2, TknSORTHEADER,   SortLVals}};

/*
** Public interface to the parser.
*/

int ParseIniFile(char* filename)
{
    int result;

    if ((result = ReadScannerFile(filename)) <= 0) return result;

    ParseINIFILE();

    /* Release scanner memory. */
    CloseScanner();
    return PARSERSUCCESS;
}

/* 
** Skip white space. 
*/

static void SkipWhiteSpace(int losely)
{
       int token;

       for (;;)
       {
	   token = PeekToken();
	   if ((token == TknSPACE) || (losely && (token == TknRETURN)))
	      GetNextToken();
	   else
	      break;
       }

}

/*
** Parse an .ini file.
*/

static void ParseINIFILE()
{
       int token, i, found;

       for (;;)
       {
	   SkipWhiteSpace(TRUE);

	   token = GetNextToken();
	   SkipWhiteSpace(FALSE);

	   if (token == TknDONE) break;
	   if (GetNextToken() != TknRETURN) ShowParserError();

	   found = FALSE;
	   for (i = 0; i < AMOFHEADERS; i++)
	       if (token == Headers[i].header)
	       {
		  found = TRUE;
		  ParseBlock(Headers[i].lvalues, Headers[i].amount);
	       }

	   if (!found) ShowParserError();
       }
}

/*
** Parse an individual block.
*/

static void ParseBlock(struct LValueArray* lvalues, int amount)
{
       int token, i, found;

       for (;;)
       {
	   SkipWhiteSpace(TRUE);
	   token = PeekToken();

	   found = FALSE;
	   for (i = 0; i < amount; i++)
	   {
	       if (lvalues[i].token == token)
	       {
		  found = TRUE;
		  break;
	       }
	   }
	   if (!found) return;

	   GetNextToken();
	   SkipWhiteSpace(FALSE);
	   
	   token = GetNextToken();
	   if (token == TknNONE) ShowParserError();
	   if ((token == TknRETURN) || (token == TknDONE)) continue;

	   if (token == TknASSIGN)
	   {
	      SkipWhiteSpace(FALSE);
	      token = PeekToken();

	      if ((token != TknRETURN) && (token != TknDONE))
		 lvalues[i].func(GetNextToken());
	   }
	   else
	      lvalues[i].func(token);

	   SkipWhiteSpace(FALSE);
	   if (((token = GetNextToken()) != TknRETURN) && (token != TknDONE))
	      ShowParserError();
       }
}

/*
** Show a syntactic error message.
*/
static void ShowParserError()
{
     printf("Syntax error on line %d in configuration file.\n", GetScannerLine());
     
     CloseScanner();
     exit(INITERROR);
}

/*
** Show a semantic error message.
*/
static void ShowParserErrorMsg(char* msg)
{
     printf("Semantic error in configuration file (%d): %s!\n", 
	    GetScannerLine(), msg);  

     CloseScanner();
     exit(INITERROR);
}

/*
** Act on REBOOT = YES|NO
*/
static void ActOnREBOOT(TOKEN rvalue)
{
   if ((rvalue != TknYES) && (rvalue != TknNO))
      ShowParserErrorMsg("Please enter YES or NO.");

}

/*
** Act on AUDIBLE = YES|NO
*/

static void ActOnAUDIBLE(TOKEN rvalue)
{
   if ((rvalue != TknYES) && (rvalue != TknNO))
      ShowParserErrorMsg("Please enter YES or NO.");
   
}

/*
** Act on AUTOEXIT = YES|NO
*/

static void ActOnAUTOEXIT(TOKEN rvalue)
{
   if ((rvalue != TknYES) && (rvalue != TknNO))
      ShowParserErrorMsg("Please enter YES or NO.");
   
}

/*
** Act on OUTPUT = FULL|NORMAL
*/

static void ActOnOUTPUT(TOKEN rvalue)
{
   if ((rvalue != TknFULL) && (rvalue != TknNORMAL))
      ShowParserErrorMsg("Please enter FULL or NORMAL.");

}

/*
** Act on METHOD = FULL|UNFRAGMENT|SELECTIVE
*/

static void ActOnMETHOD(TOKEN rvalue)
{
    switch (rvalue)
    {
      case TknFULL:
      case TknUNFRAGMENT:
      case TknSELECTIVE:
	   break;
      default:
	   ShowParserErrorMsg("Please enter FULL, UNFRAGMENT or SELECTIVE."); 
    }
}

/*
** Act on CRITERIUM = NAME|EXTENSION|SIZE|DATEANDTIME..
*/

static void ActOnCRITERIUM(TOKEN rvalue)
{
    switch (rvalue)
    {
      case TknNAME:
      case TknEXTENSION:
      case TknSIZE:
      case TknDATEANDTIME:
	   break;
      default:
	   ShowParserErrorMsg("Please enter NAME, EXTENSION, SIZE or DATEANDTIME."); 
    }
}

/*
** Act on ORDER = ASCENDING|DESCENDING
*/

static void ActOnORDER(TOKEN rvalue)
{
    switch (rvalue)
    {
      case TknASCENDING:
      case TknDESCENDING:
	   break;
      default:
	   ShowParserErrorMsg("Please enter ASCENDING or DESCENDING."); 
    }
}

