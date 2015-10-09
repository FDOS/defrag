#ifndef SECTORCACHE_H_
#define SECTORCACHE_H_

struct LowCacheFunctions
{
  int (*IsFull)    (void);
  int (*Cache)     (unsigned devid, SECTOR sector, char* buffer);
  int (*Retreive)  (unsigned devid, SECTOR sector, char* buffer);
  int (*UnCache)   (unsigned devid, SECTOR sector);
     
  int (*Invalidate)(void);
  
  int supported;
};

/* Init and destroy */
void InitSectorCache(void);
void CloseSectorCache(void);

/* User lock of cache */
void LockSectorCache(void);
void UnlockSectorCache(void);

/* Algorithm defined lockout */
void StartSectorCache(void);
void StopSectorCache(void);

/* Trash all cache information */
void InvalidateCache(void);

void CacheSector(unsigned devid, SECTOR sector, char* buffer);
int  RetreiveCachedSector(unsigned devid, SECTOR sector, char* buffer);
void UncacheSector(unsigned devid, SECTOR sector);


#endif
