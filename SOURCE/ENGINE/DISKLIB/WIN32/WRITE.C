/*
 * win32\write.c    absolute disk write
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

enum { DISK_READ, DISK_WRITE };

/*
 * disk_write       absolute disk write (INT 26h),  <= 32MB
 *
 */

extern int disk_write(int disk, long sec, void *buf, int nsecs)
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
        return regs.h.al;

    return DISK_OK;
}

/*
 * disk_write_ext   absolute disk write (INT 26h), > 32MB
 *
 */

extern int disk_write_ext(int disk, long sec, void *buf, int nsecs)
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
        return regs.h.al;

    return DISK_OK;
}

/*
 * disk_write   absolute disk write FAT32
 *
 * When bit 0 of SI is set (that is, WRITE mode), bits 13, 14 and 15 of
 * SI categorize what type of data is being written:
 *
 * 15  14  13  Description
 *
 * 0   0   0   Other/Unknown.
 * 0   0   1   FAT data.
 * 0   1   0   Directory data.
 * 0   1   1   Normal File data.
 * 1   x   x   Reserved. Bit 15 must be 0.
 *
 * All other bits of SI (1 through 12) are reserved and must be 0.
 *
 */

extern int disk_write32(int disk, long sec, void *buf, int nsecs)
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

    regs.d.esi = DISK_WRITE;

    /* possible enhancement: properly support the write mode bits */

    /*
        NOTE: Microsoft online help states that the device I/O
        control code should be VWIN32_DIOC_DOS_IOCTL and states
        that that value is 6. This is wrong in that the value of
        6 is for VWIN32_DIOC_DOS_DRIVEINFO (...IOCTL is 1).
    */

    if ((i = win_device_io(VWIN32_DIOC_DOS_DRIVEINFO,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 0x0001)
        return regs.h.al;

    return DISK_OK;
}

/*
 * disk_write_ioctl     INT 21h/440Dh/41h
 *
 * Note: This function, and disk_read_ioctl, will fail under
 *       Windows 95 OSR2 (it's a bug in Windows 95; see Microsoft
 *       Knowledge Base Article # Q174569).
 *
 * Also: This function also works on FAT32 (or should).
 *
 */

extern int disk_write_ioctl(int disk, int track, int sec, int head, void *buf,
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

    i = win_ioctl_ext(DOS_MINOR_WRITE_TRACK,disk+1,&blk);

    if (i == DEVICE_OK)             /* translate the OK number */
        i = DISK_OK;

    /* driver failed above call, try older version */

    if (i == DOS_ERR) {
        if ((i = win_ioctl(DOS_MINOR_WRITE_TRACK,disk+1,&blk)) == DEVICE_OK)
            i = DISK_OK;
    }

    return i;
}


/*
 * disk_write_ioctl     INT 21h/440Dh/41h
 *
 * Note: This function, and disk_read_ioctl, will fail under
 *       Windows 95 OSR2 (it's a bug in Windows 95; see Microsoft
 *       Knowledge Base Article # Q174569).
 *
 * Also: This function also works on FAT32 (or should).
 *
 */

extern int disk_write_ioctl32(int disk, int track, int sec, int head, void *buf,
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

    i = win_ioctl_ext(DOS_MINOR_WRITE_TRACK,disk+1,&blk);

    if (i == DEVICE_OK)             /* translate the OK number */
        i = DISK_OK;

    return i;
}
