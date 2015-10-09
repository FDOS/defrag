/*    
   actions.c - routines that call the modules through the module gate.

   Copyright (C) 2000 Imre Leber

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   If you have any questions, comments, suggestions, or fixes please
   email me at:  imre.leber@worldonline.be
*/

#include <stdio.h>

#include "fte.h"

#include "actions.h"
#include "chkargs.h"

#include "..\screen\screen.h"
#include "..\dialog\sortbox.h"
#include "..\dialog\methods.h"
#include "..\dialog\msgbxs.h"
#include "..\dialog\seldrvbx.h"
#include "..\dialog\recoment.h"

#include "..\..\modlgate\defrpars.h"
#include "..\..\modlgate\modlgate.h"
#include "..\..\misc\bool.h"

#include "..\logman\logman.h"

#include "..\screen\scrmask.h"

#include "actaspct.h"

#define METHODBORDER 80

char* OptimizationMethods[] = {"Full Optimization",
                               "Unfragment Files Only"};

void BeginOptimization(void)
{
     char* buttons[] = {"Ok"};

     CROSSCUT_BEGIN_OPTIMIZATION    /* Poor man's aspect orientation? */

     StartCounting();

     if (!SortDirectories())
     {
	ErrorBox("Disk corrupted, cannot sort!", 1, buttons);
	return;
     }
}

void SelectSortOptions (void)
{
     struct SortDialogStruct options;

     if (GetSortOptions(&options))
        SetSortOptions(options.SortCriterium, options.SortOrder);

     CROSSCUT_SET_OPTIONS           /* Poor man's aspect orientation? */
}                                   

void SelectOptimizationMethod(void)
{
     int method = 0;

     if (AskOptimizationMethod(&method))
     {
        DrawMethod (OptimizationMethods[method]);
        SetOptimizationMethod(method);
     }

     CROSSCUT_SET_METHOD            /* Poor man's aspect orientation? */
}

int SelectDrive(void)
{
     char drive;

     if ((drive = ShowDriveSelectionBox()) != 0)
     {
        CROSSCUT_SET_DRIVE             /* Poor man's aspect orientation? */

        SetOptimizationDrive(drive);
        DrawCurrentDrive(drive);
        return QueryDisk();
     }

     return FALSE;
}

int QueryDisk(void) 
{
   int   factor, goon = FALSE;
   char* buttons[] = {"Ok"};

   CROSSCUT_QUERY_BEFORE

   if (CheckDiskIntegrity())
   {
      if ((factor = ScanDrive()) == 255)
      {
         CROSSCUT_QUERY_ERROR
         ErrorBox("Disk corrupted, cannot defragment!", 1, buttons);
      }
      else
      {
         CROSSCUT_QUERY_OK
         
         if (!IsMethodEntered())
         {
            if (factor < METHODBORDER)
            {
               goon = RecommendMethod(factor, 
                                      GetOptimizationDrive(), 
                                      OptimizationMethods[FULL_OPTIMIZATION]);
               DrawMethod (OptimizationMethods[FULL_OPTIMIZATION]);
               SetOptimizationMethod(FULL_OPTIMIZATION);
            }
            else if (factor < 100)
            {
               goon = RecommendMethod(factor, 
                                      GetOptimizationDrive(), 
                                      OptimizationMethods[UNFRAGMENT_FILES]);
               SetOptimizationMethod(UNFRAGMENT_FILES);
               DrawMethod (OptimizationMethods[UNFRAGMENT_FILES]);
            }
            else
               InformUser("Disk not fragmented.");
         }
         else
         {
            goon = TRUE;
            DrawMethod (OptimizationMethods[GetOptimizationMethod()]);
         }
      }
   }
   else
   {
      CROSSCUT_QUERY_ERROR

      ErrorBox("Disk corrupted, cannot defragment!", 1, buttons);
      SetOptimizationDrive('0');
      DrawCurrentDrive('?');
      DrawBlockSize(0);
   }

   return goon;
}
