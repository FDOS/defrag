/*    
   Ovlimpl.c - host implementation for microsoft look alike interface.
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

#include "..\..\engine\header\fte.h"
#include "..\..\modlgate\callback.h"
#include "..\..\modlgate\expected.h"

#include "..\screen\drvmap.h"
#include "..\screen\scrmask.h"
#include "..\screen\screen.h"
#include "..\dialog\dialog.h"
#include "..\dialog\msgbxs.h"
#include "..\logman\logman.h"

#include "lowtime.h"

static long SavedTime;

static int IsCounting = FALSE;

void StartCounting()
{
    int SavedHours, SavedMinutes, SavedSeconds;
    
    GetTime(&SavedHours, &SavedMinutes, &SavedSeconds);

    SavedTime = SavedHours * 3600 + SavedMinutes * 60 + SavedSeconds;

    IsCounting = TRUE;
}

void StopCounting()
{
    IsCounting = FALSE;
}

static void _UpdateInterfaceState(void)
{
    long now, diff;
    int  hour, minute, second;

    if (IsCounting)
    {
       GetTime(&hour, &minute, &second);

       now = hour * 3600 + minute * 60 + second;
       if (now > SavedTime)
	  diff   = now - SavedTime;
       else
	  diff = 86400l - SavedTime + now;

       hour   = (int) diff / 3600; diff %= 3600;
       minute = (int) diff / 60;   diff %= 60;
       second = (int) diff;

       DrawTime(hour, minute, second);
    }
}

static void _SmallMessage(char* buffer)
{
     _UpdateInterfaceState();

     SetStatusBar(RED, WHITE, "                                            ");
     SetStatusBar(RED, WHITE, buffer);
}

static void _LargeMessage(char* buffer)
{
     int hour, minute, second, prevsecond, counter = 0;

     _UpdateInterfaceState();
     
     /* Put the message on the screen. */
     ShowModalMessage(buffer);

     /* Wait about 2 seconds. 
        Actually waits between 1 and 2 seconds because it counts the  
        number of times the second changes value.                     */
     GetTime (&hour, &minute, &prevsecond); 
     while (counter < 2)
     {
           GetTime(&hour, &minute, &second);
           if (prevsecond != second) 
           {
              counter++;
              prevsecond = second;
           }
     }

     /* Take the message of the screen. */
     HideDialog;
}

static void _DrawDriveMap(CLUSTER maxcluster)
{
     _UpdateInterfaceState();

     DrawDrvMap(maxcluster);   
}

static void _DrawOnDriveMap(CLUSTER cluster, int symbol)
{
     switch (symbol)
     {
       case WRITESYMBOL:
            DrawWriteBlock(cluster);
            break;

       case READSYMBOL:
            DrawReadBlock(cluster);
            break;
       
       case USEDSYMBOL:
            DrawUsedBlock(cluster);
            break;   
            
       case UNUSEDSYMBOL:
            DrawUnusedBlock(cluster);
            break;

       case BADSYMBOL:
            DrawBadBlock(cluster);
            break;
       
       case UNMOVABLESYMBOL:
            DrawUnmovableBlock(cluster);
            break;

       case OPTIMIZEDSYMBOL:
            DrawOptimizedBlock(cluster);
            break;
     }

     _UpdateInterfaceState();
}

static void _LogMessage(char* message)
{
     _UpdateInterfaceState();

     LogPrint(message);
}

void MSDEFINT_GetCallbacks(struct CallBackStruct* result)
{
   result->UpdateInterface = _UpdateInterfaceState;
   result->SmallMessage    = _SmallMessage;
   result->LargeMessage    = _LargeMessage;
   result->DrawDriveMap    = _DrawDriveMap;
   result->DrawOnDriveMap  = _DrawOnDriveMap;
   result->LogMessage      = _LogMessage;
}
