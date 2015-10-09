/*
 * media.c      list all physical and logical drives, version 2
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <conio.h>

#include "dosio.h"
#include "disklib.h"

int media_type(struct MID *mid);
enum { PT_SET, PT_GET };
int part_types(int cmd, int arg);
int get_first(unsigned long a[32]);
int get_next(unsigned long a[32], int n);
int set_first(unsigned long a[32]);

int main(int argc, char **argv)
{
int i;
int t,s,h,r,d,p,c,type;
struct MID mid;
struct PARTITION_TYPE *par;
unsigned long array[32];

    /*
        I know that this program looks funky. But there is a
        reason for it being so complex.

        I'll write up a detailed description one of these days.
    */

    part_types(PT_SET,0);

#if defined __WATCOMC__ || defined __GNUC__
   setvbuf(stdout,NULL,_IONBF,0);      /* flush stdout every printf */
#endif
    printf("\nDrive Media:\n");

    /* get list of all HDs and their number of tracks */
    /* get disk parameters from DOS and figure out if it's a partition */

    memset(array,0,sizeof(array));
    array[0] = array[1] = 1;
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
            if (type == 5) type = 6;

          //if (!part_types(PT_GET,type))   /* not finalized yet */
            if (type != 6)
            {
                par = partition_type(type);
                printf("NON-DOS (%s)",par->desc);
                continue;
            }

            d = get_first(array);
            p = 0;
            do {
                if (disk_getmedia(d,&mid) != DISK_OK)
                    break;
                if (media_type(&mid) == type) {
                    if (disk_get_logical(d,&c,0,0) != DISK_OK)
                        break;
                    printf("%c: %d ",d+'A',c);
                    p += c;
                    set_first(array);
                }
                d = get_next(array,d);
            }
            while (p <= t);
        }
    }

    printf("\n");
    return 0;
}

int media_type(struct MID *mid)
{
    if (strncmp(mid->filesys,"FAT",3) == 0)
        return 6;
    if (strncmp(mid->filesys,"NTFS",4) == 0)
        return 7;
    return 0;
}

int get_first(unsigned long a[32])
{
int i;

    for (i = 0; i < 32; i++) {
        if (a[i] == 0) {
            break;
        }
    }
    return i;
}

int get_next(unsigned long a[32], int n)
{
int i;

    for (i = n+1; i < 32; i++) {
        if (a[i] == 0) {
            break;
        }
    }
    return i;
}

int set_first(unsigned long a[32])
{
int i;

    for (i = 0; i < 32; i++) {
        if (a[i] == 0) {
            a[i] = 1;
            break;
        }
    }
    return i;
}


int part_types(int cmd, int arg)
{
int os,i;
static int *types;
static int ntypes;

    if (cmd == PT_SET) {
        os = lib_ver();
        if (os == LIB_WINNT) {
            types = calloc(sizeof(int),2);
            types[0] = 6;
            types[1] = 7;
            ntypes = 2;
        }
        else if (os == LIB_DOS) {
            types = calloc(sizeof(int),1);
            types[0] = 6;
            ntypes = 1;
        }
        return ntypes;
    }
    else if (cmd == PT_GET) {
        for (i = 0; i < ntypes; i++) {
            if (types[i] == arg)
                return 1;
        }
        return 0;
    }

    return -1;
}
