/*
   Mainmem.c - main memory manager.
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

#include <alloc.h>
#include <mem.h>
#include <dos.h>

#include "mainmem.h"

static unsigned long Allocated = 0; /* Total allocated memory so far. */
static char far* FarBlock = NULL;

/*
** Initialises the far memory machine.
**
** Accepts: initial: amount of bytes for the initial far heap block.
**
** Returns: 0 if failure, 1 if success.
*/
int MainMemAlloc(unsigned long initial)
{
    if (initial == 0) initial = 1024;

    FarBlock = farmalloc(initial);

    if (FarBlock !=  0) 
    {
       Allocated = initial;
       return 1;
    }
    else
    {
       Allocated = 0;
       return 0;
    }
}

/*
** Close the memory machine.
*/
void MainMemClose(void)
{
    if (FarBlock && Allocated)
    {
       farfree(FarBlock);
       Allocated = 0;
    }
}

/*
    Private function to calculate the offset and segment for a certain
    block of memory.
*/
static void CalculateAddress(char far* basis, unsigned long increment,
			     unsigned* segment, unsigned* offset)
{
      *segment = FP_SEG(basis);
      *offset  = FP_OFF(basis);

      *segment += *offset / 16;    /* Normalise far pointer. */
      *offset   = *offset % 16;

      *segment = *segment + (unsigned) (increment / 16);
      *offset  = *offset  + (unsigned) (increment % 16);

      *segment += *offset / 16;    /* Normalise again. */
      *offset   = *offset % 16;
}

/*
** Reads a memory block from the far heap. If the block reaches beyond
** the size of the allready allocates space. Only the part that is
** is contained in the allocated memory is returned and a value of 2 is
** returned.
**
** Returns: 0 on error, 1 on success, 2 when the end of the block lays
**          beyond the allready allocated size.
*/

int MainMemRead(unsigned long offset, unsigned bytes, void* toread)
{
    int  result;
    long rest, inseg;
    unsigned long request;
    unsigned bsegment, boffset;

    /* Error if not allready initialized. */
    if (!(FarBlock || Allocated)) return 0;

    /* Be ready to return success. */
    result = 1;

    /* Return failure if the beginning of the block is beyond the
       allready allocated size. */
    if (offset > Allocated) return 0;

    /* If the allready allocated size smaller is than the amount of
       bytes to read from adjust the amount of bytes to read and
       be ready to return 2. */
    request = offset + bytes;
    if (request > Allocated)
    {
       bytes  = (unsigned) (Allocated - offset);
       result = 2;
    }

    /* Read from memory. */
    CalculateAddress(FarBlock, offset, &bsegment, &boffset);

    inseg = (65536L - (long) boffset); /* Bytes left in segment. */
    rest  = (long) bytes - inseg;      /* Bytes to read in following segment. */

    if (rest > 0)
    {
       movedata(bsegment, boffset,
		FP_SEG((void far*) toread),
		FP_OFF((void far*) toread),
		(size_t) inseg);

       movedata(bsegment+4096, 0,
		FP_SEG((void far*) toread),
		FP_OFF((void far*) toread)+(unsigned)inseg,
		(size_t) rest);
    }
    else
       movedata(bsegment, boffset,
		FP_SEG((void far*) toread), FP_OFF((void far*) toread),
		bytes);

    return result;
}

/*
** Writes a memory block to the far heap. If the block reaches beyond
** the size of the allready allocates space. The far block is reallocated.
**
** Returns: 0 on error, 1 on success, 2 when the end of the block lays
**          beyond the allready allocated size.
*/

int MainMemWrite(unsigned long offset, unsigned bytes, void* towrite)
{
    unsigned long request;
    int           result = 1;
    long          rest, inseg;
    unsigned      boffset, bsegment;

    /* Error if not allready initialized. */
    if (!(FarBlock || Allocated)) return 0;

    request = offset + (unsigned long) bytes;

    /* If the block is to small, try to reallocate it. */
    if (request > Allocated)
    {
       FarBlock = (char far*) farrealloc(FarBlock, request);

       if (!FarBlock)
       {
	  Allocated = 0;
	  return 0;
       }

       result = 2;
       Allocated = request;
    }

    /* Write to memory. */
    CalculateAddress(FarBlock, offset, &bsegment, &boffset);

    inseg = (65536L - (long) boffset); /* Space left in segment. */
    rest  = (long) bytes - inseg;      /* Space needed in following segment.*/

    if (rest > 0)
    {
       movedata(FP_SEG((void far*) towrite),
		FP_OFF((void far*) towrite),
		bsegment, boffset, (size_t) inseg);

       movedata(FP_SEG((void far*) towrite),
		FP_OFF((void far*) towrite)+(unsigned) inseg,
		bsegment+4096, 0, (size_t) rest);
     }
     else
	movedata(FP_SEG((void far*) towrite),
		 FP_OFF((void far*) towrite),
		 bsegment, boffset, bytes);

    return result;
}

/*
** Truncates main memory to be a certain size.
**
** Only intended to make allocated memory smaller.
**
** Returns 0 if error, 1 if success.
*/

int MainMemTruncate(unsigned long newsize)
{
    FarBlock = (char far*) farrealloc(FarBlock, newsize);
    
    if (!FarBlock)
    {
       Allocated = 0;
       return 0;
    }

    return 1;
}
