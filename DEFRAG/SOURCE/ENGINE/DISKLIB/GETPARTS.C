/*
 * getparts.c       get partition information
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 14-Dec-1998  greggj  fixed casts where offsetof is used
 *
 */

/*
             ******************************************
             * GETPARTS is a "Proof of Concept" only. *
             * It really does not do anything useful. *
             ******************************************

    This program will calculate a partition table for the
    MASTER BOOT RECORD and does so by reading all of the
    BOOT SECTORS of a drive. The MBR table is NOT READ; this
    means that the data displayed here can be used to recreate
    a damaged or missing MBR partition table!

    This program displays the data only. A future version might
    have the option to write to the MBR or to write an MBR to
    a binary file.

    LIMITATIONS:

        o   FAT and NTFS drives only.
        o   Partitions within partitions ARE NOT SUPPORTED.
        o   Although will compile under Windows, it will return
            an error for disk_get_physical() (It's an MS thing.)

    MACHINES TESTED:

        See GETPARTS.TXT.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "dosio.h"
#include "disklib.h"

long get_sectors(char *buf);

void usage(void)
{
    printf("\n");
    printf("usage: getparts <0-3>\n");
    printf("\n");
    printf("Displays partition information from HDD 0-3 to STDOUT\n");
    printf("without reading the MBR.\n");
    printf("\n");

    exit(1);
}

int main(int argc, char **argv)
{
int disk;
int d,i;
int t,s,h;
long sectors,start;
char boot[512];
int cylinders,secs,heads;

    setvbuf(stdout,NULL,_IONBF,0);      /* flush stdout every printf */

    if (argc < 2) {
        disk = 0;
        printf("\nNo drive specified, defaulting to drive 0.\n");
    }
    else {
        disk = atoi(argv[1]);
    }

    if (disk < 0 || disk > 3)
        usage();

    disk += 2;      /* library uses 0,1 as FDD, 2-5 as HDD */


    /* get drive geometry */

    i = disk_get_physical(disk,&cylinders,&secs,&heads);

    if (i != DISK_OK) {
        printf("\nBIOS Error: %s [%02xh]\n",disk_error(i),i);
        exit(2);
    }

    ++heads;        /* library returns these as 0-N but our math */
    ++cylinders;    /*  here thinks of them as 1-N */

    printf("\nHDD %d: %d,%d,%d, sectors: %ld\n",disk-1,cylinders,secs,heads,
            (long)cylinders*secs*heads);

    /* start off at Partition zero and first MBR */

    d = 0;
    sectors = 0;
    t = 0;
    h = 0;
    s = 1;
    start = (long)secs;

    for (;;)
    {
        printf("\n%d, ",d);

        /* first partition is slightly different that any other */

        if (d == 0) {
            printf("start: %d,%d,%d, ",t,h+1,s);
            start = logical_sector(t,s,h+1,0,secs,heads);
        }
        else {
            printf("start: %d,%d,%d, ",t,h,s);
            start = logical_sector(t,s,h,0,secs,heads);
        }

        /* read boot sector (MBR + next head) */

        i = disk_read_p(disk,t,s,h+1,boot,1);

        if (i != DISK_OK) {
            printf("\nBIOS Error: %s [%02xh]\n",disk_error(i),i);
            exit(2);
        }

        if (*(unsigned short*)(boot+510) != 0xAA55) {
            printf(" could not find boot sector!\n");
            break;
        }

        /* get sectors from boot sector */

        sectors += get_sectors(boot);

        /* calculate where next track starts */

        physical_p(&t,&s,&h,sectors,secs,heads);

        printf("end: %d,%d,%d, ",t,h,s);
        if (d == 0)
            printf("sec: %ld, # %ld\n",start,sectors);
        else
            printf("sec: %ld, # %ld\n",start,(sectors-start)+1);

        /* done if end of drive reached */

        if (t >= cylinders-1)
            break;

        /* add to total sectors any "slack space" and the size of a track */

        sectors += (secs - s);
        sectors += secs;

        /* start CHS of next partition (an MBR) is next track, next head */

        s = 1;
        t++;
        h++;
        if (h == heads)
            h = 0;
        d++;
    }

    printf("\n");
    return 0;
}


long get_sectors(char *buf)
{
    /*
        TODO: Figure out how to get partition information on
              EXT2 drives.
    */

    if (strncmp(buf+3,"NTFS",4) == 0)
    {
        /*
            Note that `num_secs_hi' is not used... if it is non-zero
            does that mean a 64 bit number? My NT drive size is less
            than LONG_MAX. Also, I had to add one to get the correct
            number.
        */

        return *(UINT32*)(buf + offsetof(struct BOOT_NTFS,num_secs_lo))+1;
    }
    else if (*(UINT16*)(buf + offsetof(struct BOOT,secs_fat)) == 0)
    {
        return *(UINT32*)(buf + offsetof(struct BOOT_FAT32,total_sectors));
    }
    else
    {
        return *(UINT32*)(buf + offsetof(struct BOOT,total_sectors));
    }
}
