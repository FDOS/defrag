/*    
   Backmem.c - heap memory manager for a pre-reserved amount of memory.

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
#include <string.h>

#include "bool.h"
#include "backmem.h"
#include "FTEerr.h"

struct FreeListElement
{
    unsigned size;

    unsigned positionprev;
    unsigned positionnext;

    unsigned sizenext;
};

struct ReservedListElement
{
    unsigned size;
    unsigned next;
};

#define SMALLESTFREEBLOCK sizeof(struct FreeListElement)

#define ABSOLUTEPOINTER(offset) (void*)(((char*)BackupMemory) + offset)
#define RELATIVEPOINTER(pointer)(unsigned)(((char*)pointer) - ((char*)BackupMemory))

static void*    BackupMemory     = NULL;
static void*    FreePositionHead = 0;
static void*    FreeSizeHead     = 0;
static void*    ReservedHead     = 0;
static unsigned BackupSize       = 0;

static void* GetFirstBlock(void)
{
   return FreePositionHead;
}

static void* GetNextBlock(void* block)
{
   return ABSOLUTEPOINTER(((struct FreeListElement*) block)->positionnext);
}
/*
static void* GetPreviousBlock(void* block)
{
   return ABSOLUTEPOINTER(((struct FreeListElement*) block)->positionprev);
}
*/
static void* GetSmallestBlock(void)
{
   return FreeSizeHead;
}

static void* GetLargerBlock(void* block)
{
   return ABSOLUTEPOINTER(((struct FreeListElement*) block)->sizenext);
}

static unsigned GetBlockSize(void* block)
{
   return ((struct FreeListElement*) block)->size;
}

static void* GetFittingBlock(unsigned size)
{
   unsigned currentsize;
   void* current;

   current = GetSmallestBlock();
   while (current)
   {
      currentsize = GetBlockSize(current);
      if (size <= currentsize)
         return current;

      current = GetLargerBlock(current);
   }

   return NULL;
}

static BOOL IsFirstFreeBlock(void* block)
{
   return (char*)FreePositionHead == (char*)block;
}

static void RemoveBlockFromPositionList(void* block)
{
   void* previous;
   void* next;
   unsigned uprevious, unext;

   uprevious = ((struct FreeListElement*) block)->positionprev;
   unext = ((struct FreeListElement*) block)->positionnext;

   if (FreePositionHead != block)
   {
      previous=ABSOLUTEPOINTER(uprevious);
      ((struct FreeListElement*)previous)->positionnext = unext;
   }

   if (unext)
   {
      next=ABSOLUTEPOINTER(unext);
      ((struct FreeListElement*)next)->positionprev = uprevious;
   }
}

static void DecreaseFreeBlock(void* block, unsigned size)
{
   void* previous;
   void* next;
   unsigned uprevious, unext;

   uprevious = ((struct FreeListElement*) block)->positionprev;
   unext = ((struct FreeListElement*) block)->positionnext;

   if (FreePositionHead != block)
   {
      previous=ABSOLUTEPOINTER(uprevious);
      ((struct FreeListElement*)previous)->positionnext+=size;
   }

   if (unext)
   {
      next=ABSOLUTEPOINTER(unext);
      ((struct FreeListElement*)next)->positionprev+=size;
   }

   ((struct FreeListElement*) block)->size -= size;
   memcpy(((char*)block)+size, block, sizeof(struct FreeListElement));
}

static void InsertBlockIntoPositionList(void* block)
{
   void* current, *previous;

   current = GetFirstBlock();
   if ((char*)block < (char*)current) /* Make this the head of the list */
   {
         ((struct FreeListElement*) current)->positionprev =
                                              RELATIVEPOINTER(block);
         ((struct FreeListElement*) block)->positionnext =
                                              RELATIVEPOINTER(current);
	 FreePositionHead = block;
   }
   else                               /* Insert somewhere in the middle */
   {
      do {
          previous = current;         /* We need to keep a chaising pointer
                                         to be able to add to the end
                                         of the list.                      */
          current  = GetNextBlock(current);
      } while (current && ((char*)block < (char*)current));

      if (current)
      {
         ((struct FreeListElement*) current)->positionprev =
                                              RELATIVEPOINTER(block);
         ((struct FreeListElement*) previous)->positionnext =
                                               RELATIVEPOINTER(block);
         ((struct FreeListElement*) block)->positionprev =
                                               RELATIVEPOINTER(previous);
         ((struct FreeListElement*) block)->positionnext =
                                               RELATIVEPOINTER(current);
      }
      else            /* Add to the end of the list */
      {
         ((struct FreeListElement*) previous)->positionnext =
                                               RELATIVEPOINTER(block);
         ((struct FreeListElement*) block)->positionprev =
                                               RELATIVEPOINTER(previous);
         ((struct FreeListElement*) block)->positionnext = 0;
      }
   }
}

static void RemoveFromSizeList(void* block)
{
   void* chaising=NULL, *current;

   if (!FreeSizeHead) /* Size list empty */
      return;

   current = GetSmallestBlock();
   if (!current) return;

   while (current)
   {
      if (current == block)
      {
         if (chaising)
         {
            ((struct FreeListElement*) chaising)->sizenext =
                           ((struct FreeListElement*) current)->sizenext;
         }
         else
         {
	    if (((struct FreeListElement*) current)->sizenext == 0)
	    {
	       FreeSizeHead = 0;
	    }
	    else
	    {
	       FreeSizeHead = ABSOLUTEPOINTER(
			      ((struct FreeListElement*) current)->sizenext);
	    }
         }
         return;
      }

      chaising = current;
      current  = GetLargerBlock(current);
   }
}

static void InsertIntoFreeList(void* block)
{
   unsigned size = ((struct FreeListElement*)block)->size;
   void* chaising = NULL, *current;

   if (!FreeSizeHead)
   {
      FreeSizeHead = block;
      ((struct FreeListElement*) block)->sizenext = 0;
      return;
   }

   current = GetSmallestBlock();
   if (!current) return;

   while (current)
   {
      if (GetBlockSize(current) >= size)
      {
	 if (chaising)
	 {
	    ((struct FreeListElement*) chaising)->sizenext =
					       RELATIVEPOINTER(block);
	    ((struct FreeListElement*) block)->sizenext =
					    RELATIVEPOINTER(current);
	 }
	 else /* Make this the head of the list */
	 {
	    FreeSizeHead = block;

	    ((struct FreeListElement*) block)->sizenext =
					    RELATIVEPOINTER(current);
	 }

	 return;
      }

      chaising = current;
      current  = GetLargerBlock(current);
   }

   /* Add the block at the end of the size list */
   ((struct FreeListElement*) chaising)->sizenext =
					       RELATIVEPOINTER(block);
   ((struct FreeListElement*) block)->sizenext = 0;
}

static void MergeFreeBlock(void* block)
{
   void* previous, *current = block;
   void* next;
   unsigned uprevious, unext, size;

   uprevious = ((struct FreeListElement*) block)->positionprev;
   unext = ((struct FreeListElement*) block)->positionnext;

   if (FreePositionHead != block)
   {
      previous = ABSOLUTEPOINTER(uprevious);
      
      size = ((struct FreeListElement*) previous)->size;

      if (((char*) previous+size) == block)
      {
         ((struct FreeListElement*) previous)->size += GetBlockSize(block);
	 RemoveFromSizeList(previous);
	 RemoveFromSizeList(block);
	 InsertIntoFreeList(previous);
	 RemoveBlockFromPositionList(block);
         current = previous;
      }
   }

   if (unext)
   {
      next = ABSOLUTEPOINTER(unext);

      if (((char*)current+GetBlockSize(current)) == next)
      {
         ((struct FreeListElement*) current)->size += GetBlockSize(next);
	 RemoveFromSizeList(current);
	 RemoveFromSizeList(next);
	 InsertIntoFreeList(current);
	 RemoveBlockFromPositionList(next);
      }
   }
}

static void MarkAsReserved(void* block)
{
   if (ReservedHead)
   {
      ((struct ReservedListElement*) block)->next =
					       RELATIVEPOINTER(ReservedHead);
   }
   else /* No blocks reserved yet */
   {
      ((struct ReservedListElement*) block)->next = 0;
      ReservedHead = block;
   }
}

static BOOL IsReserved(void* block)
{
   void* current = ReservedHead;

   while (current)
   {
      if (current == block)
         return TRUE;

      current = ABSOLUTEPOINTER(
		     ((struct ReservedListElement*) current)->next);
   }
   return FALSE;
}

BOOL AllocateBackupMemory(unsigned size)
{
   if (size < SMALLESTFREEBLOCK)
      size = SMALLESTFREEBLOCK;

   BackupMemory = malloc(size);
   if (BackupMemory)
   {
      ((struct FreeListElement*)BackupMemory)->size = size;
      ((struct FreeListElement*)BackupMemory)->positionprev = 0;
      ((struct FreeListElement*)BackupMemory)->positionnext = 0;
      ((struct FreeListElement*)BackupMemory)->sizenext = 0;

      FreeSizeHead     = BackupMemory;
      FreePositionHead = BackupMemory;
      BackupSize       = size;
   }
   
   return BackupMemory != 0;
}

void FreeBackupMemory(void)
{
   if (BackupMemory) free(BackupMemory);
}

void* BackupAlloc(unsigned size)
{
   void* newblock;

   size += sizeof(struct ReservedListElement);
   if (size < SMALLESTFREEBLOCK)
      size = SMALLESTFREEBLOCK;

   newblock = GetFittingBlock(size);
   if (newblock)
   {
      if (GetBlockSize(newblock) == size)
      {
         RemoveBlockFromPositionList(newblock);
         RemoveFromSizeList(newblock);
      }
      else
      {
	 DecreaseFreeBlock(newblock, size);

	 if (IsFirstFreeBlock(newblock))
	 {
	    FreePositionHead = ((char*)newblock+size);
	 }

         RemoveFromSizeList(newblock);
         InsertIntoFreeList((void*)((char*)newblock+size));
      }

      MarkAsReserved(newblock);
      ((struct ReservedListElement*) newblock)->size = size;
      return (void*)(((struct ReservedListElement*)newblock)+1);
   }

   return NULL;
}

void BackupFree(void* block)
{
   /* Go to the real begining of the block. */
   block = (void*) (((struct ReservedListElement*)block)-1);

   if (IsReserved(block))
   {
      InsertBlockIntoPositionList(block);
      InsertIntoFreeList(block);
      MergeFreeBlock(block);
   }
   else
   {
      SetFTEerror(FTE_NOT_RESERVED);
   }
}

BOOL InBackupMemoryRange(void* block)
{
   return ((((char*) BackupMemory) <= ((char*)block)) &&
           (((char*) BackupMemory + BackupSize) > ((char*) block)));
}

#ifdef DEBUG

void PrintFreeList(void)
{


}

void PrintReservedList(void)
{
   void* current = ReservedHead;

   while (current)
   {
      printf("offset: %d", );

      current = ((struct ReservedListElement*) current)->next;
   }
}


#endif
