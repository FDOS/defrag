/*
 * lock.c       test the Windows 95/98 LOCK functions
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "dosio.h"
#include "disklib.h"

int main(int argc, char **argv)
{
int i,disk;
char files[261];
int numlocks,level,perm;

    /*
        This is NOT fully tested yet.

        The "Locking Hierarchy" of Windows is, to me, complex.

        I think I have the right order here: LEVEL 1 with the
        permissions, followed by LEVEL 2, check LOCK FLAG to
        see if a LOCK 3 can be acheived, in LOCK 3 disk writes
        should be allowed, when finished go back to LEVEL 2,
        do more in and out of LEVEL 3 and LEVEL 2, then go out
        of LEVEL 1 when done.

        The code will fail in Windows console mode and in an
        MS-DOS box.

        I am still working on a full WIN32 Mode version.
    */

    if (argc == 2)
        disk = atoi(argv[1]);
    else
        disk = get_drive();

    printf("\nDrive %c:\n",disk+'A');

    i = disk_lock_state(disk,&level,&perm);
    printf("\n");
    printf("result: %d\n",i);
    printf("Level       %d\n",level);
    printf("Permission  %d\n",perm);

    i = disk_lock_logical(disk,LOCK_1,LOCK_PERM_WRT|LOCK_PERM_MAP);
    if (i != DISK_OK)
        abort();
    i = disk_lock_state(disk,&level,&perm);
    printf("\n");
    printf("lock result: %d\n",i);
    printf("Level       %d\n",level);
    printf("Permission  %d\n",perm);

    if (i != DISK_OK)
        abort();

    i = disk_lock_flag(disk);
    printf("\n");
    printf("flag result: %xh\n",i);

    if (i == DOS_ERR)
        abort();

    i = disk_lock_logical(disk,LOCK_2,0);
    if (i != DISK_OK)
        abort();
    i = disk_lock_state(disk,&level,&perm);
    printf("\n");
    printf("lock result: %d\n",i);
    printf("Level       %d\n",level);
    printf("Permission  %d\n",perm);

    if (i != DISK_OK)
        abort();

    i = disk_unlock_logical(disk);
    if (i != DISK_OK)
        abort();
    i = disk_lock_state(disk,&level,&perm);
    printf("\n");
    printf("unlock result: %d\n",i);
    printf("Level       %d\n",level);
    printf("Permission  %d\n",perm);
    if (i != DISK_OK)
        abort();

    i = disk_unlock_logical(disk);
    if (i != DISK_OK)
        abort();
    i = disk_lock_state(disk,&level,&perm);
    printf("\n");
    printf("unlock result: %d\n",i);
    printf("Level       %d\n",level);
    printf("Permission  %d\n",perm);
    if (i != DISK_OK)
        abort();

    getchar();

    return 0;
}
