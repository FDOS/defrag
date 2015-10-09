/* testing... not finished */

/*
 *
 * This file is part of the ALPHA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 */

#include "win32.h"
#include "dosio.h"
#include "debug.h"

extern int intdos(union REGS *regs, union REGS *oregs)
{
int i;
DIOC_REGISTERS dregs DBG_0;

    dregs.x.ax = regs->x.ax;
    dregs.x.bx = regs->x.bx;
    dregs.x.cx = regs->x.cx;
    dregs.x.dx = regs->x.dx;
    dregs.x.di = regs->x.di;
    dregs.x.si = regs->x.si;

    if (regs->h.ah == 0x44)
        i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&dregs);
    else if (regs->h.ah == 0x73)
        i = win_device_io(VWIN32_DIOC_DOS_DRIVEINFO,&dregs);
    else
        i = DOS_ERR;            /* kludge */

    if (i == DOS_ERR) {
        oregs->x.cflag = 1;       /* kludge */
        return i;
    }

    oregs->x.ax = dregs.x.ax;
    oregs->x.bx = dregs.x.bx;
    oregs->x.cx = dregs.x.cx;
    oregs->x.dx = dregs.x.dx;
    oregs->x.di = dregs.x.di;
    oregs->x.si = dregs.x.si;
    oregs->x.cflag = dregs.x.flags;

    return oregs->x.ax;
}


extern int intdosx(union REGS *regs, union REGS *oregs, struct SREGS *sregs)
{
int i;
DIOC_REGISTERS dregs DBG_0;

    dregs.d.eax = regs->x.ax;
    dregs.d.ebx = regs->x.bx;
    dregs.d.ecx = regs->x.cx;
    dregs.d.edx = regs->x.dx;
    dregs.d.edi = regs->x.di;
    dregs.d.esi = regs->x.si;

    if (regs->h.ah == 0x44)
        i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&dregs);
    else if (regs->h.ah == 0x73)
        i = win_device_io(VWIN32_DIOC_DOS_DRIVEINFO,&dregs);
    else
        i = DOS_ERR;            /* kludge */

    if (i == DOS_ERR) {
        oregs->x.cflag = 1;       /* kludge */
        return i;
    }

    oregs->x.ax = dregs.x.ax;
    oregs->x.bx = dregs.x.bx;
    oregs->x.cx = dregs.x.cx;
    oregs->x.dx = dregs.x.dx;
    oregs->x.di = dregs.x.di;
    oregs->x.si = dregs.x.si;
    oregs->x.cflag = dregs.x.flags;

    return oregs->x.ax;
}
