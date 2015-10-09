/*
 * bootaw.c     read, modify AND WRITE boot sector
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 04-Nov-1998  added the lock stuff and more comments
 *
 */

/*
 * See also TESTIOCT.C.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "dosio.h"
#include "disklib.h"

#define MB32    (1024L*1024L*32)

void dump(unsigned char *b);

int main(int argc, char **argv)
{
int i,disk;
char buf[512];
char filesys[8];
int numlocks,level,perm;

#define MOD 1   /* you have to make this 1 to modifiy the sector data */

    /*
        This program MODIFIES the BOOT sector, possibly more than once.
        Do not run on an important drive. This is for testing only.
        Use a disk utility like Diskwarez's DISKBUG to save the BOOT
        sector to a file for backup purposes if you are going to be
        testing this program on your bootable HDD.

        This program reads and write the BOOT sector using FIVE
        different functions.

    */

    disk = 0;   /* Default to A: you have to change this to test a HDD */

    disk_getfilesys(disk,filesys);
    printf("\nBOOT sector write test\n");
    printf("\nLIB Ver %x, DOS Ver %x, WIN Ver %x",lib_ver(),dos_ver(),win_ver());
    printf("\nDrive %c:, %8.8s\n",disk+'A',filesys);

#if defined _WIN32
    /*
        Let's see what a Lock Level 1 does...

        Without a lock (a Lock Level of -1) the writes will fail.

        It turns out that a Lock Level 1 will allow the BOOT sector
        to be modified and written and all of the following writes
        might succeed -- I've only tested on Windows 95.
    */

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
#endif

    printf("\nINT 25h sector 0\n");
    i = disk_read(disk,0,buf,1);
    if (i != DISK_OK)
        lib_error("read",i);
    else {
        dump((unsigned char *)buf);
#if MOD
        buf[3] += 1;
#endif
        dump((unsigned char *)buf);
        i = disk_write(disk,0,buf,1);
        if (i != DISK_OK)
            lib_error("write",i);
    }

    printf("\nINT 25h Ext. sector 0\n");
    i = disk_read_ext(disk,0,buf,1);
    if (i != DISK_OK)
        lib_error("read",i);
    else {
        dump((unsigned char *)buf);
#if MOD
        buf[3] += 1;
#endif
        dump((unsigned char *)buf);
        i = disk_write_ext(disk,0,buf,1);
        if (i != DISK_OK)
            lib_error("write",i);
    }

    printf("\nINT 21h/7305h sector 0\n");
    i = disk_read32(disk,0,buf,1);
    if (i != DISK_OK)
        lib_error("read",i);
    else {
        dump((unsigned char *)buf);
#if MOD
        buf[3] += 1;
#endif
        dump((unsigned char *)buf);
        i = disk_write32(disk,0,buf,1);
        if (i != DISK_OK)
            lib_error("write",i);
    }

    printf("\nINT 21h/440dh/0861h t0,h0,s1\n");
    i = disk_read_ioctl(disk,0,1,0,buf,1);
    if (i != DISK_OK)
        lib_error("read",i);
    else {
        dump((unsigned char *)buf);
#if MOD
        buf[3] += 1;
#endif
        dump((unsigned char *)buf);
        i = disk_write_ioctl(disk,0,1,0,buf,1);
        if (i != DISK_OK)
            lib_error("write",i);
    }

    printf("\nINT 21h/440dh/4861h t0,h0,s1\n");
    i = disk_read_ioctl32(disk,0,1,0,buf,1);
    if (i != DISK_OK)
        lib_error("read",i);
    else {
        dump((unsigned char *)buf);
#if MOD
        buf[3] += 1;
#endif
        dump((unsigned char *)buf);
        i = disk_write_ioctl32(disk,0,1,0,buf,1);
        if (i != DISK_OK)
            lib_error("write",i);
    }

    return 0;
}

void hexdump(unsigned char *b, unsigned int n)
{
    while (n > 0) {
        printf("%02x ",*b++);
        --n;
    }
}

void asciidump(unsigned char *b, unsigned int n)
{
    while (n > 0) {
        printf("%c",isprint(*b) ? *b : '.');
        ++b;
        --n;
    }
}

void dump(unsigned char *b)
{
    hexdump(b,16);
    asciidump(b,16);
    printf("\n");
}
