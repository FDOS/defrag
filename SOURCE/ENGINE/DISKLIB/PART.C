/*
 * part.c       partition identification strings (not complete)
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>      /* NULL */

struct PARTITION_TYPE {
    int type;
    char *name;
    char *desc;
};

static struct PARTITION_TYPE part_types[] = {
    { 0x01, "FAT", "DOS FAT-12" },
    { 0x04, "FAT", "DOS FAT-16" },
    { 0x05, "FAT", "DOS Extended" },
    { 0x06, "FAT", "DOS FAT-16" },
    { 0x07, "NTFS", "Windows NT - NTFS" },
    { 0x08, "HPFS", "OS/2 (v1.0-1.3 only)" },
    { 0x0A, "OBM", "OS/2 Boot Manager" },
    { 0x0B, "FAT", "Windows 95 FAT-32" },
    { 0x0C, "FAT", "Windows 95 FAT-32 (LBA)" },
    { 0x0E, "VFAT", "LBA VFAT (BIGDOS/FAT16)" },
    { 0x0F, "VFAT", "LBA VFAT (DOS Extended)" },
    { 0x11, "FAT", "Hidden DOS FAT-12" },
    { 0x12, "COMP", "Compaq Diagnostics" },
    { 0x14, "FAT", "Hidden DOS FAT-16 (32M)" },
    { 0x16, "FAT", "Hidden DOS FAT-16 (<2G)" },
    { 0x17, "NTFS", "Hidden Windows NT NTFS" },
    { 0x18, "AST", "AST Windows swap file" },
    { 0x1B, "FAT", "Hidden Windows95 FAT-32" },
    { 0x1C, "FAT", "Hidden LBA FAT-32" },
    { 0x1E, "FAT", "Hidden LBA DOS FAT-16" },
    { 0x1F, "FAT", "Hidden LBA DOS Extended" },
    { 0x24, "FAT", "NEC MS-DOS 3.x" },
    { 0x3C, "PMR", "PartitionMagic recovery" },
    { 0x50, "ODM", "OnTrack Disk Mgr, R/O" },
    { 0x51, "NOV", "NOVEL" },
    { 0x52, "CPM", "CP/M" },
    { 0x53, "ODM", "OnTrack Disk Mgr, W/O?" },
    { 0x54, "ODM", "OnTrack Disk Mgr (DDO)" },
    { 0x55, "EZD", "EZ-Drive" },
    { 0x80, "MINIX", "Minix v1.1 - 1.4a" },
    { 0x81, "MINIX", "Minix v1.4b+" },
    { 0x82, "LINUX", "Linux Swap partition" },
    { 0x83, "LINUX", "Linux - EXT2FS/XIAFS" },
    { 0x84, "OS/2", "OS/2-renumbered FAT-16" },
    { 0x85, "LINUX", "Linux Extended" },
    { 0x86, "FAT", "FAT16 volume/stripe set" },
    { 0x87, "NTFS", "NTFS volume/stripe set" },
    { 0xA5, "FBSD", "FreeBSD, BSD/386" },
    { 0xB7, "BSDI", "BSDI secondarily swap" },
    { 0xB8, "BSDI", "BSDI swap partition" },
    { 0xC1, "FAT", "DR DOS 6 secured FAT-12" },
    { 0xC4, "FAT", "DR DOS 6 secured FAT-16" },
    { 0xC6, "FAT", "DR DOS 6 secured Huge" },
    { 0xC6, "FAT", "Corrupted FAT16 (WinNT)" },
    { 0xC7, "NTFS", "Corrupted NTFS (WinNt)" },
    { 0xD8, "CP/M", "CP/M-86" },
    { 0xDB, "CP/M", "CP/M, Concurrent DOS" },
    { 0xE1, "FAT", "SpeedStor ext FAT-12" },
    { 0xE3, "DOS", "DOS read-only" },
    { 0xEB, "BEOS", "BeOS" },
    { 0xF2, "DOS", "DOS 3.3+ secondary" },
    { 0, "UNK", "Unknown/Unrecognized" }
};

struct PARTITION_TYPE *partition_type(int type)
{
struct PARTITION_TYPE *p = part_types;

    for (; p->type; p++) {
        if (p->type == type)
            break;
    }
    return p;
}
