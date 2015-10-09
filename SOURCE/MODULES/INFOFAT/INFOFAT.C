/*    
   Infofat.c - get information.

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

#include "fte.h"
#include "expected.h"
#include "callback.h"
#include "infofat.h"

int InfoFAT(char* drive)
{
    RDWRHandle handle;
    int result=0;

    /* Mention what comes next. */
    LargeMessage("Scanning file system...");

LogMessage("Scanning file system...");

    /* Notice that we assume right input from the interface.*/
    if (!InitReadSectors(drive, &handle)) return 255;

    SmallMessage("Filling drive map.");
    if (!FillDriveMap(handle)) return 255;

    SmallMessage("Marking unmovable clusters.");
    if (!MarkUnmovables(handle)) return 255;

    SmallMessage("Calculating defragmentation factor.");
    result = GetDefragFactor(handle);
    
    SmallMessage("");
    CloseReadWriteSectors(&handle);

    return result;
}

#ifdef DEBUG

static struct CallBackStruct CallBacks;

int main(int argc, char** argv)
{
    if (argc != 2)
       printf("InfoFAT <image file>\n");
    else
    {
       CMDEFINT_GetCallbacks(&CallBacks);
       SetCallBacks(&CallBacks);

       printf("%d\n", InfoFAT(argv[1]));
    }
}

int GiveFullOutput () {return 1;}
#endif