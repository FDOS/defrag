/*
   Swapfile.c - swap file management.
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

#include <dos.h>
#include <ctype.h>
#include <io.h>
#include <dir.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "swapfile.h"

static int  FileHandle;
static char TempFile[128];
static int  TempDisk;

static unsigned long FileSize = 0;

/*
** Create and open the swap file.
**
** First try the directory pointed to by the temp (or TEMP) environment 
** variable. If that doesn't work try c:\.
** 
** Returns: 1 on success, 0 on failure.
**
** Remark: this routine must be called before any other routine.
*/

int DiskInit()
{
     char* tempvar;
     int   curdisk;

     tempvar = getenv("temp");
     if (!tempvar) tempvar = getenv("TEMP");
     if (tempvar)
	strcpy(TempFile, tempvar);
     else
	strcpy(TempFile, "C:\\");

     curdisk  = getdisk();
     TempDisk = toupper(TempFile[0]) - 65;

     for (;;)
     {
	 setdisk(TempDisk);
	 if (getdisk() == TempDisk)
	 {
	    setdisk(curdisk);
	     
	    FileHandle = creattemp(TempFile, 0);
	    if (FileHandle == -1) return 0;
	    return 1;                        /* Success.             */
	 }
	 else if (TempDisk != 2)             /* If not C drive.      */
	 {
	    strcpy(TempFile, "C:\\");
	    TempDisk = 2;                    /* Try C drive instead. */
	 }
	 else
	    break;
     }

     return 0;
}

/*
**   Closes the swap file and deletes it.
**
**   Remark: only use when the swap file was successfully created.
*/

void DiskClose()
{
     if (close(FileHandle) == 0) remove(TempFile);
}

/*
**   Returns the remaining free disksize.
*/
unsigned long DiskCoreLeft()
{
     struct dfree free;

     getdfree(TempDisk+1, &free);

     return (long) free.df_avail * (long) free.df_bsec *
	    (long) free.df_sclus;
}

/*
**   Writes a memory block to the swap file. If the block reaches beyond
**   the size of the swap file, the swap file is resized.
**
**   Returns: 0 on failure, 1 on success, 2 if not everything could be
**            written.
*/

int DiskWrite(unsigned long offset, unsigned bytes, void* towrite)
{
    unsigned long request;
    int           written;

    request = offset + (unsigned long) bytes;

    if ((request < FileSize) || ((request - FileSize) <= DiskCoreLeft()))
    {
       if (lseek(FileHandle, offset, SEEK_SET) != -1L)
       {
	  written = write(FileHandle, towrite, bytes);

	  if (written != -1)
	  {
	     request = offset + written;

	     if (request > FileSize) FileSize = request;
	     return -(written == bytes) + 2; /* return 1 if success, 2 if
						not everything could be written.
					      */
	  }
       }
    }

    return 0;                       /* Return 0 on write error. */
}

/*
**   Reads a memory block from the swap file. If the block reaches beyond
**   the size of the swap file only the part that is contained in the
**   swap file is returned and a value of 2 is returned.
**
**   Returns: 0 on failure, 1 on success, 2 when the end of the block lays
**            beyond the swap file size.
*/

int DiskRead(unsigned long offset, unsigned int bytes, void* toread)
{
    int result;
    unsigned long request;

    /* Assume success. */
    result = 1;

    /* Return failure if the beginning of the block is beyond the swap
       file size. */
    if (offset > FileSize) return 0;

    /* If the size of the swap file smaller is than the amount of
       bytes to read from adjust the size of the bytes to read and be
       ready to return a value of 2. */
    request = offset + bytes;
    if (request > FileSize)
    {
       bytes  = (unsigned) (FileSize - offset);
       result = 2;
    }

    /* Try to read from the swap file. */
    if ((lseek(FileHandle, offset, SEEK_SET) != -1L) &&
	(read(FileHandle, toread, bytes) == bytes))
       return result; /* Return 1 on success, 2 if beyond swap file size. */
    else
       return 0;      /* Return 0 on failure (disk read error, etc). */
}

/*
**  Truncates the swap file to a certain size.
**
**  Returns 1 on success, 0 on failure.
*/

int TruncateSwapFile(unsigned long offset)
{
    if (lseek(FileHandle, offset, SEEK_SET) != -1)
       return _write(FileHandle, (void*) &offset, 0) + 1;
    else                                 /* ^ = place filler. */
       return 0;
}
