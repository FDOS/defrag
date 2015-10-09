/*
   sorttree.c - takes care that every directory on the volume is sorted.

   Copyright (C) 2002 Imre Leber

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
#include "sortfatf.h"

#ifdef DEBUG

int printer(RDWRHandle handle,
		struct DirectoryPosition* pos,
		void** buffer)
{
    int i;
    struct DirectoryEntry entry;

    static int counter = 0;

    counter++;
    if (counter == 100) exit(1);

    printf("Position = (%lu, %d)\r\n", pos->sector, pos->offset);

    if (GetDirectory(handle, pos, &entry))
    {
       for (i = 0; i < 11; i++)
	   printf("%c", entry.filename[i]);
       printf("\r\n");
    }
    else
       printf("Cannot get directory\n");

    return TRUE;
}

#endif

int SortDirectoryTree(RDWRHandle handle)
{
   BOOL error=FALSE, *pError = &error;

   if (!SortEntries(handle, 0))
   {
      return FALSE;
   }

   if (!WalkDirectoryTree(handle, SortSubdir, (void**) &pError))
   {
      return FALSE;
   }
   else
   {
      return !error;
   }
}
