/*
 * dos\dosio.c      interface for INT 21h
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>
#include <string.h>

#include <dos.h>

#include "dosio.h"
#include "debug.h"

/*
 * doscall -  generic DOS calls
 *
 */

extern int doscall(DOS_FUNCTION ax, int bx, int cx, int dx, void *data)
{
union REGS regs;
struct SREGS sregs;

    regs.x.ax = ax;            /* perform the call */
    regs.x.bx = bx;
    regs.x.cx = cx;
    regs.x.dx = dx;

    DBG_reg_dump(&regs,&sregs);

    intdosx(&regs,&regs,&sregs);

    DBG_reg_dump(&regs,&sregs);

    if (regs.x.cflag) {
        DBG_err_dump("dos");
        return DOS_ERR;
    }

    switch (ax)                /* handle all special cases */
    {
        case DOS_GET_VER:
            regs.h.cl = regs.h.al;        /* trash CL */
            regs.h.al = regs.h.ah;
            regs.h.ah = regs.h.cl;
            break;
        case DOS_GET_VER_5:
            regs.h.al = regs.h.bh;
            regs.h.ah = regs.h.bl;
            break;
        case DOS_GET_FREE_SPACE:
#ifdef INT24ERRHANDLER                                  /* with crit the */
            if (regs.x.ax >= 0xff || error.num != -1)   /* INT 24h will be */
#else                                                   /* trapped and */
            if (regs.x.ax >= 0xff)                      /* error.num set */
#endif
            {
                return DOS_ERR;
            }
            else
            {
                struct FREESPACE *fs = (struct FREESPACE *)data;
                fs->secs_cluster = regs.x.ax;
                fs->avail_clusters = regs.x.bx;
                fs->sec_size = regs.x.cx;
                fs->num_clusters = regs.x.dx;
            }
            break;
        case DOS_GET_DPB:
#ifdef INT24ERRHANDLER                                  /* with crit the */
            if (regs.x.ax >= 0xff || error.num != -1)   /* INT 24h will be */
#else                                                   /* trapped and */
            if (regs.h.ah == 0xff)                      /* error.num set */
#endif
            {
                return DOS_ERR;
            }
            else
            {
                struct DPB *dpb = MK_FP(sregs.ds,regs.x.bx);
                memcpy(data,dpb,sizeof(struct DPB));
            }
            break;
        default:
            break;
    }

    return DISK_OK;
}
