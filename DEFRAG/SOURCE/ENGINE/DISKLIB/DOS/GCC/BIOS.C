/*
 * dos\gcc\bios.c       INT 13h functions
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <limits.h>

#include <dos.h>
#include <bios.h>

#include "dosio.h"
#include "debug.h"
#include "disklib.h"

static int adjdrv(int drv);

/*
 * disk_get_physical    get basic disk geometry (tracks, sectors, heads)
 *
 * returns -1 if successful else BIOS error code
 */

extern int disk_get_physical_ext(int disk, int *t, int *s, int *h)
{
__dpmi_regs regs DBG_0;
struct DISK_PACKET buf;

    buf.size = sizeof(struct DISK_PACKET);
    regs.x.ax = 0x4800;
    regs.h.dl = (unsigned char)adjdrv(disk);
    regs.x.si = 0;
    regs.x.ds = (int)&buf;

    __dpmi_int(0x13,&regs);

    if (regs.x.flags & 1)
        return disk_get_physical(disk,t,s,h);

    *t = buf.cylinders - 2;
    *h = buf.heads - 1;
    *s = buf.sectors;

    return DISK_OK;
}

extern int disk_get_physical(int disk, int *t, int *s, int *h)
{
unsigned int tmp;
union REGS regs;
#ifdef AMIBIOS
struct SREGS sregs;
char byte, *bug;
#endif

    regs.x.ax = 0x0800;
    regs.h.dl = (unsigned char)adjdrv(disk);

#ifdef AMIBIOS
    segread(&sregs);                /* get DS */
    bug = MK_FP(sregs.ds,0x74);     /* point to DS:0074 */
    byte = *bug;                    /* stash byte at DS:0074 */
#endif

    int86(0x13,&regs,&regs);

#ifdef AMIBIOS
    *bug = byte;                    /* restore */
#endif

    if (regs.x.cflag)
        return (int)regs.h.ah;

    *h = regs.h.dh;
    *t = regs.h.ch;             /* low bits only */
    tmp = *s = regs.h.cl;       /* has high bits of track also */
    *s &= 0x3f;                 /* mask off track high bits */
    *t += (tmp >> 6) << 8;      /* shift and add track high bits */

    return DISK_OK;
}

/*
 * disk_read_p      read sector(s)
 *
 */

extern int disk_read_p(int disk, int trk, int sec, int head,
                       void *buf, int nsecs)
{
unsigned i;
struct _diskinfo_t blk;

    blk.drive = adjdrv(disk);
    blk.track = trk;
    blk.sector = sec;
    blk.head = head;
    blk.nsectors = nsecs;
    blk.buffer = buf;

    _bios_disk(_DISK_READ,&blk);
    i = _bios_disk(_DISK_STATUS,&blk);

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
unsigned i;
struct _diskinfo_t blk;

    blk.drive = adjdrv(disk);
    blk.track = trk;
    blk.sector = sec;
    blk.head = head;
    blk.nsectors = nsecs;
    blk.buffer = buf;

    _bios_disk(_DISK_WRITE,&blk);
    i = _bios_disk(_DISK_STATUS,&blk);

    i >>= CHAR_BIT;     /* AH contains result; AL sectors read */

    return (i) ? i : DISK_OK;
}

extern int disk_reset(int disk)
{
unsigned i = 0;
struct _diskinfo_t blk;

    blk.drive = adjdrv(disk);
    i = _bios_disk(_DISK_RESET,&blk);
    i >>= CHAR_BIT;
    return (i) ? i : DISK_OK;
}

extern int disk_status(int disk)
{
unsigned i = 0;
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

    i = disk_read_p(disk,0,1,0,buf,1);
    if (i != DISK_OK)
        return 0;
#ifdef __GNUC__
    return (int)buf[0x1be + 4];     /* bug or what! */
#else
    return (int)buf[0x1be+4];
#endif
}



/* floppys: 0 - 7Fh, hards: 80h - FFh */

static int adjdrv(int drv)
{
    if (drv > 1)
        return drv + 0x80 - 2;
    return drv;
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
