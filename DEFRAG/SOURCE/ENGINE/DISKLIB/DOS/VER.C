 /*
 * dos\ver.c        version numbers
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

extern int lib_ver(void)
{
    return LIB_DOS;
}

#define swapHILO(w)  (((unsigned short)w >> 8) | ((unsigned short)w<<8))

extern int dos_ver(void)
{
union REGS regs;

    regs.x.ax = DOS_GET_VER;
    intdos(&regs,&regs);
    regs.x.ax = swapHILO(regs.x.ax);
    if (regs.x.ax >= 0x500) {
        regs.x.ax = DOS_GET_VER_5;
        intdos(&regs,&regs);
        regs.h.al = regs.h.bh;
        regs.h.ah = regs.h.bl;
    }
    return regs.x.ax;
}

extern int win_ver(void)
{
union REGS regs;

   regs.x.ax = 0x1600;
   int86(0x2f,&regs,&regs);

   if (regs.x.ax == 0x1600)
      regs.x.ax = 0;
   else
      regs.x.ax = swapHILO(regs.x.ax);

   return regs.x.ax;
}
