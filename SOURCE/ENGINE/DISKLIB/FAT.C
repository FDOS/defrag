/*
 * fat.c        read FAT12 and FAT16
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 11-Nov-1998  greggj  added the search...() functions
 *
 */

#include <stdlib.h>
#include <string.h>

#include "dosio.h"
#include "disklib.h"
#include "fat.h"

#ifdef NEED_LARGE
#include <malloc.h>
#define lmalloc(n) halloc(n,1)
#define lfree(p)   hfree(p)
#define SIZE_T long
#else
#define lmalloc(n) malloc(n)
#define lfree(p)   free(p)
#define SIZE_T unsigned int
#endif

int readfat12(int disk, int nclusters, long sector, int secsfat,
              unsigned short LARGE *clusters);
int readfat16(int disk, unsigned short nclusters, long sector, int secsfat,
              unsigned short LARGE *clusters);


/*
 *  readfat     reads the FAT into an array of integers (front end)
 *
 */

unsigned short LARGE *readfat(int disk, unsigned short nclusters, long sector,
                              int secsfat)
{
int i;
SIZE_T bufsiz;
unsigned short LARGE *buf;

    bufsiz = ((SIZE_T)nclusters+2) * sizeof(short);

    if (bufsiz & 1)
        ++bufsiz;

    if ((buf = lmalloc(bufsiz)) == NULL)
        return NULL;

#ifndef NEED_LARGE
    memset(buf,0,bufsiz);
#endif

    if (nclusters < 4096)
        i = readfat12(disk,nclusters,sector,secsfat,buf);
    else
        i = readfat16(disk,nclusters,sector,secsfat,buf);

    if (i != 1) {
        lfree(buf);
        return NULL;
    }

    return buf;
}

/*
 *  readfat12   reads 12-bit FAT
 *
 *  returns 1 okay, 0 disk i/o error, -1 malloc error
 */

int readfat12(int disk, int nclusters, long sector, int secsfat,
              unsigned short LARGE *clusters)
{
int h,i;
size_t bufsiz;
unsigned char *buf;
unsigned short *pc,c;

    bufsiz = (size_t)secsfat * 512;

    if ((buf = malloc(bufsiz)) == NULL)
        return -1;

    if (disk_read(disk,sector,buf,secsfat) != DISK_OK) {
        free(buf);
        return 0;
    }

    buf += 3;

    for (h = 2; h <= nclusters; buf++)
    {
        for (i = 0; i < 2; i++)
        {
            pc = (unsigned short *)(buf++);

            if (i&1)
                c = *pc >> 4;               /* odd */
            else
                c = *pc & 0xFFF;            /* even */

            clusters[h] = (unsigned short)c;

            if (++h > nclusters)
                break;

        }
    }

    free(buf);
    return 1;
}

/*
 *  readfat16   reads 16-bit FAT
 *
 *  returns 1 okay, 0 disk i/o error, -1 malloc error
 */

int readfat16(int disk, unsigned short nclusters, long sector, int secsfat,
              unsigned short LARGE *clusters)
{
long size;
unsigned int h;         /* Watcom 11.0 bug: DON'T use `unsigned short h'! */
unsigned short *buf;
unsigned int n,bufsiz;
int (*diskread)(int disk, long sector, void *buffer, int nsecs);

    /*
        TODO: There should be a way, either here or another function,
              to use another disk read function.
    */

#ifndef _WINNT
    drive_size(disk,&size);
    if (size > (1024L*1024L*32))
        diskread = disk_read_ext;
    else
#endif
        diskread = disk_read;

    clusters[0] = clusters[1] = 0;

    if (secsfat < 128)
    {
        bufsiz = (size_t)secsfat * 512;
        if ((buf = malloc(bufsiz)) != NULL)
        {
            if (diskread(disk,sector,buf,secsfat) != DISK_OK)
            {
                free(buf);
                return 0;
            }

            n = 0;
            for (h = 2; h <= nclusters; )
                clusters[h] = buf[n++];

            free(buf);
            return 1;
        }
    }

    bufsiz = 512/2;
    if ((buf = malloc(512 * sizeof(short))) == NULL)
        return -1;

    h = 2;
    for ( ; secsfat; --secsfat)
    {
        if (diskread(disk,sector++,buf,1) != DISK_OK)
        {
            free(buf);
            return 0;
        }

        if (h == 2)
            n = 1;
        else
            n = 0;

        for (; n < bufsiz; n++)
        {
            clusters[h] = buf[n];

            if (++h > nclusters)
                break;
        }
    }

    free(buf);
    return 1;
}

/*
 *  serchfat12      reads 12-bit FAT
 *
 */

unsigned short searchfat12(int disk, int nclusters, long sector, int secsfat,
                           unsigned short value)
{
int h,i;
size_t bufsiz;
unsigned char *buf;
unsigned short *pc,c,cluster;

    bufsiz = (size_t)secsfat * 512;

    if ((buf = malloc(bufsiz)) == NULL)
        return -1;

    if (disk_read(disk,sector,buf,secsfat) != DISK_OK) {
        free(buf);
        return 0;
    }

    buf += 3;
    cluster = 0;

    for (h = 2; h <= nclusters; buf++)
    {
        for (i = 0; i < 2; i++)
        {
            pc = (unsigned short *)(buf++);

            if (i&1)
                c = *pc >> 4;               /* odd */
            else
                c = *pc & 0xFFF;            /* even */

            if (c)
                cluster = h;

            if (++h > nclusters)
                break;

        }
    }

    free(buf);
    return cluster;
}

/*
 *  searchfat16   reads 16-bit FAT
 *
 */

unsigned short searchfat16(int disk, unsigned short nclusters, long sector,
                           int secsfat, unsigned short value)
{
long size;
unsigned int n;
unsigned int h,cluster;
unsigned short buf[512];
int (*diskread)(int disk, long sector, void *buffer, int nsecs);

    /*
        TODO: There should be a way, either here or another function,
              to use another disk read function.
    */

#ifndef _WINNT
    drive_size(disk,&size);
    if (size > (1024L*1024L*32))
        diskread = disk_read_ext;
    else
#endif
        diskread = disk_read;

    h = 2;
    cluster = 0;

    for ( ; secsfat; --secsfat)
    {
        if (diskread(disk,sector++,buf,1) != DISK_OK)
            return 0;

        if (h == 2)
            n = 1;
        else
            n = 0;

        for (; n < 512/2; n++)
        {
            if (buf[n])
                cluster = h;

            if (++h > nclusters)
                break;
        }
    }

    return h;
}
