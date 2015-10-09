/*
 * winnt\bios.c     physical functions (Not really BIOS calls, their equivalent)
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 20-Dec-1998  greggj  fixed translation error in diskio()
 *
 */

#include <stdio.h>

#include "win32.h"
#include "dosio.h"
#include "debug.h"
#include "disklib.h"

enum { DISK_READ, DISK_WRITE };

static int diskio(int cmd, int disk, int trk, int sec, int head,
                  void *buffer, int nsecs);

/*
 * disk_get_physical    get basic disk geometry (tracks, sectors, heads)
 *
 */

extern int disk_get_physical(int disk, int *t, int *s, int *h)
{
int i;
HANDLE dev;
unsigned long bc;
DISK_GEOMETRY geo;

    dev = opendrivephy(disk);
    if (dev == INVALID_HANDLE_VALUE)
        return DEVICE_ERR;

    /*
    When more calls to DeviceIoControl() are made the file
    DEVICE.C will be created.
    */

    i = DeviceIoControl(dev,IOCTL_DISK_GET_DRIVE_GEOMETRY,NULL,0,
                        &geo,sizeof(DISK_GEOMETRY),&bc,NULL);

    disk_errno = (int)GetLastError();

    if (i == FALSE) {
        closedrive(dev);
        return DEVICE_ERR;
    }

    /* results same as INT 13h function 48h */

    if (t) *t = geo.Cylinders.LowPart - 2;
    if (h) *h = geo.TracksPerCylinder - 1;
    if (s) *s = geo.SectorsPerTrack;

    closedrive(dev);
    return DISK_OK;
}

/*
 * disk_read_p      read sector(s)
 *
 */

extern int disk_read_p(int disk, int trk, int sec, int head,
                       void *buf, int nsecs)
{
    return diskio(DISK_READ,disk,trk,sec,head,buf,nsecs);
}

/*
 * disk_write_p     write sector(s)
 *
 */

extern int disk_write_p(int disk, int trk, int sec, int head,
                        void *buf, int nsecs)
{
    return diskio(DISK_WRITE,disk,trk,sec,head,buf,nsecs);
}

/*
 * disk_part        get disk partition sector
 *
 */

extern int disk_part(int disk, int c, int h, int s, void *buf)
{
    return disk_read_p(disk,c,s,h,buf,1);
}

/*
 * disk_type        get disk (partition) type
 *
 */

extern int disk_type(int disk)
{
int i;
unsigned char buf[512];

    /* will fail on floppy drives */

    i = disk_read_p(disk,0,1,0,buf,1);
    if (i != DISK_OK)
        return 0;
#ifdef __GNUC__
    return (int)buf[0x1be + 4]; /* what is with this white space requirement! */
#else
    return (int)buf[0x1be+4];
#endif
}

/*
 * disk_error
 *
 */

extern const char *disk_error(int error)
{
    return stroserror();
}


/*
 * diskio       common disk i/o stuff
 *
 */

static int diskio(int cmd, int disk, int trk, int sec, int head,
                  void *buffer, int nsecs)
{
int i;
unsigned long nread;
OVERLAPPED pos;

static HANDLE dev = 0;
static int drive = 0;
static int hidden,maxh,maxs;

    if (dev == 0)       /* first time */
    {
        dev = opendrivephy(disk);
        if (dev == INVALID_HANDLE_VALUE)
        {
            dev = 0;
            return DEVICE_ERR;
        }
        drive = disk;

        disk_get_physical(disk,NULL,&maxs,&maxh);

        /*
            Even though "physical" access, the NT API call is still
            absolute, not CHS, so the sectors per tracks and number
            of heads are needed to calulate the absolute position
            from the CHS (similar to LBA).
        */
    }
    else                /* not first, has the disk changed? */
    {
        if (drive != disk)
        {
            closedrive(dev);
            dev = opendrivephy(disk);
            if (dev == INVALID_HANDLE_VALUE)
            {
                dev = 0;
                return DEVICE_ERR;
            }
            drive = disk;

            disk_get_physical(disk,NULL,&maxs,&maxh);
        }
    }

    pos.hEvent = NULL;
    pos.Offset = logical_sector(trk,sec,head,hidden,maxs,maxh+1) * 512L;
    pos.OffsetHigh = 0;
    /* TODO: support for sectors > 8,388,607 */

    if (cmd == DISK_READ)
        i = ReadFile(dev,buffer,512L*nsecs,&nread,&pos);
    else if (cmd == DISK_WRITE)
        i = WriteFile(dev,buffer,512L*nsecs,&nread,&pos);

    return (i) ? DISK_OK : DISK_ERR;
}
