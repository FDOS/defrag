/*
 * win32\bios.c     INT 13h functions
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 * 22-Nov-1998  greggj  return regs.h.ah was regs.h.al (oops)
 *
 */

#include "win32.h"
#include "dosio.h"
#include "debug.h"
#include "disklib.h"        /* stroserror() */

static int adjdrv(int drv);

/*
 * disk_get_physical    get basic disk geometry (tracks, sectors, heads)
 *
 */

extern int disk_get_physical(int disk, int *t, int *s, int *h)
{
unsigned int tmp;
DIOC_REGISTERS regs DBG_0;

    regs.x.ax = 0x0800;
    regs.h.dl = (unsigned char)adjdrv(disk);

    if (win_device_io(VWIN32_DIOC_DOS_INT13,&regs) != DEVICE_OK)
        return DEVICE_ERR;

    if (regs.x.flags & 0x0001)
        return (int)regs.h.ah;

    if (h) *h = regs.h.dh;
    if (t) *t = regs.h.ch;          /* low bits only */
    tmp = regs.h.cl;                /* has high bits of track also */
    if (s) *s = regs.h.cl;          /* has high bits of track also */
    if (s) *s &= 0x3f;              /* mask off track high bits */
    if (t) *t += (tmp >> 6) << 8;   /* shift and add track high bits */

    return DISK_OK;
}

/*
 * disk_get_physical    get basic disk geometry (tracks, sectors, heads)
 *
 */

/* This was a test... and it Fatally Crashes Windows 95! */

extern int disk_get_physical_ext(int disk, int *t, int *s, int *h)
{
unsigned int tmp;
DIOC_REGISTERS regs DBG_0;
struct DISK_PACKET b,*buf;

    buf = &b;
    b.size = sizeof(struct DISK_PACKET);
    regs.x.ax = 0x4800;
    regs.h.dl = (unsigned char)adjdrv(disk);
    regs.d.esi = (DWORD)buf;

    if (win_device_io(VWIN32_DIOC_DOS_INT13,&regs) != DEVICE_OK)
        return DEVICE_ERR;

    if (regs.x.flags & 0x0001)
        return disk_get_physical(disk,t,s,h);

    if (h) *h = regs.h.dh;
    if (t) *t = regs.h.ch;          /* low bits only */
    tmp = regs.h.cl;                /* has high bits of track also */
    if (s) *s = regs.h.cl;          /* has high bits of track also */
    if (s) *s &= 0x3f;              /* mask off track high bits */
    if (t) *t += (tmp >> 6) << 8;   /* shift and add track high bits */

    return DISK_OK;
}

/*
 * disk_read_p      read sector(s)
 *
 */

/*
    From Microsoft Knowledge Base Article ID: Q137176

    Win32-based applications running under Windows 95 use CreateFile
    to open VWIN32 and then use DeviceIoControl with the
    VWIN32_DIOC_DOS_INT13 flag to perform low-level BIOS disk functions.

    The functions work on floppy disks but always fail on hard disks.

*/

extern int disk_read_p(int disk, int trk, int sec, int head,
                       void *buf, int nsecs)
{
DIOC_REGISTERS regs DBG_0;

    regs.h.ah = 2;
    regs.h.al = (unsigned char)nsecs;
    regs.h.ch = (unsigned char)trk;
    regs.h.cl = (unsigned char)sec;
    if (disk > 1) {
        regs.h.cl += (unsigned char)((trk & 0x300) >> 2);
    }
    regs.h.dh = (unsigned char)head;
    regs.h.dl = (unsigned char)adjdrv(disk);

    regs.d.ebx = (DWORD)buf;

    if (win_device_io(VWIN32_DIOC_DOS_INT13,&regs) != DEVICE_OK)
        return DEVICE_ERR;

    if (regs.x.flags & 0x0001)
        return regs.h.ah;

    return DISK_OK;
}

/*
 * disk_write_p     write sector(s)
 *
 */

extern int disk_write_p(int disk, int trk, int sec, int head,
                        void *buf, int nsecs)
{
DIOC_REGISTERS regs;

    regs.h.ah = 3;
    regs.h.al = (unsigned char)nsecs;
    regs.h.ch = (unsigned char)trk;
    regs.h.cl = (unsigned char)sec;
    if (disk > 1) {
        regs.h.cl += (unsigned char)((trk & 0x300) >> 2);
    }
    regs.h.dh = (unsigned char)head;
    regs.h.dl = (unsigned char)adjdrv(disk);

    regs.d.ebx = (DWORD)buf;

    if (win_device_io(VWIN32_DIOC_DOS_INT13,&regs) != DEVICE_OK)
        return DEVICE_ERR;

    if (regs.x.flags & 0x0001)
        return regs.h.ah;

    return DISK_OK;
}

/*
 * disk_verify_p    verify sector(s)
 *
 */

extern int disk_verify_p(int disk, int trk, int sec, int head, int nsecs)
{
    return 0;
}

extern int disk_reset(int disk)
{
    return 0;
}

extern int disk_status(int disk)
{
    return 0;
}

/*
 * disk_part        get disk partition sector
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
unsigned char buf[512];

    if (disk_read_p(disk,0,1,0,buf,1) != DISK_OK)
        return 0;
    return (int)buf[0x1be + 4];
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
    if (drv > 1) {
        return drv + 0x80 - 2;
    }
    return drv;
}
