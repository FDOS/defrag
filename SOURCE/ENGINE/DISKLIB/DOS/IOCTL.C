/*
 * dos\ioctl.c      perform DOS IOCTL calls
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

/*
 * dos_ioctl        perform DOS IOCTL call (INT 21h/440Dh)
 *
 * Note: Only SUBFUNCTION 0x08 category codes are supported.
 *       Functions that have output data are not supported.
 *
 */

extern int dos_ioctl(DOS_SUBFUNCTION func, DOS_MINOR_CODE code, int dev,
                     void *data)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;

    regs.x.ax = 0x4400 + func;
    regs.x.bx = dev + 1;

    if (code != DOS_MINOR_NONE)
        regs.x.cx = 0x0800 + code;

    if (data)
    {
        regs.x.dx = FP_OFF(data);
        sregs.ds = FP_SEG(data);
    }

    DBG_reg_dump(&regs,&sregs);

    intdosx(&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("ioctl");
        return DOS_ERR;
    }

    /* handle special cases */

    if (func == DOS_DEV_REMOVE)
        return regs.x.ax;

    if (func == DOS_DRV_REMOTE)
        return regs.x.dx;

    return DEVICE_OK;
}

/*
 * dos_ioctl32      perform DOS IOCTL call with category 48h (FAT32)
 *
 * Note: Although this function use the DOS_SUBFUNCTION parameter,
 *       so far all calls to this function are of subfunction 0Dh.
 */

extern int dos_ioctl32(DOS_SUBFUNCTION func, DOS_MINOR_CODE code, int dev,
                       void *data)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;

    regs.x.ax = 0x4400 + func;
    regs.x.bx = dev + 1;

    if (code != DOS_MINOR_NONE)
        regs.x.cx = 0x4800 + code;

    if (data)
    {
        regs.x.dx = FP_OFF(data);
        sregs.ds = FP_SEG(data);
    }

    DBG_reg_dump(&regs,&sregs);

    intdosx(&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("ioctl");
        return DOS_ERR;
    }

    return DEVICE_OK;
}
