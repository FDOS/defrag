/*    
   EMScache.c - EMS implementation of sector cache routines.

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
#include <dos.h>

#include "..\header\fat.h"
#include "..\header\fatconst.h"
#include "..\header\sctcache.h"
#include "ems.h"
#include "emscache.h"

#include "..\..\misc\bool.h"

#define READ  0
#define WRITE 1

static unsigned int  EMSBase;  
static int           EMSHandle;
static unsigned long EMSsize;
static int           CurrentEMSPage=-1; /* Physische pagina mapped to
                                          bank 0.                      */
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
static int MapEMSPage(int logicalpage);

static struct LowCacheFunctions EMSCacheTable =
{
   IsFull,
   Cache,  
   Retreive,
   Uncache,
   Invalidate,          
   TRUE     
};

void InitEMSCache(struct LowCacheFunctions* table)
{ 
     EMSBase = EMSbaseaddress();

     if (EMSBase && (EMSversion() != -1) && (EMSstatus() == 0))
     {
	/* Try allocating all available EMS memory. */
        EMSsize = (unsigned long)EMSpages();
        if (EMSsize)
        {
           EMSHandle = EMSalloc((int)EMSsize);
           if (EMSHandle != -1)
           {
              EMSsize *= 16384;
              memcpy(table, &EMSCacheTable, sizeof(struct LowCacheFunctions));
              MapEMSPage(0);
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

void CloseEMSCache()
{
     EMSfree(EMSHandle);
}

static int MapEMSPage(int logicalpage)
{
    if (CurrentEMSPage != logicalpage)
    {
       CurrentEMSPage = logicalpage;
       return EMSmap(0, EMSHandle, logicalpage);
    }
    return 0;
}

static int Invalidate()
{
   /* Calculate the number of sectors we can fit */
   TotalSectorCount = EMSsize / (BYTESPERSECTOR + sizeof(struct SectorTag));
   
   /* No sectors filled. */
   CachedSectorCount = 0;
   
   /* Always succeeds */
   return TRUE;
}

static int EMSReadWrite(unsigned long emsoffset,
                        unsigned bufsegment, unsigned bufoffset,
                        unsigned length,
                        int direction)
{
   int page;
   unsigned bankoffset;
   
   /* Calculate corresponding page(s). */
   page       = (int) (emsoffset / 16384);
   bankoffset = (unsigned) (emsoffset % 16384);
   if (MapEMSPage(page) == -1) return FALSE;
   
   if (bankoffset + length > 16384)
   {   
      if (direction == READ)     
         movedata(EMSBase, bankoffset, bufsegment, bufoffset, 16384-bankoffset);  
      else
         movedata(bufsegment, bufoffset, EMSBase, bankoffset, 16384-bankoffset);              
               
      if (MapEMSPage(page+1) == -1) return FALSE;  
      
      if (direction == READ)
         movedata(EMSBase, 0, 
                  bufsegment, bufoffset+16384-bankoffset, 
                  length-(16384-bankoffset));      
      else
         movedata(EMSBase, 0, 
                  bufsegment, bufoffset+16384-bankoffset, 
                  length-(16384-bankoffset));      
   }        
   else
   {        
      if (direction == READ)
         movedata(EMSBase, bankoffset, bufsegment, bufoffset, 
                  length);                                  
      else
         movedata(bufsegment, bufoffset, EMSBase, bankoffset,  
                  length);  
   }

   return TRUE;
}                        

static int GetTag(unsigned long index, struct SectorTag* buffer)
{
   struct SectorTag far* fpBuffer = (struct SectorTag far*) buffer;
     
   index *= sizeof(struct SectorTag);

   return EMSReadWrite(index, FP_SEG(fpBuffer), FP_OFF(fpBuffer),
                       sizeof(struct SectorTag), READ);
}

static int WriteTag(unsigned long index, struct SectorTag* tag)
{
   struct SectorTag far* fpBuffer = (struct SectorTag far*) tag;
     
   index *= sizeof(struct SectorTag);
   
   return EMSReadWrite(index, FP_SEG(fpBuffer), FP_OFF(fpBuffer),
                       sizeof(struct SectorTag), WRITE);
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
  char far* fpBuffer = (char far*) buffer;
  
  /* See if it is not already in the cache */
  if ((offset = GetAddress(devid, sector)) != 0)
  {
     /* If it is bump up the age and overwrite it. */
     if (!IncrementAge(sector)) return FALSE;
          
     if (!EMSReadWrite(offset, FP_SEG(fpBuffer), FP_OFF(fpBuffer),
                       BYTESPERSECTOR, WRITE))
     {
        /* PANIC - just close down the system and return */
        CloseEMSCache(); /* This almost never happens (EMS = RAM),
                            unless you have a faulty expansion board 
                            and no work should be done with it.      */ 
     
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
         if (EMSReadWrite(offset, FP_SEG(fpBuffer), FP_OFF(fpBuffer),
                          BYTESPERSECTOR, WRITE))
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
  
  if ((!EMSReadWrite(tag.sectoroffset, FP_SEG(fpBuffer), FP_OFF(fpBuffer),
                    BYTESPERSECTOR, WRITE)) ||
      (!WriteTag(index, &tag)))
  {
     /* PANIC - just close down the system and return */
     CloseEMSCache(); 
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
       if (EMSReadWrite(offset, FP_SEG(buffer), FP_OFF(buffer),
                        BYTESPERSECTOR, READ))
       {
          /* Bump up the age */  
          if (!IncrementAge(sector)) return FALSE;
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
   unsigned long i, j;
   struct SectorTag tag;
   
   for (i = 0; i < CachedSectorCount; i++)
   {
       if (!GetTag(i, &tag))
       {        
          /* PANIC - just close down the system and return */
          CloseEMSCache(); 
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
                CloseEMSCache(); 
                return FALSE;
             } 
             if (!WriteTag(j-1, &tag))
             {        
                /* PANIC - just close down the system and return */
                CloseEMSCache(); 
                return FALSE;
             } 
          }
          
          CachedSectorCount--;
       }   
   }
    
   return TRUE;
}
