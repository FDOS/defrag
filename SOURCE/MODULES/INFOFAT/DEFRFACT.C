/*    
   Defrfact.c - calculate defragmentation factor.

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
#include "bool.h"
#include "infofat.h"

static int FactorCalculator(RDWRHandle handle, CLUSTER cluster,
                            SECTOR sector, void** structure);

static int FileFactCalculator(RDWRHandle handle,
                              struct DirectoryPosition* position,
                              void** structure);

struct PipeStruct {
    CLUSTER total;
    CLUSTER consecutive;
    
    BOOL error;
};

struct ResultStruct {
       unsigned long count;
       unsigned long sum;
       
       BOOL error;
};

/************************************************************
***                        GetDefragFactor
*************************************************************
*** Calculates the defragmentation factor.
*************************************************************/ 

int GetDefragFactor(RDWRHandle handle)
{
    struct ResultStruct result = {0,0,FALSE}, *presult = &result;

    if (!WalkDirectoryTree(handle,
                           FileFactCalculator,
                           (void**) &presult))
       return 255;
       
    if (result.error)
       return 255;
  
    if (result.count != 0)
       return (int) (result.sum / result.count);
    else
       return 100; /* Empty disks are not fragmented. */
}

/************************************************************
***                        FileFactCalculator
*************************************************************
*** If this file is included in the count then it goes through
*** the given files and scans for the number of consecutive 
*** clusters.
*************************************************************/

static int FileFactCalculator(RDWRHandle handle,
                              struct DirectoryPosition* position,
                              void** structure)
{
    CLUSTER firstcluster;
    struct PipeStruct pipe = {0,0}, *ppipe = &pipe;
    struct ResultStruct* result = *((struct ResultStruct**) structure);
    
    struct DirectoryEntry entry;
    
    if (!GetDirectory(handle, position, &entry))
    {
       result->error = TRUE;
       return FALSE;
    }

    if (IsLFNEntry(&entry))         return TRUE;
    if (IsDeletedLabel(entry))      return TRUE;
    if (entry.attribute & FA_LABEL) return TRUE;
    if (IsCurrentDir(entry))        return TRUE;
    if (IsPreviousDir(entry))       return TRUE;
  
    firstcluster = GetFirstCluster(&entry);
    if (firstcluster == 0) return TRUE;
    
    if (!FileTraverseFat(handle, firstcluster, FactorCalculator,
                         (void**) &ppipe))
    {   
       result->error = TRUE;
       return FALSE;
    }
    if (pipe.error)
    {
       result->error = TRUE;
       return FALSE;
    }

    result->count++;

    if (pipe.total != 0)
       result->sum += (((unsigned long) pipe.consecutive * 100) / pipe.total);

    return TRUE;
}

/************************************************************
***                  FactorCalculator
*************************************************************
*** Goes through the given file and counts the number of 
*** consecutive clusters.
*************************************************************/

static int FactorCalculator(RDWRHandle handle, CLUSTER cluster,
                            SECTOR sector, void** structure)
{
    struct PipeStruct *pipe = *((struct PipeStruct**) structure);
    CLUSTER now = DataSectorToCluster(handle, sector);
    if (!now) 
    {
       pipe->error = TRUE;     
       return FALSE;
    }

    pipe->total++;

    if ((now == cluster - 1) || (FAT_LAST(cluster))) pipe->consecutive++;

    return TRUE;
}
