/*
 * win32\read.c     absolute disk read
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include "win32.h"
#include "dosio.h"
#include "debug.h"

enum { DISK_READ = 0, DISK_WRITE };

/*
 * disk_read        absolute disk read (INT 25h)    <= 32MB
 *
 * Note: This function reads floppies under Windows 98.
 *
 *       This function fails under Windows NT.
 */

extern int disk_read(int disk, long sec, void *buf, int nsecs)
{
int i;
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = disk;
    regs.d.edx = sec;              /* (truncate is okay) */
    regs.d.ecx = nsecs;
    regs.d.ebx = (DWORD)buf;

    if ((i = win_device_io(VWIN32_DIOC_DOS_INT25,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 0x0001)
        return DOS_ERR;

    return DISK_OK;
}

/*
 * disk_read_ext    absolute disk read (INT 25h), > 32MB
 *
 * Note: I have not tested this function under Windows 98.
 * GROTS: do the test
 *
 *       This function fails under Windows NT.
 *
 */

extern int disk_read_ext(int disk, long sec, void *buf, int nsecs)
{
int i;
struct DCB dcb DBG_0;
DIOC_REGISTERS regs DBG_0;

    dcb.sector = sec;
    dcb.number = (UINT16)nsecs;
    dcb.buffer = buf;
    regs.d.eax = disk;
    regs.d.ecx = 0xFFFF;
    regs.d.ebx = (DWORD)&dcb;

    if ((i = win_device_io(VWIN32_DIOC_DOS_INT25,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 0x0001)
        return DOS_ERR;

    return DISK_OK;
}

/*
 * disk_read_ioctl      INT 21h/440Dh/61h
 *
 * Note: This function, and disk_write_ioctl, will fail under
 *       Windows 95 OSR2 (it's a bug in Windows 95; see Microsoft
 *       Knowledge Base Article # Q174569).
 *
 *       However, INT 21h/440Dh/61h works fine with an MS-DOS
 *       program under Windows 95.
 *
 *       This function fails under Windows 98 and Windows NT.
 */

extern int disk_read_ioctl(int disk, int track, int sec, int head, void *buf,
                           int nsecs)
{
int i;
struct RWBLOCK blk DBG_0;

    blk.special = 0;
    blk.head = (unsigned short)head;
    blk.track = (unsigned short)track;
    blk.sector = (unsigned short)(sec - 1);
    blk.nsecs = (unsigned short)nsecs;
    blk.buffer = buf;

    /* try newer version first (FAT12, FAT16 and FAT32 support - maybe) */

    if ((i = win_ioctl_ext(DOS_MINOR_READ_TRACK,disk+1,&blk)) != DEVICE_OK)
        i = win_ioctl(DOS_MINOR_READ_TRACK,disk+1,&blk);

    /*
        TODO: Might want to catch certain errors before the retry -
        Invalid Drive perhaps. But some errors such as Invalid Function
        means that the older version must be tried.
    */

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_read_ioctl32      INT 21h/440Dh/61h
 *
 * Note: This function, and disk_write_ioctl, will fail under
 *       Windows 95 OSR2 (it's a bug in Windows 95; see Microsoft
 *       Knowledge Base Article # Q174569).
 *
 *       However, INT 21h/440Dh/61h works fine with an MS-DOS
 *       program under Windows 95.
 *
 *       This function fails under Windows 98 and Windows NT.
 */

extern int disk_read_ioctl32(int disk, int track, int sec, int head, void *buf,
                             int nsecs)
{
int i;
struct RWBLOCK blk DBG_0;

    blk.special = 0;
    blk.head = (unsigned short)head;
    blk.track = (unsigned short)track;
    blk.sector = (unsigned short)(sec - 1);
    blk.nsecs = (unsigned short)nsecs;
    blk.buffer = buf;

    /* try newer version first (FAT12, FAT16 and FAT32 support - maybe) */

    i = win_ioctl_ext(DOS_MINOR_READ_TRACK,disk+1,&blk);

    /*
        TODO: Might want to catch certain errors before the retry -
        Invalid Drive perhaps. But some errors such as Invalid Function
        means that the older version must be tried.
    */

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_read32      absolute disk read FAT32
 *
 */

extern int disk_read32(int disk, long sec, void *buf, int nsecs)
{
int i;
struct DCB dcb DBG_0;
DIOC_REGISTERS regs DBG_0;

    dcb.sector = sec;
    dcb.number = (UINT16)nsecs;
    dcb.buffer = buf;
    regs.d.eax = DOS_EXT_ABS_READ_WRITE;
    regs.d.edx = disk+1;
    regs.d.ecx = (DWORD)-1;
    regs.d.ebx = (DWORD)&dcb;

    /* dangerous function! SI:0 MUST BE 0 for READ, 1 for WRITE */

    regs.d.esi = DISK_READ;

    /*
        NOTE: Microsoft online help states that the device I/O
        control code should be VWIN32_DIOC_DOS_IOCTL and states
        that that value is 6. This is wrong in that the value of
        6 is for VWIN32_DIOC_DOS_DRIVEINFO (...IOCTL is 1).
    */

    if ((i = win_device_io(VWIN32_DIOC_DOS_DRIVEINFO,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 0x0001) {
        return DOS_ERR;
    }

    return DISK_OK;
}
