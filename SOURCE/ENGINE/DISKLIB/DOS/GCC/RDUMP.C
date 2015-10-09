/*
 * dos\gcc\rdump.c      display 80x86 registers
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#include <stdio.h>

#include "dosmem.h"

#define O_MASK 0x0800
#define D_MASK 0x0400
#define I_MASK 0x0200
#define S_MASK 0x0080
#define Z_MASK 0x0040
#define A_MASK 0x0010
#define P_MASK 0x0004
#define C_MASK 0x0001

/*
 * dpmi_dump    display 80x86 registers for the DPMI calls
 *
 */

extern void reg_dump(__dpmi_regs *regs)
{
int f;
char *over[]={"NV","OV"};
char *dire[]={"UP","DN"};
char *inte[]={"DI","EI"};
char *sign[]={"PL","NG"};
char *zero[]={"NZ","ZR"};
char *auxi[]={"NA","AC"};
char *pari[]={"PO","PE"};
char *carr[]={"NC","CY"};

    printf("\n");
    printf("EAX=%08lX  EBX=%08lX  ECX=%08lX  EDX=%08lX  ",
            regs->d.eax,regs->d.ebx,regs->d.ecx,regs->d.edx);
    printf("\n");
    printf("ESI=%08lX  EDI=%08lX  EBP=%08lX  DS=%04X  ",
           regs->d.esi,regs->d.edi,regs->d.ebp, regs->x.ds);
    f = regs->x.flags;
    printf("%s ",over[(f&O_MASK) ? (1) : (0)]);
    printf("%s ",dire[(f&D_MASK) ? (1) : (0)]);
    printf("%s ",inte[(f&I_MASK) ? (1) : (0)]);
    printf("%s ",sign[(f&S_MASK) ? (1) : (0)]);
    printf("%s ",zero[(f&Z_MASK) ? (1) : (0)]);
    printf("%s ",auxi[(f&A_MASK) ? (1) : (0)]);
    printf("%s ",pari[(f&P_MASK) ? (1) : (0)]);
    printf("%s ",carr[(f&C_MASK) ? (1) : (0)]);
    printf("\n");
}
