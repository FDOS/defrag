/*    
   rdwrsect.c - Abstractions for a common interface between absolute disk
                access and image files.
   
   Copyright (C) 2000, 2002 Imre Leber

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

#include <io.h>
#include <dos.h>
#include <ctype.h>
#include <fcntl.h>
#include <alloc.h>
#include <string.h>

#include "..\header\rdwrsect.h"
#include "..\..\misc\bool.h"
#include "..\header\drive.h"
#include "..\header\FTEerr.h"
#include "..\disklib\disklib.h"
#include "..\disklib\dosio.h"
#include "..\header\fatconst.h"
#include "..\header\lowfat32.h"
#include "..\header\direct.h"
#include "..\header\boot.h"
#include "..\header\direct.h"
#include "..\header\fsinfo.h"
#include "..\header\FTEMem.h"

#ifdef USE_SECTOR_CACHE
#include "..\header\sctcache.h"
static int UniqueCounter=0;
#endif

#define UNUSED(x) x=x

/* Image file modifiers. */
static int ReadFromImageFile(int handle, int nsects, SECTOR lsect,
                             void* buffer)
{
  if (lseek(handle, lsect*BYTESPERSECTOR, SEEK_SET) == -1L)
  {
     SetFTEerror(FTE_READ_ERR);
     return -1;
  }

  if ((read(handle, buffer, nsects*BYTESPERSECTOR)) == -1)
  {
     SetFTEerror(FTE_READ_ERR);
     return -1;
  }

  return 0;
}

static int WriteToImageFile(int handle, int nsects, SECTOR lsect,
                            void* buffer, unsigned area)
{
  UNUSED(area);

  if (lseek(handle, lsect*BYTESPERSECTOR, SEEK_SET) == -1L)
  {
     SetFTEerror(FTE_WRITE_ERR);
     return -1;
  }

  if (write(handle, buffer, nsects*BYTESPERSECTOR) == -1)
  {
     SetFTEerror(FTE_WRITE_ERR);
     return -1;
  }

  return 0;
}

/* Real disk sector modifiers. */
/*
   There are different read/write functions:
   - ReadFromRealDisk12, WriteToRealDisk12 are used for FAT12 and FAT16
     volumes <= 32 MB
   - ReadFromRealDisk16, WriteToRealDisk16 are used for FAT16
     volumes > 32 MB
   - ReadFromRealDisk32, WriteToRealDisk32 are used for FAT32 volumes

   Notice also that, despite the names absread/abswrite work independent
   of the file system used (i.e. the naming here is a litle unfortunate)
*/

#ifdef ALLOW_REALDISK

static int ReadFromRealDisk12(int handle, int nsects, SECTOR lsect,
                              void* buffer)
{
  if (absread(handle, nsects, (int) lsect, buffer) == -1)
  {
     SetFTEerror(FTE_READ_ERR);
     return -1;
  }

  return 0;
}

static int WriteToRealDisk12(int handle, int nsects, SECTOR lsect,
                             void* buffer, unsigned area)
{
  UNUSED(area);
  
  if (abswrite(handle, nsects, (int) lsect, buffer) == -1)
  {
     SetFTEerror(FTE_WRITE_ERR);
     return -1;
  }

  return 0;
}

static int ReadFromRealDisk16(int handle, int nsects, SECTOR lsect,
                              void* buffer)
{
   if (disk_read_ext(handle, lsect, buffer, nsects) != DISK_OK)
   {
     SetFTEerror(FTE_READ_ERR);
     return -1;
   }
   return 0;
}

static int WriteToRealDisk16(int handle, int nsects, SECTOR lsect,
                             void* buffer, unsigned area)
{
   UNUSED(area);
   
   if (disk_read_ext(handle, lsect, buffer, nsects) != DISK_OK)
   {
     SetFTEerror(FTE_WRITE_ERR);
     return -1;
   }
   return 0;
}

static int ReadFromRealDisk32(int handle, int nsects, SECTOR lsect,
                              void* buffer)
{
   /* Don't use DISKLIB here (it uses ioctl for FAT12/16 which according to
      microsoft docs does not work!) */

   return ExtendedAbsReadWrite(handle+1, nsects, lsect, buffer, 0);
}

static int WriteToRealDisk32(int handle, int nsects, SECTOR lsect,
                             void* buffer, unsigned area)
{
   /* Don't use DISKLIB here (it uses ioctl for FAT12/16 which according to
      microsoft docs does not work!) */

   return ExtendedAbsReadWrite(handle+1, nsects, lsect, buffer, area);
}

#endif

/*
** Public read sector function.
*/
#ifdef USE_SECTOR_CACHE

int ReadSectors(RDWRHandle handle, int nsects, SECTOR lsect, void* buffer)
{
    SECTOR sector, j;
    SECTOR beginsector=0;
    BOOL LastReadFromDisk = FALSE;
    char* cbuffer = (char*) buffer, *cbuffer1;
    char* secbuf;
    int i;

    secbuf = (char*) AllocateSector(handle);
    if (!secbuf) /* Not enough memory to use the cache */
    {            /* Read directly from disk            */
       return handle->ReadFunc(handle->handle, (int)nsects, lsect, buffer);                               
    }
    
    for (i=0, sector = lsect; i < nsects; sector++, i++)
    {
        if (RetreiveCachedSector(handle->UniqueNumber,
                                 sector, 
                                 secbuf))
        {   
            if (LastReadFromDisk)
            {
               if (handle->ReadFunc(handle->handle, (int)(sector-beginsector),
                                    beginsector, cbuffer) == -1)
               {
                  FreeSectors((SECTOR*) secbuf);
                  return -1;
	       }

	       /* Put the read sectors in the cache. */
	       cbuffer1 = cbuffer;
	       for (j = beginsector; j < sector; j++)
	       {
		   CacheSector(handle->UniqueNumber, j, cbuffer1);
		   cbuffer1 += BYTESPERSECTOR;
	       }

               cbuffer += BYTESPERSECTOR * (int)(sector-beginsector);
               LastReadFromDisk = FALSE;
           }
           memcpy(cbuffer, secbuf, BYTESPERSECTOR);
           cbuffer += BYTESPERSECTOR;
        }
        else
        {
           if (!LastReadFromDisk)
           {
              LastReadFromDisk = TRUE;
              beginsector = sector;     
           }
        }
    } 

    if (LastReadFromDisk)
    {
       if (handle->ReadFunc(handle->handle, (int)((lsect+nsects)-beginsector),
                            beginsector, cbuffer) == -1)
       {
          FreeSectors((SECTOR*) secbuf);
          return -1;
       }

       /* Put the read sectors in the cache. */
       for (j = beginsector; j < (lsect+nsects); j++)
       {
	 CacheSector(handle->UniqueNumber, j, cbuffer);
	 cbuffer += BYTESPERSECTOR;
       }
    }
    
    FreeSectors((SECTOR*) secbuf);
    return 0;
}
#else
int ReadSectors(RDWRHandle handle, int nsects, SECTOR lsect, void* buffer)
{   
    return handle->ReadFunc(handle->handle, nsects, lsect, buffer);
}
#endif

/*
** Public write sector function.
*/
#ifdef USE_SECTOR_CACHE
int WriteSectors(RDWRHandle handle, int nsects, SECTOR lsect, void* buffer,
                 unsigned area)
{
    int i;
    SECTOR sector;
    char* cbuffer = (char*) buffer;

    if (handle->ReadWriteMode == READING)
    { 
        SetFTEerror(FTE_WRITE_ON_READONLY);
        return 0; 
    }   
    
    if (handle->WriteFunc(handle->handle, nsects, lsect, buffer, area) == -1)
    { 
       for (i=0, sector = lsect; i < nsects; sector++, i++)
           UncacheSector(handle->UniqueNumber, sector);
       return -1; 
    }
  
    for (i=0, sector = lsect; i < nsects; sector++, i++)
    {
        CacheSector(handle->UniqueNumber, sector, cbuffer);
        cbuffer += BYTESPERSECTOR;
    }
    return 0;    
}
#else
int WriteSectors(RDWRHandle handle, int nsects, SECTOR lsect, void* buffer,
                 unsigned area)
{
    if (handle->ReadWriteMode == READINGANDWRITING)
        return handle->WriteFunc(handle->handle, nsects, lsect, buffer, area);
    else
    {
        SetFTEerror(FTE_MEM_INSUFFICIENT); 
        return 0;
    }
}
#endif

/*
** Private function to initialise the reading and writing of sectors.
*/
static int PrivateInitReadWriteSectors(char* driveorfile, int modus,
                                       RDWRHandle* handle)
{
    unsigned sectorsize;

    *handle = (RDWRHandle) malloc(sizeof(struct RDWRHandleStruct));
    if (*handle == NULL) return FALSE;
    memset(*handle, 0, sizeof(struct RDWRHandleStruct));

#ifdef ALLOW_REALDISK
    if (HasAllFloppyForm(driveorfile))
    {
       (*handle)->handle = toupper(driveorfile[0]) - 'A';

       sectorsize = GetFAT32SectorSize((*handle)->handle);
       if (sectorsize)
       {
          (*handle)->ReadFunc  = ReadFromRealDisk32;
	  (*handle)->WriteFunc = WriteToRealDisk32;
	  (*handle)->BytesPerSector = sectorsize;
       }
       else
       {
	  struct DEVICEPARAMS devicepars;
	  unsigned long disksize;

	  /* Get bytes per sector */
	  if (disk_getparams((*handle)->handle, &devicepars) != DISK_OK)
	  {
	     free(*handle);
	     *handle = NULL;
	     return FALSE;
	  }
	  (*handle)->BytesPerSector = devicepars.sec_size;

	  disksize = devicepars.secs_cluster * devicepars.total_sectors;

	  if (disksize < HD32MB)
	  {
	     (*handle)->ReadFunc  = ReadFromRealDisk12;
	     (*handle)->WriteFunc = WriteToRealDisk12;
	  }
	  else
	  {
	     (*handle)->ReadFunc  = ReadFromRealDisk16;
	     (*handle)->WriteFunc = WriteToRealDisk16;
	  }
       }
    }
    else

#endif
    {
       (*handle)->ReadFunc    = ReadFromImageFile;
       (*handle)->WriteFunc   = WriteToImageFile;
       (*handle)->handle      = open (driveorfile, modus|O_BINARY);
       (*handle)->IsImageFile = TRUE;

       if ((*handle)->handle == -1)
       {
          free(*handle);
          *handle = NULL;
          return FALSE;
       }

       /* Get number of bytes per sector from the boot sector, this
	  field is located at:
	  struct BootSectorStruct
	  {
	    char     Jump[3];
	    char     Identification[8];
	    unsigned short BytesPerSector;
	    ...
	  }
       */
       if (lseek((*handle)->handle, 11, SEEK_SET) == -1L)
       {
	  free(*handle);
	  *handle = NULL;
	  return FALSE;
       }

       if (read((*handle)->handle, &((*handle)->BytesPerSector),
               sizeof(unsigned short)) == -1)
       {
	  free(*handle);
	  *handle = NULL;
	  return FALSE;
       }

       /* No need to reset to the beginning of the file. */
    }

#ifdef USE_SECTOR_CACHE
    (*handle)->UniqueNumber = UniqueCounter++;    
#endif    

    if ((*handle)->BytesPerSector != 512)
    {
       SetFTEerror(FTE_INVALID_BYTESPERSECTOR);
       return FALSE;
    }

    return TRUE;
}

/*
**  Init for reading and writing of sectors.
*/
int InitReadWriteSectors(char* driveorfile, RDWRHandle* handle)
{
    int result;        
        
    result = PrivateInitReadWriteSectors(driveorfile, O_RDWR, handle);
    if (result)
    {
       (*handle)->ReadWriteMode = READINGANDWRITING;
    }
    
    return result;
}

/*
**  Init for reading only.
*/
int InitReadSectors(char* driveorfile, RDWRHandle* handle)
{
    int result;
    
    result = PrivateInitReadWriteSectors(driveorfile, O_RDONLY, handle);
    if (result)
    {
       (*handle)->ReadWriteMode = READING;
    }
    
    return result;
}

/*
**  Init for writing only (reasonably stupid).
*
int InitWriteSectors(char* driveorfile, RDWRHandle* handle)
{
    return PrivateInitReadWriteSectors(driveorfile, O_WRONLY, handle);
}
*/

/*
**  Close the sector read/write system.
*/
void CloseReadWriteSectors(RDWRHandle* handle)
{
    if (*handle != NULL)
    {
       if ((*handle)->IsImageFile) close((*handle)->handle);
       free(*handle);
       *handle = NULL;
    }
}

/*
** Return the read/write modus 
*/
int GetReadWriteModus(RDWRHandle handle)
{
    return handle->ReadWriteMode;    
}
