#include <string.h>

#include "bool.h"
#include "rdwrsect.h"
#include "boot.h"
#include "fat.h"
#include "fatconst.h"
#include "direct.h"
#include "FSInfo.h"
#include "ftemem.h"
#include "backup.h"



/************************************************************
**                        BackupBoot                       **
*************************************************************
** On FAT32 this makes a copy of the boot at the location  **
** specified by the BPB.                                   **
**                                                         **
** On FAT12/16 this function does nothing, but may be      **
** called.                                                 **
*************************************************************/

BOOL BackupBoot(RDWRHandle handle)
{
   int fatlabelsize = GetFatLabelSize(handle);
   SECTOR BackupSector;
   struct BootSectorStruct* boot;
 
   if (fatlabelsize == FAT32)
   {
      boot = AllocateBootSector();
      if (!boot) return FALSE;
   
      if (!ReadBootSector(handle, boot) == -1)
      {
         FreeBootSector(boot);
         return FALSE;
      }
      
      BackupSector = GetFAT32BackupBootSector(handle);
      if (!BackupSector)
      {
         FreeBootSector(boot);
         return FALSE;
      }
               
      if (WriteSectors(handle, 1, BackupSector, (void*) boot, WR_UNKNOWN)
                                                                     == -1) 
      {
          FreeBootSector(boot);
          return FALSE;
      }    
   }     
   
   return TRUE;
}

/************************************************************
**                        BackupFat                       **
*************************************************************
** This function makes all FATs equal to the first.        **
*************************************************************/

BOOL BackupFat(RDWRHandle handle)
{
    BOOL result = TRUE;
    unsigned i, NumberOfFats;
    unsigned long SectorsPerFat, bytesinlastsector;
    SECTOR sector, fatstart;
    char* sectbuf, *sectbuf1;
    unsigned long bytesinfat;

    bytesinfat = GetBytesInFat(handle);
    if (!bytesinfat) return FALSE;

    fatstart = GetFatStart(handle);
    if (!fatstart) return FALSE;
    
    NumberOfFats = GetNumberOfFats(handle);
    if (!NumberOfFats) return FALSE;
    
    SectorsPerFat = GetSectorsPerFat(handle);
    if (!SectorsPerFat) return FALSE;

    sectbuf  = (char*) FTEAlloc(BYTESPERSECTOR);
    if (!sectbuf) return FALSE;

    sectbuf1 = (char*) FTEAlloc(BYTESPERSECTOR);
    if (!sectbuf1)
    {
       FTEFree(sectbuf);
       return FALSE;
    }
        
    for (i = 1; i < NumberOfFats; i++)
    {
        for (sector = fatstart; sector < fatstart+SectorsPerFat-1; sector++)
        {
            if (ReadSectors(handle, 1, sector, sectbuf) != -1)
            {
               if (WriteSectors(handle, 1, sector + (i * SectorsPerFat),
                                sectbuf, WR_FAT) == -1)
               {
                  FTEFree(sectbuf);
                  FTEFree(sectbuf1);
                  return FALSE;
               }
            }
            else
            {
               FTEFree(sectbuf);
               FTEFree(sectbuf1);
               return FALSE;
            }
        }
        
        if (ReadSectors(handle, 1, fatstart+SectorsPerFat, sectbuf) != -1)
        {
           if (ReadSectors(handle, 1, (i * SectorsPerFat)+fatstart+SectorsPerFat,
                           sectbuf1) == -1)
           {
               FTEFree(sectbuf);
               FTEFree(sectbuf1);
               return FALSE;
           }
                       
           bytesinlastsector = bytesinfat % BYTESPERSECTOR;
           
           memcpy(sectbuf1, sectbuf, (size_t) bytesinlastsector);
           
           if (WriteSectors(handle, 1, (i * SectorsPerFat)+fatstart+SectorsPerFat,
                            sectbuf1, WR_FAT) == -1)
           {
              FTEFree(sectbuf);
              FTEFree(sectbuf1);
              return FALSE;
           }
        }
        else
        {
           FTEFree(sectbuf);
           FTEFree(sectbuf1);
           return FALSE;
        }
    }

    FTEFree(sectbuf);
    FTEFree(sectbuf1);
    return result;
}
