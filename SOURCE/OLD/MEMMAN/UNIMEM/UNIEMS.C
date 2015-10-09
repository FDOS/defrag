/*
   Uniems - ems unification.

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


#include <stdlib.h>
#include <mem.h>
#include <dos.h>

#include "..\..\misc\bool.h"

#include "unimem.h"
#include "..\low\ems.h"

static int Alloc(struct UniMemType* this);
static int Release(struct UniMemType* this);
static int Read(unsigned long offset, unsigned bytes, void* buffer,
	 struct UniMemType* this);
static int Write(unsigned long offset, unsigned bytes, void* buffer,
	  struct UniMemType* this);

struct EMSPrivateStruct
{
   unsigned BaseAddress;
};

int UnifyEMS(struct UniMemType* MemType)
{
   /* Initialise data members. */
   MemType->handle    = -1;
   MemType->resizable = FALSE;
   
   MemType->buffer = malloc(sizeof(struct EMSPrivateStruct));
   if (!MemType->buffer) return FALSE;

   ((struct EMSPrivateStruct*) MemType->buffer)->BaseAddress 
							  = EMSbaseaddress();

   if (!((struct EMSPrivateStruct*) MemType->buffer)->BaseAddress)
      return FALSE;

   if (EMSversion() == -1) return FALSE;      /* Good version?  */
   if (EMSstatus())        return FALSE;      /* EMS status OK? */
   
   /* Initialise function members. */
   MemType->Alloc   = Alloc;
   MemType->Release = Release;
   MemType->Write   = Write;
   MemType->Read    = Read;

   return TRUE;
}

static int Alloc(struct UniMemType* this)
{
   int size, handle;

   if ((size = EMSpages()) > 0) return FALSE; /* EMS available? */

   if ((handle = EMSalloc(size)) != -1)
   {
      this->handle    = handle;
      this->reserved  = (unsigned long) size * 16384;
      this->allocated = 0;
   }
   else 
      return FALSE;

   return TRUE;
}

static int Release(struct UniMemType* this)
{
   if (this->handle != -1) 
      return EMSfree(this->handle) + 1; /* Normalize return val.*/
   else
      return TRUE;
}

static int PrivateCopyBlock(int handle, int page, 
			    unsigned bufseg, unsigned bufoff,
			    unsigned bseg, unsigned boff, unsigned tocopy, 
			    int ForReading)
{
    int i; 

    for (i = 0; i < 4; i++)
    {
	if (EMSmap(i, handle, page) == 0)
	{
	   if (ForReading)
	      movedata(bseg, boff + i * 16384, bufseg, bufoff, tocopy);
	   else
	      movedata(bufseg, bufoff, bseg, boff + i * 16384, tocopy);

	   return TRUE;
	}
    }
    
    return FALSE;
}

static int PrivateReadWrite(unsigned long offset, unsigned bytes, 
			    char* buffer, struct UniMemType* this, 
			    int ForReading)
{
    int      i;
    unsigned page, rest, baddress, boffset, tocopy;
    unsigned bufseg = FP_SEG((char far*) buffer);
    unsigned bufoff = FP_OFF((char far*) buffer);

    baddress = ((struct EMSPrivateStruct*) this->buffer)->BaseAddress;

    /* Calculate wich page we need to start with. */
    page    = (unsigned) (offset / 16384);
    boffset = (unsigned) (offset % 16384);
    rest    = 16384 - boffset;

    if (rest > bytes)
       tocopy = bytes;
    else
       tocopy = rest;

    /* Copy first part. */
    if (!PrivateCopyBlock(this->handle, page++, bufseg, bufoff, 
			  baddress, boffset,
			  tocopy, ForReading))
       return FALSE;

    bytes   = bytes - tocopy;
    bufoff += tocopy;

    for (i = 0; i < bytes / 16384; i++)
    {
	if (PrivateCopyBlock(this->handle, page++, bufseg, bufoff, 
			     baddress, boffset,
			     16384, ForReading))
	   return FALSE; 
	   
	bufoff += 16384;
    }

    bytes %= 16384;

    /* Copy last part. */
    if (bytes > 0)
       if (!PrivateCopyBlock(this->handle, page, bufseg, bufoff, 
			     baddress, boffset,
			     bytes, ForReading))
	  return FALSE;

    return TRUE;
}

static int Read(unsigned long offset, unsigned bytes, void* buffer,
		struct UniMemType* this)
{
    return PrivateReadWrite(offset, bytes, buffer, this, TRUE);
}

static int Write(unsigned long offset, unsigned bytes, void* buffer,
		 struct UniMemType* this)
{
    return PrivateReadWrite(offset, bytes, buffer, this, FALSE);
}
