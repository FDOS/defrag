/*
 * testioct.c   test the MS-DOS/Windows IOCTL Read Track function
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 14-Dec-1998  greggj  fixed casts where offsetof is used
 * 26-Nov-1998  added more comments; lib_ver() was os_ver()
 * 04-Nov-1998  added more comments
 *
 */

/*
 * Note: Under MS-DOS the IOCTL Read function starts off at the
 *       start of the partition, which is C 0, H 0, S 1.
 *
 *       Under Windows 95 the IOCTL Read function starts off at the
 *       BOOT sector WITH THE ARGUMENTS OF C 0, H 0, S 1!
 *
 *       IOCTL fails in a strange way under MS-DOS 6.2 and 6.22: it
 *       will not read properly with sector numbers greater than 36.
 *
 *       IOCTL fails under Windows 98.
 *
 *       IOCTL is not supported by Windows NT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "dosio.h"
#include "disklib.h"

void usage(void);
void hexdump(unsigned char *b, unsigned int n);
void asciidump(unsigned char *b, unsigned int n);
void display(char *b);
void boot_fat(struct BOOT *b);
void boot_fat32(struct BOOT_FAT32 *b);
void boot_ntfs(struct BOOT_NTFS *b);

int main(int argc, char **argv)
{
char *buf;
int i,t,s,h;
int disk,nsecs;
unsigned int j;                     /* loop variable */

    if (argc == 2)
        disk = atoi(argv[1]);
    else
        usage();

    printf("\nDrive %c:\n\n",disk+'A');

    i = disk_get_logical(disk,&t,&nsecs,&h);
    if (i != DISK_OK)
    {
        lib_error("testioct - get",i);
        printf("\n");
        abort();
    }

    if (nsecs > 127 && lib_ver() == LIB_DOS) {
        printf("too many sectors/track to read entire track; adjusting down\n");
        nsecs = 127;
    }

    if ((buf = malloc(nsecs * 512)) == NULL) {
        printf("could not get enough memory to read track; skipping\n");
        if ((buf = malloc(512)) == NULL) {
            printf("could not get 512 bytes!\n");
            printf("\n");
            abort();
        }
        goto other_tests;
    }

    memset(buf,0,nsecs*512);

    t = 0;      /* first track */
    s = 1;      /* (might want to make head 1 if first track) */
    h = 0;      /* (really has all zeros after the first sector) */


    printf("Reading First Physical Track: t%d,s%d,h%d (%d sectors)\n\n",t,s,h,nsecs);
    i = disk_read_ioctl(disk,t,s,h,buf,nsecs);
    if (i != DISK_OK)
    {
        lib_error("testioct - read",i);
        printf("\n");
        abort();
    }

    for (j = 0; j < nsecs * 512; j+= 16)
    {
        hexdump(buf+j,16);
        asciidump(buf+j,16);
        printf("\n");
    }

other_tests:

    t = 0;      /* boot sector */
    s = 1;
    h = (disk < 2) ? 0 : 1;

    printf("\nReading Boot Sector: t%d,s%d,h%d\n\n",t,s,h);
    i = disk_read_ioctl(disk,t,s,h,buf,1);
    if (i != DISK_OK)
    {
        lib_error("testioct - read",i);
        printf("\n");
        abort();
    }
    display(buf);

    printf("\nSingle Steps\n");
    for (s = 1; s < nsecs; s++)
    {
        printf("\nt%d,s%d,h%d ",t,s,h);
        i = disk_read_ioctl(disk,t,s,h,buf,1);
        if (i != DISK_OK)
        {
            lib_error("testioct - read",i);
            break;
        }
        hexdump(buf,16);
        asciidump(buf,16);
    }

    printf("\n");
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

void display(char *buf)
{
    if (strncmp(buf+3,"NTFS",4) == 0)
        boot_ntfs((struct BOOT_NTFS *)buf);
    else if (*(UINT16*)(buf + offsetof(struct BOOT,secs_fat)) == 0)
        boot_fat32((struct BOOT_FAT32 *)buf);
    else
        boot_fat((struct BOOT *)buf);

    /*
        DOS "OEM name" can be anything...
    */

}

void boot_fat(struct BOOT *b)
{
    printf("OEM name:                            %.8s\n",b->name);
    printf("bytes per sector:                    %u\n",b->sec_size);
    printf("sectors per allocation unit:         %u\n",b->secs_cluster);
    printf("reserved sectors:                    %u\n",b->reserved_secs);
    printf("number of FATs:                      %u\n",b->num_fats);
    printf("number of root dir entries:          %u\n",b->dir_entries);
    printf("number of sectors in logical image:  %u\n",b->num_sectors);
    printf("media descriptor:                    %02x\n",b->media_desc);
    printf("number of FAT sectors:               %u\n",b->secs_fat);
    printf("sectors per track:                   %u\n",b->secs_track);
    printf("number of heads:                     %u\n",b->num_heads);
    printf("number of hidden sectors:            %lu\n",b->hidden_sectors);
/*
    printf("high order number of hidden sectors: %u\n",b->large_sectors);
    printf("number of logical sectors:           %u\n",b->total_sectors);
*/
    printf("number of total sectors:             %lu\n",b->total_sectors);
    printf("drive number                         %02x\n",b->drive_number);
    printf("signature                            %02x\n",b->signature);
    printf("volume id                            %lx (%04X-%04X)\n",b->volume_id,(int)(b->volume_id>>16),(unsigned int)(unsigned short)b->volume_id);
    printf("volume label                         %.11s\n",b->volume_label);
    printf("file system                          %.8s\n",b->file_system);
}

void boot_fat32(struct BOOT_FAT32 *b)
{
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
    printf("OEM name:                            %.8s\n",b->name);
    printf("bytes per sector:                    %u\n",b->sec_size);
    printf("sectors per allocation unit:         %u\n",b->secs_cluster);
    printf("media descriptor:                    %02x\n",b->media_desc);
    printf("sectors per track:                   %u\n",b->secs_track);
    printf("number of heads:                     %u\n",b->num_heads);
    printf("drive_number                         %02x\n",b->drive_number);
    printf("signature                            %02x\n",b->signature);
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

void usage(void)
{
    printf("\nTESTIOCT <0-5>\n");
    printf("\nTests the INT 21h Track Read IOCTL function. Reads and dumps the");
    printf("\nfirst physical track, reads and displays the BOOT sector, reads");
    printf("\nand dumps first 16 bytes of a consequtive number of sectors.");
    printf("\n\nSee the source code for more information.\n");
    exit(1);
}
