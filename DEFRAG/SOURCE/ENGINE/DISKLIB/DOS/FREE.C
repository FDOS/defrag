/*
 * dos\free.c       get disk free space
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>      /* NULL */
#include <dos.h>        /* REGS, intdos() */

#include "dosio.h"
#include "debug.h"
#include "disklib.h"

/*
 * disk_free_space      get disk free space, unit and sector sizes
 *
 */

extern int disk_free_space(int disk, struct FREESPACE *fs)
{
union REGS regs DBG_0;

    DBG_zero(fs,sizeof(struct FREESPACE));

    regs.x.ax = DOS_GET_FREE_SPACE;
    regs.x.dx = disk + 1;               /* 1 - N */

    intdos(&regs,&regs);

    if (regs.x.ax >= 0xff) {
        DBG_err_dump("free");
        return DOS_ERR;
    }

    fs->secs_cluster = (unsigned short)regs.x.ax;
    fs->avail_clusters = (unsigned short)regs.x.bx;
    fs->sec_size = (unsigned short)regs.x.cx;
    fs->num_clusters = (unsigned short)regs.x.dx;

    return DISK_OK;
}

#ifndef __GNUC__  /* this needs to be re-written for DJGPP */

/*
 * disk_free_space32    get disk free space, unit and sector sizes (FAT32)
 *
 */

extern int disk_free_space32(int disk, struct EXT_FREESPACE *fs)
{
char drive[] = "C:\\";
union REGS regs DBG_0;
struct SREGS sregs DBG_0;

    DBG_zero(fs,sizeof(struct EXT_FREESPACE));

    drive[0] = (char)(disk+'A');
    regs.x.ax = DOS_EXT_GET_FREE_SPACE;
    regs.x.dx = disk + 1;
    regs.x.cx = sizeof(struct EXT_FREESPACE);
    regs.x.di = FP_OFF(fs);
    sregs.es = FP_SEG(fs);
    regs.x.dx = FP_OFF(drive);
    sregs.ds = FP_SEG(drive);

    DBG_reg_dump(&regs,&sregs);

    intdos(&regs,&regs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.ax >= 0xff) {
        DBG_err_dump("free(32)");
        return DOS_ERR;
    }

    return DISK_OK;
}

#endif   /* __GNUC__ */

/*
 * drive_size       get drive size in bytes
 *
 * TODO: FAT32 support
 * GROTS: It should work FAT32
 */

extern int drive_size(int disk, long *size)
{
int i;
struct DEVICEPARAMS dp DBG_0;

    *size = 0L;

    if ((i = disk_getparams(disk,&dp)) != DISK_OK)
        return i;

    *size = dp.num_sectors;             /* (temp at first) */
    if (*size == 0L)                    /* (need to do some) */
        *size = dp.total_sectors;       /* (multiplying) */
    *size *= dp.sec_size;

    return i;
}

/*
 * get_drive        get drive number (A: = 0, B: = 1, etc.)
 *
 */

extern int get_drive(void)
{
int disk;

#ifdef __TURBOC__
    return getdisk();
#else
    _dos_getdrive((unsigned int *)&disk);
#endif
    return disk - 1;
}
