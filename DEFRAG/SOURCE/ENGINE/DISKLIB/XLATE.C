/*
 * xlate.c      logical drive translations (FAT12 and FAT16)
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 09-Dec-1998  greggj  physical_p(); long casts in logical_sector()
 * 27-Nov-1998  greggj  `hidden' was int in logical_sector(); include disklib.h
 * 26-Nov-1998  greggj  disk_size32()
 * 12-Nov-1998  greggj  physical()
 *
 */

#include "dosio.h"
#include "disklib.h"

/*
 *  disk_size       calculate disk size from DEVICEPARAMS
 *
 */

long disk_size(struct DEVICEPARAMS *dp)
{
long drivesize;

    drivesize = dp->num_sectors;
    if (drivesize == 0L)
        drivesize = dp->total_sectors;
    drivesize *= dp->sec_size;

    return drivesize;
}

#ifdef _WIN32

/*
 *  disk_size32     calculate disk size from DEVICEPARAMS for larger drives
 *
 *  See GET.C for an example of usage.
 */

__int64 disk_size32(struct DEVICEPARAMS *dp)
{
__int64 drivesize;

    drivesize = dp->num_sectors;
    if (drivesize == 0I64)
        drivesize = dp->total_sectors;
    drivesize *= dp->sec_size;

    return drivesize;
}

#endif

/*
 *  dir_sectors     get number of sectors that make up the ROOT directory
 *
 */

int dir_sectors(struct DEVICEPARAMS *dp)
{
    return dp->dir_entries / (dp->sec_size/DIR_ENTRY_SIZE);
}

/*
 *  data_sector     get sector of the start of the data area
 *
 */

int data_sector(struct DEVICEPARAMS *dp)
{
    return (dp->secs_fat*dp->num_fats) + dir_sectors(dp) + dp->reserved_secs-1;
}

/*
 *  dir_sector      get sector of the start of the ROOT directory
 *
 */

int dir_sector(struct DEVICEPARAMS *dp)
{
    return (dp->secs_fat*dp->num_fats) + dp->reserved_secs;
}

/*
 *  num_clusters    calculate the number of clusters from DEVICEPARAMS
 *
 */

unsigned int num_clusters(struct DEVICEPARAMS *dp)
{
/* use some temps to aid in debugging */
int dirsecs;
int fatsecs;
long sectors;
unsigned int nclusters;

    dirsecs = dir_sectors(dp);
    fatsecs = dp->secs_fat * dp->num_fats;

    sectors = (dp->num_sectors) ? dp->num_sectors : dp->total_sectors;
    sectors = sectors - fatsecs - dirsecs - 1;
    nclusters = (unsigned int)(sectors / (long)dp->secs_cluster);

    return nclusters + 1;
}

/*
 *  max_track       calculate the maximum track number from DEVICEPARAMS
 *
 */

int max_track(struct DEVICEPARAMS *dp)
{
long sectors;

    sectors = (dp->num_sectors) ? dp->num_sectors : dp->total_sectors;
    sectors += dp->hidden_sectors;
    sectors /= dp->num_heads;
    sectors /= dp->secs_track;
    return (int)sectors - 1;
}

/*
 *  logical_sector  convert track, sector, head to abslolute sector
 *
 *  Note: The track, sector, head values are within a logical partition.
 */

long logical_sector(int t, int s, int h, long hidden, int maxsector, int maxhead)
{
long sector;

    sector =  (long)h * maxsector;
    sector += (long)t * maxhead * maxsector;
    sector += (long)s - 1;
    sector -= hidden;

    return sector;
}

/*
 *  physical        set physical parameters
 *
 *  Note: This converts an absolute sector to its physical parameters
 *        _within_ a logical drive partition.
 *
 *        To keep function complexity down, to convert an absolute
 *        sector to track, sector, head notation, the 3 functions
 *        must be called.
 */

void physical(int *t, int *s, int *h, long sector, long hidden, int maxsector, int maxhead)
{
    if (t) *t = physical_track(sector,hidden,maxsector,maxhead);
    if (s) *s = physical_sector(sector,hidden,maxsector);
    if (h) *h = physical_head(sector,hidden,maxsector,maxhead);
}

int physical_track(long sector, long hidden, int maxsector, int maxhead)
{
int track;

    track = (int) ( (sector+hidden) / (maxsector * maxhead) );

    return track;
}

int physical_sector(long sector, long hidden, int maxsector)
{
int sec;

    sec = (int) ( (sector+hidden) % maxsector ) + 1;

    return sec;
}

int physical_head(long sector, long hidden, int maxsector, int maxhead)
{
int head;

    head = (int) ( (sector+hidden) % (maxsector * maxhead) );

    return head / maxsector;
}

/*
 *  physical_p      set physical parameters
 *
 *  Note: This converts an absolute sector to its physical parameters
 *        for a _phisical_ drive, _not_ a partition. (The formula
 *        just lacks the `hidden' value.)
 */

void physical_p(int *t, int *s, int *h, long sec, int maxs, int maxh)
{
    *t  = (unsigned)(sec / ((long)maxs * maxh));
    *s  = (unsigned)(sec % (maxs)+1);
    *h  = (unsigned)(sec % ((long)maxs * maxh));
    *h /= (unsigned)maxs;
}

/*
 *  cluster_sector  convert logical sector number to the sector within cluster
 *
 */

extern int cluster_sector(long sector, int datasec, int secscluster)
{
int sec;

    if (sector <= datasec)
        return 0;

    sec = (int) ( (sector - (datasec+1)) % secscluster);

    return sec;
}

/*
 *  cluster_to_sector   convert cluster number to logical sector number
 *
 */

extern long cluster_to_sector(long cluster, int datasec, int secscluster)
{
long sector;

    sector = ( ((cluster-2) * secscluster) + (datasec+1) );

    return sector;
}

/*
 *  sector_to_cluster   convert the logical sector number to the cluster number
 *
 */

extern int sector_to_cluster(long sector, int datasec, int secscluster)
{
int cluster;

    if (sector <= datasec)
        return 0;

    cluster = (int) ( (sector - (datasec+1)) / secscluster ) + 2;

    return cluster;
}
