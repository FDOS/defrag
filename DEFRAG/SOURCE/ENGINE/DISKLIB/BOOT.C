/*
 * boot.c       read and display boot sector
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 14-Dec-1998  greggj  fixed casts where offsetof is used
 * 26-Nov-1998  read type macros; display() correctly identifies FAT32
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "dosio.h"
#include "disklib.h"

/* Define these one at a time to see which one works: */

                    /*  DOS     WIN95   WIN98   WINNT */

#define INT25   1   /*  fh              f?            */
#define INT7305 0   /*                  fh            */
#define IOCTL   0   /*                                */
#define IOCTL32 0   /*                                */
#define BIOS    0   /*                  f             */

/*
    Notes:  ?   I have not tested WIN98 & FAT16 HDDs.

*/

#define MB32    (1024L*1024L*32)

void display(char *b);
void boot_fat(struct BOOT *b);
void boot_fat32(struct BOOT_FAT32 *b);
void boot_ntfs(struct BOOT_NTFS *b);

int main(int argc, char **argv)
{
#if INT25
long size;
#endif
int i,disk;
char buf[512];

    if (argc == 2)
        disk = atoi(argv[1]);
    else
        disk = get_drive();

    printf("\nDrive %c:\n",disk+'A');

#ifndef _WINNT                          /* if NOT Windows NT there are */
                                        /*  five ways to (try to) read */
#if INT25

    i = drive_size(disk,&size);         /* not correct if drive > 2.1G */
                                        /*  but okay here */
    if (i != DISK_OK) {
        lib_error("drive",i);
        return 1;
    }

    if (size > MB32)
        i = disk_read_ext(disk,0,buf,1);
    else
        i = disk_read(disk,0,buf,1);
#endif

#if INT7305
    i = disk_read32(disk,0,buf,1);
#endif

#if IOCTL
    if (disk < 2)
        i = disk_read_ioctl(disk,0,1,0,buf,1);
    else
        i = disk_read_ioctl(disk,0,1,1,buf,1);
#endif

#if IOCTL32
    if (disk < 2)
        i = disk_read_ioctl32(disk,0,1,0,buf,1);
    else
        i = disk_read_ioctl32(disk,0,1,1,buf,1);
#endif

#if BIOS
    if (disk < 2)
        i = disk_read_p(disk,0,1,0,buf,1);
    else
        i = disk_read_p(disk,0,1,1,buf,1);

#endif

#else /* _WINNT */

    i = disk_read(disk,0,buf,1);

#endif

    if (i != DISK_OK) {
        lib_error("read",i);
        return 1;
    }

    display(buf);

    return 0;
}

void display(char *buf)
{
    printf("Boot Sector\n\n");

    /*
        DOS/WIN32 "OEM name" can be anything...
    */

    if (strncmp(buf+3,"NTFS",4) == 0) {
        boot_ntfs((struct BOOT_NTFS *)buf);
    }
    else if (*(UINT16*)(buf+offsetof(struct BOOT,secs_fat)) == 0) {
        boot_fat32((struct BOOT_FAT32 *)buf);
    }
    else {
        boot_fat((struct BOOT *)buf);
    }

}

void boot_fat(struct BOOT *b)
{
    printf("FAT12/16\n\n");
    printf("OEM name:                            %.8s\n",b->name);
    printf("bytes per sector:                    %u\n",b->sec_size);
    printf("sectors per allocation unit:         %u\n",b->secs_cluster);
    printf("reserved sectors:                    %u\n",b->reserved_secs);
    printf("number of FATs:                      %u\n",b->num_fats);
    printf("number of root dir entries:          %u\n",b->dir_entries);
    printf("number of sectors:                   %u\n",b->num_sectors);
    printf("media descriptor:                    %02xh\n",b->media_desc);
    printf("number of FAT sectors:               %u\n",b->secs_fat);
    printf("sectors per track:                   %u\n",b->secs_track);
    printf("number of heads:                     %u\n",b->num_heads);
    printf("number of hidden sectors:            %lu\n",b->hidden_sectors);

    /* Older formats have these... */
/*
    printf("high order number of hidden sectors: %u\n",b->large_sectors);
    printf("number of logical sectors:           %u\n",b->total_sectors);
*/
    /* and do not have these... */

    printf("number of total sectors:             %lu\n",b->total_sectors);
    printf("drive number                         %02xh\n",b->drive_number);
    printf("signature                            %02xh\n",b->signature);
    printf("volume id                            %lx (%04X-%04X)\n",b->volume_id,(int)(b->volume_id>>16),(unsigned int)(unsigned short)b->volume_id);
    printf("volume label                         %.11s\n",b->volume_label);
    printf("file system                          %.8s\n",b->file_system);
}

void boot_fat32(struct BOOT_FAT32 *b)
{
    printf("FAT32\n\n");
    printf("OEM name:                            %.8s\n",b->name);
    printf("bytes per sector:                    %u\n",b->sec_size);
    printf("sectors per allocation unit:         %u\n",b->secs_cluster);
    printf("reserved sectors:                    %u\n",b->reserved_secs);
    printf("number of FATs:                      %u\n",b->num_fats);
    printf("number of root dir entries:          %u\n",b->dir_entries);
    printf("number of sectors:                   %u\n",b->num_sectors);
    printf("media descriptor:                    %02xh\n",b->media_desc);
    printf("number of FAT sectors:               %u\n",b->secs_fat);
    printf("sectors per track:                   %u\n",b->secs_track);
    printf("number of heads:                     %u\n",b->num_heads);
    printf("number of hidden sectors:            %lu\n",b->hidden_sectors);
    printf("number of total sectors:             %lu\n",b->total_sectors);
    printf("sectors per FAT:                     %lu\n",b->sectors_fat);
    printf("flags:                               %xh\n",b->flags);
    printf("version number:                      %xh\n",b->fs_version);
    printf("cluster number of root:              %lu\n",b->root_cluster);
    printf("information sector:                  %u\n",b->info_sec);
    printf("backup boot sector sector:           %u\n",b->boot_sec);
    printf("drive number:                        %02xh\n",b->drive_number);
    printf("signature:                           %02xh\n",b->signature);
    printf("volume id:                           %lx (%04X-%04X)\n",b->volume_id,(int)(b->volume_id>>16),(unsigned int)(unsigned short)b->volume_id);
    printf("volume label:                        %.11s\n",b->volume_label);
    printf("file system:                         %.8s\n",b->file_system);
}

void boot_ntfs(struct BOOT_NTFS *b)
{
    printf("NTFS\n\n");
    printf("OEM name:                            %.8s\n",b->name);
    printf("bytes per sector:                    %u\n",b->sec_size);
    printf("sectors per allocation unit:         %u\n",b->secs_cluster);
    printf("media descriptor:                    %02xh\n",b->media_desc);
    printf("sectors per track:                   %u\n",b->secs_track);
    printf("number of heads:                     %u\n",b->num_heads);
    printf("drive_number                         %02xh\n",b->drive_number);
    printf("signature                            %02xh\n",b->signature);
    printf("number of sectors (lo):              %lu\n",b->num_secs_lo);
    printf("number of sectors (hi):              %lu\n",b->num_secs_hi);
    printf("MFT cluster (lo):                    %lu\n",b->mft_clus_lo);
    printf("MFT cluster (hi):                    %lu\n",b->mft_clus_hi);
    printf("MFT2 cluster (lo):                   %lu\n",b->mft2_clus_lo);
    printf("MFT2 cluster (hi):                   %lu\n",b->mft2_clus_hi);
    printf("MFT record size:                     %lu\n",b->rec_size);
    printf("index buffer size:                   %lu\n",b->buf_size);
    printf("volume id                            %lx (%04X-%04X)\n",b->volume_id,
           (int)(b->volume_id>>16),(unsigned int)(unsigned short)b->volume_id);
}
