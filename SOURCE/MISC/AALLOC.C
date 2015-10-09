/*    
   aalloc.c - automatic freeing of allocated memory.

   Copyright (C) 2001 Imre Leber

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

/*
   All returned pointers from malloc are put into a list. All pointers
   in this list are freed when calling autofree()
*/

#include <stdlib.h>

#include "aalloc.h"

struct AllocList
{
   void*  real;
   struct AllocList* next;
};

struct AllocList* alloclist = NULL;

void* amalloc(size_t size)
{
      void* p;
      struct AllocList* entry;

      p = malloc(size);
      if (!p)
         return NULL;

      entry = (struct AllocList*) malloc(sizeof(struct AllocList));
      if (!entry)
      {
         free(p);
         return NULL;
      }

      entry->real = p;
      entry->next = alloclist;
      alloclist   = entry;

      return p;
}

void* acalloc(size_t num_elements, size_t size)
{
      return amalloc(num_elements * size);
}

void autofree()
{
      struct AllocList* next;

      while (alloclist)
      {
         next = alloclist->next;
         free(alloclist->real);
         free(alloclist);
         alloclist = next;
      }
}
