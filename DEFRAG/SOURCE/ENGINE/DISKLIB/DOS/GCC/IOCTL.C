/*
 * dos\gcc\ioctl.c      perform DOS IOCTL calls
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
__dpmi_regs regs DBG_0;
int seg,sel;
int size;

    regs.x.ax = 0x4400 + func;
    regs.x.bx = dev + 1;
    if (code != DOS_MINOR_NONE)
        regs.x.cx = 0x0800 + code;

    if (data)
    {
        switch (code) {
            case DOS_MINOR_GET_DEVICE:
                size = sizeof(struct DEVICEPARAMS);
                break;
            case DOS_MINOR_GET_MEDIA:
                size = sizeof(struct MID);
                break;
            default:
                return DEVICE_ERR;
                break;
        }
        if ((seg = _dosmalloc(size,&sel)) == -1)
            return MEM_ERR;
        regs.x.ds = seg;
        regs.x.dx = 0;
    }

    __dpmi_int(0x21,&regs);

    if (regs.x.flags & 1) {
        DBG_err_dump("ioctl");
        return DOS_ERR;
    }

    if (data) {
        _getdosmem(data,size,sel);
        _dosfree(sel);
    }

    /* handle special cases */

    if (func == DOS_DEV_REMOVE)
        return regs.x.ax;

    if (func == DOS_DRV_REMOTE)
        return regs.x.dx;

    return DISK_OK;
}

/*
 * dos_ioctl32      perform DOS IOCTL call with category 48h (FAT32)
 *
 * Note: Only SUBFUNCTION 0x08 category codes are supported.
 *       Functions that have output data are not supported.
 *
 */

extern int dos_ioctl32(DOS_SUBFUNCTION func, DOS_MINOR_CODE code, int dev,
                     void *data)
{
__dpmi_regs regs DBG_0;
int seg,sel;
int size;

    regs.x.ax = 0x4400 + func;
    regs.x.bx = dev + 1;
    if (code != DOS_MINOR_NONE)
        regs.x.cx = 0x4800 + code;

    if (data)
    {
        switch (code) {
            case DOS_MINOR_GET_DEVICE:
                size = sizeof(struct DEVICEPARAMS);
                break;
            case DOS_MINOR_GET_MEDIA:
                size = sizeof(struct MID);
                break;
            default:
                return DEVICE_ERR;
                break;
        }
        if ((seg = _dosmalloc(size,&sel)) == -1)
            return MEM_ERR;
        regs.x.ds = seg;
        regs.x.dx = 0;
    }

    __dpmi_int(0x21,&regs);

    if (regs.x.flags & 1) {
        DBG_err_dump("ioctl");
        return DOS_ERR;
    }

    if (data) {
        _getdosmem(data,size,sel);
        _dosfree(sel);
    }

    /* handle special cases */

    if (func == DOS_DEV_REMOVE)
        return regs.x.ax;

    if (func == DOS_DRV_REMOTE)
        return regs.x.dx;

    return DISK_OK;
}
