/*
 * dos\gcc\read.c       absolute disk read
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include "dosio.h"
#include "debug.h"
#include "dosmem.h"
#include "disklib.h"

#define secsize 512

/*
 * disk_read        absolute disk read (INT 25h)
 *
 */

extern int disk_read(int disk, long sector, void *buffer, int nsecs)
{
__dpmi_regs regs;
int seg,sel;

    if ((seg = _dosmalloc(secsize*nsecs,&sel)) == -1)
        return MEM_ERR;

    regs.x.ax = disk;
    regs.x.dx = sector;
    regs.x.cx = nsecs;
    regs.x.ds = seg;
    regs.x.bx = 0;

    DBG_dpmi_dump(&regs);

    __dpmi_int(0x25,&regs);

    DBG_dpmi_dump(&regs);

    if (regs.x.flags & 1) {
        DBG_err_dump("read");
        _dosfree(sel);
        return regs.h.al;
    }

    _getdosmem(buffer,secsize*nsecs,sel);
    _dosfree(sel);

    return DISK_OK;
}

/*
 * disk_read_ext    absolute disk read (INT 25h), > 32MB
 *
 */

extern int disk_read_ext(int disk, long sector, void *buffer, int nsecs)
{
__dpmi_regs regs;
struct DCB dcb;
int seg,sel;
int bseg,bsel;

    if ((seg = dosmalloc(sizeof(struct DCB),&sel)) == -1)
        return MEM_ERR;

    if ((bseg = dosmalloc(secsize*nsecs,&bsel)) == -1) {
        dosfree(sel);
        return MEM_ERR;
    }

    regs.x.ax = disk;
    regs.x.cx = 0xffff;
    dcb.sector = sector;
    dcb.number = nsecs;
    dcb.buffer = _MK_FP(bseg);
    setdosmem((void *)&dcb,sizeof(struct DCB),sel);
    regs.x.ds = seg;
    regs.x.bx = 0;

    DBG_dpmi_dump(&regs);

    __dpmi_int(0x25,&regs);

    if (regs.x.flags & 1) {
        DBG_err_dump("read(ext)");
        dosfree(sel);
        dosfree(bsel);
        return regs.h.al;
    }

    getdosmem(buffer,secsize*nsecs,bsel);
    dosfree(sel);
    dosfree(bsel);

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
__dpmi_regs regs;
int seg,sel;
struct DCB Dcb DBG_0;
struct DCB *dcb = &Dcb;


    if ((seg = _dosmalloc(secsize*nsecs,&sel)) == -1)
        return MEM_ERR;

    DBG_zero(buf,nsecs*secsize);

    dcb->sector = sec;
    dcb->number = (unsigned short)nsecs;
    dcb->buffer = buf;
    regs.x.ax = DOS_EXT_ABS_READ_WRITE;
    regs.x.dx = disk+1;
    regs.x.cx = (unsigned short)-1;
    regs.x.bx = 0;
    regs.x.ds = (int)dcb;

    /* dangerous function! SI:0 MUST BE 0 for READ, 1 for WRITE */

    regs.x.si = 0;

    DBG_dpmi_dump(&regs);

    __dpmi_int(0x21,&regs);

    DBG_dpmi_dump(&regs);

    if (regs.x.ax == 0x7300) {          /* fail indication DOS/NT */
        regs.x.flags = 1;
        setdoserror(1);
    }

    if (regs.x.flags & 1) {
        DBG_err_dump("read32");
        _dosfree(sel);
        return DOS_ERR;
    }

    _getdosmem(buf,secsize*nsecs,sel);
    _dosfree(sel);

    return DISK_OK;
}
