/*
 * win32\lock.c     LOCK functions
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <dos.h>

#include "win32.h"
#include "dosio.h"
#include "debug.h"


extern int disk_get_volinfo(int disk, struct VOLUMEINFO *vi)
{
int i;
char filesys[9];
char root[] = "C:\\";
DIOC_REGISTERS regs DBG_0;

    root[0] = (char)(disk+'A');
    regs.d.eax = 0x71A0;
    regs.d.ecx = 9;
    regs.d.edi = (DWORD)filesys;
    regs.d.edx = (DWORD)root;

    if ((i = win_device_io(VWIN32_DIOC_DOS_DRIVEINFO,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1) {
        DBG_err_dump("volinfo");
        return DOS_ERR;
    }

    vi->flags = regs.d.ebx;
    vi->maxfname = regs.d.ecx;
    vi->maxpath = regs.d.edx;

    return DISK_OK;
}

extern int disk_enum_files(int disk, char *files, int index)
{
int i;
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = 0x440D;
    regs.d.ebx = disk+1;
    regs.d.ecx = 0x086D;
    regs.d.edx = FP_OFF(files);
    regs.d.esi = index;
    regs.d.edi = 0;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1) {
        DBG_err_dump("volinfo");
        return DOS_ERR;
    }

    return regs.d.ecx;
}

extern void disk_flush(int disk)
{
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = 0x710D;
    regs.d.ebx = disk+1;
    regs.d.ecx = 0;

    win_device_io(VWIN32_DIOC_DOS_DRIVEINFO,&regs);

}

extern int disk_lock_unlock(int disk, enum LOCK_OPERATION op, int *numlocks)
{
int i;
DIOC_REGISTERS regs DBG_0;
struct PARAMBLOCK Pb DBG_0;
struct PARAMBLOCK *pb = &Pb;

    if (numlocks) *numlocks = 0;

    Pb.operation = op;
    regs.d.eax = 0x440D;
    regs.d.ebx = disk+1;
    regs.d.ecx = 0x0848;
    regs.d.edx = (DWORD)pb;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1) {
        DBG_err_dump("lock/unlock");
        return DOS_ERR;
    }

    if (numlocks) *numlocks = Pb.numlocks;
    return regs.d.eax;
}


extern int disk_lock_flag(int disk)
{
int i;
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = 0x440D;
    regs.d.ebx = disk+1;
    regs.d.ecx = 0x086C;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1) {
        DBG_err_dump("lockflag");
        return DOS_ERR;
    }

    return regs.d.eax;
}

extern int disk_lock_state(int disk, int *level, int *perm)
{
int i;
DIOC_REGISTERS regs DBG_0;

    *level = 0;
    *perm = 0;

    regs.d.eax = 0x440D;
    regs.d.ebx = disk+1;
    regs.d.ecx = 0x0870;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1) {
        DBG_err_dump("lockstate");
        return DOS_ERR;
    }

    *level = regs.d.eax;
    *perm = regs.d.ecx;
    return DISK_OK;
}

extern int disk_lock_state32(int disk, int *level, int *perm)
{
int i;
DIOC_REGISTERS regs DBG_0;

    *level = 0;
    *perm = 0;

    regs.d.eax = 0x440D;
    regs.d.ebx = disk+1;
    regs.d.ecx = 0x4870;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1)
        return disk_lock_state(disk,level,perm);

    *level = regs.d.eax;
    *perm = regs.d.ecx;
    return DISK_OK;
}

extern int disk_lock_logical(int disk, int level, int perm)
{
int i;
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = 0x440D;
    regs.d.ebx = (disk+1) | level;
    regs.d.ecx = 0x084A;
    regs.d.edx = perm;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1) {
        DBG_err_dump("lock");
        return DOS_ERR;
    }

    return DISK_OK;
}

extern int disk_unlock_logical(int disk)
{
int i;
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = 0x440D;
    regs.d.ebx = disk+1;
    regs.d.ecx = 0x0800 | DOS_MINOR_UNLOCK_LOGICAL;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1) {
        DBG_err_dump("unlock");
        return DOS_ERR;
    }

    return DISK_OK;
}

extern int disk_lock_logical32(int disk, int level, int perm)
{
int i;
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = 0x440D;
    regs.d.ebx = (disk+1) | level;
    regs.d.ecx = 0x4800 | DOS_MINOR_LOCK_LOGICAL;
    regs.d.edx = perm;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1)
        return disk_lock_logical(disk,level,perm);

    return DISK_OK;
}

extern int disk_unlock_logical32(int disk)
{
int i;
DIOC_REGISTERS regs DBG_0;

    regs.d.eax = 0x440D;
    regs.d.ebx = disk+1;
    regs.d.ecx = 0x4800 | DOS_MINOR_UNLOCK_LOGICAL;

    if ((i = win_device_io(VWIN32_DIOC_DOS_IOCTL,&regs)) != DEVICE_OK)
        return i;

    if (regs.x.flags & 1)
        return disk_unlock_logical(disk);

    return DISK_OK;
}
