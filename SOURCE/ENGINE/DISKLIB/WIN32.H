/*
 * win32.h
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#ifndef WIN32_H
#define WIN32_H

#if defined __STDC__ && defined _MSC_VER
#error "Windows' SHIT is not ANSI compatable (use /Ze or /ze)"
#endif
#if defined __WATCOMC__ && defined NO_EXT_KEYS
#error "Windows' SHIT is not ANSI compatable (do not use -za)"
#endif
#ifdef _MSC_VER
#pragma warning(disable:4001)   /* single line comment */
#pragma warning(disable:4115)   /* named type definition in parentheses */
#pragma warning(disable:4201)   /* nameless struct/union */
#pragma warning(disable:4209)   /* benign typedef */
#pragma warning(disable:4214)   /* bit field types other than int */
#pragma warning(disable:4515)   /* unreferenced inline function */
#endif
#define WIN32_LEAN_AND_MEAN

#define NO_INLINE_SHIT          /* I wrap those stupid inline functions */
                                /* Microsoft uses in headers with this */
                                /* macro (I hate compiler warnings) */
#include <windows.h>
#include <winioctl.h>           /* Cygnus' GCC does not have this, arrg! */

#pragma pack(1)

struct _DWORDREGS {
    DWORD ebx;
    DWORD edx;
    DWORD ecx;
    DWORD eax;
    DWORD edi;
    DWORD esi;
    DWORD flags;
};

struct _WORDREGS {
    WORD bx, _upper_bx;
    WORD dx, _upper_dx;
    WORD cx, _upper_cx;
    WORD ax, _upper_ax;
    WORD si, _upper_si;
    WORD di, _upper_di;
    WORD flags, _upper_flags;
};

struct _BYTEREGS {
    unsigned char bl,bh;
    unsigned short _upper_bx;
    unsigned char dl,dh;
    unsigned short _upper_dx;
    unsigned char cl,ch;
    unsigned short _upper_cx;
    unsigned char al,ah;
    unsigned short _upper_ax;
    unsigned short si, _upper_si;
    unsigned short di, _upper_di;
    unsigned short flags, _upper_flags;
};

typedef union _DIOC_REGISTERS {
    struct _DWORDREGS d;
    struct _WORDREGS x;
    struct _BYTEREGS h;
} DIOC_REGISTERS;

#if 0                       /* I was going to use these at one time */
                            /*  but am now not too sure... */
struct WORDREGS {
    unsigned short ax;
    unsigned short bx;
    unsigned short cx;
    unsigned short dx;
    unsigned short si;
    unsigned short di;
    unsigned short cflag;
};

/* byte registers */

struct BYTEREGS {
    unsigned char al, ah;
    unsigned char bl, bh;
    unsigned char cl, ch;
    unsigned char dl, dh;
};

union REGS {
    struct WORDREGS  x;
    struct BYTEREGS  h;
};

/* segment registers */

struct SREGS {
    unsigned short es, cs, ss, ds;
};

#define FP_SEG(p)   (p)
#define FP_OFF(p)   ((DWORD)(p))

#endif  /* 0 */

#pragma pack()

#define VWIN32_DIOC_DOS_IOCTL (1)
#define VWIN32_DIOC_DOS_INT25 (2)
#define VWIN32_DIOC_DOS_INT26 (3)
#define VWIN32_DIOC_DOS_INT13 (4)
#define VWIN32_DIOC_DOS_DRIVEINFO (6)

/* DEVICE.C */

extern int win_device_io(DWORD control, DIOC_REGISTERS *regs);

/* WINNT/OPEN.C */

extern HANDLE opendrive(int disk);
extern HANDLE opendrivephy(int disk);
extern void closedrive(HANDLE dev);

/* WLIB.C */

extern int kbhit(void);
extern int getch(void);

#endif
