/*
 * dump.c       read and display sector
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
#include <string.h>
#include <ctype.h>

#include "dosio.h"
#include "disklib.h"

#define MB32    (1024L*1024L*32)

void display(char *b);
void boot_ntfs(struct BOOT_NTFS *b);
void boot_fat(struct BOOT *b);
void hexdump(unsigned char *b, unsigned int n);
void asciidump(unsigned char *b, unsigned int n);
void dump(unsigned char *buf);

int main(int argc, char **argv)
{
long size;
int i,disk,t,s,h,n;
char *buf;

    /*
        This just reads and dumps sectors using disk_read_ioctl().
        It's for testing.
    */

    if (argc == 2)
        disk = atoi(argv[1]);
    else
        return 0;

    t = 0;
    s = 1;
    h = 1;
    n = 2;

    if ((buf = malloc(n * 512)) == NULL)
        abort();

    printf("\nDrive %d: track %d, sector %d, head %d\n",disk,t,s,h);

    i = disk_read_ioctl(disk,t,s,h,buf,n);

    if (i != DISK_OK) {
        lib_error("read",i);
        return 1;
    }

    for (i = 0; i < n*512; i += 16) {
        hexdump(buf+i,16);
        asciidump(buf+i,16);
        printf("\n");
    }

    return 0;
}

void dump(unsigned char *buf)
{
int i;

    for (i = 0; i < 512; i += 16) {
        hexdump(buf+i,16);
        asciidump(buf+i,16);
        printf("\n");
    }
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
