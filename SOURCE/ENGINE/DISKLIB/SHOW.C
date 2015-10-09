/*
 * show.c       list all physical drives
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

int main()
{
int i,t,s,h,r;

    printf("\nPhysical Drives:\n");

    for (i = 0; i < 6; i++) {
        t = h = s = 0;
        printf("\nDrive %d - ",i);
        if ((r = disk_get_physical(i,&t,&s,&h)) != DISK_OK) {
#if defined _WINNT
            printf("n/a: %s",stroserror());
#else
            printf("n/a: %s",disk_error(r));
#endif
            continue;
        }
        printf("C %d, H %d, S %d",t,h,s);
        printf("\t primary type %02X",disk_type(i));
    }
    printf("\n");
    return 0;
}
