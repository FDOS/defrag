/*
   Hndmeman - memory unification.

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

#include "unimem.h"

#include "initmems.inc"

static int MemTypeHead; /* Head of the memory type allocation list. */ 

int UMemInit(void)
{
    int i, order = 1, last = -1;

    for (i = 0; i < AMOFMEMTYPES; i++)
    {
	if ((MemorySet[i].active) &&
	    (MemorySet[i].init(&MemorySet[i].memtype))
	   {
	      MemorySet[i].initialised    = TRUE;
	      MemorySet[i].next           = ++order;
	      if (last == -1) MemTypeHead = i+1;
	      last                        = i;
	   }
    }

    if (last) MemorySet[last].next = 0;
    return last != -1;
}

int ActivateMemType(int idcode)
{
    int idpos, seen = FALSE;
    int chaiser=0, pointer;
    
    /* See which memory type is intended. */
    for (idpos = 0; idpos < AMOFMEMTYPES; idpos++)
	if (MemorySet[idpos].idcode == idcode)
	{
	   seen = TRUE;
	   break;
	}

    if (!seen) return FALSE;                 /* ID code not found! */

    /* See wether the memory type is already in the allocation list. */
    idpos++;
    pointer = MemTypeHead;
    while (pointer)
    {
	  if (pointer == idpos) return FALSE;
	  chaiser = pointer;
	  pointer = MemorySet[pointer-1].next;
    }

    /* Add the memory type to the end of the allocation list. */
    MemorySet[chaiser-1].next = idpos;
    MemorySet[idpos-1].next   = 0;

    return TRUE;
}

int DeActivateMemType(int idcode)
{
    int chaiser, pointer;

    pointer = MemTypeHeader;
    while (pointer)
    {
       if (MemorySet[pointer-1].idcode == idcode) 
       {
	  MemorySet[chaiser-1].next = MemorySet[pointer-1].next;
	  return TRUE;
       }

       chaiser = pointer;
       pointer = MemorySet[pointer-1].next;
    }

    return FALSE;
}

void UMemClose()
{
    int i;

    for (i = 0; i < AMOFMEMTYPES; i++)
    {
	if (MemorySet[i].initialised) 
	   MemorySet[i].memtype.Release(&MemorySet[i].memtype);
    }
}

void UMemRead(unsigned long offset, int bytes, void* buffer)
{

}

void UMemWrite(unsigned long offset, int bytes, void* buffer)
{

}

