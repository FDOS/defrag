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

static int MemTypeHead; /* Head of the memory allocation list. */ 

int UMemInit()
{
  int i, order = 1, seen = FALSE, last;

  for (i = 0; i < AMOFMEMTYPES; i++)
      if (MemorySet[i].active)
	 if (MemorySet[i].init(&MemorySet[i].memtype))
	 {
	    MemorySet[i].initialised = TRUE;
	    
	    if (!seen) MemTypeHead   = i+1;
	    MemorySet[i].next        = ++order;        
	    seen                     = TRUE;
	    last                     = i;
	 }
	 else
	 {
	    MemorySet[i].active      = FALSE;
	    MemorySet[i].initialised = FALSE;
	 }

  if (seen) MemorySet[last].next = 0;  /* Identify end of memory type 
					  allocation list.             */
  return seen;
}

int ActivateMemType(char* idcode)
{
  int i, seen = FALSE;
  int pointer, chaiser;

  for (i = 0; i < AMOFMEMTYPES; i++)
      if (strcmp(idcode, MemorySet[i].idcode) == 0)
      {  
	 seen = TRUE;

	 if (!MemorySet[i].active) 
	 {
	    MemorySet[i].active = TRUE;
	    
	    pointer = MemTypeHead;
	    while (pointer = MemorySet[pointer-1].next) chaiser = pointer;
	    MemorySet[chaiser-1].next = i+1;
	    MemorySet[i].next         = 0;
	    
	    if (!MemorySet[i].initialised) 
	       MemorySet[i].init(&MemorySet[i].memtype);
	 }
	 break;
      }

  return seen;
}

int DeActivateMemType(char* idcode)
{
  int pointer, chaiser = 0;

  pointer = MemTypeHead;
  while (pointer)
  {
      if (strcmp(idcode, MemorySet[pointer-1].idcode) == 0)
      {
	 if (MemorySet[pointer-1].allocated > 0) return FALSE;

	 if (chaiser)
	    MemorySet[chaiser-1].next = MemorySet[pointer-1].next;
	 else
	    MemTypeHead = pointer;

	 MemorySet[i].active = FALSE;

	 return TRUE;
      }
  
      chaiser = pointer;
      pointer = MemorySet[pointer-1].next
  }
  
  
  
  
  int i, seen = FALSE;


  for (i = 0; i < AMOFMEMTYPES; i++)
      if (strcmp(idcode, MemorySet[i].idcode) == 0)
      {  
	 seen = TRUE;

	 /* Refuse if memory allocated in this memory type. */
	 

	 
	 NormalizeIndexes();
	 break;
      }

  return seen;
}

void UMemClose()
{
}

void UMemRead(unsigned long offset, int bytes, void* buffer)
{

}

void UMemWrite(unsigned long offset, int bytes, void* buffer)
{

}

