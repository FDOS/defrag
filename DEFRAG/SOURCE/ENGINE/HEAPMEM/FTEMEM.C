#include <stdlib.h>

#include "FTE.h"
#include "FTEMEM.h"
#include "backmem.h"
#include "suremem.h"

ALLOCRES AllocateFTEMemory(unsigned guaranteed, unsigned char guaranteedblocks,
                           unsigned backupbytes)
{
   if (guaranteed && guaranteedblocks)
   {
      if (!AllocateGuaranteedMemory(guaranteed, guaranteedblocks))
      {
         return TOTAL_FAIL;
      }
   }

   if (backupbytes)
   {
      if (!AllocateBackupMemory(backupbytes))
      {
         return BACKUP_FAIL;
      }
   }

   return ALLOC_SUCCESS;
}

void DeallocateFTEMemory(void)
{
   FreeGuaranteedMemory();
   FreeBackupMemory();
}

/* Generic allocation */
void* FTEAlloc(size_t bytes)
{
   void* retval;

   retval = malloc(bytes);
   if (!retval)
   {
      retval = BackupAlloc(bytes);
      if (retval)
      {
         return retval;
      }
      SetFTEerror(FTE_MEM_INSUFFICIENT);
   }

   return retval;
}

void  FTEFree(void* tofree)
{
   if (InBackupMemoryRange(tofree))
   {
      BackupFree(tofree);
   }
   else
   {
      free(tofree);
   }
}

/* Sectors -- Assumes that bytespersector field is filled in at handle
              creation (InitReadWriteSectors,...)                     */
SECTOR* AllocateSector(RDWRHandle handle)
{
   return (SECTOR*) FTEAlloc(handle->BytesPerSector);
}

SECTOR* AllocateSectors(RDWRHandle handle, int count)
{
   return (SECTOR*) FTEAlloc(handle->BytesPerSector * count);
}

void FreeSectors(SECTOR* sector)
{
   FTEFree((void*)sector);
}

/* Boot */
struct BootSectorStruct* AllocateBootSector(void)
{
   return (struct BootSectorStruct*)
          FTEAlloc(sizeof(struct BootSectorStruct));
}

void FreeBootSector(struct BootSectorStruct* boot)
{
   FTEFree((void*) boot);
}

/* Directories */
struct DirectoryEntry* AllocateDirectoryEntry(void)
{
   return (struct DirectoryEntry*) FTEAlloc(sizeof(struct DirectoryEntry));
}

void FreeDirectoryEntry(struct DirectoryEntry* entry)
{
   FTEFree(entry);
}

struct FSInfoStruct* AllocateFSInfo(void)
{
   return (struct FSInfoStruct*) FTEAlloc(sizeof(struct FSInfoStruct));
}

void FreeFSInfo(struct FSInfoStruct* info)
{
   FTEFree(info);
}
