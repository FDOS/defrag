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

#include "..\..\misc\bool.h"
#include "..\header\fat.h"
#include "..\header\fatconst.h"
#include "..\header\sctcache.h"
#include "..\header\rdwrsect.h"
#include "xms.h"
#include "xmscache.h"

static unsigned int  XMSHandle;
static unsigned long XMSsize;
static unsigned long TotalSectorCount;
static unsigned long CachedSectorCount;

/* An entry in the hash table */
struct SectorTag
{
     SECTOR sectornumber;
     unsigned DeviceID;
     
     unsigned long sectoroffset;

     unsigned long ThisTag;
     unsigned long NextTag; /* Pointer to the next one in the bucket list */

     unsigned long FreeNext;
};

/* Public function prototypes */
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

/* Private function prototypes */
static void TagIsUsed(unsigned long index);
static unsigned long GetOverwritableTag(void);
static BOOL GetCachedTag(unsigned long index, struct SectorTag* buffer);
static BOOL IsCachedTagPresent(unsigned long index);
static void WriteCachedTag(unsigned long index, struct SectorTag* buffer);
static int TruelyWriteTag(unsigned long index);
static int GetTag(unsigned long index, struct SectorTag* buffer);
static int WriteTag(unsigned long index, struct SectorTag* tag);
static int HashTag(struct SectorTag* tag);
static int UnhashTag(struct SectorTag* tag);
static unsigned long FindSectorTag(unsigned devid, SECTOR sector,
                                   struct SectorTag* result,
                                   BOOL* error);
static int BlindlyWriteTag(unsigned long index, struct SectorTag* tag);

#define AMOFRECENTTAGS 128

static unsigned long RecentTags[AMOFRECENTTAGS];

static int RecentTagPointer = 0;
static int RecentTagCount   = 0;

/* Write back cache */
#define AMOFCACHEDTAGS 64 /* Number of tags that are cached in
                             conventional memory.              */
#define AMOFCACHEDMASK AMOFCACHEDTAGS - 1

static struct SectorTag TagCache[AMOFCACHEDTAGS];

static unsigned long FreeIndexListHead = 0xFFFFFFFFL;

/* Head of the list with sector offsets that have been uncached. */
static unsigned long UncachedSectorHead = 0;

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
              if (!Invalidate())
              {
                 CloseXMSCache();
                 table->supported = FALSE;
              }
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
   unsigned long i;
   struct SectorTag tag;
     
   /* Calculate the number of sectors we can fit */
   TotalSectorCount = XMSsize / (BYTESPERSECTOR + sizeof(struct SectorTag));
   
   /* No sectors filled. */
   CachedSectorCount = 0;

   /* Initialize the recent tag list. */
   RecentTagPointer = 0;
   RecentTagCount   = 0;

   /* Initialise the hash table. */
   memset(&tag, 0, sizeof(struct SectorTag));
   tag.ThisTag = 0xFFFFFFFFL;
   tag.NextTag = 0xFFFFFFFFL;

   /* Initialise free list */
   for (i = 0; i < TotalSectorCount; i++)
   {
       if (i < TotalSectorCount-1)
	  tag.FreeNext = i+1;
       else
	  tag.FreeNext = 0xFFFFFFFFL;
       
       if (!BlindlyWriteTag(i, &tag))
	  return FALSE;
   }
   FreeIndexListHead = 0;

   for (i = 0; i < AMOFCACHEDTAGS; i++)
   {
       memcpy(&TagCache[(int)i], &tag, sizeof(struct SectorTag));
   }

   UncachedSectorHead = 0;

   return TRUE;
}

static void TagIsUsed(unsigned long index)
{
   int i;

   /* See wether the index is in the list of indexes */
   for (i = 0; i < RecentTagCount; i++)
   {
       if (RecentTags[i] == index) break;
   }

   /* If it is move it to the front. */
   if ((RecentTags[i] == index) && i && (i < RecentTagCount))
   {
      memmove(&RecentTags[i], &RecentTags[i-1],
              (i-1) * sizeof(unsigned long));
      RecentTags[0] = index;
   }

   /* Otherwise add it to the end of the list. */
   RecentTags[RecentTagCount] = index;
   if (RecentTagCount < AMOFRECENTTAGS-1) RecentTagCount++;
}

static unsigned long GetOverwritableTag(void)
{
   int i, j;
   BOOL found;

   /* Take one that is not in the recent tag list. */
   for (i = RecentTagPointer; i < TotalSectorCount; i=(i+1)%AMOFRECENTTAGS)
   {
       found = FALSE;
       for (j = 0; j < RecentTagCount; j++)
       {
           if (RecentTags[j] == i)
           {
              found = TRUE;
              break;
           }
       }
       if (!found) return i;
   }

   RecentTagPointer = (RecentTagPointer + 1) % AMOFRECENTTAGS;
   
   return 0; /* Cannot happen (RecentTagCount < TotalSectorCount) */
}

static BOOL GetCachedTag(unsigned long index, struct SectorTag* buffer)
{
    int index1 = (int)(index & AMOFCACHEDMASK);
    
    if (TagCache[index1].ThisTag == index)
    {
       memcpy(buffer, &TagCache[index1], sizeof(struct SectorTag));
       return TRUE;
    }
    else
    {
       return FALSE;
    }
}

static BOOL IsCachedTagPresent(unsigned long index)
{
    int index1 = (int)(index & AMOFCACHEDMASK);
    return TagCache[index1].ThisTag != 0xFFFFFFFFL;
}

static void WriteCachedTag(unsigned long index, struct SectorTag* buffer)
{
    memcpy(&TagCache[(int)(index & AMOFCACHEDMASK)],
           buffer, sizeof(struct SectorTag));
}

static int BlindlyWriteTag(unsigned long index, struct SectorTag* tag)
{
   return DOStoXMSmove(XMSHandle, index*sizeof(struct SectorTag),
		       (char*) tag, sizeof(struct SectorTag))
          == sizeof(struct SectorTag);
}

static int TruelyWriteTag(unsigned long index)
{
   int index1 = (int)(index & AMOFCACHEDMASK);

   index = TagCache[index1].ThisTag;

   index *= sizeof(struct SectorTag);

   return BlindlyWriteTag(index, &TagCache[index1]);
}

static int GetTag(unsigned long index, struct SectorTag* buffer)
{
   int retVal = TRUE;

   if (!GetCachedTag(index, buffer))
   {
      retVal = XMStoDOSmove((char*)buffer, XMSHandle,
			    index*sizeof(struct SectorTag),
			    sizeof(struct SectorTag)) ==
					       sizeof(struct SectorTag);

      if (retVal)
      {
         retVal = WriteTag(index, buffer);
      }
   }
                                         
   return retVal;
}

static int WriteTag(unsigned long index, struct SectorTag* tag)
{
   int index1 = (int)(index & AMOFCACHEDMASK);
    
   if ((TagCache[index1].ThisTag != index) &&
       (IsCachedTagPresent(index)))
   {
      if (!TruelyWriteTag(index)) return 0;
   }
         
   WriteCachedTag(index, tag);
   return 1;
}

static int HashTag(struct SectorTag* tag)
{
   struct SectorTag tag1, tag2;
   SECTOR sector = tag->sectornumber;
   unsigned long hashkey = (unsigned long) sector % TotalSectorCount;
   unsigned long index;

   /* See wether the tag is free. */
   if (!GetTag(hashkey, &tag1)) return 0;
   
   if (tag1.ThisTag == 0xFFFFFFFFL)
   {
      /* If so, write the tag. */
      tag->ThisTag = hashkey;

      tag->NextTag  = 0xFFFFFFFFL;
      tag->FreeNext = tag1.FreeNext;
      TagIsUsed(hashkey);
      return WriteTag(hashkey, tag);
   }
   else
   {
      /* Find a free slot in the hash table. */
      for (;;)
      {
	   /* Start looking at the head. */
	   index = FreeIndexListHead;

	   /* See wether it is free */
	   if (!GetTag(index, &tag2)) return 0;
	   if (tag2.ThisTag == 0xFFFFFFFFL)
	   {
	      /* This index will be filled in momentarily. */
              FreeIndexListHead = tag2.FreeNext;
	      break;
	   }
	   /* If it is not, remove the tag from the free list. */
	   FreeIndexListHead = tag2.FreeNext;
      }
      
      /* Add it to the linked list */
      tag->NextTag  = tag1.NextTag;
      tag1.NextTag  = index;
      tag->ThisTag  = index;
      tag->FreeNext = tag2.FreeNext;

      TagIsUsed(index);
      if (!WriteTag(index,   tag))  return 0;
      if (!WriteTag(hashkey, &tag1)) return 0;
   }
           
   return 1;
}

static int UnhashTag(struct SectorTag* tag)
{
   struct SectorTag tag1, tag2;
   SECTOR sector = tag->sectornumber;
   unsigned long hashkey = (unsigned long) sector % TotalSectorCount;
   unsigned long index, previndex;

   /* See wether the tag is free. */
   if (!GetTag(hashkey, &tag1)) return 0;

   /* If the tag is the first one in the slot. */
   if ((tag->sectornumber == tag1.sectornumber) &&
       (tag->DeviceID == tag1.DeviceID))
   {
      /* If there are no other tags in the slot, just make it available. */
      if (tag1.NextTag == 0xFFFFFFFFL)
      {
         memset(&tag1, 0, sizeof(struct SectorTag));
         tag1.ThisTag = 0xFFFFFFFFL;

         /* Add it to the free list. */
	 tag1.FreeNext = FreeIndexListHead;
         FreeIndexListHead = hashkey;
         
         if (!WriteTag(hashkey, &tag1)) return 0;
      }
      /* If there are move the next tag to the front of this slot. */
      else
      {
         if (!GetTag(tag1.NextTag, &tag2)) return 0;
         if (!WriteTag(hashkey, &tag2)) return 0;

         memset(&tag2, 0, sizeof(struct SectorTag));
         tag2.ThisTag = 0xFFFFFFFFL;

         /* Add it to the free list. */
	 tag2.FreeNext = FreeIndexListHead;
	 FreeIndexListHead = tag1.FreeNext;
         
         if (!WriteTag(hashkey, &tag2)) return 0;
      }
   }
   /* If it is not the first in the slot, search for it. */
   else
   {
      index = tag1.NextTag;
      previndex = hashkey;
      while (index != 0xFFFFFFFL)
      {
         if (!GetTag(index, &tag1)) return 0;

         if ((tag->sectornumber == tag1.sectornumber) &&
             (tag->DeviceID == tag1.DeviceID))
         {
            /* Found, remove it from the list. */
            if (!GetTag(previndex, &tag2)) return 0;
	    tag2.FreeNext = tag1.FreeNext;
            if (!WriteTag(previndex, &tag2)) return 0;

            /* And remove the found slot itself. */
            memset(&tag1, 0, sizeof(struct SectorTag));
            tag1.ThisTag = 0xFFFFFFFFL;

            /* Add it to the free list. */
	    tag1.FreeNext = FreeIndexListHead;
            FreeIndexListHead = index;
         
            if (!WriteTag(hashkey, &tag1)) return 0;
         }
         previndex = index;
         index = tag1.NextTag;
      }
   }
   return 1;
}

static unsigned long FindSectorTag(unsigned devid, SECTOR sector,
                                   struct SectorTag* result,
                                   BOOL* error)
{
   static struct SectorTag tag;
   unsigned long index = (unsigned long) sector % TotalSectorCount;   

   *error = FALSE;

   do {
        if (!GetTag(index, &tag))
        {
           *error = TRUE;
           return 0;
	}

	if (tag.ThisTag == 0xFFFFFFFFL)
	{
	   return tag.ThisTag;
	}

        if ((tag.DeviceID == devid) && (tag.sectornumber == sector))
        {
           memcpy(result, &tag, sizeof(struct SectorTag));
           return index;
        }

        index = tag.NextTag;

   } while (index != 0xFFFFFFFFL);

   return index;
}

static int IsFull(void)
{
  return CachedSectorCount == TotalSectorCount;
}

static int Cache(unsigned devid, SECTOR sector, char* buffer)
{
  BOOL error;
  unsigned long offset, index;
  struct SectorTag tag;
  
  /* See if it is not already in the cache */
  index = FindSectorTag(devid, sector, &tag, &error);
  if (error)
  {
     /* PANIC - just close down the system and return */
     CloseXMSCache(); /* This almost never happens (XMS = RAM) */
     return -2;
  }
  
  if (index != 0xFFFFFFFFL)
  {
     /* If it is mark it as used. */
     TagIsUsed(index);

     /* Overwrite the sector in the cache with the new value. */
     if (DOStoXMSmove(XMSHandle, tag.sectoroffset, buffer, BYTESPERSECTOR)
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
     if (UncachedSectorHead)
     {
        offset = UncachedSectorHead;
        if (XMStoDOSmove((char*)&UncachedSectorHead, XMSHandle, UncachedSectorHead,
                         sizeof(UncachedSectorHead)) !=
            sizeof(UncachedSectorHead))
        {
           /* PANIC - just close down the system and return */
           CloseXMSCache(); /* This almost never happens (XMS = RAM) */
           return -2;
        }
     }
     else
     {
        /* Calculate where to put the sector */
        offset = TotalSectorCount*sizeof(struct SectorTag) +
                 CachedSectorCount*BYTESPERSECTOR;
     }
     
     /* Fill in a new tag and write it. */
     tag.sectornumber = sector;
     tag.sectoroffset = offset;
     tag.DeviceID     = devid;

     if (HashTag(&tag))
     {
        if (DOStoXMSmove(XMSHandle, offset, buffer, BYTESPERSECTOR)
                         == BYTESPERSECTOR)
        {
           CachedSectorCount++;
           return TRUE;
        }
     }

     /* PANIC - just close down the system and return */
     CloseXMSCache(); /* This almost never happens (XMS = RAM) */
     return -2;
  }

  /* If the cache is full, search the oldest tag and replace it */
  index = GetOverwritableTag();

  /* Mark this index as available
     (notice that the FreeIndexListHead == 0xFFFFFFFFL,
      i.e. there are no free blocks). */
  FreeIndexListHead = index;

  /* Overwrite the information in the tag and the sector itself */
  tag.sectornumber = sector;
  tag.DeviceID     = devid;

  if ((DOStoXMSmove(XMSHandle, tag.sectoroffset, buffer, BYTESPERSECTOR)
                    != BYTESPERSECTOR) ||
      (!HashTag(&tag)))
  {
     /* PANIC - just close down the system and return */
     CloseXMSCache(); /* This almost never happens (XMS = RAM) */
     return -2;         
  }
  
  return TRUE;
}

static int Retreive(unsigned devid, SECTOR sector, char* buffer)
{
    BOOL error;
    unsigned long index;
    struct SectorTag tag;
    
    /* See if it is in the cache. */
    index = FindSectorTag(devid, sector, &tag, &error);
    if (error) return FALSE;

    if ((index != 0xFFFFFFFFL) &&
	(XMStoDOSmove(buffer, XMSHandle, tag.sectoroffset, BYTESPERSECTOR) ==
		      BYTESPERSECTOR))
    {
       /* Bump up the age. */
       TagIsUsed(index);
       return TRUE;
    }
    else
       return FALSE;
}

/* This function should be called whenever the integrity of a sector contents
   could be jeperdised by a cause external to the cache */
static int Uncache(unsigned devid, SECTOR sector)
{
   BOOL error;
   struct SectorTag tag;
    
   /* Find the place where the sector is located. */
   FindSectorTag(devid, sector, &tag, &error);
   if (error) return FALSE;

   if (DOStoXMSmove(XMSHandle, tag.sectoroffset, (char*)&UncachedSectorHead,
                    sizeof(UncachedSectorHead)) !=
      sizeof(UncachedSectorHead))
   {
      return FALSE;
   }
   UncachedSectorHead = tag.sectoroffset;

   /* Remove the tag from the hash table. */
   if (!UnhashTag(&tag)) return FALSE;

   return TRUE;
}

