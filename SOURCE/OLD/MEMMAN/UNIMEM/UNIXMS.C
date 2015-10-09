/*
   Unixms - xms unification.

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

#include "..\..\misc\bool.h"

#include "unimem.h"
#include "..\low\xms.h"

static int Alloc(struct UniMemType* this);
static int Release(struct UniMemType* this);
static int Read(unsigned long offset, unsigned bytes, void* buffer,
	 struct UniMemType* this);
static int Write(unsigned long offset, unsigned bytes, void* buffer,
	  struct UniMemType* this);

int UnifyXMS(struct UniMemType* MemType)
{
   /* Initialise data members. */
   MemType->resizable = FALSE;

   /* Initialise function members. */
   MemType->Alloc   = Alloc;
   MemType->Release = Release;
   MemType->Read    = Read;
   MemType->Write   = Write;

   /* See if XMS available. */
   return XMSinit();
}

static int Alloc(struct UniMemType* this)
{
   int  handle;
   long size;

   /* Try allocating all memory. */
   /* Get the size of the largest free EMB. */
   this->handle = 0;

   size = XMScoreleft();
   if (size == 0) return FALSE; /* No more memory left. */
      
   if ((handle = XMSalloc(size)) != 0)
   {
      this->handle    = handle;
      this->reserved  = (unsigned long) size;
      this->allocated = 0;
   }
   else
      return FALSE;

   return TRUE;
}

static int Release(struct UniMemType* this)
{
   if (this->handle) 
      return XMSfree(this->handle);
   else
      return TRUE;
}

static int Read(unsigned long offset, unsigned bytes, void* buffer,
		struct UniMemType* this)
{
   if (this->handle)
      return XMStoDOSmove((char*) buffer, this->handle,
			  (long) offset, bytes);
   else
      return TRUE;
}

static int Write(unsigned long offset, unsigned bytes, void* buffer,
		 struct UniMemType* this)
{
   if (this->handle)
      return DOStoXMSmove(this->handle, (long) offset,
			  (char*) buffer, bytes);
   else
      return TRUE;
}
