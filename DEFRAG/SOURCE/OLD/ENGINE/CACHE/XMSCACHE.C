/*    
   XMScache.c - XMS implementation of sector cache routines.

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

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "..\header\fat.h"
#include "..\header\fatconst.h"
#include "..\header\sctcache.h"
#include "xms.h"
#include "xmscache.h"

#include "..\..\misc\bool.h"

static unsigned int  XMSHandle;  
static unsigned long XMSsize;
static unsigned long TotalSectorCount;
static unsigned long CachedSectorCount;

struct SectorTag
{
     SECTOR sectornumber;
     unsigned long sectoroffset;
     unsigned age;
     unsigned DeviceID;
};

static int IsFull(void);
static int Cache(unsigned devid, SECTOR sector, char* buffer);   
static int Retreive(unsigned devid, SECTOR sector, char* buffer);
static int Uncache(unsigned devid, SECTOR sector);   
static int Invalidate(void);

static struct LowCacheFunctions XMSCacheTable =
{
   IsFull,
   Cache,  
   Retreive,
   Uncache,
   Invalidate,          
   TRUE     
};

void InitXMSCache(struct LowCacheFunctions* table)
{
     if (XMSinit())
     {
        /* Try allocating all available XMS memory. */     
        XMSsize = XMScoreleft();
        if (XMSsize)
        {
           XMSHandle = XMSalloc(XMSsize);
           if (XMSHandle)
           {
              memcpy(table, &XMSCacheTable, sizeof(struct LowCacheFunctions));
              Invalidate();
           }
           else
              table->supported = FALSE;
        }
        else
           table->supported = FALSE;     
     }
     else
        table->supported = FALSE;
}

void CloseXMSCache(void)
{
     XMSfree(XMSHandle);
}

static int Invalidate(void)
{
   /* Calculate the number of sectors we can fit */
   TotalSectorCount = XMSsize / (BYTESPERSECTOR + sizeof(struct SectorTag));
   
   /* No sectors filled. */
   CachedSectorCount = 0;
   
   /* Always succeeds */
   return TRUE;
}

static int GetTag(unsigned long index, struct SectorTag* buffer)
{
   index *= sizeof(struct SectorTag);

   return XMStoDOSmove((char*)buffer, XMSHandle, index,
                       sizeof(struct SectorTag)) ==
                                         sizeof(struct SectorTag);
}

static int WriteTag(unsigned long index, struct SectorTag* tag)
{
   index *= sizeof(struct SectorTag);
   
   return DOStoXMSmove(XMSHandle, index, (char*)tag, sizeof(struct SectorTag))
          == sizeof(struct SectorTag);
}

static unsigned long GetAddress(unsigned devid, SECTOR sector)
{
   long i;
   static struct SectorTag tag;
   
   for (i = 0; i < CachedSectorCount; i++)
   {
       if (!GetTag(i, &tag)) return 0;
       
       if ((tag.DeviceID == devid) &&
           (memcmp(&tag.sectornumber, &sector, sizeof(sector)) == 0))
          return tag.sectoroffset;
   }
    
   return 0;
}

static int GetOldestTag(struct SectorTag* tag, unsigned long* index)
{
   static unsigned long i;
   unsigned long original;     
   struct SectorTag tag1;
                           
   original = (i+1) % CachedSectorCount; /* Optimize cache usage */
   i = (i+2) % CachedSectorCount;
   
   if (CachedSectorCount == 0)
      return FALSE;
      
   if (!GetTag(0, tag))
      return FALSE;
      
   if (CachedSectorCount == 1)
   {
      *index = 0;
      return TRUE;
   }
   
   for (; i != original; i=(i+1)%CachedSectorCount)
   {
       if (!GetTag(i, &tag1)) return FALSE;
       
       if (tag->age < tag1.age)
       {
          memcpy(tag, &tag1, sizeof(struct SectorTag));
          *index = i;
       }
   }
   return TRUE;
}

static int RedistributeAges(void)
{
   unsigned long i;
   struct SectorTag tag;
     
   for (i = 0; i < CachedSectorCount; i++)
   {
        if (!GetTag(i, &tag)) return FALSE;
        tag.age /= 2;
        if (!WriteTag(i, &tag)) return FALSE;
   }
   return TRUE;
}

static int IncrementAge(unsigned long offset) /* offset is the offset in XMS */
{
   unsigned long i;
   struct SectorTag tag;
   
   for (i = 0; i < CachedSectorCount; i++)
   {
       if (!GetTag(i, &tag)) return FALSE;
       
       if (tag.sectoroffset == offset)
       {
          if (tag.age == UINT_MAX)
             if (!RedistributeAges())
                return FALSE;

          tag.age++;
          
          return WriteTag(i, &tag);
       }
   }

   return TRUE;
}

static int IsFull(void)
{
  return CachedSectorCount == TotalSectorCount;
}

static int Cache(unsigned devid, SECTOR sector, char* buffer)
{
  unsigned long offset, index;
  struct SectorTag tag;
  
  /* See if it is not already in the cache */
  if ((offset = GetAddress(devid, sector)) != 0)
  {
     /* If it is bump up the age and overwrite it. */
     if (!IncrementAge(offset)) return FALSE;
          
     if (DOStoXMSmove(XMSHandle, offset, buffer, BYTESPERSECTOR)
                      != BYTESPERSECTOR)
     {
        /* PANIC - just close down the system and return */
        CloseXMSCache(); /* This almost never happens (XMS = RAM) */
        return -2;         
     }
     return TRUE;
  }
  
  /* Look wether the cache is not already full. */
  if (!IsFull())
  {
     /* Calculate where to put the sector */
     offset = TotalSectorCount*sizeof(struct SectorTag) + 
              CachedSectorCount*BYTESPERSECTOR;
     
     /* Fill in a new tag and write it. */
     memcpy(&tag.sectornumber, &sector, sizeof(SECTOR));
     tag.sectoroffset = offset;
     tag.age          = 0; 
     tag.DeviceID     = devid;       
          
     if (WriteTag(CachedSectorCount, &tag))
     {
         if (DOStoXMSmove(XMSHandle, offset, buffer, BYTESPERSECTOR)
                          == BYTESPERSECTOR)
         {
            CachedSectorCount++;
            return TRUE;
         }
     }
     return FALSE;
  }
  
  /* If the cache is full, search the oldest tag and replace it */
  if (!GetOldestTag(&tag, &index)) return FALSE;
  
  memcpy(&tag.sectornumber, &sector, sizeof(SECTOR));
  tag.age = 0;
  tag.DeviceID = devid;
  
  if ((DOStoXMSmove(XMSHandle, tag.sectoroffset, buffer, BYTESPERSECTOR)
                    != BYTESPERSECTOR) ||
      (!WriteTag(index, &tag)))  
  {
     /* PANIC - just close down the system and return */
     CloseXMSCache(); /* This almost never happens (XMS = RAM) */
     return -2;         
  }
  
  return TRUE;
}

static int Retreive(unsigned devid, SECTOR sector, char* buffer)
{
    unsigned long offset;
    
    /* See if it is in the cache. */    
    if ((offset = GetAddress(devid, sector)) != 0)
    {
       if (XMStoDOSmove(buffer, XMSHandle, offset, BYTESPERSECTOR) ==
                       BYTESPERSECTOR)
       {
          /* Bump up the age */  
          if (!IncrementAge(offset)) return FALSE;
          return TRUE;
       }
       return FALSE;
    }
    else
       return FALSE;
}

/* This function should be called whenever the integrity of a sector contents
   could be jeperdised by a cause external to the cache */
static int Uncache(unsigned devid, SECTOR sector)
{
   unsigned long i,j;
   struct SectorTag tag;
   
   for (i = 0; i < CachedSectorCount; i++)
   {
       if (!GetTag(i, &tag))
       {        
          /* PANIC - just close down the system and return */
          CloseXMSCache(); /* This almost never happens (XMS = RAM) */
          return FALSE;
       }
       
       if ((tag.DeviceID == devid) &&
           (memcmp(&tag.sectornumber, &sector, sizeof(sector)) == 0))
       {
          /* Found, move all following sector tags to the front and 
             decrement filled tag count */            
          for (j = i+1; j < CachedSectorCount; j++)
          {
             if (!GetTag(j, &tag))
             {        
                /* PANIC - just close down the system and return */
                CloseXMSCache(); /* This almost never happens (XMS = RAM) */
                return FALSE;
             } 
             if (!WriteTag(j-1, &tag))
             {        
                /* PANIC - just close down the system and return */
                CloseXMSCache(); /* This almost never happens (XMS = RAM) */
                return FALSE;
             } 
          }
          
          CachedSectorCount--;
       }   
   }
    
   return TRUE;
}   

