/*    
   Mrkunmve.c - mark unmovable clusters.

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

#include "fte.h"
#include "expected.h"
#include "infofat.h"

static int UnmovableMarker(RDWRHandle handle,
                           struct DirectoryPosition* position,
                           void** structure);

static int RealMarker(RDWRHandle handle,
                      CLUSTER label,
                      SECTOR  sector,
                      void**  structure);

/************************************************************
***                        MarkUnmovables
*************************************************************
*** Goes through all the entries on the volume and marks
*** all the clusters that can not be moved as unmovable.
*************************************************************/                      
                      
int MarkUnmovables(RDWRHandle handle)
{
   BOOL error, *perror = &error; 

   if (!WalkDirectoryTree(handle, UnmovableMarker, (void**) &perror))
      return FALSE;
      
   return error;
}

/************************************************************
***                        MarkUnmovables
*************************************************************
*** If this file cannot be moved then it goes through the given 
*** file and marks all the clusters that can not be moved as
*** unmovable.
*************************************************************/ 

static int UnmovableMarker(RDWRHandle handle,
                           struct DirectoryPosition* position,
                           void** structure)
{
    struct DirectoryEntry entry;
    BOOL error, *perror = &error;
    
    if (!GetDirectory(handle, position, &entry))
    {
       **(BOOL**) structure = TRUE; /* Set error value */
       return FALSE;
    }
    
    if (((entry.attribute & FA_HIDDEN) || (entry.attribute & FA_SYSTEM)) &&
	(!IsLFNEntry(&entry)))
    {
       CLUSTER firstcluster = GetFirstCluster(&entry);
       
       if (!FileTraverseFat(handle, firstcluster, RealMarker, 
                            (void**)&perror))
       {
          **(BOOL**) structure = TRUE;
          return FALSE;      
       }
       if (error)
       {
          **(BOOL**) structure = TRUE;
          return FALSE;        
       }
    }

    return TRUE;
}

/************************************************************
***                        RealMarker
*************************************************************
*** If this file cannot be moved then it goes through the given 
*** file and marks all the clusters that can not be moved as
*** unmovable.
*************************************************************/ 

static int RealMarker(RDWRHandle handle,
                      CLUSTER label,
                      SECTOR  sector,
                      void**  structure)
{
    CLUSTER cluster;
    
    label = label;
    
    cluster = DataSectorToCluster(handle, sector);
    if (!cluster)
    {
       **(BOOL**) structure = TRUE;
       return FALSE;
    }

    DrawOnDriveMap(cluster, UNMOVABLESYMBOL);

    return TRUE;
}
