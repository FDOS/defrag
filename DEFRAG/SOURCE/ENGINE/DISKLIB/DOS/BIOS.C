/*
 * dos\bios.c   INT 13h functions
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 12-Dec-1998  greggj  disk_get_physical() used to return AX!
 *
 */

/*
 * Note: These functions work under MS-DOS, Windows 95 and 98
 *       except as noted.
 *
 *       These functions only work on floppy drives under
 *       Windows NT.
 *
 */

#include <string.h>         /* memset */
#include <limits.h>         /* CHAR_BIT */

#include <dos.h>            /* REGS, etc. */
#include <bios.h>           /* bios_disk */

#include "debug.h"
#include "dosio.h"
#include "disklib.h"

static int adjdrv(int drv);

#ifdef __TURBOC__
struct _diskinfo_t
{
    int drive;
    int track;
    int sector;
    int head;
    int nsectors;
    void* buffer;
};

int _bios_disk(int cmd, struct _diskinfo_t* blk)
{
   return biosdisk(cmd, blk->drive, blk->head, blk->track, blk->sector,
                   blk->nsectors, blk->buffer);
}

#define _DISK_RESET     0
#define _DISK_STATUS    1
#define _DISK_READ      2
#define _DISK_WRITE     3
#define _DISK_VERIFY    4
#endif

/*
 * disk_get_physical    get basic disk geometry (tracks, sectors, heads)
 *
 */

extern int disk_get_physical(int disk, int *t, int *s, int *h)
{
unsigned int tmp;
union REGS regs DBG_0;
#ifdef AMIBIOS
struct SREGS sregs;
char byte, *bug;
#endif

#ifndef NDEBUG
    if (t) *t = 0;
    if (s) *s = 0;
    if (h) *h = 0;
#endif

    regs.x.ax = 0x0800;
    regs.h.dl = (unsigned char)adjdrv(disk);

#ifdef AMIBIOS
    segread(&sregs);                /* get DS */
    bug = MK_FP(sregs.ds,0x74);     /* point to DS:0074 */
    byte = *bug;                    /* stash byte at DS:0074 */
#endif

    DBG_reg_dump(&regs,NULL);

    int86(0x13,&regs,&regs);

    DBG_reg_dump(&regs,NULL);

#ifdef AMIBIOS
    *bug = byte;                    /* restore */
#endif

    if (regs.x.cflag)
        return (int)regs.h.ah;

    if (h) *h = regs.h.dh;
    if (t) *t = regs.h.ch;          /* low bits only */
    tmp = regs.h.cl;                /* has high bits of track also */
    if (s) *s = regs.h.cl;
    if (s) *s &= 0x3f;              /* mask off track high bits */
    if (t) *t += (tmp >> 6) << 8;   /* shift and add track high bits */

    return DISK_OK;
}

/*
 * disk_get_physical_ext    get basic disk geometry (tracks, sectors, heads)
 *
 */

extern int disk_get_physical_ext(int disk, int *t, int *s, int *h)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;
struct DISK_PACKET b DBG_0;
struct DISK_PACKET *buf;

#ifndef NDEBUG
    if (t) *t = 0;
    if (s) *s = 0;
    if (h) *h = 0;
#endif

    /*
        Microsoft bombs if the address of operator is used in the
        FP_OFF() or FP_SEG() macros so a pointer is kludged here so
        there is no need for prepocessor conditionals for MS.
    */

    buf = &b;
    b.size = sizeof(struct DISK_PACKET);
    regs.x.ax = 0x4800;
    regs.h.dl = (unsigned char)adjdrv(disk);
    regs.x.si = FP_OFF(buf);
    sregs.ds = FP_SEG(buf);

    DBG_reg_dump(&regs,&sregs);

    int86(0x13,&regs,&regs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag)
        return disk_get_physical(disk,t,s,h);

    if (t) *t = b.cylinders - 2;
    if (h) *h = b.heads - 1;
    if (s) *s = b.sectors;

    return DISK_OK;
}

/*
 * disk_read_p      read sector(s)
 *
 */

extern int disk_read_p(int disk, int trk, int sec, int head,
                       void *buf, int nsecs)
{
unsigned int i;
struct _diskinfo_t blk;

    DBG_zero(buf,nsecs*512);    /* for efficiency, only zero if debugging */

    blk.drive = adjdrv(disk);
    blk.track = trk;
    blk.sector = sec;
    blk.head = head;
    blk.nsectors = nsecs;
    blk.buffer = buf;

    _bios_disk(_DISK_READ,&blk);
    i = _bios_disk(_DISK_STATUS,&blk);

#ifdef TRACE
    printf("Int 13h, 02h: AX = %04xh\n",i);
#endif

    /*
        Probably should use int86() so that there can be a register
        dump...
    */

    i >>= CHAR_BIT;     /* AH contains result; AL sectors read */

    return (i) ? i : DISK_OK;
}

/*
 * disk_write_p     write sector(s)
 *
 */

extern int disk_write_p(int disk, int trk, int sec, int head,
                        void *buf, int nsecs)
{
unsigned int i;
struct _diskinfo_t blk;

    blk.drive = adjdrv(disk);
    blk.track = trk;
    blk.sector = sec;
    blk.head = head;
    blk.nsectors = nsecs;
    blk.buffer = buf;

    _bios_disk(_DISK_WRITE,&blk);
    i = _bios_disk(_DISK_STATUS,&blk);

#ifdef TRACE
    printf("Int 13h, 03h: AX = %04xh\n",i);
#endif

    i >>= CHAR_BIT;     /* AH contains result; AL sectors read */

    return (i) ? i : DISK_OK;
}

/*
 * disk_verify_p    verify sector(s)
 *
 */

extern int disk_verify_p(int disk, int trk, int sec, int head, int nsecs)
{
unsigned int i;
struct _diskinfo_t blk;

    blk.drive = adjdrv(disk);
    blk.track = trk;
    blk.sector = sec;
    blk.head = head;
    blk.nsectors = nsecs;

    i = _bios_disk(_DISK_VERIFY,&blk);

#ifdef TRACE
    printf("Int 13h, 04h: AX = %04xh\n",i);
#endif

    /*
        Probably should use int86() so that there can be a register
        dump...
    */

    i >>= CHAR_BIT;     /* AH contains result; AL sectors read */

    return (i) ? i : DISK_OK;
}


extern int disk_reset(int disk)
{
unsigned int i;
struct _diskinfo_t blk;

    blk.drive = adjdrv(disk);
    i = _bios_disk(_DISK_RESET,&blk);
    i >>= CHAR_BIT;
    return (i) ? i : DISK_OK;
}

extern int disk_status(int disk)
{
unsigned i;
struct _diskinfo_t blk;

    blk.drive = adjdrv(disk);
    i = _bios_disk(_DISK_STATUS,&blk);
    return i >> CHAR_BIT;
}

/*
 * disk_part        get disk partition sector (first physical sector)
 *
 */

extern int disk_part(int disk, int c, int h, int s, void *buf)
{
    DBG_zero(buf,512);

    return disk_read_p(disk,c,s,h,buf,1);
}

/*
 * disk_type        get disk (partition) type
 *
 */

extern int disk_type(int disk)
{
int i;
unsigned char buf[512];

    DBG_zero(buf,512);

    i = disk_read_p(disk,0,1,0,buf,1);
    if (i != DISK_OK)
        return 0;
#ifdef __GNUC__
    return (int)buf[0x1be + 4];     /* bug or what! */
#else
    return (int)buf[0x1be+4];
#endif
}

static struct int13_msg {
   int code;
   char *msg;
} int13_diskerr_msg[] = {
   { 0x01, "bad command/unknown drive" },
   { 0x02, "address mark not found" },
   { 0x03, "write-protected disk" },
   { 0x04, "sector not found" },
   { 0x05, "reset failed" },
   { 0x06, "diskette removed or changed" },
   { 0x07, "bad parameter table" },
   { 0x08, "DMA overrun" },
   { 0x09, "attempt to DMA across 64K boundary" },
   { 0x0A, "bad sector detected" },
   { 0x0B, "bad track detected" },
   { 0x0C, "unsupported track or media type not found" },
   { 0x0D, "invalid number of sectors on format" },
   { 0x0E, "control data address mark detected" },
   { 0x0F, "DMA arbitration level out of range" },
   { 0x10, "uncorrectable CRC/EEC on read" },
   { 0x11, "ECC corrected data error" },
   { 0x20, "controller failure" },
   { 0x40, "seek failed" },
   { 0x80, "timeout" },
   { 0xAA, "drive not ready" },
   { 0xBB, "undefined error" },
   { 0xCC, "write fault" },
   { 0xE0, "status error" },
   { 0xFF, "sense operation failed" },
   { 0,    "unknown error" }
};

#define nerrors (sizeof(int13_diskerr_msg)/sizeof(struct int13_msg))

extern const char *disk_error(int error)
{
int i;
char *errmsg;

   for (i = 0; i < nerrors-1; i++)
      if (int13_diskerr_msg[i].code == error)
         break;

   errmsg = int13_diskerr_msg[i].msg;

   return errmsg;
}



/* floppys: 0 - 7Fh, hards: 80h - FFh */

static int adjdrv(int drv)
{
    if (drv > 1)
        return drv + 0x80 - 2;
    return drv;
}
