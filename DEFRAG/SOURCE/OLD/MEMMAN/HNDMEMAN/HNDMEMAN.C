/*
   Hndmeman - handle memory manager.

   Copyright (C) 2000, Imre Leber.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have recieved a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   If you have any questions, comments, suggestions, or fixes please
   email me at:  imre.leber@worldonline.be

*/


#include "hndmeman.h"

static struct BlockInfo Handles[AMOFHANDLES];

static int HandleOrderListHead;

int HMemInit(void)
{
    memset(Handles, 0, AMOFHANDLES*sizeof(struct BlockInfo));

    return UMemInit();
}

void HMemClose(void)
{
    UMemClose();
}

/* Returns handle or -1 if error. */
int HMemAlloc(unsigned long size)
{
    int pointer, chaiser;
    int handle;

    if (size == 0) return -1; 

    /* find a free handle. */
    for (handle = 0; i < AMOFHANDLES; handle++)
        if (Handles[handle].Length == 0) 
           break;

    if (handle == AMOFHANDLES) return -1;

    /* find a memory chunk big enough to hold data. */
    pointer = HandleOrderListHead;
    
    if (pointer) 
       while ((pointer= Handles[pointer-1].next) != 0)
       {
             if (Handle[pointer-1].next == 0) break;
          
             if ((Handles[Handles[pointer-1].next].Offset - 
                  (Handles[pointer-1].Offset + Handles[pointer-1].Length))
                 >= size)
                break;
       }

    /* Fill handle struct. */
    if (pointer)
    {
       Handles[handle].Offset = Handles[pointer-1].Offset + 
                                Handles[pointer-1].Length;
       
       if (Handles[pointer-1].next)
          Handles[Handles[pointer-1].next].previous = handle+1;
       
       Handles[pointer-1].next = handle+1;
    }
    else
       Handles[handle].Offset = 0; 
       
    Handles[handle].Length = size;

    return handle;
}

void HMemRelease(int handle)
{
    Handles[handle].Length = 0;

    if (Handles[handle].next == 0)
       UMemTruncate(Handles[handle].Offset);

    if (handle == HandleOrderListHead) 
    {
       HandleOrderListHead = Handles[handle].next;
       Handles[Handles[handle].next-1].previous = 0;
    }
    else
    {
       Handles[Handles[handle].previous-1].next = Handles[handle].next;
       Handles[Handles[handle].next-1].previous = Handles[handle].previous;
    }
}

int HMemRead(int handle, unsigned long offset, unsigned bytes, void* buffer)
{
   return UMemRead(offset+Handles[handle].Offset, bytes, buffer);
}

int HMemWrite(int handle, unsigned long offset, unsigned bytes, void* buffer)
{
   return UMemWrite(offset+Handles[handle].Offset, bytes, buffer);
}

static int UMemCopy(unsigned long source, unsigned long dest, 
                    unsigned long size)
{
   int  i;
   char buffer[256];

   for (i = 0; i < (size / 256); i++)
   {
       if (!UMemRead(source, 256, buffer)) return FALSE;
       if (!UMemWrite(dest,  256, buffer)) return FALSE;
       
       source += 256;
       dest   += 256;
   }

   if (!UMemRead(source, (unsigned) (size % 256), buffer) return FALSE;
   if (!UMemWrite(dest,  (unsigned) (size % 256), buffer) return FALSE;
   
   return TRUE;
}

static int CollectHMem(void)
{
   int pointer;
   unsigned long offset = 0;

   pointer = HandleOrderListHead;
   while (pointer)
   {
         if (!UMemCopy(Handles[pointer-1].Offset, offset, 
                       Handles[pointer-1].Length))
            return FALSE;

         Handles[pointer-1].Offset = offset;

         offset += Handles[pointer-1].Length;

         pointer = Handles[pointer-1].next;
   }

   return TRUE;
}
