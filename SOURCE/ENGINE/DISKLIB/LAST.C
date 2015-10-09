/*
 * last.c       display FAT info and last allocated cluster
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <dos.h>

#include "dosio.h"
#include "disklib.h"
#include "fat.h"

unsigned short arraymax(unsigned short LARGE *array, unsigned int max);

int main(int argc, char **argv)
{
int i,disk;
struct DEVICEPARAMS dp;
int secsfat;
long size,fatsector;
unsigned int nclusters;
unsigned short maxcluster;
unsigned short LARGE *clusters;

    if (argc == 2)
        disk = atoi(argv[1]);
    else
        disk = get_drive();

    printf("\nDrive %c:\n\n",disk+'A');

    if (disk_gettype(disk) != FAT1216) {
        printf("Not FAT12/16\n");
        return 1;
    }

    if ((i = disk_getparams(disk,&dp)) != DISK_OK) {
        lib_error("last",i);
        return 1;
    }

    nclusters = num_clusters(&dp);
    fatsector = dp.reserved_secs;
    secsfat = dp.secs_fat;
    drive_size(disk,&size);

    printf("drive size              %ldMB\n",size/(1024L*1024L));
    printf("number of FATs          %d\n",dp.num_fats);
    printf("sectors per FAT         %d\n",secsfat);
    printf("number of clusters      %u\n",nclusters);

    clusters = readfat(disk,(unsigned short)nclusters,fatsector,secsfat);

    if (clusters == NULL) {
        printf("bummer dude... out a memory\n");
        return 1;
    }

    maxcluster = arraymax(clusters,nclusters);
    printf("last allocated cluster  %u\n",maxcluster);

    return 0;
}


unsigned short arraymax(unsigned short LARGE *array, unsigned int max)
{
unsigned int i;
unsigned short n;

    n = 0;
    for (i = 0; i < max; i++)
    {
        if (array[i])
            n = (unsigned short)i;
    }
    return n;
}
