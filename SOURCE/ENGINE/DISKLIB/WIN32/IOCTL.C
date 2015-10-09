/*
 * win32\ioctl.c    perform DOS IOCTL calls
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>

#include "win32.h"
#include "dosio.h"
#include "debug.h"

/*
 * win_ioctl        perform DOS IOCTL call (INT 21h/440Dh)
 *
 * Notes: Unlike the DOS version of this function this function
 *        only supports only a subset of 440Dh IOCTL functions.
 *
 */

extern int win_ioctl(DOS_MINOR_CODE func, int dev, void *buf)
{
int i;
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = 0x440D;
    regs.d.ebx = dev + 1;
    regs.d.ecx = 0x0800 + func;
    regs.d.edx = (DWORD)buf;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 0x0001)
        return regs.x.ax;
    /*
        Note: If carry is set Windows does not set the error code
        that would be returned by GetLastError() (and there appears
        to be no dosexterr() equivalent) so the driver error code is
        returned, which will be greater than 0 which is greater than
        the ..._OK and ..._ERR defines.
    */

    return DEVICE_OK;
}

/*
 * win_ioctl_ext    perform Window 95 IOCTL call (INT 21h/440Dh)
 *
 * Notes: Unlike the DOS version of this function this function
 *        only supports only a subset of 440Dh IOCTL functions.
 *
 */

extern int win_ioctl_ext(DOS_MINOR_CODE func, int dev, void *buf)
{
int i;
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = 0x440D;
    regs.d.ebx = dev + 1;
    regs.d.ecx = 0x4800 + func;
    regs.d.edx = (DWORD)buf;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 0x0001)
        return regs.x.ax;

    return DEVICE_OK;
}
