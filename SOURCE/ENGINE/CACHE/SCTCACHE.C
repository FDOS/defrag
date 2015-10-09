/*    
   Sctcache.c - sector cache interface routines.

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

#define XMS 0
#define EMS 1

#include "..\header\fat.h"
#include "..\header\sctcache.h"
#include "..\..\misc\bool.h"
#include "xmscache.h"
#include "emscache.h"

static int CacheLocked = FALSE;
static int CacheStarted = FALSE;
static int CacheInitialised = FALSE;

static struct LowCacheFunctions lowfuncs[2];

void InitSectorCache(void)
{
   if (!CacheInitialised)
   {
      InitXMSCache(&lowfuncs[0]);      
      InitEMSCache(&lowfuncs[1]);
         
      CacheInitialised = TRUE;
   }
}

void CloseSectorCache(void)
{
   if (CacheInitialised)
   {
      if (lowfuncs[XMS].supported) CloseXMSCache();
      if (lowfuncs[EMS].supported) CloseEMSCache();
   }
}

void LockSectorCache()
{
  CacheLocked = TRUE;   
}

void UnlockSectorCache()
{
  CacheLocked = FALSE;      
}

void StartSectorCache()
{
  CacheStarted = TRUE;  
}

void StopSectorCache()
{
  CacheStarted = FALSE;      
}

void CacheSector(unsigned devid, SECTOR sector, char* buffer)
{
   if (CacheLocked || !CacheStarted) return;

   InitSectorCache();

   if (lowfuncs[XMS].supported)
   {
      if (!lowfuncs[XMS].IsFull()) 
      {
         switch (lowfuncs[XMS].Cache(devid, sector, buffer))
         {
            case -2:
                    lowfuncs[XMS].supported = FALSE;
                    break;
            case TRUE:        
                    return;
         }
      }        
   }

   if (lowfuncs[EMS].supported) 
   {
      if (!lowfuncs[EMS].IsFull())
      {
         switch (lowfuncs[EMS].Cache(devid, sector, buffer))
         {
            case -2:
                   lowfuncs[EMS].supported = FALSE;
                   break;
            case TRUE:
                   return;
         }
      }
   }        

   if (lowfuncs[XMS].supported)
   {
      switch (lowfuncs[XMS].Cache(devid, sector, buffer))
      {
            case -2:
                   lowfuncs[XMS].supported = FALSE;  
                   break;
            case TRUE:
                   return;
      }
   }      

   if (lowfuncs[EMS].supported)
   {
      if (lowfuncs[EMS].Cache(devid, sector, buffer) == -2)
         lowfuncs[EMS].supported = FALSE;     
   }
}

int RetreiveCachedSector(unsigned devid, SECTOR sector, char* buffer)
{
   if (CacheLocked || !CacheStarted) return FALSE;
   InitSectorCache();

   if ((lowfuncs[XMS].supported) && 
       (lowfuncs[XMS].Retreive(devid, sector, buffer)))
   {
      return TRUE;
   }
      
   if ((lowfuncs[EMS].supported) && 
       (lowfuncs[EMS].Retreive(devid, sector, buffer)))
   {
      return TRUE;
   }
      
   return FALSE;
}

void UncacheSector(unsigned devid, SECTOR sector)
{
   if (CacheLocked || !CacheStarted) return;
   
   InitSectorCache();

   if (lowfuncs[XMS].supported) 
      if (!lowfuncs[EMS].UnCache(devid, sector))
         lowfuncs[XMS].supported = FALSE;  
         
   if (lowfuncs[EMS].supported)
      if (!lowfuncs[EMS].UnCache(devid, sector))
         lowfuncs[EMS].supported = FALSE;  
}

void InvalidateCache()
{
    int XMSinvalidated, EMSinvalidated;
       
    if (CacheLocked || !CacheStarted) return;
    InitSectorCache();

    XMSinvalidated = lowfuncs[XMS].Invalidate();
    EMSinvalidated = lowfuncs[EMS].Invalidate();  
        
    if (!XMSinvalidated || !EMSinvalidated)
    {
       /* If the cache could not be invalidated, we lock out completely */ 
       CacheLocked = TRUE;
    }
}
