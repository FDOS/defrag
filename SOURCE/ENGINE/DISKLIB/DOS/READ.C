/*
 * dos\read.c       FAT12/16/32 disk read functions
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <dos.h>        /* REGS, SREGS, etc. */

#include "dosio.h"
#include "debug.h"
#include "disklib.h"

enum { DISK_READ = 0, DISK_WRITE };

/*
 * disk_read        absolute disk read (INT 25h)
 *
 * Note: This function works only on FAT12/16 drives less than
 *       32MB in size on all Microsoft Platforms.
 */

extern int disk_read(int disk, long sector, void *buffer, int nsecs)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;

    DBG_zero(buffer,nsecs*512);

    regs.x.ax = disk;
    regs.x.dx = (unsigned int)sector;      /* truncate is okay */
    regs.x.cx = nsecs;
    regs.x.bx = FP_OFF(buffer);
    sregs.ds = FP_SEG(buffer);

    DBG_reg_dump(&regs,&sregs);

    int86x(0x25,&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("read");
        setdosioerror(regs.h.al);       /* Set the DOS error code */
        return DOS_ERR;                 /*  for later retrieval */
    }

    return DISK_OK;
}

/*
 * disk_read_ext    absolute disk read (INT 25h), > 32MB
 *
 * Note: This function works only on FAT12/16 drives greater
 *       than 32MB in size on all Microsoft Platforms.
 *
 */

extern int disk_read_ext(int disk, long sector, void *buffer, int nsecs)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;
struct DCB Dcb DBG_0;
struct DCB *dcb = &Dcb;

    DBG_zero(buffer,nsecs*512);

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

    int86x(0x25,&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("read(ext)");
        setdosioerror(regs.h.al);       /* Set the DOS error code */
        return DOS_ERR;                 /*  for later retrieval */
    }

    return DISK_OK;
}

/*
 * disk_read_ioctl      INT 21h/440Dh/61h FAT12/16
 *
 * Note: This function fails in a strange way under MS-DOS 6.2 and 6.22:
 *       it will not read properly with sector numbers greater than 36.
 *
 *       This function works under Windows 95, 98 and NT, however.
 *       however, the head is off by 1.
 *
 *       See the accompanying program TESTIOCT.C.
 *
 */

extern int disk_read_ioctl(int disk, int track, int sec, int head, void *buf,
                           int nsecs)
{
int i;
struct RWBLOCK blk DBG_0;

    DBG_zero(buf,nsecs*512);

    blk.special = 0;
    blk.head = (unsigned short)head;
    blk.track = (unsigned short)track;
    blk.sector = (unsigned short)sec-1;     /* sector is relative to track */
    blk.nsecs = (unsigned short)nsecs;
    blk.buffer = buf;

    i = dos_ioctl(DOS_DEV_IOCTL,DOS_MINOR_READ_TRACK,disk,&blk);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_read_ioctl32    INT 21h/440Dh/61h FAT12/16/32
 *
 * Note: This function fails under MS-DOS.
 *
 *       This function works under Windows 95 and 98, however,
 *       the head is off by 1.
 *
 *       See the accompanying program TESTIOCT.C.
 *
 */

extern int disk_read_ioctl32(int disk, int track, int sec, int head, void *buf,
                             int nsecs)
{
int i;
struct RWBLOCK blk DBG_0;

    DBG_zero(buf,nsecs*512);

    blk.special = 0;
    blk.head = (unsigned short)head;
    blk.track = (unsigned short)track;
    blk.sector = (unsigned short)sec-1;     /* sector is relative to track */
    blk.nsecs = (unsigned short)nsecs;
    blk.buffer = buf;

    i = dos_ioctl32(DOS_DEV_IOCTL,DOS_MINOR_READ_TRACK,disk,&blk);

    if (i == DEVICE_OK)
        i = DISK_OK;

    return i;
}

/*
 * disk_read32      absolute disk read FAT12/16/32
 *
 * Note: This function will fail without setting the carry flag
 *       under MS-DOS and Windows NT 4.0 (and strangely does fill
 *       the buffer with something and sets AL to zero).
 *
 *       This function fails under Windows 95.
 *
 *       This function reads all drives under Windows 98.
 */

extern int disk_read32(int disk, long sec, void *buf, int nsecs)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;
struct DCB Dcb DBG_0;
struct DCB *dcb = &Dcb;

    DBG_zero(buf,nsecs*512);

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

    regs.x.si = DISK_READ;

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
