/*
 * win32\getdisk.c  get disk parameters
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 20-Dec-1998  greggj  fixed byte count for reading Media Descriptor
 * 26-Nov-1998  greggj  fixed type bug in disk_setmedia()
 * 12-Nov-1998  greggj  added better media type checking in boot_...()
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disklib.h"
#include "win32.h"
#include "dosio.h"

static int boot_parameters(int disk, struct DEVICEPARAMS *dp);

/*
 * disk_getparams   get disk (FAT) parameters
 *
 */

extern int disk_getparams(int disk, struct DEVICEPARAMS *dp)
{
int i;
struct EXT_DEVICEPARAMS edp;

    memset(dp,0,sizeof(struct DEVICEPARAMS));

    if ((i = win_ioctl_ext(DOS_MINOR_GET_DEVICE,disk,&edp)) == DEVICE_OK) {
        memcpy(dp,&edp,sizeof(struct DEVICEPARAMS));
        return DISK_OK;
    }

    if ((i = win_ioctl(DOS_MINOR_GET_DEVICE,disk,dp)) == DEVICE_OK)
        return DISK_OK;

    /*
        TODO: Might want to catch certain errors here - Invalid Drive
        perhaps. But some errors such as Invalid Function means that
        the media itself should be read.
    */

    if (i != DEVICE_ERR)
        i = boot_parameters(disk,dp);

    return i;
}

/*
 * disk_getparams32   get disk parameters FAT32
 *
 */

extern int disk_getparams32(int disk, struct EXT_DEVICEPARAMS *dp)
{
int i;

    memset(dp,0,sizeof(struct EXT_DEVICEPARAMS));

    i = win_ioctl_ext(DOS_MINOR_GET_DEVICE,disk,dp);

    /*
        Althouth the FAT32 BOOT sector contains an EXT_BPB,
        if the above call fails the drive is probably not
        FAT32.
    */

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/* Note: There is no disk_setparams() function here. */

/*
 * disk_getmedia
 *
 */

extern int disk_getmedia(int disk, struct MID *mid)
{
int i;

    memset(mid,0,sizeof(struct MID));

    if ((i = win_ioctl_ext(DOS_MINOR_GET_MEDIA,disk,mid)) != DEVICE_OK)
        i = win_ioctl(DOS_MINOR_GET_MEDIA,disk,mid);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_setmedia        write MID structure to disk
 *
 */

extern int disk_setmedia(int disk, struct MID *mid)
{
int i;

    if ((i = win_ioctl_ext(DOS_MINOR_SET_MEDIA,disk,mid)) != DEVICE_OK)
        i = win_ioctl(DOS_MINOR_SET_MEDIA,disk,mid);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_get_logical     get disk's logical geometry
 *
 */

extern int disk_get_logical(int disk, int *t, int *s, int *h)
{
int i;
struct DEVICEPARAMS dp;

    if ((i = disk_getparams(disk,&dp)) != DISK_OK)
        return i;

    if (t) *t = dp.cylinders;
    if (s) *s = dp.secs_track;
    if (h) *h = dp.num_heads;

    return i;
}

/*
 * disk_gettype     get drive type
 *
 */

extern int disk_gettype(int disk)
{
int i;
struct MID m;

    i = disk_getmedia(disk,&m);

    if (i != DISK_OK)
        return i;

    if (strncmp(m.filesys,"FAT12",5) == 0)
        return FAT1216;

    if (strncmp(m.filesys,"FAT16",5) == 0)
        return FAT1216;

    if (strncmp(m.filesys,"FAT32",5) == 0)
        return FAT32;

    if (strncmp(m.filesys,"FAT",3) == 0)
        return FAT1216;

    if (strncmp(m.filesys,"NTFS",4) == 0)
        return NTFS;

    if (strncmp(m.filesys,"CDFS",4) == 0)
        return CDFS;

    return UNKFS;
}

extern int disk_getfilesys(int disk, char filesys[8])
{
int i;
struct MID m;

    i = disk_getmedia(disk,&m);

    if (i != DISK_OK) {
        strcpy(filesys,"unknown");
        return i;
    }

    strncpy(filesys,m.filesys,8);
    return DISK_OK;
}



/*
 * boot_parameters      get disk (BOOT) parameters
 *
 */

static int boot_parameters(int disk, struct DEVICEPARAMS *dp)
{
int i;
char buf[512];
struct BOOT boot;
unsigned short tmp;

    memset(buf,0,512);
    if ((i = disk_read(disk,0,buf,1)) != DISK_OK)
        return i;

    tmp = (unsigned short)*(unsigned char*)(buf+21);    /* extract md */

    if ((tmp < 0xf8 || tmp > 0xff) && tmp != 0xf0)
        return DISK_ERR;

    /* ramdrives might not have the AA55 signature */
    /*  (well, RAMDRIVE.SYS does not have the AA55 signature) */
    /* and non-DOS disks can have the AA55 signature */

    tmp = *(unsigned short*)(buf+11);       /* extract sector size */

    if ((buf[0]!=0xeb && buf[0]!=0xe9 && buf[0]!=0) ||
       (buf[2]!=0x90 && buf[2]!=0) || tmp != 512)
        return DISK_ERR;

    if (buf[510] != 0x55 && buf[511] != 0xAA)
        return DISK_ERR;

    /*
        Note that the INT 25h < 32MB function is used here without
        testing for the drive size. Because boot_parameters() is
        called only when disk_getparams() fails, and that happens
        either on a ramdrive or other installable driver that is
        rather old and/or lame and this will work, or on a drive
        where any DOS disk I/O will fail.

        This has yet to be tested on a Zip/Jazz/other removable
        device.
    */

    memset(&boot,0,sizeof(struct BOOT));
    memcpy(&boot,buf,sizeof(struct BOOT));
    memcpy(&dp->sec_size,&boot.sec_size,sizeof(struct BPB));

    return i;
}
