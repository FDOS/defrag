/*
 * dos\write.c      FAT12/16/32 disk write
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <dos.h>

#include "dosio.h"
#include "debug.h"
#include "disklib.h"

enum { DISK_READ = 0, DISK_WRITE };

/*
 * disk_write       absolute disk write (INT 26h)
 *
 */

extern int disk_write(int disk, long sector, void *buffer, int nsecs)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;

    regs.x.ax = disk;
    regs.x.dx = (unsigned int)sector;      /* truncate is okay */
    regs.x.cx = nsecs;
    regs.x.bx = FP_OFF(buffer);
    sregs.ds = FP_SEG(buffer);

    DBG_reg_dump(&regs,&sregs);

    int86x(0x26,&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("write");
        setdosioerror(regs.h.al);       /* Set the DOS error code */
        return DOS_ERR;                 /*  for later retrieval */
    }

    return DISK_OK;
}

/*
 * disk_write_ext   absolute disk write (INT 26h), > 32MB
 *
 * Note: For hard disks, Windows 95 will trap this function and ask
 *       to ignore the write or abort the program. If ignored the
 *       write fails with a Write Protected error. But if the buffer
 *       data is the same as the sector data this function will
 *       appear to be successful.
 */

extern int disk_write_ext(int disk, long sector, void *buffer, int nsecs)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;
struct DCB Dcb DBG_0;
struct DCB *dcb = &Dcb;

    /*
        Microsoft bombs if the address of operator is used in the
        FP_OFF() or FP_SEG() macros so a pointer is kludged here so
        there is no need for prepocessor conditionals for MS.
    */

    regs.x.ax = disk;
    dcb->sector = sector;
    dcb->number = (unsigned short)nsecs;
    dcb->buffer = buffer;
    regs.x.cx = 0xffff;
    regs.x.bx = FP_OFF(dcb);
    sregs.ds = FP_SEG(dcb);

    DBG_reg_dump(&regs,&sregs);

    int86x(0x26,&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("write(ext)");
        setdosioerror(regs.h.al);       /* Set the DOS error code */
        return DOS_ERR;                 /*  for later retrieval */
    }

    return DISK_OK;
}

/*
 * disk_write_ioctl     INT 21h/440Dh/41h
 *
 * NOT YET TESTED
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

    i = dos_ioctl(DOS_DEV_IOCTL,DOS_MINOR_WRITE_TRACK,disk,&blk);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_write_ioctl32   INT 21h/440Dh/41h FAT12/16/32
 *
 * Note: This function fails under MS-DOS.
 *
 *       This function works under Windows 95 and 98, however,
 *       the head is off by 1.
 *
 *       See the accompanying program TESTIOCT.C.
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
    blk.sector = (unsigned short)sec-1;     /* sector is relative to track */
    blk.nsecs = (unsigned short)nsecs;
    blk.buffer = buf;

    i = dos_ioctl32(DOS_DEV_IOCTL,DOS_MINOR_WRITE_TRACK,disk,&blk);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}


/*
 * disk_write32     absolute disk write FAT12/16/32
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
 * Note: This function will fail without setting the carry flag
 *       under MS-DOS and Windows NT 4.0 (and strangely does fill
 *       the buffer with something and sets AL to zero).
 *
 *       This function fails under Windows 95.
 *
 *       This function reads all drives under Windows 98.
 *
 */

extern int disk_write32(int disk, long sec, void *buf, int nsecs)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;
struct DCB Dcb DBG_0;
struct DCB *dcb = &Dcb;

    /*
        Microsoft bombs if the address of operator is used in the
        FP_OFF() or FP_SEG() macros so a pointer is kludged here so
        there is no need for prepocessor conditionals for MS.
    */

    dcb->sector = sec;
    dcb->number = (unsigned short)nsecs;
    dcb->buffer = buf;
    regs.x.ax = DOS_EXT_ABS_READ_WRITE;
    regs.x.dx = disk+1;
    regs.x.cx = (unsigned short)-1;
    regs.x.bx = FP_OFF(dcb);
    sregs.ds = FP_SEG(dcb);

    /* dangerous function! SI:0 MUST BE 0 for READ, 1 for WRITE */

    regs.x.si = DISK_WRITE;

    /* possible enhancement: properly support the write mode bits */

    DBG_reg_dump(&regs,&sregs);

    intdosx(&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.ax == 0x7300) {          /* fail indication DOS/NT */
        regs.x.cflag = 1;
        setdoserror(1);
    }

    if (regs.x.cflag) {
        DBG_err_dump("read(32)");
        return DOS_ERR;
    }

    return DISK_OK;
}
