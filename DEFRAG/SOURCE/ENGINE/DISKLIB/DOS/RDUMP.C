/*
 * dos\rdump.c      display 80x86 registers
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>

#include <dos.h>

#define O_MASK 0x0800
#define D_MASK 0x0400
#define I_MASK 0x0200
#define S_MASK 0x0080
#define Z_MASK 0x0040
#define A_MASK 0x0010
#define P_MASK 0x0004
#define C_MASK 0x0001

/*
 * reg_dump     display 8086 registers
 *
 */

extern void reg_dump(union REGS *regs, struct SREGS *sregs)
{
int f;
#if 0
char *over[]={"NV","OV"};
char *dire[]={"UP","DN"};
char *inte[]={"DI","EI"};
char *sign[]={"PL","NG"};
char *zero[]={"NZ","ZR"};
char *auxi[]={"NA","AC"};
char *pari[]={"PO","PE"};
#endif
char *carr[]={"NC","CY"};


    fprintf(stdout,"\n");
    fprintf(stdout,"AX=%04X  BX=%04X  CX=%04X  DX=%04X  ",
            regs->x.ax,regs->x.bx,regs->x.cx,regs->x.dx);
    if (sregs != NULL) {
        fprintf(stdout,"\n");
        fprintf(stdout,"SI=%04X  DI=%04X  DS=%04X  ES=%04X  ",
                        regs->x.si,regs->x.di,sregs->ds, sregs->es);
    }
    f = regs->x.cflag;
#if 0
    fprintf(stdout,"%s ",over[(f&O_MASK) ? (1) : (0)]);
    fprintf(stdout,"%s ",dire[(f&D_MASK) ? (1) : (0)]);
    fprintf(stdout,"%s ",inte[(f&I_MASK) ? (1) : (0)]);
    fprintf(stdout,"%s ",sign[(f&S_MASK) ? (1) : (0)]);
    fprintf(stdout,"%s ",zero[(f&Z_MASK) ? (1) : (0)]);
    fprintf(stdout,"%s ",auxi[(f&A_MASK) ? (1) : (0)]);
    fprintf(stdout,"%s ",pari[(f&P_MASK) ? (1) : (0)]);
#endif
    fprintf(stdout,"%s ",carr[(f&C_MASK) ? (1) : (0)]);
    fprintf(stdout,"\n");
}
