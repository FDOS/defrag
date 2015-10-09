/*
 * drives.c     list all physical and logical drives
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * Notes: BIOS functions always fail on HDDs under Windows.
 *
 */

#include <stdio.h>

#include "dosio.h"
#include "disklib.h"

main()
{
int i,t,s,h,d,r,p;
int type;
struct DEVICEPARAMS dp;
struct PARTITION_TYPE *par;

    printf("\nDrives & Partitions:\n");

    /* get list of all HDs and their number of tracks */
    /* get disk parameters from DOS and figure out if it's a partition */

    r = 0;
    d = 2;

    for (i = 2; i < 6; i++)
    {
        t = 0;
        disk_get_physical(i,&t,&s,&h);
        if (t > 0)
        {
            printf("\nDrive %d - Cyls: 0-%d\t",i,t);
            type = disk_type(i);

            if (type != 5 && type != 6)
            {
                par = partition_type(type);
                printf("NON-DOS (%s)",par->desc);
                continue;
            }
            p = 0;
            do {
                if (disk_getparams(d,&dp) != DISK_OK)
                    break;
                printf("%c: %d ",d+'A',dp.cylinders);
                p += dp.cylinders;
                d++;
            }
            while (p < t);
        }
    }
    printf("\n");
    return 0;
}
