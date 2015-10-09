/*
 * win32\free.c     get disk free space
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <direct.h>     /* _getdrive */

#include "win32.h"
#include "dosio.h"
#include "debug.h"

/*
 * disk_free_space      get disk free space, unit and sector sizes
 *
 */

extern int disk_free_space(int disk, struct FREESPACE *fs)
{
struct _diskfree_t df;

    if (_getdiskfree(disk,&df) != 0)
        return DOS_ERR;

    fs->secs_cluster =   (unsigned short)df.sectors_per_cluster;
    fs->avail_clusters = (unsigned short)df.avail_clusters;
    fs->sec_size =       (unsigned short)df.bytes_per_sector;
    fs->num_clusters =   (unsigned short)df.total_clusters;

    return DISK_OK;
}

/*
 * drive_size       get drive size in bytes
 *
 */

extern int drive_size(int disk, long *size)
{
int i;
struct DEVICEPARAMS dp DBG_0;

    *size = 0L;

    if ((i = disk_getparams(disk,&dp)) != DISK_OK)
        return i;

    *size = dp.num_sectors;             /* (temp at first) */
    if (*size == 0L)                    /* (need to do some) */
        *size = dp.total_sectors;       /* (multiplying) */
    *size *= dp.sec_size;

    return i;
}

/*
 * get_drive        get drive number (A: = 0, B: = 1, ... )
 *
 */

extern int get_drive(void)
{
int disk;

    disk = _getdrive();
    return disk - 1;        /* A: = 0, B: = 1, ... */
}

#if 0
/* here's an other way that looks too complicated */

int disk_free_space(int disk, struct FREESPACE *fs)
{
char root[4];
DWORD secsclus,secsize,availclus,numclus;

    root[0] = (char)(disk+'@');
    root[1] = ':';
    root[2] = '\\';
    root[3] = '\0';

    if (GetDiskFreeSpace(root,&secsclus,&secsize,&availclus,&numclus) == FALSE) {
        DBG_err_dump("free");
        return DOS_ERR;
    }

    fs->secs_cluster = (unsigned short)secsclus;
    fs->avail_clusters = (unsigned short)availclus;
    fs->sec_size = (unsigned short)secsize;
    fs->num_clusters = (unsigned short)numclus;

    return DISK_OK;
}
#endif
