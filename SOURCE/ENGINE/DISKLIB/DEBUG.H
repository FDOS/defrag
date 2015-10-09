/*
 * debug.h
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

#define AMIBIOSBUG

/* dump registers and DOS errors during DOS/BIOS calls */

void err_dump(const char *s);

#if defined __GNUC__ && !defined _WIN32
#include <dpmi.h>
void reg_dump(__dpmi_regs *regs);
#endif
#if defined _WIN32
#include "win32.h"
void reg_dump(DIOC_REGISTERS *regs);
#endif
#if !defined __GNUC__ && !defined _WIN32
void reg_dump(union REGS *regs, struct SREGS *sregs);
#endif

#ifdef TRACE
#define DBG_err_dump(s) err_dump(s)
#ifdef __GNUC__
#define DBG_dpmi_dump(r) reg_dump(r)
#endif
#if defined _WIN32
#define DBG_reg_dump(r) reg_dump(r)
#endif
#if !defined __GNUC__ && !defined _WIN32
#define DBG_reg_dump(r,s) reg_dump(r,s)
#endif
#define DBG_0 ={0}
#include <string.h>
#define DBG_zero(b,s) memset(b,0,s)
#else
#define DBG_err_dump(s)
#if defined _WIN32
#define DBG_reg_dump(r)
#else
#define DBG_reg_dump(r,s)
#endif
#define DBG_dpmi_dump(r)
#define DBG_0
#define DBG_zero(b,s)
#endif

#endif
