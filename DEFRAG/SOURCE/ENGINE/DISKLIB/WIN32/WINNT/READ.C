/*
 * winnt\read.c     logical (absolute) disk read and write
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>

#include "win32.h"
#include "dosio.h"
#include "debug.h"

enum { DISK_READ, DISK_WRITE };

static int diskio(int cmd, int disk, long sector, void *buffer, int nsecs);

/*
 * disk_read
 *
 */

extern int disk_read(int disk, long sec, void *buf, int nsecs)
{
    return diskio(DISK_READ,disk,sec,buf,nsecs);
}

/*
 * disk_write
 *
 */

extern int disk_write(int disk, long sec, void *buf, int nsecs)
{
    return diskio(DISK_WRITE,disk,sec,buf,nsecs);
}


/*
 * diskio       common disk i/o stuff
 *
 */

static int diskio(int cmd, int disk, long sector, void *buffer, int nsecs)
{
int i;
OVERLAPPED pos;
unsigned long nread;
static int drive = 0;
static HANDLE dev = 0;

    if (dev == 0)           /* first time */
    {
        dev = opendrive(disk);
        if (dev == INVALID_HANDLE_VALUE)
        {
            dev = 0;
            return DEVICE_ERR;
        }
        drive = disk;
    }
    else                    /* not first, has disk changed? */
    {
        if (drive != disk)
        {
            closedrive(dev);
            dev = opendrive(disk);
            if (dev == INVALID_HANDLE_VALUE)
            {
                dev = 0;
                return DEVICE_ERR;
            }
            drive = disk;
        }
    }

    pos.hEvent = NULL;
    pos.Offset = sector * 512L;
    pos.OffsetHigh = 0;
    /* TODO: support for sectors > 8,388,607 */

    if (cmd == DISK_READ)
        i = ReadFile(dev,buffer,512L*nsecs,&nread,&pos);
    else if (cmd == DISK_WRITE)
        i = WriteFile(dev,buffer,512L*nsecs,&nread,&pos);

    DBG_err_dump((cmd == DISK_READ) ? "read" : "write");

    return (i) ? DISK_OK : DOS_ERR;
}
