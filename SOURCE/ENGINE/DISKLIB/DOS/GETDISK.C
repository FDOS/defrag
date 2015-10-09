/*
 * dos\getdisk.c    get disk parameters
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 20-Dec-1998  greggj  fixed byte count for reading Media Descriptor
 * 12-Nov-1998  greggj  added better media type checking in boot_...()
 * 11-Nov-1998  greggj  added function disk_getparamblk()
 *
 */

#include <string.h>     /* memset, memcpy */

#include "dosio.h"
#include "disklib.h"

static int boot_parameters(int disk, struct DEVICEPARAMS *dp);

/*
 * disk_getparamblk     get disk parameter block
 *
 */

#ifndef __GNUC__    /* have not ported doscall() yet */

extern int disk_getparamblk(int disk, struct DPB *dpb)
{
int i;

    memset(dpb,0,sizeof(struct DPB));

    dpb->drive_num = disk+1;
    i = doscall(DOS_GET_DPB,0,0,disk+1,dpb);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

#endif /* __GNUC__ */

/*
 * disk_get_params      get disk (DOS) parameters
 *
 * Note: This function works on NTFS drives.
 *
 */

extern int disk_getparams(int disk, struct DEVICEPARAMS *dp)
{
int i;

    memset(dp,0,sizeof(struct DEVICEPARAMS));

    /* TODO: check for remote */
    /* TODO: check for removable */

    dp->special = 1;

    if ((i = dos_ioctl(DOS_DEV_IOCTL,DOS_MINOR_GET_DEVICE,disk,dp)) != DEVICE_OK)
        if ((i = boot_parameters(disk,dp)) == DISK_OK)
            dp->cylinders = max_track(dp);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_get_parameters32    get disk (DOS) parameters FAT32
 *
 * Note: This function fails horribly under Windows NT!
 *
 */

extern int disk_getparams32(int disk, struct EXT_DEVICEPARAMS *dp)
{
int i;

    memset(dp,0,sizeof(struct EXT_DEVICEPARAMS));

    dp->special = 1;

    i = dos_ioctl32(DOS_DEV_IOCTL,DOS_MINOR_GET_DEVICE,disk,dp);

    /*
        The BOOT sector is not read if the above call fails. It
        is doubtful that a FAT32 drive/driver will not support
        the call so if the call fails the drive is not FAT32.
    */

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_getmedia        get MID structure
 *
 * Note: This function does not get the actual filesys and vollabel
 *       MID structure members under windows NT (filesys is always
 *       "FAT" or "CDFS" and the vollabel is either all spaces or
 *       the volume label with spaceas as the middle four characters.
 *
 *       This function reads CD-ROM and NTFS drives under Windows NT.
 *
 */

extern int disk_getmedia(int disk, struct MID *mid)
{
int i;

    memset(mid,0,sizeof(struct MID));

    i = dos_ioctl(DOS_DEV_IOCTL,DOS_MINOR_GET_MEDIA,disk,mid);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_setmedia        write MID structure to disk
 *
 * Note: Although disk_getmedia() acts weirdly under Windows NT this
 *       function works fine under Windows NT.
 */

extern int disk_setmedia(int disk, struct MID *mid)
{
int i;

    i = dos_ioctl(DOS_DEV_IOCTL,DOS_MINOR_SET_MEDIA,disk,mid);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return DISK_OK;
}

/*
 * disk_getmedia32      get MID structure FAT32
 *
 */

extern int disk_getmedia32(int disk, struct MID *mid)
{
int i;

    memset(mid,0,sizeof(struct MID));

    i = dos_ioctl32(DOS_DEV_IOCTL,DOS_MINOR_GET_MEDIA,disk,mid);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_setmedia32      write MID structure to disk FAT32
 *
 */

extern int disk_setmedia32(int disk, struct MID *mid)
{
int i;

    i = dos_ioctl32(DOS_DEV_IOCTL,DOS_MINOR_SET_MEDIA,disk,mid);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return DISK_OK;
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

    return DISK_OK;
}

/*
 * disk_get_logical32   get disk's logical geometry FAT32
 *
 */

extern int disk_get_logical32(int disk, int *t, int *s, int *h)
{
int i;
struct EXT_DEVICEPARAMS dp;

    if ((i = disk_getparams32(disk,&dp)) != DISK_OK)
        return i;

    if (t) *t = dp.cylinders;
    if (s) *s = dp.secs_track;
    if (h) *h = dp.num_heads;

    return DISK_OK;
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
struct BOOT boot;
unsigned short tmp;
unsigned char buf[512];

    memset(buf,0,512);
    i = disk_read(disk,0,buf,1);

    tmp = (unsigned short)*(unsigned char*)(buf+21);    /* extract md */

    if ((tmp < 0xf8 || tmp > 0xff) && tmp != 0xf0) {
        setdoserror(DOS_ENONDOS);
        return DOS_ERR;
    }

    /* ramdrives might not have the AA55 signature */
    /*  (well, RAMDRIVE.SYS does not have the AA55 signature) */
    /* and non-DOS disks can have the AA55 signature */

    tmp = *(unsigned short*)(buf+11);       /* extract sector size */

    if ((buf[0]!=0xeb && buf[0]!=0xe9 && buf[0]!=0) ||
       (buf[2]!=0x90 && buf[2]!=0) || tmp != 512) {
        setdoserror(DOS_ENONDOS);
        return DOS_ERR;
    }

    if (buf[510] != 0x55 && buf[511] != 0xAA) {
        setdoserror(DOS_ENONDOS);
        return DOS_ERR;
    }

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

    if (i == DISK_OK) {
        memset(&boot,0,sizeof(struct BOOT));
        memcpy(&boot,buf,sizeof(struct BOOT));
        memcpy(&dp->sec_size,&boot.sec_size,sizeof(struct BPB));
    }

    return i;
}
