/*    
   Dsksort.c - this is the configuration for sorting directories directly
               on disk.

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

#include <string.h>

#include "fte.h"
#include "sortcfgf.h"
#include "sortfatf.h"

#define ENTRY 1
#define SLOT  2

struct Pipe 
{
    /* Input */
    int slotorentry; /* Indicates wether to return an entry or a slot
                        in an entry.                                  */
    unsigned entrynr;
    unsigned slot;
    
    /* Processing */
    unsigned ecounter;
    unsigned scounter;
    
    /* Output */
    int error;
    struct DirectoryEntry* entry;
};

struct DiskEntryGetterStruct
{
    RDWRHandle handle;
    CLUSTER cluster;
};

static int entrygetter (RDWRHandle handle,
         struct DirectoryPosition* pos,
         void** buffer);
                        
static int getentry(RDWRHandle handle,
                    CLUSTER cluster, 
                    int slotorentry,
                    unsigned entry,
                    unsigned slot, 
                    struct DirectoryEntry* result);   

static int GetEntryFromDisk(void* entries, unsigned entry,  
                     struct DirectoryEntry* result);                     
                   
static int GetSlotFromDisk(void* entries, int entry, int slot,  
                    struct DirectoryEntry* result);
                    
static int SwapEntriesOnDisk(void* entries, unsigned pos1, unsigned pos2);                        

static struct ResourceConfiguration DiskConfig =
{
   GetEntryFromDisk,
   SwapEntriesOnDisk,
   GetSlotFromDisk
};

BOOL DiskSortEntries(RDWRHandle handle, CLUSTER cluster)
{
   int  i=0;
   long realentries = 0, totalcount, lfncount, realcount;
   struct DiskEntryGetterStruct parameters;
   struct DirectoryPosition pos;
   struct DirectoryEntry entry;

   parameters.handle = handle;
   parameters.cluster = cluster;
   
   SetResourceConfiguration(&DiskConfig);  

   totalcount = low_dircount(handle, cluster, 0xffff);
   if (totalcount == -1)
      return FALSE;
      
   lfncount   = low_dircount(handle, cluster, LFN_ATTRIBUTES);
   if (lfncount == -1)
      return FALSE;
      
   realcount = totalcount - lfncount;
   
   for (i = 0; i < 2; i++)
   {
       if (!GetNthDirectoryPosition(handle, cluster, 0, &pos))
       {
          return FALSE;
       }
   
       if (!GetDirectory(handle, &pos, &entry))
       {
          return FALSE;
       }
   
       switch (i)
       {
          case 0:
               if (IsCurrentDir(entry))
               {
                  realcount--;
               }
               break;
          case 1:
               if (IsPreviousDir(entry))
               {
                  realcount--;
               }
       }
   } 
            
   if (realentries)
      SelectionSortEntries(&parameters, realcount);

   return TRUE;
}
              
static int getentry(RDWRHandle handle,
                    CLUSTER cluster, 
                    int slotorentry,
                    unsigned entry,
                    unsigned slot, 
                    struct DirectoryEntry* result)
{
   
    struct Pipe pipe, *ppipe = &pipe;
        
    pipe.slotorentry = slotorentry;
    pipe.entrynr = entry;
    pipe.slot    = slot;
    
    pipe.ecounter = 0;
    pipe.scounter = 0;
    
    pipe.error = FALSE;
    pipe.entry = result;
         
    if (TraverseSubdir(handle, cluster, entrygetter, (void**) &ppipe, TRUE))
    {
       if (pipe.error)
          return FALSE;
       else
          return TRUE;
    }
    return FALSE;
}

static int GetEntryFromDisk(void* entries, unsigned entry,  
                     struct DirectoryEntry* result)
{ 
    struct DiskEntryGetterStruct* parameters;

    parameters = (struct DiskEntryGetterStruct*) entries;
    
    return getentry(parameters->handle, parameters->cluster,
                    ENTRY, entry, 0, result);
}

static int GetSlotFromDisk(void* entries, int entry, int slot,  
                    struct DirectoryEntry* result)
{ 
    struct DiskEntryGetterStruct* parameters;

    parameters = (struct DiskEntryGetterStruct*) entries;
    
    return getentry(parameters->handle, parameters->cluster,
                    SLOT, entry, slot, result);
}

static int entrygetter (RDWRHandle handle,
         struct DirectoryPosition* pos,
         void** buffer)
{
     struct Pipe** pipe = (struct Pipe**) buffer;
     struct DirectoryEntry entry;
     
     if (!GetDirectory(handle, pos, &entry))
     {
        (*pipe)->error = TRUE;
        return FALSE;
     }
     
     if ((*pipe)->entrynr == (*pipe)->ecounter)
     {
        if ((*pipe)->slotorentry == ENTRY)
        {
           memcpy((*pipe)->entry, &entry, sizeof(struct DirectoryEntry));
           return FALSE;
        }
        else
        {
           if ((*pipe)->slot == (*pipe)->scounter)
           {
              memcpy((*pipe)->entry, &entry, sizeof(struct DirectoryEntry));
              return FALSE;
           }
           (*pipe)->scounter++;
           return TRUE;
        }  
     }
     
     if ((entry.attribute & FA_LABEL) == 0)
        (*pipe)->ecounter++;     
     
     return TRUE;   
}

static int SwapEntriesOnDisk(void* entries, unsigned pos1, unsigned pos2)
{
    struct DiskEntryGetterStruct* parameters;

    parameters = (struct DiskEntryGetterStruct*) entries;

    return SwapLFNEntries(parameters->handle,
                          parameters->cluster,
                          pos1, pos2);    
}


