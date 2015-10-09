/*
 * boota.c      read and dump 16 bytes of boot sector
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
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "dosio.h"
#include "disklib.h"

#define MB32    (1024L*1024L*32)

void dump(unsigned char *b);

int main(int argc, char **argv)
{
int i,disk;
char buf[512];
char filesys[8];

    if (argc == 2)
        disk = atoi(argv[1]);
    else
        disk = get_drive();

    disk_getfilesys(disk,filesys);

    printf("\nBOOT sector read test\n");
    printf("\nLIB Ver %x, DOS Ver %x, WIN Ver %x",lib_ver(),dos_ver(),win_ver());
    printf("\nDrive %c:, %8.8s\n",disk+'A',filesys);

    printf("\nINT 25h sector 0\n");
    i = disk_read(disk,0,buf,1);
    if (i != DISK_OK)
        lib_error("",i);
    else
        dump((unsigned char *)buf);

    printf("\nINT 25h Ext. sector 0\n");
    i = disk_read_ext(disk,0,buf,1);
    if (i != DISK_OK)
        lib_error("",i);
    else
        dump((unsigned char *)buf);

    printf("\nINT 21h/7305h sector 0\n");
    i = disk_read32(disk,0,buf,1);
    if (i != DISK_OK)
        lib_error("",i);
    else
        dump((unsigned char *)buf);

    printf("\nINT 21h/440dh/0861h t0,h0,s1\n");
    i = disk_read_ioctl(disk,0,1,0,buf,1);
    if (i != DISK_OK)
        lib_error("",i);
    else
        dump((unsigned char *)buf);

    printf("\nINT 21h/440dh/4861h t0,h0,s1\n");
    i = disk_read_ioctl32(disk,0,1,0,buf,1);
    if (i != DISK_OK)
        lib_error("",i);
    else
        dump((unsigned char *)buf);

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
