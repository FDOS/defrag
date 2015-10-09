/*    
   SwapClst.c - Functions to swap clusters in a volume.

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

#include <stdlib.h>
#include <alloc.h>
#include <string.h>

#include "fte.h"

BOOL SwapClusters(RDWRHandle handle, CLUSTER clust1, CLUSTER clust2)
{
     BOOL found, IsInFAT=FALSE, error;
     CLUSTER freeclust, clustervalue1, fatpos, clustervalue2;
     char sectbuf[BYTESPERSECTOR], far* fpSectBuf = (char far*) sectbuf;
     unsigned char sectorspercluster, i;
     SECTOR startsector;
     struct DirectoryPosition dirpos;
     struct DirectoryEntry entry;
     unsigned long neededmem;
     void far* MemCluster;
       
     if (!ReadFatLabel(handle, clust1, &clustervalue1))
           return FALSE;
    
     if (!ReadFatLabel(handle, clust2, &clustervalue2))
           return FALSE;
           
     if (FAT_FREE(clustervalue1) && (FAT_FREE(clustervalue2)))
     {
        return TRUE;
     }
     if (FAT_FREE(clustervalue1))
     {
        if (!RelocateCluster(handle, clust2, clust1))
           return FALSE;     
     }
     if (FAT_FREE(clustervalue2))
     {
        if (!RelocateCluster(handle, clust1, clust2))
           return FALSE;     
     }
             
     sectorspercluster = GetSectorsPerCluster(handle);
     if (!sectorspercluster) return FALSE;
   
     neededmem = sectorspercluster * BYTESPERSECTOR;   
     MemCluster = farmalloc(neededmem);
     if (MemCluster)
     {     
        /* See where the cluster is refered */
        if (!FindClusterInFAT(handle, clust1, &fatpos))
           return FALSE;
      
        if (!fatpos)
        {
           if (!FindClusterInDirectories(handle, clust1, &dirpos, &found))
              return FALSE;
           if (!found)
              return FALSE;                /* Non valid cluster! */
        }
        else
        {
           IsInFAT = TRUE;
        }
        
        /* Read cluster in memory */
        sectorspercluster = GetSectorsPerCluster(handle);
        if (!sectorspercluster) return FALSE;
        startsector = clust1 * sectorspercluster;
        
        for (i = 0; i < sectorspercluster; i++)
        {
            if (!ReadDataSectors(handle, 1, startsector+i, sectbuf))
               return FALSE;
               
            movedata(FP_SEG(fpSectBuf), FP_OFF(fpSectBuf),
                     FP_SEG(MemCluster), FP_OFF(MemCluster)+i*BYTESPERSECTOR,
                     BYTESPERSECTOR);
        }
             
        if (!RelocateCluster(handle, clust2, clust1))
           return FALSE; 
                        
        startsector = clust2 * BYTESPERSECTOR;                
        for (i = 0; i < sectorspercluster; i++)
        {
            movedata(FP_SEG(MemCluster), FP_OFF(MemCluster),
                     FP_SEG(fpSectBuf), FP_OFF(fpSectBuf)+i*BYTESPERSECTOR,
                     BYTESPERSECTOR);
            if (!WriteDataSectors(handle, 1, startsector+i, sectbuf))
               error = TRUE; /* Salvage what can be salvaged */
        }
        
        /* Write the entry in the FAT */
        if (!WriteFatLabel(handle, clust2, clustervalue1))
           error = TRUE;
        
        /* Adjust the pointer to the relocated cluster */
        if (IsInFAT)
        {
           if (!WriteFatLabel(handle, fatpos, clust2))
           error = TRUE;
        }
        else
        {
           if (!GetDirectory(handle, &dirpos, &entry))
              error = TRUE;
          
           SetFirstCluster(clust2, &entry);   
           if (!WriteDirectory(handle, &dirpos, &entry))
              error = TRUE;
        }
        if (error) return FALSE;    
     }
     else
     {
        if (!FindLastFreeCluster(handle, &freeclust))
           return FALSE;
       
        if (!freeclust)
        {
           SetFTEerror(FTE_MEM_INSUFFICIENT);     
           return FALSE;
        }
        if (!RelocateCluster(handle, clust1, freeclust))
           return FALSE;
          
        if (!RelocateCluster(handle, clust2, clust1))
           return FALSE;
        
        if (!RelocateCluster(handle, freeclust, clust1))
           return FALSE;
     }
     
     return TRUE;
}
