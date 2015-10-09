/*
 * get.c        get device parameters
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 26-Nov-1998  greggj  disk_size32()
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "dosio.h"
#include "disklib.h"

void display(struct DEVICEPARAMS *dp);

int main(int argc, char **argv)
{
int i,disk;
struct DEVICEPARAMS dp;

    if (argc == 2)
        disk = atoi(argv[1]);
    else
        disk = get_drive();

    printf("\nDrive %c:\n",disk+'A');

    if ((i = disk_getparams(disk,&dp)) != DISK_OK) {
        lib_error("get",i);
    }
    else {
#ifdef _WIN32
#if defined __WATCOMC__ && __WATCOMC__ >= 1100
        if (dp.total_sectors > MAX_FAT16_SECS)
            printf("drive size %Lu\n",disk_size32(&dp));    /* Microsoft %? */
        else
#endif
#endif
        printf("drive size %lu\n",disk_size(&dp));

        display(&dp);
    }

    return 0;
}

void display(struct DEVICEPARAMS *dp)
{
   printf("\nGet Device Parameters\n\n");
   printf("cylinders:                           %d\n",dp->cylinders);
   printf("bytes per sector:                    %d\n",dp->sec_size);
   printf("sectors per allocation unit:         %d\n",dp->secs_cluster);
   printf("reserved sectors:                    %d\n",dp->reserved_secs);
   printf("number of FATs:                      %d\n",dp->num_fats);
   printf("number of root dir entries:          %d\n",dp->dir_entries);
   printf("number of sectors in logical image:  %d\n",dp->num_sectors);
   printf("media descriptor:                    %02x\n",dp->media_desc);
   printf("number of FAT sectors:               %d\n",dp->secs_fat);
   printf("sectors per track:                   %d\n",dp->secs_track);
   printf("number of heads:                     %d\n",dp->num_heads);
   printf("number of hidden sectors:            %ld\n",dp->hidden_sectors);
   printf("number of logical sectors:           %ld\n",dp->total_sectors);
}
