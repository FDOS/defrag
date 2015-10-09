/*    
   CheckFAT.c - check disk integrity module.

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

#include "fte.h"
#include "checkfat.h"
#include "expected.h"
#include "callback.h"

int CheckFAT(char* drive)
{
   RDWRHandle handle;
   int        result = 0;

   /* Send a large message. */
   LargeMessage("Checking file system integrity ...");

   /* Notice that we assume right input from the interface.*/
   if (!InitReadSectors(drive, &handle)) return 1;

   if (CheckVolume(handle)) 
      result = 0; 
   else 
      result = 1;

   CloseReadWriteSectors(&handle);

   return result;
}

#ifdef DEBUG

static struct CallBackStruct CallBacks;

int main(int argc, char** argv)
{
    int i;

    if (argc != 2)
       printf("CheckFAT <image file>\n");
    else
    {
       CMDEFINT_GetCallbacks(&CallBacks);
       SetCallBacks(&CallBacks);

       InitSectorCache();
//       StartSectorCache();

       if (AllocateFTEMemory(0, 0, 16384) != 3)
       {
	  return 1;
       }

       printf("%d\n", CheckFAT(argv[1]));

       DeallocateFTEMemory();
       CloseSectorCache();
    }
}

int GiveFullOutput(){return 1;}

#endif
