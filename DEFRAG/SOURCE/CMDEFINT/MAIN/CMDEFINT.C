/*    
   Cmdefint.c - main module for command line interface.
   Copyright (C) 2000, 2002 Imre Leber

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

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <string.h>

#include "..\..\modlgate\modlgate.h"
#include "..\..\modlgate\argvars.h"
#include "..\..\modlgate\defrpars.h"
#include "..\..\engine\header\FTE.h"
#include "..\..\modlgate\callback.h"
#include "..\..\misc\version.h"
#include "..\..\misc\misc.h"
#include "..\..\misc\reboot.h"

#include "..\ovlhost\ovlimpl.h"

#include "..\..\misc\drvtypes.h"

#include "chkargs.h"
#include "..\..\environ\checkos.h"

static void OnExit(void);
static int  OnCBreak(void);
static int  CheckOS(void);

#define METHODBORDER 80

static struct CallBackStruct CallBacks;

int CMDefint(int argc, char *argv[])
{
    char switchchar = SwitchChar();
    int  factor;

char answer[5]; 

    /* Check parameters. */
    ParseCmdLineArguments(argc, argv, switchchar);

printf("This is an alpha version of defrag!!!\n");
printf("Are you sure you want to continue (YES/No)?");
scanf("%5s", answer);

if (stricmp(answer, "YES") != 0) return 1;

    /* Check file system integrity. */
    if (!CheckOS()) return 1;

    /* Show copyright on the screen. */
    printf("This program is free software. It comes with ABSOLUTELY NO WARANTIES.\n"
           "You are welcome to redistribute it under the terms of the\n" 
           "GNU General Public License, see http://www.GNU.org for details.\n\n");

    atexit(OnExit);
    ctrlbrk(OnCBreak);

    CMDEFINT_GetCallbacks(&CallBacks);
    SetCallBacks(&CallBacks);

    if (GetDriveType(GetOptimizationDrive() - 'A' + 1) != dtRemovable)
    {
       printf("Invalid or unsupported drive type!");
       return 1;
    }

    if (!CheckDiskIntegrity())
    {
       printf("Disk corrupted, cannot defragment!");
       return 1;
    }
    else
    {
       if ((factor = ScanDrive()) == 255)
       {
          printf("Disk corrupted, cannot defragment!");
          return 1;
       }
       else
       {
          printf("%d%% of drive %c: is not fragmented.\n",
                 factor,
                 GetOptimizationDrive());

          if (!IsMethodEntered())
          {
             if (factor == 100)
             {
                printf("No optimization necessary.\n");
                return 0;
             }
             else if (factor > METHODBORDER)
                SetOptimizationMethod(UNFRAGMENT_FILES);
             else /* if (factor < METHODBORDER) */
                SetOptimizationMethod(FULL_OPTIMIZATION);
          }
          
          if (GetOptimizationMethod() == UNFRAGMENT_FILES)
             printf("Optimization method is unfragment files only.\n");   
          else
             printf("Optimization method is full optimization.\n");
          
          printf("The actual defragmentation process is not implemented yet!\n");

          if (IsRebootRequested())
          {
             printf("Rebooting the computer...");
             ColdReboot();
          }
      }
    }

    return 0;
}

static void OnExit(void)
{
}

static int OnCBreak(void)
{
    return 1;
}

static int CheckOS(void)
{
      char* msg;

      msg = CheckDefragEnvironment();
      if (msg)
      {
         printf("%s\n");
         return FALSE;
      }
      
      return TRUE;
}
