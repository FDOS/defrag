/*
 * dos\lock.c       Windows' LOCK related functions
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


extern int disk_get_volinfo(int disk, struct VOLUMEINFO *vi)
{
char root[] = "C:\\";
char filesys[9];
union REGS regs DBG_0;
struct SREGS sregs DBG_0;

    root[0] = (char)(disk+'A');
    regs.x.ax = 0x71A0;
    regs.x.cx = 9;
    regs.x.di = FP_OFF(filesys);
    sregs.es = FP_SEG(filesys);
    regs.x.dx = FP_OFF(root);
    sregs.ds = FP_SEG(root);

    DBG_reg_dump(&regs,&sregs);

    intdosx(&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("volinfo");
        return DOS_ERR;
    }

    vi->flags = regs.x.bx;
    vi->maxfname = regs.x.cx;
    vi->maxpath = regs.x.dx;

    return DISK_OK;
}

extern int disk_enum_files(int disk, char *files, int index)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;

    regs.x.ax = 0x440D;
    regs.x.bx = disk+1;
    regs.x.cx = 0x086D;
    regs.x.dx = FP_OFF(files);
    sregs.ds = FP_SEG(files);
    regs.x.si = index;
    regs.x.di = 0;

    DBG_reg_dump(&regs,&sregs);

    intdosx(&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("volinfo");
        return DOS_ERR;
    }

    return regs.x.cx;
}

extern void disk_flush(int disk)
{
union REGS regs DBG_0;

    regs.x.ax = 0x710D;
    regs.x.bx = disk+1;
    regs.x.cx = 0;

    DBG_reg_dump(&regs,NULL);

    intdos(&regs,&regs);

    DBG_reg_dump(&regs,NULL);
}

extern int disk_lock_unlock(int disk, enum LOCK_OPERATION op, int *numlocks)
{
union REGS regs DBG_0;
struct SREGS sregs DBG_0;
struct PARAMBLOCK Pb DBG_0;
struct PARAMBLOCK *pb = &Pb;

    if (numlocks) *numlocks = 0;

    Pb.operation = op;
    regs.x.ax = 0x440D;
    regs.x.bx = disk+1;
    regs.x.cx = 0x0848;
    regs.x.dx = FP_OFF(pb);
    sregs.ds = FP_SEG(pb);

    DBG_reg_dump(&regs,&sregs);

    intdosx(&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("lock/unlock");
        return DOS_ERR;
    }

    if (numlocks) *numlocks = Pb.numlocks;
    return regs.x.ax;
}


extern int disk_lock_flag(int disk)
{
union REGS regs DBG_0;

    regs.x.ax = 0x440D;
    regs.x.bx = disk+1;
    regs.x.cx = 0x086C;

    DBG_reg_dump(&regs,NULL);

    intdos(&regs,&regs);

    DBG_reg_dump(&regs,NULL);

    if (regs.x.cflag) {
        DBG_err_dump("lockflag");
        return DOS_ERR;
    }

    return regs.x.ax;
}

extern int disk_lock_state(int disk, int *level, int *perm)
{
union REGS regs DBG_0;

    *level = 0;
    *perm = 0;

    regs.x.ax = 0x440D;
    regs.x.bx = disk+1;
    regs.x.cx = 0x0870;

    DBG_reg_dump(&regs,NULL);

    intdos(&regs,&regs);

    DBG_reg_dump(&regs,NULL);

    if (regs.x.cflag) {
        DBG_err_dump("lockstate");
        return DOS_ERR;
    }

    *level = regs.x.ax;
    *perm = regs.x.cx;
    return DISK_OK;
}

extern int disk_lock_state32(int disk, int *level, int *perm)
{
union REGS regs DBG_0;

    *level = 0;
    *perm = 0;

    regs.x.ax = 0x440D;
    regs.x.bx = disk+1;
    regs.x.cx = 0x4870;

    DBG_reg_dump(&regs,NULL);

    intdos(&regs,&regs);

    DBG_reg_dump(&regs,NULL);

    if (regs.x.cflag)
        return disk_lock_state(disk,level,perm);

    *level = regs.x.ax;
    *perm = regs.x.cx;
    return DISK_OK;
}

extern int disk_lock_logical(int disk, int level, int perm)
{
union REGS regs DBG_0;

    regs.x.ax = 0x440D;
    regs.x.bx = (disk+1) | level;
    regs.x.cx = 0x084A;
    regs.x.dx = perm;

    DBG_reg_dump(&regs,NULL);

    intdos(&regs,&regs);

    DBG_reg_dump(&regs,NULL);

    if (regs.x.cflag) {
        DBG_err_dump("lock");
        return DOS_ERR;
    }

    return DISK_OK;
}

extern int disk_unlock_logical(int disk)
{
union REGS regs DBG_0;

    regs.x.ax = 0x440D;
    regs.x.bx = disk+1;
    regs.x.cx = 0x0800 | DOS_MINOR_UNLOCK_LOGICAL;

    DBG_reg_dump(&regs,NULL);

    intdos(&regs,&regs);

    DBG_reg_dump(&regs,NULL);

    if (regs.x.cflag) {
        DBG_err_dump("unlock");
        return DOS_ERR;
    }

    return DISK_OK;
}

extern int disk_lock_logical32(int disk, int level, int perm)
{
union REGS regs DBG_0;

    regs.x.ax = 0x440D;
    regs.x.bx = (disk+1) | level;
    regs.x.cx = 0x4800 | DOS_MINOR_LOCK_LOGICAL;
    regs.x.dx = perm;

    DBG_reg_dump(&regs,NULL);

    intdos(&regs,&regs);

    DBG_reg_dump(&regs,NULL);

    if (regs.x.cflag)
        return disk_lock_logical(disk,level,perm);

    return DISK_OK;
}

extern int disk_unlock_logical32(int disk)
{
union REGS regs DBG_0;

    regs.x.ax = 0x440D;
    regs.x.bx = disk+1;
    regs.x.cx = 0x4800 | DOS_MINOR_UNLOCK_LOGICAL;

    DBG_reg_dump(&regs,NULL);

    intdos(&regs,&regs);

    DBG_reg_dump(&regs,NULL);

    if (regs.x.cflag)
        return disk_unlock_logical(disk);

    return DISK_OK;
}
