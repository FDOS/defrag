/*
   Unimain - main memory unification.

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
#include "..\low\mainmem.h"

static int Alloc(struct UniMemType* this);
static int Release(struct UniMemType* this);
static int Resize(unsigned long newsize, struct UniMemType* this);
static int Read(unsigned long offset, unsigned bytes, void* buffer, 
                struct UniMemType* this);
static int Write(unsigned long offset, unsigned bytes, void* buffer,
                 struct UniMemType* this);

int UnifyMainMem(struct UniMemType* MemType)
{
    /* Initialise data members. */
    MemType->resizable = TRUE;

    /* Initialise function members. */
    MemType->Alloc   = Alloc;
    MemType->Release = Release;
    MemType->Read    = Read;
    MemType->Write   = Write;
    MemType->Resize  = Resize;
    
    return TRUE;
}

static int Alloc(struct UniMemType* this)
{
    /* Try to allocate standard memory block. */ 
    if (MainMemAlloc(0))
    {
       this->reserved  = 1024;
       this->allocated = 0;
       return TRUE;
    }
    
    return FALSE;
}

static int Release(struct UniMemType* this)
{
    /* Try to release all memory. */
    MainMemClose();
    return TRUE;
}

static int Resize(unsigned long newsize, struct UniMemType* this)
{
    return MainMemTruncate(newsize);
}

static int Read(unsigned long offset, unsigned bytes, void* buffer, 
                struct UniMemType* this)
{
    return MainMemRead(offset, bytes, buffer);
}

static int Write(unsigned long offset, unsigned bytes, void* buffer,
                 struct UniMemType* this)
{
    return MainMemWrite(offset, bytes, buffer);
}
