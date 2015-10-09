/*
 * win32\winnt\getdisk.c    get disk parameters
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 20-Dec-1998  greggj  fixed byte count for reading Media Descriptor
 * 12-Nov-1998  greggj  added better media type checking in boot_...()
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win32.h"
#include "dosio.h"
#include "disklib.h"

static int boot_parameters(int disk, struct DEVICEPARAMS *dp);
static int boot_media_fat(int disk, struct MID *mid);
static int boot_setmedia_fat(int disk, struct MID *mid);
static int setserialnum(int disk, unsigned long serialnum);

/*
 * disk_get_parameters      get disk (DOS) parameters
 *
 * Notes: Although the DEVICEPARAMS structure is from MS-DOS
 *        the first 28h bytes, the Bios Parameter Block, are
 *        the same on FAT and NTFS (DOS member that have no
 *        equivalent under NTFS are zeroed).
 *
 */

extern int disk_getparams(int disk, struct DEVICEPARAMS *dp)
{
int i;

    memset(dp,0,sizeof(struct DEVICEPARAMS));

    /*
        There is no "Get Device Parameters" API call on NT but
        enough of the equivalent data resides in the BOOT sector.
    */

    if ((i = boot_parameters(disk,dp)) != DISK_OK)
        return i;

    /*
        But not all DEVICEPARAMS members are in the BOOT sector.
        cylinders can be calculated, others are left as 0 as they
        are rarely used.
    */

    dp->cylinders = max_track(dp) + 1;

    return DISK_OK;
}

/*
 * disk_getmedia    get MID structure
 *
 * Notes: Although the DEVICEPARAMS structure is from MS-DOS
 *        almost all of it can found on an NTFS system.
 */

extern int disk_getmedia(int disk, struct MID *mid)
{
char filesys[9];
char vollabel[13];
char buf[4] = "C:\\";
unsigned long flen,flgs,sn;     /* (only sn used) */

    memset(mid,0,sizeof(struct MID));

    /*
        First get volume label and file system name. Note that this
        API call will fail if the volume label string length parameter
        is less than 12 -- this could mean that the length parameters
        do not include the terminating nul; the WIN32.HLP help file
        does not say.
    */

    buf[0] = (char)(disk+'A');
    if (GetVolumeInformation(buf,vollabel,12L,&sn,&flen,&flgs,filesys,8L) == 0)
        return DOS_ERR;

    /*
        For FAT disks, the information from GetVolumeInformation
        does not correspond to what is on the disk, so off the disk
        it is...
    */

    if (strcmp(filesys,"FAT") == 0) {
        if (boot_media_fat(disk,mid) == DISK_OK)
            return DISK_OK;
        /* fall through if error */
    }

    /*
        The MID structure is a bit limited, think of this function
        as sort of backward compatible.
    */

    mid->serialnum = sn;
    memset(mid->vollabel,' ',11);
    memset(mid->filesys,' ',8);
    memcpy(mid->vollabel,vollabel,strlen(vollabel));
    memcpy(mid->filesys,filesys,strlen(filesys));

    return DISK_OK;
}

/*
 * disk_setmedia    set MID structure
 *
 * Notes: Although the DEVICEPARAMS structure is from MS-DOS
 *        almost all of it can found on an NTFS system.
 */

extern int disk_setmedia(int disk, struct MID *mid)
{
int i;
char filesys[9];
char vollabel[12];
char buf[4] = "C:\\";

    buf[0] = (char)(disk+'A');
    if (GetVolumeInformation(buf,NULL,0,NULL,NULL,NULL,filesys,8L) == 0)
        return DOS_ERR;

    if (strcmp(filesys,"FAT") == 0) {
        if ((i = boot_setmedia_fat(disk,mid)) != DISK_OK)
            return i;
        return DISK_OK;
    }

#if 0
    /*
    NTFS volumes do not store a volume label in the BOOT sector.
    (And the volume label in the BOOT sector of a FAT volume is
    not the same as the one written by the LABEL command.) Since
    the MID structure and the functions here are about the BOOT
    sector, setting the volume label here is not quite right.
    */
    memcpy(vollabel,mid->vollabel,11);
    vollabel[11] = '\0';
    if (SetVolumeLabel(buf,vollabel) == 0)
        return DOS_ERR;
#endif

    /*
        There appears to be no API call for setting the disk
        serial number, but it is in the BOOT sector. Note that
        the GetVolumeInformation() function will not return
        the new serial number unless the disk is unmounted first.
        (There is no provision for mounting and unmounting in
        this library.)
    */

    if (strcmp(filesys,"NTFS") == 0) {
        if ((i = setserialnum(disk,mid->serialnum)) != DISK_OK)
            return i;
    }

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

#if 0

/*
   Even though the following opens the drive as logical, i.e.
   "\\.\C:", IOCTL_DISK_GET_DRIVE_GEOMETRY returns physical
   parameters (which are incorrect for our purpose here).
*/

int i;
HANDLE dev;
unsigned long bc;
DISK_GEOMETRY geo;

    dev = opendrive(disk);
    if (dev == INVALID_HANDLE_VALUE)
        return DEVICE_ERR;

    i = DeviceIoControl(dev,IOCTL_DISK_GET_DRIVE_GEOMETRY,NULL,0,
                        &geo,sizeof(DISK_GEOMETRY),&bc,NULL);
    if (i == FALSE) {
        CloseHandle(dev);
        return DEVICE_ERR;
    }

    if (t) *t = geo.Cylinders.LowPart;
    if (h) *h = geo.TracksPerCylinder;
    if (s) *s = geo.SectorsPerTrack;

    closedrive(dev);
    return DISK_OK;

#endif
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


/*
 * setserialnum     set NTFS disk serial number
 *
 */

static int setserialnum(int disk, unsigned long serialnum)
{
int i;
char buf[512];
unsigned short tmp;
struct BOOT_NTFS *boot;

    /* read sector, change serial number, write sector */

    memset(buf,0,512);
    i = disk_read(disk,0,buf,1);

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

    if (i == DISK_OK) {
        boot = (struct BOOT_NTFS *)buf;
        boot->volume_id = serialnum;
        i = disk_write(disk,0,buf,1);
    }

    return i;
}

/*
 * boot_parameters      get disk parameters from BOOT
 *
 * Notes: Although the DEVICEPARAMS structure is from MS-DOS
 *        the first 28h bytes, the Bios Parameter Block, are
 *        the same on FAT and NTFS.
 */

static int boot_parameters(int disk, struct DEVICEPARAMS *dp)
{
int i;
char buf[512];
struct BOOT boot;
unsigned short tmp;

    /* read sector, extract parameters */

    memset(buf,0,512);
    i = disk_read(disk,0,buf,1);

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

    if (i == DISK_OK) {
        memset(&boot,0,sizeof(struct BOOT));
        memcpy(&boot,buf,sizeof(struct BOOT));
        memcpy(&dp->sec_size,&boot.sec_size,sizeof(struct BPB));
    }

    return i;
}

/*
 * boot_media_fat   get FAT disk media id structure from BOOT
 *
 */

static int boot_media_fat(int disk, struct MID *mid)
{
int i;
char buf[512];
struct BOOT boot;
unsigned short tmp;

    /* read sector, extract parameters */

    memset(buf,0,512);
    i = disk_read(disk,0,buf,1);

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

    if (i == DISK_OK) {
        memset(&boot,0,sizeof(struct BOOT));
        memcpy(&boot,buf,sizeof(struct BOOT));
        memcpy(&mid->serialnum,&boot.volume_id,
                sizeof(struct MID) - sizeof(mid->infolevel));
    }

    return i;
}

/*
 * boot_setmedia_fat    set FAT disk media id structure in BOOT
 *
 */

static int boot_setmedia_fat(int disk, struct MID *mid)
{
int i;
char buf[512];
struct BOOT *boot;
unsigned short tmp;

    /* read sector, change MID info, write sector */

    memset(buf,0,512);
    i = disk_read(disk,0,buf,1);

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

    if (i == DISK_OK) {
        boot = (struct BOOT *)buf;
        memcpy(&boot->volume_id,&mid->serialnum,
                sizeof(struct MID) - sizeof(mid->infolevel));
        i = disk_write(disk,0,buf,1);
    }

    return i;
}
