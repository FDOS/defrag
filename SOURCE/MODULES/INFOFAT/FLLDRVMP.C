/*    
   Flldrvmp.c - fill drive map.

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

#include <stdlib.h>

#include "fte.h"
#include "infofat.h"
#include "expected.h"

static int ReportDriveSize(RDWRHandle handle);

static int Reporter(RDWRHandle handle, CLUSTER label, SECTOR sector,
                    void** structure);

/************************************************************
***                        FillDriveMap
*************************************************************
*** Fills the drive map
*************************************************************/
                   
int FillDriveMap(RDWRHandle handle)
{
    BOOL error=FALSE, *pError = &error;
    
    SmallMessage("Calculating drive size.");
    if (ReportDriveSize(handle))
    {
       SmallMessage("Drawing drive map.");
       return LinearTraverseFat(handle, Reporter, (void**)&pError);
    }
    else
       return FALSE;
}

/************************************************************
***                        Reporter
*************************************************************
*** Looks at every label in the FAT and reports accordingly
*************************************************************/

static int Reporter(RDWRHandle handle, CLUSTER label, SECTOR sector,
                    void** structure)
{
    int     symbol;
    CLUSTER cluster;

    cluster = DataSectorToCluster(handle, sector);
    if (!cluster) 
    {
       **((BOOL**) structure) = TRUE;
       return FALSE;
    }

    if      (FAT_FREE(label))   symbol = UNUSEDSYMBOL;
    else if (FAT_BAD(label))    symbol = BADSYMBOL;
    else if (FAT_LAST(label))   symbol = USEDSYMBOL;
    else if (FAT_NORMAL(label)) symbol = USEDSYMBOL;
    else
    {
       **((BOOL**) structure) = TRUE;
       return FALSE;    
    }

    DrawOnDriveMap(cluster, symbol);
    return TRUE;
}

/************************************************************
***                        ReportDriveSize
*************************************************************
*** Reports the size of the volume.
*************************************************************/

static int ReportDriveSize(RDWRHandle handle)
{
   unsigned long clusters;

   clusters = GetClustersInDataArea(handle);

   if (clusters == 0) return FALSE;

   DrawDriveMap(clusters);

   return TRUE;
}
